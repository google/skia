/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/codegen/SkSLRasterPipelineCodeGenerator.h"

#include "include/core/SkPoint.h"
#include "include/core/SkSpan.h"
#include "include/private/base/SkTArray.h"
#include "include/private/base/SkTo.h"
#include "src/base/SkEnumBitMask.h"
#include "src/base/SkStringView.h"
#include "src/base/SkUtils.h"
#include "src/core/SkTHash.h"
#include "src/sksl/SkSLAnalysis.h"
#include "src/sksl/SkSLBuiltinTypes.h"
#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLConstantFolder.h"
#include "src/sksl/SkSLContext.h"
#include "src/sksl/SkSLDefines.h"
#include "src/sksl/SkSLIntrinsicList.h"
#include "src/sksl/SkSLOperator.h"
#include "src/sksl/SkSLPosition.h"
#include "src/sksl/analysis/SkSLProgramUsage.h"
#include "src/sksl/codegen/SkSLRasterPipelineBuilder.h"
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
#include "src/sksl/ir/SkSLIRNode.h"
#include "src/sksl/ir/SkSLIfStatement.h"
#include "src/sksl/ir/SkSLIndexExpression.h"
#include "src/sksl/ir/SkSLLayout.h"
#include "src/sksl/ir/SkSLLiteral.h"
#include "src/sksl/ir/SkSLModifierFlags.h"
#include "src/sksl/ir/SkSLPostfixExpression.h"
#include "src/sksl/ir/SkSLPrefixExpression.h"
#include "src/sksl/ir/SkSLProgram.h"
#include "src/sksl/ir/SkSLProgramElement.h"
#include "src/sksl/ir/SkSLReturnStatement.h"
#include "src/sksl/ir/SkSLStatement.h"
#include "src/sksl/ir/SkSLSwitchCase.h"
#include "src/sksl/ir/SkSLSwitchStatement.h"
#include "src/sksl/ir/SkSLSwizzle.h"
#include "src/sksl/ir/SkSLTernaryExpression.h"
#include "src/sksl/ir/SkSLType.h"
#include "src/sksl/ir/SkSLVarDeclarations.h"
#include "src/sksl/ir/SkSLVariable.h"
#include "src/sksl/ir/SkSLVariableReference.h"
#include "src/sksl/tracing/SkSLDebugTracePriv.h"
#include "src/sksl/transform/SkSLTransform.h"

#include <algorithm>
#include <climits>
#include <cstddef>
#include <cstdint>
#include <float.h>
#include <iterator>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

using namespace skia_private;

namespace SkSL {
namespace RP {

static bool unsupported() {
    // If MakeRasterPipelineProgram returns false, set a breakpoint here for more information.
    return false;
}

class AutoContinueMask;
class Generator;
class LValue;

class SlotManager {
public:
    SlotManager(std::vector<SlotDebugInfo>* i) : fSlotDebugInfo(i) {}

    /** Used by `createSlots` to add this variable to SlotDebugInfo inside the DebugTrace. */
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
     * Associates previously-created slots with an SkSL variable; this can allow multiple variables
     * to share overlapping ranges. If the variable was already associated with a slot range,
     * returns the previously associated range.
     */
    std::optional<SlotRange> mapVariableToSlots(const Variable& v, SlotRange range);

    /**
     * Deletes the existing mapping between a variable and its slots; a future call to
     * `getVariableSlots` will see this as a brand new variable and associate new slots.
     */
    void unmapVariableSlots(const Variable& v);

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
    THashMap<const IRNode*, SlotRange> fSlotMap;
    int fSlotCount = 0;
    std::vector<SlotDebugInfo>* fSlotDebugInfo;
};

class AutoStack {
public:
    /**
     * Creates a temporary stack. The caller is responsible for discarding every entry on this
     * stack before ~AutoStack is reached.
     */
    explicit AutoStack(Generator* g);
    ~AutoStack();

    /** Activates the associated stack. */
    void enter();

    /** Undoes a call to `enter`, returning to the previously-active stack. */
    void exit();

    /** Returns the stack ID of this AutoStack. */
    int stackID() { return fStackID; }

    /** Clones values from this stack onto the top of the active stack. */
    void pushClone(int slots);

    /** Clones values from a fixed range of this stack onto the top of the active stack. */
    void pushClone(SlotRange range, int offsetFromStackTop);

    /** Clones values from a dynamic range of this stack onto the top of the active stack. */
    void pushCloneIndirect(SlotRange range, int dynamicStackID, int offsetFromStackTop);

private:
    Generator* fGenerator;
    int fStackID = 0;
    int fParentStackID = 0;
};

class Generator {
public:
    Generator(const SkSL::Program& program, DebugTracePriv* debugTrace, bool writeTraceOps)
            : fProgram(program)
            , fContext(fProgram.fContext->fTypes, *fProgram.fContext->fErrors)
            , fDebugTrace(debugTrace)
            , fWriteTraceOps(writeTraceOps)
            , fProgramSlots(debugTrace ? &debugTrace->fSlotInfo : nullptr)
            , fUniformSlots(debugTrace ? &debugTrace->fUniformInfo : nullptr)
            , fImmutableSlots(nullptr) {
        fContext.fConfig = fProgram.fConfig.get();
        fContext.fModule = fProgram.fContext->fModule;
    }

    ~Generator() {
        // ~AutoStack calls into the Generator, so we need to make sure the trace mask is reset
        // before the Generator is destroyed.
        fTraceMask.reset();
    }

    /** Converts the SkSL main() function into a set of Instructions. */
    bool writeProgram(const FunctionDefinition& function);

    /** Returns the generated program. */
    std::unique_ptr<RP::Program> finish();

    /**
     * Converts an SkSL function into a set of Instructions. Returns nullopt if the function
     * contained unsupported statements or expressions.
     */
    std::optional<SlotRange> writeFunction(const IRNode& callSite,
                                           const FunctionDefinition& function,
                                           SkSpan<std::unique_ptr<Expression> const> arguments);

    /**
     * Returns the slot index of this function inside the FunctionDebugInfo array in DebugTracePriv.
     * The FunctionDebugInfo slot will be created if it doesn't already exist.
     */
    int getFunctionDebugInfo(const FunctionDeclaration& decl);

    /** Returns true for variables with slots in fProgramSlots; immutables or uniforms are false. */
    bool hasVariableSlots(const Variable& v) {
        return !IsUniform(v) && !fImmutableVariables.contains(&v);
    }

    /** Looks up the slots associated with an SkSL variable; creates the slots if necessary. */
    SlotRange getVariableSlots(const Variable& v) {
        SkASSERT(this->hasVariableSlots(v));
        return fProgramSlots.getVariableSlots(v);
    }

    /**
     * Looks up the slots associated with an immutable variable; creates the slots if necessary.
     */
    SlotRange getImmutableSlots(const Variable& v) {
        SkASSERT(!IsUniform(v));
        SkASSERT(fImmutableVariables.contains(&v));
        return fImmutableSlots.getVariableSlots(v);
    }

    /** Looks up the slots associated with an SkSL uniform; creates the slots if necessary. */
    SlotRange getUniformSlots(const Variable& v) {
        SkASSERT(IsUniform(v));
        SkASSERT(!fImmutableVariables.contains(&v));
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
    [[nodiscard]] bool writeImmutableVarDeclaration(const VarDeclaration& d);

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

    /** Support methods for immutable data, which trade more slots for smaller code size. */
    using ImmutableBits = int32_t;

    [[nodiscard]] bool pushImmutableData(const Expression& e);
    [[nodiscard]] std::optional<SlotRange> findPreexistingImmutableData(
            const TArray<ImmutableBits>& immutableValues);
    [[nodiscard]] std::optional<ImmutableBits> getImmutableBitsForSlot(const Expression& expr,
                                                                       size_t slot);
    [[nodiscard]] bool getImmutableValueForExpression(const Expression& expr,
                                                      TArray<ImmutableBits>* immutableValues);
    void storeImmutableValueToSlots(const TArray<ImmutableBits>& immutableValues, SlotRange slots);

    /** Pops an expression from the value stack and copies it into slots. */
    void popToSlotRange(SlotRange r) {
        fBuilder.pop_slots(r);
        if (this->shouldWriteTraceOps()) {
            fBuilder.trace_var(fTraceMask->stackID(), r);
        }
    }
    void popToSlotRangeUnmasked(SlotRange r) {
        fBuilder.pop_slots_unmasked(r);
        if (this->shouldWriteTraceOps()) {
            fBuilder.trace_var(fTraceMask->stackID(), r);
        }
    }

    /** Pops an expression from the value stack and discards it. */
    void discardExpression(int slots) { fBuilder.discard_stack(slots); }

    /** Zeroes out a range of slots. */
    void zeroSlotRangeUnmasked(SlotRange r) {
        fBuilder.zero_slots_unmasked(r);
        if (this->shouldWriteTraceOps()) {
            fBuilder.trace_var(fTraceMask->stackID(), r);
        }
    }

    /**
     * Emits a trace_line opcode. writeStatement does this, and statements that alter control flow
     * may need to explicitly add additional traces.
     */
    void emitTraceLine(Position pos);

    /**
     * Emits a trace_scope opcode, which alters the SkSL variable-scope depth.
     * Unlike the other trace ops, trace_scope takes a dedicated mask instead of the trace-scope
     * mask. Call `pushTraceScopeMask` to synthesize this mask; discard it when you're done.
     */
    void pushTraceScopeMask();
    void discardTraceScopeMask();
    void emitTraceScope(int delta);

    /** Prepares our position-to-line-offset conversion table (stored in `fLineOffsets`). */
    void calculateLineOffsets();

    bool shouldWriteTraceOps() { return fDebugTrace && fWriteTraceOps; }
    int traceMaskStackID() { return fTraceMask->stackID(); }

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
    [[nodiscard]] bool pushAbsFloatIntrinsic(int slots);
    [[nodiscard]] bool pushLengthIntrinsic(int slotCount);
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

    Analysis::ReturnComplexity returnComplexity(const FunctionDefinition* func) {
        Analysis::ReturnComplexity* complexity = fReturnComplexityMap.find(func);
        if (!complexity) {
            complexity = fReturnComplexityMap.set(fCurrentFunction,
                                                  Analysis::GetReturnComplexity(*func));
        }
        return *complexity;
    }

    bool needsReturnMask(const FunctionDefinition* func) {
        return this->returnComplexity(func) >= Analysis::ReturnComplexity::kEarlyReturns;
    }

    bool needsFunctionResultSlots(const FunctionDefinition* func) {
        return this->shouldWriteTraceOps() || (this->returnComplexity(func) >
                                               Analysis::ReturnComplexity::kSingleSafeReturn);
    }

    static bool IsUniform(const Variable& var) {
       return var.modifierFlags().isUniform();
    }

    static bool IsOutParameter(const Variable& var) {
        return (var.modifierFlags() & (ModifierFlag::kIn | ModifierFlag::kOut)) ==
               ModifierFlag::kOut;
    }

    static bool IsInoutParameter(const Variable& var) {
        return (var.modifierFlags() & (ModifierFlag::kIn | ModifierFlag::kOut)) ==
               (ModifierFlag::kIn | ModifierFlag::kOut);
    }

private:
    const SkSL::Program& fProgram;
    SkSL::Context fContext;
    Builder fBuilder;
    DebugTracePriv* fDebugTrace = nullptr;
    bool fWriteTraceOps = false;
    THashMap<const Variable*, int> fChildEffectMap;

    SlotManager fProgramSlots;
    SlotManager fUniformSlots;
    SlotManager fImmutableSlots;

    std::optional<AutoStack> fTraceMask;
    const FunctionDefinition* fCurrentFunction = nullptr;
    SlotRange fCurrentFunctionResult;
    AutoContinueMask* fCurrentContinueMask = nullptr;
    int fCurrentBreakTarget = -1;
    int fCurrentStack = 0;
    int fNextStackID = 0;
    TArray<int> fRecycledStacks;

    THashMap<const FunctionDefinition*, Analysis::ReturnComplexity> fReturnComplexityMap;

    THashMap<ImmutableBits, THashSet<Slot>> fImmutableSlotMap;
    THashSet<const Variable*> fImmutableVariables;

    // `fInsideCompoundStatement` will be nonzero if we are currently writing statements inside of a
    // compound-statement Block. (Conceptually those statements should all count as one.)
    int fInsideCompoundStatement = 0;

    // `fLineOffsets` contains the position of each newline in the source, plus a zero at the
    // beginning, and the total source length at the end, as sentinels.
    TArray<int> fLineOffsets;

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
    static constexpr auto kModOps = TypedOps{BuilderOp::mod_n_floats,
                                             BuilderOp::unsupported,
                                             BuilderOp::unsupported,
                                             BuilderOp::unsupported};
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
    static constexpr auto kInverseSqrtOps = TypedOps{BuilderOp::invsqrt_float,
                                                     BuilderOp::unsupported,
                                                     BuilderOp::unsupported,
                                                     BuilderOp::unsupported};
    friend class AutoContinueMask;
};

AutoStack::AutoStack(Generator* g)
        : fGenerator(g)
        , fStackID(g->createStack()) {}

AutoStack::~AutoStack() {
    fGenerator->recycleStack(fStackID);
}

void AutoStack::enter() {
    fParentStackID = fGenerator->currentStack();
    fGenerator->setCurrentStack(fStackID);
}

void AutoStack::exit() {
    SkASSERT(fGenerator->currentStack() == fStackID);
    fGenerator->setCurrentStack(fParentStackID);
}

void AutoStack::pushClone(int slots) {
    this->pushClone(SlotRange{0, slots}, /*offsetFromStackTop=*/slots);
}

void AutoStack::pushClone(SlotRange range, int offsetFromStackTop) {
    fGenerator->builder()->push_clone_from_stack(range, fStackID, offsetFromStackTop);
}

void AutoStack::pushCloneIndirect(SlotRange range, int dynamicStackID, int offsetFromStackTop) {
    fGenerator->builder()->push_clone_indirect_from_stack(
            range, dynamicStackID, /*otherStackID=*/fStackID, offsetFromStackTop);
}

class AutoContinueMask {
public:
    AutoContinueMask(Generator* gen) : fGenerator(gen) {}

    ~AutoContinueMask() {
        if (fPreviousContinueMask) {
            fGenerator->fCurrentContinueMask = fPreviousContinueMask;
        }
    }

    void enable() {
        SkASSERT(!fContinueMaskStack.has_value());

        fContinueMaskStack.emplace(fGenerator);
        fPreviousContinueMask = fGenerator->fCurrentContinueMask;
        fGenerator->fCurrentContinueMask = this;
    }

    void enter() {
        SkASSERT(fContinueMaskStack.has_value());
        fContinueMaskStack->enter();
    }

    void exit() {
        SkASSERT(fContinueMaskStack.has_value());
        fContinueMaskStack->exit();
    }

    void enterLoopBody() {
        if (fContinueMaskStack.has_value()) {
            fContinueMaskStack->enter();
            fGenerator->builder()->push_constant_i(0);
            fContinueMaskStack->exit();
        }
    }

    void exitLoopBody() {
        if (fContinueMaskStack.has_value()) {
            fContinueMaskStack->enter();
            fGenerator->builder()->pop_and_reenable_loop_mask();
            fContinueMaskStack->exit();
        }
    }

    int stackID() {
        SkASSERT(fContinueMaskStack.has_value());
        return fContinueMaskStack->stackID();
    }

private:
    std::optional<AutoStack> fContinueMaskStack;
    Generator* fGenerator = nullptr;
    AutoContinueMask* fPreviousContinueMask = nullptr;
};

class AutoLoopTarget {
public:
    AutoLoopTarget(Generator* gen, int* targetPtr) : fGenerator(gen), fLoopTargetPtr(targetPtr) {
        fLabelID = fGenerator->builder()->nextLabelID();
        fPreviousLoopTarget = *fLoopTargetPtr;
        *fLoopTargetPtr = fLabelID;
    }

    ~AutoLoopTarget() {
        *fLoopTargetPtr = fPreviousLoopTarget;
    }

    int labelID() {
        return fLabelID;
    }

private:
    Generator* fGenerator = nullptr;
    int* fLoopTargetPtr = nullptr;
    int fPreviousLoopTarget;
    int fLabelID;
};

class LValue {
public:
    virtual ~LValue() = default;

    /** Returns true if this lvalue is actually writable--temporaries and uniforms are not. */
    virtual bool isWritable() const = 0;

    /**
     * Returns the fixed slot range of the lvalue, after it is winnowed down to the selected
     * field/index. The range is calculated assuming every dynamic index will evaluate to zero.
     */
    virtual SlotRange fixedSlotRange(Generator* gen) = 0;

    /**
     * Returns a stack which holds a single integer, representing the dynamic offset of the lvalue.
     * This value does not incorporate the fixed offset. If null is returned, the lvalue doesn't
     * have a dynamic offset. `evaluateDynamicIndices` must be called before this is used.
     */
    virtual AutoStack* dynamicSlotRange() = 0;

    /** Returns the swizzle components of the lvalue, or an empty span for non-swizzle LValues. */
    virtual SkSpan<const int8_t> swizzle() { return {}; }

    /** Pushes values directly onto the stack. */
    [[nodiscard]] virtual bool push(Generator* gen,
                                    SlotRange fixedOffset,
                                    AutoStack* dynamicOffset,
                                    SkSpan<const int8_t> swizzle) = 0;

    /** Stores topmost values from the stack directly into the lvalue. */
    [[nodiscard]] virtual bool store(Generator* gen,
                                     SlotRange fixedOffset,
                                     AutoStack* dynamicOffset,
                                     SkSpan<const int8_t> swizzle) = 0;
    /**
     * Some lvalues refer to a temporary expression; these temps can be held in the
     * scratch-expression field to ensure that they exist for the lifetime of the lvalue.
     */
    std::unique_ptr<Expression> fScratchExpression;
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

    AutoStack* dynamicSlotRange() override {
        return nullptr;
    }

    [[nodiscard]] bool push(Generator* gen,
                            SlotRange fixedOffset,
                            AutoStack* dynamicOffset,
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

        if (dynamicOffset) {
            fDedicatedStack->pushCloneIndirect(fixedOffset, dynamicOffset->stackID(), fNumSlots);
        } else {
            fDedicatedStack->pushClone(fixedOffset, fNumSlots);
        }
        if (!swizzle.empty()) {
            gen->builder()->swizzle(fixedOffset.count, swizzle);
        }
        return true;
    }

    [[nodiscard]] bool store(Generator*, SlotRange, AutoStack*, SkSpan<const int8_t>) override {
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

    AutoStack* dynamicSlotRange() override {
        return nullptr;
    }

    [[nodiscard]] bool push(Generator* gen,
                            SlotRange fixedOffset,
                            AutoStack* dynamicOffset,
                            SkSpan<const int8_t> swizzle) override {
        if (Generator::IsUniform(*fVariable)) {
            if (dynamicOffset) {
                gen->builder()->push_uniform_indirect(fixedOffset, dynamicOffset->stackID(),
                                                      this->fixedSlotRange(gen));
            } else {
                gen->builder()->push_uniform(fixedOffset);
            }
        } else {
            if (dynamicOffset) {
                gen->builder()->push_slots_indirect(fixedOffset, dynamicOffset->stackID(),
                                                    this->fixedSlotRange(gen));
            } else {
                gen->builder()->push_slots(fixedOffset);
            }
        }
        if (!swizzle.empty()) {
            gen->builder()->swizzle(fixedOffset.count, swizzle);
        }
        return true;
    }

    [[nodiscard]] bool store(Generator* gen,
                             SlotRange fixedOffset,
                             AutoStack* dynamicOffset,
                             SkSpan<const int8_t> swizzle) override {
        SkASSERT(!Generator::IsUniform(*fVariable));

        if (swizzle.empty()) {
            if (dynamicOffset) {
                gen->builder()->copy_stack_to_slots_indirect(fixedOffset, dynamicOffset->stackID(),
                                                             this->fixedSlotRange(gen));
            } else {
                gen->builder()->copy_stack_to_slots(fixedOffset);
            }
        } else {
            if (dynamicOffset) {
                gen->builder()->swizzle_copy_stack_to_slots_indirect(fixedOffset,
                                                                     dynamicOffset->stackID(),
                                                                     this->fixedSlotRange(gen),
                                                                     swizzle,
                                                                     swizzle.size());
            } else {
                gen->builder()->swizzle_copy_stack_to_slots(fixedOffset, swizzle, swizzle.size());
            }
        }
        if (gen->shouldWriteTraceOps()) {
            if (dynamicOffset) {
                gen->builder()->trace_var_indirect(gen->traceMaskStackID(),
                                                   fixedOffset,
                                                   dynamicOffset->stackID(),
                                                   this->fixedSlotRange(gen));
            } else {
                gen->builder()->trace_var(gen->traceMaskStackID(), fixedOffset);
            }
        }
        return true;
    }

private:
    const Variable* fVariable;
};

class ImmutableLValue final : public LValue {
public:
    explicit ImmutableLValue(const Variable* v) : fVariable(v) {}

    bool isWritable() const override {
        return false;
    }

    SlotRange fixedSlotRange(Generator* gen) override {
        return gen->getImmutableSlots(*fVariable);
    }

    AutoStack* dynamicSlotRange() override {
        return nullptr;
    }

    [[nodiscard]] bool push(Generator* gen,
                            SlotRange fixedOffset,
                            AutoStack* dynamicOffset,
                            SkSpan<const int8_t> swizzle) override {
        if (dynamicOffset) {
            gen->builder()->push_immutable_indirect(fixedOffset, dynamicOffset->stackID(),
                                                    this->fixedSlotRange(gen));
        } else {
            gen->builder()->push_immutable(fixedOffset);
        }
        if (!swizzle.empty()) {
            gen->builder()->swizzle(fixedOffset.count, swizzle);
        }
        return true;
    }

    [[nodiscard]] bool store(Generator* gen,
                             SlotRange fixedOffset,
                             AutoStack* dynamicOffset,
                             SkSpan<const int8_t> swizzle) override {
        SkDEBUGFAIL("immutable values cannot be stored into");
        return unsupported();
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

    AutoStack* dynamicSlotRange() override {
        return fParent->dynamicSlotRange();
    }

    SkSpan<const int8_t> swizzle() override {
        return fComponents;
    }

    [[nodiscard]] bool push(Generator* gen,
                            SlotRange fixedOffset,
                            AutoStack* dynamicOffset,
                            SkSpan<const int8_t> swizzle) override {
        if (!swizzle.empty()) {
            SkDEBUGFAIL("swizzle-of-a-swizzle should have been folded out in front end");
            return unsupported();
        }
        return fParent->push(gen, fixedOffset, dynamicOffset, fComponents);
    }

    [[nodiscard]] bool store(Generator* gen,
                             SlotRange fixedOffset,
                             AutoStack* dynamicOffset,
                             SkSpan<const int8_t> swizzle) override {
        if (!swizzle.empty()) {
            SkDEBUGFAIL("swizzle-of-a-swizzle should have been folded out in front end");
            return unsupported();
        }
        return fParent->store(gen, fixedOffset, dynamicOffset, fComponents);
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

    AutoStack* dynamicSlotRange() override {
        return fParent->dynamicSlotRange();
    }

    [[nodiscard]] bool push(Generator* gen,
                            SlotRange fixedOffset,
                            AutoStack* dynamicOffset,
                            SkSpan<const int8_t> swizzle) override {
        return fParent->push(gen, fixedOffset, dynamicOffset, swizzle);
    }

    [[nodiscard]] bool store(Generator* gen,
                             SlotRange fixedOffset,
                             AutoStack* dynamicOffset,
                             SkSpan<const int8_t> swizzle) override {
        return fParent->store(gen, fixedOffset, dynamicOffset, swizzle);
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

class DynamicIndexLValue final : public LValue {
public:
    explicit DynamicIndexLValue(std::unique_ptr<LValue> p, const IndexExpression& i)
            : fParent(std::move(p))
            , fIndexExpr(&i) {
        SkASSERT(fIndexExpr->index()->type().isInteger());
    }

    ~DynamicIndexLValue() override {
        if (fDedicatedStack.has_value()) {
            SkASSERT(fGenerator);

            // Jettison the index expression.
            fDedicatedStack->enter();
            fGenerator->discardExpression(/*slots=*/1);
            fDedicatedStack->exit();
        }
    }

    bool isWritable() const override {
        return fParent->isWritable();
    }

    [[nodiscard]] bool evaluateDynamicIndices(Generator* gen) {
        // The index must only be computed once; the index-expression could have side effects.
        // Once it has been computed, the offset lives on `fDedicatedStack`.
        SkASSERT(!fDedicatedStack.has_value());
        SkASSERT(!fGenerator);
        fGenerator = gen;
        fDedicatedStack.emplace(fGenerator);

        if (!fParent->swizzle().empty()) {
            SkDEBUGFAIL("an indexed-swizzle should have been handled by RewriteIndexedSwizzle");
            return unsupported();
        }

        // Push the index expression onto the dedicated stack.
        fDedicatedStack->enter();
        if (!fGenerator->pushExpression(*fIndexExpr->index())) {
            return unsupported();
        }

        // Multiply the index-expression result by the per-value slot count.
        int slotCount = fIndexExpr->type().slotCount();
        if (slotCount != 1) {
            fGenerator->builder()->push_constant_i(fIndexExpr->type().slotCount());
            fGenerator->builder()->binary_op(BuilderOp::mul_n_ints, 1);
        }

        // Check to see if a parent LValue already has a dynamic index. If so, we need to
        // incorporate its value into our own.
        if (AutoStack* parentDynamicIndexStack = fParent->dynamicSlotRange()) {
            parentDynamicIndexStack->pushClone(/*slots=*/1);
            fGenerator->builder()->binary_op(BuilderOp::add_n_ints, 1);
        }
        fDedicatedStack->exit();
        return true;
    }

    SlotRange fixedSlotRange(Generator* gen) override {
        // Compute the fixed slot range as if we are indexing into position zero.
        SlotRange range = fParent->fixedSlotRange(gen);
        range.count = fIndexExpr->type().slotCount();
        return range;
    }

    AutoStack* dynamicSlotRange() override {
        // We incorporated any parent dynamic offsets when `evaluateDynamicIndices` was called.
        SkASSERT(fDedicatedStack.has_value());
        return &*fDedicatedStack;
    }

    [[nodiscard]] bool push(Generator* gen,
                            SlotRange fixedOffset,
                            AutoStack* dynamicOffset,
                            SkSpan<const int8_t> swizzle) override {
        return fParent->push(gen, fixedOffset, dynamicOffset, swizzle);
    }

    [[nodiscard]] bool store(Generator* gen,
                             SlotRange fixedOffset,
                             AutoStack* dynamicOffset,
                             SkSpan<const int8_t> swizzle) override {
        return fParent->store(gen, fixedOffset, dynamicOffset, swizzle);
    }

private:
    Generator* fGenerator = nullptr;
    std::unique_ptr<LValue> fParent;
    std::optional<AutoStack> fDedicatedStack;
    const IndexExpression* fIndexExpr = nullptr;
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
            for (const Field& field : type.fields()) {
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

std::optional<SlotRange> SlotManager::mapVariableToSlots(const Variable& v, SlotRange range) {
    SkASSERT(v.type().slotCount() == SkToSizeT(range.count));
    const SlotRange* existingEntry = fSlotMap.find(&v);
    std::optional<SlotRange> originalRange = existingEntry ? std::optional(*existingEntry)
                                                           : std::nullopt;
    fSlotMap.set(&v, range);
    return originalRange;
}

void SlotManager::unmapVariableSlots(const Variable& v) {
    fSlotMap.remove(&v);
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
    this->mapVariableToSlots(v, range);
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
        const Variable* variable = e.as<VariableReference>().variable();
        if (fImmutableVariables.contains(variable)) {
            return std::make_unique<ImmutableLValue>(variable);
        }
        return std::make_unique<VariableLValue>(variable);
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

        // If the index base is swizzled (`vec.zyx[idx]`), rewrite it into an equivalent
        // non-swizzled form (`vec[uint3(2,1,0)[idx]]`).
        if (std::unique_ptr<Expression> rewritten = Transform::RewriteIndexedSwizzle(fContext,
                                                                                     indexExpr)) {
            // Convert the rewritten expression into an lvalue.
            std::unique_ptr<LValue> lvalue = this->makeLValue(*rewritten, allowScratch);
            if (!lvalue) {
                return nullptr;
            }
            // We need to hold onto the rewritten expression for the lifetime of the lvalue.
            lvalue->fScratchExpression = std::move(rewritten);
            return lvalue;
        }
        if (std::unique_ptr<LValue> base = this->makeLValue(*indexExpr.base(),
                                                            allowScratch)) {
            // If the index is a compile-time constant, we can represent it with a fixed slice.
            SKSL_INT indexValue;
            if (ConstantFolder::GetConstantInt(*indexExpr.index(), &indexValue)) {
                int numSlots = indexExpr.type().slotCount();
                return std::make_unique<LValueSlice>(std::move(base), numSlots * indexValue,
                                                     numSlots);
            }

            // Represent non-constant indexing via a dynamic index.
            auto dynLValue = std::make_unique<DynamicIndexLValue>(std::move(base), indexExpr);
            return dynLValue->evaluateDynamicIndices(this) ? std::move(dynLValue)
                                                           : nullptr;
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

bool Generator::push(LValue& lvalue) {
    return lvalue.push(this,
                       lvalue.fixedSlotRange(this),
                       lvalue.dynamicSlotRange(),
                       /*swizzle=*/{});
}

bool Generator::store(LValue& lvalue) {
    SkASSERT(lvalue.isWritable());
    return lvalue.store(this,
                        lvalue.fixedSlotRange(this),
                        lvalue.dynamicSlotRange(),
                        /*swizzle=*/{});
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

std::optional<SlotRange> Generator::writeFunction(
        const IRNode& callSite,
        const FunctionDefinition& function,
        SkSpan<std::unique_ptr<Expression> const> arguments) {
    // Generate debug information and emit a trace-enter op.
    int funcIndex = -1;
    if (fDebugTrace) {
        funcIndex = this->getFunctionDebugInfo(function.declaration());
        SkASSERT(funcIndex >= 0);
        if (this->shouldWriteTraceOps()) {
            fBuilder.trace_enter(fTraceMask->stackID(), funcIndex);
        }
    }

    // Handle parameter lvalues.
    struct RemappedSlotRange {
        const Variable* fVariable;
        std::optional<SlotRange> fSlotRange;
    };
    SkSpan<Variable* const> parameters = function.declaration().parameters();
    TArray<std::unique_ptr<LValue>> lvalues;
    TArray<RemappedSlotRange> remappedSlotRanges;

    if (function.declaration().isMain()) {
        // For main(), the parameter slots have already been populated by `writeProgram`, but we
        // still need to explicitly emit trace ops for the variables in main(), since they are
        // initialized before it is safe to use trace-var. (We can't invoke init-lane-masks until
        // after we've copied the inputs from main into slots, because dst.rgba is used to pass in a
        // blend-destination color, but we clobber it and put in the execution mask instead.)
        if (this->shouldWriteTraceOps()) {
            for (const Variable* var : parameters) {
                fBuilder.trace_var(fTraceMask->stackID(), this->getVariableSlots(*var));
            }
        }
    } else {
        // Write all the arguments into their parameter's variable slots. Because we never allow
        // recursion, we don't need to worry about overwriting any existing values in those slots.
        // (In fact, we don't even need to apply the write mask.)
        lvalues.resize(arguments.size());

        for (size_t index = 0; index < arguments.size(); ++index) {
            const Expression& arg = *arguments[index];
            const Variable& param = *parameters[index];

            // Use LValues for out-parameters and inout-parameters, so we can store back to them
            // later.
            if (IsInoutParameter(param) || IsOutParameter(param)) {
                lvalues[index] = this->makeLValue(arg);
                if (!lvalues[index]) {
                    return std::nullopt;
                }
                // There are no guarantees on the starting value of an out-parameter, so we only
                // need to store the lvalues associated with an inout parameter.
                if (IsInoutParameter(param)) {
                    if (!this->push(*lvalues[index])) {
                        return std::nullopt;
                    }
                    this->popToSlotRangeUnmasked(this->getVariableSlots(param));
                }
                continue;
            }

            // If a parameter is never read by the function, we don't need to populate its slots.
            ProgramUsage::VariableCounts paramCounts = fProgram.fUsage->get(param);
            if (paramCounts.fRead == 0) {
                // Honor the expression's side effects, if any.
                if (Analysis::HasSideEffects(arg)) {
                    if (!this->pushExpression(arg, /*usesResult=*/false)) {
                        return std::nullopt;
                    }
                    this->discardExpression(arg.type().slotCount());
                }
                continue;
            }

            // If the expression is a plain variable and the parameter is never written to, we don't
            // need to copy it; we can just share the slots from the existing variable.
            if (paramCounts.fWrite == 0 && arg.is<VariableReference>()) {
                const Variable& var = *arg.as<VariableReference>().variable();
                if (this->hasVariableSlots(var)) {
                    std::optional<SlotRange> originalRange =
                            fProgramSlots.mapVariableToSlots(param, this->getVariableSlots(var));
                    remappedSlotRanges.push_back({&param, originalRange});
                    continue;
                }
            }

            // Copy input arguments into their respective parameter slots.
            if (!this->pushExpression(arg)) {
                return std::nullopt;
            }
            this->popToSlotRangeUnmasked(this->getVariableSlots(param));
        }
    }

    // Set up a slot range dedicated to this function's return value.
    SlotRange lastFunctionResult = fCurrentFunctionResult;
    fCurrentFunctionResult = this->getFunctionSlots(callSite, function.declaration());

    // Save off the return mask.
    if (this->needsReturnMask(&function)) {
        fBuilder.enableExecutionMaskWrites();
        if (!function.declaration().isMain()) {
            fBuilder.push_return_mask();
        }
    }

    // Emit the function body.
    if (!this->writeStatement(*function.body())) {
        return std::nullopt;
    }

    // Restore the original return mask.
    if (this->needsReturnMask(&function)) {
        if (!function.declaration().isMain()) {
            fBuilder.pop_return_mask();
        }
        fBuilder.disableExecutionMaskWrites();
    }

    // Restore the function-result slot range.
    SlotRange functionResult = fCurrentFunctionResult;
    fCurrentFunctionResult = lastFunctionResult;

    // Emit a trace-exit op.
    if (fDebugTrace && fWriteTraceOps) {
        fBuilder.trace_exit(fTraceMask->stackID(), funcIndex);
    }

    // Copy out-parameters and inout-parameters back to their homes.
    for (int index = 0; index < lvalues.size(); ++index) {
        if (lvalues[index]) {
            // Only out- and inout-parameters should have an associated lvalue.
            const Variable& param = *parameters[index];
            SkASSERT(IsInoutParameter(param) || IsOutParameter(param));

            // Copy the parameter's slots directly into the lvalue.
            fBuilder.push_slots(this->getVariableSlots(param));
            if (!this->store(*lvalues[index])) {
                return std::nullopt;
            }
            this->discardExpression(param.type().slotCount());
        }
    }

    // Restore any remapped parameter slot ranges to their original values.
    for (const RemappedSlotRange& remapped : remappedSlotRanges) {
        if (remapped.fSlotRange.has_value()) {
            fProgramSlots.mapVariableToSlots(*remapped.fVariable, *remapped.fSlotRange);
        } else {
            fProgramSlots.unmapVariableSlots(*remapped.fVariable);
        }
    }

    return functionResult;
}

void Generator::emitTraceLine(Position pos) {
    if (fDebugTrace && fWriteTraceOps && pos.valid() && fInsideCompoundStatement == 0) {
        // Binary search within fLineOffets to convert the position into a line number.
        SkASSERT(fLineOffsets.size() >= 2);
        SkASSERT(fLineOffsets[0] == 0);
        SkASSERT(fLineOffsets.back() == (int)fProgram.fSource->length());
        int lineNumber = std::distance(
                fLineOffsets.begin(),
                std::upper_bound(fLineOffsets.begin(), fLineOffsets.end(), pos.startOffset()));

        fBuilder.trace_line(fTraceMask->stackID(), lineNumber);
    }
}

void Generator::pushTraceScopeMask() {
    if (this->shouldWriteTraceOps()) {
        // Take the intersection of the trace mask and the execution mask. To do this, start with an
        // all-zero mask, then use select to overwrite those zeros with the trace mask across all
        // executing lanes. We'll get the trace mask in executing lanes, and zero in dead lanes.
        fBuilder.push_constant_i(0);
        fTraceMask->pushClone(/*slots=*/1);
        fBuilder.select(/*slots=*/1);
    }
}

void Generator::discardTraceScopeMask() {
    if (this->shouldWriteTraceOps()) {
        this->discardExpression(/*slots=*/1);
    }
}

void Generator::emitTraceScope(int delta) {
    if (this->shouldWriteTraceOps()) {
        fBuilder.trace_scope(this->currentStack(), delta);
    }
}

void Generator::calculateLineOffsets() {
    SkASSERT(fLineOffsets.empty());
    fLineOffsets.push_back(0);
    for (size_t i = 0; i < fProgram.fSource->length(); ++i) {
        if ((*fProgram.fSource)[i] == '\n') {
            fLineOffsets.push_back(i);
        }
    }
    fLineOffsets.push_back(fProgram.fSource->length());
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
            if (int builtin = var->layout().fBuiltin; builtin >= 0) {
                if (builtin == SK_FRAGCOORD_BUILTIN) {
                    fBuilder.store_device_xy01(this->getVariableSlots(*var));
                    continue;
                }
                // The only builtin variable exposed to runtime effects is sk_FragCoord.
                return unsupported();
            }

            if (IsUniform(*var)) {
                // Create the uniform slot map in first-to-last order.
                SlotRange uniformSlotRange = this->getUniformSlots(*var);

                if (this->shouldWriteTraceOps()) {
                    // We expect uniform values to show up in the debug trace. To make this happen
                    // without updating the file format, we synthesize a value-slot range for the
                    // uniform here, and copy the uniform data into the value slots. This allows
                    // trace_var to work naturally. This wastes a bit of memory, but debug traces
                    // don't need to be hyper-efficient.
                    SlotRange copyRange = fProgramSlots.getVariableSlots(*var);
                    fBuilder.push_uniform(uniformSlotRange);
                    this->popToSlotRangeUnmasked(copyRange);
                }

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
            // The debugger will stop on statements inside Blocks; there's no need for an additional
            // stop on the block's initial open-brace.
        case Statement::Kind::kFor:
            // The debugger will stop on the init-statement of a for statement, so we don't need to
            // stop on the outer for-statement itself as well.
            break;

        default:
            // The debugger should stop on other statements.
            this->emitTraceLine(s.fPosition);
            break;
    }

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
    if (b.blockKind() == Block::Kind::kCompoundStatement) {
        this->emitTraceLine(b.fPosition);
        ++fInsideCompoundStatement;
    } else {
        this->pushTraceScopeMask();
        this->emitTraceScope(+1);
    }

    for (const std::unique_ptr<Statement>& stmt : b.children()) {
        if (!this->writeStatement(*stmt)) {
            return unsupported();
        }
    }

    if (b.blockKind() == Block::Kind::kCompoundStatement) {
        --fInsideCompoundStatement;
    } else {
        this->emitTraceScope(-1);
        this->discardTraceScopeMask();
    }

    return true;
}

bool Generator::writeBreakStatement(const BreakStatement&) {
    // If all lanes have reached this break, we can just branch straight to the break target instead
    // of updating masks.
    fBuilder.branch_if_all_lanes_active(fCurrentBreakTarget);
    fBuilder.mask_off_loop_mask();
    return true;
}

bool Generator::writeContinueStatement(const ContinueStatement&) {
    fBuilder.continue_op(fCurrentContinueMask->stackID());
    return true;
}

bool Generator::writeDoStatement(const DoStatement& d) {
    // Set up a break target.
    AutoLoopTarget breakTarget(this, &fCurrentBreakTarget);

    // Save off the original loop mask.
    fBuilder.enableExecutionMaskWrites();
    fBuilder.push_loop_mask();

    // If `continue` is used in the loop...
    Analysis::LoopControlFlowInfo loopInfo = Analysis::GetLoopControlFlowInfo(*d.statement());
    AutoContinueMask autoContinueMask(this);
    if (loopInfo.fHasContinue) {
        // ... create a temporary slot for continue-mask storage.
        autoContinueMask.enable();
    }

    // Write the do-loop body.
    int labelID = fBuilder.nextLabelID();
    fBuilder.label(labelID);

    autoContinueMask.enterLoopBody();

    if (!this->writeStatement(*d.statement())) {
        return false;
    }

    autoContinueMask.exitLoopBody();

    // Point the debugger at the do-statement's test-expression before we run it.
    this->emitTraceLine(d.test()->fPosition);

    // Emit the test-expression, in order to combine it with the loop mask.
    if (!this->pushExpression(*d.test())) {
        return false;
    }

    // Mask off any lanes in the loop mask where the test-expression is false; this breaks the loop.
    // We don't use the test expression for anything else, so jettison it.
    fBuilder.merge_loop_mask();
    this->discardExpression(/*slots=*/1);

    // If any lanes are still running, go back to the top and run the loop body again.
    fBuilder.branch_if_any_lanes_active(labelID);

    // If we hit a break statement on all lanes, we will branch here to escape from the loop.
    fBuilder.label(breakTarget.labelID());

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

    // We want the loop index to disappear at the end of the loop, so wrap the for statement in a
    // trace scope.
    this->pushTraceScopeMask();
    this->emitTraceScope(+1);

    // If no lanes are active, skip over the loop entirely. This guards against looping forever;
    // with no lanes active, we wouldn't be able to write the loop variable back to its slot, so
    // we'd never make forward progress.
    int loopExitID = fBuilder.nextLabelID();
    int loopBodyID = fBuilder.nextLabelID();
    fBuilder.branch_if_no_lanes_active(loopExitID);

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

    // Point the debugger at the for-statement's next-expression before we run it, or as close as we
    // can reasonably get.
    if (f.next()) {
        this->emitTraceLine(f.next()->fPosition);
    } else if (f.test()) {
        this->emitTraceLine(f.test()->fPosition);
    } else {
        this->emitTraceLine(f.fPosition);
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

    this->emitTraceScope(-1);
    this->discardTraceScopeMask();
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

    // We want the loop index to disappear at the end of the loop, so wrap the for statement in a
    // trace scope.
    this->pushTraceScopeMask();
    this->emitTraceScope(+1);

    // Set up a break target.
    AutoLoopTarget breakTarget(this, &fCurrentBreakTarget);

    // Run the loop initializer.
    if (f.initializer()) {
        if (!this->writeStatement(*f.initializer())) {
            return unsupported();
        }
    } else {
        this->emitTraceLine(f.fPosition);
    }

    AutoContinueMask autoContinueMask(this);
    if (loopInfo.fHasContinue) {
        // Acquire a temporary slot for continue-mask storage.
        autoContinueMask.enable();
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

    autoContinueMask.enterLoopBody();

    if (!this->writeStatement(*f.statement())) {
        return unsupported();
    }

    autoContinueMask.exitLoopBody();

    // Point the debugger at the for-statement's next-expression before we run it, or as close as we
    // can reasonably get.
    if (f.next()) {
        this->emitTraceLine(f.next()->fPosition);
    } else if (f.test()) {
        this->emitTraceLine(f.test()->fPosition);
    } else {
        this->emitTraceLine(f.fPosition);
    }

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
    fBuilder.branch_if_any_lanes_active(loopBodyID);

    // If we hit a break statement on all lanes, we will branch here to escape from the loop.
    fBuilder.label(breakTarget.labelID());

    // Restore the loop mask.
    fBuilder.pop_loop_mask();
    fBuilder.disableExecutionMaskWrites();

    this->emitTraceScope(-1);
    this->discardTraceScopeMask();
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
        // Apply the inverse condition-mask. Then run the if-false branch.
        fBuilder.merge_inv_condition_mask();
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
        if (this->needsFunctionResultSlots(fCurrentFunction)) {
            this->popToSlotRange(fCurrentFunctionResult);
        }
    }
    if (fBuilder.executionMaskWritesAreEnabled() && this->needsReturnMask(fCurrentFunction)) {
        fBuilder.mask_off_return_mask();
    }
    return true;
}

bool Generator::writeSwitchStatement(const SwitchStatement& s) {
    const StatementArray& cases = s.cases();
    SkASSERT(std::all_of(cases.begin(), cases.end(), [](const std::unique_ptr<Statement>& stmt) {
        return stmt->is<SwitchCase>();
    }));

    // Set up a break target.
    AutoLoopTarget breakTarget(this, &fCurrentBreakTarget);

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
            fBuilder.branch_if_no_lanes_active(skipLabelID);
            if (!this->writeStatement(*sc.statement())) {
                return unsupported();
            }
        } else {
            // The case-op will enable the loop mask if the switch-value matches, and mask off lanes
            // from the default-mask.
            fBuilder.case_op(sc.value());
            // Execute the switch-case block, if any lanes are alive to see it.
            fBuilder.branch_if_no_lanes_active(skipLabelID);
            if (!this->writeStatement(*sc.statement())) {
                return unsupported();
            }
        }
        fBuilder.label(skipLabelID);
    }

    // Jettison the switch value, and the default case mask if it was never consumed above.
    this->discardExpression(/*slots=*/foundDefaultCase ? 1 : 2);

    // If we hit a break statement on all lanes, we will branch here to escape from the switch.
    fBuilder.label(breakTarget.labelID());

    // Restore the loop mask.
    fBuilder.pop_loop_mask();
    fBuilder.disableExecutionMaskWrites();
    return true;
}

bool Generator::writeImmutableVarDeclaration(const VarDeclaration& d) {
    // In a debugging session, we expect debug traces for a variable declaration to appear, even if
    // it's constant, so we don't use immutable slots for variables when tracing is on.
    if (this->shouldWriteTraceOps()) {
        return false;
    }

    // Find the constant value for this variable.
    const Expression* initialValue = ConstantFolder::GetConstantValueForVariable(*d.value());
    SkASSERT(initialValue);

    // For a variable to be immutable, it cannot be written-to besides its initial declaration.
    ProgramUsage::VariableCounts counts = fProgram.fUsage->get(*d.var());
    if (counts.fWrite != 1) {
        return false;
    }

    STArray<16, ImmutableBits> immutableValues;
    if (!this->getImmutableValueForExpression(*initialValue, &immutableValues)) {
        return false;
    }

    fImmutableVariables.add(d.var());

    std::optional<SlotRange> preexistingSlots = this->findPreexistingImmutableData(immutableValues);
    if (preexistingSlots.has_value()) {
        // Associate this variable with a preexisting range of immutable data (no new data or code).
        fImmutableSlots.mapVariableToSlots(*d.var(), *preexistingSlots);
    } else {
        // Write out the constant value back to immutable slots. (This generates data, but no
        // runtime code.)
        SlotRange slots = this->getImmutableSlots(*d.var());
        this->storeImmutableValueToSlots(immutableValues, slots);
    }

    return true;
}

bool Generator::writeVarDeclaration(const VarDeclaration& v) {
    if (v.value()) {
        // If a variable never actually changes, we can make it immutable.
        if (this->writeImmutableVarDeclaration(v)) {
            return true;
        }
        // This is a real variable which can change over the course of execution.
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

        case Expression::Kind::kConstructorArray:
        case Expression::Kind::kConstructorArrayCast:
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

        case Expression::Kind::kEmpty:
            return true;

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

    // Insert padding space on the stack to hold the result.
    fBuilder.pad_stack(rightColumns * leftRows);

    // Push the left and right matrices onto the stack.
    if (!this->pushLValueOrExpression(lvalue, left) || !this->pushExpression(right)) {
        return unsupported();
    }

    fBuilder.matrix_multiply(leftColumns, leftRows, rightColumns, rightRows);

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
        SkSpan<const Field> fields = type.fields();
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
            [[fallthrough]];

        // Rewrite commutative ops so that the literal is on the right-hand side. This gives the
        // Builder more opportunities to use immediate-mode ops.
        case OperatorKind::PLUS:
        case OperatorKind::STAR:
        case OperatorKind::BITWISEAND:
        case OperatorKind::BITWISEXOR:
        case OperatorKind::LOGICALXOR: {
            double unused;
            if (ConstantFolder::GetConstantValue(left, &unused) &&
                !ConstantFolder::GetConstantValue(right, &unused)) {
                return this->pushBinaryExpression(right, op, left);
            }
            break;
        }
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

std::optional<Generator::ImmutableBits> Generator::getImmutableBitsForSlot(const Expression& expr,
                                                                           size_t slot) {
    // Determine the constant-value of the slot; bail if it isn't constant.
    std::optional<double> v = expr.getConstantValue(slot);
    if (!v.has_value()) {
        return std::nullopt;
    }
    // Determine the number-kind of the slot, and convert the value to its bit-representation.
    Type::NumberKind kind = expr.type().slotType(slot).numberKind();
    double value = *v;
    switch (kind) {
        case Type::NumberKind::kFloat:
            return sk_bit_cast<ImmutableBits>((float)value);

        case Type::NumberKind::kSigned:
            return sk_bit_cast<ImmutableBits>((int32_t)value);

        case Type::NumberKind::kUnsigned:
            return sk_bit_cast<ImmutableBits>((uint32_t)value);

        case Type::NumberKind::kBoolean:
            return value ? ~0 : 0;

        default:
            return std::nullopt;
    }
}

bool Generator::getImmutableValueForExpression(const Expression& expr,
                                               TArray<ImmutableBits>* immutableValues) {
    if (!expr.supportsConstantValues()) {
        return false;
    }
    size_t numSlots = expr.type().slotCount();
    immutableValues->reserve_exact(numSlots);
    for (size_t index = 0; index < numSlots; ++index) {
        std::optional<ImmutableBits> bits = this->getImmutableBitsForSlot(expr, index);
        if (!bits.has_value()) {
            return false;
        }
        immutableValues->push_back(*bits);
    }
    return true;
}

void Generator::storeImmutableValueToSlots(const TArray<ImmutableBits>& immutableValues,
                                           SlotRange slots) {
    for (int index = 0; index < slots.count; ++index) {
        // Store the immutable value in its slot.
        const Slot slot = slots.index++;
        const ImmutableBits bits = immutableValues[index];
        fBuilder.store_immutable_value_i(slot, bits);

        // Keep track of every stored immutable value for potential later reuse.
        fImmutableSlotMap[bits].add(slot);
    }
}

std::optional<SlotRange> Generator::findPreexistingImmutableData(
        const TArray<ImmutableBits>& immutableValues) {
    STArray<16, const THashSet<Slot>*> slotArray;
    slotArray.reserve_exact(immutableValues.size());

    // Find all the slots associated with each immutable-value bit representation.
    // If a given bit-pattern doesn't exist anywhere in our program yet, we can stop searching.
    for (const ImmutableBits& immutableValue : immutableValues) {
        const THashSet<Slot>* slotsForValue = fImmutableSlotMap.find(immutableValue);
        if (!slotsForValue) {
            return std::nullopt;
        }
        slotArray.push_back(slotsForValue);
    }

    // Look for the group with the fewest number of entries, since that can be searched in the
    // least amount of effort.
    int leastSlotIndex = 0, leastSlotCount = INT_MAX;
    for (int index = 0; index < slotArray.size(); ++index) {
        int currentCount = slotArray[index]->count();
        if (currentCount < leastSlotCount) {
            leastSlotIndex = index;
            leastSlotCount = currentCount;
        }
    }

    // See if we can reconstitute the value that we want with any of the data we've already got.
    for (int slot : *slotArray[leastSlotIndex]) {
        int firstSlot = slot - leastSlotIndex;
        bool found = true;
        for (int index = 0; index < slotArray.size(); ++index) {
            if (!slotArray[index]->contains(firstSlot + index)) {
                found = false;
                break;
            }
        }
        if (found) {
            // We've found an exact match for the input value; return its slot-range.
            return SlotRange{firstSlot, slotArray.size()};
        }
    }

    // We didn't find any reusable slot ranges.
    return std::nullopt;
}

bool Generator::pushImmutableData(const Expression& e) {
    STArray<16, ImmutableBits> immutableValues;
    if (!this->getImmutableValueForExpression(e, &immutableValues)) {
        return false;
    }
    std::optional<SlotRange> preexistingData = this->findPreexistingImmutableData(immutableValues);
    if (preexistingData.has_value()) {
        fBuilder.push_immutable(*preexistingData);
        return true;
    }
    SlotRange range = fImmutableSlots.createSlots(e.description(),
                                                  e.type(),
                                                  e.fPosition,
                                                  /*isFunctionReturnValue=*/false);
    this->storeImmutableValueToSlots(immutableValues, range);
    fBuilder.push_immutable(range);
    return true;
}

bool Generator::pushConstructorCompound(const AnyConstructor& c) {
    if (c.type().slotCount() > 1 && this->pushImmutableData(c)) {
        return true;
    }
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
            SkASSERT(arg->type().matches(*fContext.fTypes.fFloat2));

            // `exchange_src` will use the top four values on the stack, but we don't care what goes
            // into the blue/alpha components. We inject padding here to balance the stack.
            fBuilder.pad_stack(2);

            // Move the argument into src.rgba while also preserving the execution mask.
            fBuilder.exchange_src();
            fBuilder.invoke_shader(*childIdx);
            break;
        }
        case Type::TypeKind::kColorFilter: {
            // The argument must be a half4/float4.
            SkASSERT(c.arguments().size() == 1);
            SkASSERT(arg->type().matches(*fContext.fTypes.fHalf4) ||
                     arg->type().matches(*fContext.fTypes.fFloat4));

            // Move the argument into src.rgba while also preserving the execution mask.
            fBuilder.exchange_src();
            fBuilder.invoke_color_filter(*childIdx);
            break;
        }
        case Type::TypeKind::kBlender: {
            // Both arguments must be half4/float4.
            SkASSERT(c.arguments().size() == 2);
            SkASSERT(c.arguments()[0]->type().matches(*fContext.fTypes.fHalf4) ||
                     c.arguments()[0]->type().matches(*fContext.fTypes.fFloat4));
            SkASSERT(c.arguments()[1]->type().matches(*fContext.fTypes.fHalf4) ||
                     c.arguments()[1]->type().matches(*fContext.fTypes.fFloat4));

            // Move the second argument into dst.rgba, and the first argument into src.rgba, while
            // simultaneously preserving the execution mask.
            if (!this->pushExpression(*c.arguments()[1])) {
                return unsupported();
            }
            fBuilder.pop_dst_rgba();
            fBuilder.exchange_src();
            fBuilder.invoke_blender(*childIdx);
            break;
        }
        default: {
            SkDEBUGFAILF("cannot sample from type '%s'", c.child().type().description().c_str());
        }
    }

    // The child call has returned the result color via src.rgba, and the SkRP execution mask is
    // on top of the stack. Swapping the two puts the result color on top of the stack, and also
    // restores our execution masks.
    fBuilder.exchange_src();
    return true;
}

bool Generator::pushConstructorCast(const AnyConstructor& c) {
    SkASSERT(c.argumentSpan().size() == 1);
    const Expression& inner = *c.argumentSpan().front();
    SkASSERT(inner.type().slotCount() == c.type().slotCount());

    if (!this->pushExpression(inner)) {
        return unsupported();
    }
    const Type::NumberKind innerKind = inner.type().componentType().numberKind();
    const Type::NumberKind outerKind = c.type().componentType().numberKind();

    if (innerKind == outerKind) {
        // Since we ignore type precision, this cast is effectively a no-op.
        return true;
    }

    switch (innerKind) {
        case Type::NumberKind::kSigned:
            if (outerKind == Type::NumberKind::kUnsigned) {
                // Treat uint(int) as a no-op.
                return true;
            }
            if (outerKind == Type::NumberKind::kFloat) {
                fBuilder.unary_op(BuilderOp::cast_to_float_from_int, c.type().slotCount());
                return true;
            }
            break;

        case Type::NumberKind::kUnsigned:
            if (outerKind == Type::NumberKind::kSigned) {
                // Treat int(uint) as a no-op.
                return true;
            }
            if (outerKind == Type::NumberKind::kFloat) {
                fBuilder.unary_op(BuilderOp::cast_to_float_from_uint, c.type().slotCount());
                return true;
            }
            break;

        case Type::NumberKind::kBoolean:
            // Converting boolean to int or float can be accomplished via bitwise-and.
            if (outerKind == Type::NumberKind::kFloat) {
                fBuilder.push_constant_f(1.0f);
            } else if (outerKind == Type::NumberKind::kSigned ||
                       outerKind == Type::NumberKind::kUnsigned) {
                fBuilder.push_constant_i(1);
            } else {
                SkDEBUGFAILF("unexpected cast from bool to %s", c.type().description().c_str());
                return unsupported();
            }
            fBuilder.push_duplicates(c.type().slotCount() - 1);
            fBuilder.binary_op(BuilderOp::bitwise_and_n_ints, c.type().slotCount());
            return true;

        case Type::NumberKind::kFloat:
            if (outerKind == Type::NumberKind::kSigned) {
                fBuilder.unary_op(BuilderOp::cast_to_int_from_float, c.type().slotCount());
                return true;
            }
            if (outerKind == Type::NumberKind::kUnsigned) {
                fBuilder.unary_op(BuilderOp::cast_to_uint_from_float, c.type().slotCount());
                return true;
            }
            break;

        case Type::NumberKind::kNonnumeric:
            break;
    }

    if (outerKind == Type::NumberKind::kBoolean) {
        // Converting int or float to boolean can be accomplished via `notEqual(x, 0)`.
        fBuilder.push_zeros(c.type().slotCount());
        return this->binaryOp(inner.type(), kNotEqualOps);
    }

    SkDEBUGFAILF("unexpected cast from %s to %s",
                 c.type().description().c_str(), inner.type().description().c_str());
    return unsupported();
}

bool Generator::pushConstructorDiagonalMatrix(const ConstructorDiagonalMatrix& c) {
    if (this->pushImmutableData(c)) {
        return true;
    }
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
    // (If the function call was trivial, it would likely have been inlined in the frontend, so we
    // assume here that function calls generally represent a significant amount of work.)
    int skipLabelID = fBuilder.nextLabelID();
    fBuilder.branch_if_no_lanes_active(skipLabelID);

    // Emit the function body.
    std::optional<SlotRange> r = this->writeFunction(c, *fCurrentFunction, c.arguments());
    if (!r.has_value()) {
        return unsupported();
    }

    // If the function uses result slots, move its result from slots onto the stack.
    if (this->needsFunctionResultSlots(fCurrentFunction)) {
        fBuilder.push_slots(*r);
    }

    // We've returned back to the last function.
    fCurrentFunction = lastFunction;

    // Copy the function result from its slots onto the stack.
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

bool Generator::pushLengthIntrinsic(int slotCount) {
    if (slotCount == 1) {
        // `length(scalar)` is `sqrt(x^2)`, which is equivalent to `abs(x)`.
        return this->pushAbsFloatIntrinsic(/*slots=*/1);
    }
    // Implement `length(vec)` as `sqrt(dot(x, x))`.
    fBuilder.push_clone(slotCount);
    fBuilder.dot_floats(slotCount);
    fBuilder.unary_op(BuilderOp::sqrt_float, 1);
    return true;
}

bool Generator::pushAbsFloatIntrinsic(int slots) {
    // Perform abs(float) by masking off the sign bit.
    fBuilder.push_constant_u(0x7FFFFFFF, slots);
    fBuilder.binary_op(BuilderOp::bitwise_and_n_ints, slots);
    return true;
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
            if (arg0.type().componentType().isFloat()) {
                // Perform abs(float) by masking off the sign bit.
                if (!this->pushExpression(arg0)) {
                    return unsupported();
                }
                return this->pushAbsFloatIntrinsic(arg0.type().slotCount());
            }
            // We have a dedicated op for abs(int).
            return this->pushIntrinsic(BuilderOp::abs_int, arg0);

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

        case IntrinsicKind::k_acos_IntrinsicKind:
            return this->pushIntrinsic(BuilderOp::acos_float, arg0);

        case IntrinsicKind::k_asin_IntrinsicKind:
            return this->pushIntrinsic(BuilderOp::asin_float, arg0);

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

        case IntrinsicKind::k_exp2_IntrinsicKind:
            return this->pushIntrinsic(BuilderOp::exp2_float, arg0);

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

        case IntrinsicKind::k_inverse_IntrinsicKind:
            SkASSERT(arg0.type().isMatrix());
            SkASSERT(arg0.type().rows() == arg0.type().columns());
            if (!this->pushExpression(arg0)) {
                return unsupported();
            }
            fBuilder.inverse_matrix(arg0.type().rows());
            return true;

        case IntrinsicKind::k_inversesqrt_IntrinsicKind:
            return this->pushIntrinsic(kInverseSqrtOps, arg0);

        case IntrinsicKind::k_length_IntrinsicKind:
            return this->pushExpression(arg0) &&
                   this->pushLengthIntrinsic(arg0.type().slotCount());

        case IntrinsicKind::k_log_IntrinsicKind:
            if (!this->pushExpression(arg0)) {
                return unsupported();
            }
            fBuilder.unary_op(BuilderOp::log_float, arg0.type().slotCount());
            return true;

        case IntrinsicKind::k_log2_IntrinsicKind:
            if (!this->pushExpression(arg0)) {
                return unsupported();
            }
            fBuilder.unary_op(BuilderOp::log2_float, arg0.type().slotCount());
            return true;

        case IntrinsicKind::k_normalize_IntrinsicKind: {
            // Implement normalize as `x / length(x)`. First, push the expression.
            if (!this->pushExpression(arg0)) {
                return unsupported();
            }
            int slotCount = arg0.type().slotCount();
            if (slotCount > 1) {
#if defined(SK_USE_RSQRT_IN_RP_NORMALIZE)
                // Instead of `x / sqrt(dot(x, x))`, we can get roughly the same result in less time
                // by computing `x * invsqrt(dot(x, x))`.
                fBuilder.push_clone(slotCount);
                fBuilder.push_clone(slotCount);
                fBuilder.dot_floats(slotCount);

                // Compute `vec(inversesqrt(dot(x, x)))`.
                fBuilder.unary_op(BuilderOp::invsqrt_float, 1);
                fBuilder.push_duplicates(slotCount - 1);

                // Return `x * vec(inversesqrt(dot(x, x)))`.
                return this->binaryOp(arg0.type(), kMultiplyOps);
#else
                // TODO: We can get roughly the same result in less time by using `invsqrt`, but
                // that leads to more variance across architectures, which Chromium layout tests do
                // not handle nicely.
                fBuilder.push_clone(slotCount);
                fBuilder.push_clone(slotCount);
                fBuilder.dot_floats(slotCount);

                // Compute `vec(sqrt(dot(x, x)))`.
                fBuilder.unary_op(BuilderOp::sqrt_float, 1);
                fBuilder.push_duplicates(slotCount - 1);

                // Return `x / vec(sqrt(dot(x, x)))`.
                return this->binaryOp(arg0.type(), kDivideOps);
#endif
            } else {
                // For single-slot normalization, we can simplify `sqrt(x * x)` into `abs(x)`.
                fBuilder.push_clone(slotCount);
                return this->pushAbsFloatIntrinsic(/*slots=*/1) &&
                       this->binaryOp(arg0.type(), kDivideOps);
            }
        }
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

        case IntrinsicKind::k_trunc_IntrinsicKind:
            // Implement trunc as `float(int(x))`, since float-to-int rounds toward zero.
            if (!this->pushExpression(arg0)) {
                return unsupported();
            }
            fBuilder.unary_op(BuilderOp::cast_to_int_from_float, arg0.type().slotCount());
            fBuilder.unary_op(BuilderOp::cast_to_float_from_int, arg0.type().slotCount());
            return true;

        case IntrinsicKind::k_fromLinearSrgb_IntrinsicKind:
        case IntrinsicKind::k_toLinearSrgb_IntrinsicKind:
            // The argument must be a half3.
            SkASSERT(arg0.type().matches(*fContext.fTypes.fHalf3));
            if (!this->pushExpression(arg0)) {
                return unsupported();
            }

            if (intrinsic == IntrinsicKind::k_fromLinearSrgb_IntrinsicKind) {
                fBuilder.invoke_from_linear_srgb();
            } else {
                fBuilder.invoke_to_linear_srgb();
            }
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
            subexpressionStack.pushClone(/*slots=*/3);

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
            subexpressionStack.pushClone(/*slots=*/3);

            fBuilder.swizzle(/*consumedSlots=*/3, {2, 0, 1});
            fBuilder.binary_op(BuilderOp::mul_n_floats, 3);

            subexpressionStack.enter();
            fBuilder.swizzle(/*consumedSlots=*/3, {1, 2, 0});
            fBuilder.binary_op(BuilderOp::mul_n_floats, 3);
            subexpressionStack.exit();

            // Migrate the result of the second subexpression (`arg0.zxy * arg1.yzx`) back onto the
            // main stack and subtract it from the first subexpression (`arg0.yzx * arg1.zxy`).
            subexpressionStack.pushClone(/*slots=*/3);
            fBuilder.binary_op(BuilderOp::sub_n_floats, 3);

            // Now that the calculation is complete, discard the subexpression on the next stack.
            subexpressionStack.enter();
            this->discardExpression(/*slots=*/3);
            subexpressionStack.exit();
            return true;
        }
        case IntrinsicKind::k_distance_IntrinsicKind:
            // Implement distance as `length(a - b)`.
            SkASSERT(arg0.type().slotCount() == arg1.type().slotCount());
            return this->pushBinaryExpression(arg0, OperatorKind::MINUS, arg1) &&
                   this->pushLengthIntrinsic(arg0.type().slotCount());

        case IntrinsicKind::k_dot_IntrinsicKind:
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

        case IntrinsicKind::k_mod_IntrinsicKind:
            SkASSERT(arg0.type().componentType().matches(arg1.type().componentType()));
            return this->pushIntrinsic(kModOps, arg0, arg1);

        case IntrinsicKind::k_pow_IntrinsicKind:
            SkASSERT(arg0.type().matches(arg1.type()));
            return this->pushIntrinsic(BuilderOp::pow_n_floats, arg0, arg1);

        case IntrinsicKind::k_reflect_IntrinsicKind: {
            // Implement reflect as `I - (N * dot(I,N) * 2)`.
            SkASSERT(arg0.type().matches(arg1.type()));
            SkASSERT(arg0.type().slotCount() == arg1.type().slotCount());
            SkASSERT(arg0.type().componentType().isFloat());
            int slotCount = arg0.type().slotCount();

            // Stack: I, N.
            if (!this->pushExpression(arg0) || !this->pushExpression(arg1)) {
                return unsupported();
            }
            // Stack: I, N, I, N.
            fBuilder.push_clone(2 * slotCount);
            // Stack: I, N, dot(I,N)
            fBuilder.dot_floats(slotCount);
            // Stack: I, N, dot(I,N), 2
            fBuilder.push_constant_f(2.0);
            // Stack: I, N, dot(I,N) * 2
            fBuilder.binary_op(BuilderOp::mul_n_floats, 1);
            // Stack: I, N * dot(I,N) * 2
            fBuilder.push_duplicates(slotCount - 1);
            fBuilder.binary_op(BuilderOp::mul_n_floats, slotCount);
            // Stack: I - (N * dot(I,N) * 2)
            fBuilder.binary_op(BuilderOp::sub_n_floats, slotCount);
            return true;
        }
        case IntrinsicKind::k_step_IntrinsicKind: {
            // Compute step as `float(lessThanEqual(edge, x))`. We convert from boolean 0/~0 to
            // floating point zero/one by using a bitwise-and against the bit-pattern of 1.0.
            SkASSERT(arg0.type().componentType().matches(arg1.type().componentType()));
            if (!this->pushVectorizedExpression(arg0, arg1.type()) || !this->pushExpression(arg1)) {
                return unsupported();
            }
            if (!this->binaryOp(arg1.type(), kLessThanEqualOps)) {
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

        case IntrinsicKind::k_faceforward_IntrinsicKind: {
            // Implement faceforward as `N ^ ((0 <= dot(I, NRef)) & 0x80000000)`.
            // In other words, flip the sign bit of N if `0 <= dot(I, NRef)`.
            SkASSERT(arg0.type().matches(arg1.type()));
            SkASSERT(arg0.type().matches(arg2.type()));
            int slotCount = arg0.type().slotCount();

            // Stack: N, 0, I, Nref
            if (!this->pushExpression(arg0)) {
                return unsupported();
            }
            fBuilder.push_constant_f(0.0);
            if (!this->pushExpression(arg1) || !this->pushExpression(arg2)) {
                return unsupported();
            }
            // Stack: N, 0, dot(I,NRef)
            fBuilder.dot_floats(slotCount);
            // Stack: N, (0 <= dot(I,NRef))
            fBuilder.binary_op(BuilderOp::cmple_n_floats, 1);
            // Stack: N, (0 <= dot(I,NRef)), 0x80000000
            fBuilder.push_constant_u(0x80000000);
            // Stack: N, (0 <= dot(I,NRef)) & 0x80000000)
            fBuilder.binary_op(BuilderOp::bitwise_and_n_ints, 1);
            // Stack: N, vec(0 <= dot(I,NRef)) & 0x80000000)
            fBuilder.push_duplicates(slotCount - 1);
            // Stack: N ^ vec((0 <= dot(I,NRef)) & 0x80000000)
            fBuilder.binary_op(BuilderOp::bitwise_xor_n_ints, slotCount);
            return true;
        }
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

        case IntrinsicKind::k_refract_IntrinsicKind: {
            // We always calculate refraction using vec4s, so we pad out unused N/I slots with zero.
            int padding = 4 - arg0.type().slotCount();
            if (!this->pushExpression(arg0)) {
                return unsupported();
            }
            fBuilder.push_zeros(padding);

            if (!this->pushExpression(arg1)) {
                return unsupported();
            }
            fBuilder.push_zeros(padding);

            // eta is always a scalar and doesn't need padding.
            if (!this->pushExpression(arg2)) {
                return unsupported();
            }
            fBuilder.refract_floats();

            // The result vector was returned as a vec4, so discard the extra columns.
            fBuilder.discard_stack(padding);
            return true;
        }
        case IntrinsicKind::k_smoothstep_IntrinsicKind:
            SkASSERT(arg0.type().componentType().isFloat());
            SkASSERT(arg1.type().matches(arg0.type()));
            SkASSERT(arg2.type().componentType().isFloat());

            if (!this->pushVectorizedExpression(arg0, arg2.type()) ||
                !this->pushVectorizedExpression(arg1, arg2.type()) ||
                !this->pushExpression(arg2)) {
                return unsupported();
            }
            fBuilder.ternary_op(BuilderOp::smoothstep_n_floats, arg2.type().slotCount());
            return true;

        default:
            break;
    }
    return unsupported();
}

bool Generator::pushLiteral(const Literal& l) {
    switch (l.type().numberKind()) {
        case Type::NumberKind::kFloat:
            fBuilder.push_constant_f(l.floatValue());
            return true;

        case Type::NumberKind::kSigned:
            fBuilder.push_constant_i(l.intValue());
            return true;

        case Type::NumberKind::kUnsigned:
            fBuilder.push_constant_u(l.intValue());
            return true;

        case Type::NumberKind::kBoolean:
            fBuilder.push_constant_i(l.boolValue() ? ~0 : 0);
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
            fBuilder.push_constant_u(~0, expr.type().slotCount());
            fBuilder.binary_op(BuilderOp::bitwise_xor_n_ints, expr.type().slotCount());
            return true;

        case OperatorKind::MINUS: {
            if (!this->pushExpression(expr)) {
                return unsupported();
            }
            if (expr.type().componentType().isFloat()) {
                // Handle float negation as an integer `x ^ 0x80000000`. This toggles the sign bit.
                fBuilder.push_constant_u(0x80000000, expr.type().slotCount());
                fBuilder.binary_op(BuilderOp::bitwise_xor_n_ints, expr.type().slotCount());
            } else {
                // Handle integer negation as a componentwise `expr * -1`.
                fBuilder.push_constant_i(-1, expr.type().slotCount());
                fBuilder.binary_op(BuilderOp::mul_n_ints, expr.type().slotCount());
            }
            return true;
        }
        case OperatorKind::PLUSPLUS: {
            // Rewrite as `expr += 1`.
            Literal oneLiteral{Position{}, 1.0, &expr.type().componentType()};
            return this->pushBinaryExpression(expr, OperatorKind::PLUSEQ, oneLiteral);
        }
        case OperatorKind::MINUSMINUS: {
            // Rewrite as `expr += -1`.
            Literal minusOneLiteral{expr.fPosition, -1.0, &expr.type().componentType()};
            return this->pushBinaryExpression(expr, OperatorKind::PLUSEQ, minusOneLiteral);
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
            fBuilder.branch_if_no_lanes_active(cleanupLabelID);
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

        // Switch back to the test-expression stack and apply the inverted test condition.
        testStack.enter();
        fBuilder.merge_inv_condition_mask();
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

bool Generator::pushVariableReference(const VariableReference& var) {
    // If we are pushing a constant-value variable, push the value directly; literal values are more
    // amenable to optimization.
    if (var.type().isScalar() || var.type().isVector()) {
        if (const Expression* expr = ConstantFolder::GetConstantValueOrNull(var)) {
            return this->pushExpression(*expr);
        }
        if (fImmutableVariables.contains(var.variable())) {
            return this->pushExpression(*var.variable()->initialValue());
        }
    }
    return this->pushVariableReferencePartial(var, SlotRange{0, (int)var.type().slotCount()});
}

bool Generator::pushVariableReferencePartial(const VariableReference& v, SlotRange subset) {
    const Variable& var = *v.variable();
    SlotRange r;
    if (IsUniform(var)) {
        // Push a uniform.
        r = this->getUniformSlots(var);
        SkASSERT(r.count == (int)var.type().slotCount());
        r.index += subset.index;
        r.count = subset.count;
        fBuilder.push_uniform(r);
    } else if (fImmutableVariables.contains(&var)) {
        // If we only need a single slot, we can push a constant. This saves a lookup, and can
        // occasionally permit the use of an immediate-mode op.
        if (subset.count == 1) {
            const Expression& expr = *v.variable()->initialValue();
            std::optional<ImmutableBits> bits = this->getImmutableBitsForSlot(expr, subset.index);
            if (bits.has_value()) {
                fBuilder.push_constant_i(*bits);
                return true;
            }
        }
        // Push the immutable slot range.
        r = this->getImmutableSlots(var);
        SkASSERT(r.count == (int)var.type().slotCount());
        r.index += subset.index;
        r.count = subset.count;
        fBuilder.push_immutable(r);
    } else {
        // Push the variable.
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

        if (fWriteTraceOps) {
            // The Raster Pipeline blitter generates centered pixel coordinates. (0.5, 1.5, 2.5,
            // etc.) Add 0.5 to the requested trace coordinate to match this, then compare against
            // src.rg, which contains the shader's coordinates. We keep this result in a dedicated
            // trace-mask stack.
            fTraceMask.emplace(this);
            fTraceMask->enter();
            fBuilder.push_device_xy01();
            fBuilder.discard_stack(2);
            fBuilder.push_constant_f(fDebugTrace->fTraceCoord.fX + 0.5f);
            fBuilder.push_constant_f(fDebugTrace->fTraceCoord.fY + 0.5f);
            fBuilder.binary_op(BuilderOp::cmpeq_n_floats, 2);
            fBuilder.binary_op(BuilderOp::bitwise_and_n_ints, 1);
            fTraceMask->exit();

            // Assemble a position-to-line-number mapping for the debugger.
            this->calculateLineOffsets();
        }
    }

    // Assign slots to the parameters of main; copy src and dst into those slots as appropriate.
    const SkSL::Variable* mainCoordsParam = function.declaration().getMainCoordsParameter();
    const SkSL::Variable* mainInputColorParam = function.declaration().getMainInputColorParameter();
    const SkSL::Variable* mainDestColorParam = function.declaration().getMainDestColorParameter();

    for (const SkSL::Variable* param : function.declaration().parameters()) {
        if (param == mainCoordsParam) {
            // Coordinates are passed via RG.
            SlotRange fragCoord = this->getVariableSlots(*param);
            SkASSERT(fragCoord.count == 2);
            fBuilder.store_src_rg(fragCoord);
        } else if (param == mainInputColorParam) {
            // Input colors are passed via RGBA.
            SlotRange srcColor = this->getVariableSlots(*param);
            SkASSERT(srcColor.count == 4);
            fBuilder.store_src(srcColor);
        } else if (param == mainDestColorParam) {
            // Dest colors are passed via dRGBA.
            SlotRange destColor = this->getVariableSlots(*param);
            SkASSERT(destColor.count == 4);
            fBuilder.store_dst(destColor);
        } else {
            SkDEBUGFAIL("Invalid parameter to main()");
            return unsupported();
        }
    }

    // Initialize the program.
    fBuilder.init_lane_masks();

    // Emit global variables.
    if (!this->writeGlobals()) {
        return unsupported();
    }

    // Invoke main().
    std::optional<SlotRange> mainResult = this->writeFunction(function, function, /*arguments=*/{});
    if (!mainResult.has_value()) {
        return unsupported();
    }

    // Move the result of main() from slots into RGBA.
    SkASSERT(mainResult->count == 4);
    if (this->needsFunctionResultSlots(fCurrentFunction)) {
        fBuilder.load_src(*mainResult);
    } else {
        fBuilder.pop_src_rgba();
    }

    // Discard the trace mask.
    if (fTraceMask.has_value()) {
        fTraceMask->enter();
        fBuilder.discard_stack(1);
        fTraceMask->exit();
    }

    return true;
}

std::unique_ptr<RP::Program> Generator::finish() {
    return fBuilder.finish(fProgramSlots.slotCount(),
                           fUniformSlots.slotCount(),
                           fImmutableSlots.slotCount(),
                           fDebugTrace);
}

}  // namespace RP

std::unique_ptr<RP::Program> MakeRasterPipelineProgram(const SkSL::Program& program,
                                                       const FunctionDefinition& function,
                                                       DebugTracePriv* debugTrace,
                                                       bool writeTraceOps) {
    RP::Generator generator(program, debugTrace, writeTraceOps);
    if (!generator.writeProgram(function)) {
        return nullptr;
    }
    return generator.finish();
}

}  // namespace SkSL
