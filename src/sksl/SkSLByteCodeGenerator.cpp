/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLByteCodeGenerator.h"

#include <algorithm>

namespace SkSL {

ByteCodeGenerator::ByteCodeGenerator(const Context* context, const Program* program, ErrorReporter* errors,
                  ByteCode* output)
    : INHERITED(program, errors, nullptr)
    , fContext(*context)
    , fOutput(output)
    , fIntrinsics {
        { "cos",     ByteCodeInstruction::kCos },
        { "dot",     SpecialIntrinsic::kDot },
        { "inverse", ByteCodeInstruction::kInverse2x2 },
        { "sin",     ByteCodeInstruction::kSin },
        { "sqrt",    ByteCodeInstruction::kSqrt },
        { "tan",     ByteCodeInstruction::kTan },
      } {}


int ByteCodeGenerator::SlotCount(const Type& type) {
    if (type.kind() == Type::kOther_Kind) {
        return 0;
    } else if (type.kind() == Type::kStruct_Kind) {
        int slots = 0;
        for (const auto& f : type.fields()) {
            slots += SlotCount(*f.fType);
        }
        SkASSERT(slots <= 255);
        return slots;
    } else if (type.kind() == Type::kArray_Kind) {
        int columns = type.columns();
        SkASSERT(columns >= 0);
        int slots = columns * SlotCount(type.componentType());
        SkASSERT(slots <= 255);
        return slots;
    } else {
        return type.columns() * type.rows();
    }
}

static inline bool is_uniform(const SkSL::Variable& var) {
    return var.fModifiers.fFlags & Modifiers::kUniform_Flag;
}

bool ByteCodeGenerator::generateCode() {
    for (const auto& e : fProgram) {
        switch (e.fKind) {
            case ProgramElement::kFunction_Kind: {
                std::unique_ptr<ByteCodeFunction> f = this->writeFunction((FunctionDefinition&) e);
                if (!f) {
                    return false;
                }
                fOutput->fFunctions.push_back(std::move(f));
                fFunctions.push_back(&(FunctionDefinition&)e);
                break;
            }
            case ProgramElement::kVar_Kind: {
                VarDeclarations& decl = (VarDeclarations&) e;
                for (const auto& v : decl.fVars) {
                    const Variable* declVar = ((VarDeclaration&) *v).fVar;
                    if (declVar->fModifiers.fLayout.fBuiltin >= 0) {
                        continue;
                    }
                    // if you trip this assert, it means the program has raw 'in' variables. You
                    // should either specialize the program (Compiler::specialize) to bake in the
                    // final values of the 'in' variables, or not use 'in' variables (maybe you
                    // meant to use 'uniform' instead?).
//                    SkASSERT(!(declVar->fModifiers.fFlags & Modifiers::kIn_Flag));
                    if (is_uniform(*declVar)) {
                        fOutput->fUniformSlotCount += SlotCount(declVar->fType);
                    } else {
                        fOutput->fGlobalSlotCount += SlotCount(declVar->fType);
                    }
                }
                break;
            }
            default:
                ; // ignore
        }
    }
    return 0 == fErrors.errorCount();
}

std::unique_ptr<ByteCodeFunction> ByteCodeGenerator::writeFunction(const FunctionDefinition& f) {
    fFunction = &f;
    std::unique_ptr<ByteCodeFunction> result(new ByteCodeFunction(&f.fDeclaration));
    fParameterCount = result->fParameterCount;
    fLoopCount = fMaxLoopCount = 0;
    fConditionCount = fMaxConditionCount = 0;
    fStackCount = fMaxStackCount = 0;
    fCode = &result->fCode;

    this->writeStatement(*f.fBody);
    if (0 == fErrors.errorCount()) {
        SkASSERT(fLoopCount == 0);
        SkASSERT(fConditionCount == 0);
        SkASSERT(fStackCount == 0);
    }
    this->write(ByteCodeInstruction::kReturn, 0);
    this->write8(0);

    result->fLocalCount     = fLocals.size();
    result->fConditionCount = fMaxConditionCount;
    result->fLoopCount      = fMaxLoopCount;
    result->fStackCount     = fMaxStackCount;

    const Type& returnType = f.fDeclaration.fReturnType;
    if (returnType != *fContext.fVoid_Type) {
        result->fReturnCount = SlotCount(returnType);
    }
    fLocals.clear();
    fFunction = nullptr;
    return result;
}

enum class TypeCategory {
    kBool,
    kSigned,
    kUnsigned,
    kFloat,
};

static TypeCategory type_category(const Type& type) {
    switch (type.kind()) {
        case Type::Kind::kVector_Kind:
        case Type::Kind::kMatrix_Kind:
            return type_category(type.componentType());
        default:
            if (type.fName == "bool") {
                return TypeCategory::kBool;
            } else if (type.fName == "int" || type.fName == "short") {
                return TypeCategory::kSigned;
            } else if (type.fName == "uint" || type.fName == "ushort") {
                return TypeCategory::kUnsigned;
            } else {
                SkASSERT(type.fName == "float" || type.fName == "half");
                return TypeCategory::kFloat;
            }
            ABORT("unsupported type: %s\n", type.description().c_str());
    }
}

// A "simple" Swizzle is based on a variable (or a compound variable like a struct or array), and
// that references consecutive values, such that it can be implemented using normal load/store ops
// with an offset. Note that all single-component swizzles (of suitable base types) are simple.
static bool swizzle_is_simple(const Swizzle& s) {
    switch (s.fBase->fKind) {
        case Expression::kFieldAccess_Kind:
        case Expression::kIndex_Kind:
        case Expression::kVariableReference_Kind:
            break;
        default:
            return false;
    }

    for (size_t i = 1; i < s.fComponents.size(); ++i) {
        if (s.fComponents[i] != s.fComponents[i - 1] + 1) {
            return false;
        }
    }
    return true;
}

int ByteCodeGenerator::StackUsage(ByteCodeInstruction inst, int count_) {
    // Ensures that we use count iff we're passed a non-default value. Most instructions have an
    // implicit count, so the caller shouldn't need to worry about it (or count makes no sense).
    // The asserts avoids callers thinking they're supplying useful information in that scenario,
    // or failing to supply necessary information for the ops that need a count.
    struct CountValue {
        operator int() {
            SkASSERT(val != ByteCodeGenerator::kUnusedStackCount);
            SkDEBUGCODE(used = true);
            return val;
        }
        ~CountValue() {
            SkASSERT(used || val == ByteCodeGenerator::kUnusedStackCount);
        }
        int val;
        SkDEBUGCODE(bool used = false;)
    } count = { count_ };

    switch (inst) {
        // Unary functions/operators that don't change stack depth at all:
#define VECTOR_UNARY_OP(base)                \
        case ByteCodeInstruction::base:      \
        case ByteCodeInstruction::base ## 2: \
        case ByteCodeInstruction::base ## 3: \
        case ByteCodeInstruction::base ## 4: \
            return 0;

        VECTOR_UNARY_OP(kConvertFtoI)
        VECTOR_UNARY_OP(kConvertStoF)
        VECTOR_UNARY_OP(kConvertUtoF)

        VECTOR_UNARY_OP(kCos)
        VECTOR_UNARY_OP(kSin)
        VECTOR_UNARY_OP(kSqrt)
        VECTOR_UNARY_OP(kTan)

        VECTOR_UNARY_OP(kNegateF)
        VECTOR_UNARY_OP(kNegateI)

        case ByteCodeInstruction::kInverse2x2:
        case ByteCodeInstruction::kInverse3x3:
        case ByteCodeInstruction::kInverse4x4: return 0;

        case ByteCodeInstruction::kClampIndex: return 0;
        case ByteCodeInstruction::kNotB: return 0;
        case ByteCodeInstruction::kNegateFN: return 0;
        case ByteCodeInstruction::kShiftLeft: return 0;
        case ByteCodeInstruction::kShiftRightS: return 0;
        case ByteCodeInstruction::kShiftRightU: return 0;

#undef VECTOR_UNARY_OP

        // Binary functions/operators that do a 2 -> 1 reduction (possibly N times)
#define VECTOR_BINARY_OP(base)                          \
        case ByteCodeInstruction::base:      return -1; \
        case ByteCodeInstruction::base ## 2: return -2; \
        case ByteCodeInstruction::base ## 3: return -3; \
        case ByteCodeInstruction::base ## 4: return -4;

#define VECTOR_MATRIX_BINARY_OP(base)                   \
        VECTOR_BINARY_OP(base)                          \
        case ByteCodeInstruction::base ## N: return -count;

        case ByteCodeInstruction::kAndB: return -1;
        case ByteCodeInstruction::kOrB:  return -1;
        case ByteCodeInstruction::kXorB: return -1;

        VECTOR_BINARY_OP(kAddI)
        VECTOR_MATRIX_BINARY_OP(kAddF)

        VECTOR_BINARY_OP(kCompareIEQ)
        VECTOR_MATRIX_BINARY_OP(kCompareFEQ)
        VECTOR_BINARY_OP(kCompareINEQ)
        VECTOR_MATRIX_BINARY_OP(kCompareFNEQ)
        VECTOR_BINARY_OP(kCompareSGT)
        VECTOR_BINARY_OP(kCompareUGT)
        VECTOR_BINARY_OP(kCompareFGT)
        VECTOR_BINARY_OP(kCompareSGTEQ)
        VECTOR_BINARY_OP(kCompareUGTEQ)
        VECTOR_BINARY_OP(kCompareFGTEQ)
        VECTOR_BINARY_OP(kCompareSLT)
        VECTOR_BINARY_OP(kCompareULT)
        VECTOR_BINARY_OP(kCompareFLT)
        VECTOR_BINARY_OP(kCompareSLTEQ)
        VECTOR_BINARY_OP(kCompareULTEQ)
        VECTOR_BINARY_OP(kCompareFLTEQ)

        VECTOR_BINARY_OP(kDivideS)
        VECTOR_BINARY_OP(kDivideU)
        VECTOR_MATRIX_BINARY_OP(kDivideF)
        VECTOR_BINARY_OP(kMultiplyI)
        VECTOR_MATRIX_BINARY_OP(kMultiplyF)
        VECTOR_BINARY_OP(kRemainderF)
        VECTOR_BINARY_OP(kRemainderS)
        VECTOR_BINARY_OP(kRemainderU)
        VECTOR_BINARY_OP(kSubtractI)
        VECTOR_MATRIX_BINARY_OP(kSubtractF)

#undef VECTOR_BINARY_OP
#undef VECTOR_MATRIX_BINARY_OP

        // Ops that push or load data to grow the stack:
        case ByteCodeInstruction::kDup:
        case ByteCodeInstruction::kLoad:
        case ByteCodeInstruction::kLoadGlobal:
        case ByteCodeInstruction::kLoadUniform:
        case ByteCodeInstruction::kReadExternal:
        case ByteCodeInstruction::kPushImmediate:
            return 1;

        case ByteCodeInstruction::kDup2:
        case ByteCodeInstruction::kLoad2:
        case ByteCodeInstruction::kLoadGlobal2:
        case ByteCodeInstruction::kLoadUniform2:
        case ByteCodeInstruction::kReadExternal2:
            return 2;

        case ByteCodeInstruction::kDup3:
        case ByteCodeInstruction::kLoad3:
        case ByteCodeInstruction::kLoadGlobal3:
        case ByteCodeInstruction::kLoadUniform3:
        case ByteCodeInstruction::kReadExternal3:
            return 3;

        case ByteCodeInstruction::kDup4:
        case ByteCodeInstruction::kLoad4:
        case ByteCodeInstruction::kLoadGlobal4:
        case ByteCodeInstruction::kLoadUniform4:
        case ByteCodeInstruction::kReadExternal4:
            return 4;

        case ByteCodeInstruction::kDupN:
        case ByteCodeInstruction::kLoadSwizzle:
        case ByteCodeInstruction::kLoadSwizzleGlobal:
        case ByteCodeInstruction::kLoadSwizzleUniform:
            return count;

        // Pushes 'count' values, minus one for the 'address' that's consumed first
        case ByteCodeInstruction::kLoadExtended:
        case ByteCodeInstruction::kLoadExtendedGlobal:
        case ByteCodeInstruction::kLoadExtendedUniform:
            return count - 1;

        // Ops that pop or store data to shrink the stack:
        case ByteCodeInstruction::kPop:
        case ByteCodeInstruction::kStore:
        case ByteCodeInstruction::kStoreGlobal:
        case ByteCodeInstruction::kWriteExternal:
            return -1;

        case ByteCodeInstruction::kPop2:
        case ByteCodeInstruction::kStore2:
        case ByteCodeInstruction::kStoreGlobal2:
        case ByteCodeInstruction::kWriteExternal2:
            return -2;

        case ByteCodeInstruction::kPop3:
        case ByteCodeInstruction::kStore3:
        case ByteCodeInstruction::kStoreGlobal3:
        case ByteCodeInstruction::kWriteExternal3:
            return -3;

        case ByteCodeInstruction::kPop4:
        case ByteCodeInstruction::kStore4:
        case ByteCodeInstruction::kStoreGlobal4:
        case ByteCodeInstruction::kWriteExternal4:
            return -4;

        case ByteCodeInstruction::kPopN:
        case ByteCodeInstruction::kStoreSwizzle:
        case ByteCodeInstruction::kStoreSwizzleGlobal:
            return -count;

        // Consumes 'count' values, plus one for the 'address'
        case ByteCodeInstruction::kStoreExtended:
        case ByteCodeInstruction::kStoreExtendedGlobal:
        case ByteCodeInstruction::kStoreSwizzleIndirect:
        case ByteCodeInstruction::kStoreSwizzleIndirectGlobal:
            return -count - 1;

        // Strange ops where the caller computes the delta for us:
        case ByteCodeInstruction::kCallExternal:
        case ByteCodeInstruction::kMatrixToMatrix:
        case ByteCodeInstruction::kMatrixMultiply:
        case ByteCodeInstruction::kReserve:
        case ByteCodeInstruction::kReturn:
        case ByteCodeInstruction::kScalarToMatrix:
        case ByteCodeInstruction::kSwizzle:
            return count;

        // Miscellaneous

        // kCall is net-zero. Max stack depth is adjusted in writeFunctionCall.
        case ByteCodeInstruction::kCall:             return 0;
        case ByteCodeInstruction::kBranch:           return 0;
        case ByteCodeInstruction::kBranchIfAllFalse: return 0;

        case ByteCodeInstruction::kMaskPush:         return -1;
        case ByteCodeInstruction::kMaskPop:          return 0;
        case ByteCodeInstruction::kMaskNegate:       return 0;
        case ByteCodeInstruction::kMaskBlend:        return -count;

        case ByteCodeInstruction::kLoopBegin:        return 0;
        case ByteCodeInstruction::kLoopNext:         return 0;
        case ByteCodeInstruction::kLoopMask:         return -1;
        case ByteCodeInstruction::kLoopEnd:          return 0;
        case ByteCodeInstruction::kLoopBreak:        return 0;
        case ByteCodeInstruction::kLoopContinue:     return 0;

        default:
            ABORT("unsupported instruction %d\n", (int)inst);
            return 0;
    }
}

ByteCodeGenerator::Location ByteCodeGenerator::getLocation(const Variable& var) {
    // given that we seldom have more than a couple of variables, linear search is probably the most
    // efficient way to handle lookups
    switch (var.fStorage) {
        case Variable::kLocal_Storage: {
            for (int i = fLocals.size() - 1; i >= 0; --i) {
                if (fLocals[i] == &var) {
                    SkASSERT(fParameterCount + i <= 255);
                    return { fParameterCount + i, Storage::kLocal };
                }
            }
            int result = fParameterCount + fLocals.size();
            fLocals.push_back(&var);
            for (int i = 0; i < SlotCount(var.fType) - 1; ++i) {
                fLocals.push_back(nullptr);
            }
            SkASSERT(result <= 255);
            return { result, Storage::kLocal };
        }
        case Variable::kParameter_Storage: {
            int offset = 0;
            for (const auto& p : fFunction->fDeclaration.fParameters) {
                if (p == &var) {
                    SkASSERT(offset <= 255);
                    return { offset, Storage::kLocal };
                }
                offset += SlotCount(p->fType);
            }
            SkASSERT(false);
            return Location::MakeInvalid();
        }
        case Variable::kGlobal_Storage: {
            int offset = 0;
            bool isUniform = is_uniform(var);
            for (const auto& e : fProgram) {
                if (e.fKind == ProgramElement::kVar_Kind) {
                    VarDeclarations& decl = (VarDeclarations&) e;
                    for (const auto& v : decl.fVars) {
                        const Variable* declVar = ((VarDeclaration&) *v).fVar;
                        if (declVar->fModifiers.fLayout.fBuiltin >= 0) {
                            continue;
                        }
                        if (isUniform != is_uniform(*declVar)) {
                            continue;
                        }
                        if (declVar == &var) {
                            SkASSERT(offset <= 255);
                            return  { offset, isUniform ? Storage::kUniform : Storage::kGlobal };
                        }
                        offset += SlotCount(declVar->fType);
                    }
                }
            }
            SkASSERT(false);
            return Location::MakeInvalid();
        }
        default:
            SkASSERT(false);
            return Location::MakeInvalid();
    }
}

ByteCodeGenerator::Location ByteCodeGenerator::getLocation(const Expression& expr) {
    switch (expr.fKind) {
        case Expression::kFieldAccess_Kind: {
            const FieldAccess& f = (const FieldAccess&)expr;
            Location baseLoc = this->getLocation(*f.fBase);
            int offset = 0;
            for (int i = 0; i < f.fFieldIndex; ++i) {
                offset += SlotCount(*f.fBase->fType.fields()[i].fType);
            }
            if (baseLoc.isOnStack()) {
                if (offset != 0) {
                    this->write(ByteCodeInstruction::kPushImmediate);
                    this->write32(offset);
                    this->write(ByteCodeInstruction::kAddI);
                    this->write8(1);
                }
                return baseLoc;
            } else {
                return baseLoc + offset;
            }
        }
        case Expression::kIndex_Kind: {
            const IndexExpression& i = (const IndexExpression&)expr;
            int stride = SlotCount(i.fType);
            int length = i.fBase->fType.columns();
            SkASSERT(length <= 255);
            int offset = -1;
            if (i.fIndex->isConstant()) {
                int64_t index = i.fIndex->getConstantInt();
                if (index < 0 || index >= length) {
                    fErrors.error(i.fIndex->fOffset, "Array index out of bounds.");
                    return Location::MakeInvalid();
                }
                offset = index * stride;
            } else {
                if (i.fIndex->hasSideEffects()) {
                    // Having a side-effect in an indexer is technically safe for an rvalue,
                    // but with lvalues we have to evaluate the indexer twice, so make it an error.
                    fErrors.error(i.fIndex->fOffset,
                            "Index expressions with side-effects not supported in byte code.");
                    return Location::MakeInvalid();
                }
                this->writeExpression(*i.fIndex);
                this->write(ByteCodeInstruction::kClampIndex);
                this->write8(length);
                if (stride != 1) {
                    this->write(ByteCodeInstruction::kPushImmediate);
                    this->write32(stride);
                    this->write(ByteCodeInstruction::kMultiplyI);
                    this->write8(1);
                }
            }
            Location baseLoc = this->getLocation(*i.fBase);

            // Are both components known statically?
            if (!baseLoc.isOnStack() && offset >= 0) {
                return baseLoc + offset;
            }

            // At least one component is dynamic (and on the stack).

            // If the other component is zero, we're done
            if (baseLoc.fSlot == 0 || offset == 0) {
                return baseLoc.makeOnStack();
            }

            // Push the non-dynamic component (if any) to the stack, then add the two
            if (!baseLoc.isOnStack()) {
                this->write(ByteCodeInstruction::kPushImmediate);
                this->write32(baseLoc.fSlot);
            }
            if (offset >= 0) {
                this->write(ByteCodeInstruction::kPushImmediate);
                this->write32(offset);
            }
            this->write(ByteCodeInstruction::kAddI);
            this->write8(1);
            return baseLoc.makeOnStack();
        }
        case Expression::kSwizzle_Kind: {
            const Swizzle& s = (const Swizzle&)expr;
            SkASSERT(swizzle_is_simple(s));
            Location baseLoc = this->getLocation(*s.fBase);
            int offset = s.fComponents[0];
            if (baseLoc.isOnStack()) {
                if (offset != 0) {
                    this->write(ByteCodeInstruction::kPushImmediate);
                    this->write32(offset);
                    this->write(ByteCodeInstruction::kAddI);
                    this->write8(1);
                }
                return baseLoc;
            } else {
                return baseLoc + offset;
            }
        }
        case Expression::kVariableReference_Kind: {
            const Variable& var = ((const VariableReference&)expr).fVariable;
            return this->getLocation(var);
        }
        default:
            SkASSERT(false);
            return Location::MakeInvalid();
    }
}

void ByteCodeGenerator::write8(uint8_t b) {
    fCode->push_back(b);
}

void ByteCodeGenerator::write16(uint16_t i) {
    size_t n = fCode->size();
    fCode->resize(n+2);
    memcpy(fCode->data() + n, &i, 2);
}

void ByteCodeGenerator::write32(uint32_t i) {
    size_t n = fCode->size();
    fCode->resize(n+4);
    memcpy(fCode->data() + n, &i, 4);
}

void ByteCodeGenerator::write(ByteCodeInstruction i, int count) {
    switch (i) {
        case ByteCodeInstruction::kLoopBegin: this->enterLoop();      break;
        case ByteCodeInstruction::kLoopEnd:   this->exitLoop();       break;

        case ByteCodeInstruction::kMaskPush:  this->enterCondition(); break;
        case ByteCodeInstruction::kMaskPop:
        case ByteCodeInstruction::kMaskBlend: this->exitCondition();  break;
        default: /* Do nothing */ break;
    }
    instruction val = (instruction) i;
    size_t n = fCode->size();
    fCode->resize(n + sizeof(val));
    memcpy(fCode->data() + n, &val, sizeof(val));
    fStackCount += StackUsage(i, count);
    fMaxStackCount = std::max(fMaxStackCount, fStackCount);
}

static ByteCodeInstruction vector_instruction(ByteCodeInstruction base, int count) {
    SkASSERT(count >= 1 && count <= 4);
    return ((ByteCodeInstruction) ((int) base + 1 - count));
}

void ByteCodeGenerator::writeTypedInstruction(const Type& type, ByteCodeInstruction s,
                                              ByteCodeInstruction u, ByteCodeInstruction f,
                                              int count, bool writeCount) {
    switch (type_category(type)) {
        case TypeCategory::kSigned:
            this->write(vector_instruction(s, count));
            break;
        case TypeCategory::kUnsigned:
            this->write(vector_instruction(u, count));
            break;
        case TypeCategory::kFloat: {
            if (count > 4) {
                this->write((ByteCodeInstruction)((int)f + 1), count);
            } else {
                this->write(vector_instruction(f, count));
            }
            break;
        }
        default:
            SkASSERT(false);
    }
    if (writeCount) {
        this->write8(count);
    }
}

bool ByteCodeGenerator::writeBinaryExpression(const BinaryExpression& b, bool discard) {
    if (b.fOperator == Token::Kind::EQ) {
        std::unique_ptr<LValue> lvalue = this->getLValue(*b.fLeft);
        this->writeExpression(*b.fRight);
        lvalue->store(discard);
        discard = false;
        return discard;
    }
    const Type& lType = b.fLeft->fType;
    const Type& rType = b.fRight->fType;
    bool lVecOrMtx = (lType.kind() == Type::kVector_Kind || lType.kind() == Type::kMatrix_Kind);
    bool rVecOrMtx = (rType.kind() == Type::kVector_Kind || rType.kind() == Type::kMatrix_Kind);
    Token::Kind op;
    std::unique_ptr<LValue> lvalue;
    if (is_assignment(b.fOperator)) {
        lvalue = this->getLValue(*b.fLeft);
        lvalue->load();
        op = remove_assignment(b.fOperator);
    } else {
        this->writeExpression(*b.fLeft);
        op = b.fOperator;
        if (!lVecOrMtx && rVecOrMtx) {
            for (int i = SlotCount(rType); i > 1; --i) {
                this->write(ByteCodeInstruction::kDup);
                this->write8(1);
            }
        }
    }
    int count = std::max(SlotCount(lType), SlotCount(rType));
    SkDEBUGCODE(TypeCategory tc = type_category(lType));
    switch (op) {
        case Token::Kind::LOGICALAND: {
            SkASSERT(tc == SkSL::TypeCategory::kBool && count == 1);
            this->write(ByteCodeInstruction::kDup);
            this->write8(1);
            this->write(ByteCodeInstruction::kMaskPush);
            this->write(ByteCodeInstruction::kBranchIfAllFalse);
            DeferredLocation falseLocation(this);
            this->writeExpression(*b.fRight);
            this->write(ByteCodeInstruction::kAndB);
            falseLocation.set();
            this->write(ByteCodeInstruction::kMaskPop);
            return false;
        }
        case Token::Kind::LOGICALOR: {
            SkASSERT(tc == SkSL::TypeCategory::kBool && count == 1);
            this->write(ByteCodeInstruction::kDup);
            this->write8(1);
            this->write(ByteCodeInstruction::kNotB);
            this->write(ByteCodeInstruction::kMaskPush);
            this->write(ByteCodeInstruction::kBranchIfAllFalse);
            DeferredLocation falseLocation(this);
            this->writeExpression(*b.fRight);
            this->write(ByteCodeInstruction::kOrB);
            falseLocation.set();
            this->write(ByteCodeInstruction::kMaskPop);
            return false;
        }
        case Token::Kind::SHL:
        case Token::Kind::SHR: {
            SkASSERT(count == 1 && (tc == SkSL::TypeCategory::kSigned ||
                                    tc == SkSL::TypeCategory::kUnsigned));
            if (!b.fRight->isConstant()) {
                fErrors.error(b.fRight->fOffset, "Shift amounts must be constant");
                return false;
            }
            int64_t shift = b.fRight->getConstantInt();
            if (shift < 0 || shift > 31) {
                fErrors.error(b.fRight->fOffset, "Shift amount out of range");
                return false;
            }

            if (op == Token::Kind::SHL) {
                this->write(ByteCodeInstruction::kShiftLeft);
            } else {
                this->write(type_category(lType) == TypeCategory::kSigned
                                ? ByteCodeInstruction::kShiftRightS
                                : ByteCodeInstruction::kShiftRightU);
            }
            this->write8(shift);
            return false;
        }

        default:
            break;
    }
    this->writeExpression(*b.fRight);
    if (lVecOrMtx && !rVecOrMtx) {
        for (int i = SlotCount(lType); i > 1; --i) {
            this->write(ByteCodeInstruction::kDup);
            this->write8(1);
        }
    }
    // Special case for M*V, V*M, M*M (but not V*V!)
    if (op == Token::Kind::STAR && lVecOrMtx && rVecOrMtx &&
        !(lType.kind() == Type::kVector_Kind && rType.kind() == Type::kVector_Kind)) {
        this->write(ByteCodeInstruction::kMatrixMultiply,
                    SlotCount(b.fType) - (SlotCount(lType) + SlotCount(rType)));
        int rCols = rType.columns(),
            rRows = rType.rows(),
            lCols = lType.columns(),
            lRows = lType.rows();
        // M*V treats the vector as a column
        if (rType.kind() == Type::kVector_Kind) {
            std::swap(rCols, rRows);
        }
        SkASSERT(lCols == rRows);
        SkASSERT(SlotCount(b.fType) == lRows * rCols);
        this->write8(lCols);
        this->write8(lRows);
        this->write8(rCols);
    } else {
        switch (op) {
            case Token::Kind::EQEQ:
                this->writeTypedInstruction(lType, ByteCodeInstruction::kCompareIEQ,
                                            ByteCodeInstruction::kCompareIEQ,
                                            ByteCodeInstruction::kCompareFEQ,
                                            count);
                // Collapse to a single bool
                for (int i = count; i > 1; --i) {
                    this->write(ByteCodeInstruction::kAndB);
                }
                break;
            case Token::Kind::GT:
                this->writeTypedInstruction(lType, ByteCodeInstruction::kCompareSGT,
                                            ByteCodeInstruction::kCompareUGT,
                                            ByteCodeInstruction::kCompareFGT,
                                            count);
                break;
            case Token::Kind::GTEQ:
                this->writeTypedInstruction(lType, ByteCodeInstruction::kCompareSGTEQ,
                                            ByteCodeInstruction::kCompareUGTEQ,
                                            ByteCodeInstruction::kCompareFGTEQ,
                                            count);
                break;
            case Token::Kind::LT:
                this->writeTypedInstruction(lType, ByteCodeInstruction::kCompareSLT,
                                            ByteCodeInstruction::kCompareULT,
                                            ByteCodeInstruction::kCompareFLT,
                                            count);
                break;
            case Token::Kind::LTEQ:
                this->writeTypedInstruction(lType, ByteCodeInstruction::kCompareSLTEQ,
                                            ByteCodeInstruction::kCompareULTEQ,
                                            ByteCodeInstruction::kCompareFLTEQ,
                                            count);
                break;
            case Token::Kind::MINUS:
                this->writeTypedInstruction(lType, ByteCodeInstruction::kSubtractI,
                                            ByteCodeInstruction::kSubtractI,
                                            ByteCodeInstruction::kSubtractF,
                                            count);
                break;
            case Token::Kind::NEQ:
                this->writeTypedInstruction(lType, ByteCodeInstruction::kCompareINEQ,
                                            ByteCodeInstruction::kCompareINEQ,
                                            ByteCodeInstruction::kCompareFNEQ,
                                            count);
                // Collapse to a single bool
                for (int i = count; i > 1; --i) {
                    this->write(ByteCodeInstruction::kOrB);
                }
                break;
            case Token::Kind::PERCENT:
                this->writeTypedInstruction(lType, ByteCodeInstruction::kRemainderS,
                                            ByteCodeInstruction::kRemainderU,
                                            ByteCodeInstruction::kRemainderF,
                                            count);
                break;
            case Token::Kind::PLUS:
                this->writeTypedInstruction(lType, ByteCodeInstruction::kAddI,
                                            ByteCodeInstruction::kAddI,
                                            ByteCodeInstruction::kAddF,
                                            count);
                break;
            case Token::Kind::SLASH:
                this->writeTypedInstruction(lType, ByteCodeInstruction::kDivideS,
                                            ByteCodeInstruction::kDivideU,
                                            ByteCodeInstruction::kDivideF,
                                            count);
                break;
            case Token::Kind::STAR:
                this->writeTypedInstruction(lType, ByteCodeInstruction::kMultiplyI,
                                            ByteCodeInstruction::kMultiplyI,
                                            ByteCodeInstruction::kMultiplyF,
                                            count);
                break;

            case Token::Kind::LOGICALXOR:
                SkASSERT(tc == SkSL::TypeCategory::kBool && count == 1);
                this->write(ByteCodeInstruction::kXorB);
                break;

            case Token::Kind::BITWISEAND:
                SkASSERT(count == 1 && (tc == SkSL::TypeCategory::kSigned ||
                                        tc == SkSL::TypeCategory::kUnsigned));
                this->write(ByteCodeInstruction::kAndB);
                break;
            case Token::Kind::BITWISEOR:
                SkASSERT(count == 1 && (tc == SkSL::TypeCategory::kSigned ||
                                        tc == SkSL::TypeCategory::kUnsigned));
                this->write(ByteCodeInstruction::kOrB);
                break;
            case Token::Kind::BITWISEXOR:
                SkASSERT(count == 1 && (tc == SkSL::TypeCategory::kSigned ||
                                        tc == SkSL::TypeCategory::kUnsigned));
                this->write(ByteCodeInstruction::kXorB);
                break;

            default:
                fErrors.error(b.fOffset, SkSL::String::printf("Unsupported binary operator '%s'",
                                                              Compiler::OperatorName(op)));
                break;
        }
    }
    if (lvalue) {
        lvalue->store(discard);
        discard = false;
    }
    return discard;
}

void ByteCodeGenerator::writeBoolLiteral(const BoolLiteral& b) {
    this->write(ByteCodeInstruction::kPushImmediate);
    this->write32(b.fValue ? ~0 : 0);
}

void ByteCodeGenerator::writeConstructor(const Constructor& c) {
    for (const auto& arg : c.fArguments) {
        this->writeExpression(*arg);
    }
    if (c.fArguments.size() == 1) {
        const Type& inType = c.fArguments[0]->fType;
        const Type& outType = c.fType;
        TypeCategory inCategory = type_category(inType);
        TypeCategory outCategory = type_category(outType);
        int inCount = SlotCount(inType);
        int outCount = SlotCount(outType);
        if (inCategory != outCategory) {
            SkASSERT(inCount == outCount);
            if (inCategory == TypeCategory::kFloat) {
                SkASSERT(outCategory == TypeCategory::kSigned ||
                         outCategory == TypeCategory::kUnsigned);
                this->write(vector_instruction(ByteCodeInstruction::kConvertFtoI, outCount));
            } else if (outCategory == TypeCategory::kFloat) {
                if (inCategory == TypeCategory::kSigned) {
                    this->write(vector_instruction(ByteCodeInstruction::kConvertStoF, outCount));
                } else {
                    SkASSERT(inCategory == TypeCategory::kUnsigned);
                    this->write(vector_instruction(ByteCodeInstruction::kConvertUtoF, outCount));
                }
            } else {
                SkASSERT(false);
            }
        }
        if (inType.kind() == Type::kMatrix_Kind && outType.kind() == Type::kMatrix_Kind) {
            this->write(ByteCodeInstruction::kMatrixToMatrix,
                        SlotCount(outType) - SlotCount(inType));
            this->write8(inType.columns());
            this->write8(inType.rows());
            this->write8(outType.columns());
            this->write8(outType.rows());
        } else if (inCount != outCount) {
            SkASSERT(inCount == 1);
            if (outType.kind() == Type::kMatrix_Kind) {
                this->write(ByteCodeInstruction::kScalarToMatrix, SlotCount(outType) - 1);
                this->write8(outType.columns());
                this->write8(outType.rows());
            } else {
                SkASSERT(outType.kind() == Type::kVector_Kind);
                for (; inCount != outCount; ++inCount) {
                    this->write(ByteCodeInstruction::kDup);
                    this->write8(1);
                }
            }
        }
    }
}

void ByteCodeGenerator::writeExternalFunctionCall(const ExternalFunctionCall& f) {
    int argumentCount = 0;
    for (const auto& arg : f.fArguments) {
        this->writeExpression(*arg);
        argumentCount += SlotCount(arg->fType);
    }
    this->write(ByteCodeInstruction::kCallExternal, SlotCount(f.fType) - argumentCount);
    SkASSERT(argumentCount <= 255);
    this->write8(argumentCount);
    this->write8(SlotCount(f.fType));
    int index = fOutput->fExternalValues.size();
    fOutput->fExternalValues.push_back(f.fFunction);
    SkASSERT(index <= 255);
    this->write8(index);
}

void ByteCodeGenerator::writeExternalValue(const ExternalValueReference& e) {
    int count = SlotCount(e.fValue->type());
    this->write(vector_instruction(ByteCodeInstruction::kReadExternal, count));
    this->write8(count);
    int index = fOutput->fExternalValues.size();
    fOutput->fExternalValues.push_back(e.fValue);
    SkASSERT(index <= 255);
    this->write8(index);
}

void ByteCodeGenerator::writeVariableExpression(const Expression& expr) {
    Location location = this->getLocation(expr);
    int count = SlotCount(expr.fType);
    if (location.isOnStack() || count > 4) {
        if (!location.isOnStack()) {
            this->write(ByteCodeInstruction::kPushImmediate);
            this->write32(location.fSlot);
        }
        this->write(location.selectLoad(ByteCodeInstruction::kLoadExtended,
                                        ByteCodeInstruction::kLoadExtendedGlobal,
                                        ByteCodeInstruction::kLoadExtendedUniform),
                    count);
        this->write8(count);
    } else {
        this->write(vector_instruction(location.selectLoad(ByteCodeInstruction::kLoad,
                                                           ByteCodeInstruction::kLoadGlobal,
                                                           ByteCodeInstruction::kLoadUniform),
                                       count));
        this->write8(count);
        this->write8(location.fSlot);
    }
}

static inline uint32_t float_to_bits(float x) {
    uint32_t u;
    memcpy(&u, &x, sizeof(uint32_t));
    return u;
}

void ByteCodeGenerator::writeFloatLiteral(const FloatLiteral& f) {
    this->write(ByteCodeInstruction::kPushImmediate);
    this->write32(float_to_bits(f.fValue));
}

void ByteCodeGenerator::writeIntrinsicCall(const FunctionCall& c) {
    auto found = fIntrinsics.find(c.fFunction.fName);
    if (found == fIntrinsics.end()) {
        fErrors.error(c.fOffset, "unsupported intrinsic function");
        return;
    }
    int count = SlotCount(c.fArguments[0]->fType);
    if (found->second.fIsSpecial) {
        SpecialIntrinsic special = found->second.fValue.fSpecial;
        switch (special) {
            case SpecialIntrinsic::kDot: {
                SkASSERT(c.fArguments.size() == 2);
                SkASSERT(count == SlotCount(c.fArguments[1]->fType));
                this->write(vector_instruction(ByteCodeInstruction::kMultiplyF, count));
                this->write8(count);
                for (int i = count; i > 1; --i) {
                    this->write(ByteCodeInstruction::kAddF);
                    this->write8(1);
                }
                break;
            }
            default:
                SkASSERT(false);
        }
    } else {
        switch (found->second.fValue.fInstruction) {
            case ByteCodeInstruction::kCos:
            case ByteCodeInstruction::kSin:
            case ByteCodeInstruction::kTan:
                SkASSERT(c.fArguments.size() > 0);
                this->write(vector_instruction(found->second.fValue.fInstruction, count));
                this->write8(count);
                break;
            case ByteCodeInstruction::kSqrt:
                SkASSERT(c.fArguments.size() > 0);
                this->write(vector_instruction(found->second.fValue.fInstruction, count));
                break;
            case ByteCodeInstruction::kInverse2x2: {
                SkASSERT(c.fArguments.size() > 0);
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
                SkASSERT(false);
        }
    }
}

void ByteCodeGenerator::writeFunctionCall(const FunctionCall& f) {
    // Builtins have simple signatures...
    if (f.fFunction.fBuiltin) {
        for (const auto& arg : f.fArguments) {
            this->writeExpression(*arg);
        }
        this->writeIntrinsicCall(f);
        return;
    }

    // Find the index of the function we're calling. We explicitly do not allow calls to functions
    // before they're defined. This is an easy-to-understand rule that prevents recursion.
    size_t idx;
    for (idx = 0; idx < fFunctions.size(); ++idx) {
        if (f.fFunction.matches(fFunctions[idx]->fDeclaration)) {
            break;
        }
    }
    if (idx > 255) {
        fErrors.error(f.fOffset, "Function count limit exceeded");
        return;
    } else if (idx >= fFunctions.size()) {
        fErrors.error(f.fOffset, "Call to undefined function");
        return;
    }

    // We may need to deal with out parameters, so the sequence is tricky
    if (int returnCount = SlotCount(f.fType)) {
        this->write(ByteCodeInstruction::kReserve, returnCount);
        this->write8(returnCount);
    }

    int argCount = f.fArguments.size();
    std::vector<std::unique_ptr<LValue>> lvalues;
    for (int i = 0; i < argCount; ++i) {
        const auto& param = f.fFunction.fParameters[i];
        const auto& arg = f.fArguments[i];
        if (param->fModifiers.fFlags & Modifiers::kOut_Flag) {
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
        if (popCount > 4) {
            this->write(ByteCodeInstruction::kPopN, popCount);
            this->write8(popCount);
        } else if (popCount > 0) {
            this->write(vector_instruction(ByteCodeInstruction::kPop, popCount));
        }
        popCount = 0;
    };

    for (int i = argCount - 1; i >= 0; --i) {
        const auto& param = f.fFunction.fParameters[i];
        const auto& arg = f.fArguments[i];
        if (param->fModifiers.fFlags & Modifiers::kOut_Flag) {
            pop();
            lvalues.back()->store(true);
            lvalues.pop_back();
        } else {
            popCount += SlotCount(arg->fType);
        }
    }
    pop();
}

void ByteCodeGenerator::writeIntLiteral(const IntLiteral& i) {
    this->write(ByteCodeInstruction::kPushImmediate);
    this->write32(i.fValue);
}

void ByteCodeGenerator::writeNullLiteral(const NullLiteral& n) {
    // not yet implemented
    abort();
}

bool ByteCodeGenerator::writePrefixExpression(const PrefixExpression& p, bool discard) {
    switch (p.fOperator) {
        case Token::Kind::PLUSPLUS: // fall through
        case Token::Kind::MINUSMINUS: {
            SkASSERT(SlotCount(p.fOperand->fType) == 1);
            std::unique_ptr<LValue> lvalue = this->getLValue(*p.fOperand);
            lvalue->load();
            this->write(ByteCodeInstruction::kPushImmediate);
            this->write32(type_category(p.fType) == TypeCategory::kFloat ? float_to_bits(1.0f) : 1);
            if (p.fOperator == Token::Kind::PLUSPLUS) {
                this->writeTypedInstruction(p.fType,
                                            ByteCodeInstruction::kAddI,
                                            ByteCodeInstruction::kAddI,
                                            ByteCodeInstruction::kAddF,
                                            1);
            } else {
                this->writeTypedInstruction(p.fType,
                                            ByteCodeInstruction::kSubtractI,
                                            ByteCodeInstruction::kSubtractI,
                                            ByteCodeInstruction::kSubtractF,
                                            1);
            }
            lvalue->store(discard);
            discard = false;
            break;
        }
        case Token::Kind::MINUS: {
            this->writeExpression(*p.fOperand);
            this->writeTypedInstruction(p.fType,
                                        ByteCodeInstruction::kNegateI,
                                        ByteCodeInstruction::kNegateI,
                                        ByteCodeInstruction::kNegateF,
                                        SlotCount(p.fOperand->fType),
                                        false);
            break;
        }
        case Token::Kind::LOGICALNOT:
        case Token::Kind::BITWISENOT: {
            SkASSERT(SlotCount(p.fOperand->fType) == 1);
            SkDEBUGCODE(TypeCategory tc = type_category(p.fOperand->fType));
            SkASSERT((p.fOperator == Token::Kind::LOGICALNOT && tc == TypeCategory::kBool) ||
                     (p.fOperator == Token::Kind::BITWISENOT && (tc == TypeCategory::kSigned ||
                                                                 tc == TypeCategory::kUnsigned)));
            this->writeExpression(*p.fOperand);
            this->write(ByteCodeInstruction::kNotB);
            break;
        }
        default:
            SkASSERT(false);
    }
    return discard;
}

bool ByteCodeGenerator::writePostfixExpression(const PostfixExpression& p, bool discard) {
    switch (p.fOperator) {
        case Token::Kind::PLUSPLUS: // fall through
        case Token::Kind::MINUSMINUS: {
            SkASSERT(SlotCount(p.fOperand->fType) == 1);
            std::unique_ptr<LValue> lvalue = this->getLValue(*p.fOperand);
            lvalue->load();
            // If we're not supposed to discard the result, then make a copy *before* the +/-
            if (!discard) {
                this->write(ByteCodeInstruction::kDup);
                this->write8(1);
            }
            this->write(ByteCodeInstruction::kPushImmediate);
            this->write32(type_category(p.fType) == TypeCategory::kFloat ? float_to_bits(1.0f) : 1);
            if (p.fOperator == Token::Kind::PLUSPLUS) {
                this->writeTypedInstruction(p.fType,
                                            ByteCodeInstruction::kAddI,
                                            ByteCodeInstruction::kAddI,
                                            ByteCodeInstruction::kAddF,
                                            1);
            } else {
                this->writeTypedInstruction(p.fType,
                                            ByteCodeInstruction::kSubtractI,
                                            ByteCodeInstruction::kSubtractI,
                                            ByteCodeInstruction::kSubtractF,
                                            1);
            }
            // Always consume the result as part of the store
            lvalue->store(true);
            discard = false;
            break;
        }
        default:
            SkASSERT(false);
    }
    return discard;
}

void ByteCodeGenerator::writeSwizzle(const Swizzle& s) {
    if (swizzle_is_simple(s)) {
        this->writeVariableExpression(s);
        return;
    }

    switch (s.fBase->fKind) {
        case Expression::kVariableReference_Kind: {
            Location location = this->getLocation(*s.fBase);
            this->write(location.selectLoad(ByteCodeInstruction::kLoadSwizzle,
                                            ByteCodeInstruction::kLoadSwizzleGlobal,
                                            ByteCodeInstruction::kLoadSwizzleUniform),
                        s.fComponents.size());
            this->write8(location.fSlot);
            this->write8(s.fComponents.size());
            for (int c : s.fComponents) {
                this->write8(c);
            }
            break;
        }
        default:
            this->writeExpression(*s.fBase);
            this->write(ByteCodeInstruction::kSwizzle,
                        s.fComponents.size() - s.fBase->fType.columns());
            this->write8(s.fBase->fType.columns());
            this->write8(s.fComponents.size());
            for (int c : s.fComponents) {
                this->write8(c);
            }
    }
}

void ByteCodeGenerator::writeTernaryExpression(const TernaryExpression& t) {
    int count = SlotCount(t.fType);
    SkASSERT(count == SlotCount(t.fIfTrue->fType));
    SkASSERT(count == SlotCount(t.fIfFalse->fType));

    this->writeExpression(*t.fTest);
    this->write(ByteCodeInstruction::kMaskPush);
    this->writeExpression(*t.fIfTrue);
    this->write(ByteCodeInstruction::kMaskNegate);
    this->writeExpression(*t.fIfFalse);
    this->write(ByteCodeInstruction::kMaskBlend, count);
    this->write8(count);
}

void ByteCodeGenerator::writeExpression(const Expression& e, bool discard) {
    switch (e.fKind) {
        case Expression::kBinary_Kind:
            discard = this->writeBinaryExpression((BinaryExpression&) e, discard);
            break;
        case Expression::kBoolLiteral_Kind:
            this->writeBoolLiteral((BoolLiteral&) e);
            break;
        case Expression::kConstructor_Kind:
            this->writeConstructor((Constructor&) e);
            break;
        case Expression::kExternalFunctionCall_Kind:
            this->writeExternalFunctionCall((ExternalFunctionCall&) e);
            break;
        case Expression::kExternalValue_Kind:
            this->writeExternalValue((ExternalValueReference&) e);
            break;
        case Expression::kFieldAccess_Kind:
        case Expression::kIndex_Kind:
        case Expression::kVariableReference_Kind:
            this->writeVariableExpression(e);
            break;
        case Expression::kFloatLiteral_Kind:
            this->writeFloatLiteral((FloatLiteral&) e);
            break;
        case Expression::kFunctionCall_Kind:
            this->writeFunctionCall((FunctionCall&) e);
            break;
        case Expression::kIntLiteral_Kind:
            this->writeIntLiteral((IntLiteral&) e);
            break;
        case Expression::kNullLiteral_Kind:
            this->writeNullLiteral((NullLiteral&) e);
            break;
        case Expression::kPrefix_Kind:
            discard = this->writePrefixExpression((PrefixExpression&) e, discard);
            break;
        case Expression::kPostfix_Kind:
            discard = this->writePostfixExpression((PostfixExpression&) e, discard);
            break;
        case Expression::kSwizzle_Kind:
            this->writeSwizzle((Swizzle&) e);
            break;
        case Expression::kTernary_Kind:
            this->writeTernaryExpression((TernaryExpression&) e);
            break;
        default:
            printf("unsupported expression %s\n", e.description().c_str());
            SkASSERT(false);
    }
    if (discard) {
        int count = SlotCount(e.fType);
        if (count > 4) {
            this->write(ByteCodeInstruction::kPopN, count);
            this->write8(count);
        } else if (count != 0) {
            this->write(vector_instruction(ByteCodeInstruction::kPop, count));
        }
        discard = false;
    }
}

class ByteCodeExternalValueLValue : public ByteCodeGenerator::LValue {
public:
    ByteCodeExternalValueLValue(ByteCodeGenerator* generator, ExternalValue& value, int index)
        : INHERITED(*generator)
        , fCount(ByteCodeGenerator::SlotCount(value.type()))
        , fIndex(index) {}

    void load() override {
        fGenerator.write(vector_instruction(ByteCodeInstruction::kReadExternal, fCount));
        fGenerator.write8(fCount);
        fGenerator.write8(fIndex);
    }

    void store(bool discard) override {
        if (!discard) {
            fGenerator.write(vector_instruction(ByteCodeInstruction::kDup, fCount));
            fGenerator.write8(fCount);
        }
        fGenerator.write(vector_instruction(ByteCodeInstruction::kWriteExternal, fCount));
        fGenerator.write8(fCount);
        fGenerator.write8(fIndex);
    }

private:
    typedef LValue INHERITED;

    int fCount;

    int fIndex;
};

class ByteCodeSwizzleLValue : public ByteCodeGenerator::LValue {
public:
    ByteCodeSwizzleLValue(ByteCodeGenerator* generator, const Swizzle& swizzle)
        : INHERITED(*generator)
        , fSwizzle(swizzle) {}

    void load() override {
        fGenerator.writeSwizzle(fSwizzle);
    }

    void store(bool discard) override {
        int count = fSwizzle.fComponents.size();
        if (!discard) {
            fGenerator.write(vector_instruction(ByteCodeInstruction::kDup, count));
            fGenerator.write8(count);
        }
        ByteCodeGenerator::Location location = fGenerator.getLocation(*fSwizzle.fBase);
        if (location.isOnStack()) {
            fGenerator.write(location.selectStore(ByteCodeInstruction::kStoreSwizzleIndirect,
                                                  ByteCodeInstruction::kStoreSwizzleIndirectGlobal),
                             count);
        } else {
            fGenerator.write(location.selectStore(ByteCodeInstruction::kStoreSwizzle,
                                                  ByteCodeInstruction::kStoreSwizzleGlobal),
                             count);
            fGenerator.write8(location.fSlot);
        }
        fGenerator.write8(count);
        for (int c : fSwizzle.fComponents) {
            fGenerator.write8(c);
        }
    }

private:
    const Swizzle& fSwizzle;

    typedef LValue INHERITED;
};

class ByteCodeExpressionLValue : public ByteCodeGenerator::LValue {
public:
    ByteCodeExpressionLValue(ByteCodeGenerator* generator, const Expression& expr)
        : INHERITED(*generator)
        , fExpression(expr) {}

    void load() override {
        fGenerator.writeVariableExpression(fExpression);
    }

    void store(bool discard) override {
        int count = ByteCodeGenerator::SlotCount(fExpression.fType);
        if (!discard) {
            if (count > 4) {
                fGenerator.write(ByteCodeInstruction::kDupN, count);
                fGenerator.write8(count);
            } else {
                fGenerator.write(vector_instruction(ByteCodeInstruction::kDup, count));
                fGenerator.write8(count);
            }
        }
        ByteCodeGenerator::Location location = fGenerator.getLocation(fExpression);
        if (location.isOnStack() || count > 4) {
            if (!location.isOnStack()) {
                fGenerator.write(ByteCodeInstruction::kPushImmediate);
                fGenerator.write32(location.fSlot);
            }
            fGenerator.write(location.selectStore(ByteCodeInstruction::kStoreExtended,
                                                  ByteCodeInstruction::kStoreExtendedGlobal),
                             count);
            fGenerator.write8(count);
        } else {
            fGenerator.write(
                    vector_instruction(location.selectStore(ByteCodeInstruction::kStore,
                                                            ByteCodeInstruction::kStoreGlobal),
                                       count));
            fGenerator.write8(location.fSlot);
        }
    }

private:
    typedef LValue INHERITED;

    const Expression& fExpression;
};

std::unique_ptr<ByteCodeGenerator::LValue> ByteCodeGenerator::getLValue(const Expression& e) {
    switch (e.fKind) {
        case Expression::kExternalValue_Kind: {
            ExternalValue* value = ((ExternalValueReference&) e).fValue;
            int index = fOutput->fExternalValues.size();
            fOutput->fExternalValues.push_back(value);
            SkASSERT(index <= 255);
            return std::unique_ptr<LValue>(new ByteCodeExternalValueLValue(this, *value, index));
        }
        case Expression::kFieldAccess_Kind:
        case Expression::kIndex_Kind:
        case Expression::kVariableReference_Kind:
            return std::unique_ptr<LValue>(new ByteCodeExpressionLValue(this, e));
        case Expression::kSwizzle_Kind: {
            const Swizzle& s = (const Swizzle&) e;
            return swizzle_is_simple(s)
                    ? std::unique_ptr<LValue>(new ByteCodeExpressionLValue(this, e))
                    : std::unique_ptr<LValue>(new ByteCodeSwizzleLValue(this, s));
        }
        case Expression::kTernary_Kind:
        default:
            printf("unsupported lvalue %s\n", e.description().c_str());
            return nullptr;
    }
}

void ByteCodeGenerator::writeBlock(const Block& b) {
    for (const auto& s : b.fStatements) {
        this->writeStatement(*s);
    }
}

void ByteCodeGenerator::setBreakTargets() {
    std::vector<DeferredLocation>& breaks = fBreakTargets.top();
    for (DeferredLocation& b : breaks) {
        b.set();
    }
    fBreakTargets.pop();
}

void ByteCodeGenerator::setContinueTargets() {
    std::vector<DeferredLocation>& continues = fContinueTargets.top();
    for (DeferredLocation& c : continues) {
        c.set();
    }
    fContinueTargets.pop();
}

void ByteCodeGenerator::writeBreakStatement(const BreakStatement& b) {
    // TODO: Include BranchIfAllFalse to top-most LoopNext
    this->write(ByteCodeInstruction::kLoopBreak);
}

void ByteCodeGenerator::writeContinueStatement(const ContinueStatement& c) {
    // TODO: Include BranchIfAllFalse to top-most LoopNext
    this->write(ByteCodeInstruction::kLoopContinue);
}

void ByteCodeGenerator::writeDoStatement(const DoStatement& d) {
    this->write(ByteCodeInstruction::kLoopBegin);
    size_t start = fCode->size();
    this->writeStatement(*d.fStatement);
    this->write(ByteCodeInstruction::kLoopNext);
    this->writeExpression(*d.fTest);
    this->write(ByteCodeInstruction::kLoopMask);
    // TODO: Could shorten this with kBranchIfAnyTrue
    this->write(ByteCodeInstruction::kBranchIfAllFalse);
    DeferredLocation endLocation(this);
    this->write(ByteCodeInstruction::kBranch);
    this->write16(start);
    endLocation.set();
    this->write(ByteCodeInstruction::kLoopEnd);
}

void ByteCodeGenerator::writeForStatement(const ForStatement& f) {
    fContinueTargets.emplace();
    fBreakTargets.emplace();
    if (f.fInitializer) {
        this->writeStatement(*f.fInitializer);
    }
    this->write(ByteCodeInstruction::kLoopBegin);
    size_t start = fCode->size();
    if (f.fTest) {
        this->writeExpression(*f.fTest);
        this->write(ByteCodeInstruction::kLoopMask);
    }
    this->write(ByteCodeInstruction::kBranchIfAllFalse);
    DeferredLocation endLocation(this);
    this->writeStatement(*f.fStatement);
    this->write(ByteCodeInstruction::kLoopNext);
    if (f.fNext) {
        this->writeExpression(*f.fNext, true);
    }
    this->write(ByteCodeInstruction::kBranch);
    this->write16(start);
    endLocation.set();
    this->write(ByteCodeInstruction::kLoopEnd);
}

void ByteCodeGenerator::writeIfStatement(const IfStatement& i) {
    this->writeExpression(*i.fTest);
    this->write(ByteCodeInstruction::kMaskPush);
    this->write(ByteCodeInstruction::kBranchIfAllFalse);
    DeferredLocation falseLocation(this);
    this->writeStatement(*i.fIfTrue);
    falseLocation.set();
    if (i.fIfFalse) {
        this->write(ByteCodeInstruction::kMaskNegate);
        this->write(ByteCodeInstruction::kBranchIfAllFalse);
        DeferredLocation endLocation(this);
        this->writeStatement(*i.fIfFalse);
        endLocation.set();
    }
    this->write(ByteCodeInstruction::kMaskPop);
}

void ByteCodeGenerator::writeReturnStatement(const ReturnStatement& r) {
    if (fLoopCount || fConditionCount) {
        fErrors.error(r.fOffset, "return not allowed inside conditional or loop");
        return;
    }
    int count = SlotCount(r.fExpression->fType);
    this->writeExpression(*r.fExpression);

    // Technically, the kReturn also pops fOutput->fLocalCount values from the stack, too, but we
    // haven't counted pushing those (they're outside the scope of our stack tracking). Instead,
    // we account for those in writeFunction().

    // This is all fine because we don't allow conditional returns, so we only return once anyway.
    this->write(ByteCodeInstruction::kReturn, -count);
    this->write8(count);
}

void ByteCodeGenerator::writeSwitchStatement(const SwitchStatement& r) {
    // not yet implemented
    abort();
}

void ByteCodeGenerator::writeVarDeclarations(const VarDeclarations& v) {
    for (const auto& declStatement : v.fVars) {
        const VarDeclaration& decl = (VarDeclaration&) *declStatement;
        // we need to grab the location even if we don't use it, to ensure it has been allocated
        Location location = this->getLocation(*decl.fVar);
        if (decl.fValue) {
            this->writeExpression(*decl.fValue);
            int count = SlotCount(decl.fValue->fType);
            if (count > 4) {
                this->write(ByteCodeInstruction::kPushImmediate);
                this->write32(location.fSlot);
                this->write(ByteCodeInstruction::kStoreExtended, count);
                this->write8(count);
            } else {
                this->write(vector_instruction(ByteCodeInstruction::kStore, count));
                this->write8(location.fSlot);
            }
        }
    }
}

void ByteCodeGenerator::writeWhileStatement(const WhileStatement& w) {
    this->write(ByteCodeInstruction::kLoopBegin);
    size_t cond = fCode->size();
    this->writeExpression(*w.fTest);
    this->write(ByteCodeInstruction::kLoopMask);
    this->write(ByteCodeInstruction::kBranchIfAllFalse);
    DeferredLocation endLocation(this);
    this->writeStatement(*w.fStatement);
    this->write(ByteCodeInstruction::kLoopNext);
    this->write(ByteCodeInstruction::kBranch);
    this->write16(cond);
    endLocation.set();
    this->write(ByteCodeInstruction::kLoopEnd);
}

void ByteCodeGenerator::writeStatement(const Statement& s) {
    switch (s.fKind) {
        case Statement::kBlock_Kind:
            this->writeBlock((Block&) s);
            break;
        case Statement::kBreak_Kind:
            this->writeBreakStatement((BreakStatement&) s);
            break;
        case Statement::kContinue_Kind:
            this->writeContinueStatement((ContinueStatement&) s);
            break;
        case Statement::kDiscard_Kind:
            // not yet implemented
            abort();
        case Statement::kDo_Kind:
            this->writeDoStatement((DoStatement&) s);
            break;
        case Statement::kExpression_Kind:
            this->writeExpression(*((ExpressionStatement&) s).fExpression, true);
            break;
        case Statement::kFor_Kind:
            this->writeForStatement((ForStatement&) s);
            break;
        case Statement::kIf_Kind:
            this->writeIfStatement((IfStatement&) s);
            break;
        case Statement::kNop_Kind:
            break;
        case Statement::kReturn_Kind:
            this->writeReturnStatement((ReturnStatement&) s);
            break;
        case Statement::kSwitch_Kind:
            this->writeSwitchStatement((SwitchStatement&) s);
            break;
        case Statement::kVarDeclarations_Kind:
            this->writeVarDeclarations(*((VarDeclarationsStatement&) s).fDeclaration);
            break;
        case Statement::kWhile_Kind:
            this->writeWhileStatement((WhileStatement&) s);
            break;
        default:
            SkASSERT(false);
    }
}

ByteCodeFunction::ByteCodeFunction(const FunctionDeclaration* declaration)
        : fName(declaration->fName) {
    fParameterCount = 0;
    for (const auto& p : declaration->fParameters) {
        int slots = ByteCodeGenerator::SlotCount(p->fType);
        fParameters.push_back({ slots, (bool)(p->fModifiers.fFlags & Modifiers::kOut_Flag) });
        fParameterCount += slots;
    }
}

}
