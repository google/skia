/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLVMGenerator.h"

#include <algorithm>

#if !defined(SKSL_STANDALONE)

namespace SkSL {

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

SkVMGenerator::SkVMGenerator(const Context* context,
                             const Program* program,
                             ErrorReporter* errors,
                             skvm::Builder* builder,
                             SkSpan<skvm::Val> uniforms,
                             skvm::Coord deviceCoord,
                             SkSpan<skvm::Val> params,
                             const char* function)
        : INHERITED(program, errors, nullptr)
        , fContext(*context)
        , fBuilder(builder)
        , fDeviceCoord(deviceCoord)
        , fFunction(nullptr) {
    (void)fContext;
    fMask     = fBuilder->splat(0xffff'ffff);
    fReturned = fBuilder->splat(0);

    // First, copy the uniform IDs to our list of (all) variable IDs
    fVariables.insert(fVariables.end(), uniforms.begin(), uniforms.end());

    // Now, add all globals (including uniforms) to our variable list
    size_t uniformIdx = 0;
    for (const ProgramElement* e : fProgram.elements()) {
        if (e->is<GlobalVarDeclaration>()) {
            const GlobalVarDeclaration& decl = e->as<GlobalVarDeclaration>();
            const Variable& var = decl.declaration()->as<VarDeclaration>().var();
            SkASSERT(fVariableMap.find(&var) == fVariableMap.end());
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
        } else if (!fFunction && e->is<FunctionDefinition>()) {
            if (e->as<FunctionDefinition>().declaration().name() == function) {
                fFunction = static_cast<const FunctionDefinition*>(e);
            }
        }
    }
    SkASSERT(uniformIdx == uniforms.size());

    if (fFunction) {
        const FunctionDeclaration& decl = fFunction->declaration();

        // Reserve space for our return value
        fReturnSlot = fVariables.size();
        fVariables.resize(fReturnSlot + SlotCount(decl.returnType()), fBuilder->splat(0.0f).id);

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
            fErrors.error(-1, "Expected " + to_string(paramSlot - paramBase) +
                              " parameters, but received " + to_string(params.size()));
            fFunction = nullptr;
        }
    } else {
        fErrors.error(-1, "Function '" + String(function) + "' not found");
    }
}

bool SkVMGenerator::generateCode() {
    if (!fFunction) {
        return false;
    }
    this->writeFunction(*fFunction);
    return 0 == fErrors.errorCount();
}

void SkVMGenerator::writeFunction(const FunctionDefinition& f) {
    this->writeStatement(*f.body());
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

SkVMGenerator::Val SkVMGenerator::writeBinaryExpression(const BinaryExpression& b) {
    const Expression& left = *b.left();
    const Expression& right = *b.right();
    Token::Kind op = b.getOperator();
    if (op == Token::Kind::TK_EQ) {
        Val val = this->writeExpression(right);
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
    SkDEBUGCODE(Type::NumberKind nk = BaseNumberKind(lType));

    // A few ops require special treatment:
    switch (op) {
        case Token::Kind::TK_LOGICALAND: {
            SkASSERT(nk == Type::NumberKind::kBoolean);
            Val lVal = this->writeExpression(left);

            AutoMask shortCircuit(this, i32(lVal));
            Val rVal = this->writeExpression(right);
            return i32(lVal) & i32(rVal);
        }
        case Token::Kind::TK_LOGICALOR: {
            SkASSERT(nk == Type::NumberKind::kBoolean);
            Val lVal = this->writeExpression(left);

            AutoMask shortCircuit(this, ~i32(lVal));
            Val rVal = this->writeExpression(right);
            return i32(lVal) | i32(rVal);
        }
        case Token::Kind::TK_SHL:
        case Token::Kind::TK_SHR: {
            SkASSERT(nk == Type::NumberKind::kSigned || nk == Type::NumberKind::kUnsigned);
            if (!right.isCompileTimeConstant()) {
                fErrors.error(right.fOffset, "Shift amounts must be constant");
                return false;
            }
            int64_t shift = right.getConstantInt();
            if (shift < 0 || shift > 31) {
                fErrors.error(right.fOffset, "Shift amount out of range");
                return false;
            }

            Val val = this->writeExpression(left);

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
    Val lVal = this->writeExpression(left),
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
        Val result(lRows * rCols);
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
    Val result(count);

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
            binary([](skvm::F32 x, skvm::F32 y) { return x / y; },
                   unsupported_i);
            break;
        case Token::Kind::TK_PERCENT:
            binary(unsupported_f,
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

SkVMGenerator::Val SkVMGenerator::writeConstructor(const Constructor& c) {
    if (c.arguments().size() == 1) {
        // One-argument constructors can do type conversion:
        Val src = this->writeExpression(*c.arguments()[0]);
        Type::NumberKind srcKind = BaseNumberKind(c.arguments()[0]->type()),
                         dstKind = BaseNumberKind(c.type());
        // TODO: Handle signed vs. unsigned? Assuming unsigned never appears here.
        if (srcKind != dstKind) {
            Val dst(src.size());
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
                SkDEBUGFAIL("Unsupported type conversion");
            }
            return dst;
        }

        // TODO: Matrix re-shaping, scalar -> vector
    } else {
        // Multi-argument constructors just aggregate their arguments, with no conversion:
        Val result;
        for (const auto &arg : c.arguments()) {
            Val tmp = this->writeExpression(*arg);
            result.fVals.push_back_n(tmp.size(), tmp.fVals.begin());
        }
        return result;
    }
    return {};
    /*
    for (const auto& arg : c.arguments()) {
        this->writeExpression(*arg);
    }
    if (c.arguments().size() == 1) {
        const Type& inType = c.arguments()[0]->type();
        const Type& outType = c.type();
        TypeCategory inCategory = type_category(inType);
        TypeCategory outCategory = type_category(outType);
        int inCount = SlotCount(inType);
        int outCount = SlotCount(outType);
        if (inCategory != outCategory) {
            SkASSERT(inCount == outCount);
            if (inCategory == TypeCategory::kFloat) {
                SkASSERT(outCategory == TypeCategory::kSigned ||
                         outCategory == TypeCategory::kUnsigned);
                this->write(ByteCodeInstruction::kConvertFtoI, outCount);
            } else if (outCategory == TypeCategory::kFloat) {
                if (inCategory == TypeCategory::kSigned) {
                    this->write(ByteCodeInstruction::kConvertStoF, outCount);
                } else {
                    SkASSERT(inCategory == TypeCategory::kUnsigned);
                    this->write(ByteCodeInstruction::kConvertUtoF, outCount);
                }
            } else {
                SkASSERT(false);
            }
        }
        if (inType.isMatrix() && outType.isMatrix()) {
            this->write(ByteCodeInstruction::kMatrixToMatrix,
                        SlotCount(outType) - SlotCount(inType));
            this->write8(inType.columns());
            this->write8(inType.rows());
            this->write8(outType.columns());
            this->write8(outType.rows());
        } else if (inCount != outCount) {
            SkASSERT(inCount == 1);
            if (outType.isMatrix()) {
                this->write(ByteCodeInstruction::kScalarToMatrix, SlotCount(outType) - 1);
                this->write8(outType.columns());
                this->write8(outType.rows());
            } else {
                SkASSERT(outType.isVector());
                for (; inCount != outCount; ++inCount) {
                    this->write(ByteCodeInstruction::kDup, 1);
                }
            }
        }
    }
    */
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

SkVMGenerator::Val SkVMGenerator::writeVariableExpression(const Expression& e) {
    if (int builtin = expression_as_builtin(e); builtin >= 0) {
        switch (builtin) {
            case SK_FRAGCOORD_BUILTIN: {
                Val result;
                result.push_back(fDeviceCoord.x);
                result.push_back(fDeviceCoord.y);
                result.push_back(fBuilder->splat(0.0f));
                result.push_back(fBuilder->splat(1.0f));
                return result;
            }
            default:
                fErrors.error(e.fOffset, "Unsupported builtin");
                return {};
        }
    }

    Location location = this->getLocation(e);
    size_t count = SlotCount(e.type());
    if (location == kInvalidLocation || count == 0) {
        return {};
    }
    Val val(count);
    for (size_t i = 0; i < count; ++i) {
        val.push_back(fVariables[location + i]);
    }
    return val;
}

SkVMGenerator::Val SkVMGenerator::writeIntrinsicCall(const FunctionCall& c) {
    return {};
    /*
    auto found = fIntrinsics.find(c.function().name());
    if (found == fIntrinsics.end()) {
        fErrors.error(c.fOffset, String::printf("Unsupported intrinsic: '%s'",
                                                String(c.function().name()).c_str()));
        return;
    }
    Intrinsic intrin = found->second;

    const auto& args = c.arguments();
    const size_t nargs = args.size();
    SkASSERT(nargs >= 1);

    int count = SlotCount(args[0]->type());

    // Several intrinsics have variants where one argument is either scalar, or the same size as
    // the first argument. Call dupSmallerType(SlotCount(argType)) to ensure equal component count.
    auto dupSmallerType = [count, this](int smallCount) {
        SkASSERT(smallCount == 1 || smallCount == count);
        for (int i = smallCount; i < count; ++i) {
            this->write(ByteCodeInstruction::kDup, 1);
        }
    };

    if (intrin.is_special && intrin.special == SpecialIntrinsic::kSample) {
        // Sample is very special, the first argument is an FP, which can't be pushed to the stack.
        if (nargs > 2 || args[0]->type() != *fContext.fFragmentProcessor_Type ||
            (nargs == 2 && (args[1]->type() != *fContext.fFloat2_Type &&
                            args[1]->type() != *fContext.fFloat3x3_Type))) {
            fErrors.error(c.fOffset, "Unsupported form of sample");
            return;
        }

        if (nargs == 2) {
            // Write our coords or matrix
            this->writeExpression(*args[1]);
            this->write(args[1]->type() == *fContext.fFloat3x3_Type
                                ? ByteCodeInstruction::kSampleMatrix
                                : ByteCodeInstruction::kSampleExplicit);
        } else {
            this->write(ByteCodeInstruction::kSample);
        }

        Location childLoc = this->getLocation(*args[0]);
        SkASSERT(childLoc.fStorage == Storage::kChildFP);
        this->write8(childLoc.fSlot);
        return;
    }

    if (intrin.is_special && intrin.special == SpecialIntrinsic::kSmoothstep) {
        this->writeSmoothstep(args);
        return;
    }

    if (intrin.is_special && intrin.special == SpecialIntrinsic::kStep) {
        // There are variants where the *first* argument is scalar
        SkASSERT(nargs == 2);
        int xCount = SlotCount(args[1]->type());
        SkASSERT(count == 1 || count == xCount);

        this->writeExpression(*args[0]);  // 'edge'

        // Not 'dupSmallerType', because we're duping the first to match the second
        for (int i = count; i < xCount; ++i) {
            this->write(ByteCodeInstruction::kDup, 1);
        }

        this->writeExpression(*args[1]);  // 'x'
        this->write(ByteCodeInstruction::kStep, xCount);
        return;
    }

    if (intrin.is_special && (intrin.special == SpecialIntrinsic::kClamp ||
                              intrin.special == SpecialIntrinsic::kSaturate)) {
        // These intrinsics are extra-special, we need instructions interleaved with arguments
        bool saturate = (intrin.special == SpecialIntrinsic::kSaturate);
        SkASSERT(nargs == (saturate ? 1 : 3));
        int limitCount = saturate ? 1 : SlotCount(args[1]->type());

        // 'x'
        this->writeExpression(*args[0]);

        // 'minVal'
        if (saturate) {
            this->write(ByteCodeInstruction::kPushImmediate);
            this->write32(float_to_bits(0.0f));
        } else {
            this->writeExpression(*args[1]);
        }
        dupSmallerType(limitCount);
        this->writeTypedInstruction(args[0]->type(),
                                    ByteCodeInstruction::kMaxS,
                                    ByteCodeInstruction::kMaxS,
                                    ByteCodeInstruction::kMaxF,
                                    count);

        // 'maxVal'
        if (saturate) {
            this->write(ByteCodeInstruction::kPushImmediate);
            this->write32(float_to_bits(1.0f));
        } else {
            SkASSERT(limitCount == SlotCount(args[2]->type()));
            this->writeExpression(*args[2]);
        }
        dupSmallerType(limitCount);
        this->writeTypedInstruction(args[0]->type(),
                                    ByteCodeInstruction::kMinS,
                                    ByteCodeInstruction::kMinS,
                                    ByteCodeInstruction::kMinF,
                                    count);
        return;
    }

    // All other intrinsics can handle their arguments being on the stack in order
    for (const auto& arg : args) {
        this->writeExpression(*arg);
    }

    if (intrin.is_special) {
        auto doDotProduct = [count, this] {
            this->write(ByteCodeInstruction::kMultiplyF, count);
            for (int i = count - 1; i-- > 0;) {
                this->write(ByteCodeInstruction::kAddF, 1);
            }
        };

        auto doLength = [count, this, &doDotProduct] {
            this->write(ByteCodeInstruction::kDup, count);
            doDotProduct();
            this->write(ByteCodeInstruction::kSqrt, 1);
        };

        switch (intrin.special) {
            case SpecialIntrinsic::kAll: {
                for (int i = count-1; i --> 0;) {
                    this->write(ByteCodeInstruction::kAndB, 1);
                }
            } break;

            case SpecialIntrinsic::kAny: {
                for (int i = count-1; i --> 0;) {
                    this->write(ByteCodeInstruction::kOrB, 1);
                }
            } break;

            case SpecialIntrinsic::kATan: {
                // GLSL uses "atan" for both 'atan' and 'atan2'
                SkASSERT(nargs == 1 || (nargs == 2 && count == SlotCount(args[1]->type())));
                this->write(nargs == 1 ? ByteCodeInstruction::kATan : ByteCodeInstruction::kATan2,
                            count);
            } break;

            case SpecialIntrinsic::kDistance: {
                SkASSERT(nargs == 2 && count == SlotCount(args[1]->type()));
                this->write(ByteCodeInstruction::kSubtractF, count);
                doLength();
            } break;

            case SpecialIntrinsic::kDot: {
                SkASSERT(nargs == 2 && count == SlotCount(args[1]->type()));
                doDotProduct();
            } break;

            case SpecialIntrinsic::kLength: {
                SkASSERT(nargs == 1);
                doLength();
            } break;

            case SpecialIntrinsic::kMax:
            case SpecialIntrinsic::kMin: {
                SkASSERT(nargs == 2);
                // There are variants where the second argument is scalar
                dupSmallerType(SlotCount(args[1]->type()));
                if (intrin.special == SpecialIntrinsic::kMax) {
                    this->writeTypedInstruction(args[0]->type(),
                                                ByteCodeInstruction::kMaxS,
                                                ByteCodeInstruction::kMaxS,
                                                ByteCodeInstruction::kMaxF,
                                                count);
                } else {
                    this->writeTypedInstruction(args[0]->type(),
                                                ByteCodeInstruction::kMinS,
                                                ByteCodeInstruction::kMinS,
                                                ByteCodeInstruction::kMinF,
                                                count);
                }
            } break;

            case SpecialIntrinsic::kMix: {
                // Two main variants of mix to handle
                SkASSERT(nargs == 3);
                SkASSERT(count == SlotCount(args[1]->type()));
                int selectorCount = SlotCount(args[2]->type());

                if (is_generic_type(&args[2]->type(), fContext.fGenBType_Type.get())) {
                    // mix(genType, genType, genBoolType)
                    SkASSERT(selectorCount == count);
                    this->write(ByteCodeInstruction::kMix, count);
                } else {
                    // mix(genType, genType, genType) or mix(genType, genType, float)
                    dupSmallerType(selectorCount);
                    this->write(ByteCodeInstruction::kLerp, count);
                }
            } break;

            case SpecialIntrinsic::kMod: {
                SkASSERT(nargs == 2);
                // There are variants where the second argument is scalar
                dupSmallerType(SlotCount(args[1]->type()));
                this->write(ByteCodeInstruction::kMod, count);
            } break;

            case SpecialIntrinsic::kNormalize: {
                SkASSERT(nargs == 1);
                this->write(ByteCodeInstruction::kDup, count);
                doLength();
                dupSmallerType(1);
                this->write(ByteCodeInstruction::kDivideF, count);
            } break;

            default:
                SkASSERT(false);
        }
    } else {
        switch (intrin.inst_f) {
            case ByteCodeInstruction::kInverse2x2: {
                auto op = ByteCodeInstruction::kInverse2x2;
                switch (count) {
                    case 4: break;  // float2x2
                    case 9:  op = ByteCodeInstruction::kInverse3x3; break;
                    case 16: op = ByteCodeInstruction::kInverse4x4; break;
                    default: SkASSERT(false);
                }
                this->write(op);
                break;
            }

            default:
                this->writeTypedInstruction(args[0]->type(),
                                            intrin.inst_s,
                                            intrin.inst_u,
                                            intrin.inst_f,
                                            count);
                break;
        }
    }
    */
}

SkVMGenerator::Val SkVMGenerator::writeFunctionCall(const FunctionCall& f) {
    SkASSERT(false);
    return {};
    /*
    // Find the index of the function we're calling. We explicitly do not allow calls to functions
    // before they're defined. This is an easy-to-understand rule that prevents recursion.
    int idx = -1;
    for (size_t i = 0; i < fFunctions.size(); ++i) {
        if (f.function().matches(fFunctions[i]->declaration())) {
            idx = i;
            break;
        }
    }
    if (idx == -1) {
        this->writeIntrinsicCall(f);
        return;
    }


    if (idx > 255) {
        fErrors.error(f.fOffset, "Function count limit exceeded");
        return;
    } else if (idx >= (int) fFunctions.size()) {
        fErrors.error(f.fOffset, "Call to undefined function");
        return;
    }

    // We may need to deal with out parameters, so the sequence is tricky
    if (int returnCount = SlotCount(f.type())) {
        this->write(ByteCodeInstruction::kReserve, returnCount);
    }

    int argCount = f.arguments().size();
    std::vector<std::unique_ptr<LValue>> lvalues;
    for (int i = 0; i < argCount; ++i) {
        const auto& param = f.function().parameters()[i];
        const auto& arg = f.arguments()[i];
        if (param->modifiers().fFlags & Modifiers::kOut_Flag) {
            lvalues.emplace_back(this->getLValue(*arg));
            lvalues.back()->load();
        } else {
            this->writeExpression(*arg);
        }
    }

    // The space used by the call is based on the callee, but it also unwinds all of that before
    // we continue execution. We adjust our max stack depths below.
    this->write(ByteCodeInstruction::kCall);
    this->write8(idx);

    const ByteCodeFunction* callee = fOutput->fFunctions[idx].get();
    fMaxLoopCount      = std::max(fMaxLoopCount,      fLoopCount      + callee->fLoopCount);
    fMaxConditionCount = std::max(fMaxConditionCount, fConditionCount + callee->fConditionCount);
    fMaxStackCount     = std::max(fMaxStackCount,     fStackCount     + callee->fLocalCount
                                                                      + callee->fStackCount);

    // After the called function returns, the stack will still contain our arguments. We have to
    // pop them (storing any out parameters back to their lvalues as we go). We glob together slot
    // counts for all parameters that aren't out-params, so we can pop them in one big chunk.
    int popCount = 0;
    auto pop = [&]() {
        if (popCount > 0) {
            this->write(ByteCodeInstruction::kPop, popCount);
        }
        popCount = 0;
    };

    for (int i = argCount - 1; i >= 0; --i) {
        const auto& param = f.function().parameters()[i];
        const auto& arg = f.arguments()[i];
        if (param->modifiers().fFlags & Modifiers::kOut_Flag) {
            pop();
            lvalues.back()->store(true);
            lvalues.pop_back();
        } else {
            popCount += SlotCount(arg->type());
        }
    }
    pop();
    */
}

SkVMGenerator::Val SkVMGenerator::writePrefixExpression(const PrefixExpression& p) {
    switch (p.getOperator()) {
        case Token::Kind::TK_PLUSPLUS:
        case Token::Kind::TK_MINUSMINUS: {
            SkASSERT(SlotCount(p.operand()->type()) == 1);
            Val val = this->writeExpression(*p.operand());
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
            Val val = this->writeExpression(*p.operand());
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

SkVMGenerator::Val SkVMGenerator::writePostfixExpression(const PostfixExpression& p) {
    switch (p.getOperator()) {
        case Token::Kind::TK_PLUSPLUS:
        case Token::Kind::TK_MINUSMINUS: {
            SkASSERT(SlotCount(p.operand()->type()) == 1);
            Val old = this->writeExpression(*p.operand()),
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

SkVMGenerator::Val SkVMGenerator::writeSwizzle(const Swizzle& s) {
    Val base = this->writeExpression(*s.base());
    if (!base) {
        return {};
    }

    Val swizzled(s.components().size());
    for (int c : s.components()) {
        swizzled.push_back(base[c]);
    }
    return swizzled;
}

SkVMGenerator::Val SkVMGenerator::writeTernaryExpression(const TernaryExpression& t) {
    Val test = this->writeExpression(*t.test());
    Val ifTrue = this->writeExpression(*t.ifTrue());
    Val ifFalse = this->writeExpression(*t.ifFalse());
    if (!test || !ifTrue || !ifFalse) {
        return {};
    }

    size_t count = ifTrue.size();
    SkASSERT(count == ifFalse.size());

    Val result(count);
    for (size_t i = 0; i < count; ++i) {
        result.push_back(skvm::select(i32(test), i32(ifTrue[i]), i32(ifFalse[i])));
    }
    return result;
}

SkVMGenerator::Val SkVMGenerator::writeExpression(const Expression& e) {
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

void SkVMGenerator::writeStore(const Expression& lhs, const Val& rhs) {
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
        skvm::F32 curr = f32(fVariables[base + i]),
                  next = f32(rhs[i]);
        fVariables[base + i] = select(mask, next, curr).id;
    }
}

void SkVMGenerator::writeBlock(const Block& b) {
    for (const std::unique_ptr<Statement>& stmt : b.children()) {
        this->writeStatement(*stmt);
    }
}

void SkVMGenerator::writeIfStatement(const IfStatement& i) {
    Val test = this->writeExpression(*i.test());
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
    Val val = this->writeExpression(*r.expression());

    for (size_t i = 0; i < val.size(); ++i) {
        fVariables[fReturnSlot + i] =
                select(returnsHere, f32(val[i]), f32(fVariables[fReturnSlot + i])).id;
    }

    fReturned |= returnsHere;
}

void SkVMGenerator::writeVarDeclaration(const VarDeclaration& decl) {
    // Always grab the location, to ensure space is allocated in fVariables
    Location base = this->getLocation(decl.var());
    if (decl.value()) {
        Val val = this->writeExpression(*decl.value());
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

}  // namespace SkSL

#endif
