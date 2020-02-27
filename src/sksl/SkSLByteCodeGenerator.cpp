/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLByteCodeGenerator.h"

namespace SkSL {

ByteCodeGenerator::ByteCodeGenerator(const Program* program, ErrorReporter* errors,
                                     ByteCode* output)
    : INHERITED(program, errors, nullptr)
    , fOutput(output)
    , fIntrinsics {
        // "Normal" intrinsics are all $genType f($genType), mapped to a single instruction
        { "cos",     ByteCode::Instruction::kCos },
        { "sin",     ByteCode::Instruction::kSin },
        { "sqrt",    ByteCode::Instruction::kSqrt },
        { "tan",     ByteCode::Instruction::kTan },

        // Special intrinsics have other signatures, or non-standard code-gen
        { "dot",     SpecialIntrinsic::kDot },
        { "inverse", SpecialIntrinsic::kInverse },
        { "print",   SpecialIntrinsic::kPrint },
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

static inline bool is_in(const SkSL::Variable& var) {
    return var.fModifiers.fFlags & Modifiers::kIn_Flag;
}
ByteCodeGenerator::Location ByteCodeGenerator::getLocation(const Variable& var) {
    // given that we seldom have more than a couple of variables, linear search is probably the most
    // efficient way to handle lookups
    switch (var.fStorage) {
        case Variable::kLocal_Storage: {
            for (int i = fLocals.size() - 1; i >= 0; --i) {
                if (fLocals[i] == &var) {
                    return ByteCode::Pointer{(uint16_t) (i + fParameterCount)};
                }
            }
            int result = fLocals.size() + fParameterCount;
            fLocals.push_back(&var);
            for (int i = 0; i < SlotCount(var.fType) - 1; ++i) {
                fLocals.push_back(nullptr);
            }
            SkASSERT(result <= ByteCode::kPointerMax);
            return ByteCode::Pointer{(uint16_t) result};
        }
        case Variable::kParameter_Storage: {
            int offset = 0;
            for (const auto& p : fFunction->fDeclaration.fParameters) {
                if (p == &var) {
                    SkASSERT(offset <= ByteCode::kPointerMax);
                    return ByteCode::Pointer{(uint16_t) offset};
                }
                offset += SlotCount(p->fType);
            }
            SkASSERT(false);
            return ByteCode::Pointer{0};
        }
        case Variable::kGlobal_Storage: {
            if (is_in(var)) {
                // If you see this error, it means the program is using raw 'in' variables. You
                // should either specialize the program (Compiler::specialize) to bake in the final
                // values of the 'in' variables, or not use 'in' variables (maybe you meant to use
                // 'uniform' instead?).
                fErrors.error(var.fOffset,
                              "'in' variable is not specialized or has unsupported type");
                return ByteCode::Pointer{0};
            }
            bool isUniform = is_uniform(var);
            int offset = isUniform ? fOutput->getGlobalSlotCount() : 0;
            for (const auto& e : fProgram) {
                if (e.fKind == ProgramElement::kVar_Kind) {
                    VarDeclarations& decl = (VarDeclarations&) e;
                    for (const auto& v : decl.fVars) {
                        const Variable* declVar = ((VarDeclaration&) *v).fVar;
                        if (declVar->fModifiers.fLayout.fBuiltin >= 0 || is_in(*declVar)) {
                            continue;
                        }
                        if (isUniform != is_uniform(*declVar)) {
                            continue;
                        }
                        if (declVar == &var) {
                            SkASSERT(offset <= ByteCode::kPointerMax);
                            return ByteCode::Pointer{(uint16_t) offset};
                        }
                        offset += SlotCount(declVar->fType);
                    }
                }
            }
            SkASSERT(false);
            return ByteCode::Pointer{0};
        }
        default:
            SkASSERT(false);
            return ByteCode::Pointer{0};
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

ByteCodeGenerator::Location ByteCodeGenerator::getLocation(const Expression& expr) {
    switch (expr.fKind) {
        case Expression::kFieldAccess_Kind: {
            const FieldAccess& f = (const FieldAccess&) expr;
            Location result = this->getLocation(*f.fBase);
            int offset = 0;
            for (int i = 0; i < f.fFieldIndex; ++i) {
                offset += SlotCount(*f.fBase->fType.fields()[i].fType);
            }
            return result.offset(*this, offset);
        }
        case Expression::kIndex_Kind: {
            const IndexExpression& idx = (const IndexExpression&) expr;
            int stride = SlotCount(idx.fType);
            int length = idx.fBase->fType.columns();
            Location result = this->getLocation(*idx.fBase);
            if (idx.fIndex->isConstant()) {
                int64_t index = idx.fIndex->getConstantInt();
                if (index < 0 || index >= length) {
                    fErrors.error(idx.fIndex->fOffset, "Array index out of bounds");
                    return result;
                }
                return result.offset(*this, index * stride);
            } else {
                ByteCode::Register index = this->next(1);
                this->writeExpression(*idx.fIndex, index);
                this->write(ByteCode::Instruction::kBoundsCheck);
                this->write(index);
                this->write(length);
                ByteCode::Register imm = this->next(1);
                this->write(ByteCode::Instruction::kImmediate);
                this->write(imm);
                this->write(ByteCode::Immediate{stride});
                ByteCode::Register offset = this->next(1);
                this->write(ByteCode::Instruction::kMultiplyI);
                this->write(offset);
                this->write(index);
                this->write(imm);
                return result.offset(*this, offset);
            }
        }
        case Expression::kSwizzle_Kind: {
            const Swizzle& s = (const Swizzle&) expr;
            SkASSERT(swizzle_is_simple(s));
            return this->getLocation(*s.fBase).offset(*this, s.fComponents[0]);
        }
        case Expression::kVariableReference_Kind: {
            const Variable& var = ((const VariableReference&) expr).fVariable;
            return this->getLocation(var);
        }
        default:
            SkASSERT(false);
            return ByteCode::Pointer{0};
    }
}

Variable::Storage ByteCodeGenerator::getStorage(const Expression& expr) {
    switch (expr.fKind) {
        case Expression::kFieldAccess_Kind: {
            const FieldAccess& f = (const FieldAccess&) expr;
            return this->getStorage(*f.fBase);
        }
        case Expression::kIndex_Kind: {
            const IndexExpression& idx = (const IndexExpression&) expr;
            return this->getStorage(*idx.fBase);
        }
        case Expression::kSwizzle_Kind: {
            const Swizzle& s = (const Swizzle&) expr;
            return this->getStorage(*s.fBase);
        }
        case Expression::kVariableReference_Kind: {
            const Variable& var = ((const VariableReference&) expr).fVariable;
            return var.fStorage;
        }
        default:
            SkASSERT(false);
            return Variable::kLocal_Storage;
    }
}

ByteCode::Instruction ByteCodeGenerator::getLoadInstruction(ByteCodeGenerator::Location location,
                                                            Variable::Storage storage) {
    switch (storage) {
        case Variable::kGlobal_Storage:
            switch (location.fKind) {
                case Location::kPointer_Kind: return ByteCode::Instruction::kLoadDirect;
                case Location::kRegister_Kind: return ByteCode::Instruction::kLoad;
            }
        case Variable::kParameter_Storage:
            switch (location.fKind) {
                case Location::kPointer_Kind: return ByteCode::Instruction::kLoadParameterDirect;
                case Location::kRegister_Kind: return ByteCode::Instruction::kLoadParameter;
            }
        case Variable::kLocal_Storage:
            switch (location.fKind) {
                case Location::kPointer_Kind: return ByteCode::Instruction::kLoadStackDirect;
                case Location::kRegister_Kind: return ByteCode::Instruction::kLoadStack;
            }
        default:
            break;
    }
    SkASSERT(false);
    return ByteCode::Instruction::kNop;
}

ByteCode::Instruction ByteCodeGenerator::getStoreInstruction(ByteCodeGenerator::Location location,
                                                             Variable::Storage storage) {
    switch (storage) {
        case Variable::kGlobal_Storage:
            switch (location.fKind) {
                case Location::kPointer_Kind: return ByteCode::Instruction::kStoreDirect;
                case Location::kRegister_Kind: return ByteCode::Instruction::kStore;
            }
        case Variable::kParameter_Storage:
            switch (location.fKind) {
                case Location::kPointer_Kind: return ByteCode::Instruction::kStoreParameterDirect;
                case Location::kRegister_Kind: return ByteCode::Instruction::kStoreParameter;
            }
        case Variable::kLocal_Storage:
            switch (location.fKind) {
                case Location::kPointer_Kind: return ByteCode::Instruction::kStoreStackDirect;
                case Location::kRegister_Kind: return ByteCode::Instruction::kStoreStack;
            }
        default:
            break;
    }
    SkASSERT(false);
    return ByteCode::Instruction::kNop;
}

#define VEC(inst) ((ByteCode::Instruction) ((uint16_t) inst + 1))

class ByteCodeSimpleLValue : public ByteCodeGenerator::LValue {
public:
    ByteCodeSimpleLValue(ByteCodeGenerator* generator, ByteCodeGenerator::Location location,
                         int count, ByteCode::Instruction load, ByteCode::Instruction store)
        : INHERITED(*generator)
        , fLocation(location)
        , fCount((uint8_t) count)
        , fLoad(load)
        , fStore(store) {}

    void load(ByteCode::Register result) override {
        fGenerator.write(fLoad, fCount);
        fGenerator.write(result);
        fGenerator.write(fLocation);
    }

    void store(ByteCode::Register src) override {
        fGenerator.write(fStore, fCount);
        fGenerator.write(fLocation);
        fGenerator.write(src);
    }

private:
    ByteCodeGenerator::Location fLocation;

    uint8_t fCount;

    ByteCode::Instruction fLoad;

    ByteCode::Instruction fStore;

    typedef ByteCodeGenerator::LValue INHERITED;
};

class ByteCodeSwizzleLValue : public ByteCodeGenerator::LValue {
public:
    ByteCodeSwizzleLValue(ByteCodeGenerator* generator, const Swizzle* swizzle)
        : INHERITED(*generator)
        , fSwizzle(*swizzle) {}

    void load(ByteCode::Register result) override {
        fGenerator.writeSwizzle(fSwizzle, result);
    }

    void store(ByteCode::Register src) override {
        ByteCodeGenerator::Location target = fGenerator.getLocation(*fSwizzle.fBase);
        ByteCode::Instruction inst = fGenerator.getStoreInstruction(
                                                            target,
                                                            fGenerator.getStorage(*fSwizzle.fBase));
        for (size_t i = 0; i < fSwizzle.fComponents.size(); ++i) {
            ByteCodeGenerator::Location final = target.offset(fGenerator, fSwizzle.fComponents[i]);
            fGenerator.write(inst);
            fGenerator.write(final);
            fGenerator.write(src + i);
        }
    }

private:
    const Swizzle& fSwizzle;

    typedef ByteCodeGenerator::LValue INHERITED;
};

class ByteCodeExternalValueLValue : public ByteCodeGenerator::LValue {
public:
    ByteCodeExternalValueLValue(ByteCodeGenerator* generator, ExternalValue& value, int index)
        : INHERITED(*generator)
        , fIndex(index)
        , fSlotCount(ByteCodeGenerator::SlotCount(value.type())) {
        SkASSERT(fSlotCount <= 4);
    }

    void load(ByteCode::Register result) override {
        fGenerator.write(ByteCode::Instruction::kReadExternal);
        fGenerator.write(result);
        fGenerator.write((uint8_t) fSlotCount);
        fGenerator.write((uint8_t) fIndex);
    }

    void store(ByteCode::Register src) override {
        fGenerator.write(ByteCode::Instruction::kWriteExternal);
        fGenerator.write((uint8_t) fIndex);
        fGenerator.write((uint8_t) fSlotCount);
        fGenerator.write(src);
    }

private:
    typedef LValue INHERITED;

    int fIndex;

    int fSlotCount;
};

std::unique_ptr<ByteCodeGenerator::LValue> ByteCodeGenerator::getLValue(const Expression& expr) {
    switch (expr.fKind) {
        case Expression::kExternalValue_Kind: {
            ExternalValue* value = ((ExternalValueReference&) expr).fValue;
            int index = fOutput->fExternalValues.size();
            fOutput->fExternalValues.push_back(value);
            SkASSERT(index <= 255);
            return std::unique_ptr<LValue>(new ByteCodeExternalValueLValue(this, *value, index));
        }
        case Expression::kFieldAccess_Kind:
        case Expression::kIndex_Kind:
        case Expression::kVariableReference_Kind: {
            Location location = this->getLocation(expr);
            Variable::Storage storage = this->getStorage(expr);
            ByteCode::Instruction loadInst = this->getLoadInstruction(location, storage);
            ByteCode::Instruction storeInst = this->getStoreInstruction(location, storage);
            return std::unique_ptr<LValue>(new ByteCodeSimpleLValue(this, location,
                                                                    SlotCount(expr.fType),
                                                                    loadInst, storeInst));
        }
        case Expression::kSwizzle_Kind:
            return std::unique_ptr<LValue>(new ByteCodeSwizzleLValue(this, &(Swizzle&) expr));
        default:
            ABORT("unsupported lvalue\n");
    }
}

ByteCode::Register ByteCodeGenerator::next(int count) {
    SkASSERT(fNextRegister + count <= ByteCode::kRegisterMax);
    fNextRegister += count;
    return ByteCode::Register{(uint16_t) (fNextRegister - count)};
}

static TypeCategory type_category(const Type& type) {
    switch (type.kind()) {
        case Type::Kind::kVector_Kind:
        case Type::Kind::kMatrix_Kind:
            return type_category(type.componentType());
        default:
            String name = type.displayName();
            if (name == "bool") {
                return TypeCategory::kBool;
            } else if (name == "int" || name == "short") {
                return TypeCategory::kSigned;
            } else if (name == "uint" || name == "ushort") {
                return TypeCategory::kUnsigned;
            } else {
                SkASSERT(name == "float" || name == "half");
                return TypeCategory::kFloat;
            }
            ABORT("unsupported type: %s\n", name.c_str());
    }
}

void ByteCodeGenerator::write(ByteCode::Instruction inst, int count) {
    SkASSERT(count <= 255);
    if (count > 1) {
        this->write(VEC(inst));
        this->write((uint8_t) count);
    }
    else {
        this->write(inst);
    }
}

void ByteCodeGenerator::writeTypedInstruction(const Type& type, ByteCode::Instruction s,
                                              ByteCode::Instruction u, ByteCode::Instruction f) {
    switch (type_category(type)) {
        case TypeCategory::kSigned:
            this->write(s);
            break;
        case TypeCategory::kUnsigned:
            this->write(u);
            break;
        case TypeCategory::kFloat: {
            this->write(f);
            break;
        }
        default:
            SkASSERT(false);
    }
}

void ByteCodeGenerator::writeVectorBinaryInstruction(const Type& operandType,
                                                     ByteCode::Register left,
                                                     ByteCode::Register right,
                                                     ByteCode::Instruction s,
                                                     ByteCode::Instruction u,
                                                     ByteCode::Instruction f,
                                                     ByteCode::Register result) {
    uint8_t count = (uint8_t) SlotCount(operandType);
    if (count == 1) {
        this->writeTypedInstruction(operandType, s, u, f);
    }
    else {
        this->writeTypedInstruction(operandType, VEC(s), VEC(u), VEC(f));
        this->write(count);
    }
    this->write(result);
    this->write(left);
    this->write(right);
}

void ByteCodeGenerator::writeBinaryInstruction(const Type& operandType,
                                               ByteCode::Register left,
                                               ByteCode::Register right,
                                               ByteCode::Instruction s,
                                               ByteCode::Instruction u,
                                               ByteCode::Instruction f,
                                               ByteCode::Register result) {
    for (int i = 0; i < SlotCount(operandType); ++i) {
        this->writeTypedInstruction(operandType, s, u, f);
        this->write(result + i);
        this->write(left + i);
        this->write(right + i);
    }
}

void ByteCodeGenerator::writeBinaryExpression(const BinaryExpression& b,
                                              ByteCode::Register result) {
    if (b.fOperator == Token::Kind::EQ) {
        std::unique_ptr<LValue> lvalue = this->getLValue(*b.fLeft);
        this->writeExpression(*b.fRight, result);
        lvalue->store(result);
        return;
    }
    const Type& lType = b.fLeft->fType;
    const Type& rType = b.fRight->fType;
    bool lVecOrMtx = (lType.kind() == Type::kVector_Kind || lType.kind() == Type::kMatrix_Kind);
    bool rVecOrMtx = (rType.kind() == Type::kVector_Kind || rType.kind() == Type::kMatrix_Kind);
    const Type* operandType;
    if (!lVecOrMtx && rVecOrMtx) {
        operandType = &rType;
    } else {
        operandType = &lType;
    }
    Token::Kind op;
    std::unique_ptr<LValue> lvalue;
    ByteCode::Register left;
    switch (b.fOperator) {
        case Token::Kind::LOGICALAND:
        case Token::Kind::LOGICALANDEQ:
        case Token::Kind::LOGICALOR:
        case Token::Kind::LOGICALOREQ:
            left = result;
            break;
        default:
            left = this->next(SlotCount(*operandType));
    }
    if (is_assignment(b.fOperator)) {
        lvalue = this->getLValue(*b.fLeft);
        lvalue->load(left);
        op = remove_assignment(b.fOperator);
    } else {
        this->writeExpression(*b.fLeft, left);
        op = b.fOperator;
        if (!lVecOrMtx && rVecOrMtx) {
            this->write(ByteCode::Instruction::kSplat);
            this->write((uint8_t) (SlotCount(rType) - 1));
            this->write(left + 1);
            this->write(left);
        }
    }
    SkDEBUGCODE(TypeCategory tc = type_category(lType));
    int count = std::max(SlotCount(lType), SlotCount(rType));
    switch (op) {
        case Token::Kind::LOGICALAND: {
            SkASSERT(left.fIndex == result.fIndex);
            this->write(ByteCode::Instruction::kMaskPush);
            ++fConditionCount;
            this->write(left);
            this->write(ByteCode::Instruction::kBranchIfAllFalse);
            DeferredLocation falseLocation(this);
            SkASSERT(SlotCount(b.fRight->fType) == 1);
            ByteCode::Register right = this->next(1);
            this->writeExpression(*b.fRight, right);
            this->write(ByteCode::Instruction::kAnd);
            this->write(result);
            this->write(left);
            this->write(right);
            falseLocation.set();
            --fConditionCount;
            this->write(ByteCode::Instruction::kMaskPop);
            return;
        }
        case Token::Kind::LOGICALOR: {
            SkASSERT(left.fIndex == result.fIndex);
            ByteCode::Register mask = this->next(1);
            this->write(ByteCode::Instruction::kNot);
            this->write(mask);
            this->write(left);
            this->write(ByteCode::Instruction::kMaskPush);
            ++fConditionCount;
            this->write(mask);
            this->write(ByteCode::Instruction::kBranchIfAllFalse);
            DeferredLocation falseLocation(this);
            SkASSERT(SlotCount(b.fRight->fType) == 1);
            ByteCode::Register right = this->next(1);
            this->writeExpression(*b.fRight, right);
            this->write(ByteCode::Instruction::kOr);
            this->write(result);
            this->write(left);
            this->write(right);
            falseLocation.set();
            --fConditionCount;
            this->write(ByteCode::Instruction::kMaskPop);
            return;
        }
        case Token::Kind::SHL:
        case Token::Kind::SHR: {
            SkASSERT(count == 1 && (tc == SkSL::TypeCategory::kSigned ||
                                    tc == SkSL::TypeCategory::kUnsigned));
            if (!b.fRight->isConstant()) {
                fErrors.error(b.fRight->fOffset, "Shift amounts must be constant");
                return;
            }
            int64_t shift = b.fRight->getConstantInt();
            if (shift < 0 || shift > 31) {
                fErrors.error(b.fRight->fOffset, "Shift amount out of range");
                return;
            }

            if (op == Token::Kind::SHL) {
                this->write(ByteCode::Instruction::kShiftLeft);
            } else {
                this->write(type_category(lType) == TypeCategory::kSigned
                                ? ByteCode::Instruction::kShiftRightS
                                : ByteCode::Instruction::kShiftRightU);
            }
            this->write(result);
            this->write(left);
            this->write((uint8_t) shift);
            return;
        }
        case Token::Kind::STAR:
            // Special case for M*V, V*M, M*M (but not V*V!)
            if (lType.columns() > 1 && rType.columns() > 1 &&
                (lType.rows() > 1 || rType.rows() > 1)) {
                ByteCode::Register right = this->next(SlotCount(rType));
                this->writeExpression(*b.fRight, right);
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
                this->write(ByteCode::Instruction::kMatrixMultiply);
                this->write(result);
                this->write(left);
                this->write(right);
                this->write((uint8_t) lCols);
                this->write((uint8_t) lRows);
                this->write((uint8_t) rCols);
                return;
            }

        default:
            break;
    }
    ByteCode::Register right = this->next(SlotCount(*operandType));
    this->writeExpression(*b.fRight, right);
    if (lVecOrMtx && !rVecOrMtx) {
        this->write(ByteCode::Instruction::kSplat);
        this->write((uint8_t) (SlotCount(*operandType) - 1));
        this->write(right + 1);
        this->write(right);
    }
    switch (op) {
        case Token::Kind::EQEQ:
            this->writeBinaryInstruction(*operandType, left, right,
                                         ByteCode::Instruction::kCompareEQI,
                                         ByteCode::Instruction::kCompareEQI,
                                         ByteCode::Instruction::kCompareEQF,
                                         result);
            // Collapse to a single bool
            for (int i = 1; i < count; ++i) {
                this->write(ByteCode::Instruction::kAnd);
                this->write(result);
                this->write(result);
                this->write(result + i);
            }
            break;
        case Token::Kind::GT:
            this->writeBinaryInstruction(*operandType, left, right,
                                         ByteCode::Instruction::kCompareGTS,
                                         ByteCode::Instruction::kCompareGTU,
                                         ByteCode::Instruction::kCompareGTF,
                                         result);
            break;
        case Token::Kind::GTEQ:
            this->writeBinaryInstruction(*operandType, left, right,
                                         ByteCode::Instruction::kCompareGTEQS,
                                         ByteCode::Instruction::kCompareGTEQU,
                                         ByteCode::Instruction::kCompareGTEQF,
                                         result);
            break;
        case Token::Kind::LT:
            this->writeBinaryInstruction(*operandType, left, right,
                                         ByteCode::Instruction::kCompareLTS,
                                         ByteCode::Instruction::kCompareLTU,
                                         ByteCode::Instruction::kCompareLTF,
                                         result);
            break;
        case Token::Kind::LTEQ:
            this->writeBinaryInstruction(*operandType, left, right,
                                         ByteCode::Instruction::kCompareLTEQS,
                                         ByteCode::Instruction::kCompareLTEQU,
                                         ByteCode::Instruction::kCompareLTEQF,
                                         result);
            break;
        case Token::Kind::MINUS:
            this->writeVectorBinaryInstruction(*operandType, left, right,
                                               ByteCode::Instruction::kSubtractI,
                                               ByteCode::Instruction::kSubtractI,
                                               ByteCode::Instruction::kSubtractF,
                                               result);
            break;
        case Token::Kind::NEQ:
            this->writeBinaryInstruction(*operandType, left, right,
                                         ByteCode::Instruction::kCompareNEQI,
                                         ByteCode::Instruction::kCompareNEQI,
                                         ByteCode::Instruction::kCompareNEQF,
                                         result);
            // Collapse to a single bool
            for (int i = 1; i < count; ++i) {
                this->write(ByteCode::Instruction::kOr);
                this->write(result);
                this->write(result);
                this->write(result + i);
            }
            break;
        case Token::Kind::PERCENT:
            this->writeVectorBinaryInstruction(*operandType, left, right,
                                               ByteCode::Instruction::kRemainderS,
                                               ByteCode::Instruction::kRemainderU,
                                               ByteCode::Instruction::kRemainderF,
                                               result);
            break;
        case Token::Kind::PLUS:
            this->writeVectorBinaryInstruction(*operandType, left, right,
                                               ByteCode::Instruction::kAddI,
                                               ByteCode::Instruction::kAddI,
                                               ByteCode::Instruction::kAddF,
                                               result);
            break;
        case Token::Kind::SLASH:
            this->writeVectorBinaryInstruction(*operandType, left, right,
                                               ByteCode::Instruction::kDivideS,
                                               ByteCode::Instruction::kDivideU,
                                               ByteCode::Instruction::kDivideF,
                                               result);
            break;
        case Token::Kind::STAR:
            this->writeVectorBinaryInstruction(*operandType, left, right,
                                               ByteCode::Instruction::kMultiplyI,
                                               ByteCode::Instruction::kMultiplyI,
                                               ByteCode::Instruction::kMultiplyF,
                                               result);
            break;
        case Token::Kind::LOGICALXOR: {
            SkASSERT(tc == SkSL::TypeCategory::kBool);
            this->write(ByteCode::Instruction::kXor);
            this->write(result);
            this->write(left);
            this->write(right);
            break;
        }
        case Token::Kind::BITWISEAND: {
            SkASSERT(tc == SkSL::TypeCategory::kSigned || tc == SkSL::TypeCategory::kUnsigned);
            this->write(ByteCode::Instruction::kAnd);
            this->write(result);
            this->write(left);
            this->write(right);
            break;
        }
        case Token::Kind::BITWISEOR: {
            SkASSERT(tc == SkSL::TypeCategory::kSigned || tc == SkSL::TypeCategory::kUnsigned);
            this->write(ByteCode::Instruction::kOr);
            this->write(result);
            this->write(left);
            this->write(right);
            break;
        }
        case Token::Kind::BITWISEXOR: {
            SkASSERT(tc == SkSL::TypeCategory::kSigned || tc == SkSL::TypeCategory::kUnsigned);
            this->write(ByteCode::Instruction::kXor);
            this->write(result);
            this->write(left);
            this->write(right);
            break;
        }
        default:
            fErrors.error(b.fOffset, SkSL::String::printf("Unsupported binary operator '%s'",
                                                          Compiler::OperatorName(op)));
            break;
    }
    if (lvalue) {
        lvalue->store(result);
    }
}

void ByteCodeGenerator::writeConstructor(const Constructor& c, ByteCode::Register result) {
    if (c.fType.rows() > 1) {
        if (c.fArguments.size() == 1) {
            if (SlotCount(c.fArguments[0]->fType) == 1) {
                ByteCode::Register v = this->next(1);
                this->writeExpression(*c.fArguments[0], v);
                this->write(ByteCode::Instruction::kScalarToMatrix);
                this->write(result);
                this->write(v);
                this->write((uint8_t) c.fType.columns());
                this->write((uint8_t) c.fType.rows());
                return;
            } else if (c.fArguments[0]->fType.rows() > 1) {
                ByteCode::Register v = this->next(SlotCount(c.fArguments[0]->fType));
                this->writeExpression(*c.fArguments[0], v);
                this->write(ByteCode::Instruction::kMatrixToMatrix);
                this->write(result);
                this->write(v);
                this->write((uint8_t) c.fArguments[0]->fType.columns());
                this->write((uint8_t) c.fArguments[0]->fType.rows());
                this->write((uint8_t) c.fType.columns());
                this->write((uint8_t) c.fType.rows());
                return;
            }
        }
        int offset = 0;
        for (const auto& arg : c.fArguments) {
            this->writeExpression(*arg, ByteCode::Register{(uint16_t) (result.fIndex + offset)});
            offset += SlotCount(arg->fType);
        }
        return;
    }
    if (c.fArguments.size() == 1 && c.fArguments[0]->fType.columns() == 1 &&
        c.fType.columns() > 1) {
        SkASSERT(SlotCount(c.fArguments[0]->fType) == 1);
        ByteCode::Register v = result;
        this->writeExpression(*c.fArguments[0], v);
        this->write(ByteCode::Instruction::kSplat);
        this->write((uint8_t) (c.fType.columns() - 1));
        this->write(v + 1);
        this->write(v);
        return;
    }
    ByteCode::Instruction inst;
    switch (type_category(c.fArguments[0]->fType)) {
        case TypeCategory::kSigned:
            if (type_category(c.fType) == TypeCategory::kFloat) {
                inst = ByteCode::Instruction::kSignedToFloat;
            } else {
                inst = ByteCode::Instruction::kNop;
            }
            break;
        case TypeCategory::kUnsigned:
            if (type_category(c.fType) == TypeCategory::kFloat) {
                inst = ByteCode::Instruction::kUnsignedToFloat;
            } else {
                inst = ByteCode::Instruction::kNop;
            }
            break;
        case TypeCategory::kFloat:
            if (type_category(c.fType) == TypeCategory::kSigned) {
                inst = ByteCode::Instruction::kFloatToSigned;
            } else if (type_category(c.fType) == TypeCategory::kUnsigned) {
                inst = ByteCode::Instruction::kFloatToUnsigned;
            } else {
                inst = ByteCode::Instruction::kNop;
            }
            break;
        default:
            SkASSERT(false);
            return;
    }
    ByteCode::Register values;
    if (inst == ByteCode::Instruction::kNop) {
        values = result;
    } else {
        values = this->next(SlotCount(c.fType));
    }
    ByteCode::Register v = values;
    for (size_t i = 0; i < c.fArguments.size(); ++i) {
        this->writeExpression(*c.fArguments[i], v);
        v.fIndex += SlotCount(c.fArguments[i]->fType);
    }
    if (inst != ByteCode::Instruction::kNop) {
        v = values;
        ByteCode::Register target = result;
        for (size_t i = 0; i < c.fArguments.size(); ++i) {
            int count = SlotCount(c.fArguments[i]->fType);
            for (int j = 0; j < count; ++j) {
                this->write(inst);
                this->write(target);
                ++target.fIndex;
                this->write(v + j);
            }
        }
    }
}

void ByteCodeGenerator::writeExternalFunctionCall(const ExternalFunctionCall& f,
                                                  ByteCode::Register result) {
    int argumentCount = 0;
    for (const auto& arg : f.fArguments) {
        argumentCount += SlotCount(arg->fType);
    }
    ByteCode::Register args = this->next(argumentCount);
    argumentCount = 0;
    for (const auto& arg : f.fArguments) {
        this->writeExpression(*arg, args + argumentCount);
        argumentCount += SlotCount(arg->fType);
    }
    this->write(ByteCode::Instruction::kCallExternal);
    this->write(result);
    int index = fOutput->fExternalValues.size();
    fOutput->fExternalValues.push_back(f.fFunction);
    SkASSERT(index <= 255);
    this->write((uint8_t) index);
    SkASSERT(SlotCount(f.fType) <= 255);
    this->write((uint8_t) SlotCount(f.fType));
    this->write(args);
    SkASSERT(argumentCount <= 255);
    this->write((uint8_t) argumentCount);
}

void ByteCodeGenerator::writeExternalValue(const ExternalValueReference& e,
                                           ByteCode::Register result) {
    this->write(ByteCode::Instruction::kReadExternal);
    this->write(result);
    this->write((uint8_t) SlotCount(e.fValue->type()));
    int index = fOutput->fExternalValues.size();
    fOutput->fExternalValues.push_back(e.fValue);
    SkASSERT(index <= 255);
    this->write((uint8_t) index);
}

void ByteCodeGenerator::writeIntrinsicCall(const FunctionCall& c, Intrinsic intrinsic,
                                           ByteCode::Register result) {
    if (intrinsic.fIsSpecial) {
        switch (intrinsic.fValue.fSpecial) {
            case SpecialIntrinsic::kDot: {
                SkASSERT(c.fArguments.size() == 2);
                int count = SlotCount(c.fArguments[0]->fType);
                ByteCode::Register left = this->next(count);
                this->writeExpression(*c.fArguments[0], left);
                ByteCode::Register right = this->next(count);
                this->writeExpression(*c.fArguments[1], right);
                ByteCode::Register product = this->next(count);
                this->writeTypedInstruction(c.fType,
                                            ByteCode::Instruction::kMultiplyIN,
                                            ByteCode::Instruction::kMultiplyIN,
                                            ByteCode::Instruction::kMultiplyFN);
                this->write((uint8_t) count);
                this->write(product);
                this->write(left);
                this->write(right);
                ByteCode::Register total = product;
                for (int i = 1; i < count; ++i) {
                    this->writeTypedInstruction(c.fType,
                                                ByteCode::Instruction::kAddI,
                                                ByteCode::Instruction::kAddI,
                                                ByteCode::Instruction::kAddF);
                    ByteCode::Register sum = i == count - 1 ? result : this->next(1);
                    this->write(sum);
                    this->write(total);
                    this->write(product + i);
                    total = sum;
                }
                break;
            }
            case SpecialIntrinsic::kInverse: {
                SkASSERT(c.fArguments.size() == 1);
                int count = SlotCount(c.fArguments[0]->fType);
                ByteCode::Register arg = this->next(count);
                this->writeExpression(*c.fArguments[0], arg);
                switch (SlotCount(c.fArguments[0]->fType)) {
                    case 4:  this->write(ByteCode::Instruction::kInverse2x2); break;
                    case 9:  this->write(ByteCode::Instruction::kInverse3x3); break;
                    case 16: this->write(ByteCode::Instruction::kInverse4x4); break;
                    default: SkASSERT(false);
                }
                this->write(result);
                this->write(arg);
                break;
            }
            case SpecialIntrinsic::kPrint: {
                SkASSERT(c.fArguments.size() == 1);
                SkASSERT(SlotCount(c.fArguments[0]->fType) == 1);
                ByteCode::Register arg = this->next(1);
                this->writeExpression(*c.fArguments[0], arg);
                this->write(ByteCode::Instruction::kPrint);
                this->write(arg);
                break;
            }
        }
    } else {
        int count = SlotCount(c.fType);
        std::vector<ByteCode::Register> argRegs;
        for (const auto& expr : c.fArguments) {
            SkASSERT(SlotCount(expr->fType) == count);
            ByteCode::Register reg = this->next(count);
            this->writeExpression(*expr, reg);
            argRegs.push_back(reg);
        }
        for (int i = 0; i < count; ++i) {
            this->write(intrinsic.fValue.fInstruction);
            if (c.fType.fName != "void") {
                this->write(result + i);
            }
            for (ByteCode::Register arg : argRegs) {
                this->write(arg + i);
            }
        }
    }
}

void ByteCodeGenerator::writeFunctionCall(const FunctionCall& c, ByteCode::Register result) {
    auto found = fIntrinsics.find(c.fFunction.fName);
    if (found != fIntrinsics.end()) {
        return this->writeIntrinsicCall(c, found->second, result);
    }
    int argCount = c.fArguments.size();
    std::vector<std::unique_ptr<LValue>> lvalues;
    int parameterSlotCount = 0;
    for (const auto& p : c.fFunction.fParameters) {
        parameterSlotCount += SlotCount(p->fType);
    }
    ByteCode::Register argStart = this->next(parameterSlotCount);
    ByteCode::Register nextArg = argStart;
    for (int i = 0; i < argCount; ++i) {
        const auto& param = c.fFunction.fParameters[i];
        const auto& arg = c.fArguments[i];
        if (param->fModifiers.fFlags & Modifiers::kOut_Flag) {
            lvalues.emplace_back(this->getLValue(*arg));
            lvalues.back()->load(nextArg);
        } else {
            this->writeExpression(*arg, nextArg);
        }
        nextArg.fIndex += SlotCount(arg->fType);
    }
    // Find the index of the function we're calling. We explicitly do not allow calls to functions
    // before they're defined. This is an easy-to-understand rule that prevents recursion.
    size_t idx;
    for (idx = 0; idx < fFunctions.size(); ++idx) {
        if (c.fFunction.matches(fFunctions[idx]->fDeclaration)) {
            break;
        }
    }
    if (idx > 255) {
        fErrors.error(c.fOffset, "Function count limit exceeded");
        return;
    } else if (idx >= fOutput->fFunctions.size()) {
        fErrors.error(c.fOffset, "Call to undefined function");
        return;
    }

    this->write(ByteCode::Instruction::kCall);
    this->write(result);
    this->write((uint8_t) idx);
    this->write(argStart);
    nextArg = argStart;
    auto lvalue = lvalues.begin();
    for (int i = 0; i < argCount; ++i) {
        const auto& param = c.fFunction.fParameters[i];
        if (param->fModifiers.fFlags & Modifiers::kOut_Flag) {
            (*(lvalue++))->store(nextArg);
        }
        nextArg.fIndex += SlotCount(param->fType);
    }
}

void ByteCodeGenerator::incOrDec(Token::Kind op, Expression& operand, bool prefix,
                                 ByteCode::Register result) {
    SkASSERT(op == Token::Kind::PLUSPLUS || op == Token::Kind::MINUSMINUS);
    std::unique_ptr<LValue> lvalue = this->getLValue(operand);
    SkASSERT(SlotCount(operand.fType) == 1);
    ByteCode::Register value;
    if (prefix) {
        value = this->next(1);
    } else {
        value = result;
    }
    lvalue->load(value);
    ByteCode::Register one = this->next(1);
    this->write(ByteCode::Instruction::kImmediate);
    this->write(one);
    if (type_category(operand.fType) == TypeCategory::kFloat) {
        this->write(ByteCode::Immediate(1.0f));
    } else {
        this->write(ByteCode::Immediate((int32_t) 1));
    }
    if (op == Token::Kind::PLUSPLUS) {
        this->writeTypedInstruction(operand.fType,
                                    ByteCode::Instruction::kAddI,
                                    ByteCode::Instruction::kAddI,
                                    ByteCode::Instruction::kAddF);
    } else {
        this->writeTypedInstruction(operand.fType,
                                    ByteCode::Instruction::kSubtractI,
                                    ByteCode::Instruction::kSubtractI,
                                    ByteCode::Instruction::kSubtractF);
    }
    if (prefix) {
        this->write(result);
        this->write(value);
        this->write(one);
        lvalue->store(result);
    } else {
        ByteCode::Register temp = this->next(1);
        this->write(temp);
        this->write(value);
        this->write(one);
        lvalue->store(temp);
    }
}

void ByteCodeGenerator::writePostfixExpression(const PostfixExpression& p,
                                               ByteCode::Register result) {
    this->incOrDec(p.fOperator, *p.fOperand, false, result);
}

void ByteCodeGenerator::writePrefixExpression(const PrefixExpression& p,
                                              ByteCode::Register result) {
    switch (p.fOperator) {
        case Token::Kind::PLUSPLUS:
        case Token::Kind::MINUSMINUS: {
            return this->incOrDec(p.fOperator, *p.fOperand, true, result);
        }
        case Token::Kind::MINUS: {
            ByteCode::Register src = this->next(SlotCount(p.fType));
            this->writeExpression(*p.fOperand, src);
            for (int i = 0; i < SlotCount(p.fType); ++i) {
                this->writeTypedInstruction(p.fType,
                                            ByteCode::Instruction::kNegateS,
                                            ByteCode::Instruction::kNegateS,
                                            ByteCode::Instruction::kNegateF);
                this->write(result + i);
                this->write(src + i);
            }
            break;
        }
        case Token::Kind::LOGICALNOT:
        case Token::Kind::BITWISENOT: {
            ByteCode::Register src = this->next(SlotCount(p.fType));
            this->writeExpression(*p.fOperand, src);
            for (int i = 0; i < SlotCount(p.fType); ++i) {
                this->write(ByteCode::Instruction::kNot);
                this->write(result + i);
                this->write(src + i);
            }
            break;
        }
        default:
            SkASSERT(false);
    }
}

void ByteCodeGenerator::writeSwizzle(const Swizzle& s, ByteCode::Register result) {
    if (swizzle_is_simple(s)) {
        this->writeVariableExpression(s, result);
        return;
    }
    ByteCode::Register base = this->writeExpression(*s.fBase);
    for (int i = 0; i < (int) s.fComponents.size(); ++i) {
        this->write(ByteCode::Instruction::kCopy);
        this->write(result + i);
        this->write(base + s.fComponents[i]);
    }
}

void ByteCodeGenerator::writeTernaryExpression(const TernaryExpression& t,
                                               ByteCode::Register result) {
    int count = SlotCount(t.fType);
    SkASSERT(count == SlotCount(t.fIfTrue->fType));
    SkASSERT(count == SlotCount(t.fIfFalse->fType));

    ByteCode::Register test = this->writeExpression(*t.fTest);
    this->write(ByteCode::Instruction::kMaskPush);
    ++fConditionCount;
    this->write(test);
    ByteCode::Register ifTrue = this->writeExpression(*t.fIfTrue);
    this->write(ByteCode::Instruction::kMaskNegate);
    ByteCode::Register ifFalse = this->writeExpression(*t.fIfFalse);
    --fConditionCount;
    this->write(ByteCode::Instruction::kMaskPop);
    for (int i = 0; i < count; ++i) {
        this->write(ByteCode::Instruction::kSelect);
        this->write(result + i);
        this->write(test);
        this->write(ifTrue + i);
        this->write(ifFalse + i);
    }
}

void ByteCodeGenerator::writeVariableExpression(const Expression& expr,
                                                ByteCode::Register result) {
    ByteCodeGenerator::Location location = this->getLocation(expr);
    int count = SlotCount(expr.fType);
    ByteCode::Instruction load = this->getLoadInstruction(location, this->getStorage(expr));
    this->write(load, count);
    this->write(result);
    this->write(location);
}

void ByteCodeGenerator::writeExpression(const Expression& expr, ByteCode::Register result) {
    switch (expr.fKind) {
        case Expression::kBoolLiteral_Kind: {
            this->write(ByteCode::Instruction::kImmediate);
            this->write(result);
            this->write(ByteCode::Immediate((int32_t) (((BoolLiteral&) expr).fValue ? -1 : 0)));
            break;
        }
        case Expression::kBinary_Kind: {
            this->writeBinaryExpression((BinaryExpression&) expr, result);
            break;
        }
        case Expression::kConstructor_Kind: {
            this->writeConstructor((Constructor&) expr, result);
            break;
        }
        case Expression::kExternalFunctionCall_Kind:
            this->writeExternalFunctionCall((ExternalFunctionCall&) expr, result);
            break;
        case Expression::kExternalValue_Kind:
            this->writeExternalValue((ExternalValueReference&) expr, result);
            break;
        case Expression::kFloatLiteral_Kind: {
            this->write(ByteCode::Instruction::kImmediate);
            this->write(result);
            this->write(ByteCode::Immediate((float) ((FloatLiteral&) expr).fValue));
            break;
        }
        case Expression::kFunctionCall_Kind: {
            this->writeFunctionCall((FunctionCall&) expr, result);
            break;
        }
        case Expression::kIntLiteral_Kind: {
            this->write(ByteCode::Instruction::kImmediate);
            this->write(result);
            this->write(ByteCode::Immediate((int32_t) ((IntLiteral&) expr).fValue));
            break;
        }
        case Expression::kPostfix_Kind:
            this->writePostfixExpression((PostfixExpression&) expr, result);
            break;
        case Expression::kPrefix_Kind:
            this->writePrefixExpression((PrefixExpression&) expr, result);
            break;
        case Expression::kSwizzle_Kind:
            this->writeSwizzle((Swizzle&) expr, result);
            break;
        case Expression::kTernary_Kind:
            this->writeTernaryExpression((TernaryExpression&) expr, result);
            break;
        case Expression::kFieldAccess_Kind:
        case Expression::kIndex_Kind:
        case Expression::kVariableReference_Kind:
            this->writeVariableExpression(expr, result);
            break;
        default:
#ifdef SK_DEBUG
            ABORT("unsupported lvalue %s\n", expr.description().c_str());
#endif
            break;
    }
}

ByteCode::Register ByteCodeGenerator::writeExpression(const Expression& expr) {
    ByteCode::Register result = this->next(SlotCount(expr.fType));
    this->writeExpression(expr, result);
    return result;
}

void ByteCodeGenerator::writeBlock(const Block& b) {
    for (const auto& s : b.fStatements) {
        this->writeStatement(*s);
    }
}

void ByteCodeGenerator::writeDoStatement(const DoStatement& d) {
    this->write(ByteCode::Instruction::kLoopBegin);
    ++fConditionCount;
    SkASSERT(fCode->size() < ByteCode::kPointerMax);
    ByteCode::Pointer start{(uint16_t) fCode->size()};
    this->writeStatement(*d.fStatement);
    ByteCode::Register test = this->writeExpression(*d.fTest);
    this->write(ByteCode::Instruction::kLoopNext);
    this->write(ByteCode::Instruction::kLoopMask);
    this->write(test);
    this->write(ByteCode::Instruction::kBranchIfAllFalse);
    DeferredLocation endLocation(this);
    this->write(ByteCode::Instruction::kBranch);
    this->write(start);
    endLocation.set();
    --fConditionCount;
    this->write(ByteCode::Instruction::kLoopEnd);
}

void ByteCodeGenerator::writeForStatement(const ForStatement& f) {
    if (f.fInitializer) {
        this->writeStatement(*f.fInitializer);
    }
    this->write(ByteCode::Instruction::kLoopBegin);
    ++fConditionCount;
    ByteCode::Pointer start{(uint16_t) fCode->size()};
    if (f.fTest) {
        ByteCode::Register test = this->writeExpression(*f.fTest);
        this->write(ByteCode::Instruction::kLoopMask);
        this->write(test);
    }
    this->write(ByteCode::Instruction::kBranchIfAllFalse);
    DeferredLocation endLocation(this);
    this->writeStatement(*f.fStatement);
    this->write(ByteCode::Instruction::kLoopNext);
    if (f.fNext) {
        this->writeExpression(*f.fNext);
    }
    this->write(ByteCode::Instruction::kBranch);
    this->write(start);
    endLocation.set();
    --fConditionCount;
    this->write(ByteCode::Instruction::kLoopEnd);
}

void ByteCodeGenerator::writeIfStatement(const IfStatement& i) {
    ByteCode::Register test = this->writeExpression(*i.fTest);
    this->write(ByteCode::Instruction::kMaskPush);
    ++fConditionCount;
    this->write(test);
    this->write(ByteCode::Instruction::kBranchIfAllFalse);
    DeferredLocation falseLocation(this);
    this->writeStatement(*i.fIfTrue);
    falseLocation.set();
    if (i.fIfFalse) {
        this->write(ByteCode::Instruction::kMaskNegate);
        this->write(ByteCode::Instruction::kBranchIfAllFalse);
        DeferredLocation endLocation(this);
        this->writeStatement(*i.fIfFalse);
        endLocation.set();
    }
    --fConditionCount;
    this->write(ByteCode::Instruction::kMaskPop);
}

void ByteCodeGenerator::writeReturn(const ReturnStatement& r) {
    if (fConditionCount) {
        fErrors.error(r.fOffset, "return not allowed inside conditional or loop");
        return;
    }
    if (r.fExpression) {
        ByteCode::Register value = this->writeExpression(*r.fExpression);
        this->write(ByteCode::Instruction::kReturnValue);
        this->write(value);
    }
    else {
        this->write(ByteCode::Instruction::kReturn);
    }
}

void ByteCodeGenerator::writeVarDeclarations(const VarDeclarations& v) {
    for (const auto& declStatement : v.fVars) {
        const VarDeclaration& decl = (VarDeclaration&) *declStatement;
        // we need to grab the location even if we don't use it, to ensure it
        // has been allocated
        ByteCodeGenerator::Location location = this->getLocation(*decl.fVar);
        if (decl.fValue) {
            ByteCode::Register src = this->writeExpression(*decl.fValue);
            uint8_t count = (uint8_t) SlotCount(decl.fVar->fType);
            this->write(ByteCode::Instruction::kStoreStackDirect, count);
            this->write(location);
            this->write(src);
        }
    }
}

void ByteCodeGenerator::writeWhileStatement(const WhileStatement& w) {
    this->write(ByteCode::Instruction::kLoopBegin);
    ++fConditionCount;
    SkASSERT(fCode->size() < ByteCode::kPointerMax);
    ByteCode::Pointer start{(uint16_t) fCode->size()};
    ByteCode::Register test = this->writeExpression(*w.fTest);
    this->write(ByteCode::Instruction::kLoopMask);
    this->write(test);
    this->write(ByteCode::Instruction::kBranchIfAllFalse);
    DeferredLocation endLocation(this);
    this->writeStatement(*w.fStatement);
    this->write(ByteCode::Instruction::kLoopNext);
    this->write(ByteCode::Instruction::kBranch);
    this->write(start);
    endLocation.set();
    --fConditionCount;
    this->write(ByteCode::Instruction::kLoopEnd);
}

void ByteCodeGenerator::writeStatement(const Statement& s) {
    switch (s.fKind) {
        case Statement::kBlock_Kind:
            this->writeBlock((Block&) s);
            break;
        case Statement::kBreak_Kind:
            this->write(ByteCode::Instruction::kBreak);
            break;
        case Statement::kContinue_Kind:
            this->write(ByteCode::Instruction::kContinue);
            break;
        case Statement::kDo_Kind:
            this->writeDoStatement((DoStatement&) s);
            break;
        case Statement::kExpression_Kind:
            this->writeExpression(*((ExpressionStatement&) s).fExpression);
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
            this->writeReturn((ReturnStatement&) s);
            break;
        case Statement::kVarDeclarations_Kind:
            this->writeVarDeclarations(*((VarDeclarationsStatement&) s).fDeclaration);
            break;
        case Statement::kWhile_Kind:
            this->writeWhileStatement((WhileStatement&) s);
            break;
        default:
            ABORT("unsupported statement\n");
    }
}

void ByteCodeGenerator::writeFunction(const FunctionDefinition& f) {
    fFunction = &f;
    std::unique_ptr<ByteCodeFunction> result(new ByteCodeFunction(&f.fDeclaration));
    result->fReturnSlotCount = SlotCount(f.fDeclaration.fReturnType);
    fParameterCount = 0;
    fConditionCount = 0;
    for (const auto& p : f.fDeclaration.fParameters) {
        int count = SlotCount(p->fType);
        bool isOut = ((p->fModifiers.fFlags & Modifiers::kOut_Flag) != 0);
        result->fParameters.push_back(ByteCodeFunction::Parameter{count, isOut});
        fParameterCount += count;
    }
    result->fParameterSlotCount = fParameterCount;
    fCode = &result->fCode;
    this->writeStatement(*f.fBody);
    result->fStackSlotCount = fLocals.size();
    if (f.fDeclaration.fReturnType.fName == "void") {
        this->write(ByteCode::Instruction::kReturn);
    } else {
        this->write(ByteCode::Instruction::kAbort);
    }
    fOutput->fFunctions.push_back(std::move(result));
    SkASSERT(fConditionCount == 0);
}

void ByteCodeGenerator::gatherUniforms(const Type& type, const String& name) {
    if (type.kind() == Type::kOther_Kind) {
        return;
    } else if (type.kind() == Type::kStruct_Kind) {
        for (const auto& f : type.fields()) {
            this->gatherUniforms(*f.fType, name + "." + f.fName);
        }
    } else if (type.kind() == Type::kArray_Kind) {
        for (int i = 0; i < type.columns(); ++i) {
            this->gatherUniforms(type.componentType(), String::printf("%s[%d]", name.c_str(), i));
        }
    } else {
        fOutput->fUniforms.push_back({ name, type_category(type), type.rows(), type.columns(),
                                       fOutput->fUniformSlotCount });
        fOutput->fUniformSlotCount += type.columns() * type.rows();
    }
}

bool ByteCodeGenerator::generateCode() {
    fOutput->fGlobalSlotCount = 0;
    fOutput->fUniformSlotCount = 0;
    for (const auto& pe : fProgram) {
        if (pe.fKind == ProgramElement::kVar_Kind) {
            VarDeclarations& decl = (VarDeclarations&) pe;
            for (const auto& v : decl.fVars) {
                const Variable* declVar = ((VarDeclaration&) *v).fVar;
                if (declVar->fModifiers.fLayout.fBuiltin >= 0 || is_in(*declVar)) {
                    continue;
                }
                if (is_uniform(*declVar)) {
                    this->gatherUniforms(declVar->fType, declVar->fName);
                } else {
                    fOutput->fGlobalSlotCount += SlotCount(declVar->fType);
                }
            }
        }
    }
    for (const auto& pe : fProgram) {
        if (pe.fKind == ProgramElement::kFunction_Kind) {
            FunctionDefinition& f = (FunctionDefinition&) pe;
            fFunctions.push_back(&f);
            this->writeFunction(f);
        }
    }
    return fErrors.errorCount() == 0;
}

} // namespace
