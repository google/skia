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
#include "src/sksl/ir/SkSLWhileStatement.h"

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
    explicit Value(int reserveCount) : fVals(reserveCount) {}
    Value(skvm::F32 x) : fVals({ x.id }) {}
    Value(skvm::I32 x) : fVals({ x.id }) {}

    explicit operator bool() const {
        return !fVals.empty() && std::all_of(fVals.begin(), fVals.end(), [](skvm::Val id) {
            return id != skvm::NA;
        });
    }

    void push_back(skvm::Val x) { fVals.push_back(x); }
    void push_back(skvm::F32 x) { fVals.push_back(x.id); }
    void push_back(skvm::I32 x) { fVals.push_back(x.id); }

    size_t size() const { return fVals.size(); }

    skvm::Val& operator[](int i)       { return fVals[i]; }
    skvm::Val  operator[](int i) const { return fVals[i]; }

    SkSTArray<4, skvm::Val, true> fVals;
};

}

class SkVMGenerator {
public:
    SkVMGenerator(const Program* program,
                  const FunctionDefinition* function,
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
        // Angle & Trigonometry
        // kRadians,  // defined in sksl_public
        // kDegrees,  // defined in sksl_public
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
        // kCross,        // defined in sksl_public
        kNormalize,
        // kFaceForward,  // defined in sksl_public
        // kReflect,      // defined in sksl_public
        // kRefract,      // defined in sksl_public

        // Matrix
        // kMatrixCompMult  // TODO (see sksl_public)
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

    using Location = size_t;
    static constexpr Location kInvalidLocation = ~static_cast<size_t>(0);

    /**
     * Returns the slot into which var should be stored, allocating space if necessary. Compound
     * variables (e.g. vectors) will consume more than one slot, with the getLocation return value
     * indicating where the first element should be stored.
     */
    Location getLocation(const Variable& v);

    /**
     * As above, but computes the address of an expression involving indexing & field access.
     * The address must not be dynamic (no computed indexing). If the address is known, it's
     * returned. If not, kInvalidLocation is returned.
     */
    Location getLocation(const Expression& e);

    skvm::F32 f32(skvm::Val id) { return {fBuilder, id}; }
    skvm::I32 i32(skvm::Val id) { return {fBuilder, id}; }

    // Shorthand for scalars
    skvm::F32 f32(const Value& v) { SkASSERT(v.size() == 1); return f32(v[0]); }
    skvm::I32 i32(const Value& v) { SkASSERT(v.size() == 1); return i32(v[0]); }

    template <typename Fn>
    Value unary_f(const Value& v, Fn&& fn) {
        Value result(v.size());
        for (size_t i = 0; i < v.size(); ++i) {
            result.push_back(fn(f32(v[i])));
        }
        return result;
    }

    template <typename Fn>
    Value unary_i(const Value& v, Fn&& fn) {
        Value result(v.size());
        for (size_t i = 0; i < v.size(); ++i) {
            result.push_back(fn(i32(v[i])));
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

    void writeStore(const Expression& lhs, const Value& rhs);

    Value writeMatrixInverse2x2(const Value& m);
    Value writeMatrixInverse3x3(const Value& m);
    Value writeMatrixInverse4x4(const Value& m);

    const Program& fProgram;
    const FunctionDefinition* fFunction;

    skvm::Builder* fBuilder;

    std::vector<skvm::Val> fVariables;
    skvm::Coord fDeviceCoord;
    skvm::Coord fLocalCoord;
    SampleChildFn fSampleChild;
    SkSpan<skvm::Val> fReturnValue;

    ErrorReporter& fErrors;

    skvm::I32 fMask;
    skvm::I32 fReturned;

    // [Variable, first slot in fVariables]
    std::unordered_map<const Variable*, Location> fVariableMap;

    const std::unordered_map<String, Intrinsic> fIntrinsics;

    friend class AutoMask;
};

class AutoMask {
public:
    AutoMask(SkVMGenerator* generator, skvm::I32 mask)
            : fGenerator(generator), fOldMask(fGenerator->fMask) {
        fGenerator->fMask &= mask;
    }

    ~AutoMask() {
        fGenerator->fMask = fOldMask;
    }

private:
    SkVMGenerator* fGenerator;
    skvm::I32 fOldMask;
};

static Type::NumberKind BaseNumberKind(const Type& type) {
    if (type.typeKind() == Type::TypeKind::kMatrix || type.typeKind() == Type::TypeKind::kVector) {
        return BaseNumberKind(type.componentType());
    }
    return type.numberKind();
}

static inline bool IsUniform(const SkSL::Variable& var) {
    return var.modifiers().fFlags & Modifiers::kUniform_Flag;
}

static size_t SlotCount(const Type& type) {
    switch (type.typeKind()) {
        case Type::TypeKind::kOther:
            return 0;
        case Type::TypeKind::kStruct: {
            size_t slots = 0;
            for (const auto& f : type.fields()) {
                slots += SlotCount(*f.fType);
            }
            return slots;
        }
        case Type::TypeKind::kArray:
            SkASSERT(type.columns() > 0);
            return type.columns() * SlotCount(type.componentType());
        default:
            return type.columns() * type.rows();
    }
}

SkVMGenerator::SkVMGenerator(const Program* program,
                             const FunctionDefinition* function,
                             skvm::Builder* builder,
                             SkSpan<skvm::Val> uniforms,
                             SkSpan<skvm::Val> params,
                             skvm::Coord device,
                             skvm::Coord local,
                             SampleChildFn sampleChild,
                             SkSpan<skvm::Val> outReturn,
                             ErrorReporter* errors)
        : fProgram(*program)
        , fFunction(function)
        , fBuilder(builder)
        , fDeviceCoord(device)
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
    SkASSERT(fFunction);
    fMask     = fBuilder->splat(0xffff'ffff);
    fReturned = fBuilder->splat(0);

    // First, copy the uniform IDs to our list of (all) variable IDs
    fVariables.insert(fVariables.end(), uniforms.begin(), uniforms.end());

    // Now, add all globals (including uniforms) to our variable list
    size_t uniformIdx = 0;
    size_t fpCount = 0;
    for (const ProgramElement* e : fProgram.elements()) {
        if (e->is<GlobalVarDeclaration>()) {
            const GlobalVarDeclaration& decl = e->as<GlobalVarDeclaration>();
            const Variable& var = decl.declaration()->as<VarDeclaration>().var();
            SkASSERT(fVariableMap.find(&var) == fVariableMap.end());

            // fragmentProcessors (eg, child shaders) just store their index in the variable map
            if (var.type() == *fProgram.fContext->fFragmentProcessor_Type) {
                fVariableMap[&var] = fpCount++;
                continue;
            }

            if (var.type().isOpaque() || var.isBuiltin()) {
                continue;
            }

            size_t count = SlotCount(var.type());

            if (IsUniform(var)) {
                // Compute where each uniform lives in the variable ID list
                SkASSERT(uniformIdx + count <= uniforms.size());
                fVariableMap[&var] = uniformIdx;
                uniformIdx += count;
            } else {
                // Allocate space for other globals, initialize them to zero
                size_t slot = fVariables.size();
                fVariables.resize(slot + count, fBuilder->splat(0.0f).id);
                fVariableMap[&var] = slot;
            }
        }
    }
    SkASSERT(uniformIdx == uniforms.size());

    const FunctionDeclaration& decl = fFunction->declaration();

    // Workaround for emscripten: size_t is ambiguous among the set of to_string overloads
    auto size_str = [](size_t s) { return to_string(static_cast<uint64_t>(s)); };

    // Ensure that outReturn (where we place the return values) is the correct size
    size_t returnCount = SlotCount(decl.returnType());
    if (fReturnValue.size() != returnCount) {
        fErrors.error(-1, "Expected " + size_str(returnCount) + " return slots, but received " +
                                  size_str(fReturnValue.size()));
        fFunction = nullptr;
    }

    // Copy parameter IDs to our list of (all) variable IDs
    size_t paramBase = fVariables.size(),
           paramSlot = paramBase;
    fVariables.insert(fVariables.end(), params.begin(), params.end());

    // Compute where each parameter variable lives in the variable ID list
    for (const Variable* p : decl.parameters()) {
        fVariableMap[p] = paramSlot;
        paramSlot += SlotCount(p->type());
    }

    if (paramSlot != fVariables.size()) {
        fErrors.error(-1, "Expected " + size_str(paramSlot - paramBase) +
                            " parameters, but received " + size_str(params.size()));
        fFunction = nullptr;
    }
}

bool SkVMGenerator::generateCode() {
    if (!fFunction) {
        return false;
    }
    this->writeStatement(*fFunction->body());
    return 0 == fErrors.errorCount();
}

SkVMGenerator::Location SkVMGenerator::getLocation(const Variable& v) {
    auto entry = fVariableMap.find(&v);
    if (entry != fVariableMap.end()) {
        return entry->second;
    }

    SkASSERT(!IsUniform(v));  // Should have been added at construction time

    size_t slot  = fVariables.size(),
           count = SlotCount(v.type());
    fVariables.resize(slot + count, fBuilder->splat(0.0f).id);
    fVariableMap[&v] = slot;
    return slot;
}

SkVMGenerator::Location SkVMGenerator::getLocation(const Expression& e) {
    switch (e.kind()) {
        case Expression::Kind::kFieldAccess: {
            const FieldAccess& f = e.as<FieldAccess>();
            Location baseLoc = this->getLocation(*f.base());
            size_t offset = 0;
            for (int i = 0; i < f.fieldIndex(); ++i) {
                offset += SlotCount(*f.base()->type().fields()[i].fType);
            }

            if (baseLoc == kInvalidLocation) {
                return kInvalidLocation;
            } else {
                return baseLoc + offset;
            }
        }
        case Expression::Kind::kIndex: {
            const IndexExpression& i = e.as<IndexExpression>();
            Location baseLoc = this->getLocation(*i.base());
            if (baseLoc == kInvalidLocation) {
                return kInvalidLocation;
            }

            const Expression& index = *i.index();
            if (!index.isCompileTimeConstant()) {
                fErrors.error(index.fOffset, "Array index must be a constant");
                return kInvalidLocation;
            }

            int64_t indexValue = index.getConstantInt();
            if (indexValue < 0 || indexValue >= i.base()->type().columns()) {
                fErrors.error(index.fOffset, "Array index out of bounds");
                return kInvalidLocation;
            }

            size_t stride = SlotCount(i.type());
            return baseLoc + indexValue * stride;
        }
        case Expression::Kind::kVariableReference:
            return this->getLocation(*e.as<VariableReference>().variable());
        default:
            SkASSERT(false);
            return kInvalidLocation;
    }
}

Value SkVMGenerator::writeBinaryExpression(const BinaryExpression& b) {
    const Expression& left = *b.left();
    const Expression& right = *b.right();
    Token::Kind op = b.getOperator();
    if (op == Token::Kind::TK_EQ) {
        Value val = this->writeExpression(right);
        this->writeStore(left, val);
        // There is a subtle distinction between returning 'val' and the "result" of writeStore
        // here. The result is subject to lane masking, val isn't. That shouldn't matter, though.
        // (The value propagated in dead lanes should be irrelevant).
        return val;
    }

    const Type& lType = left.type();
    const Type& rType = right.type();
    bool lVecOrMtx = (lType.isVector() || lType.isMatrix());
    bool rVecOrMtx = (rType.isVector() || rType.isMatrix());
    bool isAssignment = Compiler::IsAssignment(op);
    if (isAssignment) {
        op = Compiler::RemoveAssignment(op);
    }
    Type::NumberKind nk = BaseNumberKind(lType);

    // A few ops require special treatment:
    switch (op) {
        case Token::Kind::TK_LOGICALAND: {
            SkASSERT(nk == Type::NumberKind::kBoolean);
            Value lVal = this->writeExpression(left);

            AutoMask shortCircuit(this, i32(lVal));
            Value rVal = this->writeExpression(right);
            return i32(lVal) & i32(rVal);
        }
        case Token::Kind::TK_LOGICALOR: {
            SkASSERT(nk == Type::NumberKind::kBoolean);
            Value lVal = this->writeExpression(left);

            AutoMask shortCircuit(this, ~i32(lVal));
            Value rVal = this->writeExpression(right);
            return i32(lVal) | i32(rVal);
        }
        case Token::Kind::TK_SHL:
        case Token::Kind::TK_SHR: {
            SkASSERT(nk == Type::NumberKind::kSigned || nk == Type::NumberKind::kUnsigned);
            if (!right.isCompileTimeConstant()) {
                fErrors.error(right.fOffset, "Shift amounts must be constant");
                return {};
            }
            int64_t shift = right.getConstantInt();
            if (shift < 0 || shift > 31) {
                fErrors.error(right.fOffset, "Shift amount out of range");
                return {};
            }

            Value val = this->writeExpression(left);

            if (op == Token::Kind::TK_SHL) {
                val = shl(i32(val), shift);
            } else {
                val = lType.isSigned() ? sra(i32(val), shift)
                                       : shr(i32(val), shift);
            }

            if (isAssignment) {
                this->writeStore(left, val);
            }
            return val;
        }
        default:
            break;
    }

    // All of the other ops are simpler
    Value lVal = this->writeExpression(left),
          rVal = this->writeExpression(right);

    // Special case for M*V, V*M, M*M (but not V*V!)
    if (op == Token::Kind::TK_STAR && lVecOrMtx && rVecOrMtx &&
        !(lType.isVector() && rType.isVector())) {
        int rCols = rType.columns(),
            rRows = rType.rows(),
            lCols = lType.columns(),
            lRows = lType.rows();
        // M*V treats the vector as a column
        if (rType.isVector()) {
            std::swap(rCols, rRows);
        }
        SkASSERT(lCols == rRows);
        SkASSERT(SlotCount(b.type()) == static_cast<size_t>(lRows * rCols));
        Value result(lRows * rCols);
        for (int c = 0; c < rCols; ++c)
        for (int r = 0; r < lRows; ++r) {
            skvm::F32 sum = fBuilder->splat(0.0f);
            for (int j = 0; j < lCols; ++j) {
                sum += f32(lVal[j*lRows + r]) * f32(rVal[c*rRows + j]);
            }
            result.push_back(sum);
        }
        if (isAssignment) {
            this->writeStore(left, result);
        }
        return result;
    }

    size_t count = std::max(lVal.size(), rVal.size());
    Value result(count);

    // TODO: This treats all unsigned types as signed. Need to pick a policy. (Either produce errors
    // if a program uses unsigned types, or just silently convert everything to signed?)
    auto binary = [&](auto&& f_fn, auto&& i_fn) {
        for (size_t i = 0; i < count; ++i) {
            // If one side is scalar, replicate it to all channels
            skvm::Val l_id = lVal.size() == 1 ? lVal[0] : lVal[i],
                      r_id = rVal.size() == 1 ? rVal[0] : rVal[i];
            result.push_back(nk == Type::NumberKind::kFloat ? f_fn(f32(l_id), f32(r_id)).id
                                                            : i_fn(i32(l_id), i32(r_id)).id);
        }
    };

    auto unsupported_f = [&](skvm::F32, skvm::F32) {
        fErrors.error(b.fOffset, "operator not supported for floats");
        return skvm::F32{};
    };

    auto unsupported_i = [&](skvm::I32, skvm::I32) {
        fErrors.error(b.fOffset, "operator not supported for this type");
        return skvm::I32{};
    };

    switch (op) {
        case Token::Kind::TK_EQEQ: {
            binary([](skvm::F32 x, skvm::F32 y) { return x == y; },
                   [](skvm::I32 x, skvm::I32 y) { return x == y; });
            skvm::I32 folded = i32(result[0]);
            for (size_t i = 1; i < count; ++i) {
                folded &= i32(result[i]);
            }
            result = folded;
        } break;
        case Token::Kind::TK_NEQ: {
            binary([](skvm::F32 x, skvm::F32 y) { return x != y; },
                   [](skvm::I32 x, skvm::I32 y) { return x != y; });
            skvm::I32 folded = i32(result[0]);
            for (size_t i = 1; i < count; ++i) {
                folded |= i32(result[i]);
            }
            result = folded;
        } break;
        case Token::Kind::TK_GT:
            binary([](skvm::F32 x, skvm::F32 y) { return x > y; },
                   [](skvm::I32 x, skvm::I32 y) { return x > y; });
            break;
        case Token::Kind::TK_GTEQ:
            binary([](skvm::F32 x, skvm::F32 y) { return x >= y; },
                   [](skvm::I32 x, skvm::I32 y) { return x >= y; });
            break;
        case Token::Kind::TK_LT:
            binary([](skvm::F32 x, skvm::F32 y) { return x < y; },
                   [](skvm::I32 x, skvm::I32 y) { return x < y; });
            break;
        case Token::Kind::TK_LTEQ:
            binary([](skvm::F32 x, skvm::F32 y) { return x <= y; },
                   [](skvm::I32 x, skvm::I32 y) { return x <= y; });
            break;

        case Token::Kind::TK_PLUS:
            binary([](skvm::F32 x, skvm::F32 y) { return x + y; },
                   [](skvm::I32 x, skvm::I32 y) { return x + y; });
            break;
        case Token::Kind::TK_MINUS:
            binary([](skvm::F32 x, skvm::F32 y) { return x - y; },
                   [](skvm::I32 x, skvm::I32 y) { return x - y; });
            break;
        case Token::Kind::TK_STAR:
            binary([](skvm::F32 x, skvm::F32 y) { return x * y; },
                   [](skvm::I32 x, skvm::I32 y) { return x * y; });
            break;
        case Token::Kind::TK_SLASH:
            // TODO: Integer division
            binary([](skvm::F32 x, skvm::F32 y) { return x / y; },
                   unsupported_i);
            break;
        case Token::Kind::TK_PERCENT:
            // TODO: Integer modulus
            binary(unsupported_f, // Disallowed by SkSL
                   unsupported_i);
            break;

        case Token::Kind::TK_LOGICALXOR:
            binary(unsupported_f,
                   [](skvm::I32 x, skvm::I32 y) { return x ^ y; });
            break;

        case Token::Kind::TK_BITWISEAND:
            binary(unsupported_f,
                   [](skvm::I32 x, skvm::I32 y) { return x & y; });
            break;
        case Token::Kind::TK_BITWISEOR:
            binary(unsupported_f,
                   [](skvm::I32 x, skvm::I32 y) { return x | y; });
            break;
        case Token::Kind::TK_BITWISEXOR:
            binary(unsupported_f,
                   [](skvm::I32 x, skvm::I32 y) { return x ^ y; });
            break;

        default:
            fErrors.error(b.fOffset,
                          SkSL::String::printf("Unsupported binary operator '%s'",
                                               Compiler::OperatorName(op)));
            break;
    }

    if (isAssignment) {
        this->writeStore(left, result);
    }
    return result;
}

Value SkVMGenerator::writeConstructor(const Constructor& c) {
    if (c.arguments().size() == 1) {
        const Type& srcType = c.arguments()[0]->type();
        const Type& dstType = c.type();
        Type::NumberKind srcKind = BaseNumberKind(srcType),
                         dstKind = BaseNumberKind(dstType);
        Value src = this->writeExpression(*c.arguments()[0]);
        size_t dstSize = SlotCount(dstType);

        // Conversion among "similar" types (floatN <-> halfN), (shortN <-> intN), etc. is a no-op
        if (srcKind == dstKind && src.size() == dstSize) {
            return src;
        }

        // TODO: Handle signed vs. unsigned? Assuming unsigned never appears here.
        // TODO: Handle conversion to/from bool
        if (srcKind != dstKind) {
            // One argument constructors can do type conversion
            Value dst(src.size());
            if (dstKind == Type::NumberKind::kFloat) {
                SkASSERT(srcKind == Type::NumberKind::kSigned);
                for (auto id : src.fVals) {
                    dst.push_back(skvm::to_F32(i32(id)));
                }
            } else if (dstKind == Type::NumberKind::kSigned) {
                SkASSERT(srcKind == Type::NumberKind::kFloat);
                for (auto id : src.fVals) {
                    dst.push_back(skvm::trunc(f32(id)));
                }
            } else {
                fErrors.error(-1, "Unsupported type conversion");
            }
            return dst;
        }

        // Matrices can be constructed from scalars or other matrices
        if (dstType.isMatrix()) {
            Value dst(dstType.rows() * dstType.columns());
            if (srcType.isMatrix()) {
                // Matrix-from-matrix uses src where it overlaps, fills in missing with identity
                for (int c = 0; c < dstType.columns(); ++c)
                for (int r = 0; r < dstType.rows(); ++r) {
                    if (c < srcType.columns() && r < srcType.rows()) {
                        dst.push_back(src[c * srcType.rows() + r]);
                    } else {
                        dst.push_back(fBuilder->splat(c == r ? 1.0f : 0.0f));
                    }
                }
            } else if (srcType.isScalar()) {
                // Matrix-from-scalar builds a diagonal scale matrix
                for (int c = 0; c < dstType.columns(); ++c)
                for (int r = 0; r < dstType.rows(); ++r) {
                    dst.push_back(c == r ? f32(src) : fBuilder->splat(0.0f));
                }
            } else {
                fErrors.error(-1, "Unsupported matrix constructor");
            }
            return dst;
        }

        // We can splat scalars to all components of a vector
        if (dstType.isVector() && srcType.isScalar()) {
            Value dst(dstType.columns());
            for (int i = 0; i < dstType.columns(); ++i) {
                dst.push_back(src[0]);
            }
            return dst;
        }

        fErrors.error(-1, "Unsupported constructor usage");
        return {};
    } else {
        // Multi-argument constructors just aggregate their arguments, with no conversion:
        // NOTE: This (SkSL rule) is actually more restrictive than GLSL.
        Value result;
        for (const auto &arg : c.arguments()) {
            Value tmp = this->writeExpression(*arg);
            result.fVals.push_back_n(tmp.size(), tmp.fVals.begin());
        }
        return result;
    }
}

// If the expression is a reference to a builtin global variable, return the builtin ID.
// Otherwise, return -1.
static int expression_as_builtin(const Expression& e) {
    if (e.is<VariableReference>()) {
        const Variable& var(*e.as<VariableReference>().variable());
        if (var.storage() == Variable::Storage::kGlobal) {
            return var.modifiers().fLayout.fBuiltin;
        }
    }
    return -1;
}

Value SkVMGenerator::writeVariableExpression(const Expression& e) {
    if (int builtin = expression_as_builtin(e); builtin >= 0) {
        switch (builtin) {
            case SK_FRAGCOORD_BUILTIN: {
                Value result;
                result.push_back(fDeviceCoord.x);
                result.push_back(fDeviceCoord.y);
                result.push_back(fBuilder->splat(0.0f));
                result.push_back(fBuilder->splat(1.0f));
                return result;
            }
            default:
                fErrors.error(e.fOffset, "Unsupported builtin: " +
                                                 e.as<VariableReference>().variable()->name());
                return {};
        }
    }

    Location location = this->getLocation(e);
    size_t count = SlotCount(e.type());
    if (location == kInvalidLocation || count == 0) {
        return {};
    }
    Value val(count);
    for (size_t i = 0; i < count; ++i) {
        val.push_back(fVariables[location + i]);
    }
    return val;
}

Value SkVMGenerator::writeMatrixInverse2x2(const Value& m) {
    SkASSERT(m.size() == 4);
    skvm::F32 a = f32(m[0]),
              b = f32(m[1]),
              c = f32(m[2]),
              d = f32(m[3]);
    skvm::F32 idet = 1.0f / (a*d - b*c);

    Value result(m.size());
    result.push_back(d * idet);
    result.push_back(-b * idet);
    result.push_back(-c * idet);
    result.push_back(a * idet);
    return result;
}

Value SkVMGenerator::writeMatrixInverse3x3(const Value& m) {
    SkASSERT(m.size() == 9);
    skvm::F32 a11 = f32(m[0]), a12 = f32(m[3]), a13 = f32(m[6]),
              a21 = f32(m[1]), a22 = f32(m[4]), a23 = f32(m[7]),
              a31 = f32(m[2]), a32 = f32(m[5]), a33 = f32(m[8]);
    skvm::F32 idet = 1.0f / (a11 * a22 * a33 + a12 * a23 * a31 + a13 * a21 * a32 -
                             a11 * a23 * a32 - a12 * a21 * a33 - a13 * a22 * a31);

    Value result(m.size());
    result.push_back((a22 * a33 - a23 * a32) * idet);
    result.push_back((a23 * a31 - a21 * a33) * idet);
    result.push_back((a21 * a32 - a22 * a31) * idet);
    result.push_back((a13 * a32 - a12 * a33) * idet);
    result.push_back((a11 * a33 - a13 * a31) * idet);
    result.push_back((a12 * a31 - a11 * a32) * idet);
    result.push_back((a12 * a23 - a13 * a22) * idet);
    result.push_back((a13 * a21 - a11 * a23) * idet);
    result.push_back((a11 * a22 - a12 * a21) * idet);
    return result;
}

Value SkVMGenerator::writeMatrixInverse4x4(const Value& m) {
    SkASSERT(m.size() == 16);
    skvm::F32 a00 = f32(m[0]), a10 = f32(m[4]), a20 = f32(m[ 8]), a30 = f32(m[12]),
              a01 = f32(m[1]), a11 = f32(m[5]), a21 = f32(m[ 9]), a31 = f32(m[13]),
              a02 = f32(m[2]), a12 = f32(m[6]), a22 = f32(m[10]), a32 = f32(m[14]),
              a03 = f32(m[3]), a13 = f32(m[7]), a23 = f32(m[11]), a33 = f32(m[15]);

    skvm::F32 b00 = a00 * a11 - a01 * a10,
              b01 = a00 * a12 - a02 * a10,
              b02 = a00 * a13 - a03 * a10,
              b03 = a01 * a12 - a02 * a11,
              b04 = a01 * a13 - a03 * a11,
              b05 = a02 * a13 - a03 * a12,
              b06 = a20 * a31 - a21 * a30,
              b07 = a20 * a32 - a22 * a30,
              b08 = a20 * a33 - a23 * a30,
              b09 = a21 * a32 - a22 * a31,
              b10 = a21 * a33 - a23 * a31,
              b11 = a22 * a33 - a23 * a32;

    skvm::F32 idet = 1.0f / (b00 * b11 - b01 * b10 + b02 * b09 + b03 * b08 - b04 * b07 + b05 * b06);

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

    Value result(m.size());
    result.push_back(a11 * b11 - a12 * b10 + a13 * b09);
    result.push_back(a02 * b10 - a01 * b11 - a03 * b09);
    result.push_back(a31 * b05 - a32 * b04 + a33 * b03);
    result.push_back(a22 * b04 - a21 * b05 - a23 * b03);
    result.push_back(a12 * b08 - a10 * b11 - a13 * b07);
    result.push_back(a00 * b11 - a02 * b08 + a03 * b07);
    result.push_back(a32 * b02 - a30 * b05 - a33 * b01);
    result.push_back(a20 * b05 - a22 * b02 + a23 * b01);
    result.push_back(a10 * b10 - a11 * b08 + a13 * b06);
    result.push_back(a01 * b08 - a00 * b10 - a03 * b06);
    result.push_back(a30 * b04 - a31 * b02 + a33 * b00);
    result.push_back(a21 * b02 - a20 * b04 - a23 * b00);
    result.push_back(a11 * b07 - a10 * b09 - a12 * b06);
    result.push_back(a00 * b09 - a01 * b07 + a02 * b06);
    result.push_back(a31 * b01 - a30 * b03 - a32 * b00);
    result.push_back(a20 * b03 - a21 * b01 + a22 * b00);
    return result;
}

Value SkVMGenerator::writeIntrinsicCall(const FunctionCall& c) {
    auto found = fIntrinsics.find(c.function().name());
    if (found == fIntrinsics.end()) {
        fErrors.error(c.fOffset, "Unsupported intrinsic: '" + c.function().name() + "'");
        return {};
    }

    const size_t kMaxArgs = 3,
                 nargs    = c.arguments().size();
    Value args[kMaxArgs];
    SkASSERT(nargs >= 1 && nargs <= SK_ARRAY_COUNT(args));

    if (found->second == Intrinsic::kSample) {
        // Sample is very special, the first argument is an FP, which can't be evaluated
        const Context& ctx = *fProgram.fContext;
        if (nargs > 2 || c.arguments()[0]->type() != *ctx.fFragmentProcessor_Type ||
            (nargs == 2 && (c.arguments()[1]->type() != *ctx.fFloat2_Type &&
                            c.arguments()[1]->type() != *ctx.fFloat3x3_Type))) {
            fErrors.error(c.fOffset, "Unsupported form of sample");
            return {};
        }

        auto fp_it = fVariableMap.find(c.arguments()[0]->as<VariableReference>().variable());
        SkASSERT(fp_it != fVariableMap.end());

        skvm::Coord coord = fLocalCoord;
        if (nargs == 2) {
            Value arg = this->writeExpression(*c.arguments()[1]);
            if (arg.size() == 2) {
                // explicit sampling
                coord = {f32(arg[0]), f32(arg[1])};
            } else {
                // matrix sampling
                SkASSERT(arg.size() == 9);
                skvm::F32 x = f32(arg[0])*coord.x + f32(arg[3])*coord.y + f32(arg[6]),
                          y = f32(arg[1])*coord.x + f32(arg[4])*coord.y + f32(arg[7]),
                          w = f32(arg[2])*coord.x + f32(arg[5])*coord.y + f32(arg[8]);
                x = x * (1.0f / w);
                y = y * (1.0f / w);
                coord = {x, y};
            }
        }

        skvm::Color color = fSampleChild(fp_it->second, coord);
        Value result;
        result.push_back(color.r);
        result.push_back(color.g);
        result.push_back(color.b);
        result.push_back(color.a);
        return result;
    }

    // All other intrinsics have at most three args, and those can all be evaluated up front:
    for (size_t i = 0; i < nargs; ++i) {
        args[i] = this->writeExpression(*c.arguments()[i]);
    }
    Type::NumberKind nk = BaseNumberKind(c.arguments()[0]->type());

    auto unary = [&](auto&& fn) {
        size_t count = args[0].size();
        Value result(count);
        for (size_t i = 0; i < count; ++i) {
            result.push_back(fn({fBuilder, args[0][i]}));
        }
        return result;
    };

    auto binary = [&](auto&& fn) {
        // Binary intrinsics are (vecN, vecN), (vecN, float), or (float, vecN)
        size_t count = std::max(args[0].size(), args[1].size());
        Value result(count);
        SkASSERT(args[0].size() == count || args[0].size() == 1);
        SkASSERT(args[1].size() == count || args[1].size() == 1);

        for (size_t i = 0; i < count; ++i) {
            result.push_back(fn({fBuilder, args[0][std::min(i, args[0].size() - 1)]},
                                {fBuilder, args[1][std::min(i, args[1].size() - 1)]}));
        }
        return result;
    };

    auto ternary = [&](auto&& fn) {
        // Ternary intrinsics are some combination of vecN and float
        size_t count = std::max({args[0].size(), args[1].size(), args[2].size()});
        Value result(count);
        SkASSERT(args[0].size() == count || args[0].size() == 1);
        SkASSERT(args[1].size() == count || args[1].size() == 1);
        SkASSERT(args[2].size() == count || args[2].size() == 1);

        for (size_t i = 0; i < count; ++i) {
            result.push_back(fn({fBuilder, args[0][std::min(i, args[0].size() - 1)]},
                                {fBuilder, args[1][std::min(i, args[1].size() - 1)]},
                                {fBuilder, args[2][std::min(i, args[2].size() - 1)]}));
        }
        return result;
    };

    auto dot = [&](const Value& x, const Value& y) {
        SkASSERT(x.size() == y.size());
        skvm::F32 result = f32(x[0]) * f32(y[0]);
        for (size_t i = 1; i < x.size(); ++i) {
            result += f32(x[i]) * f32(y[i]);
        }
        return result;
    };

    switch (found->second) {
        case Intrinsic::kSin: return unary(skvm::approx_sin);
        case Intrinsic::kCos: return unary(skvm::approx_cos);
        case Intrinsic::kTan: return unary(skvm::approx_tan);

        case Intrinsic::kASin: return unary(skvm::approx_asin);
        case Intrinsic::kACos: return unary(skvm::approx_acos);

        case Intrinsic::kATan: return nargs == 1 ? unary(skvm::approx_atan)
                                                 : binary(skvm::approx_atan2);

        case Intrinsic::kPow:
            return binary([](skvm::F32 x, skvm::F32 y) { return skvm::approx_powf(x, y); });
        case Intrinsic::kExp:  return unary(skvm::approx_exp);
        case Intrinsic::kLog:  return unary(skvm::approx_log);
        case Intrinsic::kExp2: return unary(skvm::approx_pow2);
        case Intrinsic::kLog2: return unary(skvm::approx_log2);

        case Intrinsic::kSqrt: return unary(skvm::sqrt);
        case Intrinsic::kInverseSqrt:
            return unary([](skvm::F32 x) { return 1.0f / skvm::sqrt(x); });

        case Intrinsic::kAbs: return unary(skvm::abs);
        case Intrinsic::kSign:
            return unary([](skvm::F32 x) { return select(x < 0, -1.0f,
                                                  select(x > 0, +1.0f, 0.0f)); });
        case Intrinsic::kFloor: return unary(skvm::floor);
        case Intrinsic::kCeil:  return unary(skvm::ceil);
        case Intrinsic::kFract: return unary(skvm::fract);
        case Intrinsic::kMod:
            return binary([](skvm::F32 x, skvm::F32 y) { return x - y * skvm::floor(x / y); });

        case Intrinsic::kMin:
            return binary([](skvm::F32 x, skvm::F32 y) { return skvm::min(x, y); });
        case Intrinsic::kMax:
            return binary([](skvm::F32 x, skvm::F32 y) { return skvm::max(x, y); });
        case Intrinsic::kClamp:
            return ternary(
                    [](skvm::F32 x, skvm::F32 lo, skvm::F32 hi) { return skvm::clamp(x, lo, hi); });
        case Intrinsic::kSaturate:
            return unary([](skvm::F32 x) { return skvm::clamp01(x); });
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
            return unary([&](skvm::F32 x) { return x * invLen; });
        }

        case Intrinsic::kInverse: {
            switch (args[0].size()) {
                case  4: return this->writeMatrixInverse2x2(args[0]);
                case  9: return this->writeMatrixInverse3x3(args[0]);
                case 16: return this->writeMatrixInverse4x4(args[0]);
                default:
                    fErrors.error(c.fOffset, "Unsupported form of inverse()");
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
            for (size_t i = 1; i < args[0].size(); ++i) {
                result |= i32(args[0][i]);
            }
            return result;
        }
        case Intrinsic::kAll: {
            skvm::I32 result = i32(args[0][0]);
            for (size_t i = 1; i < args[0].size(); ++i) {
                result &= i32(args[0][i]);
            }
            return result;
        }
        case Intrinsic::kNot: return unary([](skvm::I32 x) { return ~x; });

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
    switch (p.getOperator()) {
        case Token::Kind::TK_PLUSPLUS:
        case Token::Kind::TK_MINUSMINUS: {
            SkASSERT(SlotCount(p.operand()->type()) == 1);
            Value val = this->writeExpression(*p.operand());
            bool incr = p.getOperator() == Token::Kind::TK_PLUSPLUS;

            switch (BaseNumberKind(p.type())) {
                case Type::NumberKind::kFloat: {
                    skvm::F32 delta = fBuilder->splat(incr ? 1.0f : -1.0f);
                    for (size_t i = 0; i < val.size(); ++i) {
                        val.fVals[i] = (f32(val[i]) + delta).id;
                    }
                } break;
                case Type::NumberKind::kSigned: {
                    skvm::I32 delta = fBuilder->splat(incr ? 1 : -1);
                    for (size_t i = 0; i < val.size(); ++i) {
                        val.fVals[i] = (i32(val[i]) + delta).id;
                    }
                } break;
                default:
                    SkASSERT(false);
                    return {};
            }
            this->writeStore(*p.operand(), val);
            return val;
        }
        case Token::Kind::TK_MINUS: {
            Value val = this->writeExpression(*p.operand());
            switch (BaseNumberKind(p.type())) {
                case Type::NumberKind::kFloat:
                    return this->unary_f(val, [](skvm::F32 x) { return -x; });
                case Type::NumberKind::kSigned:
                    return this->unary_i(val, [](skvm::I32 x) { return -x; });
                default:
                    SkASSERT(false);
                    return {};
            }
        }
        case Token::Kind::TK_LOGICALNOT:
        case Token::Kind::TK_BITWISENOT:
            return this->unary_i(this->writeExpression(*p.operand()),
                                 [](skvm::I32 x) { return ~x; });
        default:
            SkASSERT(false);
            return {};
    }
}

Value SkVMGenerator::writePostfixExpression(const PostfixExpression& p) {
    switch (p.getOperator()) {
        case Token::Kind::TK_PLUSPLUS:
        case Token::Kind::TK_MINUSMINUS: {
            SkASSERT(SlotCount(p.operand()->type()) == 1);
            Value old = this->writeExpression(*p.operand()),
                  val = old;
            bool incr = p.getOperator() == Token::Kind::TK_PLUSPLUS;

            switch (BaseNumberKind(p.type())) {
                case Type::NumberKind::kFloat: {
                    skvm::F32 delta = fBuilder->splat(incr ? 1.0f : -1.0f);
                    for (size_t i = 0; i < val.size(); ++i) {
                        val.fVals[i] = (f32(val[i]) + delta).id;
                    }
                } break;
                case Type::NumberKind::kSigned: {
                    skvm::I32 delta = fBuilder->splat(incr ? 1 : -1);
                    for (size_t i = 0; i < val.size(); ++i) {
                        val.fVals[i] = (i32(val[i]) + delta).id;
                    }
                } break;
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
    if (!base) {
        return {};
    }

    Value swizzled(s.components().size());
    for (int c : s.components()) {
        swizzled.push_back(base[c]);
    }
    return swizzled;
}

Value SkVMGenerator::writeTernaryExpression(const TernaryExpression& t) {
    Value test    = this->writeExpression(*t.test()),
          ifTrue  = this->writeExpression(*t.ifTrue()),
          ifFalse = this->writeExpression(*t.ifFalse());
    if (!test || !ifTrue || !ifFalse) {
        return {};
    }

    size_t count = ifTrue.size();
    SkASSERT(count == ifFalse.size());

    Value result(count);
    for (size_t i = 0; i < count; ++i) {
        result.push_back(skvm::select(i32(test), i32(ifTrue[i]), i32(ifFalse[i])));
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
            fErrors.error(e.fOffset, "Unsupported expression");
            return {};
    }
}

void SkVMGenerator::writeStore(const Expression& lhs, const Value& rhs) {
    SkASSERT(lhs.is<FieldAccess>() || lhs.is<IndexExpression>() || lhs.is<Swizzle>() ||
             lhs.is<VariableReference>());
    SkASSERT(rhs.size() == SlotCount(lhs.type()));

    skvm::I32 mask = this->mask();
    size_t count = rhs.size();
    for (int i = count; i --> 0;) {
        const Expression* expr = &lhs;
        int component = i;
        while (expr->is<Swizzle>()) {
            component = expr->as<Swizzle>().components()[component];
            expr = expr->as<Swizzle>().base().get();
        }
        Location base = this->getLocation(*expr);
        SkASSERT(base != kInvalidLocation);
        skvm::F32 curr = f32(fVariables[base + component]),
                  next = f32(rhs[i]);
        fVariables[base + component] = select(mask, next, curr).id;
    }
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

    for (size_t i = 0; i < val.size(); ++i) {
        fReturnValue[i] = select(returnsHere, f32(val[i]), f32(fReturnValue[i])).id;
    }

    fReturned |= returnsHere;
}

void SkVMGenerator::writeVarDeclaration(const VarDeclaration& decl) {
    // Always grab the location, to ensure space is allocated in fVariables
    Location base = this->getLocation(decl.var());
    if (decl.value()) {
        Value val = this->writeExpression(*decl.value());
        for (size_t i = 0; i < val.size(); ++i) {
            fVariables[base + i] = val[i];
        }
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
        case Statement::Kind::kWhile:
            fErrors.error(s.fOffset, "Unsupported control flow");
            break;
        case Statement::Kind::kInlineMarker:
        case Statement::Kind::kNop:
            break;
        default:
            SkASSERT(false);
    }
}

bool ProgramToSkVM(const Program* program,
                   const FunctionDefinition* function,
                   skvm::Builder* builder,
                   SkSpan<skvm::Val> uniforms,
                   SkSpan<skvm::Val> params,
                   skvm::Coord device,
                   skvm::Coord local,
                   SampleChildFn sampleChild,
                   SkSpan<skvm::Val> outReturn) {
    DebugfErrorReporter errors;
    SkVMGenerator generator(program, function, builder, uniforms, params, device, local,
                            std::move(sampleChild), outReturn, &errors);
    return generator.generateCode();
}

}  // namespace SkSL

#endif
