/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLByteCodeGenerator.h"

namespace SkSL {

static int slot_count(const Type& type) {
    return type.columns() * type.rows();
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
                break;
            }
            case ProgramElement::kVar_Kind: {
                VarDeclarations& decl = (VarDeclarations&) e;
                for (const auto& v : decl.fVars) {
                    const Variable* declVar = ((VarDeclaration&) *v).fVar;
                    if (declVar->fModifiers.fLayout.fBuiltin >= 0) {
                        continue;
                    }
                    if (declVar->fModifiers.fFlags & Modifiers::kIn_Flag) {
                        for (int i = slot_count(declVar->fType); i > 0; --i) {
                            fOutput->fInputSlots.push_back(fOutput->fGlobalCount++);
                        }
                    } else {
                        fOutput->fGlobalCount += slot_count(declVar->fType);
                    }
                }
                break;
            }
            default:
                ; // ignore
        }
    }
    for (auto& call : fCallTargets) {
        if (!call.set()) {
            return false;
        }
    }
    return true;
}

std::unique_ptr<ByteCodeFunction> ByteCodeGenerator::writeFunction(const FunctionDefinition& f) {
    fFunction = &f;
    std::unique_ptr<ByteCodeFunction> result(new ByteCodeFunction(&f.fDeclaration));
    fParameterCount = 0;
    for (const auto& p : f.fDeclaration.fParameters) {
        fParameterCount += p->fType.columns() * p->fType.rows();
    }
    fCode = &result->fCode;
    this->writeStatement(*f.fBody);
    this->write(ByteCodeInstruction::kReturn);
    this->write8(0);
    result->fParameterCount = fParameterCount;
    result->fLocalCount = fLocals.size();
    const Type& returnType = f.fDeclaration.fReturnType;
    if (returnType != *fContext.fVoid_Type) {
        result->fReturnCount = returnType.columns() * returnType.rows();
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

int ByteCodeGenerator::getLocation(const Variable& var) {
    // given that we seldom have more than a couple of variables, linear search is probably the most
    // efficient way to handle lookups
    switch (var.fStorage) {
        case Variable::kLocal_Storage: {
            for (int i = fLocals.size() - 1; i >= 0; --i) {
                if (fLocals[i] == &var) {
                    return fParameterCount + i;
                }
            }
            int result = fParameterCount + fLocals.size();
            fLocals.push_back(&var);
            for (int i = 0; i < slot_count(var.fType) - 1; ++i) {
                fLocals.push_back(nullptr);
            }
            return result;
        }
        case Variable::kParameter_Storage: {
            int offset = 0;
            for (const auto& p : fFunction->fDeclaration.fParameters) {
                if (p == &var) {
                    return offset;
                }
                offset += slot_count(p->fType);
            }
            SkASSERT(false);
            return -1;
        }
        case Variable::kGlobal_Storage: {
            int offset = 0;
            for (const auto& e : fProgram) {
                if (e.fKind == ProgramElement::kVar_Kind) {
                    VarDeclarations& decl = (VarDeclarations&) e;
                    for (const auto& v : decl.fVars) {
                        const Variable* declVar = ((VarDeclaration&) *v).fVar;
                        if (declVar->fModifiers.fLayout.fBuiltin >= 0) {
                            continue;
                        }
                        if (declVar == &var) {
                            SkASSERT(offset <= 255);
                            return offset;
                        }
                        offset += slot_count(declVar->fType);
                    }
                }
            }
            SkASSERT(false);
            return -1;
        }
        default:
            SkASSERT(false);
            return 0;
    }
}

void ByteCodeGenerator::align(int divisor, int remainder) {
    switch (remainder - (int) fCode->size() % divisor) {
        case 0: return;
        case 3: this->write(ByteCodeInstruction::kNop3); // fall through
        case 2: this->write(ByteCodeInstruction::kNop2); // fall through
        case 1: this->write(ByteCodeInstruction::kNop1);
                break;
        default: SkASSERT(false);
    }
}

void ByteCodeGenerator::write8(uint8_t b) {
    fCode->push_back(b);
}

void ByteCodeGenerator::write16(uint16_t i) {
    SkASSERT(fCode->size() % 2 == 0);
    this->write8(i >> 0);
    this->write8(i >> 8);
}

void ByteCodeGenerator::write32(uint32_t i) {
    SkASSERT(fCode->size() % 4 == 0);
    this->write8((i >>  0) & 0xFF);
    this->write8((i >>  8) & 0xFF);
    this->write8((i >> 16) & 0xFF);
    this->write8((i >> 24) & 0xFF);
}

void ByteCodeGenerator::write(ByteCodeInstruction i) {
    this->write8((uint8_t) i);
}

void ByteCodeGenerator::writeTypedInstruction(const Type& type, ByteCodeInstruction s,
                                              ByteCodeInstruction u, ByteCodeInstruction f) {
    switch (type_category(type)) {
        case TypeCategory::kSigned:
            this->write(s);
            break;
        case TypeCategory::kUnsigned:
            this->write(u);
            break;
        case TypeCategory::kFloat:
            this->write(f);
            break;
        default:
            SkASSERT(false);
    }
}

void ByteCodeGenerator::writeBinaryExpression(const BinaryExpression& b) {
    if (b.fOperator == Token::Kind::EQ) {
        std::unique_ptr<LValue> lvalue = this->getLValue(*b.fLeft);
        this->writeExpression(*b.fRight);
        this->write(ByteCodeInstruction::kDupDown);
        this->write8(slot_count(b.fRight->fType));
        lvalue->store();
        return;
    }
    Token::Kind op;
    std::unique_ptr<LValue> lvalue;
    if (is_assignment(b.fOperator)) {
        lvalue = this->getLValue(*b.fLeft);
        lvalue->load();
        op = remove_assignment(b.fOperator);
    } else {
        this->writeExpression(*b.fLeft);
        op = b.fOperator;
        if (b.fLeft->fType.kind() == Type::kScalar_Kind &&
            b.fRight->fType.kind() == Type::kVector_Kind) {
            for (int i = b.fRight->fType.columns(); i > 1; --i) {
                this->write(ByteCodeInstruction::kDup);
            }
        }
    }
    this->writeExpression(*b.fRight);
    if (b.fLeft->fType.kind() == Type::kVector_Kind &&
        b.fRight->fType.kind() == Type::kScalar_Kind) {
        for (int i = b.fLeft->fType.columns(); i > 1; --i) {
            this->write(ByteCodeInstruction::kDup);
        }
    }
    int count = slot_count(b.fType);
    if (count > 1) {
        this->write(ByteCodeInstruction::kVector);
        this->write8(count);
    }
    switch (op) {
        case Token::Kind::EQEQ:
            this->writeTypedInstruction(b.fLeft->fType, ByteCodeInstruction::kCompareIEQ,
                                        ByteCodeInstruction::kCompareIEQ,
                                        ByteCodeInstruction::kCompareFEQ);
            break;
        case Token::Kind::GT:
            this->writeTypedInstruction(b.fLeft->fType, ByteCodeInstruction::kCompareSGT,
                                        ByteCodeInstruction::kCompareUGT,
                                        ByteCodeInstruction::kCompareFGT);
            break;
        case Token::Kind::GTEQ:
            this->writeTypedInstruction(b.fLeft->fType, ByteCodeInstruction::kCompareSGTEQ,
                                        ByteCodeInstruction::kCompareUGTEQ,
                                        ByteCodeInstruction::kCompareFGTEQ);
            break;
        case Token::Kind::LT:
            this->writeTypedInstruction(b.fLeft->fType, ByteCodeInstruction::kCompareSLT,
                                        ByteCodeInstruction::kCompareULT,
                                        ByteCodeInstruction::kCompareFLT);
            break;
        case Token::Kind::LTEQ:
            this->writeTypedInstruction(b.fLeft->fType, ByteCodeInstruction::kCompareSLTEQ,
                                        ByteCodeInstruction::kCompareULTEQ,
                                        ByteCodeInstruction::kCompareFLTEQ);
            break;
        case Token::Kind::MINUS:
            this->writeTypedInstruction(b.fLeft->fType, ByteCodeInstruction::kSubtractI,
                                        ByteCodeInstruction::kSubtractI,
                                        ByteCodeInstruction::kSubtractF);
            break;
        case Token::Kind::NEQ:
            this->writeTypedInstruction(b.fLeft->fType, ByteCodeInstruction::kCompareINEQ,
                                        ByteCodeInstruction::kCompareINEQ,
                                        ByteCodeInstruction::kCompareFNEQ);
            break;
        case Token::Kind::PERCENT:
            this->writeTypedInstruction(b.fLeft->fType, ByteCodeInstruction::kRemainderS,
                                        ByteCodeInstruction::kRemainderU,
                                        ByteCodeInstruction::kRemainderF);
            break;
        case Token::Kind::PLUS:
            this->writeTypedInstruction(b.fLeft->fType, ByteCodeInstruction::kAddI,
                                        ByteCodeInstruction::kAddI,
                                        ByteCodeInstruction::kAddF);
            break;
        case Token::Kind::SLASH:
            this->writeTypedInstruction(b.fLeft->fType, ByteCodeInstruction::kDivideS,
                                        ByteCodeInstruction::kDivideU,
                                        ByteCodeInstruction::kDivideF);
            break;
        case Token::Kind::STAR:
            this->writeTypedInstruction(b.fLeft->fType, ByteCodeInstruction::kMultiplyS,
                                        ByteCodeInstruction::kMultiplyU,
                                        ByteCodeInstruction::kMultiplyF);
            break;
        default:
            SkASSERT(false);
    }
    if (lvalue) {
        this->write(ByteCodeInstruction::kDupDown);
        this->write8(slot_count(b.fType));
        lvalue->store();
    }
}

void ByteCodeGenerator::writeBoolLiteral(const BoolLiteral& b) {
    this->align(4, 3);
    this->write(ByteCodeInstruction::kPushImmediate);
    this->write32(b.fValue ? 1 : 0);
}

void ByteCodeGenerator::writeConstructor(const Constructor& c) {
    if (c.fArguments.size() == 1 &&
        type_category(c.fType) == type_category(c.fArguments[0]->fType)) {
        // cast from float to half or similar no-op
        this->writeExpression(*c.fArguments[0]);
        return;
    }
    for (const auto& arg : c.fArguments) {
        this->writeExpression(*arg);
    }
    if (c.fArguments.size() == 1) {
        TypeCategory inCategory = type_category(c.fArguments[0]->fType);
        TypeCategory outCategory = type_category(c.fType);
        if (inCategory != outCategory) {
            int count = c.fType.columns();
            if (count > 1) {
                this->write(ByteCodeInstruction::kVector);
                this->write8(count);
            }
            if (inCategory == TypeCategory::kFloat) {
                SkASSERT(outCategory == TypeCategory::kSigned ||
                         outCategory == TypeCategory::kUnsigned);
                this->write(ByteCodeInstruction::kFloatToInt);
            } else if (outCategory == TypeCategory::kFloat) {
                if (inCategory == TypeCategory::kSigned) {
                    this->write(ByteCodeInstruction::kSignedToFloat);
                } else {
                    SkASSERT(inCategory == TypeCategory::kUnsigned);
                    this->write(ByteCodeInstruction::kUnsignedToFloat);
                }
            } else {
                SkASSERT(false);
            }
        }
    }
}

void ByteCodeGenerator::writeFieldAccess(const FieldAccess& f) {
    // not yet implemented
    abort();
}

void ByteCodeGenerator::writeFloatLiteral(const FloatLiteral& f) {
    this->align(4, 3);
    this->write(ByteCodeInstruction::kPushImmediate);
    union { float f; uint32_t u; } pun = { (float) f.fValue };
    this->write32(pun.u);
}

void ByteCodeGenerator::writeFunctionCall(const FunctionCall& f) {
    for (const auto& arg : f.fArguments) {
        this->writeExpression(*arg);
    }
    this->write(ByteCodeInstruction::kCall);
    fCallTargets.emplace_back(this, f.fFunction);
}

void ByteCodeGenerator::writeIndexExpression(const IndexExpression& i) {
    // not yet implemented
    abort();
}

void ByteCodeGenerator::writeIntLiteral(const IntLiteral& i) {
    this->align(4, 3);
    this->write(ByteCodeInstruction::kPushImmediate);
    this->write32(i.fValue);
}

void ByteCodeGenerator::writeNullLiteral(const NullLiteral& n) {
    // not yet implemented
    abort();
}

void ByteCodeGenerator::writePrefixExpression(const PrefixExpression& p) {
    switch (p.fOperator) {
        case Token::Kind::PLUSPLUS: // fall through
        case Token::Kind::MINUSMINUS: {
            std::unique_ptr<LValue> lvalue = this->getLValue(*p.fOperand);
            lvalue->load();
            this->align(4, 3);
            this->write(ByteCodeInstruction::kPushImmediate);
            this->write32(1);
            if (p.fOperator == Token::Kind::PLUSPLUS) {
                this->writeTypedInstruction(p.fType,
                                            ByteCodeInstruction::kAddI,
                                            ByteCodeInstruction::kAddI,
                                            ByteCodeInstruction::kAddF);
            } else {
                this->writeTypedInstruction(p.fType,
                                            ByteCodeInstruction::kSubtractI,
                                            ByteCodeInstruction::kSubtractI,
                                            ByteCodeInstruction::kSubtractF);
            }
            this->write(ByteCodeInstruction::kDupDown);
            this->write8(slot_count(p.fType));
            lvalue->store();
            break;
        }
        case Token::Kind::MINUS: {
            this->writeExpression(*p.fOperand);
            int count = slot_count(p.fOperand->fType);
            if (count > 1) {
                this->write(ByteCodeInstruction::kVector);
                this->write8(count);
            }
            this->writeTypedInstruction(p.fType,
                                        ByteCodeInstruction::kNegateS,
                                        ByteCodeInstruction::kInvalid,
                                        ByteCodeInstruction::kNegateF);
            break;
        }
        default:
            SkASSERT(false);
    }
}

void ByteCodeGenerator::writePostfixExpression(const PostfixExpression& p) {
    // not yet implemented
    abort();
}

void ByteCodeGenerator::writeSwizzle(const Swizzle& s) {
    switch (s.fBase->fKind) {
        case Expression::kVariableReference_Kind: {
            const Variable& var = ((VariableReference&) *s.fBase).fVariable;
            if (var.fStorage == Variable::kGlobal_Storage) {
                this->write(ByteCodeInstruction::kLoadSwizzleGlobal);
                this->write8(this->getLocation(var));
            } else {
                this->align(4, 3);
                this->write(ByteCodeInstruction::kPushImmediate);
                this->write32(this->getLocation(var));
                this->write(ByteCodeInstruction::kLoadSwizzle);
            }
            this->write8(s.fComponents.size());
            for (int c : s.fComponents) {
                this->write8(c);
            }
            break;
        }
        default:
            this->writeExpression(*s.fBase);
            this->write(ByteCodeInstruction::kSwizzle);
            this->write8(s.fBase->fType.columns());
            this->write8(s.fComponents.size());
            for (int c : s.fComponents) {
                this->write8(c);
            }
    }
}

void ByteCodeGenerator::writeVariableReference(const VariableReference& v) {
    if (v.fVariable.fStorage == Variable::kGlobal_Storage) {
        int count = slot_count(v.fType);
        if (count > 1) {
            this->write(ByteCodeInstruction::kVector);
            this->write8(count);
        }
        this->write(ByteCodeInstruction::kLoadGlobal);
        this->write8(this->getLocation(v.fVariable));
    } else {
        this->align(4, 3);
        this->write(ByteCodeInstruction::kPushImmediate);
        this->write32(this->getLocation(v.fVariable));
        int count = slot_count(v.fType);
        if (count > 1) {
            this->write(ByteCodeInstruction::kVector);
            this->write8(count);
        }
        this->write(ByteCodeInstruction::kLoad);
    }
}

void ByteCodeGenerator::writeTernaryExpression(const TernaryExpression& t) {
    // not yet implemented
    abort();
}

void ByteCodeGenerator::writeExpression(const Expression& e) {
    switch (e.fKind) {
        case Expression::kBinary_Kind:
            this->writeBinaryExpression((BinaryExpression&) e);
            break;
        case Expression::kBoolLiteral_Kind:
            this->writeBoolLiteral((BoolLiteral&) e);
            break;
        case Expression::kConstructor_Kind:
            this->writeConstructor((Constructor&) e);
            break;
        case Expression::kFieldAccess_Kind:
            this->writeFieldAccess((FieldAccess&) e);
            break;
        case Expression::kFloatLiteral_Kind:
            this->writeFloatLiteral((FloatLiteral&) e);
            break;
        case Expression::kFunctionCall_Kind:
            this->writeFunctionCall((FunctionCall&) e);
            break;
        case Expression::kIndex_Kind:
            this->writeIndexExpression((IndexExpression&) e);
            break;
        case Expression::kIntLiteral_Kind:
            this->writeIntLiteral((IntLiteral&) e);
            break;
        case Expression::kNullLiteral_Kind:
            this->writeNullLiteral((NullLiteral&) e);
            break;
        case Expression::kPrefix_Kind:
            this->writePrefixExpression((PrefixExpression&) e);
            break;
        case Expression::kPostfix_Kind:
            this->writePostfixExpression((PostfixExpression&) e);
            break;
        case Expression::kSwizzle_Kind:
            this->writeSwizzle((Swizzle&) e);
            break;
        case Expression::kVariableReference_Kind:
            this->writeVariableReference((VariableReference&) e);
            break;
        case Expression::kTernary_Kind:
            this->writeTernaryExpression((TernaryExpression&) e);
            break;
        default:
            printf("unsupported expression %s\n", e.description().c_str());
            SkASSERT(false);
    }
}

void ByteCodeGenerator::writeTarget(const Expression& e) {
    switch (e.fKind) {
        case Expression::kVariableReference_Kind:
            this->align(4, 3);
            this->write(ByteCodeInstruction::kPushImmediate);
            this->write32(this->getLocation(((VariableReference&) e).fVariable));
            break;
        case Expression::kIndex_Kind:
        case Expression::kTernary_Kind:
        default:
            printf("unsupported target %s\n", e.description().c_str());
            SkASSERT(false);
    }
}

class ByteCodeSwizzleLValue : public ByteCodeGenerator::LValue {
public:
    ByteCodeSwizzleLValue(ByteCodeGenerator* generator, const Swizzle& swizzle)
        : INHERITED(*generator)
        , fSwizzle(swizzle) {
        fGenerator.writeTarget(*swizzle.fBase);
    }

    void load() override {
        fGenerator.write(ByteCodeInstruction::kDup);
        fGenerator.write(ByteCodeInstruction::kLoadSwizzle);
        fGenerator.write8(fSwizzle.fComponents.size());
        for (int c : fSwizzle.fComponents) {
            fGenerator.write8(c);
        }
    }

    void store() override {
        fGenerator.write(ByteCodeInstruction::kStoreSwizzle);
        fGenerator.write8(fSwizzle.fComponents.size());
        for (int c : fSwizzle.fComponents) {
            fGenerator.write8(c);
        }
    }

private:
    const Swizzle& fSwizzle;

    typedef LValue INHERITED;
};

class ByteCodeVariableLValue : public ByteCodeGenerator::LValue {
public:
    ByteCodeVariableLValue(ByteCodeGenerator* generator, const Variable& var)
        : INHERITED(*generator)
        , fCount(slot_count(var.fType))
        , fIsGlobal(var.fStorage == Variable::kGlobal_Storage) {
        fGenerator.align(4, 3);
        fGenerator.write(ByteCodeInstruction::kPushImmediate);
        fGenerator.write32(generator->getLocation(var));
    }

    void load() override {
        fGenerator.write(ByteCodeInstruction::kDup);
        if (fCount > 1) {
            fGenerator.write(ByteCodeInstruction::kVector);
            fGenerator.write8(fCount);
        }
        fGenerator.write(fIsGlobal ? ByteCodeInstruction::kLoadGlobal : ByteCodeInstruction::kLoad);
    }

    void store() override {
        if (fCount > 1) {
            fGenerator.write(ByteCodeInstruction::kVector);
            fGenerator.write8(fCount);
        }
        fGenerator.write(fIsGlobal ? ByteCodeInstruction::kStoreGlobal
                                  : ByteCodeInstruction::kStore);
    }

private:
    typedef LValue INHERITED;

    int fCount;

    bool fIsGlobal;
};

std::unique_ptr<ByteCodeGenerator::LValue> ByteCodeGenerator::getLValue(const Expression& e) {
    switch (e.fKind) {
        case Expression::kIndex_Kind:
            // not yet implemented
            abort();
        case Expression::kVariableReference_Kind:
            return std::unique_ptr<LValue>(new ByteCodeVariableLValue(this,
                                                               ((VariableReference&) e).fVariable));
        case Expression::kSwizzle_Kind:
            return std::unique_ptr<LValue>(new ByteCodeSwizzleLValue(this, (Swizzle&) e));
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
    this->align(2, 1);
    this->write(ByteCodeInstruction::kBranch);
    fBreakTargets.top().emplace_back(this);
}

void ByteCodeGenerator::writeContinueStatement(const ContinueStatement& c) {
    this->align(2, 1);
    this->write(ByteCodeInstruction::kBranch);
    fContinueTargets.top().emplace_back(this);
}

void ByteCodeGenerator::writeDoStatement(const DoStatement& d) {
    fContinueTargets.emplace();
    fBreakTargets.emplace();
    size_t start = fCode->size();
    this->writeStatement(*d.fStatement);
    this->setContinueTargets();
    this->writeExpression(*d.fTest);
    this->align(2, 1);
    this->write(ByteCodeInstruction::kConditionalBranch);
    this->write16(start);
    this->setBreakTargets();
}

void ByteCodeGenerator::writeForStatement(const ForStatement& f) {
    fContinueTargets.emplace();
    fBreakTargets.emplace();
    if (f.fInitializer) {
        this->writeStatement(*f.fInitializer);
    }
    size_t start = fCode->size();
    if (f.fTest) {
        this->writeExpression(*f.fTest);
        this->write(ByteCodeInstruction::kNot);
        this->align(2, 1);
        this->write(ByteCodeInstruction::kConditionalBranch);
        DeferredLocation endLocation(this);
        this->writeStatement(*f.fStatement);
        this->setContinueTargets();
        if (f.fNext) {
            this->writeExpression(*f.fNext);
            this->write(ByteCodeInstruction::kPop);
            this->write8(slot_count(f.fNext->fType));
        }
        this->align(2, 1);
        this->write(ByteCodeInstruction::kBranch);
        this->write16(start);
        endLocation.set();
    } else {
        this->writeStatement(*f.fStatement);
        this->setContinueTargets();
        if (f.fNext) {
            this->writeExpression(*f.fNext);
            this->write(ByteCodeInstruction::kPop);
            this->write8(slot_count(f.fNext->fType));
        }
        this->align(2, 1);
        this->write(ByteCodeInstruction::kBranch);
        this->write16(start);
    }
    this->setBreakTargets();
}

void ByteCodeGenerator::writeIfStatement(const IfStatement& i) {
    this->writeExpression(*i.fTest);
    this->write(ByteCodeInstruction::kNot);
    this->align(2, 1);
    this->write(ByteCodeInstruction::kConditionalBranch);
    DeferredLocation elseLocation(this);
    this->writeStatement(*i.fIfTrue);
    this->align(2, 1);
    this->write(ByteCodeInstruction::kBranch);
    DeferredLocation endLocation(this);
    elseLocation.set();
    if (i.fIfFalse) {
        this->writeStatement(*i.fIfFalse);
    }
    endLocation.set();
}

void ByteCodeGenerator::writeReturnStatement(const ReturnStatement& r) {
    this->writeExpression(*r.fExpression);
    this->write(ByteCodeInstruction::kReturn);
    this->write8(r.fExpression->fType.columns() * r.fExpression->fType.rows());
}

void ByteCodeGenerator::writeSwitchStatement(const SwitchStatement& r) {
    // not yet implemented
    abort();
}

void ByteCodeGenerator::writeVarDeclarations(const VarDeclarations& v) {
    for (const auto& declStatement : v.fVars) {
        const VarDeclaration& decl = (VarDeclaration&) *declStatement;
        // we need to grab the location even if we don't use it, to ensure it
        // has been allocated
        int location = getLocation(*decl.fVar);
        if (decl.fValue) {
            this->align(4, 3);
            this->write(ByteCodeInstruction::kPushImmediate);
            this->write32(location);
            this->writeExpression(*decl.fValue);
            int count = slot_count(decl.fValue->fType);
            if (count > 1) {
                this->write(ByteCodeInstruction::kVector);
                this->write8(count);
            }
            this->write(ByteCodeInstruction::kStore);
        }
    }
}

void ByteCodeGenerator::writeWhileStatement(const WhileStatement& w) {
    fContinueTargets.emplace();
    fBreakTargets.emplace();
    size_t start = fCode->size();
    this->writeExpression(*w.fTest);
    this->write(ByteCodeInstruction::kNot);
    this->align(2, 1);
    this->write(ByteCodeInstruction::kConditionalBranch);
    DeferredLocation endLocation(this);
    this->writeStatement(*w.fStatement);
    this->setContinueTargets();
    this->align(2, 1);
    this->write(ByteCodeInstruction::kBranch);
    this->write16(start);
    endLocation.set();
    this->setBreakTargets();
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
        case Statement::kExpression_Kind: {
            const Expression& expr = *((ExpressionStatement&) s).fExpression;
            this->writeExpression(expr);
            this->write(ByteCodeInstruction::kPop);
            this->write8(slot_count(expr.fType));
            break;
        }
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

}
