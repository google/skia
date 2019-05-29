/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLByteCodeGenerator.h"
#include "src/sksl/SkSLInterpreter.h"

namespace SkSL {

ByteCodeGenerator::ByteCodeGenerator(const Context* context, const Program* program, ErrorReporter* errors,
                  ByteCode* output)
    : INHERITED(program, errors, nullptr)
    , fContext(*context)
    , fOutput(output)
    , fIntrinsics {
         { "cos",   ByteCodeInstruction::kCos },
         { "cross", ByteCodeInstruction::kCross },
         { "dot",   SpecialIntrinsic::kDot },
         { "sin",   ByteCodeInstruction::kSin },
         { "sqrt",  ByteCodeInstruction::kSqrt },
         { "tan",   ByteCodeInstruction::kTan },
         { "mix",   ByteCodeInstruction::kMix },
      } {}


int ByteCodeGenerator::SlotCount(const Type& type) {
    if (type.kind() == Type::kStruct_Kind) {
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
                        for (int i = SlotCount(declVar->fType); i > 0; --i) {
                            fOutput->fInputSlots.push_back(fOutput->fGlobalCount++);
                        }
                    } else {
                        fOutput->fGlobalCount += SlotCount(declVar->fType);
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
        fParameterCount += SlotCount(p->fType);
    }
    fCode = &result->fCode;
    this->writeStatement(*f.fBody);
    this->write(ByteCodeInstruction::kReturn);
    this->write8(0);
    result->fParameterCount = fParameterCount;
    result->fLocalCount = fLocals.size();
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

int ByteCodeGenerator::getLocation(const Variable& var) {
    // given that we seldom have more than a couple of variables, linear search is probably the most
    // efficient way to handle lookups
    switch (var.fStorage) {
        case Variable::kLocal_Storage: {
            for (int i = fLocals.size() - 1; i >= 0; --i) {
                if (fLocals[i] == &var) {
                    SkASSERT(fParameterCount + i <= 255);
                    return fParameterCount + i;
                }
            }
            int result = fParameterCount + fLocals.size();
            fLocals.push_back(&var);
            for (int i = 0; i < SlotCount(var.fType) - 1; ++i) {
                fLocals.push_back(nullptr);
            }
            SkASSERT(result <= 255);
            return result;
        }
        case Variable::kParameter_Storage: {
            int offset = 0;
            for (const auto& p : fFunction->fDeclaration.fParameters) {
                if (p == &var) {
                    SkASSERT(offset <= 255);
                    return offset;
                }
                offset += SlotCount(p->fType);
            }
            SkASSERT(false);
            return 0;
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
                        offset += SlotCount(declVar->fType);
                    }
                }
            }
            SkASSERT(false);
            return 0;
        }
        default:
            SkASSERT(false);
            return 0;
    }
}

// TODO: Elide Add 0 and Mul 1 sequences
int ByteCodeGenerator::getLocation(const Expression& expr, Variable::Storage* storage) {
    switch (expr.fKind) {
        case Expression::kFieldAccess_Kind: {
            const FieldAccess& f = (const FieldAccess&)expr;
            int baseAddr = this->getLocation(*f.fBase, storage);
            int offset = 0;
            for (int i = 0; i < f.fFieldIndex; ++i) {
                offset += SlotCount(*f.fBase->fType.fields()[i].fType);
            }
            if (baseAddr < 0) {
                this->write(ByteCodeInstruction::kPushImmediate);
                this->write32(offset);
                this->write(ByteCodeInstruction::kAddI);
                return -1;
            } else {
                return baseAddr + offset;
            }
        }
        case Expression::kIndex_Kind: {
            const IndexExpression& i = (const IndexExpression&)expr;
            int stride = SlotCount(i.fType);
            int offset = -1;
            if (i.fIndex->isConstant()) {
                offset = i.fIndex->getConstantInt() * stride;
            } else {
                this->writeExpression(*i.fIndex);
                this->write(ByteCodeInstruction::kPushImmediate);
                this->write32(stride);
                this->write(ByteCodeInstruction::kMultiplyI);
            }
            int baseAddr = this->getLocation(*i.fBase, storage);
            if (baseAddr >= 0 && offset >= 0) {
                return baseAddr + offset;
            }
            if (baseAddr >= 0) {
                this->write(ByteCodeInstruction::kPushImmediate);
                this->write32(baseAddr);
            }
            if (offset >= 0) {
                this->write(ByteCodeInstruction::kPushImmediate);
                this->write32(offset);
            }
            this->write(ByteCodeInstruction::kAddI);
            return -1;
        }
        case Expression::kSwizzle_Kind: {
            const Swizzle& s = (const Swizzle&)expr;
            SkASSERT(swizzle_is_simple(s));
            int baseAddr = this->getLocation(*s.fBase, storage);
            int offset = s.fComponents[0];
            if (baseAddr < 0) {
                this->write(ByteCodeInstruction::kPushImmediate);
                this->write32(offset);
                this->write(ByteCodeInstruction::kAddI);
                return -1;
            } else {
                return baseAddr + offset;
            }
        }
        case Expression::kVariableReference_Kind: {
            const Variable& var = ((const VariableReference&)expr).fVariable;
            *storage = var.fStorage;
            return this->getLocation(var);
        }
        default:
            SkASSERT(false);
            return 0;
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

void ByteCodeGenerator::write(ByteCodeInstruction i) {
    this->write16((uint16_t)i);
}

static ByteCodeInstruction vector_instruction(ByteCodeInstruction base, int count) {
    SkASSERT(count >= 1 && count <= 4);
    return ((ByteCodeInstruction) ((int) base + count - 1));
}

void ByteCodeGenerator::writeTypedInstruction(const Type& type, ByteCodeInstruction s,
                                              ByteCodeInstruction u, ByteCodeInstruction f,
                                              int count) {
    switch (type_category(type)) {
        case TypeCategory::kSigned:
            this->write(vector_instruction(s, count));
            break;
        case TypeCategory::kUnsigned:
            this->write(vector_instruction(u, count));
            break;
        case TypeCategory::kFloat:
            this->write(vector_instruction(f, count));
            break;
        default:
            SkASSERT(false);
    }
}

void ByteCodeGenerator::writeBinaryExpression(const BinaryExpression& b) {
    if (b.fOperator == Token::Kind::EQ) {
        std::unique_ptr<LValue> lvalue = this->getLValue(*b.fLeft);
        this->writeExpression(*b.fRight);
        lvalue->store();
        return;
    }
    const Type& lType = b.fLeft->fType;
    const Type& rType = b.fRight->fType;
    Token::Kind op;
    std::unique_ptr<LValue> lvalue;
    if (is_assignment(b.fOperator)) {
        lvalue = this->getLValue(*b.fLeft);
        lvalue->load();
        op = remove_assignment(b.fOperator);
    } else {
        this->writeExpression(*b.fLeft);
        op = b.fOperator;
        if (lType.kind() == Type::kScalar_Kind &&
            (rType.kind() == Type::kVector_Kind || rType.kind() == Type::kMatrix_Kind)) {
            for (int i = SlotCount(rType); i > 1; --i) {
                this->write(ByteCodeInstruction::kDup);
            }
        }
    }
    this->writeExpression(*b.fRight);
    if ((lType.kind() == Type::kVector_Kind || lType.kind() == Type::kMatrix_Kind) &&
        rType.kind() == Type::kScalar_Kind) {
        for (int i = SlotCount(lType); i > 1; --i) {
            this->write(ByteCodeInstruction::kDup);
        }
    }
    int count = std::max(SlotCount(lType), SlotCount(rType));
    switch (op) {
        case Token::Kind::EQEQ:
            this->writeTypedInstruction(b.fLeft->fType, ByteCodeInstruction::kCompareIEQ,
                                        ByteCodeInstruction::kCompareIEQ,
                                        ByteCodeInstruction::kCompareFEQ,
                                        count);
            // Collapse to a single bool
            for (int i = count; i > 1; --i) {
                this->write(ByteCodeInstruction::kAndB);
            }
            break;
        case Token::Kind::GT:
            this->writeTypedInstruction(b.fLeft->fType, ByteCodeInstruction::kCompareSGT,
                                        ByteCodeInstruction::kCompareUGT,
                                        ByteCodeInstruction::kCompareFGT,
                                        count);
            break;
        case Token::Kind::GTEQ:
            this->writeTypedInstruction(b.fLeft->fType, ByteCodeInstruction::kCompareSGTEQ,
                                        ByteCodeInstruction::kCompareUGTEQ,
                                        ByteCodeInstruction::kCompareFGTEQ,
                                        count);
            break;
        case Token::Kind::LT:
            this->writeTypedInstruction(b.fLeft->fType, ByteCodeInstruction::kCompareSLT,
                                        ByteCodeInstruction::kCompareULT,
                                        ByteCodeInstruction::kCompareFLT,
                                        count);
            break;
        case Token::Kind::LTEQ:
            this->writeTypedInstruction(b.fLeft->fType, ByteCodeInstruction::kCompareSLTEQ,
                                        ByteCodeInstruction::kCompareULTEQ,
                                        ByteCodeInstruction::kCompareFLTEQ,
                                        count);
            break;
        case Token::Kind::MINUS:
            this->writeTypedInstruction(b.fLeft->fType, ByteCodeInstruction::kSubtractI,
                                        ByteCodeInstruction::kSubtractI,
                                        ByteCodeInstruction::kSubtractF,
                                        count);
            break;
        case Token::Kind::NEQ:
            this->writeTypedInstruction(b.fLeft->fType, ByteCodeInstruction::kCompareINEQ,
                                        ByteCodeInstruction::kCompareINEQ,
                                        ByteCodeInstruction::kCompareFNEQ,
                                        count);
            // Collapse to a single bool
            for (int i = count; i > 1; --i) {
                this->write(ByteCodeInstruction::kOrB);
            }
            break;
        case Token::Kind::PERCENT:
            this->writeTypedInstruction(b.fLeft->fType, ByteCodeInstruction::kRemainderS,
                                        ByteCodeInstruction::kRemainderU,
                                        ByteCodeInstruction::kRemainderF,
                                        count);
            break;
        case Token::Kind::PLUS:
            this->writeTypedInstruction(b.fLeft->fType, ByteCodeInstruction::kAddI,
                                        ByteCodeInstruction::kAddI,
                                        ByteCodeInstruction::kAddF,
                                        count);
            break;
        case Token::Kind::SLASH:
            this->writeTypedInstruction(b.fLeft->fType, ByteCodeInstruction::kDivideS,
                                        ByteCodeInstruction::kDivideU,
                                        ByteCodeInstruction::kDivideF,
                                        count);
            break;
        case Token::Kind::STAR:
            this->writeTypedInstruction(b.fLeft->fType, ByteCodeInstruction::kMultiplyI,
                                        ByteCodeInstruction::kMultiplyI,
                                        ByteCodeInstruction::kMultiplyF,
                                        count);
            break;
        default:
            SkASSERT(false);
    }
    if (lvalue) {
        lvalue->store();
    }
}

void ByteCodeGenerator::writeBoolLiteral(const BoolLiteral& b) {
    this->write(ByteCodeInstruction::kPushImmediate);
    this->write32(b.fValue ? 1 : 0);
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
            this->write(ByteCodeInstruction::kMatrixToMatrix);
            this->write8(inType.columns());
            this->write8(inType.rows());
            this->write8(outType.columns());
            this->write8(outType.rows());
        } else if (inCount != outCount) {
            SkASSERT(inCount == 1);
            if (outType.kind() == Type::kMatrix_Kind) {
                this->write(ByteCodeInstruction::kScalarToMatrix);
                this->write8(outType.columns());
                this->write8(outType.rows());
            } else {
                SkASSERT(outType.kind() == Type::kVector_Kind);
                for (; inCount != outCount; ++inCount) {
                    this->write(ByteCodeInstruction::kDup);
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
    this->write(ByteCodeInstruction::kCallExternal);
    SkASSERT(argumentCount <= 255);
    this->write8(argumentCount);
    this->write8(SlotCount(f.fType));
    int index = fOutput->fExternalValues.size();
    fOutput->fExternalValues.push_back(f.fFunction);
    SkASSERT(index <= 255);
    this->write8(index);
}

void ByteCodeGenerator::writeExternalValue(const ExternalValueReference& e) {
    this->write(vector_instruction(ByteCodeInstruction::kReadExternal,
                                   SlotCount(e.fValue->type())));
    int index = fOutput->fExternalValues.size();
    fOutput->fExternalValues.push_back(e.fValue);
    SkASSERT(index <= 255);
    this->write8(index);
}

void ByteCodeGenerator::writeVariableExpression(const Expression& expr) {
    Variable::Storage storage;
    int location = this->getLocation(expr, &storage);
    bool isGlobal = storage == Variable::kGlobal_Storage;
    int count = SlotCount(expr.fType);
    if (location < 0 || count > 4) {
        if (location >= 0) {
            this->write(ByteCodeInstruction::kPushImmediate);
            this->write32(location);
        }
        this->write(isGlobal ? ByteCodeInstruction::kLoadExtendedGlobal
                             : ByteCodeInstruction::kLoadExtended);
        this->write8(count);
    } else {
        this->write(vector_instruction(isGlobal ? ByteCodeInstruction::kLoadGlobal
                                                : ByteCodeInstruction::kLoad,
                                       count));
        this->write8(location);
    }
}

void ByteCodeGenerator::writeFloatLiteral(const FloatLiteral& f) {
    this->write(ByteCodeInstruction::kPushImmediate);
    this->write32(Interpreter::Value((float) f.fValue).fUnsigned);
}

void ByteCodeGenerator::writeIntrinsicCall(const FunctionCall& c) {
    auto found = fIntrinsics.find(c.fFunction.fName);
    if (found == fIntrinsics.end()) {
        fErrors.error(c.fOffset, "unsupported intrinsic function");
        return;
    }
    if (found->second.fIsSpecial) {
        SkASSERT(found->second.fValue.fSpecial == SpecialIntrinsic::kDot);
        SkASSERT(c.fArguments.size() == 2);
        SkASSERT(SlotCount(c.fArguments[0]->fType) == SlotCount(c.fArguments[1]->fType));
        this->write((ByteCodeInstruction) ((int) ByteCodeInstruction::kMultiplyF +
                    SlotCount(c.fArguments[0]->fType) - 1));
        for (int i = SlotCount(c.fArguments[0]->fType); i > 1; --i) {
            this->write(ByteCodeInstruction::kAddF);
        }
    } else {
        switch (found->second.fValue.fInstruction) {
            case ByteCodeInstruction::kCos:
            case ByteCodeInstruction::kMix:
            case ByteCodeInstruction::kSin:
            case ByteCodeInstruction::kSqrt:
            case ByteCodeInstruction::kTan:
                SkASSERT(c.fArguments.size() > 0);
                this->write((ByteCodeInstruction) ((int) found->second.fValue.fInstruction +
                            SlotCount(c.fArguments[0]->fType) - 1));
                break;
            case ByteCodeInstruction::kCross:
                this->write(found->second.fValue.fInstruction);
                break;
            default:
                SkASSERT(false);
        }
    }
}

void ByteCodeGenerator::writeFunctionCall(const FunctionCall& f) {
    for (const auto& arg : f.fArguments) {
        this->writeExpression(*arg);
    }
    if (f.fFunction.fBuiltin) {
        this->writeIntrinsicCall(f);
        return;
    }
    this->write(ByteCodeInstruction::kCall);
    fCallTargets.emplace_back(this, f.fFunction);
}

void ByteCodeGenerator::writeIntLiteral(const IntLiteral& i) {
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
            SkASSERT(SlotCount(p.fOperand->fType) == 1);
            std::unique_ptr<LValue> lvalue = this->getLValue(*p.fOperand);
            lvalue->load();
            this->write(ByteCodeInstruction::kPushImmediate);
            this->write32(type_category(p.fType) == TypeCategory::kFloat
                            ? Interpreter::Value(1.0f).fUnsigned : 1);
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
            lvalue->store();
            break;
        }
        case Token::Kind::MINUS: {
            this->writeExpression(*p.fOperand);
            this->writeTypedInstruction(p.fType,
                                        ByteCodeInstruction::kNegateI,
                                        ByteCodeInstruction::kNegateI,
                                        ByteCodeInstruction::kNegateF,
                                        SlotCount(p.fOperand->fType));
            break;
        }
        default:
            SkASSERT(false);
    }
}

void ByteCodeGenerator::writePostfixExpression(const PostfixExpression& p) {
    switch (p.fOperator) {
        case Token::Kind::PLUSPLUS: // fall through
        case Token::Kind::MINUSMINUS: {
            SkASSERT(SlotCount(p.fOperand->fType) == 1);
            std::unique_ptr<LValue> lvalue = this->getLValue(*p.fOperand);
            lvalue->load();
            this->write(ByteCodeInstruction::kDup);
            this->write(ByteCodeInstruction::kPushImmediate);
            this->write32(type_category(p.fType) == TypeCategory::kFloat
                            ? Interpreter::Value(1.0f).fUnsigned : 1);
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
            lvalue->store();
            this->write(ByteCodeInstruction::kPop);
            break;
        }
        default:
            SkASSERT(false);
    }
}

void ByteCodeGenerator::writeSwizzle(const Swizzle& s) {
    if (swizzle_is_simple(s)) {
        this->writeVariableExpression(s);
        return;
    }

    switch (s.fBase->fKind) {
        case Expression::kVariableReference_Kind: {
            const Variable& var = ((VariableReference&) *s.fBase).fVariable;
            this->write(var.fStorage == Variable::kGlobal_Storage
                            ? ByteCodeInstruction::kLoadSwizzleGlobal
                            : ByteCodeInstruction::kLoadSwizzle);
            this->write8(this->getLocation(var));
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

void ByteCodeGenerator::writeTernaryExpression(const TernaryExpression& t) {
    this->writeExpression(*t.fTest);
    this->write(ByteCodeInstruction::kConditionalBranch);
    DeferredLocation trueLocation(this);
    this->writeExpression(*t.fIfFalse);
    this->write(ByteCodeInstruction::kBranch);
    DeferredLocation endLocation(this);
    trueLocation.set();
    this->writeExpression(*t.fIfTrue);
    endLocation.set();
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
            this->writePrefixExpression((PrefixExpression&) e);
            break;
        case Expression::kPostfix_Kind:
            this->writePostfixExpression((PostfixExpression&) e);
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
}

class ByteCodeExternalValueLValue : public ByteCodeGenerator::LValue {
public:
    ByteCodeExternalValueLValue(ByteCodeGenerator* generator, ExternalValue& value, int index)
        : INHERITED(*generator)
        , fCount(ByteCodeGenerator::SlotCount(value.type()))
        , fIndex(index) {}

    void load() override {
        fGenerator.write(vector_instruction(ByteCodeInstruction::kReadExternal, fCount));
        fGenerator.write8(fIndex);
    }

    void store() override {
        fGenerator.write(vector_instruction(ByteCodeInstruction::kDup, fCount));
        fGenerator.write(vector_instruction(ByteCodeInstruction::kWriteExternal, fCount));
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

    void store() override {
        fGenerator.write(vector_instruction(ByteCodeInstruction::kDup,
                                            fSwizzle.fComponents.size()));
        Variable::Storage storage;
        int location = fGenerator.getLocation(*fSwizzle.fBase, &storage);
        bool isGlobal = storage == Variable::kGlobal_Storage;
        if (location < 0) {
            fGenerator.write(isGlobal ? ByteCodeInstruction::kStoreSwizzleIndirectGlobal
                                      : ByteCodeInstruction::kStoreSwizzleIndirect);
        } else {
            fGenerator.write(isGlobal ? ByteCodeInstruction::kStoreSwizzleGlobal
                                      : ByteCodeInstruction::kStoreSwizzle);
            fGenerator.write8(location);
        }
        fGenerator.write8(fSwizzle.fComponents.size());
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

    void store() override {
        int count = ByteCodeGenerator::SlotCount(fExpression.fType);
        if (count > 4) {
            fGenerator.write(ByteCodeInstruction::kDupN);
            fGenerator.write8(count);
        } else {
            fGenerator.write(vector_instruction(ByteCodeInstruction::kDup, count));
        }
        Variable::Storage storage;
        int location = fGenerator.getLocation(fExpression, &storage);
        bool isGlobal = storage == Variable::kGlobal_Storage;
        if (location < 0 || count > 4) {
            if (location >= 0) {
                fGenerator.write(ByteCodeInstruction::kPushImmediate);
                fGenerator.write32(location);
            }
            fGenerator.write(isGlobal ? ByteCodeInstruction::kStoreExtendedGlobal
                                      : ByteCodeInstruction::kStoreExtended);
            fGenerator.write8(count);
        } else {
            fGenerator.write(vector_instruction(isGlobal ? ByteCodeInstruction::kStoreGlobal
                                                         : ByteCodeInstruction::kStore,
                                                count));
            fGenerator.write8(location);
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
    this->write(ByteCodeInstruction::kBranch);
    fBreakTargets.top().emplace_back(this);
}

void ByteCodeGenerator::writeContinueStatement(const ContinueStatement& c) {
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
        this->write(ByteCodeInstruction::kConditionalBranch);
        DeferredLocation endLocation(this);
        this->writeStatement(*f.fStatement);
        this->setContinueTargets();
        if (f.fNext) {
            this->writeExpression(*f.fNext);
            this->write(vector_instruction(ByteCodeInstruction::kPop, SlotCount(f.fNext->fType)));
        }
        this->write(ByteCodeInstruction::kBranch);
        this->write16(start);
        endLocation.set();
    } else {
        this->writeStatement(*f.fStatement);
        this->setContinueTargets();
        if (f.fNext) {
            this->writeExpression(*f.fNext);
            this->write(vector_instruction(ByteCodeInstruction::kPop, SlotCount(f.fNext->fType)));
        }
        this->write(ByteCodeInstruction::kBranch);
        this->write16(start);
    }
    this->setBreakTargets();
}

void ByteCodeGenerator::writeIfStatement(const IfStatement& i) {
    if (i.fIfFalse) {
        // if (test) { ..ifTrue.. } else { .. ifFalse .. }
        this->writeExpression(*i.fTest);
        this->write(ByteCodeInstruction::kConditionalBranch);
        DeferredLocation trueLocation(this);
        this->writeStatement(*i.fIfFalse);
        this->write(ByteCodeInstruction::kBranch);
        DeferredLocation endLocation(this);
        trueLocation.set();
        this->writeStatement(*i.fIfTrue);
        endLocation.set();
    } else {
        // if (test) { ..ifTrue.. }
        this->writeExpression(*i.fTest);
        this->write(ByteCodeInstruction::kNot);
        this->write(ByteCodeInstruction::kConditionalBranch);
        DeferredLocation endLocation(this);
        this->writeStatement(*i.fIfTrue);
        endLocation.set();
    }
}

void ByteCodeGenerator::writeReturnStatement(const ReturnStatement& r) {
    this->writeExpression(*r.fExpression);
    this->write(ByteCodeInstruction::kReturn);
    this->write8(SlotCount(r.fExpression->fType));
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
            this->writeExpression(*decl.fValue);
            int count = SlotCount(decl.fValue->fType);
            if (count > 4) {
                this->write(ByteCodeInstruction::kPushImmediate);
                this->write32(location);
                this->write(ByteCodeInstruction::kStoreExtended);
                this->write8(count);
            } else {
                this->write(vector_instruction(ByteCodeInstruction::kStore, count));
                this->write8(location);
            }
        }
    }
}

void ByteCodeGenerator::writeWhileStatement(const WhileStatement& w) {
    fContinueTargets.emplace();
    fBreakTargets.emplace();
    size_t start = fCode->size();
    this->writeExpression(*w.fTest);
    this->write(ByteCodeInstruction::kNot);
    this->write(ByteCodeInstruction::kConditionalBranch);
    DeferredLocation endLocation(this);
    this->writeStatement(*w.fStatement);
    this->setContinueTargets();
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
            int count = SlotCount(expr.fType);
            if (count > 4) {
                this->write(ByteCodeInstruction::kPopN);
                this->write8(count);
            } else {
                this->write(vector_instruction(ByteCodeInstruction::kPop, count));
            }
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
