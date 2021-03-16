/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/SkSLProgramElement.h"
#include "include/private/SkSLStatement.h"
#include "include/private/SkTArray.h"
#include "include/private/SkTPin.h"
#include "src/sksl/SkSLCodeGenerator.h"
#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLOperators.h"
#include "src/sksl/SkSLVMGenerator.h"
#include "src/sksl/ir/SkSLBinaryExpression.h"
#include "src/sksl/ir/SkSLBlock.h"
#include "src/sksl/ir/SkSLBoolLiteral.h"
#include "src/sksl/ir/SkSLBreakStatement.h"
#include "src/sksl/ir/SkSLConstructor.h"
#include "src/sksl/ir/SkSLContinueStatement.h"
#include "src/sksl/ir/SkSLDoStatement.h"
#include "src/sksl/ir/SkSLExpressionStatement.h"
#include "src/sksl/ir/SkSLExternalFunctionCall.h"
#include "src/sksl/ir/SkSLExternalFunctionReference.h"
#include "src/sksl/ir/SkSLFieldAccess.h"
#include "src/sksl/ir/SkSLFloatLiteral.h"
#include "src/sksl/ir/SkSLForStatement.h"
#include "src/sksl/ir/SkSLFunctionCall.h"
#include "src/sksl/ir/SkSLFunctionDeclaration.h"
#include "src/sksl/ir/SkSLFunctionDefinition.h"
#include "src/sksl/ir/SkSLIfStatement.h"
#include "src/sksl/ir/SkSLIndexExpression.h"
#include "src/sksl/ir/SkSLIntLiteral.h"
#include "src/sksl/ir/SkSLPostfixExpression.h"
#include "src/sksl/ir/SkSLPrefixExpression.h"
#include "src/sksl/ir/SkSLReturnStatement.h"
#include "src/sksl/ir/SkSLSwitchStatement.h"
#include "src/sksl/ir/SkSLSwizzle.h"
#include "src/sksl/ir/SkSLTernaryExpression.h"
#include "src/sksl/ir/SkSLVarDeclarations.h"
#include "src/sksl/ir/SkSLVariableReference.h"

#include <algorithm>
#include <unordered_map>

namespace {
    // sksl allows the optimizations of fast_mul(), so we want to use that most of the time.
    // This little sneaky snippet of code lets us use ** as a fast multiply infix operator.
    struct FastF32 { skvm::F32 val; };
    static FastF32 operator*(skvm::F32 y) { return {y}; }
    static skvm::F32 operator*(skvm::F32 x, FastF32 y) { return fast_mul(x, y.val); }
    static skvm::F32 operator*(float     x, FastF32 y) { return fast_mul(x, y.val); }
}

namespace SkSL {

namespace {

// Holds scalars, vectors, or matrices
struct Value {
    Value() = default;
    explicit Value(size_t slots) {
        fVals.resize(slots);
    }
    Value(skvm::F32 x) : fVals({ x.id }) {}
    Value(skvm::I32 x) : fVals({ x.id }) {}

    explicit operator bool() const { return !fVals.empty(); }

    size_t slots() const { return fVals.size(); }

    struct ValRef {
        ValRef(skvm::Val& val) : fVal(val) {}

        ValRef& operator=(ValRef    v) { fVal = v.fVal; return *this; }
        ValRef& operator=(skvm::Val v) { fVal = v;      return *this; }
        ValRef& operator=(skvm::F32 v) { fVal = v.id;   return *this; }
        ValRef& operator=(skvm::I32 v) { fVal = v.id;   return *this; }

        operator skvm::Val() { return fVal; }

        skvm::Val& fVal;
    };

    ValRef    operator[](size_t i) {
        // These redundant asserts work around what we think is a codegen bug in GCC 8.x for
        // 32-bit x86 Debug builds.
        SkASSERT(i < fVals.size());
        return fVals[i];
    }
    skvm::Val operator[](size_t i) const {
        // These redundant asserts work around what we think is a codegen bug in GCC 8.x for
        // 32-bit x86 Debug builds.
        SkASSERT(i < fVals.size());
        return fVals[i];
    }

    SkSpan<skvm::Val> asSpan() { return fVals; }

private:
    SkSTArray<4, skvm::Val, true> fVals;
};

}  // namespace

class SkVMGenerator {
public:
    SkVMGenerator(const Program& program,
                  skvm::Builder* builder,
                  SkSpan<skvm::Val> uniforms,
                  skvm::Coord device,
                  skvm::Coord local,
                  SampleChildFn sampleChild);

    void writeFunction(const FunctionDefinition& function,
                       SkSpan<skvm::Val> arguments,
                       SkSpan<skvm::Val> outReturn);

private:
    enum class Intrinsic {
        // sksl_public.sksl declares these intrinsics (and defines some other inline)

        // Angle & Trigonometry
        kRadians,
        kDegrees,
        kSin,
        kCos,
        kTan,

        kASin,
        kACos,
        kATan,

        // Exponential
        kPow,
        kExp,
        kLog,
        kExp2,
        kLog2,

        kSqrt,
        kInverseSqrt,

        // Common
        kAbs,
        kSign,
        kFloor,
        kCeil,
        kFract,
        kMod,

        kMin,
        kMax,
        kClamp,
        kSaturate,
        kMix,
        kStep,
        kSmoothstep,

        // Geometric
        kLength,
        kDistance,
        kDot,
        kCross,
        kNormalize,
        kFaceforward,
        kReflect,
        kRefract,

        // Matrix
        kMatrixCompMult,
        kInverse,

        // Vector Relational
        kLessThan,
        kLessThanEqual,
        kGreaterThan,
        kGreaterThanEqual,
        kEqual,
        kNotEqual,

        kAny,
        kAll,
        kNot,

        // SkSL
        kSample,
    };

    /**
     * In SkSL, a Variable represents a named, typed value (along with qualifiers, etc).
     * Every Variable is mapped to one (or several, contiguous) indices into our vector of
     * skvm::Val. Those skvm::Val entries hold the current actual value of that variable.
     *
     * NOTE: Conceptually, each Variable is just mapped to a Value. We could implement it that way,
     * (and eliminate the indirection), but it would add overhead for each Variable,
     * and add additional (different) bookkeeping for things like lvalue-swizzles.
     *
     * Any time a variable appears in an expression, that's a VariableReference, which is a kind of
     * Expression. Evaluating that VariableReference (or any other Expression) produces a Value,
     * which is a set of skvm::Val. (This allows an Expression to produce a vector or matrix, in
     * addition to a scalar).
     *
     * For a VariableReference, producing a Value is straightforward - we get the slot of the
     * Variable (from fVariableMap), use that to look up the current skvm::Vals holding the
     * variable's contents, and construct a Value with those ids.
     */

    /**
     * Returns the slot holding v's Val(s). Allocates storage if this is first time 'v' is
     * referenced. Compound variables (e.g. vectors) will consume more than one slot, with
     * getSlot returning the start of the contiguous chunk of slots.
     */
    size_t getSlot(const Variable& v);

    skvm::F32 f32(skvm::Val id) { SkASSERT(id != skvm::NA); return {fBuilder, id}; }
    skvm::I32 i32(skvm::Val id) { SkASSERT(id != skvm::NA); return {fBuilder, id}; }

    // Shorthand for scalars
    skvm::F32 f32(const Value& v) { SkASSERT(v.slots() == 1); return f32(v[0]); }
    skvm::I32 i32(const Value& v) { SkASSERT(v.slots() == 1); return i32(v[0]); }

    template <typename Fn>
    Value unary(const Value& v, Fn&& fn) {
        Value result(v.slots());
        for (size_t i = 0; i < v.slots(); ++i) {
            result[i] = fn({fBuilder, v[i]});
        }
        return result;
    }

    skvm::I32 mask() {
        // As we encounter (possibly conditional) return statements, fReturned is updated to store
        // the lanes that have already returned. For the remainder of the current function, those
        // lanes should be disabled.
        return fConditionMask & fLoopMask & ~currentFunction().fReturned;
    }

    size_t fieldSlotOffset(const FieldAccess& expr);
    size_t indexSlotOffset(const IndexExpression& expr);

    Value writeExpression(const Expression& expr);
    Value writeBinaryExpression(const BinaryExpression& b);
    Value writeConstructor(const Constructor& c);
    Value writeFunctionCall(const FunctionCall& c);
    Value writeExternalFunctionCall(const ExternalFunctionCall& c);
    Value writeFieldAccess(const FieldAccess& expr);
    Value writeIndexExpression(const IndexExpression& expr);
    Value writeIntrinsicCall(const FunctionCall& c);
    Value writePostfixExpression(const PostfixExpression& p);
    Value writePrefixExpression(const PrefixExpression& p);
    Value writeSwizzle(const Swizzle& swizzle);
    Value writeTernaryExpression(const TernaryExpression& t);
    Value writeVariableExpression(const VariableReference& expr);

    void writeStatement(const Statement& s);
    void writeBlock(const Block& b);
    void writeBreakStatement();
    void writeContinueStatement();
    void writeForStatement(const ForStatement& f);
    void writeIfStatement(const IfStatement& stmt);
    void writeReturnStatement(const ReturnStatement& r);
    void writeVarDeclaration(const VarDeclaration& decl);

    Value writeStore(const Expression& lhs, const Value& rhs);

    Value writeMatrixInverse2x2(const Value& m);
    Value writeMatrixInverse3x3(const Value& m);
    Value writeMatrixInverse4x4(const Value& m);

    //
    // Global state for the lifetime of the generator:
    //
    const Program& fProgram;
    skvm::Builder* fBuilder;

    const skvm::Coord fLocalCoord;
    const SampleChildFn fSampleChild;

    // [Variable, first slot in fSlots]
    std::unordered_map<const Variable*, size_t> fVariableMap;
    std::vector<skvm::Val> fSlots;

    // Conditional execution mask (managed by ScopedCondition, and tied to control-flow scopes)
    skvm::I32 fConditionMask;

    // Similar: loop execution masks. Each loop starts with all lanes active (fLoopMask).
    // 'break' disables a lane in fLoopMask until the loop finishes
    // 'continue' disables a lane in fLoopMask, and sets fContinueMask to be re-enabled on the next
    //   iteration
    skvm::I32 fLoopMask;
    skvm::I32 fContinueMask;

    //
    // State that's local to the generation of a single function:
    //
    struct Function {
        const SkSpan<skvm::Val> fReturnValue;
        skvm::I32               fReturned;
    };
    std::vector<Function> fFunctionStack;
    Function& currentFunction() { return fFunctionStack.back(); }

    class ScopedCondition {
    public:
        ScopedCondition(SkVMGenerator* generator, skvm::I32 mask)
                : fGenerator(generator), fOldConditionMask(fGenerator->fConditionMask) {
            fGenerator->fConditionMask &= mask;
        }

        ~ScopedCondition() { fGenerator->fConditionMask = fOldConditionMask; }

    private:
        SkVMGenerator* fGenerator;
        skvm::I32 fOldConditionMask;
    };
};

static Type::NumberKind base_number_kind(const Type& type) {
    if (type.typeKind() == Type::TypeKind::kMatrix || type.typeKind() == Type::TypeKind::kVector) {
        return base_number_kind(type.componentType());
    }
    return type.numberKind();
}

static inline bool is_uniform(const SkSL::Variable& var) {
    return var.modifiers().fFlags & Modifiers::kUniform_Flag;
}

static size_t slot_count(const Type& type) {
    switch (type.typeKind()) {
        case Type::TypeKind::kOther:
        case Type::TypeKind::kVoid:
            return 0;
        case Type::TypeKind::kStruct: {
            size_t slots = 0;
            for (const auto& f : type.fields()) {
                slots += slot_count(*f.fType);
            }
            return slots;
        }
        case Type::TypeKind::kArray:
            SkASSERT(type.columns() > 0);
            return type.columns() * slot_count(type.componentType());
        default:
            return type.columns() * type.rows();
    }
}

SkVMGenerator::SkVMGenerator(const Program& program,
                             skvm::Builder* builder,
                             SkSpan<skvm::Val> uniforms,
                             skvm::Coord device,
                             skvm::Coord local,
                             SampleChildFn sampleChild)
        : fProgram(program)
        , fBuilder(builder)
        , fLocalCoord(local)
        , fSampleChild(std::move(sampleChild)) {
    fConditionMask = fLoopMask = fBuilder->splat(0xffff'ffff);

    // Now, add storage for each global variable (including uniforms) to fSlots, and entries in
    // fVariableMap to remember where every variable is stored.
    const skvm::Val* uniformIter = uniforms.begin();
    size_t fpCount = 0;
    for (const ProgramElement* e : fProgram.elements()) {
        if (e->is<GlobalVarDeclaration>()) {
            const GlobalVarDeclaration& gvd = e->as<GlobalVarDeclaration>();
            const VarDeclaration& decl = gvd.declaration()->as<VarDeclaration>();
            const Variable& var = decl.var();
            SkASSERT(fVariableMap.find(&var) == fVariableMap.end());

            // For most variables, fVariableMap stores an index into fSlots, but for fragment
            // processors (child shaders), fVariableMap stores the index to pass to fSampleChild().
            if (var.type() == *fProgram.fContext->fTypes.fFragmentProcessor) {
                fVariableMap[&var] = fpCount++;
                continue;
            }

            // Opaque types include fragment processors, GL objects (samplers, textures, etc), and
            // special types like 'void'. Of those, only fragment processors are legal variables.
            SkASSERT(!var.type().isOpaque());

            // getSlot() allocates space for the variable's value in fSlots, initializes it to zero,
            // and populates fVariableMap.
            size_t slot   = this->getSlot(var),
                   nslots = slot_count(var.type());

            if (int builtin = var.modifiers().fLayout.fBuiltin; builtin >= 0) {
                // builtin variables are system-defined, with special semantics. The only builtin
                // variable exposed to runtime effects is sk_FragCoord.
                switch (builtin) {
                    case SK_FRAGCOORD_BUILTIN:
                        SkASSERT(nslots == 4);
                        fSlots[slot + 0] = device.x.id;
                        fSlots[slot + 1] = device.y.id;
                        fSlots[slot + 2] = fBuilder->splat(0.0f).id;
                        fSlots[slot + 3] = fBuilder->splat(1.0f).id;
                        break;
                    default:
                        SkDEBUGFAIL("Unsupported builtin");
                }
            } else if (is_uniform(var)) {
                // For uniforms, copy the supplied IDs over
                SkASSERT(uniformIter + nslots <= uniforms.end());
                std::copy(uniformIter, uniformIter + nslots, fSlots.begin() + slot);
                uniformIter += nslots;
            } else if (decl.value()) {
                // For other globals, populate with the initializer expression (if there is one)
                Value val = this->writeExpression(*decl.value());
                for (size_t i = 0; i < nslots; ++i) {
                    fSlots[slot + i] = val[i];
                }
            }
        }
    }
    SkASSERT(uniformIter == uniforms.end());
}

void SkVMGenerator::writeFunction(const FunctionDefinition& function,
                                  SkSpan<skvm::Val> arguments,
                                  SkSpan<skvm::Val> outReturn) {
    const FunctionDeclaration& decl = function.declaration();
    SkASSERT(slot_count(decl.returnType()) == outReturn.size());

    fFunctionStack.push_back({outReturn, /*returned=*/fBuilder->splat(0)});

    // For all parameters, copy incoming argument IDs to our vector of (all) variable IDs
    size_t argIdx = 0;
    for (const Variable* p : decl.parameters()) {
        size_t paramSlot = this->getSlot(*p),
               nslots    = slot_count(p->type());

        for (size_t i = 0; i < nslots; ++i) {
            fSlots[paramSlot + i] = arguments[argIdx + i];
        }
        argIdx += nslots;
    }
    SkASSERT(argIdx == arguments.size());

    this->writeStatement(*function.body());

    // Copy 'out' and 'inout' parameters back to their caller-supplied argument storage
    argIdx = 0;
    for (const Variable* p : decl.parameters()) {
        size_t nslots = slot_count(p->type());

        if (p->modifiers().fFlags & Modifiers::kOut_Flag) {
            size_t paramSlot = this->getSlot(*p);
            for (size_t i = 0; i < nslots; ++i) {
                arguments[argIdx + i] = fSlots[paramSlot + i];
            }
        }
        argIdx += nslots;
    }
    SkASSERT(argIdx == arguments.size());

    fFunctionStack.pop_back();
}

size_t SkVMGenerator::getSlot(const Variable& v) {
    auto entry = fVariableMap.find(&v);
    if (entry != fVariableMap.end()) {
        return entry->second;
    }

    size_t slot   = fSlots.size(),
           nslots = slot_count(v.type());
    fSlots.resize(slot + nslots, fBuilder->splat(0.0f).id);
    fVariableMap[&v] = slot;
    return slot;
}

Value SkVMGenerator::writeBinaryExpression(const BinaryExpression& b) {
    const Expression& left = *b.left();
    const Expression& right = *b.right();
    Operator op = b.getOperator();
    if (op.kind() == Token::Kind::TK_EQ) {
        return this->writeStore(left, this->writeExpression(right));
    }

    const Type& lType = left.type();
    const Type& rType = right.type();
    bool lVecOrMtx = (lType.isVector() || lType.isMatrix());
    bool rVecOrMtx = (rType.isVector() || rType.isMatrix());
    bool isAssignment = op.isAssignment();
    if (isAssignment) {
        op = op.removeAssignment();
    }
    Type::NumberKind nk = base_number_kind(lType);

    // A few ops require special treatment:
    switch (op.kind()) {
        case Token::Kind::TK_LOGICALAND: {
            SkASSERT(!isAssignment);
            SkASSERT(nk == Type::NumberKind::kBoolean);
            skvm::I32 lVal = i32(this->writeExpression(left));
            ScopedCondition shortCircuit(this, lVal);
            skvm::I32 rVal = i32(this->writeExpression(right));
            return lVal & rVal;
        }
        case Token::Kind::TK_LOGICALOR: {
            SkASSERT(!isAssignment);
            SkASSERT(nk == Type::NumberKind::kBoolean);
            skvm::I32 lVal = i32(this->writeExpression(left));
            ScopedCondition shortCircuit(this, ~lVal);
            skvm::I32 rVal = i32(this->writeExpression(right));
            return lVal | rVal;
        }
        case Token::Kind::TK_COMMA:
            // We write the left side of the expression to preserve its side effects, even though we
            // immediately discard the result.
            this->writeExpression(left);
            return this->writeExpression(right);
        default:
            break;
    }

    // All of the other ops always evaluate both sides of the expression
    Value lVal = this->writeExpression(left),
          rVal = this->writeExpression(right);

    // Special case for M*V, V*M, M*M (but not V*V!)
    if (op.kind() == Token::Kind::TK_STAR
        && lVecOrMtx && rVecOrMtx && !(lType.isVector() && rType.isVector())) {
        int rCols = rType.columns(),
            rRows = rType.rows(),
            lCols = lType.columns(),
            lRows = lType.rows();
        // M*V treats the vector as a column
        if (rType.isVector()) {
            std::swap(rCols, rRows);
        }
        SkASSERT(lCols == rRows);
        SkASSERT(slot_count(b.type()) == static_cast<size_t>(lRows * rCols));
        Value result(lRows * rCols);
        size_t resultIdx = 0;
        for (int c = 0; c < rCols; ++c)
        for (int r = 0; r < lRows; ++r) {
            skvm::F32 sum = fBuilder->splat(0.0f);
            for (int j = 0; j < lCols; ++j) {
                sum += f32(lVal[j*lRows + r]) * f32(rVal[c*rRows + j]);
            }
            result[resultIdx++] = sum;
        }
        SkASSERT(resultIdx == result.slots());
        return isAssignment ? this->writeStore(left, result) : result;
    }

    size_t nslots = std::max(lVal.slots(), rVal.slots());

    auto binary = [&](auto&& f_fn, auto&& i_fn) {
        Value result(nslots);
        for (size_t i = 0; i < nslots; ++i) {
            // If one side is scalar, replicate it to all channels
            skvm::Val L = lVal.slots() == 1 ? lVal[0] : lVal[i],
                      R = rVal.slots() == 1 ? rVal[0] : rVal[i];
            if (nk == Type::NumberKind::kFloat) {
                result[i] = f_fn(f32(L), f32(R));
            } else {
                result[i] = i_fn(i32(L), i32(R));
            }
        }
        return isAssignment ? this->writeStore(left, result) : result;
    };

    auto unsupported_f = [&](skvm::F32, skvm::F32) {
        SkDEBUGFAIL("Unsupported operator");
        return skvm::F32{};
    };

    switch (op.kind()) {
        case Token::Kind::TK_EQEQ: {
            SkASSERT(!isAssignment);
            Value cmp = binary([](skvm::F32 x, skvm::F32 y) { return x == y; },
                               [](skvm::I32 x, skvm::I32 y) { return x == y; });
            skvm::I32 folded = i32(cmp[0]);
            for (size_t i = 1; i < nslots; ++i) {
                folded &= i32(cmp[i]);
            }
            return folded;
        }
        case Token::Kind::TK_NEQ: {
            SkASSERT(!isAssignment);
            Value cmp = binary([](skvm::F32 x, skvm::F32 y) { return x != y; },
                               [](skvm::I32 x, skvm::I32 y) { return x != y; });
            skvm::I32 folded = i32(cmp[0]);
            for (size_t i = 1; i < nslots; ++i) {
                folded |= i32(cmp[i]);
            }
            return folded;
        }
        case Token::Kind::TK_GT:
            return binary([](skvm::F32 x, skvm::F32 y) { return x > y; },
                          [](skvm::I32 x, skvm::I32 y) { return x > y; });
        case Token::Kind::TK_GTEQ:
            return binary([](skvm::F32 x, skvm::F32 y) { return x >= y; },
                          [](skvm::I32 x, skvm::I32 y) { return x >= y; });
        case Token::Kind::TK_LT:
            return binary([](skvm::F32 x, skvm::F32 y) { return x < y; },
                          [](skvm::I32 x, skvm::I32 y) { return x < y; });
        case Token::Kind::TK_LTEQ:
            return binary([](skvm::F32 x, skvm::F32 y) { return x <= y; },
                          [](skvm::I32 x, skvm::I32 y) { return x <= y; });

        case Token::Kind::TK_PLUS:
            return binary([](skvm::F32 x, skvm::F32 y) { return x + y; },
                          [](skvm::I32 x, skvm::I32 y) { return x + y; });
        case Token::Kind::TK_MINUS:
            return binary([](skvm::F32 x, skvm::F32 y) { return x - y; },
                          [](skvm::I32 x, skvm::I32 y) { return x - y; });
        case Token::Kind::TK_STAR:
            return binary([](skvm::F32 x, skvm::F32 y) { return x ** y; },
                          [](skvm::I32 x, skvm::I32 y) { return x * y; });
        case Token::Kind::TK_SLASH:
            // Minimum spec (GLSL ES 1.0) has very loose requirements for integer operations.
            // (Low-end GPUs may not have integer ALUs). Given that, we are allowed to do floating
            // point division plus rounding. Section 10.28 of the spec even clarifies that the
            // rounding mode is undefined (but round-towards-zero is the obvious/common choice).
            return binary([](skvm::F32 x, skvm::F32 y) { return x / y; },
                          [](skvm::I32 x, skvm::I32 y) {
                              return skvm::trunc(skvm::to_F32(x) / skvm::to_F32(y));
                          });

        case Token::Kind::TK_BITWISEXOR:
        case Token::Kind::TK_LOGICALXOR:
            return binary(unsupported_f, [](skvm::I32 x, skvm::I32 y) { return x ^ y; });
        case Token::Kind::TK_BITWISEAND:
            return binary(unsupported_f, [](skvm::I32 x, skvm::I32 y) { return x & y; });
        case Token::Kind::TK_BITWISEOR:
            return binary(unsupported_f, [](skvm::I32 x, skvm::I32 y) { return x | y; });

        // These three operators are all 'reserved' (illegal) in our minimum spec, but will require
        // implementation in the future.
        case Token::Kind::TK_PERCENT:
        case Token::Kind::TK_SHL:
        case Token::Kind::TK_SHR:
        default:
            SkDEBUGFAIL("Unsupported operator");
            return {};
    }
}

Value SkVMGenerator::writeConstructor(const Constructor& c) {
    if (c.arguments().size() > 1) {
        // Multi-argument constructors just aggregate their arguments, with no conversion
        // NOTE: This (SkSL rule) is actually more restrictive than GLSL.
        Value result(slot_count(c.type()));
        size_t resultIdx = 0;
        for (const auto &arg : c.arguments()) {
            Value tmp = this->writeExpression(*arg);
            for (size_t tmpSlot = 0; tmpSlot < tmp.slots(); ++tmpSlot) {
                result[resultIdx++] = tmp[tmpSlot];
            }
        }
        return result;
    }

    const Type& srcType = c.arguments()[0]->type();
    const Type& dstType = c.type();
    Type::NumberKind srcKind = base_number_kind(srcType),
                     dstKind = base_number_kind(dstType);
    Value src = this->writeExpression(*c.arguments()[0]);
    size_t dstSlots = slot_count(dstType);

    // Conversion among "similar" types (floatN <-> halfN), (shortN <-> intN), etc. is a no-op
    if (srcKind == dstKind && src.slots() == dstSlots) {
        return src;
    }

    // TODO: Handle signed vs. unsigned. GLSL ES 1.0 only has 'int', so no problem yet.
    if (srcKind != dstKind) {
        // One argument constructors can do type conversion
        Value dst(src.slots());
        switch (dstKind) {
            case Type::NumberKind::kFloat:
                if (srcKind == Type::NumberKind::kSigned) {
                    // int -> float
                    for (size_t i = 0; i < src.slots(); ++i) {
                        dst[i] = skvm::to_F32(i32(src[i]));
                    }
                    return dst;
                } else if (srcKind == Type::NumberKind::kBoolean) {
                    // bool -> float
                    for (size_t i = 0; i < src.slots(); ++i) {
                        dst[i] = skvm::select(i32(src[i]), 1.0f, 0.0f);
                    }
                    return dst;
                }
                break;

            case Type::NumberKind::kSigned:
                if (srcKind == Type::NumberKind::kFloat) {
                    // float -> int
                    for (size_t i = 0; i < src.slots(); ++i) {
                        dst[i] = skvm::trunc(f32(src[i]));
                    }
                    return dst;
                } else if (srcKind == Type::NumberKind::kBoolean) {
                    // bool -> int
                    for (size_t i = 0; i < src.slots(); ++i) {
                        dst[i] = skvm::select(i32(src[i]), 1, 0);
                    }
                    return dst;
                }
                break;

            case Type::NumberKind::kBoolean:
                if (srcKind == Type::NumberKind::kSigned) {
                    // int -> bool
                    for (size_t i = 0; i < src.slots(); ++i) {
                        dst[i] = i32(src[i]) != 0;
                    }
                    return dst;
                } else if (srcKind == Type::NumberKind::kFloat) {
                    // float -> bool
                    for (size_t i = 0; i < src.slots(); ++i) {
                        dst[i] = f32(src[i]) != 0.0;
                    }
                    return dst;
                }
                break;

            default:
                break;
        }
        SkDEBUGFAILF("Unsupported type conversion: %s -> %s", srcType.displayName().c_str(),
                                                              dstType.displayName().c_str());
        return {};
    }

    // Matrices can be constructed from scalars or other matrices
    if (dstType.isMatrix()) {
        Value dst(dstType.rows() * dstType.columns());
        size_t dstIndex = 0;
        if (srcType.isMatrix()) {
            // Matrix-from-matrix uses src where it overlaps, fills in missing with identity
            for (int c = 0; c < dstType.columns(); ++c)
            for (int r = 0; r < dstType.rows(); ++r) {
                if (c < srcType.columns() && r < srcType.rows()) {
                    dst[dstIndex++] = src[c * srcType.rows() + r];
                } else {
                    dst[dstIndex++] = fBuilder->splat(c == r ? 1.0f : 0.0f);
                }
            }
        } else if (srcType.isScalar()) {
            // Matrix-from-scalar builds a diagonal scale matrix
            for (int c = 0; c < dstType.columns(); ++c)
            for (int r = 0; r < dstType.rows(); ++r) {
                dst[dstIndex++] = (c == r ? f32(src) : fBuilder->splat(0.0f));
            }
        } else {
            SkDEBUGFAIL("Invalid matrix constructor");
        }
        SkASSERT(dstIndex == dst.slots());
        return dst;
    }

    // We can splat scalars to all components of a vector
    if (dstType.isVector() && srcType.isScalar()) {
        Value dst(dstType.columns());
        for (int i = 0; i < dstType.columns(); ++i) {
            dst[i] = src[0];
        }
        return dst;
    }

    SkDEBUGFAIL("Invalid constructor");
    return {};
}

size_t SkVMGenerator::fieldSlotOffset(const FieldAccess& expr) {
    size_t offset = 0;
    for (int i = 0; i < expr.fieldIndex(); ++i) {
        offset += slot_count(*expr.base()->type().fields()[i].fType);
    }
    return offset;
}

Value SkVMGenerator::writeFieldAccess(const FieldAccess& expr) {
    Value base = this->writeExpression(*expr.base());
    Value field(slot_count(expr.type()));
    size_t offset = this->fieldSlotOffset(expr);
    for (size_t i = 0; i < field.slots(); ++i) {
        field[i] = base[offset + i];
    }
    return field;
}

size_t SkVMGenerator::indexSlotOffset(const IndexExpression& expr) {
    Value index = this->writeExpression(*expr.index());
    int indexValue = -1;
    SkAssertResult(fBuilder->allImm(index[0], &indexValue));

    // When indexing by a literal, the front-end guarantees that we don't go out of bounds.
    // But when indexing by a loop variable, it's possible to generate out-of-bounds access.
    // The GLSL spec leaves that behavior undefined - we'll just clamp everything here.
    indexValue = SkTPin(indexValue, 0, expr.base()->type().columns() - 1);

    size_t stride = slot_count(expr.type());
    return indexValue * stride;
}

Value SkVMGenerator::writeIndexExpression(const IndexExpression& expr) {
    Value base = this->writeExpression(*expr.base());
    Value element(slot_count(expr.type()));
    size_t offset = this->indexSlotOffset(expr);
    for (size_t i = 0; i < element.slots(); ++i) {
        element[i] = base[offset + i];
    }
    return element;
}

Value SkVMGenerator::writeVariableExpression(const VariableReference& expr) {
    size_t slot = this->getSlot(*expr.variable());
    Value val(slot_count(expr.type()));
    for (size_t i = 0; i < val.slots(); ++i) {
        val[i] = fSlots[slot + i];
    }
    return val;
}

Value SkVMGenerator::writeMatrixInverse2x2(const Value& m) {
    SkASSERT(m.slots() == 4);
    skvm::F32 a = f32(m[0]),
              b = f32(m[1]),
              c = f32(m[2]),
              d = f32(m[3]);
    skvm::F32 idet = 1.0f / (a*d - b*c);

    Value result(m.slots());
    result[0] = ( d ** idet);
    result[1] = (-b ** idet);
    result[2] = (-c ** idet);
    result[3] = ( a ** idet);
    return result;
}

Value SkVMGenerator::writeMatrixInverse3x3(const Value& m) {
    SkASSERT(m.slots() == 9);
    skvm::F32 a11 = f32(m[0]), a12 = f32(m[3]), a13 = f32(m[6]),
              a21 = f32(m[1]), a22 = f32(m[4]), a23 = f32(m[7]),
              a31 = f32(m[2]), a32 = f32(m[5]), a33 = f32(m[8]);
    skvm::F32 idet = 1.0f / (a11*a22*a33 + a12*a23*a31 + a13*a21*a32 -
                             a11*a23*a32 - a12*a21*a33 - a13*a22*a31);

    Value result(m.slots());
    result[0] = ((a22**a33 - a23**a32) ** idet);
    result[1] = ((a23**a31 - a21**a33) ** idet);
    result[2] = ((a21**a32 - a22**a31) ** idet);
    result[3] = ((a13**a32 - a12**a33) ** idet);
    result[4] = ((a11**a33 - a13**a31) ** idet);
    result[5] = ((a12**a31 - a11**a32) ** idet);
    result[6] = ((a12**a23 - a13**a22) ** idet);
    result[7] = ((a13**a21 - a11**a23) ** idet);
    result[8] = ((a11**a22 - a12**a21) ** idet);
    return result;
}

Value SkVMGenerator::writeMatrixInverse4x4(const Value& m) {
    SkASSERT(m.slots() == 16);
    skvm::F32 a00 = f32(m[0]), a10 = f32(m[4]), a20 = f32(m[ 8]), a30 = f32(m[12]),
              a01 = f32(m[1]), a11 = f32(m[5]), a21 = f32(m[ 9]), a31 = f32(m[13]),
              a02 = f32(m[2]), a12 = f32(m[6]), a22 = f32(m[10]), a32 = f32(m[14]),
              a03 = f32(m[3]), a13 = f32(m[7]), a23 = f32(m[11]), a33 = f32(m[15]);

    skvm::F32 b00 = a00**a11 - a01**a10,
              b01 = a00**a12 - a02**a10,
              b02 = a00**a13 - a03**a10,
              b03 = a01**a12 - a02**a11,
              b04 = a01**a13 - a03**a11,
              b05 = a02**a13 - a03**a12,
              b06 = a20**a31 - a21**a30,
              b07 = a20**a32 - a22**a30,
              b08 = a20**a33 - a23**a30,
              b09 = a21**a32 - a22**a31,
              b10 = a21**a33 - a23**a31,
              b11 = a22**a33 - a23**a32;

    skvm::F32 idet = 1.0f / (b00**b11 - b01**b10 + b02**b09 + b03**b08 - b04**b07 + b05**b06);

    b00 *= idet;
    b01 *= idet;
    b02 *= idet;
    b03 *= idet;
    b04 *= idet;
    b05 *= idet;
    b06 *= idet;
    b07 *= idet;
    b08 *= idet;
    b09 *= idet;
    b10 *= idet;
    b11 *= idet;

    Value result(m.slots());
    result[ 0] = (a11*b11 - a12*b10 + a13*b09);
    result[ 1] = (a02*b10 - a01*b11 - a03*b09);
    result[ 2] = (a31*b05 - a32*b04 + a33*b03);
    result[ 3] = (a22*b04 - a21*b05 - a23*b03);
    result[ 4] = (a12*b08 - a10*b11 - a13*b07);
    result[ 5] = (a00*b11 - a02*b08 + a03*b07);
    result[ 6] = (a32*b02 - a30*b05 - a33*b01);
    result[ 7] = (a20*b05 - a22*b02 + a23*b01);
    result[ 8] = (a10*b10 - a11*b08 + a13*b06);
    result[ 9] = (a01*b08 - a00*b10 - a03*b06);
    result[10] = (a30*b04 - a31*b02 + a33*b00);
    result[11] = (a21*b02 - a20*b04 - a23*b00);
    result[12] = (a11*b07 - a10*b09 - a12*b06);
    result[13] = (a00*b09 - a01*b07 + a02*b06);
    result[14] = (a31*b01 - a30*b03 - a32*b00);
    result[15] = (a20*b03 - a21*b01 + a22*b00);
    return result;
}

Value SkVMGenerator::writeIntrinsicCall(const FunctionCall& c) {
    static std::unordered_map<String, Intrinsic> intrinsics {
        { "radians", Intrinsic::kRadians },
        { "degrees", Intrinsic::kDegrees },
        { "sin",     Intrinsic::kSin },
        { "cos",     Intrinsic::kCos },
        { "tan",     Intrinsic::kTan },
        { "asin",    Intrinsic::kASin },
        { "acos",    Intrinsic::kACos },
        { "atan",    Intrinsic::kATan },

        { "pow",  Intrinsic::kPow },
        { "exp",  Intrinsic::kExp },
        { "log",  Intrinsic::kLog },
        { "exp2", Intrinsic::kExp2 },
        { "log2", Intrinsic::kLog2 },
        { "sqrt", Intrinsic::kSqrt },
        { "inversesqrt", Intrinsic::kInverseSqrt },

        { "abs",   Intrinsic::kAbs },
        { "sign",  Intrinsic::kSign },
        { "floor", Intrinsic::kFloor },
        { "ceil",  Intrinsic::kCeil },
        { "fract", Intrinsic::kFract },
        { "mod",   Intrinsic::kMod },

        { "min",        Intrinsic::kMin },
        { "max",        Intrinsic::kMax },
        { "clamp",      Intrinsic::kClamp },
        { "saturate",   Intrinsic::kSaturate },
        { "mix",        Intrinsic::kMix },
        { "step",       Intrinsic::kStep },
        { "smoothstep", Intrinsic::kSmoothstep },

        { "length",      Intrinsic::kLength },
        { "distance",    Intrinsic::kDistance },
        { "dot",         Intrinsic::kDot },
        { "cross",       Intrinsic::kCross },
        { "normalize",   Intrinsic::kNormalize },
        { "faceforward", Intrinsic::kFaceforward },
        { "reflect",     Intrinsic::kReflect },
        { "refract",     Intrinsic::kRefract },

        { "matrixCompMult", Intrinsic::kMatrixCompMult },
        { "inverse",        Intrinsic::kInverse },

        { "lessThan",         Intrinsic::kLessThan },
        { "lessThanEqual",    Intrinsic::kLessThanEqual },
        { "greaterThan",      Intrinsic::kGreaterThan },
        { "greaterThanEqual", Intrinsic::kGreaterThanEqual },
        { "equal",            Intrinsic::kEqual },
        { "notEqual",         Intrinsic::kNotEqual },

        { "any", Intrinsic::kAny },
        { "all", Intrinsic::kAll },
        { "not", Intrinsic::kNot },

        { "sample", Intrinsic::kSample } };

    auto found = intrinsics.find(c.function().name());
    if (found == intrinsics.end()) {
        SkDEBUGFAILF("Missing intrinsic: '%s'", String(c.function().name()).c_str());
        return {};
    }

    const size_t nargs = c.arguments().size();

    if (found->second == Intrinsic::kSample) {
        // Sample is very special, the first argument is an FP, which can't be evaluated
        const Context& ctx = *fProgram.fContext;
        if (nargs > 2 || c.arguments()[0]->type() != *ctx.fTypes.fFragmentProcessor ||
            (nargs == 2 && (c.arguments()[1]->type() != *ctx.fTypes.fFloat2 &&
                            c.arguments()[1]->type() != *ctx.fTypes.fFloat3x3))) {
            SkDEBUGFAIL("Invalid call to sample");
            return {};
        }

        auto fp_it = fVariableMap.find(c.arguments()[0]->as<VariableReference>().variable());
        SkASSERT(fp_it != fVariableMap.end());

        skvm::Coord coord = fLocalCoord;
        if (nargs == 2) {
            Value arg = this->writeExpression(*c.arguments()[1]);
            if (arg.slots() == 2) {
                // explicit sampling
                coord = {f32(arg[0]), f32(arg[1])};
            } else {
                // matrix sampling
                SkASSERT(arg.slots() == 9);
                skvm::F32 x = f32(arg[0])**coord.x + f32(arg[3])**coord.y + f32(arg[6]),
                          y = f32(arg[1])**coord.x + f32(arg[4])**coord.y + f32(arg[7]),
                          w = f32(arg[2])**coord.x + f32(arg[5])**coord.y + f32(arg[8]);
                x = x ** (1.0f / w);
                y = y ** (1.0f / w);
                coord = {x, y};
            }
        }

        skvm::Color color = fSampleChild(fp_it->second, coord);
        Value result(4);
        result[0] = color.r;
        result[1] = color.g;
        result[2] = color.b;
        result[3] = color.a;
        return result;
    }

    const size_t kMaxArgs = 3;  // eg: clamp, mix, smoothstep
    Value args[kMaxArgs];
    SkASSERT(nargs >= 1 && nargs <= SK_ARRAY_COUNT(args));

    // All other intrinsics have at most three args, and those can all be evaluated up front:
    for (size_t i = 0; i < nargs; ++i) {
        args[i] = this->writeExpression(*c.arguments()[i]);
    }
    Type::NumberKind nk = base_number_kind(c.arguments()[0]->type());

    auto binary = [&](auto&& fn) {
        // Binary intrinsics are (vecN, vecN), (vecN, float), or (float, vecN)
        size_t nslots = std::max(args[0].slots(), args[1].slots());
        Value result(nslots);
        SkASSERT(args[0].slots() == nslots || args[0].slots() == 1);
        SkASSERT(args[1].slots() == nslots || args[1].slots() == 1);

        for (size_t i = 0; i < nslots; ++i) {
            result[i] = fn({fBuilder, args[0][args[0].slots() == 1 ? 0 : i]},
                           {fBuilder, args[1][args[1].slots() == 1 ? 0 : i]});
        }
        return result;
    };

    auto ternary = [&](auto&& fn) {
        // Ternary intrinsics are some combination of vecN and float
        size_t nslots = std::max({args[0].slots(), args[1].slots(), args[2].slots()});
        Value result(nslots);
        SkASSERT(args[0].slots() == nslots || args[0].slots() == 1);
        SkASSERT(args[1].slots() == nslots || args[1].slots() == 1);
        SkASSERT(args[2].slots() == nslots || args[2].slots() == 1);

        for (size_t i = 0; i < nslots; ++i) {
            result[i] = fn({fBuilder, args[0][args[0].slots() == 1 ? 0 : i]},
                           {fBuilder, args[1][args[1].slots() == 1 ? 0 : i]},
                           {fBuilder, args[2][args[2].slots() == 1 ? 0 : i]});
        }
        return result;
    };

    auto dot = [&](const Value& x, const Value& y) {
        SkASSERT(x.slots() == y.slots());
        skvm::F32 result = f32(x[0]) * f32(y[0]);
        for (size_t i = 1; i < x.slots(); ++i) {
            result += f32(x[i]) * f32(y[i]);
        }
        return result;
    };

    switch (found->second) {
        case Intrinsic::kRadians:
            return unary(args[0], [](skvm::F32 deg) { return deg * (SK_FloatPI / 180); });
        case Intrinsic::kDegrees:
            return unary(args[0], [](skvm::F32 rad) { return rad * (180 / SK_FloatPI); });

        case Intrinsic::kSin: return unary(args[0], skvm::approx_sin);
        case Intrinsic::kCos: return unary(args[0], skvm::approx_cos);
        case Intrinsic::kTan: return unary(args[0], skvm::approx_tan);

        case Intrinsic::kASin: return unary(args[0], skvm::approx_asin);
        case Intrinsic::kACos: return unary(args[0], skvm::approx_acos);

        case Intrinsic::kATan: return nargs == 1 ? unary(args[0], skvm::approx_atan)
                                                 : binary(skvm::approx_atan2);

        case Intrinsic::kPow:
            return binary([](skvm::F32 x, skvm::F32 y) { return skvm::approx_powf(x, y); });
        case Intrinsic::kExp:  return unary(args[0], skvm::approx_exp);
        case Intrinsic::kLog:  return unary(args[0], skvm::approx_log);
        case Intrinsic::kExp2: return unary(args[0], skvm::approx_pow2);
        case Intrinsic::kLog2: return unary(args[0], skvm::approx_log2);

        case Intrinsic::kSqrt: return unary(args[0], skvm::sqrt);
        case Intrinsic::kInverseSqrt:
            return unary(args[0], [](skvm::F32 x) { return 1.0f / skvm::sqrt(x); });

        case Intrinsic::kAbs: return unary(args[0], skvm::abs);
        case Intrinsic::kSign:
            return unary(args[0], [](skvm::F32 x) { return select(x < 0, -1.0f,
                                                           select(x > 0, +1.0f, 0.0f)); });
        case Intrinsic::kFloor: return unary(args[0], skvm::floor);
        case Intrinsic::kCeil:  return unary(args[0], skvm::ceil);
        case Intrinsic::kFract: return unary(args[0], skvm::fract);
        case Intrinsic::kMod:
            return binary([](skvm::F32 x, skvm::F32 y) { return x - y*skvm::floor(x / y); });

        case Intrinsic::kMin:
            return binary([](skvm::F32 x, skvm::F32 y) { return skvm::min(x, y); });
        case Intrinsic::kMax:
            return binary([](skvm::F32 x, skvm::F32 y) { return skvm::max(x, y); });
        case Intrinsic::kClamp:
            return ternary(
                    [](skvm::F32 x, skvm::F32 lo, skvm::F32 hi) { return skvm::clamp(x, lo, hi); });
        case Intrinsic::kSaturate:
            return unary(args[0], [](skvm::F32 x) { return skvm::clamp01(x); });
        case Intrinsic::kMix:
            return ternary(
                    [](skvm::F32 x, skvm::F32 y, skvm::F32 t) { return skvm::lerp(x, y, t); });
        case Intrinsic::kStep:
            return binary([](skvm::F32 edge, skvm::F32 x) { return select(x < edge, 0.0f, 1.0f); });
        case Intrinsic::kSmoothstep:
            return ternary([](skvm::F32 edge0, skvm::F32 edge1, skvm::F32 x) {
                skvm::F32 t = skvm::clamp01((x - edge0) / (edge1 - edge0));
                return t ** t ** (3 - 2 ** t);
            });

        case Intrinsic::kLength: return skvm::sqrt(dot(args[0], args[0]));
        case Intrinsic::kDistance: {
            Value vec = binary([](skvm::F32 x, skvm::F32 y) { return x - y; });
            return skvm::sqrt(dot(vec, vec));
        }
        case Intrinsic::kDot: return dot(args[0], args[1]);
        case Intrinsic::kCross: {
            skvm::F32 ax = f32(args[0][0]), ay = f32(args[0][1]), az = f32(args[0][2]),
                      bx = f32(args[1][0]), by = f32(args[1][1]), bz = f32(args[1][2]);
            Value result(3);
            result[0] = ay**bz - az**by;
            result[1] = az**bx - ax**bz;
            result[2] = ax**by - ay**bx;
            return result;
        }
        case Intrinsic::kNormalize: {
            skvm::F32 invLen = 1.0f / skvm::sqrt(dot(args[0], args[0]));
            return unary(args[0], [&](skvm::F32 x) { return x ** invLen; });
        }
        case Intrinsic::kFaceforward: {
            const Value &N    = args[0],
                        &I    = args[1],
                        &Nref = args[2];

            skvm::F32 dotNrefI = dot(Nref, I);
            return unary(N, [&](skvm::F32 n) { return select(dotNrefI<0, n, -n); });
        }
        case Intrinsic::kReflect: {
            const Value &I = args[0],
                        &N = args[1];

            skvm::F32 dotNI = dot(N, I);
            return binary([&](skvm::F32 i, skvm::F32 n) {
                return i - 2**dotNI**n;
            });
        }
        case Intrinsic::kRefract: {
            const Value &I  = args[0],
                        &N  = args[1];
            skvm::F32   eta = f32(args[2]);

            skvm::F32 dotNI = dot(N, I),
                      k     = 1 - eta**eta**(1 - dotNI**dotNI);
            return binary([&](skvm::F32 i, skvm::F32 n) {
                return select(k<0, 0.0f, eta**i - (eta**dotNI + sqrt(k))**n);
            });
        }

        case Intrinsic::kMatrixCompMult:
            return binary([](skvm::F32 x, skvm::F32 y) { return x ** y; });
        case Intrinsic::kInverse: {
            switch (args[0].slots()) {
                case  4: return this->writeMatrixInverse2x2(args[0]);
                case  9: return this->writeMatrixInverse3x3(args[0]);
                case 16: return this->writeMatrixInverse4x4(args[0]);
                default:
                    SkDEBUGFAIL("Invalid call to inverse");
                    return {};
            }
        }

        case Intrinsic::kLessThan:
            return nk == Type::NumberKind::kFloat
                           ? binary([](skvm::F32 x, skvm::F32 y) { return x < y; })
                           : binary([](skvm::I32 x, skvm::I32 y) { return x < y; });
        case Intrinsic::kLessThanEqual:
            return nk == Type::NumberKind::kFloat
                           ? binary([](skvm::F32 x, skvm::F32 y) { return x <= y; })
                           : binary([](skvm::I32 x, skvm::I32 y) { return x <= y; });
        case Intrinsic::kGreaterThan:
            return nk == Type::NumberKind::kFloat
                           ? binary([](skvm::F32 x, skvm::F32 y) { return x > y; })
                           : binary([](skvm::I32 x, skvm::I32 y) { return x > y; });
        case Intrinsic::kGreaterThanEqual:
            return nk == Type::NumberKind::kFloat
                           ? binary([](skvm::F32 x, skvm::F32 y) { return x >= y; })
                           : binary([](skvm::I32 x, skvm::I32 y) { return x >= y; });

        case Intrinsic::kEqual:
            return nk == Type::NumberKind::kFloat
                           ? binary([](skvm::F32 x, skvm::F32 y) { return x == y; })
                           : binary([](skvm::I32 x, skvm::I32 y) { return x == y; });
        case Intrinsic::kNotEqual:
            return nk == Type::NumberKind::kFloat
                           ? binary([](skvm::F32 x, skvm::F32 y) { return x != y; })
                           : binary([](skvm::I32 x, skvm::I32 y) { return x != y; });

        case Intrinsic::kAny: {
            skvm::I32 result = i32(args[0][0]);
            for (size_t i = 1; i < args[0].slots(); ++i) {
                result |= i32(args[0][i]);
            }
            return result;
        }
        case Intrinsic::kAll: {
            skvm::I32 result = i32(args[0][0]);
            for (size_t i = 1; i < args[0].slots(); ++i) {
                result &= i32(args[0][i]);
            }
            return result;
        }
        case Intrinsic::kNot: return unary(args[0], [](skvm::I32 x) { return ~x; });

        case Intrinsic::kSample:
            // Handled earlier
            SkASSERT(false);
            return {};
    }
    SkUNREACHABLE;
}

Value SkVMGenerator::writeFunctionCall(const FunctionCall& f) {
    if (f.function().isBuiltin() && !f.function().definition()) {
        return this->writeIntrinsicCall(f);
    }

    const FunctionDeclaration& decl = f.function();

    // Evaluate all arguments, gather the results into a contiguous list of IDs
    std::vector<skvm::Val> argVals;
    for (const auto& arg : f.arguments()) {
        Value v = this->writeExpression(*arg);
        for (size_t i = 0; i < v.slots(); ++i) {
            argVals.push_back(v[i]);
        }
    }

    // Create storage for the return value
    size_t nslots = slot_count(f.type());
    Value result(nslots);
    for (size_t i = 0; i < nslots; ++i) {
        result[i] = fBuilder->splat(0.0f);
    }

    {
        // This merges currentFunction().fReturned into fConditionMask. Lanes that conditionally
        // returned in the current function would otherwise resume execution within the child.
        ScopedCondition m(this, ~currentFunction().fReturned);
        this->writeFunction(*f.function().definition(), argVals, result.asSpan());
    }

    // Propagate new values of any 'out' params back to the original arguments
    const std::unique_ptr<Expression>* argIter = f.arguments().begin();
    size_t valIdx = 0;
    for (const Variable* p : decl.parameters()) {
        size_t nslots = slot_count(p->type());
        if (p->modifiers().fFlags & Modifiers::kOut_Flag) {
            Value v(nslots);
            for (size_t i = 0; i < nslots; ++i) {
                v[i] = argVals[valIdx + i];
            }
            const std::unique_ptr<Expression>& arg = *argIter;
            this->writeStore(*arg, v);
        }
        valIdx += nslots;
        argIter++;
    }

    return result;
}

Value SkVMGenerator::writeExternalFunctionCall(const ExternalFunctionCall& c) {
    // Evaluate all arguments, gather the results into a contiguous list of F32
    std::vector<skvm::F32> args;
    for (const auto& arg : c.arguments()) {
        Value v = this->writeExpression(*arg);
        for (size_t i = 0; i < v.slots(); ++i) {
            args.push_back(f32(v[i]));
        }
    }

    // Create storage for the return value
    size_t nslots = slot_count(c.type());
    std::vector<skvm::F32> result(nslots, fBuilder->splat(0.0f));

    c.function().call(fBuilder, args.data(), result.data(), this->mask());

    // Convert from 'vector of F32' to Value
    Value resultVal(nslots);
    for (size_t i = 0; i < nslots; ++i) {
        resultVal[i] = result[i];
    }

    return resultVal;
}

Value SkVMGenerator::writePrefixExpression(const PrefixExpression& p) {
    Value val = this->writeExpression(*p.operand());

    switch (p.getOperator().kind()) {
        case Token::Kind::TK_PLUSPLUS:
        case Token::Kind::TK_MINUSMINUS: {
            bool incr = p.getOperator().kind() == Token::Kind::TK_PLUSPLUS;

            switch (base_number_kind(p.type())) {
                case Type::NumberKind::kFloat:
                    val = f32(val) + fBuilder->splat(incr ? 1.0f : -1.0f);
                    break;
                case Type::NumberKind::kSigned:
                    val = i32(val) + fBuilder->splat(incr ? 1 : -1);
                    break;
                default:
                    SkASSERT(false);
                    return {};
            }
            return this->writeStore(*p.operand(), val);
        }
        case Token::Kind::TK_MINUS: {
            switch (base_number_kind(p.type())) {
                case Type::NumberKind::kFloat:
                    return this->unary(val, [](skvm::F32 x) { return -x; });
                case Type::NumberKind::kSigned:
                    return this->unary(val, [](skvm::I32 x) { return -x; });
                default:
                    SkASSERT(false);
                    return {};
            }
        }
        case Token::Kind::TK_LOGICALNOT:
        case Token::Kind::TK_BITWISENOT:
            return this->unary(val, [](skvm::I32 x) { return ~x; });
        default:
            SkASSERT(false);
            return {};
    }
}

Value SkVMGenerator::writePostfixExpression(const PostfixExpression& p) {
    switch (p.getOperator().kind()) {
        case Token::Kind::TK_PLUSPLUS:
        case Token::Kind::TK_MINUSMINUS: {
            Value old = this->writeExpression(*p.operand()),
                  val = old;
            SkASSERT(val.slots() == 1);
            bool incr = p.getOperator().kind() == Token::Kind::TK_PLUSPLUS;

            switch (base_number_kind(p.type())) {
                case Type::NumberKind::kFloat:
                    val = f32(val) + fBuilder->splat(incr ? 1.0f : -1.0f);
                    break;
                case Type::NumberKind::kSigned:
                    val = i32(val) + fBuilder->splat(incr ? 1 : -1);
                    break;
                default:
                    SkASSERT(false);
                    return {};
            }
            this->writeStore(*p.operand(), val);
            return old;
        }
        default:
            SkASSERT(false);
            return {};
    }
}

Value SkVMGenerator::writeSwizzle(const Swizzle& s) {
    Value base = this->writeExpression(*s.base());
    Value swizzled(s.components().size());
    for (size_t i = 0; i < s.components().size(); ++i) {
        swizzled[i] = base[s.components()[i]];
    }
    return swizzled;
}

Value SkVMGenerator::writeTernaryExpression(const TernaryExpression& t) {
    skvm::I32 test = i32(this->writeExpression(*t.test()));
    Value ifTrue, ifFalse;

    {
        ScopedCondition m(this, test);
        ifTrue = this->writeExpression(*t.ifTrue());
    }
    {
        ScopedCondition m(this, ~test);
        ifFalse = this->writeExpression(*t.ifFalse());
    }

    size_t nslots = ifTrue.slots();
    SkASSERT(nslots == ifFalse.slots());

    Value result(nslots);
    for (size_t i = 0; i < nslots; ++i) {
        result[i] = skvm::select(test, i32(ifTrue[i]), i32(ifFalse[i]));
    }
    return result;
}

Value SkVMGenerator::writeExpression(const Expression& e) {
    switch (e.kind()) {
        case Expression::Kind::kBinary:
            return this->writeBinaryExpression(e.as<BinaryExpression>());
        case Expression::Kind::kBoolLiteral:
            return fBuilder->splat(e.as<BoolLiteral>().value() ? ~0 : 0);
        case Expression::Kind::kConstructor:
            return this->writeConstructor(e.as<Constructor>());
        case Expression::Kind::kFieldAccess:
            return this->writeFieldAccess(e.as<FieldAccess>());
        case Expression::Kind::kIndex:
            return this->writeIndexExpression(e.as<IndexExpression>());
        case Expression::Kind::kVariableReference:
            return this->writeVariableExpression(e.as<VariableReference>());
        case Expression::Kind::kFloatLiteral:
            return fBuilder->splat(e.as<FloatLiteral>().value());
        case Expression::Kind::kFunctionCall:
            return this->writeFunctionCall(e.as<FunctionCall>());
        case Expression::Kind::kExternalFunctionCall:
            return this->writeExternalFunctionCall(e.as<ExternalFunctionCall>());
        case Expression::Kind::kIntLiteral:
            return fBuilder->splat(static_cast<int>(e.as<IntLiteral>().value()));
        case Expression::Kind::kPrefix:
            return this->writePrefixExpression(e.as<PrefixExpression>());
        case Expression::Kind::kPostfix:
            return this->writePostfixExpression(e.as<PostfixExpression>());
        case Expression::Kind::kSwizzle:
            return this->writeSwizzle(e.as<Swizzle>());
        case Expression::Kind::kTernary:
            return this->writeTernaryExpression(e.as<TernaryExpression>());
        case Expression::Kind::kExternalFunctionReference:
        default:
            SkDEBUGFAIL("Unsupported expression");
            return {};
    }
}

Value SkVMGenerator::writeStore(const Expression& lhs, const Value& rhs) {
    SkASSERTF(rhs.slots() == slot_count(lhs.type()),
              "lhs=%s (%s)\nrhs=%d slot",
              lhs.type().description().c_str(), lhs.description().c_str(), rhs.slots());

    // We need to figure out the collection of slots that we're storing into. The l-value (lhs)
    // is always a VariableReference, possibly wrapped by one or more Swizzle, FieldAccess, or
    // IndexExpressions. The underlying VariableReference has a range of slots for its storage,
    // and each expression wrapped around that selects a sub-set of those slots (Field/Index),
    // or rearranges them (Swizzle).
    SkSTArray<4, size_t, true> slots;
    slots.resize(rhs.slots());

    // Start with the identity slot map - this basically says that the values from rhs belong in
    // slots [0, 1, 2 ... N] of the lhs.
    for (size_t i = 0; i < slots.size(); ++i) {
        slots[i] = i;
    }

    // Now, as we peel off each outer expression, adjust 'slots' to be the locations relative to
    // the next (inner) expression:
    const Expression* expr = &lhs;
    while (!expr->is<VariableReference>()) {
        switch (expr->kind()) {
            case Expression::Kind::kFieldAccess: {
                const FieldAccess& fld = expr->as<FieldAccess>();
                size_t offset = this->fieldSlotOffset(fld);
                for (size_t& s : slots) {
                    s += offset;
                }
                expr = fld.base().get();
            } break;
            case Expression::Kind::kIndex: {
                const IndexExpression& idx = expr->as<IndexExpression>();
                size_t offset = this->indexSlotOffset(idx);
                for (size_t& s : slots) {
                    s += offset;
                }
                expr = idx.base().get();
            } break;
            case Expression::Kind::kSwizzle: {
                const Swizzle& swz = expr->as<Swizzle>();
                for (size_t& s : slots) {
                    s = swz.components()[s];
                }
                expr = swz.base().get();
            } break;
            default:
                // No other kinds of expressions are valid in lvalues. (see Analysis::IsAssignable)
                SkDEBUGFAIL("Invalid expression type");
                return {};
        }
    }

    // When we get here, 'slots' are all relative to the first slot holding 'var's storage
    const Variable& var = *expr->as<VariableReference>().variable();
    size_t varSlot = this->getSlot(var);
    skvm::I32 mask = this->mask();
    for (size_t i = rhs.slots(); i --> 0;) {
        SkASSERT(slots[i] < slot_count(var.type()));
        skvm::F32 curr = f32(fSlots[varSlot + slots[i]]),
                  next = f32(rhs[i]);
        fSlots[varSlot + slots[i]] = select(mask, next, curr).id;
    }
    return rhs;
}

void SkVMGenerator::writeBlock(const Block& b) {
    for (const std::unique_ptr<Statement>& stmt : b.children()) {
        this->writeStatement(*stmt);
    }
}

void SkVMGenerator::writeBreakStatement() {
    // Any active lanes stop executing for the duration of the current loop
    fLoopMask &= ~this->mask();
}

void SkVMGenerator::writeContinueStatement() {
    // Any active lanes stop executing for the current iteration.
    // Remember them in fContinueMask, to be re-enabled later.
    skvm::I32 mask = this->mask();
    fLoopMask &= ~mask;
    fContinueMask |= mask;
}

void SkVMGenerator::writeForStatement(const ForStatement& f) {
    // We require that all loops be ES2-compliant (unrollable), and actually unroll them here
    Analysis::UnrollableLoopInfo loop;
    SkAssertResult(Analysis::ForLoopIsValidForES2(f.fOffset, f.initializer().get(), f.test().get(),
                                                  f.next().get(), f.statement().get(), &loop,
                                                  /*errors=*/nullptr));
    SkASSERT(slot_count(loop.fIndex->type()) == 1);

    size_t indexSlot = this->getSlot(*loop.fIndex);
    double val = loop.fStart;

    skvm::I32 oldLoopMask     = fLoopMask,
              oldContinueMask = fContinueMask;

    for (int i = 0; i < loop.fCount; ++i) {
        fSlots[indexSlot] = loop.fIndex->type().isInteger()
                                    ? fBuilder->splat(static_cast<int>(val)).id
                                    : fBuilder->splat(static_cast<float>(val)).id;

        fContinueMask = fBuilder->splat(0);
        this->writeStatement(*f.statement());
        fLoopMask |= fContinueMask;

        val += loop.fDelta;
    }

    fLoopMask     = oldLoopMask;
    fContinueMask = oldContinueMask;
}

void SkVMGenerator::writeIfStatement(const IfStatement& i) {
    Value test = this->writeExpression(*i.test());
    {
        ScopedCondition ifTrue(this, i32(test));
        this->writeStatement(*i.ifTrue());
    }
    if (i.ifFalse()) {
        ScopedCondition ifFalse(this, ~i32(test));
        this->writeStatement(*i.ifFalse());
    }
}

void SkVMGenerator::writeReturnStatement(const ReturnStatement& r) {
    skvm::I32 returnsHere = this->mask();

    if (r.expression()) {
        Value val = this->writeExpression(*r.expression());

        int i = 0;
        for (skvm::Val& slot : currentFunction().fReturnValue) {
            slot = select(returnsHere, f32(val[i]), f32(slot)).id;
            i++;
        }
    }

    currentFunction().fReturned |= returnsHere;
}

void SkVMGenerator::writeVarDeclaration(const VarDeclaration& decl) {
    size_t slot   = this->getSlot(decl.var()),
           nslots = slot_count(decl.var().type());

    Value val = decl.value() ? this->writeExpression(*decl.value()) : Value{};
    for (size_t i = 0; i < nslots; ++i) {
        fSlots[slot + i] = val ? val[i] : fBuilder->splat(0.0f).id;
    }
}

void SkVMGenerator::writeStatement(const Statement& s) {
    switch (s.kind()) {
        case Statement::Kind::kBlock:
            this->writeBlock(s.as<Block>());
            break;
        case Statement::Kind::kBreak:
            this->writeBreakStatement();
            break;
        case Statement::Kind::kContinue:
            this->writeContinueStatement();
            break;
        case Statement::Kind::kExpression:
            this->writeExpression(*s.as<ExpressionStatement>().expression());
            break;
        case Statement::Kind::kFor:
            this->writeForStatement(s.as<ForStatement>());
            break;
        case Statement::Kind::kIf:
            this->writeIfStatement(s.as<IfStatement>());
            break;
        case Statement::Kind::kReturn:
            this->writeReturnStatement(s.as<ReturnStatement>());
            break;
        case Statement::Kind::kVarDeclaration:
            this->writeVarDeclaration(s.as<VarDeclaration>());
            break;
        case Statement::Kind::kDiscard:
        case Statement::Kind::kDo:
        case Statement::Kind::kSwitch:
            SkDEBUGFAIL("Unsupported control flow");
            break;
        case Statement::Kind::kInlineMarker:
        case Statement::Kind::kNop:
            break;
        default:
            SkASSERT(false);
    }
}

skvm::Color ProgramToSkVM(const Program& program,
                          const FunctionDefinition& function,
                          skvm::Builder* builder,
                          SkSpan<skvm::Val> uniforms,
                          skvm::Coord device,
                          skvm::Coord local,
                          SampleChildFn sampleChild) {
    skvm::Val args[2] = {local.x.id, local.y.id};
    skvm::Val zero = builder->splat(0.0f).id;
    skvm::Val result[4] = {zero,zero,zero,zero};
    size_t paramSlots = 0;
    for (const SkSL::Variable* param : function.declaration().parameters()) {
        paramSlots += slot_count(param->type());
    }
    SkASSERT(paramSlots <= SK_ARRAY_COUNT(args));

    SkVMGenerator generator(program, builder, uniforms, device, local, std::move(sampleChild));
    generator.writeFunction(function, {args, paramSlots}, result);

    return skvm::Color{{builder, result[0]},
                       {builder, result[1]},
                       {builder, result[2]},
                       {builder, result[3]}};
}

bool ProgramToSkVM(const Program& program,
                   const FunctionDefinition& function,
                   skvm::Builder* b,
                   SkSpan<skvm::Val> uniforms,
                   SkVMSignature* outSignature) {
    SkVMSignature ignored,
                  *signature = outSignature ? outSignature : &ignored;

    std::vector<skvm::Ptr> argPtrs;
    std::vector<skvm::Val> argVals;

    for (const Variable* p : function.declaration().parameters()) {
        size_t slots = slot_count(p->type());
        signature->fParameterSlots += slots;
        for (size_t i = 0; i < slots; ++i) {
            argPtrs.push_back(b->varying<float>());
            argVals.push_back(b->loadF(argPtrs.back()).id);
        }
    }

    std::vector<skvm::Ptr> returnPtrs;
    std::vector<skvm::Val> returnVals;

    signature->fReturnSlots = slot_count(function.declaration().returnType());
    for (size_t i = 0; i < signature->fReturnSlots; ++i) {
        returnPtrs.push_back(b->varying<float>());
        returnVals.push_back(b->splat(0.0f).id);
    }

    skvm::Coord zeroCoord = {b->splat(0.0f), b->splat(0.0f)};
    SkVMGenerator generator(program, b, uniforms, /*device=*/zeroCoord, /*local=*/zeroCoord,
                            /*sampleChild=*/{});
    generator.writeFunction(function, argVals, returnVals);

    // generateCode has updated the contents of 'argVals' for any 'out' or 'inout' parameters.
    // Propagate those changes back to our varying buffers:
    size_t argIdx = 0;
    for (const Variable* p : function.declaration().parameters()) {
        size_t nslots = slot_count(p->type());
        if (p->modifiers().fFlags & Modifiers::kOut_Flag) {
            for (size_t i = 0; i < nslots; ++i) {
                b->storeF(argPtrs[argIdx + i], skvm::F32{b, argVals[argIdx + i]});
            }
        }
        argIdx += nslots;
    }

    // It's also updated the contents of 'returnVals' with the return value of the entry point.
    // Store that as well:
    for (size_t i = 0; i < signature->fReturnSlots; ++i) {
        b->storeF(returnPtrs[i], skvm::F32{b, returnVals[i]});
    }

    return true;
}

const FunctionDefinition* Program_GetFunction(const Program& program, const char* function) {
    for (const ProgramElement* e : program.elements()) {
        if (e->is<FunctionDefinition>() &&
            e->as<FunctionDefinition>().declaration().name() == function) {
            return &e->as<FunctionDefinition>();
        }
    }
    return nullptr;
}

static void gather_uniforms(UniformInfo* info, const Type& type, const String& name) {
    switch (type.typeKind()) {
        case Type::TypeKind::kStruct:
            for (const auto& f : type.fields()) {
                gather_uniforms(info, *f.fType, name + "." + f.fName);
            }
            break;
        case Type::TypeKind::kArray:
            for (int i = 0; i < type.columns(); ++i) {
                gather_uniforms(info, type.componentType(),
                                String::printf("%s[%d]", name.c_str(), i));
            }
            break;
        case Type::TypeKind::kScalar:
        case Type::TypeKind::kVector:
        case Type::TypeKind::kMatrix:
            info->fUniforms.push_back({name, base_number_kind(type), type.rows(), type.columns(),
                                       info->fUniformSlotCount});
            info->fUniformSlotCount += type.columns() * type.rows();
            break;
        default:
            break;
    }
}

std::unique_ptr<UniformInfo> Program_GetUniformInfo(const Program& program) {
    auto info = std::make_unique<UniformInfo>();
    for (const ProgramElement* e : program.elements()) {
        if (!e->is<GlobalVarDeclaration>()) {
            continue;
        }
        const GlobalVarDeclaration& decl = e->as<GlobalVarDeclaration>();
        const Variable& var = decl.declaration()->as<VarDeclaration>().var();
        if (var.modifiers().fFlags & Modifiers::kUniform_Flag) {
            gather_uniforms(info.get(), var.type(), var.name());
        }
    }
    return info;
}

/*
 * Testing utility function that emits program's "main" with a minimal harness. Used to create
 * representative skvm op sequences for SkSL tests.
 */
bool testingOnly_ProgramToSkVMShader(const Program& program, skvm::Builder* builder) {
    const SkSL::FunctionDefinition* main = Program_GetFunction(program, "main");
    if (!main) {
        return false;
    }

    size_t uniformSlots = 0;
    int childSlots = 0;
    for (const SkSL::ProgramElement* e : program.elements()) {
        if (e->is<GlobalVarDeclaration>()) {
            const GlobalVarDeclaration& decl = e->as<GlobalVarDeclaration>();
            const Variable& var = decl.declaration()->as<VarDeclaration>().var();
            if (var.type() == *program.fContext->fTypes.fFragmentProcessor) {
                childSlots++;
            } else if (is_uniform(var)) {
                uniformSlots += slot_count(var.type());
            }
        }
    }

    skvm::Uniforms uniforms(builder->uniform(), 0);

    auto new_uni = [&]() { return builder->uniformF(uniforms.pushF(0.0f)); };

    // Assume identity CTM
    skvm::Coord device = {pun_to_F32(builder->index()), new_uni()};
    skvm::Coord local  = device;

    struct Child {
        skvm::Uniform addr;
        skvm::I32     rowBytesAsPixels;
    };

    std::vector<Child> children;
    for (int i = 0; i < childSlots; ++i) {
        children.push_back({uniforms.pushPtr(nullptr), builder->uniform32(uniforms.push(0))});
    }

    auto sampleChild = [&](int i, skvm::Coord coord) {
        skvm::PixelFormat pixelFormat = skvm::SkColorType_to_PixelFormat(kRGBA_F32_SkColorType);
        skvm::I32 index  = trunc(coord.x);
                  index += trunc(coord.y) * children[i].rowBytesAsPixels;
        return gather(pixelFormat, children[i].addr, index);
    };

    std::vector<skvm::Val> uniformVals;
    for (size_t i = 0; i < uniformSlots; ++i) {
        uniformVals.push_back(new_uni().id);
    }

    skvm::Color result =
            SkSL::ProgramToSkVM(program, *main, builder, uniformVals, device, local, sampleChild);

    storeF(builder->varying<float>(), result.r);
    storeF(builder->varying<float>(), result.g);
    storeF(builder->varying<float>(), result.b);
    storeF(builder->varying<float>(), result.a);

    return true;

}

}  // namespace SkSL
