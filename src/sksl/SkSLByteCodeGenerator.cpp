/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLByteCodeGenerator.h"

namespace SkSL {

ByteCodeGenerator::ByteCodeGenerator(const Context* context, const Program* program,
                                     ErrorReporter* errors, ByteCode* output)
    : INHERITED(program, errors, nullptr)
    , fContext(*context)
    , fOutput(output)
    , fIntrinsics {
        { "cos",     ByteCode::Instruction::kCos },
        { "dot",     SpecialIntrinsic::kDot },
        { "sin",     ByteCode::Instruction::kSin },
        { "sqrt",    ByteCode::Instruction::kSqrt },
        { "tan",     ByteCode::Instruction::kTan },
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
            uint16_t result = fLocals.size() + fParameterCount;
            fLocals.push_back(&var);
            for (int i = 0; i < SlotCount(var.fType) - 1; ++i) {
                fLocals.push_back(nullptr);
            }
            SkASSERT(result <= ByteCode::kPointerMax);
            return ByteCode::Pointer{result};
        }
        case Variable::kParameter_Storage: {
            uint16_t offset = 0;
            for (const auto& p : fFunction->fDeclaration.fParameters) {
                if (p == &var) {
                    SkASSERT(offset <= ByteCode::kPointerMax);
                    return ByteCode::Pointer{offset};
                }
                offset += SlotCount(p->fType);
            }
            SkASSERT(false);
            return ByteCode::Pointer{0};
        }
        case Variable::kGlobal_Storage: {
            bool isUniform = is_uniform(var);
            uint16_t offset = isUniform ? fOutput->getGlobalCount() : 0;
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
                            SkASSERT(offset <= ByteCode::kPointerMax);
                            return ByteCode::Pointer{offset};
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
            Location result = this->getLocation(*idx.fBase);
            if (idx.fIndex->isConstant()) {
                int64_t index = idx.fIndex->getConstantInt();
                return result.offset(*this, index * stride);
            } else {
                ByteCode::Register index = this->writeExpression(*idx.fIndex);
                ByteCode::Register imm = this->next();
                this->write(ByteCode::Instruction::kImmediate);
                this->write(imm);
                this->write(ByteCode::Immediate{stride});
                ByteCode::Register offset = this->next();
                this->write(ByteCode::Instruction::kMultiplyI);
                this->write(offset);
                this->write(index);
                this->write(imm);
                return result.offset(*this, offset);
            }
        }
        case Expression::kSwizzle_Kind: {
            const Swizzle& s = (const Swizzle&) expr;
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
    if (storage == Variable::kGlobal_Storage) {
        switch (location.fKind) {
            case Location::kPointer_Kind: return ByteCode::Instruction::kLoadDirect;
            case Location::kRegister_Kind: return ByteCode::Instruction::kLoad;
        }
    } else {
        switch (location.fKind) {
            case Location::kPointer_Kind: return ByteCode::Instruction::kLoadStackDirect;
            case Location::kRegister_Kind: return ByteCode::Instruction::kLoadStack;
        }
    }
}

ByteCode::Instruction ByteCodeGenerator::getStoreInstruction(ByteCodeGenerator::Location location,
                                                             Variable::Storage storage) {
    if (storage == Variable::kGlobal_Storage) {
        switch (location.fKind) {
            case Location::kPointer_Kind: return ByteCode::Instruction::kStoreDirect;
            case Location::kRegister_Kind: return ByteCode::Instruction::kStore;
        }
    } else {
        switch (location.fKind) {
            case Location::kPointer_Kind: return ByteCode::Instruction::kStoreStackDirect;
            case Location::kRegister_Kind: return ByteCode::Instruction::kStoreStack;
        }
    }
}

class ByteCodeSimpleLValue : public ByteCodeGenerator::LValue {
public:
    ByteCodeSimpleLValue(ByteCodeGenerator* generator, ByteCodeGenerator::Location location,
                         int count, ByteCode::Instruction load, ByteCode::Instruction store)
        : INHERITED(*generator)
        , fLocation(location)
        , fCount(count)
        , fLoad(load)
        , fStore(store) {}

    ByteCode::Register load() override {
        ByteCode::Register result{fGenerator.fNextRegister};
        for (int i = 0; i < fCount; ++i) {
            ByteCodeGenerator::Location final = fLocation.offset(fGenerator, i);
            fGenerator.write(fLoad);
            fGenerator.write(fGenerator.next());
            fGenerator.write(final);
        }
        return result;
    }

    void store(ByteCode::Register src) override {
        for (int i = 0; i < fCount; ++i) {
            ByteCodeGenerator::Location final = fLocation.offset(fGenerator, i);
            fGenerator.write(fStore);
            fGenerator.write(final);
            fGenerator.write(src + i);
        }
    }

private:
    ByteCodeGenerator::Location fLocation;

    int fCount;

    ByteCode::Instruction fLoad;

    ByteCode::Instruction fStore;

    typedef ByteCodeGenerator::LValue INHERITED;
};

class ByteCodeSwizzleLValue : public ByteCodeGenerator::LValue {
public:
    ByteCodeSwizzleLValue(ByteCodeGenerator* generator, const Swizzle* swizzle)
        : INHERITED(*generator)
        , fSwizzle(*swizzle) {}

    ByteCode::Register load() override {
        return fGenerator.writeSwizzle(fSwizzle);
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

std::unique_ptr<ByteCodeGenerator::LValue> ByteCodeGenerator::getLValue(const Expression& expr) {
    Location location = this->getLocation(expr);
    switch (expr.fKind) {
        case Expression::kFieldAccess_Kind:
        case Expression::kIndex_Kind:
        case Expression::kVariableReference_Kind: {
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
            printf("unsupported lvalue %s\n", expr.description().c_str());
            return nullptr;
    }
}

ByteCode::Register ByteCodeGenerator::next() {
    SkASSERT(fNextRegister + 1 > fNextRegister);
    return ByteCode::Register{fNextRegister++};
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

ByteCode::Register ByteCodeGenerator::writeBinaryInstruction(const Type& operandType,
                                                             ByteCode::Register left,
                                                             ByteCode::Register right,
                                                             ByteCode::Instruction s,
                                                             ByteCode::Instruction u,
                                                             ByteCode::Instruction f) {
    ByteCode::Register result{fNextRegister};
    for (int i = 0; i < SlotCount(operandType); ++i) {
        this->writeTypedInstruction(operandType, s, u, f);
        this->write(this->next());
        this->write(left + i);
        this->write(right + i);
    }
    return result;
}

ByteCode::Register ByteCodeGenerator::writeBinaryExpression(const BinaryExpression& b) {
    if (b.fOperator == Token::Kind::EQ) {
        std::unique_ptr<LValue> lvalue = this->getLValue(*b.fLeft);
        ByteCode::Register right = this->writeExpression(*b.fRight);
        lvalue->store(right);
        return right;
    }
    const Type& lType = b.fLeft->fType;
    const Type& rType = b.fRight->fType;
    bool lVecOrMtx = (lType.kind() == Type::kVector_Kind || lType.kind() == Type::kMatrix_Kind);
    bool rVecOrMtx = (rType.kind() == Type::kVector_Kind || rType.kind() == Type::kMatrix_Kind);
    Token::Kind op;
    std::unique_ptr<LValue> lvalue;
    ByteCode::Register left{0};
    if (is_assignment(b.fOperator)) {
        lvalue = this->getLValue(*b.fLeft);
        left = lvalue->load();
        op = remove_assignment(b.fOperator);
    } else {
        left = this->writeExpression(*b.fLeft);
        op = b.fOperator;
        if (!lVecOrMtx && rVecOrMtx) {
            for (int i = 0; i < SlotCount(rType) - 1; ++i) {
                this->write(ByteCode::Instruction::kCopy);
                this->write(this->next());
                this->write(left);
            }
        }
    }
    SkDEBUGCODE(TypeCategory tc = type_category(lType));
    int count = std::max(SlotCount(lType), SlotCount(rType));
    switch (op) {
        case Token::Kind::LOGICALAND: {
            ByteCode::Register result = left;
            this->write(ByteCode::Instruction::kMaskPush);
            this->write(left);
            this->write(ByteCode::Instruction::kBranchIfAllFalse);
            DeferredLocation falseLocation(this);
            ByteCode::Register right = this->writeExpression(*b.fRight);
            this->write(ByteCode::Instruction::kAnd);
            this->write(result);
            this->write(left);
            this->write(right);
            falseLocation.set();
            this->write(ByteCode::Instruction::kMaskPop);
            return result;
        }
        case Token::Kind::LOGICALOR: {
            ByteCode::Register result = left;
            ByteCode::Register mask = this->next();
            this->write(ByteCode::Instruction::kNot);
            this->write(mask);
            this->write(left);
            this->write(ByteCode::Instruction::kMaskPush);
            this->write(mask);
            this->write(ByteCode::Instruction::kBranchIfAllFalse);
            DeferredLocation falseLocation(this);
            ByteCode::Register right = this->writeExpression(*b.fRight);
            this->write(ByteCode::Instruction::kOr);
            this->write(result);
            this->write(left);
            this->write(right);
            falseLocation.set();
            this->write(ByteCode::Instruction::kMaskPop);
            return result;
        }
        case Token::Kind::SHL:
        case Token::Kind::SHR: {
            SkASSERT(count == 1 && (tc == SkSL::TypeCategory::kSigned ||
                                    tc == SkSL::TypeCategory::kUnsigned));
            if (!b.fRight->isConstant()) {
                fErrors.error(b.fRight->fOffset, "Shift amounts must be constant");
                return ByteCode::Register{0};
            }
            int64_t shift = b.fRight->getConstantInt();
            if (shift < 0 || shift > 31) {
                fErrors.error(b.fRight->fOffset, "Shift amount out of range");
                return ByteCode::Register{0};
            }

            if (op == Token::Kind::SHL) {
                this->write(ByteCode::Instruction::kShiftLeft);
            } else {
                this->write(type_category(lType) == TypeCategory::kSigned
                                ? ByteCode::Instruction::kShiftRightS
                                : ByteCode::Instruction::kShiftRightU);
            }
            ByteCode::Register result = this->next();
            this->write(result);
            this->write(left);
            this->write((uint8_t) shift);
            return result;
        }
        default:
            break;
    }
    ByteCode::Register right = this->writeExpression(*b.fRight);
    if (lVecOrMtx && !rVecOrMtx) {
        for (int i = 0; i < SlotCount(lType) - 1; ++i) {
            this->write(ByteCode::Instruction::kCopy);
            auto v = this->next();
            this->write(v);
            this->write(right);
        }
    }
    ByteCode::Register result;
    switch (op) {
        case Token::Kind::EQEQ:
            result = this->writeBinaryInstruction(lType, left, right,
                                                  ByteCode::Instruction::kCompareEQI,
                                                  ByteCode::Instruction::kCompareEQI,
                                                  ByteCode::Instruction::kCompareEQF);
            break;
        case Token::Kind::GT:
            result = this->writeBinaryInstruction(lType, left, right,
                                                  ByteCode::Instruction::kCompareGTS,
                                                  ByteCode::Instruction::kCompareGTU,
                                                  ByteCode::Instruction::kCompareGTF);
            break;
        case Token::Kind::GTEQ:
            result = this->writeBinaryInstruction(lType, left, right,
                                                  ByteCode::Instruction::kCompareGTEQS,
                                                  ByteCode::Instruction::kCompareGTEQU,
                                                  ByteCode::Instruction::kCompareGTEQF);
            break;
        case Token::Kind::LT:
            result = this->writeBinaryInstruction(lType, left, right,
                                                  ByteCode::Instruction::kCompareLTS,
                                                  ByteCode::Instruction::kCompareLTU,
                                                  ByteCode::Instruction::kCompareLTF);
            break;
        case Token::Kind::LTEQ:
            result = this->writeBinaryInstruction(lType, left, right,
                                                  ByteCode::Instruction::kCompareLTEQS,
                                                  ByteCode::Instruction::kCompareLTEQU,
                                                  ByteCode::Instruction::kCompareLTEQF);
            break;
        case Token::Kind::MINUS:
            result = this->writeBinaryInstruction(lType, left, right,
                                                  ByteCode::Instruction::kSubtractI,
                                                  ByteCode::Instruction::kSubtractI,
                                                  ByteCode::Instruction::kSubtractF);
            break;
        case Token::Kind::NEQ:
            result = this->writeBinaryInstruction(lType, left, right,
                                                  ByteCode::Instruction::kCompareNEQI,
                                                  ByteCode::Instruction::kCompareNEQI,
                                                  ByteCode::Instruction::kCompareNEQF);
            break;
        case Token::Kind::PERCENT:
            result = this->writeBinaryInstruction(lType, left, right,
                                                  ByteCode::Instruction::kRemainderS,
                                                  ByteCode::Instruction::kRemainderU,
                                                  ByteCode::Instruction::kRemainderF);
            break;
        case Token::Kind::PLUS:
            result = this->writeBinaryInstruction(lType, left, right,
                                                  ByteCode::Instruction::kAddI,
                                                  ByteCode::Instruction::kAddI,
                                                  ByteCode::Instruction::kAddF);
            break;
        case Token::Kind::SLASH:
            result = this->writeBinaryInstruction(lType, left, right,
                                                  ByteCode::Instruction::kDivideS,
                                                  ByteCode::Instruction::kDivideU,
                                                  ByteCode::Instruction::kDivideF);
            break;
        case Token::Kind::STAR:
            result = this->writeBinaryInstruction(lType, left, right,
                                                  ByteCode::Instruction::kMultiplyI,
                                                  ByteCode::Instruction::kMultiplyI,
                                                  ByteCode::Instruction::kMultiplyF);
            break;
        case Token::Kind::LOGICALAND: {
            SkASSERT(tc == SkSL::TypeCategory::kBool);
            this->write(ByteCode::Instruction::kAnd);
            result = this->next();
            this->write(result);
            this->write(left);
            this->write(right);
            break;
        }
        case Token::Kind::LOGICALOR: {
            SkASSERT(tc == SkSL::TypeCategory::kBool);
            this->write(ByteCode::Instruction::kOr);
            result = this->next();
            this->write(result);
            this->write(left);
            this->write(right);
            break;
        }
        case Token::Kind::LOGICALXOR: {
            SkASSERT(tc == SkSL::TypeCategory::kBool);
            this->write(ByteCode::Instruction::kXor);
            result = this->next();
            this->write(result);
            this->write(left);
            this->write(right);
            break;
        }
        case Token::Kind::BITWISEAND: {
            SkASSERT(tc == SkSL::TypeCategory::kSigned || tc == SkSL::TypeCategory::kUnsigned);
            this->write(ByteCode::Instruction::kAnd);
            result = this->next();
            this->write(result);
            this->write(left);
            this->write(right);
            break;
        }
        case Token::Kind::BITWISEOR: {
            SkASSERT(tc == SkSL::TypeCategory::kSigned || tc == SkSL::TypeCategory::kUnsigned);
            this->write(ByteCode::Instruction::kOr);
            result = this->next();
            this->write(result);
            this->write(left);
            this->write(right);
            break;
        }
        case Token::Kind::BITWISEXOR: {
            SkASSERT(tc == SkSL::TypeCategory::kSigned || tc == SkSL::TypeCategory::kUnsigned);
            this->write(ByteCode::Instruction::kXor);
            result = this->next();
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
    return result;
}

ByteCode::Register ByteCodeGenerator::writeConstructor(const Constructor& c) {
    if (c.fArguments.size() == 1 & c.fArguments[0]->fType.columns() == 1 && c.fType.columns() > 1) {
        ByteCode::Register v = this->writeExpression(*c.fArguments[0]);
        SkASSERT(v.fIndex == fNextRegister - 1);
        for (int i = 1; i < c.fType.columns(); ++i) {
            this->write(ByteCode::Instruction::kCopy);
            this->write(this->next());
            this->write(v);
        }
        return v;
    }
    ByteCode::Register values[4];
    for (size_t i = 0; i < c.fArguments.size(); ++i) {
        values[i] = this->writeExpression(*c.fArguments[i]);
    }
    ByteCode::Instruction inst;
    switch (type_category(c.fArguments[0]->fType)) {
        case TypeCategory::kSigned:
            if (type_category(c.fType) == TypeCategory::kFloat) {
                inst = ByteCode::Instruction::kSignedToFloat;
            } else {
                inst = ByteCode::Instruction::kCopy;
            }
            break;
        case TypeCategory::kUnsigned:
            if (type_category(c.fType) == TypeCategory::kFloat) {
                inst = ByteCode::Instruction::kUnsignedToFloat;
            } else {
                inst = ByteCode::Instruction::kCopy;
            }
            break;
        case TypeCategory::kFloat:
            if (type_category(c.fType) == TypeCategory::kSigned) {
                inst = ByteCode::Instruction::kFloatToSigned;
            } else if (type_category(c.fType) == TypeCategory::kUnsigned) {
                inst = ByteCode::Instruction::kFloatToUnsigned;
            } else {
                inst = ByteCode::Instruction::kCopy;
            }
            break;
        default:
            SkASSERT(false);
            return ByteCode::Register{0};
    }
    ByteCode::Register result = ByteCode::Register{fNextRegister};
    for (size_t i = 0; i < c.fArguments.size(); ++i) {
        int count = SlotCount(c.fArguments[i]->fType);
        for (int j = 0; j < count; ++j) {
            this->write(inst);
            this->write(this->next());
            this->write(values[i] + j);
        }
    }
    return result;
}

ByteCode::Register ByteCodeGenerator::writeIntrinsicCall(const FunctionCall& c,
                                                         std::vector<ByteCode::Register> args) {
    auto found = fIntrinsics.find(c.fFunction.fName);
    if (found == fIntrinsics.end()) printf("unsupported intrinsic %s\n", String(c.fFunction.fName).c_str());
    SkASSERT(found != fIntrinsics.end());
    ByteCode::Register result = this->next();
    if (found->second.fIsSpecial) {
        switch (found->second.fValue.fSpecial) {
            case SpecialIntrinsic::kDot: {
                SkASSERT(args.size() == 2);
                int count = SlotCount(c.fArguments[0]->fType);
                ByteCode::Register product{fNextRegister};
                for (int i = 0; i < count; ++i) {
                    this->writeTypedInstruction(c.fType,
                                                ByteCode::Instruction::kMultiplyI,
                                                ByteCode::Instruction::kMultiplyI,
                                                ByteCode::Instruction::kMultiplyF);
                    this->write(this->next());
                    this->write(args[0] + i);
                    this->write(args[1] + i);
                }
                ByteCode::Register total = product;
                for (int i = 1; i < count; ++i) {
                    this->writeTypedInstruction(c.fType,
                                                ByteCode::Instruction::kAddI,
                                                ByteCode::Instruction::kAddI,
                                                ByteCode::Instruction::kAddF);
                    ByteCode::Register sum = this->next();
                    this->write(sum);
                    this->write(total);
                    this->write(product + i);
                    total = sum;
                }
                return total;
            }
        }
    } else {
        this->write(found->second.fValue.fInstruction);
        this->write(result);
        for (ByteCode::Register arg : args) {
            this->write(arg);
        }
        return result;
    }
}

ByteCode::Register ByteCodeGenerator::writeFunctionCall(const FunctionCall& c) {
    std::vector<ByteCode::Register> args;
    int argCount = c.fArguments.size();
    std::vector<std::unique_ptr<LValue>> lvalues;
    for (int i = 0; i < argCount; ++i) {
        const auto& param = c.fFunction.fParameters[i];
        const auto& arg = c.fArguments[i];
        if (param->fModifiers.fFlags & Modifiers::kOut_Flag) {
            lvalues.emplace_back(this->getLValue(*arg));
            ByteCode::Register load = lvalues.back()->load();
            args.push_back(load);
        } else {
            args.push_back(this->writeExpression(*arg));
        }
    }
    if (c.fFunction.fBuiltin) {
        return this->writeIntrinsicCall(c, args);
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
        return ByteCode::Register{0};
    } else if (idx >= fFunctions.size()) {
        fErrors.error(c.fOffset, "Call to undefined function");
        return ByteCode::Register{0};
    }

    ByteCode::Register result;
    this->write(ByteCode::Instruction::kCall);
    this->write((uint8_t) idx);
    if (c.fType != *fContext.fVoid_Type) {
        result = this->next();
        this->write(result);
    } else {
        result.fIndex = -1;
    }
    for (ByteCode::Register arg : args) {
        this->write(arg);
    }
    for (int i = 0; i < argCount; ++i) {
        const auto& param = c.fFunction.fParameters[i];
        if (param->fModifiers.fFlags & Modifiers::kOut_Flag) {
            lvalues.back()->store(args[i]);
        }
    }
    return result;
}

ByteCode::Register ByteCodeGenerator::incOrDec(Token::Kind op, Expression& operand,
                                               bool prefix) {
    SkASSERT(op == Token::Kind::PLUSPLUS || op == Token::Kind::MINUSMINUS);
    std::unique_ptr<LValue> lvalue = this->getLValue(operand);
    ByteCode::Register value = lvalue->load();
    ByteCode::Register one = this->next();
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
    ByteCode::Register result = this->next();
    this->write(result);
    this->write(value);
    this->write(one);
    lvalue->store(result);
    return prefix ? result : value;
}

ByteCode::Register ByteCodeGenerator::writePostfixExpression(const PostfixExpression& p) {
    return this->incOrDec(p.fOperator, *p.fOperand, false);
}

ByteCode::Register ByteCodeGenerator::writePrefixExpression(const PrefixExpression& p) {
    switch (p.fOperator) {
        case Token::Kind::PLUSPLUS:
        case Token::Kind::MINUSMINUS: {
            return this->incOrDec(p.fOperator, *p.fOperand, true);
        }
        case Token::Kind::MINUS: {
            ByteCode::Register src = this->writeExpression(*p.fOperand);
            ByteCode::Register result{fNextRegister};
            for (int i = 0; i < SlotCount(p.fType); ++i) {
                this->writeTypedInstruction(p.fType,
                                            ByteCode::Instruction::kNegateS,
                                            ByteCode::Instruction::kNegateS,
                                            ByteCode::Instruction::kNegateF);
                this->write(this->next());
                this->write(src + i);
            }
            return result;
        }
        case Token::Kind::LOGICALNOT:
        case Token::Kind::BITWISENOT: {
            ByteCode::Register src = this->writeExpression(*p.fOperand);
            ByteCode::Register result{fNextRegister};
            for (int i = 0; i < SlotCount(p.fType); ++i) {
                this->write(ByteCode::Instruction::kNot);
                this->write(this->next());
                this->write(src + i);
            }
            return result;
        }
        default:
            SkASSERT(false);
            return ByteCode::Register{0};
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

ByteCode::Register ByteCodeGenerator::writeSwizzle(const Swizzle& s) {
    if (swizzle_is_simple(s)) {
        return this->writeVariableExpression(s);
    }
    ByteCode::Register base = this->writeExpression(*s.fBase);
    ByteCode::Register result{fNextRegister};
    for (int c : s.fComponents) {
        this->write(ByteCode::Instruction::kCopy);
        this->write(this->next());
        this->write(base + c);
    }
    return result;
}

ByteCode::Register ByteCodeGenerator::writeTernaryExpression(const TernaryExpression& t) {
    int count = SlotCount(t.fType);
    SkASSERT(count == SlotCount(t.fIfTrue->fType));
    SkASSERT(count == SlotCount(t.fIfFalse->fType));

    ByteCode::Register test = this->writeExpression(*t.fTest);
    this->write(ByteCode::Instruction::kMaskPush);
    this->write(test);
    ByteCode::Register ifTrue = this->writeExpression(*t.fIfTrue);
    this->write(ByteCode::Instruction::kMaskNegate);
    ByteCode::Register ifFalse = this->writeExpression(*t.fIfFalse);
    this->write(ByteCode::Instruction::kMaskPop);
    ByteCode::Register result{fNextRegister};
    for (int i = 0; i < count; ++i) {
        this->write(ByteCode::Instruction::kSelect);
        this->write(this->next());
        this->write(test);
        this->write(ifTrue + i);
        this->write(ifFalse + i);
    }
    return result;
}

ByteCode::Register ByteCodeGenerator::writeVariableExpression(const Expression& expr) {
    ByteCodeGenerator::Location location = this->getLocation(expr);
    int count = SlotCount(expr.fType);
    ByteCode::Register result{fNextRegister};
    fNextRegister += count;
    for (int i = 0; i < count; ++i) {
        ByteCodeGenerator::Location final = location.offset(*this, i);
        this->write(this->getLoadInstruction(location, this->getStorage(expr)));
        this->write(result + i);
        this->write(final);
    }
    return result;
}

ByteCode::Register ByteCodeGenerator::writeExpression(const Expression& expr) {
    switch (expr.fKind) {
        case Expression::kBoolLiteral_Kind: {
            ByteCode::Register result = this->next();
            this->write(ByteCode::Instruction::kImmediate);
            this->write(result);
            this->write(ByteCode::Immediate((int32_t) (((BoolLiteral&) expr).fValue ? -1 : 0)));
            return result;
        }
        case Expression::kBinary_Kind: {
            return this->writeBinaryExpression((BinaryExpression&) expr);
        }
        case Expression::kConstructor_Kind: {
            return this->writeConstructor((Constructor&) expr);
        }
        case Expression::kFloatLiteral_Kind: {
            ByteCode::Register result = this->next();
            this->write(ByteCode::Instruction::kImmediate);
            this->write(result);
            this->write(ByteCode::Immediate((float) ((FloatLiteral&) expr).fValue));
            return result;
        }
        case Expression::kFunctionCall_Kind: {
            return this->writeFunctionCall((FunctionCall&) expr);
        }
        case Expression::kIntLiteral_Kind: {
            ByteCode::Register result = this->next();
            this->write(ByteCode::Instruction::kImmediate);
            this->write(result);
            this->write(ByteCode::Immediate((int32_t) ((IntLiteral&) expr).fValue));
            return result;
        }
        case Expression::kPostfix_Kind:
            return this->writePostfixExpression((PostfixExpression&) expr);
        case Expression::kPrefix_Kind:
            return this->writePrefixExpression((PrefixExpression&) expr);
        case Expression::kSwizzle_Kind:
            return this->writeSwizzle((Swizzle&) expr);
        case Expression::kTernary_Kind:
            return this->writeTernaryExpression((TernaryExpression&) expr);
        case Expression::kFieldAccess_Kind:
        case Expression::kIndex_Kind:
        case Expression::kVariableReference_Kind:
            return this->writeVariableExpression(expr);
        default:
            printf("unsupported expression: %s(%d)\n", expr.description().c_str(), expr.fKind);
            SkASSERT(false);
            return ByteCode::Register{0};
    }
}

void ByteCodeGenerator::writeBlock(const Block& b) {
    for (const auto& s : b.fStatements) {
        this->writeStatement(*s);
    }
}

void ByteCodeGenerator::writeDoStatement(const DoStatement& d) {
    this->write(ByteCode::Instruction::kLoopBegin);
    SkASSERT(fCode->size() < ByteCode::kPointerMax);
    ByteCode::Pointer start{(uint16_t) fCode->size()};
    this->writeStatement(*d.fStatement);
    ByteCode::Register test = this->writeExpression(*d.fTest);
    this->write(ByteCode::Instruction::kLoopMask);
    this->write(test);
    this->write(ByteCode::Instruction::kLoopNext);
    this->write(ByteCode::Instruction::kBranchIfAllFalse);
    DeferredLocation endLocation(this);
    this->write(ByteCode::Instruction::kBranch);
    this->write(start);
    endLocation.set();
    this->write(ByteCode::Instruction::kLoopEnd);
}

void ByteCodeGenerator::writeForStatement(const ForStatement& f) {
    if (f.fInitializer) {
        this->writeStatement(*f.fInitializer);
    }
    this->write(ByteCode::Instruction::kLoopBegin);
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
    this->write(ByteCode::Instruction::kLoopEnd);
}

void ByteCodeGenerator::writeIfStatement(const IfStatement& i) {
    ByteCode::Register test = this->writeExpression(*i.fTest);
    this->write(ByteCode::Instruction::kMaskPush);
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
    this->write(ByteCode::Instruction::kMaskPop);
}

void ByteCodeGenerator::writeReturn(const ReturnStatement& r) {
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
        ByteCodeGenerator::Location location = getLocation(*decl.fVar);
        if (decl.fValue) {
            ByteCode::Register src = this->writeExpression(*decl.fValue);
            for (int i = 0; i < SlotCount(decl.fVar->fType); ++i) {
                ByteCodeGenerator::Location final = location.offset(*this, i);
                this->write(ByteCode::Instruction::kStoreStackDirect);
                this->write(final);
                this->write(src + i);
            }
        }
    }
}

void ByteCodeGenerator::writeWhileStatement(const WhileStatement& w) {
    this->write(ByteCode::Instruction::kLoopBegin);
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
            printf("unsupported statement: %s\n", s.description().c_str());
            SkASSERT(false);
    }
}

void ByteCodeGenerator::writeFunction(const FunctionDefinition& f) {
    fFunction = &f;
    std::unique_ptr<ByteCodeFunction> result(new ByteCodeFunction(&f.fDeclaration));
    result->fReturnSlotCount = SlotCount(f.fDeclaration.fReturnType);
    fParameterCount = 0;
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
    this->write(ByteCode::Instruction::kReturn);
    fOutput->fFunctions.push_back(std::move(result));
}

bool ByteCodeGenerator::generateCode() {
    fOutput->fGlobalCount = 0;
    fOutput->fUniformCount = 0;
    for (const auto& pe : fProgram) {
        if (pe.fKind == ProgramElement::kVar_Kind) {
            VarDeclarations& decl = (VarDeclarations&) pe;
            for (const auto& v : decl.fVars) {
                const Variable* declVar = ((VarDeclaration&) *v).fVar;
                if (declVar->fModifiers.fLayout.fBuiltin >= 0) {
                    continue;
                }
                if (is_uniform(*declVar)) {
                    fOutput->fUniformCount += SlotCount(declVar->fType);
                } else {
                    fOutput->fGlobalCount += SlotCount(declVar->fType);
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
    return true;
}

} // namespace
