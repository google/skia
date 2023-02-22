/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkSpan.h"
#include "include/private/SkSLDefines.h"
#include "include/private/SkSLIRNode.h"
#include "include/private/SkSLLayout.h"
#include "include/private/SkSLModifiers.h"
#include "include/private/SkSLProgramElement.h"
#include "include/private/SkSLStatement.h"
#include "include/private/SkSLString.h"
#include "include/private/base/SkTArray.h"
#include "include/sksl/SkSLOperator.h"
#include "include/sksl/SkSLPosition.h"
#include "src/base/SkStringView.h"
#include "src/core/SkTHash.h"
#include "src/sksl/SkSLAnalysis.h"
#include "src/sksl/SkSLBuiltinTypes.h"
#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLConstantFolder.h"
#include "src/sksl/SkSLIntrinsicList.h"
#include "src/sksl/codegen/SkSLRasterPipelineBuilder.h"
#include "src/sksl/codegen/SkSLRasterPipelineCodeGenerator.h"
#include "src/sksl/ir/SkSLBinaryExpression.h"
#include "src/sksl/ir/SkSLBlock.h"
#include "src/sksl/ir/SkSLBreakStatement.h"
#include "src/sksl/ir/SkSLChildCall.h"
#include "src/sksl/ir/SkSLConstructor.h"
#include "src/sksl/ir/SkSLConstructorDiagonalMatrix.h"
#include "src/sksl/ir/SkSLConstructorMatrixResize.h"
#include "src/sksl/ir/SkSLConstructorSplat.h"
#include "src/sksl/ir/SkSLContinueStatement.h"
#include "src/sksl/ir/SkSLDoStatement.h"
#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLExpressionStatement.h"
#include "src/sksl/ir/SkSLFieldAccess.h"
#include "src/sksl/ir/SkSLForStatement.h"
#include "src/sksl/ir/SkSLFunctionCall.h"
#include "src/sksl/ir/SkSLFunctionDeclaration.h"
#include "src/sksl/ir/SkSLFunctionDefinition.h"
#include "src/sksl/ir/SkSLIfStatement.h"
#include "src/sksl/ir/SkSLIndexExpression.h"
#include "src/sksl/ir/SkSLLiteral.h"
#include "src/sksl/ir/SkSLPostfixExpression.h"
#include "src/sksl/ir/SkSLPrefixExpression.h"
#include "src/sksl/ir/SkSLProgram.h"
#include "src/sksl/ir/SkSLReturnStatement.h"
#include "src/sksl/ir/SkSLSwitchCase.h"
#include "src/sksl/ir/SkSLSwitchStatement.h"
#include "src/sksl/ir/SkSLSwizzle.h"
#include "src/sksl/ir/SkSLTernaryExpression.h"
#include "src/sksl/ir/SkSLType.h"
#include "src/sksl/ir/SkSLVarDeclarations.h"
#include "src/sksl/ir/SkSLVariable.h"
#include "src/sksl/ir/SkSLVariableReference.h"
#include "src/sksl/tracing/SkRPDebugTrace.h"
#include "src/sksl/tracing/SkSLDebugInfo.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <float.h>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace SkSL {
namespace RP {

static bool unsupported() {
    // If MakeRasterPipelineProgram returns false, set a breakpoint here for more information.
    return false;
}

class SlotManager {
public:
    SlotManager(std::vector<SlotDebugInfo>* i) : fSlotDebugInfo(i) {}

    /** Used by `create` to add this variable to SlotDebugInfo inside SkRPDebugTrace. */
    void addSlotDebugInfoForGroup(const std::string& varName,
                                  const Type& type,
                                  Position pos,
                                  int* groupIndex,
                                  bool isFunctionReturnValue);
    void addSlotDebugInfo(const std::string& varName,
                          const Type& type,
                          Position pos,
                          bool isFunctionReturnValue);

    /** Creates slots associated with an SkSL variable or return value. */
    SlotRange createSlots(std::string name,
                          const Type& type,
                          Position pos,
                          bool isFunctionReturnValue);

    /**
     * Creates a single temporary slot for scratch storage. Temporary slots can be recycled, which
     * frees them up for reuse. Temporary slots are not assigned a name and have an arbitrary type.
     */
    SlotRange createTemporarySlot(const Type& type);
    void recycleTemporarySlot(SlotRange temporarySlot);


    /** Looks up the slots associated with an SkSL variable; creates the slot if necessary. */
    SlotRange getVariableSlots(const Variable& v);

    /**
     * Looks up the slots associated with an SkSL function's return value; creates the range if
     * necessary. Note that recursion is never supported, so we don't need to maintain return values
     * in a stack; we can just statically allocate one slot per function call-site.
     */
    SlotRange getFunctionSlots(const IRNode& callSite, const FunctionDeclaration& f);

    /** Returns the total number of slots consumed. */
    int slotCount() const { return fSlotCount; }

private:
    std::string makeTempName() { return SkSL::String::printf("[temporary %d]", fTemporaryCount++); }

    SkTHashMap<const IRNode*, SlotRange> fSlotMap;
    SkTArray<Slot> fRecycledSlots;
    int fSlotCount = 0;
    int fTemporaryCount = 0;
    std::vector<SlotDebugInfo>* fSlotDebugInfo;
};

class AutoContinueMask {
public:
    AutoContinueMask() = default;

    ~AutoContinueMask() {
        if (fSlotRange) {
            fSlotManager->recycleTemporarySlot(*fSlotRange);
            *fSlotRange = fPreviousSlotRange;
        }
    }

    void enable(SlotManager* mgr, const Type& type, SlotRange* range) {
        fSlotManager = mgr;
        fSlotRange = range;
        fPreviousSlotRange = *fSlotRange;
        *fSlotRange = fSlotManager->createTemporarySlot(type);
    }

    void enterLoopBody(Builder& builder) {
        if (fSlotRange) {
            builder.zero_slots_unmasked(*fSlotRange);
        }
    }

    void exitLoopBody(Builder& builder) {
        if (fSlotRange) {
            builder.reenable_loop_mask(*fSlotRange);
        }
    }

private:
    SlotManager* fSlotManager = nullptr;
    SlotRange* fSlotRange = nullptr;
    SlotRange fPreviousSlotRange;
};

class LValue;

class Generator {
public:
    Generator(const SkSL::Program& program, SkRPDebugTrace* debugTrace)
            : fProgram(program)
            , fDebugTrace(debugTrace)
            , fProgramSlots(debugTrace ? &debugTrace->fSlotInfo : nullptr)
            , fUniformSlots(debugTrace ? &debugTrace->fUniformInfo : nullptr) {}

    /** Converts the SkSL main() function into a set of Instructions. */
    bool writeProgram(const FunctionDefinition& function);

    /** Returns the generated program. */
    std::unique_ptr<RP::Program> finish();

    /**
     * Converts an SkSL function into a set of Instructions. Returns nullopt if the function
     * contained unsupported statements or expressions.
     */
    std::optional<SlotRange> writeFunction(const IRNode& callSite,
                                           const FunctionDefinition& function);

    /**
     * Returns the slot index of this function inside the FunctionDebugInfo array in SkRPDebugTrace.
     * The FunctionDebugInfo slot will be created if it doesn't already exist.
     */
    int getFunctionDebugInfo(const FunctionDeclaration& decl);

    /** Looks up the slots associated with an SkSL variable; creates the slot if necessary. */
    SlotRange getVariableSlots(const Variable& v) {
        SkASSERT(!IsUniform(v));
        return fProgramSlots.getVariableSlots(v);
    }

    /** Looks up the slots associated with an SkSL uniform; creates the slot if necessary. */
    SlotRange getUniformSlots(const Variable& v) {
        SkASSERT(IsUniform(v));
        return fUniformSlots.getVariableSlots(v);
    }

    /**
     * Looks up the slots associated with an SkSL function's return value; creates the range if
     * necessary. Note that recursion is never supported, so we don't need to maintain return values
     * in a stack; we can just statically allocate one slot per function call-site.
     */
    SlotRange getFunctionSlots(const IRNode& callSite, const FunctionDeclaration& f) {
        return fProgramSlots.getFunctionSlots(callSite, f);
    }

    /**
     * Creates an additional stack for the program to push values onto. The stack will not become
     * actively in-use until `setCurrentStack` is called.
     */
    int createStack();

    /** Frees a stack generated by `createStack`. The freed stack must be completely empty. */
    void recycleStack(int stackID);

    /** Redirects builder ops to point to a different stack (created by `createStack`). */
    void setCurrentStack(int stackID);

    /** Reports the currently active stack. */
    int currentStack() {
        return fCurrentStack;
    }

    /**
     * Returns an LValue for the passed-in expression; if the expression isn't supported as an
     * LValue, returns nullptr.
     */
    std::unique_ptr<LValue> makeLValue(const Expression& e, bool allowScratch = false);

    /** Copies the top-of-stack value into this lvalue, without discarding it from the stack. */
    [[nodiscard]] bool store(LValue& lvalue);

    /** Pushes the lvalue onto the top-of-stack. */
    [[nodiscard]] bool push(LValue& lvalue);

    /** The Builder stitches our instructions together into Raster Pipeline code. */
    Builder* builder() { return &fBuilder; }

    /** Appends a statement to the program. */
    [[nodiscard]] bool writeStatement(const Statement& s);
    [[nodiscard]] bool writeBlock(const Block& b);
    [[nodiscard]] bool writeBreakStatement(const BreakStatement& b);
    [[nodiscard]] bool writeContinueStatement(const ContinueStatement& b);
    [[nodiscard]] bool writeDoStatement(const DoStatement& d);
    [[nodiscard]] bool writeExpressionStatement(const ExpressionStatement& e);
    [[nodiscard]] bool writeMasklessForStatement(const ForStatement& f);
    [[nodiscard]] bool writeForStatement(const ForStatement& f);
    [[nodiscard]] bool writeGlobals();
    [[nodiscard]] bool writeIfStatement(const IfStatement& i);
    [[nodiscard]] bool writeDynamicallyUniformIfStatement(const IfStatement& i);
    [[nodiscard]] bool writeReturnStatement(const ReturnStatement& r);
    [[nodiscard]] bool writeSwitchStatement(const SwitchStatement& s);
    [[nodiscard]] bool writeVarDeclaration(const VarDeclaration& v);

    /** Pushes an expression to the value stack. */
    [[nodiscard]] bool pushBinaryExpression(const BinaryExpression& e);
    [[nodiscard]] bool pushBinaryExpression(const Expression& left,
                                            Operator op,
                                            const Expression& right);
    [[nodiscard]] bool pushChildCall(const ChildCall& c);
    [[nodiscard]] bool pushConstructorCast(const AnyConstructor& c);
    [[nodiscard]] bool pushConstructorCompound(const AnyConstructor& c);
    [[nodiscard]] bool pushConstructorDiagonalMatrix(const ConstructorDiagonalMatrix& c);
    [[nodiscard]] bool pushConstructorMatrixResize(const ConstructorMatrixResize& c);
    [[nodiscard]] bool pushConstructorSplat(const ConstructorSplat& c);
    [[nodiscard]] bool pushExpression(const Expression& e, bool usesResult = true);
    [[nodiscard]] bool pushFieldAccess(const FieldAccess& f);
    [[nodiscard]] bool pushFunctionCall(const FunctionCall& c);
    [[nodiscard]] bool pushIndexExpression(const IndexExpression& i);
    [[nodiscard]] bool pushIntrinsic(const FunctionCall& c);
    [[nodiscard]] bool pushIntrinsic(IntrinsicKind intrinsic, const Expression& arg0);
    [[nodiscard]] bool pushIntrinsic(IntrinsicKind intrinsic,
                                     const Expression& arg0,
                                     const Expression& arg1);
    [[nodiscard]] bool pushIntrinsic(IntrinsicKind intrinsic,
                                     const Expression& arg0,
                                     const Expression& arg1,
                                     const Expression& arg2);
    [[nodiscard]] bool pushLiteral(const Literal& l);
    [[nodiscard]] bool pushPostfixExpression(const PostfixExpression& p, bool usesResult);
    [[nodiscard]] bool pushPrefixExpression(const PrefixExpression& p);
    [[nodiscard]] bool pushPrefixExpression(Operator op, const Expression& expr);
    [[nodiscard]] bool pushSwizzle(const Swizzle& s);
    [[nodiscard]] bool pushTernaryExpression(const TernaryExpression& t);
    [[nodiscard]] bool pushTernaryExpression(const Expression& test,
                                             const Expression& ifTrue,
                                             const Expression& ifFalse);
    [[nodiscard]] bool pushDynamicallyUniformTernaryExpression(const Expression& test,
                                                               const Expression& ifTrue,
                                                               const Expression& ifFalse);
    [[nodiscard]] bool pushVariableReference(const VariableReference& v);

    /** Pops an expression from the value stack and copies it into slots. */
    void popToSlotRange(SlotRange r) { fBuilder.pop_slots(r); }
    void popToSlotRangeUnmasked(SlotRange r) { fBuilder.pop_slots_unmasked(r); }

    /** Pops an expression from the value stack and discards it. */
    void discardExpression(int slots) { fBuilder.discard_stack(slots); }

    /** Zeroes out a range of slots. */
    void zeroSlotRangeUnmasked(SlotRange r) { fBuilder.zero_slots_unmasked(r); }

    /** Expression utilities. */
    struct TypedOps {
        BuilderOp fFloatOp;
        BuilderOp fSignedOp;
        BuilderOp fUnsignedOp;
        BuilderOp fBooleanOp;
    };

    static BuilderOp GetTypedOp(const SkSL::Type& type, const TypedOps& ops);

    [[nodiscard]] bool unaryOp(const SkSL::Type& type, const TypedOps& ops);
    [[nodiscard]] bool binaryOp(const SkSL::Type& type, const TypedOps& ops);
    [[nodiscard]] bool ternaryOp(const SkSL::Type& type, const TypedOps& ops);
    [[nodiscard]] bool pushIntrinsic(const TypedOps& ops, const Expression& arg0);
    [[nodiscard]] bool pushIntrinsic(const TypedOps& ops,
                                     const Expression& arg0,
                                     const Expression& arg1);
    [[nodiscard]] bool pushIntrinsic(BuilderOp builderOp, const Expression& arg0);
    [[nodiscard]] bool pushIntrinsic(BuilderOp builderOp,
                                     const Expression& arg0,
                                     const Expression& arg1);
    [[nodiscard]] bool pushVectorizedExpression(const Expression& expr, const Type& vectorType);
    [[nodiscard]] bool pushVariableReferencePartial(const VariableReference& v, SlotRange subset);
    [[nodiscard]] bool pushLValueOrExpression(LValue* lvalue, const Expression& expr);
    [[nodiscard]] bool pushMatrixMultiply(LValue* lvalue,
                                          const Expression& left,
                                          const Expression& right,
                                          int leftColumns, int leftRows,
                                          int rightColumns, int rightRows);
    [[nodiscard]] bool pushStructuredComparison(LValue* left,
                                                Operator op,
                                                LValue* right,
                                                const Type& type);

    void foldWithMultiOp(BuilderOp op, int elements);
    void foldComparisonOp(Operator op, int elements);

    BuilderOp getTypedOp(const SkSL::Type& type, const TypedOps& ops) const;

    bool needsReturnMask() {
        Analysis::ReturnComplexity* complexity = fReturnComplexityMap.find(fCurrentFunction);
        if (!complexity) {
            complexity = fReturnComplexityMap.set(fCurrentFunction,
                                                  Analysis::GetReturnComplexity(*fCurrentFunction));
        }
        return *complexity >= Analysis::ReturnComplexity::kEarlyReturns;
    }

    static bool IsUniform(const Variable& var) {
       return var.modifiers().fFlags & Modifiers::kUniform_Flag;
    }

    static bool IsOutParameter(const Variable& var) {
        return (var.modifiers().fFlags & (Modifiers::kIn_Flag | Modifiers::kOut_Flag)) ==
               Modifiers::kOut_Flag;
    }

    static bool IsInoutParameter(const Variable& var) {
        return (var.modifiers().fFlags & (Modifiers::kIn_Flag | Modifiers::kOut_Flag)) ==
               (Modifiers::kIn_Flag | Modifiers::kOut_Flag);
    }

private:
    const SkSL::Program& fProgram;
    Builder fBuilder;
    SkRPDebugTrace* fDebugTrace = nullptr;
    SkTHashMap<const Variable*, int> fChildEffectMap;

    SlotManager fProgramSlots;
    SlotManager fUniformSlots;

    const FunctionDefinition* fCurrentFunction = nullptr;
    SlotRange fCurrentFunctionResult;
    SlotRange fCurrentContinueMask;
    int fCurrentStack = 0;
    int fNextStackID = 0;
    SkTArray<int> fRecycledStacks;

    SkTHashMap<const FunctionDefinition*, Analysis::ReturnComplexity> fReturnComplexityMap;

    static constexpr auto kAbsOps = TypedOps{BuilderOp::abs_float,
                                             BuilderOp::abs_int,
                                             BuilderOp::unsupported,
                                             BuilderOp::unsupported};
    static constexpr auto kAddOps = TypedOps{BuilderOp::add_n_floats,
                                             BuilderOp::add_n_ints,
                                             BuilderOp::add_n_ints,
                                             BuilderOp::unsupported};
    static constexpr auto kSubtractOps = TypedOps{BuilderOp::sub_n_floats,
                                                  BuilderOp::sub_n_ints,
                                                  BuilderOp::sub_n_ints,
                                                  BuilderOp::unsupported};
    static constexpr auto kMultiplyOps = TypedOps{BuilderOp::mul_n_floats,
                                                  BuilderOp::mul_n_ints,
                                                  BuilderOp::mul_n_ints,
                                                  BuilderOp::unsupported};
    static constexpr auto kDivideOps = TypedOps{BuilderOp::div_n_floats,
                                                BuilderOp::div_n_ints,
                                                BuilderOp::div_n_uints,
                                                BuilderOp::unsupported};
    static constexpr auto kLessThanOps = TypedOps{BuilderOp::cmplt_n_floats,
                                                  BuilderOp::cmplt_n_ints,
                                                  BuilderOp::cmplt_n_uints,
                                                  BuilderOp::unsupported};
    static constexpr auto kLessThanEqualOps = TypedOps{BuilderOp::cmple_n_floats,
                                                       BuilderOp::cmple_n_ints,
                                                       BuilderOp::cmple_n_uints,
                                                       BuilderOp::unsupported};
    static constexpr auto kEqualOps = TypedOps{BuilderOp::cmpeq_n_floats,
                                               BuilderOp::cmpeq_n_ints,
                                               BuilderOp::cmpeq_n_ints,
                                               BuilderOp::cmpeq_n_ints};
    static constexpr auto kNotEqualOps = TypedOps{BuilderOp::cmpne_n_floats,
                                                  BuilderOp::cmpne_n_ints,
                                                  BuilderOp::cmpne_n_ints,
                                                  BuilderOp::cmpne_n_ints};
    static constexpr auto kMinOps = TypedOps{BuilderOp::min_n_floats,
                                             BuilderOp::min_n_ints,
                                             BuilderOp::min_n_uints,
                                             BuilderOp::min_n_uints};
    static constexpr auto kMaxOps = TypedOps{BuilderOp::max_n_floats,
                                             BuilderOp::max_n_ints,
                                             BuilderOp::max_n_uints,
                                             BuilderOp::max_n_uints};
    static constexpr auto kMixOps = TypedOps{BuilderOp::mix_n_floats,
                                             BuilderOp::unsupported,
                                             BuilderOp::unsupported,
                                             BuilderOp::unsupported};
};

class AutoStack {
public:
    explicit AutoStack(Generator* g)
            : fGenerator(g)
            , fStackID(g->createStack()) {}

    ~AutoStack() {
        fGenerator->recycleStack(fStackID);
    }

    void enter() {
        fParentStackID = fGenerator->currentStack();
        fGenerator->setCurrentStack(fStackID);
    }

    void exit() {
        SkASSERT(fGenerator->currentStack() == fStackID);
        fGenerator->setCurrentStack(fParentStackID);
    }

    void pushClone(int slots, int offsetFromStackTop = 0) {
        fGenerator->builder()->push_clone_from_stack(slots, fStackID, offsetFromStackTop);
    }

private:
    Generator* fGenerator;
    int fStackID = 0;
    int fParentStackID = 0;
};

class LValue {
public:
    virtual ~LValue() = default;

    /** Returns true if this lvalue is actually writable--temporaries and uniforms are not. */
    virtual bool isWritable() const = 0;

    /**
     * Returns the slot range of the lvalue, after it is winnowed down to the selected field/index.
     * The range is calculated assuming every dynamic index will evaluate to zero.
     */
    virtual SlotRange fixedSlotRange(Generator* gen) = 0;

    /** Pushes values directly onto the stack. */
    [[nodiscard]] virtual bool push(Generator* gen,
                                    SlotRange fixedOffset,
                                    SkSpan<const int8_t> swizzle) = 0;

    /** Stores topmost values from the stack directly into the lvalue. */
    [[nodiscard]] virtual bool store(Generator* gen,
                                     SlotRange fixedOffset,
                                     SkSpan<const int8_t> swizzle) = 0;
};

class ScratchLValue final : public LValue {
public:
    explicit ScratchLValue(const Expression& e)
            : fExpression(&e)
            , fNumSlots(e.type().slotCount()) {}

    ~ScratchLValue() override {
        if (fGenerator && fDedicatedStack.has_value()) {
            // Jettison the scratch expression.
            fDedicatedStack->enter();
            fGenerator->discardExpression(fNumSlots);
            fDedicatedStack->exit();
        }
    }

    bool isWritable() const override {
        return false;
    }

    SlotRange fixedSlotRange(Generator* gen) override {
        return SlotRange{0, fNumSlots};
    }

    [[nodiscard]] bool push(Generator* gen,
                            SlotRange fixedOffset,
                            SkSpan<const int8_t> swizzle) override {
        if (!fDedicatedStack.has_value()) {
            // Push the scratch expression onto a dedicated stack.
            fGenerator = gen;
            fDedicatedStack.emplace(fGenerator);
            fDedicatedStack->enter();
            if (!fGenerator->pushExpression(*fExpression)) {
                return unsupported();
            }
            fDedicatedStack->exit();
        }

        fDedicatedStack->pushClone(fixedOffset.count,
                                   fNumSlots - fixedOffset.count - fixedOffset.index);
        if (!swizzle.empty()) {
            gen->builder()->swizzle(fixedOffset.count, swizzle);
        }
        return true;
    }

    [[nodiscard]] bool store(Generator*, SlotRange, SkSpan<const int8_t>) override {
        SkDEBUGFAIL("scratch lvalues cannot be stored into");
        return unsupported();
    }

private:
    Generator* fGenerator = nullptr;
    const Expression* fExpression = nullptr;
    std::optional<AutoStack> fDedicatedStack;
    int fNumSlots = 0;
};

class VariableLValue final : public LValue {
public:
    explicit VariableLValue(const Variable* v) : fVariable(v) {}

    bool isWritable() const override {
        return !Generator::IsUniform(*fVariable);
    }

    SlotRange fixedSlotRange(Generator* gen) override {
        return Generator::IsUniform(*fVariable) ? gen->getUniformSlots(*fVariable)
                                                : gen->getVariableSlots(*fVariable);
    }

    [[nodiscard]] bool push(Generator* gen,
                            SlotRange fixedOffset,
                            SkSpan<const int8_t> swizzle) override {
        if (Generator::IsUniform(*fVariable)) {
            gen->builder()->push_uniform(fixedOffset);
        } else {
            gen->builder()->push_slots(fixedOffset);
        }
        if (!swizzle.empty()) {
            gen->builder()->swizzle(fixedOffset.count, swizzle);
        }
        return true;
    }

    [[nodiscard]] bool store(Generator* gen,
                             SlotRange fixedOffset,
                             SkSpan<const int8_t> swizzle) override {
        SkASSERT(!Generator::IsUniform(*fVariable));

        if (swizzle.empty()) {
            gen->builder()->copy_stack_to_slots(fixedOffset, fixedOffset.count);
        } else {
            gen->builder()->swizzle_copy_stack_to_slots(fixedOffset, swizzle, swizzle.size());
        }
        return true;
    }

private:
    const Variable* fVariable;
};

class SwizzleLValue final : public LValue {
public:
    explicit SwizzleLValue(std::unique_ptr<LValue> p, const ComponentArray& c)
            : fParent(std::move(p))
            , fComponents(c) {
        SkASSERT(!fComponents.empty() && fComponents.size() <= 4);
    }

    bool isWritable() const override {
        return fParent->isWritable();
    }

    SlotRange fixedSlotRange(Generator* gen) override {
        return fParent->fixedSlotRange(gen);
    }

    [[nodiscard]] bool push(Generator* gen,
                            SlotRange fixedOffset,
                            SkSpan<const int8_t> swizzle) override {
        if (!swizzle.empty()) {
            SkDEBUGFAIL("swizzle-of-a-swizzle should have been folded out in front end");
            return unsupported();
        }
        return fParent->push(gen, fixedOffset, fComponents);
    }

    [[nodiscard]] bool store(Generator* gen,
                             SlotRange fixedOffset,
                             SkSpan<const int8_t> swizzle) override {
        if (!swizzle.empty()) {
            SkDEBUGFAIL("swizzle-of-a-swizzle should have been folded out in front end");
            return unsupported();
        }
        return fParent->store(gen, fixedOffset, fComponents);
    }

private:
    std::unique_ptr<LValue> fParent;
    const ComponentArray& fComponents;
};

class UnownedLValueSlice : public LValue {
public:
    explicit UnownedLValueSlice(LValue* p, int initialSlot, int numSlots)
            : fParent(p)
            , fInitialSlot(initialSlot)
            , fNumSlots(numSlots) {
        SkASSERT(fInitialSlot >= 0);
        SkASSERT(fNumSlots > 0);
    }

    bool isWritable() const override {
        return fParent->isWritable();
    }

    SlotRange fixedSlotRange(Generator* gen) override {
        SlotRange range = fParent->fixedSlotRange(gen);
        SlotRange adjusted = range;
        adjusted.index += fInitialSlot;
        adjusted.count = fNumSlots;
        SkASSERT((adjusted.index + adjusted.count) <= (range.index + range.count));
        return adjusted;
    }

    [[nodiscard]] bool push(Generator* gen,
                            SlotRange fixedOffset,
                            SkSpan<const int8_t> swizzle) override {
        return fParent->push(gen, fixedOffset, swizzle);
    }

    [[nodiscard]] bool store(Generator* gen,
                             SlotRange fixedOffset,
                             SkSpan<const int8_t> swizzle) override {
        return fParent->store(gen, fixedOffset, swizzle);
    }

protected:
    LValue* fParent;

private:
    int fInitialSlot = 0;
    int fNumSlots = 0;
};

class LValueSlice final : public UnownedLValueSlice {
public:
    explicit LValueSlice(std::unique_ptr<LValue> p, int initialSlot, int numSlots)
            : UnownedLValueSlice(p.release(), initialSlot, numSlots) {}

    ~LValueSlice() override {
        delete fParent;
    }
};

void SlotManager::addSlotDebugInfoForGroup(const std::string& varName,
                                           const Type& type,
                                           Position pos,
                                           int* groupIndex,
                                           bool isFunctionReturnValue) {
    SkASSERT(fSlotDebugInfo);
    switch (type.typeKind()) {
        case Type::TypeKind::kArray: {
            int nslots = type.columns();
            const Type& elemType = type.componentType();
            for (int slot = 0; slot < nslots; ++slot) {
                this->addSlotDebugInfoForGroup(varName + "[" + std::to_string(slot) + "]", elemType,
                                               pos, groupIndex, isFunctionReturnValue);
            }
            break;
        }
        case Type::TypeKind::kStruct: {
            for (const Type::Field& field : type.fields()) {
                this->addSlotDebugInfoForGroup(varName + "." + std::string(field.fName),
                                               *field.fType, pos, groupIndex,
                                               isFunctionReturnValue);
            }
            break;
        }
        default:
            SkASSERTF(0, "unsupported slot type %d", (int)type.typeKind());
            [[fallthrough]];

        case Type::TypeKind::kScalar:
        case Type::TypeKind::kVector:
        case Type::TypeKind::kMatrix: {
            Type::NumberKind numberKind = type.componentType().numberKind();
            int nslots = type.slotCount();

            for (int slot = 0; slot < nslots; ++slot) {
                SlotDebugInfo slotInfo;
                slotInfo.name = varName;
                slotInfo.columns = type.columns();
                slotInfo.rows = type.rows();
                slotInfo.componentIndex = slot;
                slotInfo.groupIndex = (*groupIndex)++;
                slotInfo.numberKind = numberKind;
                slotInfo.pos = pos;
                slotInfo.fnReturnValue = isFunctionReturnValue ? 1 : -1;
                fSlotDebugInfo->push_back(std::move(slotInfo));
            }
            break;
        }
    }
}

void SlotManager::addSlotDebugInfo(const std::string& varName,
                                   const Type& type,
                                   Position pos,
                                   bool isFunctionReturnValue) {
    int groupIndex = 0;
    this->addSlotDebugInfoForGroup(varName, type, pos, &groupIndex, isFunctionReturnValue);
    SkASSERT((size_t)groupIndex == type.slotCount());
}

SlotRange SlotManager::createTemporarySlot(const Type& type) {
    SkASSERT(type.slotCount() == 1);

    // If we have an available slot to reclaim, take it now.
    if (!fRecycledSlots.empty()) {
        SlotRange result = {fRecycledSlots.back(), 1};
        fRecycledSlots.pop_back();
        return result;
    }

    // Synthesize a new temporary slot.
    if (fSlotDebugInfo) {
        // Our debug slot-info table should have the same length as the actual slot table.
        SkASSERT(fSlotDebugInfo->size() == (size_t)fSlotCount);

        // Add this temporary slot to the debug slot-info table. It's just scratch space which can
        // be reused over the course of execution, so it doesn't get a name or type (uint will do).
        this->addSlotDebugInfo(this->makeTempName(), type, Position{},
                               /*isFunctionReturnValue=*/false);

        // Confirm that we added the expected number of slots.
        SkASSERT(fSlotDebugInfo->size() == (size_t)(fSlotCount + 1));
    }

    SlotRange result = {fSlotCount, 1};
    ++fSlotCount;
    return result;
}

void SlotManager::recycleTemporarySlot(SlotRange temporarySlot) {
    SkASSERT(temporarySlot.count == 1);
    fRecycledSlots.push_back(temporarySlot.index);
}

SlotRange SlotManager::createSlots(std::string name,
                                   const Type& type,
                                   Position pos,
                                   bool isFunctionReturnValue) {
    size_t nslots = type.slotCount();
    if (nslots == 0) {
        return {};
    }
    if (fSlotDebugInfo) {
        // Our debug slot-info table should have the same length as the actual slot table.
        SkASSERT(fSlotDebugInfo->size() == (size_t)fSlotCount);

        // Append slot names and types to our debug slot-info table.
        fSlotDebugInfo->reserve(fSlotCount + nslots);
        this->addSlotDebugInfo(name, type, pos, isFunctionReturnValue);

        // Confirm that we added the expected number of slots.
        SkASSERT(fSlotDebugInfo->size() == (size_t)(fSlotCount + nslots));
    }

    SlotRange result = {fSlotCount, (int)nslots};
    fSlotCount += nslots;
    return result;
}

SlotRange SlotManager::getVariableSlots(const Variable& v) {
    SlotRange* entry = fSlotMap.find(&v);
    if (entry != nullptr) {
        return *entry;
    }
    SlotRange range = this->createSlots(std::string(v.name()),
                                        v.type(),
                                        v.fPosition,
                                        /*isFunctionReturnValue=*/false);
    fSlotMap.set(&v, range);
    return range;
}

SlotRange SlotManager::getFunctionSlots(const IRNode& callSite, const FunctionDeclaration& f) {
    SlotRange* entry = fSlotMap.find(&callSite);
    if (entry != nullptr) {
        return *entry;
    }
    SlotRange range = this->createSlots("[" + std::string(f.name()) + "].result",
                                        f.returnType(),
                                        f.fPosition,
                                        /*isFunctionReturnValue=*/true);
    fSlotMap.set(&callSite, range);
    return range;
}

static bool is_sliceable_swizzle(SkSpan<const int8_t> components) {
    // Determine if the swizzle rearranges its elements, or if it's a simple subset of its elements.
    // (A simple subset would be a sequential non-repeating range of components, like `.xyz` or
    // `.yzw` or `.z`, but not `.xx` or `.xz`, which can be accessed as a slice of the variable.)
    for (size_t index = 1; index < components.size(); ++index) {
        if (components[index] != int8_t(components[0] + index)) {
            return false;
        }
    }
    return true;
}

std::unique_ptr<LValue> Generator::makeLValue(const Expression& e, bool allowScratch) {
    if (e.is<VariableReference>()) {
        return std::make_unique<VariableLValue>(e.as<VariableReference>().variable());
    }
    if (e.is<Swizzle>()) {
        const Swizzle& swizzleExpr = e.as<Swizzle>();
        if (std::unique_ptr<LValue> base = this->makeLValue(*swizzleExpr.base(),
                                                            allowScratch)) {
            const ComponentArray& components = swizzleExpr.components();
            if (is_sliceable_swizzle(components)) {
                // If the swizzle is a contiguous subset, we can represent it with a fixed slice.
                return std::make_unique<LValueSlice>(std::move(base), components[0],
                                                     components.size());
            }
            return std::make_unique<SwizzleLValue>(std::move(base), components);
        }
        return nullptr;
    }
    if (e.is<FieldAccess>()) {
        const FieldAccess& fieldExpr = e.as<FieldAccess>();
        if (std::unique_ptr<LValue> base = this->makeLValue(*fieldExpr.base(),
                                                            allowScratch)) {
            // Represent field access with a slice.
            return std::make_unique<LValueSlice>(std::move(base), fieldExpr.initialSlot(),
                                                 fieldExpr.type().slotCount());
        }
        return nullptr;
    }
    if (e.is<IndexExpression>()) {
        const IndexExpression& indexExpr = e.as<IndexExpression>();
        if (std::unique_ptr<LValue> base = this->makeLValue(*indexExpr.base(),
                                                            allowScratch)) {
            // If the index is a compile-time constant, we can represent it with a fixed slice.
            SKSL_INT indexValue;
            if (ConstantFolder::GetConstantInt(*indexExpr.index(), &indexValue)) {
                int numSlots = indexExpr.type().slotCount();
                return std::make_unique<LValueSlice>(std::move(base), numSlots * indexValue,
                                                     numSlots);
            }

            // TODO(skia:13676): support non-constant indices
        }
        return nullptr;
    }
    if (allowScratch) {
        // This path allows us to perform field- and index-accesses on an expression as if it were
        // an lvalue, but is a temporary and shouldn't be written back to.
        return std::make_unique<ScratchLValue>(e);
    }
    return nullptr;
}

bool Generator::store(LValue& lvalue) {
    SkASSERT(lvalue.isWritable());
    return lvalue.store(this, lvalue.fixedSlotRange(this), /*swizzle=*/{});
}

bool Generator::push(LValue& lvalue) {
    return lvalue.push(this, lvalue.fixedSlotRange(this), /*swizzle=*/{});
}

int Generator::getFunctionDebugInfo(const FunctionDeclaration& decl) {
    SkASSERT(fDebugTrace);

    std::string name = decl.description();

    // When generating the debug trace, we typically mark every function as `noinline`. This makes
    // the trace more confusing, since this isn't in the source program, so remove it.
    static constexpr std::string_view kNoInline = "noinline ";
    if (skstd::starts_with(name, kNoInline)) {
        name = name.substr(kNoInline.size());
    }

    // Look for a matching FunctionDebugInfo slot.
    for (size_t index = 0; index < fDebugTrace->fFuncInfo.size(); ++index) {
        if (fDebugTrace->fFuncInfo[index].name == name) {
            return index;
        }
    }

    // We've never called this function before; create a new slot to hold its information.
    int slot = (int)fDebugTrace->fFuncInfo.size();
    fDebugTrace->fFuncInfo.push_back(FunctionDebugInfo{std::move(name)});
    return slot;
}

int Generator::createStack() {
    if (!fRecycledStacks.empty()) {
        int stackID = fRecycledStacks.back();
        fRecycledStacks.pop_back();
        return stackID;
    }
    return ++fNextStackID;
}

void Generator::recycleStack(int stackID) {
    fRecycledStacks.push_back(stackID);
}

void Generator::setCurrentStack(int stackID) {
    if (fCurrentStack != stackID) {
        fCurrentStack = stackID;
        fBuilder.set_current_stack(stackID);
    }
}

std::optional<SlotRange> Generator::writeFunction(const IRNode& callSite,
                                                  const FunctionDefinition& function) {
    [[maybe_unused]] int funcIndex = -1;
    if (fDebugTrace) {
        funcIndex = this->getFunctionDebugInfo(function.declaration());
        SkASSERT(funcIndex >= 0);
        // TODO(debugger): add trace for function-enter
    }

    SlotRange lastFunctionResult = fCurrentFunctionResult;
    fCurrentFunctionResult = this->getFunctionSlots(callSite, function.declaration());

    if (!this->writeStatement(*function.body())) {
        return std::nullopt;
    }

    SlotRange functionResult = fCurrentFunctionResult;
    fCurrentFunctionResult = lastFunctionResult;

    if (fDebugTrace) {
        // TODO(debugger): add trace for function-exit
    }

    return functionResult;
}

bool Generator::writeGlobals() {
    for (const ProgramElement* e : fProgram.elements()) {
        if (e->is<GlobalVarDeclaration>()) {
            const GlobalVarDeclaration& gvd = e->as<GlobalVarDeclaration>();
            const VarDeclaration& decl = gvd.varDeclaration();
            const Variable* var = decl.var();

            if (var->type().isEffectChild()) {
                // Associate each child effect variable with its numeric index.
                SkASSERT(!fChildEffectMap.find(var));
                int childEffectIndex = fChildEffectMap.count();
                fChildEffectMap[var] = childEffectIndex;
                continue;
            }

            // Opaque types include child processors and GL objects (samplers, textures, etc).
            // Of those, only child processors are legal variables.
            SkASSERT(!var->type().isVoid());
            SkASSERT(!var->type().isOpaque());

            // Builtin variables are system-defined, with special semantics.
            if (int builtin = var->modifiers().fLayout.fBuiltin; builtin >= 0) {
                if (builtin == SK_FRAGCOORD_BUILTIN) {
                    fBuilder.store_device_xy01(this->getVariableSlots(*var));
                    continue;
                }
                // The only builtin variable exposed to runtime effects is sk_FragCoord.
                return unsupported();
            }

            if (IsUniform(*var)) {
                // Create the uniform slot map in first-to-last order.
                (void)this->getUniformSlots(*var);
                continue;
            }

            // Other globals are treated as normal variable declarations.
            if (!this->writeVarDeclaration(decl)) {
                return unsupported();
            }
        }
    }

    return true;
}

bool Generator::writeStatement(const Statement& s) {
    switch (s.kind()) {
        case Statement::Kind::kBlock:
            return this->writeBlock(s.as<Block>());

        case Statement::Kind::kBreak:
            return this->writeBreakStatement(s.as<BreakStatement>());

        case Statement::Kind::kContinue:
            return this->writeContinueStatement(s.as<ContinueStatement>());

        case Statement::Kind::kDo:
            return this->writeDoStatement(s.as<DoStatement>());

        case Statement::Kind::kExpression:
            return this->writeExpressionStatement(s.as<ExpressionStatement>());

        case Statement::Kind::kFor:
            return this->writeForStatement(s.as<ForStatement>());

        case Statement::Kind::kIf:
            return this->writeIfStatement(s.as<IfStatement>());

        case Statement::Kind::kNop:
            return true;

        case Statement::Kind::kReturn:
            return this->writeReturnStatement(s.as<ReturnStatement>());

        case Statement::Kind::kSwitch:
            return this->writeSwitchStatement(s.as<SwitchStatement>());

        case Statement::Kind::kVarDeclaration:
            return this->writeVarDeclaration(s.as<VarDeclaration>());

        default:
            return unsupported();
    }
}

bool Generator::writeBlock(const Block& b) {
    for (const std::unique_ptr<Statement>& stmt : b.children()) {
        if (!this->writeStatement(*stmt)) {
            return unsupported();
        }
    }
    return true;
}

bool Generator::writeBreakStatement(const BreakStatement&) {
    fBuilder.mask_off_loop_mask();
    return true;
}

bool Generator::writeContinueStatement(const ContinueStatement&) {
    // This could be written as one hand-tuned RasterPipeline op, but for now, we reuse existing ops
    // to assemble a continue op.

    // Set any currently-executing lanes in the continue-mask to true via push-pop.
    SkASSERT(fCurrentContinueMask.count == 1);
    fBuilder.push_literal_i(~0);
    this->popToSlotRange(fCurrentContinueMask);

    // Disable any currently-executing lanes from the loop mask.
    fBuilder.mask_off_loop_mask();
    return true;
}

bool Generator::writeDoStatement(const DoStatement& d) {
    // Save off the original loop mask.
    fBuilder.enableExecutionMaskWrites();
    fBuilder.push_loop_mask();

    // If `continue` is used in the loop...
    Analysis::LoopControlFlowInfo loopInfo = Analysis::GetLoopControlFlowInfo(*d.statement());
    AutoContinueMask autoContinueMask;
    if (loopInfo.fHasContinue) {
        // ... create a temporary slot for continue-mask storage.
        autoContinueMask.enable(&fProgramSlots, *fProgram.fContext->fTypes.fUInt,
                                &fCurrentContinueMask);
    }

    // Write the do-loop body.
    int labelID = fBuilder.nextLabelID();
    fBuilder.label(labelID);

    autoContinueMask.enterLoopBody(fBuilder);

    if (!this->writeStatement(*d.statement())) {
        return false;
    }

    autoContinueMask.exitLoopBody(fBuilder);

    // Emit the test-expression, in order to combine it with the loop mask.
    if (!this->pushExpression(*d.test())) {
        return false;
    }

    // Mask off any lanes in the loop mask where the test-expression is false; this breaks the loop.
    // We don't use the test expression for anything else, so jettison it.
    fBuilder.merge_loop_mask();
    this->discardExpression(/*slots=*/1);

    // If any lanes are still running, go back to the top and run the loop body again.
    fBuilder.branch_if_any_active_lanes(labelID);

    // Restore the loop mask.
    fBuilder.pop_loop_mask();
    fBuilder.disableExecutionMaskWrites();

    return true;
}

bool Generator::writeMasklessForStatement(const ForStatement& f) {
    SkASSERT(f.unrollInfo());
    SkASSERT(f.unrollInfo()->fCount > 0);
    SkASSERT(f.initializer());
    SkASSERT(f.test());
    SkASSERT(f.next());

    // If no lanes are active, skip over the loop entirely. This guards against looping forever;
    // with no lanes active, we wouldn't be able to write the loop variable back to its slot, so
    // we'd never make forward progress.
    int loopExitID = fBuilder.nextLabelID();
    int loopBodyID = fBuilder.nextLabelID();
    fBuilder.branch_if_no_active_lanes(loopExitID);

    // Run the loop initializer.
    if (!this->writeStatement(*f.initializer())) {
        return unsupported();
    }

    // Write the for-loop body. We know the for-loop has a standard ES2 unrollable structure, and
    // that it runs for at least one iteration, so we can plow straight ahead into the loop body
    // instead of running the loop-test first.
    fBuilder.label(loopBodyID);

    if (!this->writeStatement(*f.statement())) {
        return unsupported();
    }

    // If the loop only runs for a single iteration, we are already done. If not...
    if (f.unrollInfo()->fCount > 1) {
        // ... run the next-expression, and immediately discard its result.
        if (!this->pushExpression(*f.next(), /*usesResult=*/false)) {
            return unsupported();
        }
        this->discardExpression(f.next()->type().slotCount());

        // Run the test-expression, and repeat the loop until the test-expression evaluates false.
        if (!this->pushExpression(*f.test())) {
            return unsupported();
        }
        fBuilder.branch_if_no_active_lanes_on_stack_top_equal(0, loopBodyID);

        // Jettison the test-expression.
        this->discardExpression(/*slots=*/1);
    }

    fBuilder.label(loopExitID);
    return true;
}

bool Generator::writeForStatement(const ForStatement& f) {
    // If we've determined that the loop does not run, omit its code entirely.
    if (f.unrollInfo() && f.unrollInfo()->fCount == 0) {
        return true;
    }

    // If the loop doesn't escape early due to a `continue`, `break` or `return`, and the loop
    // conforms to ES2 structure, we know that we will run the full number of iterations across all
    // lanes and don't need to use a loop mask.
    Analysis::LoopControlFlowInfo loopInfo = Analysis::GetLoopControlFlowInfo(*f.statement());
    if (!loopInfo.fHasContinue && !loopInfo.fHasBreak && !loopInfo.fHasReturn && f.unrollInfo()) {
        return this->writeMasklessForStatement(f);
    }

    // Run the loop initializer.
    if (f.initializer() && !this->writeStatement(*f.initializer())) {
        return unsupported();
    }

    AutoContinueMask autoContinueMask;
    if (loopInfo.fHasContinue) {
        // Acquire a temporary slot for continue-mask storage.
        autoContinueMask.enable(&fProgramSlots, *fProgram.fContext->fTypes.fUInt,
                                &fCurrentContinueMask);
    }

    // Save off the original loop mask.
    fBuilder.enableExecutionMaskWrites();
    fBuilder.push_loop_mask();

    int loopTestID = fBuilder.nextLabelID();
    int loopBodyID = fBuilder.nextLabelID();

    // Jump down to the loop test so we can fall out of the loop immediately if it's zero-iteration.
    fBuilder.jump(loopTestID);

    // Write the for-loop body.
    fBuilder.label(loopBodyID);

    autoContinueMask.enterLoopBody(fBuilder);

    if (!this->writeStatement(*f.statement())) {
        return unsupported();
    }

    autoContinueMask.exitLoopBody(fBuilder);

    // Run the next-expression. Immediately discard its result.
    if (f.next()) {
        if (!this->pushExpression(*f.next(), /*usesResult=*/false)) {
            return unsupported();
        }
        this->discardExpression(f.next()->type().slotCount());
    }

    fBuilder.label(loopTestID);
    if (f.test()) {
        // Emit the test-expression, in order to combine it with the loop mask.
        if (!this->pushExpression(*f.test())) {
            return unsupported();
        }
        // Mask off any lanes in the loop mask where the test-expression is false; this breaks the
        // loop. We don't use the test expression for anything else, so jettison it.
        fBuilder.merge_loop_mask();
        this->discardExpression(/*slots=*/1);
    }

    // If any lanes are still running, go back to the top and run the loop body again.
    fBuilder.branch_if_any_active_lanes(loopBodyID);

    // Restore the loop mask.
    fBuilder.pop_loop_mask();
    fBuilder.disableExecutionMaskWrites();

    return true;
}

bool Generator::writeExpressionStatement(const ExpressionStatement& e) {
    if (!this->pushExpression(*e.expression(), /*usesResult=*/false)) {
        return unsupported();
    }
    this->discardExpression(e.expression()->type().slotCount());
    return true;
}

bool Generator::writeDynamicallyUniformIfStatement(const IfStatement& i) {
    SkASSERT(Analysis::IsDynamicallyUniformExpression(*i.test()));

    int falseLabelID = fBuilder.nextLabelID();
    int exitLabelID = fBuilder.nextLabelID();

    if (!this->pushExpression(*i.test())) {
        return unsupported();
    }

    fBuilder.branch_if_no_active_lanes_on_stack_top_equal(~0, falseLabelID);

    if (!this->writeStatement(*i.ifTrue())) {
        return unsupported();
    }

    if (!i.ifFalse()) {
        // We don't have an if-false condition at all.
        fBuilder.label(falseLabelID);
    } else {
        // We do have an if-false condition. We've just completed the if-true block, so we need to
        // jump past the if-false block to avoid executing it.
        fBuilder.jump(exitLabelID);

        // The if-false block starts here.
        fBuilder.label(falseLabelID);

        if (!this->writeStatement(*i.ifFalse())) {
            return unsupported();
        }

        fBuilder.label(exitLabelID);
    }

    // Jettison the test-expression.
    this->discardExpression(/*slots=*/1);
    return true;
}

bool Generator::writeIfStatement(const IfStatement& i) {
    // If the test condition is known to be uniform, we can skip over the untrue portion entirely.
    if (Analysis::IsDynamicallyUniformExpression(*i.test())) {
        return this->writeDynamicallyUniformIfStatement(i);
    }

    // Save the current condition-mask.
    fBuilder.enableExecutionMaskWrites();
    fBuilder.push_condition_mask();

    // Push the test condition mask.
    if (!this->pushExpression(*i.test())) {
        return unsupported();
    }

    // Merge the current condition-mask with the test condition, then run the if-true branch.
    fBuilder.merge_condition_mask();
    if (!this->writeStatement(*i.ifTrue())) {
        return unsupported();
    }

    if (i.ifFalse()) {
        // Negate the test-condition, then reapply it to the condition-mask.
        // Then, run the if-false branch.
        fBuilder.unary_op(BuilderOp::bitwise_not_int, /*slots=*/1);
        fBuilder.merge_condition_mask();
        if (!this->writeStatement(*i.ifFalse())) {
            return unsupported();
        }
    }

    // Jettison the test-expression, and restore the the condition-mask.
    this->discardExpression(/*slots=*/1);
    fBuilder.pop_condition_mask();
    fBuilder.disableExecutionMaskWrites();

    return true;
}

bool Generator::writeReturnStatement(const ReturnStatement& r) {
    if (r.expression()) {
        if (!this->pushExpression(*r.expression())) {
            return unsupported();
        }
        this->popToSlotRange(fCurrentFunctionResult);
    }
    if (fBuilder.executionMaskWritesAreEnabled() && this->needsReturnMask()) {
        fBuilder.mask_off_return_mask();
    }
    return true;
}

bool Generator::writeSwitchStatement(const SwitchStatement& s) {
    const StatementArray& cases = s.cases();
    SkASSERT(std::all_of(cases.begin(), cases.end(), [](const std::unique_ptr<Statement>& stmt) {
        return stmt->is<SwitchCase>();
    }));

    // Save off the original loop mask.
    fBuilder.enableExecutionMaskWrites();
    fBuilder.push_loop_mask();

    // Push the switch-case value, and write a default-mask that enables every lane which already
    // has an active loop mask. As we match cases, the default mask will get pared down.
    if (!this->pushExpression(*s.value())) {
        return unsupported();
    }
    fBuilder.push_loop_mask();

    // Zero out the loop mask; each case op will re-enable it as we go.
    fBuilder.mask_off_loop_mask();

    // Write each switch-case.
    bool foundDefaultCase = false;
    for (const std::unique_ptr<Statement>& stmt : cases) {
        int skipLabelID = fBuilder.nextLabelID();

        const SwitchCase& sc = stmt->as<SwitchCase>();
        if (sc.isDefault()) {
            foundDefaultCase = true;
            if (stmt.get() != cases.back().get()) {
                // We only support a default case when it is the very last case. If that changes,
                // this logic will need to be updated.
                return unsupported();
            }
            // Keep whatever lanes are executing now, and also enable any lanes in the default mask.
            fBuilder.pop_and_reenable_loop_mask();
            // Execute the switch-case block, if any lanes are alive to see it.
            fBuilder.branch_if_no_active_lanes(skipLabelID);
            if (!this->writeStatement(*sc.statement())) {
                return unsupported();
            }
        } else {
            // The case-op will enable the loop mask if the switch-value matches, and mask off lanes
            // from the default-mask.
            fBuilder.case_op(sc.value());
            // Execute the switch-case block, if any lanes are alive to see it.
            fBuilder.branch_if_no_active_lanes(skipLabelID);
            if (!this->writeStatement(*sc.statement())) {
                return unsupported();
            }
        }
        fBuilder.label(skipLabelID);
    }

    // Jettison the switch value, and the default case mask if it was never consumed above.
    this->discardExpression(/*slots=*/foundDefaultCase ? 1 : 2);

    // Restore the loop mask.
    fBuilder.pop_loop_mask();
    fBuilder.disableExecutionMaskWrites();
    return true;
}

bool Generator::writeVarDeclaration(const VarDeclaration& v) {
    if (v.value()) {
        if (!this->pushExpression(*v.value())) {
            return unsupported();
        }
        this->popToSlotRangeUnmasked(this->getVariableSlots(*v.var()));
    } else {
        this->zeroSlotRangeUnmasked(this->getVariableSlots(*v.var()));
    }
    return true;
}

bool Generator::pushExpression(const Expression& e, bool usesResult) {
    switch (e.kind()) {
        case Expression::Kind::kBinary:
            return this->pushBinaryExpression(e.as<BinaryExpression>());

        case Expression::Kind::kChildCall:
            return this->pushChildCall(e.as<ChildCall>());

        case Expression::Kind::kConstructorCompound:
        case Expression::Kind::kConstructorStruct:
            return this->pushConstructorCompound(e.asAnyConstructor());

        case Expression::Kind::kConstructorCompoundCast:
        case Expression::Kind::kConstructorScalarCast:
            return this->pushConstructorCast(e.asAnyConstructor());

        case Expression::Kind::kConstructorDiagonalMatrix:
            return this->pushConstructorDiagonalMatrix(e.as<ConstructorDiagonalMatrix>());

        case Expression::Kind::kConstructorMatrixResize:
            return this->pushConstructorMatrixResize(e.as<ConstructorMatrixResize>());

        case Expression::Kind::kConstructorSplat:
            return this->pushConstructorSplat(e.as<ConstructorSplat>());

        case Expression::Kind::kFieldAccess:
            return this->pushFieldAccess(e.as<FieldAccess>());

        case Expression::Kind::kFunctionCall:
            return this->pushFunctionCall(e.as<FunctionCall>());

        case Expression::Kind::kIndex:
            return this->pushIndexExpression(e.as<IndexExpression>());

        case Expression::Kind::kLiteral:
            return this->pushLiteral(e.as<Literal>());

        case Expression::Kind::kPrefix:
            return this->pushPrefixExpression(e.as<PrefixExpression>());

        case Expression::Kind::kPostfix:
            return this->pushPostfixExpression(e.as<PostfixExpression>(), usesResult);

        case Expression::Kind::kSwizzle:
            return this->pushSwizzle(e.as<Swizzle>());

        case Expression::Kind::kTernary:
            return this->pushTernaryExpression(e.as<TernaryExpression>());

        case Expression::Kind::kVariableReference:
            return this->pushVariableReference(e.as<VariableReference>());

        default:
            return unsupported();
    }
}

BuilderOp Generator::GetTypedOp(const SkSL::Type& type, const TypedOps& ops) {
    switch (type.componentType().numberKind()) {
        case Type::NumberKind::kFloat:    return ops.fFloatOp;
        case Type::NumberKind::kSigned:   return ops.fSignedOp;
        case Type::NumberKind::kUnsigned: return ops.fUnsignedOp;
        case Type::NumberKind::kBoolean:  return ops.fBooleanOp;
        default:                          return BuilderOp::unsupported;
    }
}

bool Generator::unaryOp(const SkSL::Type& type, const TypedOps& ops) {
    BuilderOp op = GetTypedOp(type, ops);
    if (op == BuilderOp::unsupported) {
        return unsupported();
    }
    fBuilder.unary_op(op, type.slotCount());
    return true;
}

bool Generator::binaryOp(const SkSL::Type& type, const TypedOps& ops) {
    BuilderOp op = GetTypedOp(type, ops);
    if (op == BuilderOp::unsupported) {
        return unsupported();
    }
    fBuilder.binary_op(op, type.slotCount());
    return true;
}

bool Generator::ternaryOp(const SkSL::Type& type, const TypedOps& ops) {
    BuilderOp op = GetTypedOp(type, ops);
    if (op == BuilderOp::unsupported) {
        return unsupported();
    }
    fBuilder.ternary_op(op, type.slotCount());
    return true;
}

void Generator::foldWithMultiOp(BuilderOp op, int elements) {
    // Fold the top N elements on the stack using an op that supports multiple slots, e.g.:
    // (A + B + C + D) -> add_2_floats $0..1 += $2..3
    //                    add_float    $0    += $1
    for (; elements >= 8; elements -= 4) {
        fBuilder.binary_op(op, /*slots=*/4);
    }
    for (; elements >= 6; elements -= 3) {
        fBuilder.binary_op(op, /*slots=*/3);
    }
    for (; elements >= 4; elements -= 2) {
        fBuilder.binary_op(op, /*slots=*/2);
    }
    for (; elements >= 2; elements -= 1) {
        fBuilder.binary_op(op, /*slots=*/1);
    }
}

bool Generator::pushLValueOrExpression(LValue* lvalue, const Expression& expr) {
    return lvalue ? this->push(*lvalue)
                  : this->pushExpression(expr);
}

bool Generator::pushMatrixMultiply(LValue* lvalue,
                                   const Expression& left,
                                   const Expression& right,
                                   int leftColumns,
                                   int leftRows,
                                   int rightColumns,
                                   int rightRows) {
    SkASSERT(left.type().isMatrix() || left.type().isVector());
    SkASSERT(right.type().isMatrix() || right.type().isVector());

    SkASSERT(leftColumns == rightRows);
    int outColumns   = rightColumns,
        outRows      = leftRows;

    // Push the left matrix onto the adjacent-neighbor stack. We transpose it so that we can copy
    // rows from it in a single op, instead of gathering one element at a time.
    AutoStack matrixStack(this);
    matrixStack.enter();
    if (!this->pushLValueOrExpression(lvalue, left)) {
        return unsupported();
    }
    fBuilder.transpose(leftColumns, leftRows);

    // Push the right matrix as well, then go back to the primary stack.
    if (!this->pushExpression(right)) {
        return unsupported();
    }
    matrixStack.exit();

    // Calculate the offsets of the left- and right-matrix, relative to the stack-top.
    int leftMtxBase  = left.type().slotCount() + right.type().slotCount() - leftColumns;
    int rightMtxBase = right.type().slotCount() - leftColumns;

    // Emit each matrix element.
    for (int c = 0; c < outColumns; ++c) {
        for (int r = 0; r < outRows; ++r) {
            // Dot a vector from left[*][r] with right[c][*].
            // (Because the left=matrix has been transposed, we actually pull left[r][*], which
            // allows us to clone a column at once instead of cloning each slot individually.)
            matrixStack.pushClone(leftColumns, leftMtxBase  - r * leftColumns);
            matrixStack.pushClone(leftColumns, rightMtxBase - c * leftColumns);
            fBuilder.dot_floats(leftColumns);
        }
    }

    // Dispose of the source matrices on the adjacent-neighbor stack.
    matrixStack.enter();
    this->discardExpression(left.type().slotCount());
    this->discardExpression(right.type().slotCount());
    matrixStack.exit();

    // If this multiply was actually an assignment (via *=), write the result back to the lvalue.
    return lvalue ? this->store(*lvalue)
                  : true;
}

void Generator::foldComparisonOp(Operator op, int elements) {
    switch (op.kind()) {
        case OperatorKind::EQEQ:
            // equal(x,y) returns a vector; use & to fold into a scalar.
            this->foldWithMultiOp(BuilderOp::bitwise_and_n_ints, elements);
            break;

        case OperatorKind::NEQ:
            // notEqual(x,y) returns a vector; use | to fold into a scalar.
            this->foldWithMultiOp(BuilderOp::bitwise_or_n_ints, elements);
            break;

        default:
            SkDEBUGFAIL("comparison only allows == and !=");
            break;
    }
}

bool Generator::pushStructuredComparison(LValue* left,
                                         Operator op,
                                         LValue* right,
                                         const Type& type) {
    if (type.isStruct()) {
        // Compare every field in the struct.
        SkSpan<const Type::Field> fields = type.fields();
        int currentSlot = 0;
        for (size_t index = 0; index < fields.size(); ++index) {
            const Type& fieldType = *fields[index].fType;
            const int   fieldSlotCount = fieldType.slotCount();
            UnownedLValueSlice fieldLeft {left,  currentSlot, fieldSlotCount};
            UnownedLValueSlice fieldRight{right, currentSlot, fieldSlotCount};
            if (!this->pushStructuredComparison(&fieldLeft, op, &fieldRight, fieldType)) {
                return unsupported();
            }
            currentSlot += fieldSlotCount;
        }

        this->foldComparisonOp(op, fields.size());
        return true;
    }

    if (type.isArray()) {
        const Type& indexedType = type.componentType();
        if (indexedType.numberKind() == Type::NumberKind::kNonnumeric) {
            // Compare every element in the array.
            const int indexedSlotCount = indexedType.slotCount();
            int       currentSlot = 0;
            for (int index = 0; index < type.columns(); ++index) {
                UnownedLValueSlice indexedLeft {left,  currentSlot, indexedSlotCount};
                UnownedLValueSlice indexedRight{right, currentSlot, indexedSlotCount};
                if (!this->pushStructuredComparison(&indexedLeft, op, &indexedRight, indexedType)) {
                    return unsupported();
                }
                currentSlot += indexedSlotCount;
            }

            this->foldComparisonOp(op, type.columns());
            return true;
        }
    }

    // We've winnowed down to a single element, or an array of homogeneous numeric elements.
    // Push the elements onto the stack, then compare them.
    if (!this->push(*left) || !this->push(*right)) {
        return unsupported();
    }
    switch (op.kind()) {
        case OperatorKind::EQEQ:
            if (!this->binaryOp(type, kEqualOps)) {
                return unsupported();
            }
            break;

        case OperatorKind::NEQ:
            if (!this->binaryOp(type, kNotEqualOps)) {
                return unsupported();
            }
            break;

        default:
            SkDEBUGFAIL("comparison only allows == and !=");
            break;
    }

    this->foldComparisonOp(op, type.slotCount());
    return true;
}

bool Generator::pushBinaryExpression(const BinaryExpression& e) {
    return this->pushBinaryExpression(*e.left(), e.getOperator(), *e.right());
}

bool Generator::pushBinaryExpression(const Expression& left, Operator op, const Expression& right) {
    switch (op.kind()) {
        // Rewrite greater-than ops as their less-than equivalents.
        case OperatorKind::GT:
            return this->pushBinaryExpression(right, OperatorKind::LT, left);

        case OperatorKind::GTEQ:
            return this->pushBinaryExpression(right, OperatorKind::LTEQ, left);

        // Handle struct and array comparisons.
        case OperatorKind::EQEQ:
        case OperatorKind::NEQ:
            if (left.type().isStruct() || left.type().isArray()) {
                SkASSERT(left.type().matches(right.type()));
                std::unique_ptr<LValue> lvLeft = this->makeLValue(left, /*allowScratch=*/true);
                std::unique_ptr<LValue> lvRight = this->makeLValue(right, /*allowScratch=*/true);
                return this->pushStructuredComparison(lvLeft.get(), op, lvRight.get(), left.type());
            }
            break;

        // Emit comma expressions.
        case OperatorKind::COMMA:
            if (Analysis::HasSideEffects(left)) {
                if (!this->pushExpression(left, /*usesResult=*/false)) {
                    return unsupported();
                }
                this->discardExpression(left.type().slotCount());
            }
            return this->pushExpression(right);

        default:
            break;
    }

    // Handle binary expressions with mismatched types.
    bool vectorizeLeft = false, vectorizeRight = false;
    if (!left.type().matches(right.type())) {
        if (left.type().componentType().numberKind() != right.type().componentType().numberKind()) {
            return unsupported();
        }
        if (left.type().isScalar() && (right.type().isVector() || right.type().isMatrix())) {
            vectorizeLeft = true;
        } else if ((left.type().isVector() || left.type().isMatrix()) && right.type().isScalar()) {
            vectorizeRight = true;
        }
    }

    const Type& type = vectorizeLeft ? right.type() : left.type();

    // If this is an assignment...
    std::unique_ptr<LValue> lvalue;
    if (op.isAssignment()) {
        // ... turn the left side into an lvalue.
        lvalue = this->makeLValue(left);
        if (!lvalue) {
            return unsupported();
        }

        // Handle simple assignment (`var = expr`).
        if (op.kind() == OperatorKind::EQ) {
            return this->pushExpression(right) &&
                   this->store(*lvalue);
        }

        // Strip off the assignment from the op (turning += into +).
        op = op.removeAssignment();
    }

    // Handle matrix multiplication (MxM/MxV/VxM).
    if (op.kind() == OperatorKind::STAR) {
        // Matrix * matrix:
        if (left.type().isMatrix() && right.type().isMatrix()) {
            return this->pushMatrixMultiply(lvalue.get(), left, right,
                                            left.type().columns(), left.type().rows(),
                                            right.type().columns(), right.type().rows());
        }

        // Vector * matrix:
        if (left.type().isVector() && right.type().isMatrix()) {
            return this->pushMatrixMultiply(lvalue.get(), left, right,
                                            left.type().columns(), 1,
                                            right.type().columns(), right.type().rows());
        }

        // Matrix * vector:
        if (left.type().isMatrix() && right.type().isVector()) {
            return this->pushMatrixMultiply(lvalue.get(), left, right,
                                            left.type().columns(), left.type().rows(),
                                            1, right.type().columns());
        }
    }

    if (!vectorizeLeft && !vectorizeRight && !type.matches(right.type())) {
        // We have mismatched types but don't know how to handle them.
        return unsupported();
    }

    // Handle binary ops which require short-circuiting.
    switch (op.kind()) {
        case OperatorKind::LOGICALAND:
            if (Analysis::HasSideEffects(right)) {
                // If the RHS has side effects, we rewrite `a && b` as `a ? b : false`. This
                // generates pretty solid code and gives us the required short-circuit behavior.
                SkASSERT(!op.isAssignment());
                SkASSERT(type.componentType().isBoolean());
                SkASSERT(type.slotCount() == 1);  // operator&& only works with scalar types
                Literal falseLiteral{Position{}, 0.0, &right.type()};
                return this->pushTernaryExpression(left, right, falseLiteral);
            }
            break;

        case OperatorKind::LOGICALOR:
            if (Analysis::HasSideEffects(right)) {
                // If the RHS has side effects, we rewrite `a || b` as `a ? true : b`.
                SkASSERT(!op.isAssignment());
                SkASSERT(type.componentType().isBoolean());
                SkASSERT(type.slotCount() == 1);  // operator|| only works with scalar types
                Literal trueLiteral{Position{}, 1.0, &right.type()};
                return this->pushTernaryExpression(left, trueLiteral, right);
            }
            break;

        default:
            break;
    }

    // Push the left- and right-expressions onto the stack.
    if (!this->pushLValueOrExpression(lvalue.get(), left)) {
        return unsupported();
    }
    if (vectorizeLeft) {
        fBuilder.push_duplicates(right.type().slotCount() - 1);
    }
    if (!this->pushExpression(right)) {
        return unsupported();
    }
    if (vectorizeRight) {
        fBuilder.push_duplicates(left.type().slotCount() - 1);
    }

    switch (op.kind()) {
        case OperatorKind::PLUS:
            if (!this->binaryOp(type, kAddOps)) {
                return unsupported();
            }
            break;

        case OperatorKind::MINUS:
            if (!this->binaryOp(type, kSubtractOps)) {
                return unsupported();
            }
            break;

        case OperatorKind::STAR:
            if (!this->binaryOp(type, kMultiplyOps)) {
                return unsupported();
            }
            break;

        case OperatorKind::SLASH:
            if (!this->binaryOp(type, kDivideOps)) {
                return unsupported();
            }
            break;

        case OperatorKind::LT:
        case OperatorKind::GT:
            if (!this->binaryOp(type, kLessThanOps)) {
                return unsupported();
            }
            SkASSERT(type.slotCount() == 1);  // operator< only works with scalar types
            break;

        case OperatorKind::LTEQ:
        case OperatorKind::GTEQ:
            if (!this->binaryOp(type, kLessThanEqualOps)) {
                return unsupported();
            }
            SkASSERT(type.slotCount() == 1);  // operator<= only works with scalar types
            break;

        case OperatorKind::EQEQ:
            if (!this->binaryOp(type, kEqualOps)) {
                return unsupported();
            }
            this->foldComparisonOp(op, type.slotCount());
            break;

        case OperatorKind::NEQ:
            if (!this->binaryOp(type, kNotEqualOps)) {
                return unsupported();
            }
            this->foldComparisonOp(op, type.slotCount());
            break;

        case OperatorKind::LOGICALAND:
        case OperatorKind::BITWISEAND:
            // For logical-and, we verified above that the RHS does not have side effects, so we
            // don't need to worry about short-circuiting side effects.
            fBuilder.binary_op(BuilderOp::bitwise_and_n_ints, type.slotCount());
            break;

        case OperatorKind::LOGICALOR:
        case OperatorKind::BITWISEOR:
            // For logical-or, we verified above that the RHS does not have side effects.
            fBuilder.binary_op(BuilderOp::bitwise_or_n_ints, type.slotCount());
            break;

        case OperatorKind::LOGICALXOR:
        case OperatorKind::BITWISEXOR:
            // Logical-xor does not short circuit.
            fBuilder.binary_op(BuilderOp::bitwise_xor_n_ints, type.slotCount());
            break;

        default:
            return unsupported();
    }

    // If we have an lvalue, we need to write the result back into it.
    return lvalue ? this->store(*lvalue)
                  : true;
}

bool Generator::pushConstructorCompound(const AnyConstructor& c) {
    for (const std::unique_ptr<Expression> &arg : c.argumentSpan()) {
        if (!this->pushExpression(*arg)) {
            return unsupported();
        }
    }
    return true;
}

bool Generator::pushChildCall(const ChildCall& c) {
    int* childIdx = fChildEffectMap.find(&c.child());
    SkASSERT(childIdx != nullptr);
    SkASSERT(!c.arguments().empty());

    // Save the dst.rgba fields; these hold our execution masks, and could potentially be
    // clobbered by the child effect.
    fBuilder.push_dst_rgba();

    // All child calls have at least one argument.
    const Expression* arg = c.arguments()[0].get();
    if (!this->pushExpression(*arg)) {
        return unsupported();
    }

    // Copy arguments from the stack into src/dst as required by this particular child-call.
    switch (c.child().type().typeKind()) {
        case Type::TypeKind::kShader: {
            // The argument must be a float2.
            SkASSERT(c.arguments().size() == 1);
            SkASSERT(arg->type().matches(*fProgram.fContext->fTypes.fFloat2));
            fBuilder.pop_src_rg();
            fBuilder.invoke_shader(*childIdx);
            break;
        }
        case Type::TypeKind::kColorFilter: {
            // The argument must be a half4/float4.
            SkASSERT(c.arguments().size() == 1);
            SkASSERT(arg->type().matches(*fProgram.fContext->fTypes.fHalf4) ||
                     arg->type().matches(*fProgram.fContext->fTypes.fFloat4));
            fBuilder.pop_src_rgba();
            fBuilder.invoke_color_filter(*childIdx);
            break;
        }
        case Type::TypeKind::kBlender: {
            // The first argument must be a half4/float4.
            SkASSERT(c.arguments().size() == 2);
            SkASSERT(arg->type().matches(*fProgram.fContext->fTypes.fHalf4) ||
                     arg->type().matches(*fProgram.fContext->fTypes.fFloat4));

            // The second argument must also be a half4/float4.
            arg = c.arguments()[1].get();
            SkASSERT(arg->type().matches(*fProgram.fContext->fTypes.fHalf4) ||
                     arg->type().matches(*fProgram.fContext->fTypes.fFloat4));

            if (!this->pushExpression(*arg)) {
                return unsupported();
            }

            fBuilder.pop_dst_rgba();
            fBuilder.pop_src_rgba();
            fBuilder.invoke_blender(*childIdx);
            break;
        }
        default: {
            SkDEBUGFAILF("cannot sample from type '%s'", c.child().type().description().c_str());
        }
    }

    // Restore dst.rgba so our execution masks are back to normal.
    fBuilder.pop_dst_rgba();

    // The child call has returned the result color via src.rgba; push it onto the stack.
    fBuilder.push_src_rgba();
    return true;
}

bool Generator::pushConstructorCast(const AnyConstructor& c) {
    SkASSERT(c.argumentSpan().size() == 1);
    const Expression& inner = *c.argumentSpan().front();
    SkASSERT(inner.type().slotCount() == c.type().slotCount());

    if (!this->pushExpression(inner)) {
        return unsupported();
    }
    if (inner.type().componentType().numberKind() == c.type().componentType().numberKind()) {
        // Since we ignore type precision, this cast is effectively a no-op.
        return true;
    }
    if (inner.type().componentType().isSigned() && c.type().componentType().isUnsigned()) {
        // Treat uint(int) as a no-op.
        return true;
    }
    if (inner.type().componentType().isUnsigned() && c.type().componentType().isSigned()) {
        // Treat int(uint) as a no-op.
        return true;
    }

    if (c.type().componentType().isBoolean()) {
        // Converting int or float to boolean can be accomplished via `notEqual(x, 0)`.
        fBuilder.push_zeros(c.type().slotCount());
        return this->binaryOp(inner.type(), kNotEqualOps);
    }
    if (inner.type().componentType().isBoolean()) {
        // Converting boolean to int or float can be accomplished via bitwise-and.
        if (c.type().componentType().isFloat()) {
            fBuilder.push_literal_f(1.0f);
        } else if (c.type().componentType().isSigned() || c.type().componentType().isUnsigned()) {
            fBuilder.push_literal_i(1);
        } else {
            SkDEBUGFAILF("unexpected cast from bool to %s", c.type().description().c_str());
            return unsupported();
        }
        fBuilder.push_duplicates(c.type().slotCount() - 1);
        fBuilder.binary_op(BuilderOp::bitwise_and_n_ints, c.type().slotCount());
        return true;
    }
    // We have dedicated ops to cast between float and integer types.
    if (inner.type().componentType().isFloat()) {
        if (c.type().componentType().isSigned()) {
            fBuilder.unary_op(BuilderOp::cast_to_int_from_float, c.type().slotCount());
            return true;
        }
        if (c.type().componentType().isUnsigned()) {
            fBuilder.unary_op(BuilderOp::cast_to_uint_from_float, c.type().slotCount());
            return true;
        }
    } else if (c.type().componentType().isFloat()) {
        if (inner.type().componentType().isSigned()) {
            fBuilder.unary_op(BuilderOp::cast_to_float_from_int, c.type().slotCount());
            return true;
        }
        if (inner.type().componentType().isUnsigned()) {
            fBuilder.unary_op(BuilderOp::cast_to_float_from_uint, c.type().slotCount());
            return true;
        }
    }

    SkDEBUGFAILF("unexpected cast from %s to %s",
                 c.type().description().c_str(), inner.type().description().c_str());
    return unsupported();
}

bool Generator::pushConstructorDiagonalMatrix(const ConstructorDiagonalMatrix& c) {
    fBuilder.push_zeros(1);
    if (!this->pushExpression(*c.argument())) {
        return unsupported();
    }
    fBuilder.diagonal_matrix(c.type().columns(), c.type().rows());

    return true;
}

bool Generator::pushConstructorMatrixResize(const ConstructorMatrixResize& c) {
    if (!this->pushExpression(*c.argument())) {
        return unsupported();
    }
    fBuilder.matrix_resize(c.argument()->type().columns(),
                           c.argument()->type().rows(),
                           c.type().columns(),
                           c.type().rows());
    return true;
}

bool Generator::pushConstructorSplat(const ConstructorSplat& c) {
    if (!this->pushExpression(*c.argument())) {
        return unsupported();
    }
    fBuilder.push_duplicates(c.type().slotCount() - 1);
    return true;
}

bool Generator::pushFieldAccess(const FieldAccess& f) {
    // If possible, get direct field access via the lvalue.
    std::unique_ptr<LValue> lvalue = this->makeLValue(f, /*allowScratch=*/true);
    return lvalue && this->push(*lvalue);
}

bool Generator::pushFunctionCall(const FunctionCall& c) {
    if (c.function().isIntrinsic()) {
        return this->pushIntrinsic(c);
    }

    // Keep track of the current function.
    const FunctionDefinition* lastFunction = fCurrentFunction;
    fCurrentFunction = c.function().definition();

    // Skip over the function body entirely if there are no active lanes.
    // (If the function call was trivial, it would likely have been inlined in the frontend, so this
    // is likely to save a significant amount of work if the lanes are all dead.)
    int skipLabelID = fBuilder.nextLabelID();
    fBuilder.branch_if_no_active_lanes(skipLabelID);

    // Save off the return mask.
    if (this->needsReturnMask()) {
        fBuilder.enableExecutionMaskWrites();
        fBuilder.push_return_mask();
    }

    // Write all the arguments into their parameter's variable slots. Because we never allow
    // recursion, we don't need to worry about overwriting any existing values in those slots.
    // (In fact, we don't even need to apply the write mask.)
    SkTArray<std::unique_ptr<LValue>> lvalues;
    lvalues.resize(c.arguments().size());

    for (int index = 0; index < c.arguments().size(); ++index) {
        const Expression& arg = *c.arguments()[index];
        const Variable& param = *c.function().parameters()[index];

        // Use LValues for out-parameters and inout-parameters, so we can store back to them later.
        if (IsInoutParameter(param) || IsOutParameter(param)) {
            lvalues[index] = this->makeLValue(arg);
            if (!lvalues[index]) {
                return unsupported();
            }
            // There are no guarantees on the starting value of an out-parameter, so we only need to
            // store the lvalues associated with an inout parameter.
            if (IsInoutParameter(param)) {
                if (!this->push(*lvalues[index])) {
                    return unsupported();
                }
                this->popToSlotRangeUnmasked(this->getVariableSlots(param));
            }
        } else {
            // Copy input arguments into their respective parameter slots.
            if (!this->pushExpression(arg)) {
                return unsupported();
            }
            this->popToSlotRangeUnmasked(this->getVariableSlots(param));
        }
    }

    // Emit the function body.
    std::optional<SlotRange> r = this->writeFunction(c, *fCurrentFunction);
    if (!r.has_value()) {
        return unsupported();
    }

    // Restore the original return mask.
    if (this->needsReturnMask()) {
        fBuilder.pop_return_mask();
        fBuilder.disableExecutionMaskWrites();
    }

    // We've returned back to the last function.
    fCurrentFunction = lastFunction;

    // Copy out-parameters and inout-parameters back to their homes.
    for (int index = 0; index < c.arguments().size(); ++index) {
        if (lvalues[index]) {
            // Only out- and inout-parameters should have an associated lvalue.
            const Variable& param = *c.function().parameters()[index];
            SkASSERT(IsInoutParameter(param) || IsOutParameter(param));

            // Copy the parameter's slots directly into the lvalue.
            fBuilder.push_slots(this->getVariableSlots(param));
            if (!this->store(*lvalues[index])) {
                return unsupported();
            }
            this->discardExpression(param.type().slotCount());
        }
    }

    // Copy the function result from its slots onto the stack.
    fBuilder.push_slots(*r);
    fBuilder.label(skipLabelID);
    return true;
}

bool Generator::pushIndexExpression(const IndexExpression& i) {
    std::unique_ptr<LValue> lvalue = this->makeLValue(i, /*allowScratch=*/true);
    return lvalue && this->push(*lvalue);
}

bool Generator::pushIntrinsic(const FunctionCall& c) {
    const ExpressionArray& args = c.arguments();
    switch (args.size()) {
        case 1:
            return this->pushIntrinsic(c.function().intrinsicKind(), *args[0]);

        case 2:
            return this->pushIntrinsic(c.function().intrinsicKind(), *args[0], *args[1]);

        case 3:
            return this->pushIntrinsic(c.function().intrinsicKind(), *args[0], *args[1], *args[2]);

        default:
            break;
    }

    return unsupported();
}

bool Generator::pushVectorizedExpression(const Expression& expr, const Type& vectorType) {
    if (!this->pushExpression(expr)) {
        return unsupported();
    }
    if (vectorType.slotCount() > expr.type().slotCount()) {
        SkASSERT(expr.type().slotCount() == 1);
        fBuilder.push_duplicates(vectorType.slotCount() - expr.type().slotCount());
    }
    return true;
}

bool Generator::pushIntrinsic(const TypedOps& ops, const Expression& arg0) {
    if (!this->pushExpression(arg0)) {
        return unsupported();
    }
    return this->unaryOp(arg0.type(), ops);
}

bool Generator::pushIntrinsic(BuilderOp builderOp, const Expression& arg0) {
    if (!this->pushExpression(arg0)) {
        return unsupported();
    }
    fBuilder.unary_op(builderOp, arg0.type().slotCount());
    return true;
}

bool Generator::pushIntrinsic(IntrinsicKind intrinsic, const Expression& arg0) {
    switch (intrinsic) {
        case IntrinsicKind::k_abs_IntrinsicKind:
            return this->pushIntrinsic(kAbsOps, arg0);

        case IntrinsicKind::k_any_IntrinsicKind:
            if (!this->pushExpression(arg0)) {
                return unsupported();
            }
            this->foldWithMultiOp(BuilderOp::bitwise_or_n_ints, arg0.type().slotCount());
            return true;

        case IntrinsicKind::k_all_IntrinsicKind:
            if (!this->pushExpression(arg0)) {
                return unsupported();
            }
            this->foldWithMultiOp(BuilderOp::bitwise_and_n_ints, arg0.type().slotCount());
            return true;

        case IntrinsicKind::k_atan_IntrinsicKind:
            return this->pushIntrinsic(BuilderOp::atan_float, arg0);

        case IntrinsicKind::k_ceil_IntrinsicKind:
            return this->pushIntrinsic(BuilderOp::ceil_float, arg0);

        case IntrinsicKind::k_cos_IntrinsicKind:
            return this->pushIntrinsic(BuilderOp::cos_float, arg0);

        case IntrinsicKind::k_degrees_IntrinsicKind: {
            Literal lit180OverPi{Position{}, 57.2957795131f, &arg0.type().componentType()};
            return this->pushBinaryExpression(arg0, OperatorKind::STAR, lit180OverPi);
        }
        case IntrinsicKind::k_floatBitsToInt_IntrinsicKind:
        case IntrinsicKind::k_floatBitsToUint_IntrinsicKind:
        case IntrinsicKind::k_intBitsToFloat_IntrinsicKind:
        case IntrinsicKind::k_uintBitsToFloat_IntrinsicKind:
            return this->pushExpression(arg0);

        case IntrinsicKind::k_exp_IntrinsicKind:
            return this->pushIntrinsic(BuilderOp::exp_float, arg0);

        case IntrinsicKind::k_floor_IntrinsicKind:
            return this->pushIntrinsic(BuilderOp::floor_float, arg0);

        case IntrinsicKind::k_fract_IntrinsicKind:
            // Implement fract as `x - floor(x)`.
            if (!this->pushExpression(arg0)) {
                return unsupported();
            }
            fBuilder.push_clone(arg0.type().slotCount());
            fBuilder.unary_op(BuilderOp::floor_float, arg0.type().slotCount());
            return this->binaryOp(arg0.type(), kSubtractOps);

        case IntrinsicKind::k_length_IntrinsicKind:
            if (!this->pushExpression(arg0)) {
                return unsupported();
            }
            // Implement length as `sqrt(dot(x, x))`.
            if (arg0.type().slotCount() > 1) {
                fBuilder.push_clone(arg0.type().slotCount());
                fBuilder.dot_floats(arg0.type().slotCount());
                fBuilder.unary_op(BuilderOp::sqrt_float, 1);
            } else {
                // The length of a scalar is `sqrt(x^2)`, which is equivalent to `abs(x)`.
                fBuilder.unary_op(BuilderOp::abs_float, 1);
            }
            return true;

        case IntrinsicKind::k_not_IntrinsicKind:
            return this->pushPrefixExpression(OperatorKind::LOGICALNOT, arg0);

        case IntrinsicKind::k_radians_IntrinsicKind: {
            Literal litPiOver180{Position{}, 0.01745329251f, &arg0.type().componentType()};
            return this->pushBinaryExpression(arg0, OperatorKind::STAR, litPiOver180);
        }
        case IntrinsicKind::k_saturate_IntrinsicKind: {
            // Implement saturate as clamp(arg, 0, 1).
            Literal zeroLiteral{Position{}, 0.0, &arg0.type().componentType()};
            Literal oneLiteral{Position{}, 1.0, &arg0.type().componentType()};
            return this->pushIntrinsic(k_clamp_IntrinsicKind, arg0, zeroLiteral, oneLiteral);
        }
        case IntrinsicKind::k_sign_IntrinsicKind: {
            // Implement floating-point sign() as `clamp(arg * FLT_MAX, -1, 1)`.
            // FLT_MIN * FLT_MAX evaluates to 4, so multiplying any float value against FLT_MAX is
            // sufficient to ensure that |value| is always 1 or greater (excluding zero and nan).
            // Integer sign() doesn't need to worry about fractional values or nans, and can simply
            // be `clamp(arg, -1, 1)`.
            if (!this->pushExpression(arg0)) {
                return unsupported();
            }
            if (arg0.type().componentType().isFloat()) {
                Literal fltMaxLiteral{Position{}, FLT_MAX, &arg0.type().componentType()};
                if (!this->pushVectorizedExpression(fltMaxLiteral, arg0.type())) {
                    return unsupported();
                }
                if (!this->binaryOp(arg0.type(), kMultiplyOps)) {
                    return unsupported();
                }
            }
            Literal neg1Literal{Position{}, -1.0, &arg0.type().componentType()};
            if (!this->pushVectorizedExpression(neg1Literal, arg0.type())) {
                return unsupported();
            }
            if (!this->binaryOp(arg0.type(), kMaxOps)) {
                return unsupported();
            }
            Literal pos1Literal{Position{}, 1.0, &arg0.type().componentType()};
            if (!this->pushVectorizedExpression(pos1Literal, arg0.type())) {
                return unsupported();
            }
            return this->binaryOp(arg0.type(), kMinOps);
        }
        case IntrinsicKind::k_sin_IntrinsicKind:
            return this->pushIntrinsic(BuilderOp::sin_float, arg0);

        case IntrinsicKind::k_sqrt_IntrinsicKind:
            return this->pushIntrinsic(BuilderOp::sqrt_float, arg0);

        case IntrinsicKind::k_tan_IntrinsicKind:
            return this->pushIntrinsic(BuilderOp::tan_float, arg0);

        case IntrinsicKind::k_transpose_IntrinsicKind:
            SkASSERT(arg0.type().isMatrix());
            if (!this->pushExpression(arg0)) {
                return unsupported();
            }
            fBuilder.transpose(arg0.type().columns(), arg0.type().rows());
            return true;

        default:
            break;
    }
    return unsupported();
}

bool Generator::pushIntrinsic(const TypedOps& ops, const Expression& arg0, const Expression& arg1) {
    if (!this->pushExpression(arg0) || !this->pushVectorizedExpression(arg1, arg0.type())) {
        return unsupported();
    }
    return this->binaryOp(arg0.type(), ops);
}

bool Generator::pushIntrinsic(BuilderOp builderOp, const Expression& arg0, const Expression& arg1) {
    if (!this->pushExpression(arg0) || !this->pushVectorizedExpression(arg1, arg0.type())) {
        return unsupported();
    }
    fBuilder.binary_op(builderOp, arg0.type().slotCount());
    return true;
}

bool Generator::pushIntrinsic(IntrinsicKind intrinsic,
                              const Expression& arg0,
                              const Expression& arg1) {
    switch (intrinsic) {
        case IntrinsicKind::k_atan_IntrinsicKind:
            return this->pushIntrinsic(BuilderOp::atan2_n_floats, arg0, arg1);

        case IntrinsicKind::k_cross_IntrinsicKind: {
            // Implement cross as `arg0.yzx * arg1.zxy - arg0.zxy * arg1.yzx`. We use two stacks so
            // that each subexpression can be multiplied separately.
            SkASSERT(arg0.type().matches(arg1.type()));
            SkASSERT(arg0.type().slotCount() == 3);
            SkASSERT(arg1.type().slotCount() == 3);

            // Push `arg0.yzx` onto this stack and `arg0.zxy` onto a separate subexpression stack.
            AutoStack subexpressionStack(this);
            subexpressionStack.enter();
            if (!this->pushExpression(arg0)) {
                return unsupported();
            }
            subexpressionStack.exit();
            subexpressionStack.pushClone(3);

            fBuilder.swizzle(/*consumedSlots=*/3, {1, 2, 0});
            subexpressionStack.enter();
            fBuilder.swizzle(/*consumedSlots=*/3, {2, 0, 1});
            subexpressionStack.exit();

            // Push `arg1.zxy` onto this stack and `arg1.yzx` onto the next stack. Perform the
            // multiply on each subexpression (`arg0.yzx * arg1.zxy` on the first stack, and
            // `arg0.zxy * arg1.yzx` on the next).
            subexpressionStack.enter();
            if (!this->pushExpression(arg1)) {
                return unsupported();
            }
            subexpressionStack.exit();
            subexpressionStack.pushClone(3);

            fBuilder.swizzle(/*consumedSlots=*/3, {2, 0, 1});
            fBuilder.binary_op(BuilderOp::mul_n_floats, 3);

            subexpressionStack.enter();
            fBuilder.swizzle(/*consumedSlots=*/3, {1, 2, 0});
            fBuilder.binary_op(BuilderOp::mul_n_floats, 3);
            subexpressionStack.exit();

            // Migrate the result of the second subexpression (`arg0.zxy * arg1.yzx`) back onto the
            // main stack and subtract it from the first subexpression (`arg0.yzx * arg1.zxy`).
            subexpressionStack.pushClone(3);
            fBuilder.binary_op(BuilderOp::sub_n_floats, 3);

            // Now that the calculation is complete, discard the subexpression on the next stack.
            subexpressionStack.enter();
            this->discardExpression(/*slots=*/3);
            subexpressionStack.exit();
            return true;
        }
        case IntrinsicKind::k_dot_IntrinsicKind:
            // Implement dot as `a*b`, followed by folding via addition.
            SkASSERT(arg0.type().matches(arg1.type()));
            if (!this->pushExpression(arg0) || !this->pushExpression(arg1)) {
                return unsupported();
            }
            fBuilder.dot_floats(arg0.type().slotCount());
            return true;

        case IntrinsicKind::k_equal_IntrinsicKind:
            SkASSERT(arg0.type().matches(arg1.type()));
            return this->pushIntrinsic(kEqualOps, arg0, arg1);

        case IntrinsicKind::k_notEqual_IntrinsicKind:
            SkASSERT(arg0.type().matches(arg1.type()));
            return this->pushIntrinsic(kNotEqualOps, arg0, arg1);

        case IntrinsicKind::k_lessThan_IntrinsicKind:
            SkASSERT(arg0.type().matches(arg1.type()));
            return this->pushIntrinsic(kLessThanOps, arg0, arg1);

        case IntrinsicKind::k_greaterThan_IntrinsicKind:
            SkASSERT(arg0.type().matches(arg1.type()));
            return this->pushIntrinsic(kLessThanOps, arg1, arg0);

        case IntrinsicKind::k_lessThanEqual_IntrinsicKind:
            SkASSERT(arg0.type().matches(arg1.type()));
            return this->pushIntrinsic(kLessThanEqualOps, arg0, arg1);

        case IntrinsicKind::k_greaterThanEqual_IntrinsicKind:
            SkASSERT(arg0.type().matches(arg1.type()));
            return this->pushIntrinsic(kLessThanEqualOps, arg1, arg0);

        case IntrinsicKind::k_min_IntrinsicKind:
            SkASSERT(arg0.type().componentType().matches(arg1.type().componentType()));
            return this->pushIntrinsic(kMinOps, arg0, arg1);

        case IntrinsicKind::k_matrixCompMult_IntrinsicKind:
            SkASSERT(arg0.type().matches(arg1.type()));
            return this->pushIntrinsic(kMultiplyOps, arg0, arg1);

        case IntrinsicKind::k_max_IntrinsicKind:
            SkASSERT(arg0.type().componentType().matches(arg1.type().componentType()));
            return this->pushIntrinsic(kMaxOps, arg0, arg1);

        case IntrinsicKind::k_pow_IntrinsicKind:
            SkASSERT(arg0.type().matches(arg1.type()));
            return this->pushIntrinsic(BuilderOp::pow_n_floats, arg0, arg1);

        case IntrinsicKind::k_step_IntrinsicKind: {
            // Compute step as `float(lessThan(edge, x))`. We convert from boolean 0/~0 to floating
            // point zero/one by using a bitwise-and against the bit-pattern of 1.0.
            SkASSERT(arg0.type().componentType().matches(arg1.type().componentType()));
            if (!this->pushVectorizedExpression(arg0, arg1.type()) || !this->pushExpression(arg1)) {
                return unsupported();
            }
            if (!this->binaryOp(arg1.type(), kLessThanOps)) {
                return unsupported();
            }
            Literal pos1Literal{Position{}, 1.0, &arg1.type().componentType()};
            if (!this->pushVectorizedExpression(pos1Literal, arg1.type())) {
                return unsupported();
            }
            fBuilder.binary_op(BuilderOp::bitwise_and_n_ints, arg1.type().slotCount());
            return true;
        }

        default:
            break;
    }
    return unsupported();
}

bool Generator::pushIntrinsic(IntrinsicKind intrinsic,
                              const Expression& arg0,
                              const Expression& arg1,
                              const Expression& arg2) {
    switch (intrinsic) {
        case IntrinsicKind::k_clamp_IntrinsicKind:
            // Implement clamp as min(max(arg, low), high).
            SkASSERT(arg0.type().componentType().matches(arg1.type().componentType()));
            SkASSERT(arg0.type().componentType().matches(arg2.type().componentType()));
            if (!this->pushExpression(arg0) || !this->pushVectorizedExpression(arg1, arg0.type())) {
                return unsupported();
            }
            if (!this->binaryOp(arg0.type(), kMaxOps)) {
                return unsupported();
            }
            if (!this->pushVectorizedExpression(arg2, arg0.type())) {
                return unsupported();
            }
            if (!this->binaryOp(arg0.type(), kMinOps)) {
                return unsupported();
            }
            return true;

        case IntrinsicKind::k_mix_IntrinsicKind:
            // Note: our SkRP mix op takes the interpolation point first, not the interpolants.
            SkASSERT(arg0.type().matches(arg1.type()));
            if (arg2.type().componentType().isFloat()) {
                SkASSERT(arg0.type().componentType().matches(arg2.type().componentType()));
                if (!this->pushVectorizedExpression(arg2, arg0.type())) {
                    return unsupported();
                }
                if (!this->pushExpression(arg0) || !this->pushExpression(arg1)) {
                    return unsupported();
                }
                return this->ternaryOp(arg0.type(), kMixOps);
            }
            if (arg2.type().componentType().isBoolean()) {
                if (!this->pushExpression(arg2)) {
                    return unsupported();
                }
                if (!this->pushExpression(arg0) || !this->pushExpression(arg1)) {
                    return unsupported();
                }
                // The `mix_int` op isn't doing a lerp; it uses the third argument to select values
                // from the first and second arguments. It's safe for use with any type in arguments
                // 0 and 1.
                fBuilder.ternary_op(BuilderOp::mix_n_ints, arg0.type().slotCount());
                return true;
            }
            return unsupported();

        default:
            break;
    }
    return unsupported();
}

bool Generator::pushLiteral(const Literal& l) {
    switch (l.type().numberKind()) {
        case Type::NumberKind::kFloat:
            fBuilder.push_literal_f(l.floatValue());
            return true;

        case Type::NumberKind::kSigned:
            fBuilder.push_literal_i(l.intValue());
            return true;

        case Type::NumberKind::kUnsigned:
            fBuilder.push_literal_u(l.intValue());
            return true;

        case Type::NumberKind::kBoolean:
            fBuilder.push_literal_i(l.boolValue() ? ~0 : 0);
            return true;

        default:
            SkUNREACHABLE;
    }
}

bool Generator::pushPostfixExpression(const PostfixExpression& p, bool usesResult) {
    // If the result is ignored...
    if (!usesResult) {
        // ... just emit a prefix expression instead.
        return this->pushPrefixExpression(p.getOperator(), *p.operand());
    }
    // Get the operand as an lvalue, and push it onto the stack as-is.
    std::unique_ptr<LValue> lvalue = this->makeLValue(*p.operand());
    if (!lvalue || !this->push(*lvalue)) {
        return unsupported();
    }

    // Push a scratch copy of the operand.
    fBuilder.push_clone(p.type().slotCount());

    // Increment or decrement the scratch copy by one.
    Literal oneLiteral{Position{}, 1.0, &p.type().componentType()};
    if (!this->pushVectorizedExpression(oneLiteral, p.type())) {
        return unsupported();
    }

    switch (p.getOperator().kind()) {
        case OperatorKind::PLUSPLUS:
            if (!this->binaryOp(p.type(), kAddOps)) {
                return unsupported();
            }
            break;

        case OperatorKind::MINUSMINUS:
            if (!this->binaryOp(p.type(), kSubtractOps)) {
                return unsupported();
            }
            break;

        default:
            SkUNREACHABLE;
    }

    // Write the new value back to the operand.
    if (!this->store(*lvalue)) {
        return unsupported();
    }

    // Discard the scratch copy, leaving only the original value as-is.
    this->discardExpression(p.type().slotCount());
    return true;
}

bool Generator::pushPrefixExpression(const PrefixExpression& p) {
    return this->pushPrefixExpression(p.getOperator(), *p.operand());
}

bool Generator::pushPrefixExpression(Operator op, const Expression& expr) {
    switch (op.kind()) {
        case OperatorKind::BITWISENOT:
        case OperatorKind::LOGICALNOT:
            // Handle operators ! and ~.
            if (!this->pushExpression(expr)) {
                return unsupported();
            }
            fBuilder.unary_op(BuilderOp::bitwise_not_int, expr.type().slotCount());
            return true;

        case OperatorKind::MINUS:
            // Handle negation as a componentwise `0 - expr`.
            fBuilder.push_zeros(expr.type().slotCount());
            if (!this->pushExpression(expr)) {
                return unsupported();
            }
            return this->binaryOp(expr.type(), kSubtractOps);

        case OperatorKind::PLUSPLUS: {
            // Rewrite as `expr += 1`.
            Literal oneLiteral{Position{}, 1.0, &expr.type().componentType()};
            return this->pushBinaryExpression(expr, OperatorKind::PLUSEQ, oneLiteral);
        }
        case OperatorKind::MINUSMINUS: {
            // Rewrite as `expr -= 1`.
            Literal oneLiteral{Position{}, 1.0, &expr.type().componentType()};
            return this->pushBinaryExpression(expr, OperatorKind::MINUSEQ, oneLiteral);
        }
        default:
            break;
    }

    return unsupported();
}

bool Generator::pushSwizzle(const Swizzle& s) {
    SkASSERT(!s.components().empty() && s.components().size() <= 4);

    // If this is a simple subset of a variable's slots...
    bool isSimpleSubset = is_sliceable_swizzle(s.components());
    if (isSimpleSubset && s.base()->is<VariableReference>()) {
        // ... we can just push part of the variable directly onto the stack, rather than pushing
        // the whole expression and then immediately cutting it down. (Either way works, but this
        // saves a step.)
        return this->pushVariableReferencePartial(
                s.base()->as<VariableReference>(),
                SlotRange{/*index=*/s.components()[0], /*count=*/s.components().size()});
    }
    // Push the base expression.
    if (!this->pushExpression(*s.base())) {
        return false;
    }
    // An identity swizzle doesn't rearrange the data; it just (potentially) discards tail elements.
    if (isSimpleSubset && s.components()[0] == 0) {
        int discardedElements = s.base()->type().slotCount() - s.components().size();
        SkASSERT(discardedElements >= 0);
        fBuilder.discard_stack(discardedElements);
        return true;
    }
    // Perform the swizzle.
    fBuilder.swizzle(s.base()->type().slotCount(), s.components());
    return true;
}

bool Generator::pushTernaryExpression(const TernaryExpression& t) {
    return this->pushTernaryExpression(*t.test(), *t.ifTrue(), *t.ifFalse());
}

bool Generator::pushDynamicallyUniformTernaryExpression(const Expression& test,
                                                        const Expression& ifTrue,
                                                        const Expression& ifFalse) {
    SkASSERT(Analysis::IsDynamicallyUniformExpression(test));

    int falseLabelID = fBuilder.nextLabelID();
    int exitLabelID = fBuilder.nextLabelID();

    // First, push the test-expression into a separate stack.
    AutoStack testStack(this);
    testStack.enter();
    if (!this->pushExpression(test)) {
        return unsupported();
    }

    // Branch to the true- or false-expression based on the test-expression. We can skip the
    // non-true path entirely since the test is known to be uniform.
    fBuilder.branch_if_no_active_lanes_on_stack_top_equal(~0, falseLabelID);
    testStack.exit();

    if (!this->pushExpression(ifTrue)) {
        return unsupported();
    }

    fBuilder.jump(exitLabelID);

    // The builder doesn't understand control flow, and assumes that every push moves the stack-top
    // forwards. We need to manually balance out the `pushExpression` from the if-true path by
    // moving the stack position backwards, so that the if-false path pushes its expression into the
    // same as the if-true result.
    this->discardExpression(/*slots=*/ifTrue.type().slotCount());

    fBuilder.label(falseLabelID);

    if (!this->pushExpression(ifFalse)) {
        return unsupported();
    }

    fBuilder.label(exitLabelID);

    // Jettison the text-expression from the separate stack.
    testStack.enter();
    this->discardExpression(/*slots=*/1);
    testStack.exit();
    return true;
}

bool Generator::pushTernaryExpression(const Expression& test,
                                      const Expression& ifTrue,
                                      const Expression& ifFalse) {
    // If the test-expression is dynamically-uniform, we can skip over the non-true expressions
    // entirely, and not need to involve the condition mask.
    if (Analysis::IsDynamicallyUniformExpression(test)) {
        return this->pushDynamicallyUniformTernaryExpression(test, ifTrue, ifFalse);
    }

    // Analyze the ternary to see which corners we can safely cut.
    bool ifFalseHasSideEffects = Analysis::HasSideEffects(ifFalse);
    bool ifTrueHasSideEffects  = Analysis::HasSideEffects(ifTrue);
    bool ifTrueIsTrivial       = Analysis::IsTrivialExpression(ifTrue);
    int  cleanupLabelID        = fBuilder.nextLabelID();

    // If the true- and false-expressions both lack side effects, we evaluate both of them safely
    // without masking off their effects. In that case, we can emit both sides and use boolean mix
    // to select the correct result without using the condition mask at all.
    if (!ifFalseHasSideEffects && !ifTrueHasSideEffects && ifTrueIsTrivial) {
        // Push all of the arguments to mix.
        if (!this->pushVectorizedExpression(test, ifTrue.type())) {
            return unsupported();
        }
        if (!this->pushExpression(ifFalse)) {
            return unsupported();
        }
        if (!this->pushExpression(ifTrue)) {
            return unsupported();
        }
        // Use boolean mix to select the true- or false-expression via the test-expression.
        fBuilder.ternary_op(BuilderOp::mix_n_ints, ifTrue.type().slotCount());
        return true;
    }

    // First, push the current condition-mask and the test-expression into a separate stack.
    fBuilder.enableExecutionMaskWrites();
    AutoStack testStack(this);
    testStack.enter();
    fBuilder.push_condition_mask();
    if (!this->pushExpression(test)) {
        return unsupported();
    }
    testStack.exit();

    // We can take some shortcuts with condition-mask handling if the false-expression is entirely
    // side-effect free. (We can evaluate it without masking off its effects.) We always handle the
    // condition mask properly for the test-expression and true-expression properly.
    if (!ifFalseHasSideEffects) {
        // Push the false-expression onto the primary stack.
        if (!this->pushExpression(ifFalse)) {
            return unsupported();
        }

        // Next, merge the condition mask (on the separate stack) with the test expression.
        testStack.enter();
        fBuilder.merge_condition_mask();
        testStack.exit();

        // If no lanes are active, we can skip the true-expression entirely. This isn't super likely
        // to happen, so it's probably only a win for non-trivial true-expressions.
        if (!ifTrueIsTrivial) {
            fBuilder.branch_if_no_active_lanes(cleanupLabelID);
        }

        // Push the true-expression onto the primary stack, immediately after the false-expression.
        if (!this->pushExpression(ifTrue)) {
            return unsupported();
        }

        // Use a select to conditionally mask-merge the true-expression and false-expression lanes.
        fBuilder.select(/*slots=*/ifTrue.type().slotCount());
        fBuilder.label(cleanupLabelID);
    } else {
        // Merge the condition mask (on the separate stack) with the test expression.
        testStack.enter();
        fBuilder.merge_condition_mask();
        testStack.exit();

        // Push the true-expression onto the primary stack.
        if (!this->pushExpression(ifTrue)) {
            return unsupported();
        }

        // Switch back to the test-expression stack temporarily, and negate the test condition.
        testStack.enter();
        fBuilder.unary_op(BuilderOp::bitwise_not_int, /*slots=*/1);
        fBuilder.merge_condition_mask();
        testStack.exit();

        // Push the false-expression onto the primary stack, immediately after the true-expression.
        if (!this->pushExpression(ifFalse)) {
            return unsupported();
        }

        // Use a select to conditionally mask-merge the true-expression and false-expression lanes;
        // the mask is already set up for this.
        fBuilder.select(/*slots=*/ifTrue.type().slotCount());
    }

    // Restore the condition-mask to its original state and jettison the test-expression.
    testStack.enter();
    this->discardExpression(/*slots=*/1);
    fBuilder.pop_condition_mask();
    testStack.exit();

    fBuilder.disableExecutionMaskWrites();
    return true;
}

bool Generator::pushVariableReference(const VariableReference& v) {
    return this->pushVariableReferencePartial(v, SlotRange{0, (int)v.type().slotCount()});
}

bool Generator::pushVariableReferencePartial(const VariableReference& v, SlotRange subset) {
    const Variable& var = *v.variable();
    SlotRange r;
    if (IsUniform(var)) {
        r = this->getUniformSlots(var);
        SkASSERT(r.count == (int)var.type().slotCount());
        r.index += subset.index;
        r.count = subset.count;
        fBuilder.push_uniform(r);
    } else {
        r = this->getVariableSlots(var);
        SkASSERT(r.count == (int)var.type().slotCount());
        r.index += subset.index;
        r.count = subset.count;
        fBuilder.push_slots(r);
    }
    return true;
}

bool Generator::writeProgram(const FunctionDefinition& function) {
    fCurrentFunction = &function;

    if (fDebugTrace) {
        // Copy the program source into the debug info so that it will be written in the trace file.
        fDebugTrace->setSource(*fProgram.fSource);
    }
    // Assign slots to the parameters of main; copy src and dst into those slots as appropriate.
    for (const SkSL::Variable* param : function.declaration().parameters()) {
        switch (param->modifiers().fLayout.fBuiltin) {
            case SK_MAIN_COORDS_BUILTIN: {
                // Coordinates are passed via RG.
                SlotRange fragCoord = this->getVariableSlots(*param);
                SkASSERT(fragCoord.count == 2);
                fBuilder.store_src_rg(fragCoord);
                break;
            }
            case SK_INPUT_COLOR_BUILTIN: {
                // Input colors are passed via RGBA.
                SlotRange srcColor = this->getVariableSlots(*param);
                SkASSERT(srcColor.count == 4);
                fBuilder.store_src(srcColor);
                break;
            }
            case SK_DEST_COLOR_BUILTIN: {
                // Dest colors are passed via dRGBA.
                SlotRange destColor = this->getVariableSlots(*param);
                SkASSERT(destColor.count == 4);
                fBuilder.store_dst(destColor);
                break;
            }
            default: {
                SkDEBUGFAIL("Invalid parameter to main()");
                return unsupported();
            }
        }
    }

    // Initialize the program.
    fBuilder.init_lane_masks();

    // Emit global variables.
    if (!this->writeGlobals()) {
        return unsupported();
    }

    // Invoke main().
    if (this->needsReturnMask()) {
        fBuilder.enableExecutionMaskWrites();
    }

    std::optional<SlotRange> mainResult = this->writeFunction(function, function);
    if (!mainResult.has_value()) {
        return unsupported();
    }

    if (this->needsReturnMask()) {
        fBuilder.disableExecutionMaskWrites();
    }

    // Move the result of main() from slots into RGBA. Allow dRGBA to remain in a trashed state.
    SkASSERT(mainResult->count == 4);
    fBuilder.load_src(*mainResult);
    return true;
}

std::unique_ptr<RP::Program> Generator::finish() {
    return fBuilder.finish(fProgramSlots.slotCount(), fUniformSlots.slotCount(), fDebugTrace);
}

}  // namespace RP

std::unique_ptr<RP::Program> MakeRasterPipelineProgram(const SkSL::Program& program,
                                                       const FunctionDefinition& function,
                                                       SkRPDebugTrace* debugTrace) {
    RP::Generator generator(program, debugTrace);
    if (!generator.writeProgram(function)) {
        return nullptr;
    }
    return generator.finish();
}

}  // namespace SkSL
