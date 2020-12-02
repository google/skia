/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/SkTArray.h"
#include "src/sksl/SkSLCodeGenerator.h"
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
#include "src/sksl/ir/SkSLExternalValueReference.h"
#include "src/sksl/ir/SkSLFieldAccess.h"
#include "src/sksl/ir/SkSLFloatLiteral.h"
#include "src/sksl/ir/SkSLForStatement.h"
#include "src/sksl/ir/SkSLFunctionCall.h"
#include "src/sksl/ir/SkSLFunctionDeclaration.h"
#include "src/sksl/ir/SkSLFunctionDefinition.h"
#include "src/sksl/ir/SkSLIfStatement.h"
#include "src/sksl/ir/SkSLIndexExpression.h"
#include "src/sksl/ir/SkSLIntLiteral.h"
#include "src/sksl/ir/SkSLNullLiteral.h"
#include "src/sksl/ir/SkSLPostfixExpression.h"
#include "src/sksl/ir/SkSLPrefixExpression.h"
#include "src/sksl/ir/SkSLProgramElement.h"
#include "src/sksl/ir/SkSLReturnStatement.h"
#include "src/sksl/ir/SkSLStatement.h"
#include "src/sksl/ir/SkSLSwitchStatement.h"
#include "src/sksl/ir/SkSLSwizzle.h"
#include "src/sksl/ir/SkSLTernaryExpression.h"
#include "src/sksl/ir/SkSLVarDeclarations.h"
#include "src/sksl/ir/SkSLVariableReference.h"

#include <algorithm>
#include <unordered_map>

#if !defined(SKSL_STANDALONE)

namespace SkSL {

namespace {

class DebugfErrorReporter : public ErrorReporter {
public:
    void error(int offset, String msg) override {
        SkDebugf("%s\n", msg.c_str());
        ++fErrorCount;
    }
    int errorCount() override { return fErrorCount; }

private:
    int fErrorCount = 0;
};

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

    ValRef    operator[](int i)       { return fVals[i]; }
    skvm::Val operator[](int i) const { return fVals[i]; }

private:
    SkSTArray<4, skvm::Val, true> fVals;
};

}  // namespace

class SkVMGenerator {
public:
    SkVMGenerator(const Program& program,
                  const FunctionDefinition& function,
                  skvm::Builder* builder,
                  SkSpan<skvm::Val> uniforms,
                  SkSpan<skvm::Val> params,
                  skvm::Coord device,
                  skvm::Coord local,
                  SampleChildFn sampleChild,
                  SkSpan<skvm::Val> outReturn,
                  ErrorReporter* errors);

    bool generateCode();

private:
    enum class Intrinsic {
        // sksl_public.sksl declares these intrinsics (and defines some other inline)

        // Angle & Trigonometry
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
        kNormalize,

        // Matrix
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

    using Slot = size_t;

    /**
     * In SkSL, a Variable represents a named, typed value (along with qualifiers, etc).
     * Every Variable is mapped to one (or several, contiguous) Slots -- indices into our vector of
     * skvm::Val. Those skvm::Val entries hold the current actual value of that variable.
     *
     * NOTE: Conceptually, each Variable is just mapped to a Value. We could implement it that way,
     * (and eliminate the Slot indirection), but it would add overhead for each Variable,
     * and add additional (different) bookkeeping for things like lvalue-swizzles.
     *
     * Any time a variable appears in an expression, that's a VariableReference, which is a kind of
     * Expression. Evaluating that VariableReference (or any other Expression) produces a Value,
     * which is a set of skvm::Val. (This allows an Expression to produce a vector or matrix, in
     * addition to a scalar).
     *
     * For a VariableReference, producing a Value is straightforward - we get the Slot of the
     * Variable, use that to look up the current skvm::Vals holding the variable's contents, and
     * construct a Value with those ids.
     */

    /**
     * Returns the Slot holding v's Val(s). Allocates storage if this is first time 'v' is
     * referenced. Compound variables (e.g. vectors) will consume more than one slot, with
     * getSlot returning the start of the contiguous chunk of slots.
     */
    Slot getSlot(const Variable& v);

    /**
     * As above, but computes the Slot of an expression involving indexing & field access.
     * The expression doesn't have separate storage - this returns the Slot of the underlying
     * Variable, with some offset applied to account for the indexing and field access.
     */
    Slot getSlot(const Expression& e);

    skvm::F32 f32(skvm::Val id) { return {fBuilder, id}; }
    skvm::I32 i32(skvm::Val id) { return {fBuilder, id}; }

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

    skvm::I32 mask() { return fMask; }

    Value writeExpression(const Expression& expr);
    Value writeBinaryExpression(const BinaryExpression& b);
    Value writeConstructor(const Constructor& c);
    Value writeFunctionCall(const FunctionCall& c);
    Value writeIntrinsicCall(const FunctionCall& c);
    Value writePostfixExpression(const PostfixExpression& p);
    Value writePrefixExpression(const PrefixExpression& p);
    Value writeSwizzle(const Swizzle& swizzle);
    Value writeTernaryExpression(const TernaryExpression& t);
    Value writeVariableExpression(const Expression& expr);

    void writeStatement(const Statement& s);
    void writeBlock(const Block& b);
    void writeIfStatement(const IfStatement& stmt);
    void writeReturnStatement(const ReturnStatement& r);
    void writeVarDeclaration(const VarDeclaration& decl);

    Value writeStore(const Expression& lhs, const Value& rhs);

    Value writeMatrixInverse2x2(const Value& m);
    Value writeMatrixInverse3x3(const Value& m);
    Value writeMatrixInverse4x4(const Value& m);

    const Program& fProgram;
    const FunctionDefinition& fFunction;

    skvm::Builder* fBuilder;

    std::vector<skvm::Val> fSlots;
    const skvm::Coord fLocalCoord;
    const SampleChildFn fSampleChild;
    const SkSpan<skvm::Val> fReturnValue;

    ErrorReporter& fErrors;

    skvm::I32 fMask;
    skvm::I32 fReturned;

    // [Variable, first slot in fSlots]
    std::unordered_map<const Variable*, Slot> fVariableMap;

    const std::unordered_map<String, Intrinsic> fIntrinsics;

    class AutoMask {
    public:
        AutoMask(SkVMGenerator* generator, skvm::I32 mask)
                : fGenerator(generator), fOldMask(fGenerator->fMask) {
            fGenerator->fMask &= mask;
        }

        ~AutoMask() { fGenerator->fMask = fOldMask; }

    private:
        SkVMGenerator* fGenerator;
        skvm::I32 fOldMask;
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
                             const FunctionDefinition& function,
                             skvm::Builder* builder,
                             SkSpan<skvm::Val> uniforms,
                             SkSpan<skvm::Val> params,
                             skvm::Coord device,
                             skvm::Coord local,
                             SampleChildFn sampleChild,
                             SkSpan<skvm::Val> outReturn,
                             ErrorReporter* errors)
        : fProgram(program)
        , fFunction(function)
        , fBuilder(builder)
        , fLocalCoord(local)
        , fSampleChild(std::move(sampleChild))
        , fReturnValue(outReturn)
        , fErrors(*errors)
        , fIntrinsics {
            { "sin", Intrinsic::kSin },
            { "cos", Intrinsic::kCos },
            { "tan", Intrinsic::kTan },
            { "asin", Intrinsic::kASin },
            { "acos", Intrinsic::kACos },
            { "atan", Intrinsic::kATan },

            { "pow", Intrinsic::kPow },
            { "exp", Intrinsic::kExp },
            { "log", Intrinsic::kLog },
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

            { "length",    Intrinsic::kLength },
            { "distance",  Intrinsic::kDistance },
            { "dot",       Intrinsic::kDot },
            { "normalize", Intrinsic::kNormalize },

            { "inverse", Intrinsic::kInverse },

            { "lessThan",         Intrinsic::kLessThan },
            { "lessThanEqual",    Intrinsic::kLessThanEqual },
            { "greaterThan",      Intrinsic::kGreaterThan },
            { "greaterThanEqual", Intrinsic::kGreaterThanEqual },
            { "equal",            Intrinsic::kEqual },
            { "notEqual",         Intrinsic::kNotEqual },

            { "any", Intrinsic::kAny },
            { "all", Intrinsic::kAll },
            { "not", Intrinsic::kNot },

            { "sample", Intrinsic::kSample },
        } {
    fMask     = fBuilder->splat(0xffff'ffff);
    fReturned = fBuilder->splat(0);

    // Now, add storage for each global variable (including uniforms) to fSlots, and entries in
    // fVariableMap to remember where every variable is stored.
    const skvm::Val* uniformIter = uniforms.begin();
    size_t fpCount = 0;
    for (const ProgramElement* e : fProgram.elements()) {
        if (e->is<GlobalVarDeclaration>()) {
            const GlobalVarDeclaration& decl = e->as<GlobalVarDeclaration>();
            const Variable& var = decl.declaration()->as<VarDeclaration>().var();
            SkASSERT(fVariableMap.find(&var) == fVariableMap.end());

            // For most variables, fVariableMap stores an index into fSlots, but for fragment
            // processors (child shaders), fVariableMap stores the index to pass to fSampleChild().
            if (var.type() == *fProgram.fContext->fFragmentProcessor_Type) {
                fVariableMap[&var] = fpCount++;
                continue;
            }

            // Opaque types include fragment processors, GL objects (samplers, textures, etc), and
            // special types like 'void'. Of those, only fragment processors are legal variables.
            SkASSERT(!var.type().isOpaque());

            size_t nslots = slot_count(var.type());
            fVariableMap[&var] = fSlots.size();

            if (int builtin = var.modifiers().fLayout.fBuiltin; builtin >= 0) {
                // builtin variables are system-defined, with special semantics. The only builtin
                // variable exposed to runtime effects is sk_FragCoord.
                switch (builtin) {
                    case SK_FRAGCOORD_BUILTIN:
                        SkASSERT(nslots == 4);
                        fSlots.insert(fSlots.end(), {device.x.id,
                                                     device.y.id,
                                                     fBuilder->splat(0.0f).id,
                                                     fBuilder->splat(1.0f).id});
                        break;
                    default:
                        SkDEBUGFAIL("Unsupported builtin");
                }
            } else if (is_uniform(var)) {
                // For uniforms, copy the supplied IDs over
                SkASSERT(uniformIter + nslots <= uniforms.end());
                fSlots.insert(fSlots.end(), uniformIter, uniformIter + nslots);
                uniformIter += nslots;
            } else {
                // For other globals, initialize them to zero
                fSlots.insert(fSlots.end(), nslots, fBuilder->splat(0.0f).id);
            }
        }
    }
    SkASSERT(uniformIter == uniforms.end());

    const FunctionDeclaration& decl = fFunction.declaration();

    // Ensure that outReturn (where we place the return values) is the correct size
    SkASSERT(slot_count(decl.returnType()) == fReturnValue.size());

    // Copy parameter IDs to our list of (all) variable IDs
    size_t paramBase = fSlots.size(),
           paramSlot = paramBase;
    fSlots.insert(fSlots.end(), params.begin(), params.end());

    // Compute where each parameter variable lives in the variable ID list
    for (const Variable* p : decl.parameters()) {
        fVariableMap[p] = paramSlot;
        paramSlot += slot_count(p->type());
    }
    SkASSERT(paramSlot == fSlots.size());
}

bool SkVMGenerator::generateCode() {
    this->writeStatement(*fFunction.body());
    return 0 == fErrors.errorCount();
}

SkVMGenerator::Slot SkVMGenerator::getSlot(const Variable& v) {
    auto entry = fVariableMap.find(&v);
    if (entry != fVariableMap.end()) {
        return entry->second;
    }

    SkASSERT(!is_uniform(v));  // Should have been added at construction time

    size_t slot   = fSlots.size(),
           nslots = slot_count(v.type());
    fSlots.resize(slot + nslots, fBuilder->splat(0.0f).id);
    fVariableMap[&v] = slot;
    return slot;
}

SkVMGenerator::Slot SkVMGenerator::getSlot(const Expression& e) {
    switch (e.kind()) {
        case Expression::Kind::kFieldAccess: {
            const FieldAccess& f = e.as<FieldAccess>();
            Slot slot = this->getSlot(*f.base());
            for (int i = 0; i < f.fieldIndex(); ++i) {
                slot += slot_count(*f.base()->type().fields()[i].fType);
            }
            return slot;
        }
        case Expression::Kind::kIndex: {
            const IndexExpression& i = e.as<IndexExpression>();
            Slot baseSlot = this->getSlot(*i.base());

            const Expression& index = *i.index();
            SkASSERT(index.isCompileTimeConstant());

            int64_t indexValue = index.getConstantInt();
            SkASSERT(indexValue >= 0 && indexValue < i.base()->type().columns());

            size_t stride = slot_count(i.type());
            return baseSlot + indexValue * stride;
        }
        case Expression::Kind::kVariableReference:
            return this->getSlot(*e.as<VariableReference>().variable());
        default:
            SkDEBUGFAIL("Invalid expression type");
            return ~static_cast<Slot>(0);
    }
}

Value SkVMGenerator::writeBinaryExpression(const BinaryExpression& b) {
    const Expression& left = *b.left();
    const Expression& right = *b.right();
    Token::Kind op = b.getOperator();
    if (op == Token::Kind::TK_EQ) {
        return this->writeStore(left, this->writeExpression(right));
    }

    const Type& lType = left.type();
    const Type& rType = right.type();
    bool lVecOrMtx = (lType.isVector() || lType.isMatrix());
    bool rVecOrMtx = (rType.isVector() || rType.isMatrix());
    bool isAssignment = Compiler::IsAssignment(op);
    if (isAssignment) {
        op = Compiler::RemoveAssignment(op);
    }
    Type::NumberKind nk = base_number_kind(lType);

    // A few ops require special treatment:
    switch (op) {
        case Token::Kind::TK_LOGICALAND: {
            SkASSERT(!isAssignment);
            SkASSERT(nk == Type::NumberKind::kBoolean);
            skvm::I32 lVal = i32(this->writeExpression(left));
            AutoMask shortCircuit(this, lVal);
            skvm::I32 rVal = i32(this->writeExpression(right));
            return lVal & rVal;
        }
        case Token::Kind::TK_LOGICALOR: {
            SkASSERT(!isAssignment);
            SkASSERT(nk == Type::NumberKind::kBoolean);
            skvm::I32 lVal = i32(this->writeExpression(left));
            AutoMask shortCircuit(this, ~lVal);
            skvm::I32 rVal = i32(this->writeExpression(right));
            return lVal | rVal;
        }
        default:
            break;
    }

    // All of the other ops always evaluate both sides of the expression
    Value lVal = this->writeExpression(left),
          rVal = this->writeExpression(right);

    // Special case for M*V, V*M, M*M (but not V*V!)
    if (op == Token::Kind::TK_STAR
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

    // TODO: This treats all unsigned types as signed. Need to pick a policy. (Either produce errors
    // if a program uses unsigned types, or just silently convert everything to signed?)
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

    switch (op) {
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
            return binary([](skvm::F32 x, skvm::F32 y) { return x * y; },
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

    // TODO: Handle conversion to/from bool
    // TODO: Handle signed vs. unsigned? GLSL ES 1.0 only has 'int', so no problem yet
    if (srcKind != dstKind) {
        // One argument constructors can do type conversion
        Value dst(src.slots());
        if (dstKind == Type::NumberKind::kFloat) {
            SkASSERT(srcKind == Type::NumberKind::kSigned);
            for (size_t i = 0; i < src.slots(); ++i) {
                dst[i] = skvm::to_F32(i32(src[i]));
            }
        } else if (dstKind == Type::NumberKind::kSigned) {
            SkASSERT(srcKind == Type::NumberKind::kFloat);
            for (size_t i = 0; i < src.slots(); ++i) {
                dst[i] = skvm::trunc(f32(src[i]));
            }
        } else {
            SkDEBUGFAIL("Unsupported type conversion");
        }
        return dst;
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

Value SkVMGenerator::writeVariableExpression(const Expression& e) {
    Slot slot = this->getSlot(e);
    Value val(slot_count(e.type()));
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
    result[0] = ( d * idet);
    result[1] = (-b * idet);
    result[2] = (-c * idet);
    result[3] = ( a * idet);
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
    result[0] = ((a22*a33 - a23*a32) * idet);
    result[1] = ((a23*a31 - a21*a33) * idet);
    result[2] = ((a21*a32 - a22*a31) * idet);
    result[3] = ((a13*a32 - a12*a33) * idet);
    result[4] = ((a11*a33 - a13*a31) * idet);
    result[5] = ((a12*a31 - a11*a32) * idet);
    result[6] = ((a12*a23 - a13*a22) * idet);
    result[7] = ((a13*a21 - a11*a23) * idet);
    result[8] = ((a11*a22 - a12*a21) * idet);
    return result;
}

Value SkVMGenerator::writeMatrixInverse4x4(const Value& m) {
    SkASSERT(m.slots() == 16);
    skvm::F32 a00 = f32(m[0]), a10 = f32(m[4]), a20 = f32(m[ 8]), a30 = f32(m[12]),
              a01 = f32(m[1]), a11 = f32(m[5]), a21 = f32(m[ 9]), a31 = f32(m[13]),
              a02 = f32(m[2]), a12 = f32(m[6]), a22 = f32(m[10]), a32 = f32(m[14]),
              a03 = f32(m[3]), a13 = f32(m[7]), a23 = f32(m[11]), a33 = f32(m[15]);

    skvm::F32 b00 = a00*a11 - a01*a10,
              b01 = a00*a12 - a02*a10,
              b02 = a00*a13 - a03*a10,
              b03 = a01*a12 - a02*a11,
              b04 = a01*a13 - a03*a11,
              b05 = a02*a13 - a03*a12,
              b06 = a20*a31 - a21*a30,
              b07 = a20*a32 - a22*a30,
              b08 = a20*a33 - a23*a30,
              b09 = a21*a32 - a22*a31,
              b10 = a21*a33 - a23*a31,
              b11 = a22*a33 - a23*a32;

    skvm::F32 idet = 1.0f / (b00*b11 - b01*b10 + b02*b09 + b03*b08 - b04*b07 + b05*b06);

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
    auto found = fIntrinsics.find(c.function().name());
    if (found == fIntrinsics.end()) {
        SkDEBUGFAILF("Missing intrinsic: '%s'", SkString(c.function().name()).c_str());
        return {};
    }

    const size_t nargs = c.arguments().size();

    if (found->second == Intrinsic::kSample) {
        // Sample is very special, the first argument is an FP, which can't be evaluated
        const Context& ctx = *fProgram.fContext;
        if (nargs > 2 || c.arguments()[0]->type() != *ctx.fFragmentProcessor_Type ||
            (nargs == 2 && (c.arguments()[1]->type() != *ctx.fFloat2_Type &&
                            c.arguments()[1]->type() != *ctx.fFloat3x3_Type))) {
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
                skvm::F32 x = f32(arg[0])*coord.x + f32(arg[3])*coord.y + f32(arg[6]),
                          y = f32(arg[1])*coord.x + f32(arg[4])*coord.y + f32(arg[7]),
                          w = f32(arg[2])*coord.x + f32(arg[5])*coord.y + f32(arg[8]);
                x = x * (1.0f / w);
                y = y * (1.0f / w);
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
                return t * t * (3 - 2 * t);
            });

        case Intrinsic::kLength: return skvm::sqrt(dot(args[0], args[0]));
        case Intrinsic::kDistance: {
            Value vec = binary([](skvm::F32 x, skvm::F32 y) { return x - y; });
            return skvm::sqrt(dot(vec, vec));
        }
        case Intrinsic::kDot: return dot(args[0], args[1]);
        case Intrinsic::kNormalize: {
            skvm::F32 invLen = 1.0f / skvm::sqrt(dot(args[0], args[0]));
            return unary(args[0], [&](skvm::F32 x) { return x * invLen; });
        }

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
            return binary([](skvm::F32 x, skvm::F32 y) { return x < y; });
        case Intrinsic::kLessThanEqual:
            return binary([](skvm::F32 x, skvm::F32 y) { return x <= y; });
        case Intrinsic::kGreaterThan:
            return binary([](skvm::F32 x, skvm::F32 y) { return x > y; });
        case Intrinsic::kGreaterThanEqual:
            return binary([](skvm::F32 x, skvm::F32 y) { return x >= y; });

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
    // TODO: Support calling other functions (by recursively generating their programs, eg inlining)
    if (f.function().isBuiltin()) {
        return this->writeIntrinsicCall(f);
    }

    fErrors.error(-1, "Function calls not supported yet");
    return {};
}

Value SkVMGenerator::writePrefixExpression(const PrefixExpression& p) {
    Value val = this->writeExpression(*p.operand());

    switch (p.getOperator()) {
        case Token::Kind::TK_PLUSPLUS:
        case Token::Kind::TK_MINUSMINUS: {
            bool incr = p.getOperator() == Token::Kind::TK_PLUSPLUS;

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
    switch (p.getOperator()) {
        case Token::Kind::TK_PLUSPLUS:
        case Token::Kind::TK_MINUSMINUS: {
            Value old = this->writeExpression(*p.operand()),
                  val = old;
            SkASSERT(val.slots() == 1);
            bool incr = p.getOperator() == Token::Kind::TK_PLUSPLUS;

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
        AutoMask m(this, test);
        ifTrue = this->writeExpression(*t.ifTrue());
    }
    {
        AutoMask m(this, ~test);
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
        case Expression::Kind::kIndex:
        case Expression::Kind::kVariableReference:
            return this->writeVariableExpression(e);
        case Expression::Kind::kFloatLiteral:
            return fBuilder->splat(e.as<FloatLiteral>().value());
        case Expression::Kind::kFunctionCall:
            return this->writeFunctionCall(e.as<FunctionCall>());
        case Expression::Kind::kIntLiteral:
            return fBuilder->splat(static_cast<int>(e.as<IntLiteral>().value()));
        case Expression::Kind::kNullLiteral:
            return fBuilder->splat(0);
        case Expression::Kind::kPrefix:
            return this->writePrefixExpression(e.as<PrefixExpression>());
        case Expression::Kind::kPostfix:
            return this->writePostfixExpression(e.as<PostfixExpression>());
        case Expression::Kind::kSwizzle:
            return this->writeSwizzle(e.as<Swizzle>());
        case Expression::Kind::kTernary:
            return this->writeTernaryExpression(e.as<TernaryExpression>());
        case Expression::Kind::kExternalFunctionCall:
        case Expression::Kind::kExternalValue:
        default:
            SkDEBUGFAIL("Unsupported expression");
            return {};
    }
}

Value SkVMGenerator::writeStore(const Expression& lhs, const Value& rhs) {
    SkASSERT(lhs.is<FieldAccess>() || lhs.is<IndexExpression>() || lhs.is<Swizzle>() ||
             lhs.is<VariableReference>());
    SkASSERT(rhs.slots() == slot_count(lhs.type()));

    skvm::I32 mask = this->mask();
    for (size_t i = rhs.slots(); i --> 0;) {
        const Expression* expr = &lhs;
        int component = i;
        while (expr->is<Swizzle>()) {
            component = expr->as<Swizzle>().components()[component];
            expr = expr->as<Swizzle>().base().get();
        }
        Slot slot = this->getSlot(*expr);
        skvm::F32 curr = f32(fSlots[slot + component]),
                  next = f32(rhs[i]);
        fSlots[slot + component] = select(mask, next, curr).id;
    }
    return rhs;
}

void SkVMGenerator::writeBlock(const Block& b) {
    for (const std::unique_ptr<Statement>& stmt : b.children()) {
        this->writeStatement(*stmt);
    }
}

void SkVMGenerator::writeIfStatement(const IfStatement& i) {
    Value test = this->writeExpression(*i.test());
    {
        AutoMask ifTrue(this, i32(test));
        this->writeStatement(*i.ifTrue());
    }
    if (i.ifFalse()) {
        AutoMask ifFalse(this, ~i32(test));
        this->writeStatement(*i.ifFalse());
    }
}

void SkVMGenerator::writeReturnStatement(const ReturnStatement& r) {
    // TODO: Can we suppress other side effects for lanes that have returned? fMask needs to
    // fold in knowledge of conditional returns earlier in the function.
    skvm::I32 returnsHere = bit_clear(this->mask(), fReturned);

    // TODO: returns with no expression
    Value val = this->writeExpression(*r.expression());

    for (size_t i = 0; i < val.slots(); ++i) {
        fReturnValue[i] = select(returnsHere, f32(val[i]), f32(fReturnValue[i])).id;
    }

    fReturned |= returnsHere;
}

void SkVMGenerator::writeVarDeclaration(const VarDeclaration& decl) {
    Slot slot = this->getSlot(decl.var());
    size_t nslots = slot_count(decl.var().type());

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
        case Statement::Kind::kExpression:
            this->writeExpression(*s.as<ExpressionStatement>().expression());
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
        case Statement::Kind::kBreak:
        case Statement::Kind::kContinue:
        case Statement::Kind::kDiscard:
        case Statement::Kind::kDo:
        case Statement::Kind::kFor:
        case Statement::Kind::kSwitch:
            fErrors.error(s.fOffset, "Unsupported control flow");
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
    DebugfErrorReporter errors;

    skvm::Val params[2] = {local.x.id, local.y.id};
    skvm::Val result[4] = {skvm::NA, skvm::NA, skvm::NA, skvm::NA};
    size_t paramSlots = 0;
    for (const SkSL::Variable* param : function.declaration().parameters()) {
        paramSlots += slot_count(param->type());
    }
    SkASSERT(paramSlots <= SK_ARRAY_COUNT(params));

    SkVMGenerator generator(program, function, builder, uniforms, {params, paramSlots},
                            device, local, std::move(sampleChild), result, &errors);

    return generator.generateCode() ? skvm::Color{{builder, result[0]},
                                                  {builder, result[1]},
                                                  {builder, result[2]},
                                                  {builder, result[3]}}
                                    : skvm::Color{};
}

}  // namespace SkSL

#endif
