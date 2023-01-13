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
#include "src/sksl/SkSLIntrinsicList.h"
#include "src/sksl/codegen/SkSLRasterPipelineBuilder.h"
#include "src/sksl/codegen/SkSLRasterPipelineCodeGenerator.h"
#include "src/sksl/ir/SkSLBinaryExpression.h"
#include "src/sksl/ir/SkSLBlock.h"
#include "src/sksl/ir/SkSLBreakStatement.h"
#include "src/sksl/ir/SkSLConstructor.h"
#include "src/sksl/ir/SkSLConstructorCompound.h"
#include "src/sksl/ir/SkSLConstructorDiagonalMatrix.h"
#include "src/sksl/ir/SkSLConstructorMatrixResize.h"
#include "src/sksl/ir/SkSLConstructorSplat.h"
#include "src/sksl/ir/SkSLContinueStatement.h"
#include "src/sksl/ir/SkSLDoStatement.h"
#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLExpressionStatement.h"
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
#include "src/sksl/ir/SkSLSwizzle.h"
#include "src/sksl/ir/SkSLTernaryExpression.h"
#include "src/sksl/ir/SkSLType.h"
#include "src/sksl/ir/SkSLVarDeclarations.h"
#include "src/sksl/ir/SkSLVariable.h"
#include "src/sksl/ir/SkSLVariableReference.h"
#include "src/sksl/tracing/SkRPDebugTrace.h"
#include "src/sksl/tracing/SkSLDebugInfo.h"

#include <cstddef>
#include <cstdint>
#include <float.h>
#include <numeric>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace SkSL {
namespace RP {

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

    /** Implements low-level slot creation; slots will not be known to the debugger. */
    SlotRange createSlots(int slots);

    /** Creates slots associated with an SkSL variable or return value. */
    SlotRange createSlots(std::string name,
                          const Type& type,
                          Position pos,
                          bool isFunctionReturnValue);

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
    SkTHashMap<const IRNode*, SlotRange> fSlotMap;
    int fSlotCount = 0;
    std::vector<SlotDebugInfo>* fSlotDebugInfo;
};

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

    /** The Builder stitches our instructions together into Raster Pipeline code. */
    Builder* builder() { return &fBuilder; }

    /** Appends a statement to the program. */
    [[nodiscard]] bool writeStatement(const Statement& s);
    [[nodiscard]] bool writeBlock(const Block& b);
    [[nodiscard]] bool writeBreakStatement(const BreakStatement& b);
    [[nodiscard]] bool writeContinueStatement(const ContinueStatement& b);
    [[nodiscard]] bool writeDoStatement(const DoStatement& d);
    [[nodiscard]] bool writeExpressionStatement(const ExpressionStatement& e);
    [[nodiscard]] bool writeForStatement(const ForStatement& f);
    [[nodiscard]] bool writeGlobals();
    [[nodiscard]] bool writeIfStatement(const IfStatement& i);
    [[nodiscard]] bool writeReturnStatement(const ReturnStatement& r);
    [[nodiscard]] bool writeVarDeclaration(const VarDeclaration& v);

    /** Pushes an expression to the value stack. */
    [[nodiscard]] bool pushBinaryExpression(const BinaryExpression& e);
    [[nodiscard]] bool pushBinaryExpression(const Expression& left,
                                            Operator op,
                                            const Expression& right);
    [[nodiscard]] bool pushConstructorCast(const AnyConstructor& c);
    [[nodiscard]] bool pushConstructorCompound(const ConstructorCompound& c);
    [[nodiscard]] bool pushConstructorDiagonalMatrix(const ConstructorDiagonalMatrix& c);
    [[nodiscard]] bool pushConstructorMatrixResize(const ConstructorMatrixResize& c);
    [[nodiscard]] bool pushConstructorSplat(const ConstructorSplat& c);
    [[nodiscard]] bool pushExpression(const Expression& e, bool usesResult = true);
    [[nodiscard]] bool pushFunctionCall(const FunctionCall& e);
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
    [[nodiscard]] bool pushVectorizedExpression(const Expression& expr, const Type& vectorType);
    [[nodiscard]] bool pushVariableReferencePartial(const VariableReference& v, SlotRange subset);

    void foldWithMultiOp(BuilderOp op, int elements);
    void nextTempStack() {
        fBuilder.set_current_stack(++fCurrentTempStack);
    }
    void previousTempStack() {
        fBuilder.set_current_stack(--fCurrentTempStack);
    }
    void pushCloneFromNextTempStack(int slots, int offsetFromStackTop = 0) {
        fBuilder.push_clone_from_stack(slots, fCurrentTempStack + 1, offsetFromStackTop);
    }
    void pushCloneFromPreviousTempStack(int slots, int offsetFromStackTop = 0) {
        fBuilder.push_clone_from_stack(slots, fCurrentTempStack - 1, offsetFromStackTop);
    }
    BuilderOp getTypedOp(const SkSL::Type& type, const TypedOps& ops) const;

    std::string makeMaskName(const char* name) {
        return SkSL::String::printf("[%s %d]", name, fTempNameIndex++);
    }

    static bool IsUniform(const Variable& var) {
       return var.modifiers().fFlags & Modifiers::kUniform_Flag;
    }

private:
    const SkSL::Program& fProgram;
    Builder fBuilder;
    SkRPDebugTrace* fDebugTrace = nullptr;

    SlotManager fProgramSlots;
    SlotManager fUniformSlots;

    SkTArray<SlotRange> fFunctionStack;
    SlotRange fCurrentContinueMask;
    int fCurrentTempStack = 0;
    int fTempNameIndex = 0;

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

struct LValue {
    virtual ~LValue() = default;

    /**
     * Returns an LValue for the passed-in expression; if the expression isn't supported as an
     * LValue, returns nullptr.
     */
    static std::unique_ptr<LValue> Make(const Expression& e);

    /** Copies the top-of-stack value into this lvalue, without discarding it from the stack. */
    void store(Generator* gen);

    /** Pushes the lvalue onto the top-of-stack. */
    void push(Generator* gen);

    /**
     * Returns the value slots associated with this LValue. For instance, a plain four-slot Variable
     * will have monotonically increasing slots like {5,6,7,8}.
     */
    struct SlotMap {
        SkTArray<int> slots;  // the destination slots
    };
    virtual SlotMap getSlotMap(Generator* gen) const = 0;

    /** Returns true if this lvalue actually refers to a uniform. */
    virtual bool isUniform() const = 0;

    /** Converts a SlotMap into a list of ranges of consecutive slots. */
    static SkTArray<SlotRange> CoalesceSlotRanges(const SlotMap& slotMap);
};

struct VariableLValue final : public LValue {
    VariableLValue(const Variable* v) : fVariable(v) {}

    SlotMap getSlotMap(Generator* gen) const override {
        // Map every slot in the variable, in consecutive order, e.g. a half4 at slot 5 = {5,6,7,8}.
        SlotMap out;
        SlotRange range = this->isUniform() ? gen->getUniformSlots(*fVariable)
                                            : gen->getVariableSlots(*fVariable);
        out.slots.resize(range.count);
        std::iota(out.slots.begin(), out.slots.end(), range.index);
        return out;
    }

    bool isUniform() const override {
        return Generator::IsUniform(*fVariable);
    }

    const Variable* fVariable;
};

struct SwizzleLValue final : public LValue {
    SwizzleLValue(std::unique_ptr<LValue> p, const ComponentArray& c)
            : fParent(std::move(p))
            , fComponents(c) {}

    SlotMap getSlotMap(Generator* gen) const override {
        // Get slots from the parent expression.
        SlotMap in = fParent->getSlotMap(gen);

        // Rearrange the slots based to honor the swizzle components.
        SlotMap out;
        out.slots.resize(fComponents.size());
        for (int index = 0; index < fComponents.size(); ++index) {
            SkASSERT(fComponents[index] < in.slots.size());
            out.slots[index] = in.slots[fComponents[index]];
        }

        return out;
    }

    bool isUniform() const override {
        return fParent->isUniform();
    }

    std::unique_ptr<LValue> fParent;
    const ComponentArray& fComponents;
};

struct IndexLValue final : public LValue {
    IndexLValue(std::unique_ptr<LValue> p, const Expression& i, const Type& ti)
            : fParent(std::move(p))
            , fIndexExpr(i)
            , fIndexedType(ti) {}

    SlotMap getSlotMap(Generator* gen) const override {
        // Get slots from the parent expression.
        SlotMap in = fParent->getSlotMap(gen);

        // Take a subset of the parent's slots.
        int numElements = fIndexedType.slotCount();

        SlotMap out;
        out.slots.resize(numElements);
        // TODO(skia:13676): support non-constant indices
        int startingIndex = numElements * fIndexExpr.as<Literal>().intValue();
        for (int count = 0; count < numElements; ++count) {
            out.slots[count] = in.slots[startingIndex + count];
        }

        return out;
    }

    bool isUniform() const override {
        return fParent->isUniform();
    }

    std::unique_ptr<LValue> fParent;
    const Expression& fIndexExpr;
    const Type& fIndexedType;
};

std::unique_ptr<LValue> LValue::Make(const Expression& e) {
    if (e.is<VariableReference>()) {
        return std::make_unique<VariableLValue>(e.as<VariableReference>().variable());
    }
    if (e.is<Swizzle>()) {
        if (std::unique_ptr<LValue> base = LValue::Make(*e.as<Swizzle>().base())) {
            return std::make_unique<SwizzleLValue>(std::move(base), e.as<Swizzle>().components());
        }
    }
    if (e.is<IndexExpression>()) {
        const IndexExpression& indexExpr = e.as<IndexExpression>();

        // TODO(skia:13676): support non-constant indices
        if (indexExpr.index()->is<Literal>()) {
            if (std::unique_ptr<LValue> base = LValue::Make(*indexExpr.base())) {
                return std::make_unique<IndexLValue>(std::move(base),
                                                     *indexExpr.index(),
                                                     indexExpr.type());
            }
        }
    }
    // TODO(skia:13676): add support for other kinds of lvalues
    return nullptr;
}

SkTArray<SlotRange> LValue::CoalesceSlotRanges(const SlotMap& slotMap) {
    SkTArray<SlotRange> ranges;
    ranges.push_back({slotMap.slots.front(), 1});

    for (int index = 1; index < slotMap.slots.size(); ++index) {
        Slot dst = slotMap.slots[index];
        if (dst == ranges.back().index + ranges.back().count) {
            ++ranges.back().count;
        } else {
            ranges.push_back({dst, 1});
        }
    }

    return ranges;
}

void LValue::store(Generator* gen) {
    SkASSERT(!this->isUniform());

    SlotMap out = this->getSlotMap(gen);

    if (!out.slots.empty()) {
        // Coalesce our list of slots into ranges of consecutive slots.
        SkTArray<SlotRange> ranges = CoalesceSlotRanges(out);

        // Copy our coalesced slot ranges from the stack.
        int offsetFromStackTop = out.slots.size();
        for (const SlotRange& r : ranges) {
            gen->builder()->copy_stack_to_slots(r, offsetFromStackTop);
            offsetFromStackTop -= r.count;
        }
        SkASSERT(offsetFromStackTop == 0);
    }
}

void LValue::push(Generator* gen) {
    SlotMap out = this->getSlotMap(gen);

    if (!out.slots.empty()) {
        // Coalesce our list of slots into ranges of consecutive slots.
        SkTArray<SlotRange> ranges = CoalesceSlotRanges(out);

        // Push our coalesced slot ranges onto the stack.
        bool isUniform = this->isUniform();
        for (const SlotRange& r : ranges) {
            if (isUniform) {
                gen->builder()->push_uniform(r);
            } else {
                gen->builder()->push_slots(r);
            }
        }
    }
}

static bool unsupported() {
    // If MakeRasterPipelineProgram returns false, set a breakpoint here for more information.
    return false;
}

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

SlotRange SlotManager::createSlots(int slots) {
    SlotRange range = {fSlotCount, slots};
    fSlotCount += slots;
    return range;
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

    return this->createSlots(nslots);
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

std::optional<SlotRange> Generator::writeFunction(const IRNode& callSite,
                                                  const FunctionDefinition& function) {
    [[maybe_unused]] int funcIndex = -1;
    if (fDebugTrace) {
        funcIndex = this->getFunctionDebugInfo(function.declaration());
        SkASSERT(funcIndex >= 0);
        // TODO(debugger): add trace for function-enter
    }

    fFunctionStack.push_back(this->getFunctionSlots(callSite, function.declaration()));

    if (!this->writeStatement(*function.body())) {
        return std::nullopt;
    }

    SlotRange functionResult = fFunctionStack.back();
    fFunctionStack.pop_back();

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
                // TODO(skia:13676): handle child effects
                return unsupported();
            }

            // Opaque types include child processors and GL objects (samplers, textures, etc).
            // Of those, only child processors are legal variables.
            SkASSERT(!var->type().isVoid());
            SkASSERT(!var->type().isOpaque());

            // builtin variables are system-defined, with special semantics. The only builtin
            // variable exposed to runtime effects is sk_FragCoord.
            if (int builtin = var->modifiers().fLayout.fBuiltin; builtin >= 0) {
                switch (builtin) {
                    case SK_FRAGCOORD_BUILTIN:
                        // TODO: populate slots with device coordinates xy01
                        return unsupported();

                    default:
                        SkDEBUGFAILF("Unsupported builtin %d", builtin);
                        return unsupported();
                }
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
    fBuilder.push_loop_mask();

    // Create a dedicated slot for continue-mask storage.
    SlotRange previousContinueMask = fCurrentContinueMask;
    fCurrentContinueMask = fProgramSlots.createSlots(this->makeMaskName("do-loop continue mask"),
                                                     *fProgram.fContext->fTypes.fBool,
                                                     Position{},
                                                     /*isFunctionReturnValue=*/false);
    // Write the do-loop body.
    int labelID = fBuilder.nextLabelID();
    fBuilder.label(labelID);

    fBuilder.zero_slots_unmasked(fCurrentContinueMask);
    if (!this->writeStatement(*d.statement())) {
        return false;
    }
    fBuilder.reenable_loop_mask(fCurrentContinueMask);

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

    // Restore the loop and continue masks.
    fBuilder.pop_loop_mask();
    fCurrentContinueMask = previousContinueMask;

    return true;
}

bool Generator::writeForStatement(const ForStatement& f) {
    // If we've determined that the loop does not run, omit its code entirely.
    if (f.unrollInfo() && f.unrollInfo()->fCount == 0) {
        return true;
    }

    // Run the loop initializer.
    if (f.initializer() && !this->writeStatement(*f.initializer())) {
        return unsupported();
    }

    // Save off the original loop mask.
    fBuilder.push_loop_mask();

    // Create a dedicated slot for continue-mask storage.
    SlotRange previousContinueMask = fCurrentContinueMask;
    fCurrentContinueMask = fProgramSlots.createSlots(this->makeMaskName("for-loop continue mask"),
                                                     *fProgram.fContext->fTypes.fBool,
                                                     Position{},
                                                     /*isFunctionReturnValue=*/false);
    int loopTestID = fBuilder.nextLabelID();
    int loopBodyID = fBuilder.nextLabelID();

    // Jump down to the loop test so we can fall out of the loop immediately if it's zero-iteration.
    fBuilder.jump(loopTestID);

    // Write the for-loop body.
    fBuilder.label(loopBodyID);
    fBuilder.zero_slots_unmasked(fCurrentContinueMask);
    if (!this->writeStatement(*f.statement())) {
        return unsupported();
    }
    fBuilder.reenable_loop_mask(fCurrentContinueMask);

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

    // Restore the loop and continue masks.
    fBuilder.pop_loop_mask();
    fCurrentContinueMask = previousContinueMask;

    return true;
}


bool Generator::writeExpressionStatement(const ExpressionStatement& e) {
    if (!this->pushExpression(*e.expression(), /*usesResult=*/false)) {
        return unsupported();
    }
    this->discardExpression(e.expression()->type().slotCount());
    return true;
}

bool Generator::writeIfStatement(const IfStatement& i) {
    // Save the current condition-mask.
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
    return true;
}

bool Generator::writeReturnStatement(const ReturnStatement& r) {
    if (r.expression()) {
        if (!this->pushExpression(*r.expression())) {
            return unsupported();
        }
        this->popToSlotRange(fFunctionStack.back());
    }
    fBuilder.mask_off_return_mask();
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

        case Expression::Kind::kConstructorCompound:
            return this->pushConstructorCompound(e.as<ConstructorCompound>());

        case Expression::Kind::kConstructorCompoundCast:
        case Expression::Kind::kConstructorScalarCast:
            return this->pushConstructorCast(e.asAnyConstructor());

        case Expression::Kind::kConstructorDiagonalMatrix:
            return this->pushConstructorDiagonalMatrix(e.as<ConstructorDiagonalMatrix>());

        case Expression::Kind::kConstructorMatrixResize:
            return this->pushConstructorMatrixResize(e.as<ConstructorMatrixResize>());

        case Expression::Kind::kConstructorSplat:
            return this->pushConstructorSplat(e.as<ConstructorSplat>());

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
        case Type::NumberKind::kFloat:    return ops.fFloatOp;    break;
        case Type::NumberKind::kSigned:   return ops.fSignedOp;   break;
        case Type::NumberKind::kUnsigned: return ops.fUnsignedOp; break;
        case Type::NumberKind::kBoolean:  return ops.fBooleanOp;  break;
        default:                          SkUNREACHABLE;
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

bool Generator::pushBinaryExpression(const BinaryExpression& e) {
    return this->pushBinaryExpression(*e.left(), e.getOperator(), *e.right());
}

bool Generator::pushBinaryExpression(const Expression& left, Operator op, const Expression& right) {
    // Rewrite greater-than ops as their less-than equivalents.
    if (op.kind() == OperatorKind::GT) {
        return this->pushBinaryExpression(right, OperatorKind::LT, left);
    }
    if (op.kind() == OperatorKind::GTEQ) {
        return this->pushBinaryExpression(right, OperatorKind::LTEQ, left);
    }

    // Emit comma expressions.
    if (op.kind() == OperatorKind::COMMA) {
        if (Analysis::HasSideEffects(left)) {
            if (!this->pushExpression(left, /*usesResult=*/false)) {
                return unsupported();
            }
            this->discardExpression(left.type().slotCount());
        }
        return this->pushExpression(right);
    }

    // Handle binary expressions with mismatched types.
    const Type& type = left.type();
    if (!type.matches(right.type())) {
        if (type.componentType().numberKind() != right.type().componentType().numberKind()) {
            return unsupported();
        }

        if (left.type().isScalar() && right.type().isVector()) {
            // SxV becomes VxV via an implicit splat.
            ConstructorSplat leftAsVector(left.fPosition, right.type(), left.clone());
            return this->pushBinaryExpression(leftAsVector, op, right);
        }

        if (left.type().isVector() && right.type().isScalar()) {
            // VxS becomes VxV via an implicit splat.
            ConstructorSplat rightAsVector(right.fPosition, left.type(), right.clone());
            return this->pushBinaryExpression(left, op, rightAsVector);
        }

        // TODO: add support for other non-matching types (matrix-vector and matrix-matrix ops)
        return unsupported();
    }

    // If this is an assignment...
    std::unique_ptr<LValue> lvalue;
    if (op.isAssignment()) {
        // ... turn the left side into an lvalue.
        lvalue = LValue::Make(left);
        if (!lvalue) {
            return unsupported();
        }

        // Handle simple assignment (`var = expr`).
        if (op.kind() == OperatorKind::EQ) {
            if (!this->pushExpression(right)) {
                return unsupported();
            }
            lvalue->store(this);
            return true;
        }

        // Strip off the assignment from the op (turning += into +).
        op = op.removeAssignment();
    }

    // Matrix * matrix is a matrix multiply, NOT a componentwise op.
    if (op.kind() == OperatorKind::STAR && type.isMatrix() && right.type().isMatrix()) {
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

    // Push the left-expression on the stack.
    if (lvalue) {
        lvalue->push(this);
    } else {
        if (!this->pushExpression(left)) {
            return unsupported();
        }
    }

    // Push the right-expression on the stack.
    if (!this->pushExpression(right)) {
        return unsupported();
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
            // equal(x,y) returns a vector; use & to fold into a scalar.
            this->foldWithMultiOp(BuilderOp::bitwise_and_n_ints, type.slotCount());
            break;

        case OperatorKind::NEQ:
            if (!this->binaryOp(type, kNotEqualOps)) {
                return unsupported();
            }
            // notEqual(x,y) returns a vector; use | to fold into a scalar.
            this->foldWithMultiOp(BuilderOp::bitwise_or_n_ints, type.slotCount());
            break;

        case OperatorKind::LOGICALAND:
            // We verified above that the RHS does not have side effects, so we don't need to worry
            // about short-circuiting side effects.
            fBuilder.binary_op(BuilderOp::bitwise_and_n_ints, /*slots=*/1);
            break;

        case OperatorKind::LOGICALOR:
            // We verified above that the RHS does not have side effects.
            fBuilder.binary_op(BuilderOp::bitwise_and_n_ints, /*slots=*/1);
            break;

        default:
            return unsupported();
    }

    // If we have an lvalue, we need to write the result back into it.
    if (lvalue) {
        lvalue->store(this);
    }

    return true;
}

bool Generator::pushConstructorCompound(const ConstructorCompound& c) {
    for (const std::unique_ptr<Expression> &arg : c.arguments()) {
        if (!this->pushExpression(*arg)) {
            return unsupported();
        }
    }
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

bool Generator::pushFunctionCall(const FunctionCall& c) {
    if (c.function().isIntrinsic()) {
        return this->pushIntrinsic(c);
    }

    // Skip over the function body entirely if there are no active lanes.
    // (If the function call was trivial, it would likely have been inlined in the frontend, so this
    // is likely to save a significant amount of work if the lanes are all dead.)
    int skipLabelID = fBuilder.nextLabelID();
    fBuilder.branch_if_no_active_lanes(skipLabelID);

    // Save off the return mask.
    fBuilder.push_return_mask();

    // Write all the arguments into their parameter's variable slots. Because we never allow
    // recursion, we don't need to worry about overwriting any existing values in those slots.
    // (In fact, we don't even need to apply the write mask.)
    for (int index = 0; index < c.arguments().size(); ++index) {
        const Expression& arg = *c.arguments()[index];
        const Variable& param = *c.function().parameters()[index];

        if (param.modifiers().fFlags & Modifiers::kOut_Flag) {
            // TODO(skia:13676): out and inout parameters
            return unsupported();
        }

        if (!this->pushExpression(arg)) {
            return unsupported();
        }

        this->popToSlotRangeUnmasked(this->getVariableSlots(param));
    }

    // Emit the function body.
    std::optional<SlotRange> r = this->writeFunction(c, *c.function().definition());
    if (!r.has_value()) {
        return unsupported();
    }

    // Restore the original return mask.
    fBuilder.pop_return_mask();

    // Copy the function result from its slots onto the stack.
    fBuilder.push_slots(*r);
    fBuilder.label(skipLabelID);
    return true;
}

bool Generator::pushIndexExpression(const IndexExpression& i) {
    std::unique_ptr<LValue> lvalue = LValue::Make(i);
    if (!lvalue) {
        return unsupported();
    }
    lvalue->push(this);
    return true;
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

bool Generator::pushIntrinsic(IntrinsicKind intrinsic, const Expression& arg0) {
    switch (intrinsic) {
        case IntrinsicKind::k_abs_IntrinsicKind:
            if (!this->pushExpression(arg0)) {
                return unsupported();
            }
            return this->unaryOp(arg0.type(), kAbsOps);

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

        case IntrinsicKind::k_ceil_IntrinsicKind:
            if (!this->pushExpression(arg0)) {
                return unsupported();
            }
            fBuilder.unary_op(BuilderOp::ceil_float, arg0.type().slotCount());
            return true;

        case IntrinsicKind::k_floatBitsToInt_IntrinsicKind:
        case IntrinsicKind::k_floatBitsToUint_IntrinsicKind:
        case IntrinsicKind::k_intBitsToFloat_IntrinsicKind:
        case IntrinsicKind::k_uintBitsToFloat_IntrinsicKind:
            if (!this->pushExpression(arg0)) {
                return unsupported();
            }
            return true;

        case IntrinsicKind::k_floor_IntrinsicKind:
            if (!this->pushExpression(arg0)) {
                return unsupported();
            }
            fBuilder.unary_op(BuilderOp::floor_float, arg0.type().slotCount());
            return true;

        case IntrinsicKind::k_fract_IntrinsicKind:
            // Implement fract as `x - floor(x)`.
            if (!this->pushExpression(arg0)) {
                return unsupported();
            }
            fBuilder.push_clone(arg0.type().slotCount());
            fBuilder.unary_op(BuilderOp::floor_float, arg0.type().slotCount());
            return this->binaryOp(arg0.type(), kSubtractOps);

        case IntrinsicKind::k_not_IntrinsicKind:
            return this->pushPrefixExpression(OperatorKind::LOGICALNOT, arg0);

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

bool Generator::pushIntrinsic(IntrinsicKind intrinsic,
                              const Expression& arg0,
                              const Expression& arg1) {
    switch (intrinsic) {
        case IntrinsicKind::k_cross_IntrinsicKind:
            // Implement cross as `arg0.yzx * arg1.zxy - arg0.zxy * arg1.yzx`. We use two stacks so
            // that each subexpression can be multiplied separately.
            SkASSERT(arg0.type().matches(arg1.type()));
            SkASSERT(arg0.type().slotCount() == 3);
            SkASSERT(arg1.type().slotCount() == 3);

            // Push `arg0.yzx` onto this stack and `arg0.zxy` onto the next stack.
            if (!this->pushExpression(arg0)) {
                return unsupported();
            }

            this->nextTempStack();
            this->pushCloneFromPreviousTempStack(3);
            fBuilder.swizzle(/*consumedSlots=*/3, {2, 0, 1});
            this->previousTempStack();

            fBuilder.swizzle(/*consumedSlots=*/3, {1, 2, 0});

            // Push `arg1.zxy` onto this stack and `arg1.yzx` onto the next stack. Perform the
            // multiply on each subexpression (`arg0.yzx * arg1.zxy` on the first stack, and
            // `arg0.zxy * arg1.yzx` on the next).
            if (!this->pushExpression(arg1)) {
                return unsupported();
            }

            this->nextTempStack();
            this->pushCloneFromPreviousTempStack(3);
            fBuilder.swizzle(/*consumedSlots=*/3, {1, 2, 0});
            fBuilder.binary_op(BuilderOp::mul_n_floats, 3);
            this->previousTempStack();

            fBuilder.swizzle(/*consumedSlots=*/3, {2, 0, 1});
            fBuilder.binary_op(BuilderOp::mul_n_floats, 3);

            // Migrate the result of the second subexpression (`arg0.zxy * arg1.yzx`) back onto the
            // main stack and subtract it from the first subexpression (`arg0.yzx * arg1.zxy`).
            this->pushCloneFromNextTempStack(3);
            fBuilder.binary_op(BuilderOp::sub_n_floats, 3);

            // Now that the calculation is complete, discard the subexpression on the next stack.
            this->nextTempStack();
            this->discardExpression(/*slots=*/3);
            this->previousTempStack();
            return true;

        case IntrinsicKind::k_dot_IntrinsicKind:
            // Implement dot as `a*b`, followed by folding via addition.
            SkASSERT(arg0.type().matches(arg1.type()));
            if (!this->pushExpression(arg0) || !this->pushExpression(arg1)) {
                return unsupported();
            }
            fBuilder.binary_op(BuilderOp::mul_n_floats, arg0.type().slotCount());
            this->foldWithMultiOp(BuilderOp::add_n_floats, arg0.type().slotCount());
            return true;

        case IntrinsicKind::k_equal_IntrinsicKind:
            SkASSERT(arg0.type().matches(arg1.type()));
            if (!this->pushExpression(arg0) || !this->pushExpression(arg1)) {
                return unsupported();
            }
            return this->binaryOp(arg0.type(), kEqualOps);

        case IntrinsicKind::k_notEqual_IntrinsicKind:
            SkASSERT(arg0.type().matches(arg1.type()));
            if (!this->pushExpression(arg0) || !this->pushExpression(arg1)) {
                return unsupported();
            }
            return this->binaryOp(arg0.type(), kNotEqualOps);

        case IntrinsicKind::k_lessThan_IntrinsicKind:
            SkASSERT(arg0.type().matches(arg1.type()));
            if (!this->pushExpression(arg0) || !this->pushExpression(arg1)) {
                return unsupported();
            }
            return this->binaryOp(arg0.type(), kLessThanOps);

        case IntrinsicKind::k_greaterThan_IntrinsicKind:
            SkASSERT(arg0.type().matches(arg1.type()));
            if (!this->pushExpression(arg1) || !this->pushExpression(arg0)) {
                return unsupported();
            }
            return this->binaryOp(arg0.type(), kLessThanOps);

        case IntrinsicKind::k_lessThanEqual_IntrinsicKind:
            SkASSERT(arg0.type().matches(arg1.type()));
            if (!this->pushExpression(arg0) || !this->pushExpression(arg1)) {
                return unsupported();
            }
            return this->binaryOp(arg0.type(), kLessThanEqualOps);

        case IntrinsicKind::k_greaterThanEqual_IntrinsicKind:
            SkASSERT(arg0.type().matches(arg1.type()));
            if (!this->pushExpression(arg1) || !this->pushExpression(arg0)) {
                return unsupported();
            }
            return this->binaryOp(arg0.type(), kLessThanEqualOps);

        case IntrinsicKind::k_min_IntrinsicKind:
            SkASSERT(arg0.type().componentType().matches(arg1.type().componentType()));
            if (!this->pushExpression(arg0) || !this->pushVectorizedExpression(arg1, arg0.type())) {
                return unsupported();
            }
            return this->binaryOp(arg0.type(), kMinOps);

        case IntrinsicKind::k_max_IntrinsicKind:
            SkASSERT(arg0.type().componentType().matches(arg1.type().componentType()));
            if (!this->pushExpression(arg0) || !this->pushVectorizedExpression(arg1, arg0.type())) {
                return unsupported();
            }
            return this->binaryOp(arg0.type(), kMaxOps);

        case IntrinsicKind::k_matrixCompMult_IntrinsicKind:
            SkASSERT(arg0.type().matches(arg1.type()));
            if (!this->pushExpression(arg0) || !this->pushExpression(arg1)) {
                return unsupported();
            }
            return this->binaryOp(arg0.type(), kMultiplyOps);

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
            // TODO: implement mix(a,b,genBType)
            if (!arg2.type().componentType().isFloat()) {
                return unsupported();
            }
            SkASSERT(arg0.type().matches(arg1.type()));
            SkASSERT(arg0.type().componentType().matches(arg2.type().componentType()));
            if (!this->pushExpression(arg0) || !this->pushExpression(arg1)) {
                return unsupported();
            }
            if (!this->pushVectorizedExpression(arg2, arg0.type())) {
                return unsupported();
            }
            if (!this->ternaryOp(arg0.type(), kMixOps)) {
                return unsupported();
            }
            return true;

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
    std::unique_ptr<LValue> lvalue = LValue::Make(*p.operand());
    if (!lvalue) {
        return unsupported();
    }
    lvalue->push(this);

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
    lvalue->store(this);

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
    SkASSERT(s.components().size() >= 1 && s.components().size() <= 4);

    // Determine if the swizzle rearranges its elements, or if it's a simple subset of its elements.
    // (A simple subset would be a sequential non-repeating range of components, like `.xyz` or
    // `.yzw` or `.z`, but not `.xx` or `.xz`.)
    bool isSimpleSubset = true;
    for (int index = 1; index < s.components().size(); ++index) {
        if (s.components()[index] != s.components()[0] + index) {
            isSimpleSubset = false;
            break;
        }
    }
    // If this is a simple subset of a variable's slots...
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

bool Generator::pushTernaryExpression(const Expression& test,
                                      const Expression& ifTrue,
                                      const Expression& ifFalse) {
    // First, push the current condition-mask and the test-expression into a separate stack.
    this->nextTempStack();
    fBuilder.push_condition_mask();
    if (!this->pushExpression(test)) {
        return unsupported();
    }
    this->previousTempStack();

    // We can take some shortcuts with condition-mask handling if the false-expression is entirely
    // side-effect free. (We can evaluate it without masking off its effects.) We always handle the
    // condition mask properly for the test-expression and true-expression properly.
    if (!Analysis::HasSideEffects(ifFalse)) {
        // Push the false-expression onto the primary stack.
        int cleanupLabelID = fBuilder.nextLabelID();
        if (!this->pushExpression(ifFalse)) {
            return unsupported();
        }

        // Next, merge the condition mask (on the separate stack) with the test expression.
        this->nextTempStack();
        fBuilder.merge_condition_mask();
        this->previousTempStack();

        // If no lanes are active, we can skip the true-expression entirely. This isn't super likely
        // to happen, so it's probably only a win for non-trivial true-expressions.
        if (!Analysis::IsTrivialExpression(ifTrue)) {
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
        this->nextTempStack();
        fBuilder.merge_condition_mask();
        this->previousTempStack();

        // Push the true-expression onto the primary stack.
        if (!this->pushExpression(ifTrue)) {
            return unsupported();
        }

        // Switch back to the test-expression stack temporarily, and negate the test condition.
        this->nextTempStack();
        fBuilder.unary_op(BuilderOp::bitwise_not_int, /*slots=*/1);
        fBuilder.merge_condition_mask();
        this->previousTempStack();

        // Push the false-expression onto the primary stack, immediately after the true-expression.
        if (!this->pushExpression(ifFalse)) {
            return unsupported();
        }

        // Use a select to conditionally mask-merge the true-expression and false-expression lanes;
        // the mask is already set up for this.
        fBuilder.select(/*slots=*/ifTrue.type().slotCount());
    }

    // Restore the condition-mask to its original state and jettison the test-expression.
    this->nextTempStack();
    this->discardExpression(/*slots=*/1);
    fBuilder.pop_condition_mask();
    this->previousTempStack();

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
    std::optional<SlotRange> mainResult = this->writeFunction(function, function);
    if (!mainResult.has_value()) {
        return unsupported();
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
    // TODO(skia:13676): add mechanism for uniform passing
    RP::Generator generator(program, debugTrace);
    if (!generator.writeProgram(function)) {
        return nullptr;
    }
    return generator.finish();
}

}  // namespace SkSL
