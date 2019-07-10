/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLByteCodeGenerator.h"

#include <algorithm>

namespace SkSL {

ByteCodeGenerator::ByteCodeGenerator(const Context* context, const Program* program,
                                     ErrorReporter* errors, ByteCode* output)
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
    if (type.kind() == Type::kOther_Kind) {
        return 0;
    } else if (type.kind() == Type::kStruct_Kind) {
        int slots = 0;
        for (const auto& f : type.fields()) {
            slots += SlotCount(f.fType.typeNode());
        }
        SkASSERT(slots <= 255);
        return slots;
    } else if (type.kind() == Type::kArray_Kind) {
        int columns = type.columns();
        SkASSERT(columns >= 0);
        int slots = columns * SlotCount(type.componentType().typeNode());
        SkASSERT(slots <= 255);
        return slots;
    } else {
        return type.columns() * type.rows();
    }
}

bool ByteCodeGenerator::generateCode() {
    for (IRNode::ID eID : fProgram) {
        ProgramElement& e = (ProgramElement&) eID.node();
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
                    Variable& declVar = (Variable&) ((VarDeclaration&) v.node()).fVar.node();
                    if (declVar.fModifiers.fLayout.fBuiltin >= 0) {
                        continue;
                    }
                    if (declVar.fModifiers.fFlags & Modifiers::kIn_Flag) {
                        for (int i = SlotCount(declVar.fType.typeNode()); i > 0; --i) {
                            fOutput->fInputSlots.push_back(fOutput->fGlobalCount++);
                        }
                    } else {
                        fOutput->fGlobalCount += SlotCount(declVar.fType.typeNode());
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
    std::unique_ptr<ByteCodeFunction> result(new ByteCodeFunction(
                                                    (FunctionDeclaration*) &f.fDeclaration.node()));
    fParameterCount = result->fParameterCount;
    fCode = &result->fCode;
    this->writeStatement((Statement&) f.fBody.node());
    this->write(ByteCodeInstruction::kReturn);
    this->write8(0);
    result->fLocalCount = fLocals.size();
    IRNode::ID returnType = ((FunctionDeclaration&) f.fDeclaration.node()).fReturnType;
    if (returnType != fContext.fVoid_Type) {
        result->fReturnCount = SlotCount(returnType.typeNode());
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
            return type_category(type.componentType().typeNode());
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
static bool swizzle_is_simple(const Program& program, const Swizzle& s) {
    switch (((Expression&) s.fBase.node()).fKind) {
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
            for (int i = 0; i < SlotCount(var.fType.typeNode()) - 1; ++i) {
                fLocals.push_back(nullptr);
            }
            SkASSERT(result <= 255);
            return result;
        }
        case Variable::kParameter_Storage: {
            int offset = 0;
            for (IRNode::ID p :
                 ((FunctionDeclaration&) fFunction->fDeclaration.node()).fParameters) {
                if (&p.node() == &var) {
                    SkASSERT(offset <= 255);
                    return offset;
                }
                offset += SlotCount(((Variable&) p.node()).fType.typeNode());
            }
            SkASSERT(false);
            return 0;
        }
        case Variable::kGlobal_Storage: {
            int offset = 0;
            for (IRNode::ID e : fProgram) {
                if (((ProgramElement&) e.node()).fKind == ProgramElement::kVar_Kind) {
                    VarDeclarations& decl = (VarDeclarations&) e;
                    for (IRNode::ID v : decl.fVars) {
                        Variable& declVar = (Variable&) ((VarDeclaration&) v.node()).fVar.node();
                        if (declVar.fModifiers.fLayout.fBuiltin >= 0) {
                            continue;
                        }
                        if (declVar == var) {
                            SkASSERT(offset <= 255);
                            return offset;
                        }
                        offset += SlotCount(declVar.fType.typeNode());
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

int ByteCodeGenerator::getLocation(const Expression& expr, Variable::Storage* storage) {
    switch (expr.fKind) {
        case Expression::kFieldAccess_Kind: {
            const FieldAccess& f = (const FieldAccess&)expr;
            const Expression& base = (Expression&) f.fBase.node();
            int baseAddr = this->getLocation(base, storage);
            int offset = 0;
            for (int i = 0; i < f.fFieldIndex; ++i) {
                offset += SlotCount(base.fType.typeNode().fields()[i].fType.typeNode());
            }
            if (baseAddr < 0) {
                if (offset != 0) {
                    this->write(ByteCodeInstruction::kPushImmediate);
                    this->write32(offset);
                    this->write(ByteCodeInstruction::kAddI);
                }
                return -1;
            } else {
                return baseAddr + offset;
            }
        }
        case Expression::kIndex_Kind: {
            const IndexExpression& i = (const IndexExpression&)expr;
            int stride = SlotCount(i.fType.typeNode());
            int offset = -1;
            if (i.fIndex.node().isConstant()) {
                offset = ((Expression&) i.fIndex.node()).getConstantInt() * stride;
            } else {
                if (i.fIndex.expressionNode().hasSideEffects()) {
                    // Having a side-effect in an indexer is technically safe for an rvalue,
                    // but with lvalues we have to evaluate the indexer twice, so make it an error.
                    fErrors.error(i.fIndex.node().fOffset,
                            "Index expressions with side-effects not supported in byte code.");
                    return 0;
                }
                this->writeExpression(i.fIndex.expressionNode());
                if (stride != 1) {
                    this->write(ByteCodeInstruction::kPushImmediate);
                    this->write32(stride);
                    this->write(ByteCodeInstruction::kMultiplyI);
                }
            }
            int baseAddr = this->getLocation(i.fBase.expressionNode(), storage);

            // Are both components known statically?
            if (baseAddr >= 0 && offset >= 0) {
                return baseAddr + offset;
            }

            // At least one component is dynamic (and on the stack).

            // If the other component is zero, we're done
            if (baseAddr == 0 || offset == 0) {
                return -1;
            }

            // Push the non-dynamic component (if any) to the stack, then add the two
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
            SkASSERT(swizzle_is_simple(fProgram, s));
            int baseAddr = this->getLocation((Expression&) s.fBase.node(), storage);
            int offset = s.fComponents[0];
            if (baseAddr < 0) {
                if (offset != 0) {
                    this->write(ByteCodeInstruction::kPushImmediate);
                    this->write32(offset);
                    this->write(ByteCodeInstruction::kAddI);
                }
                return -1;
            } else {
                return baseAddr + offset;
            }
        }
        case Expression::kVariableReference_Kind: {
            const Variable& var = (Variable&) ((VariableReference&) expr).fVariable.node();
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
        case TypeCategory::kFloat: {
            if (count > 4) {
                this->write((ByteCodeInstruction)((int)f + 4));
                this->write8(count);
            } else {
                this->write(vector_instruction(f, count));
            }
            break;
        }
        default:
            SkASSERT(false);
    }
}

bool ByteCodeGenerator::writeBinaryExpression(const BinaryExpression& b, bool discard) {
    if (b.fOperator == Token::Kind::EQ) {
        std::unique_ptr<LValue> lvalue = this->getLValue((Expression&) b.fLeft.node());
        this->writeExpression((Expression&) b.fRight.node());
        lvalue->store(discard);
        discard = false;
        return discard;
    }
    IRNode::ID lTypeID = ((Expression&) b.fLeft.node()).fType;
    IRNode::ID rTypeID = ((Expression&) b.fRight.node()).fType;
    const Type& lType = lTypeID.typeNode();
    const Type& rType = rTypeID.typeNode();
    bool lVecOrMtx = (lType.kind() == Type::kVector_Kind || lType.kind() == Type::kMatrix_Kind);
    bool rVecOrMtx = (rType.kind() == Type::kVector_Kind || rType.kind() == Type::kMatrix_Kind);
    Token::Kind op;
    std::unique_ptr<LValue> lvalue;
    if (is_assignment(b.fOperator)) {
        lvalue = this->getLValue((Expression&) b.fLeft.node());
        lvalue->load();
        op = remove_assignment(b.fOperator);
    } else {
        this->writeExpression((Expression&) b.fLeft.node());
        op = b.fOperator;
        if (!lVecOrMtx && rVecOrMtx) {
            for (int i = SlotCount(rTypeID.typeNode()); i > 1; --i) {
                this->write(ByteCodeInstruction::kDup);
            }
        }
    }
    if (lVecOrMtx && !rVecOrMtx) {
        for (int i = SlotCount(lTypeID.typeNode()); i > 1; --i) {
            this->write(ByteCodeInstruction::kDup);
        }
    }
    // Special case for M*V, V*M, M*M (but not V*V!)
    if (op == Token::Kind::STAR && lVecOrMtx && rVecOrMtx &&
        !(lType.kind() == Type::kVector_Kind && rType.kind() == Type::kVector_Kind)) {
        this->write(ByteCodeInstruction::kMatrixMultiply);
        int rCols = rType.columns(),
            rRows = rType.rows(),
            lCols = lType.columns(),
            lRows = lType.rows();
        // M*V treats the vector as a column
        if (rType.kind() == Type::kVector_Kind) {
            std::swap(rCols, rRows);
        }
        SkASSERT(lCols == rRows);
        SkASSERT(SlotCount(b.fType.typeNode()) == lRows * rCols);
        this->write8(lCols);
        this->write8(lRows);
        this->write8(rCols);
    } else {
        int count = std::max(SlotCount(lTypeID.typeNode()), SlotCount(rTypeID.typeNode()));
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

            case Token::Kind::LOGICALAND:
                SkASSERT(type_category(lType) == SkSL::TypeCategory::kBool && count == 1);
                this->write(ByteCodeInstruction::kAndB);
                break;
            case Token::Kind::LOGICALNOT:
                SkASSERT(type_category(lType) == SkSL::TypeCategory::kBool && count == 1);
                this->write(ByteCodeInstruction::kNotB);
                break;
            case Token::Kind::LOGICALOR:
                SkASSERT(type_category(lType) == SkSL::TypeCategory::kBool && count == 1);
                this->write(ByteCodeInstruction::kOrB);
                break;
            case Token::Kind::LOGICALXOR:
                SkASSERT(type_category(lType) == SkSL::TypeCategory::kBool && count == 1);
                this->write(ByteCodeInstruction::kXorB);
                break;

            default:
                SkASSERT(false);
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
        this->writeExpression((Expression&) arg.node());
    }
    if (c.fArguments.size() == 1) {
        const Type& inType = ((Expression&) c.fArguments[0].node()).fType.typeNode();
        const Type& outType = c.fType.typeNode();
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
    for (auto argID : f.fArguments) {
        Expression& arg = (Expression&) argID.node();
        this->writeExpression(arg);
        argumentCount += SlotCount(arg.fType.typeNode());
    }
    this->write(ByteCodeInstruction::kCallExternal);
    SkASSERT(argumentCount <= 255);
    this->write8(argumentCount);
    this->write8(SlotCount(f.fType.typeNode()));
    int index = fOutput->fExternalValues.size();
    fOutput->fExternalValues.push_back(&f.fFunction);
    SkASSERT(index <= 255);
    this->write8(index);
}

void ByteCodeGenerator::writeExternalValue(const ExternalValueReference& e) {
    this->write(vector_instruction(ByteCodeInstruction::kReadExternal,
                                   SlotCount(e.fValue.type().typeNode())));
    int index = fOutput->fExternalValues.size();
    fOutput->fExternalValues.push_back(&e.fValue);
    SkASSERT(index <= 255);
    this->write8(index);
}

void ByteCodeGenerator::writeVariableExpression(const Expression& expr) {
    Variable::Storage storage;
    int location = this->getLocation(expr, &storage);
    bool isGlobal = storage == Variable::kGlobal_Storage;
    int count = SlotCount(expr.fType.typeNode());
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
    auto found = fIntrinsics.find(((FunctionDeclaration&) c.fFunction.node()).fName);
    if (found == fIntrinsics.end()) {
        fErrors.error(c.fOffset, "unsupported intrinsic function");
        return;
    }
    if (found->second.fIsSpecial) {
        SkASSERT(found->second.fValue.fSpecial == SpecialIntrinsic::kDot);
        SkASSERT(c.fArguments.size() == 2);
        Expression& arg0 = (Expression&) c.fArguments[0].node();
        Expression& arg1 = (Expression&) c.fArguments[1].node();
        SkASSERT(SlotCount(arg0.fType.typeNode()) == SlotCount(arg1.fType.typeNode()));
        this->write((ByteCodeInstruction) ((int) ByteCodeInstruction::kMultiplyF +
                    SlotCount(arg0.fType.typeNode()) - 1));
        for (int i = SlotCount(arg0.fType.typeNode()); i > 1; --i) {
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
                            SlotCount(c.fArguments[0].expressionNode().fType.typeNode()) - 1));
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
    FunctionDeclaration& fn = (FunctionDeclaration&) f.fFunction.node();
    // Builtins have simple signatures...
    if (fn.fBuiltin) {
        for (IRNode::ID arg : f.fArguments) {
            this->writeExpression(arg.expressionNode());
        }
        this->writeIntrinsicCall(f);
        return;
    }

    // Otherwise, we may need to deal with out parameters, so the sequence is trickier...
    if (int returnCount = SlotCount(f.fType.typeNode())) {
        this->write(ByteCodeInstruction::kReserve);
        this->write8(returnCount);
    }

    int argCount = f.fArguments.size();
    std::vector<std::unique_ptr<LValue>> lvalues;
    for (int i = 0; i < argCount; ++i) {
        const Variable& param = (Variable&) fn.fParameters[i].node();
        const Expression& arg = f.fArguments[i].expressionNode();
        if (param.fModifiers.fFlags & Modifiers::kOut_Flag) {
            lvalues.emplace_back(this->getLValue(arg));
            lvalues.back()->load();
        } else {
            this->writeExpression(arg);
        }
    }

    this->write(ByteCodeInstruction::kCall);
    fCallTargets.emplace_back(this, fn);

    // After the called function returns, the stack will still contain our arguments. We have to
    // pop them (storing any out parameters back to their lvalues as we go). We glob together slot
    // counts for all parameters that aren't out-params, so we can pop them in one big chunk.
    int popCount = 0;
    auto pop = [&]() {
        if (popCount > 4) {
            this->write(ByteCodeInstruction::kPopN);
            this->write8(popCount);
        } else if (popCount > 0) {
            this->write(vector_instruction(ByteCodeInstruction::kPop, popCount));
        }
        popCount = 0;
    };

    for (int i = argCount - 1; i >= 0; --i) {
        Variable& param = (Variable&) fn.fParameters[i].node();
        Expression& arg = f.fArguments[i].expressionNode();
        if (param.fModifiers.fFlags & Modifiers::kOut_Flag) {
            pop();
            lvalues.back()->store(true);
            lvalues.pop_back();
        } else {
            popCount += SlotCount(arg.fType.typeNode());
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
    Expression& operand = (Expression&) p.fOperand.node();
    switch (p.fOperator) {
        case Token::Kind::PLUSPLUS: // fall through
        case Token::Kind::MINUSMINUS: {
            SkASSERT(SlotCount(operand.fType.typeNode()) == 1);
            std::unique_ptr<LValue> lvalue = this->getLValue(operand);
            lvalue->load();
            this->write(ByteCodeInstruction::kPushImmediate);
            this->write32(type_category(p.fType.typeNode()) == TypeCategory::kFloat ?
                          float_to_bits(1.0f) : 1);
            if (p.fOperator == Token::Kind::PLUSPLUS) {
                this->writeTypedInstruction(p.fType.typeNode(),
                                            ByteCodeInstruction::kAddI,
                                            ByteCodeInstruction::kAddI,
                                            ByteCodeInstruction::kAddF,
                                            1);
            } else {
                this->writeTypedInstruction(p.fType.typeNode(),
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
            this->writeExpression(operand);
            this->writeTypedInstruction(p.fType.typeNode(),
                                        ByteCodeInstruction::kNegateI,
                                        ByteCodeInstruction::kNegateI,
                                        ByteCodeInstruction::kNegateF,
                                        SlotCount(operand.fType.typeNode()));
            break;
        }
        default:
            SkASSERT(false);
    }
    return discard;
}

bool ByteCodeGenerator::writePostfixExpression(const PostfixExpression& p, bool discard) {
    Expression& operand = (Expression&) p.fOperand.node();
    switch (p.fOperator) {
        case Token::Kind::PLUSPLUS: // fall through
        case Token::Kind::MINUSMINUS: {
            SkASSERT(SlotCount(operand.fType.typeNode()) == 1);
            std::unique_ptr<LValue> lvalue = this->getLValue(operand);
            lvalue->load();
            if (!discard) {
                this->write(ByteCodeInstruction::kDup);
            }
            this->write(ByteCodeInstruction::kPushImmediate);
            this->write32(type_category(p.fType.typeNode()) == TypeCategory::kFloat ?
                          float_to_bits(1.0f) : 1);
            if (p.fOperator == Token::Kind::PLUSPLUS) {
                this->writeTypedInstruction(p.fType.typeNode(),
                                            ByteCodeInstruction::kAddI,
                                            ByteCodeInstruction::kAddI,
                                            ByteCodeInstruction::kAddF,
                                            1);
            } else {
                this->writeTypedInstruction(p.fType.typeNode(),
                                            ByteCodeInstruction::kSubtractI,
                                            ByteCodeInstruction::kSubtractI,
                                            ByteCodeInstruction::kSubtractF,
                                            1);
            }
            lvalue->store(discard);
            this->write(ByteCodeInstruction::kPop);
            discard = false;
            break;
        }
        default:
            SkASSERT(false);
    }
    return discard;
}

void ByteCodeGenerator::writeSwizzle(const Swizzle& s) {
    if (swizzle_is_simple(fProgram, s)) {
        this->writeVariableExpression(s);
        return;
    }

    Expression& base = (Expression&) s.fBase.node();
    switch (base.fKind) {
        case Expression::kVariableReference_Kind: {
            const Variable& var = (Variable&) ((VariableReference&) base).fVariable.node();
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
            this->writeExpression(base);
            this->write(ByteCodeInstruction::kSwizzle);
            this->write8(base.fType.typeNode().columns());
            this->write8(s.fComponents.size());
            for (int c : s.fComponents) {
                this->write8(c);
            }
    }
}

void ByteCodeGenerator::writeTernaryExpression(const TernaryExpression& t) {
    this->writeExpression(t.fTest.expressionNode());
    this->write(ByteCodeInstruction::kMaskPush);
    this->writeExpression(t.fIfTrue.expressionNode());
    this->write(ByteCodeInstruction::kMaskNegate);
    this->writeExpression(t.fIfFalse.expressionNode());
    this->write(ByteCodeInstruction::kMaskBlend);
    this->write8(SlotCount(t.fType.typeNode()));
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
        int count = SlotCount(e.fType.typeNode());
        if (count > 4) {
            this->write(ByteCodeInstruction::kPopN);
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
        , fCount(ByteCodeGenerator::SlotCount(value.type().typeNode()))
        , fIndex(index) {}

    void load() override {
        fGenerator.write(vector_instruction(ByteCodeInstruction::kReadExternal, fCount));
        fGenerator.write8(fIndex);
    }

    void store(bool discard) override {
        if (!discard) {
            fGenerator.write(vector_instruction(ByteCodeInstruction::kDup, fCount));
        }
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

    void store(bool discard) override {
        if (!discard) {
            fGenerator.write(vector_instruction(ByteCodeInstruction::kDup,
                                                fSwizzle.fComponents.size()));
        }
        Variable::Storage storage;
        int location = fGenerator.getLocation(fSwizzle.fBase.expressionNode(), &storage);
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

    void store(bool discard) override {
        int count = ByteCodeGenerator::SlotCount(fExpression.fType.typeNode());
        if (!discard) {
            if (count > 4) {
                fGenerator.write(ByteCodeInstruction::kDupN);
                fGenerator.write8(count);
            } else {
                fGenerator.write(vector_instruction(ByteCodeInstruction::kDup, count));
            }
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
            ExternalValue& value = ((ExternalValueReference&) e).fValue;
            int index = fOutput->fExternalValues.size();
            fOutput->fExternalValues.push_back(&value);
            SkASSERT(index <= 255);
            return std::unique_ptr<LValue>(new ByteCodeExternalValueLValue(this, value, index));
        }
        case Expression::kFieldAccess_Kind:
        case Expression::kIndex_Kind:
        case Expression::kVariableReference_Kind:
            return std::unique_ptr<LValue>(new ByteCodeExpressionLValue(this, e));
        case Expression::kSwizzle_Kind: {
            const Swizzle& s = (const Swizzle&) e;
            return swizzle_is_simple(fProgram, s)
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
    for (auto s : b.fStatements) {
        this->writeStatement((Statement&) s.node());
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
    this->writeStatement(d.fStatement.statementNode());
    this->write(ByteCodeInstruction::kLoopNext);
    this->writeExpression(d.fTest.expressionNode());
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
        this->writeStatement((Statement&) f.fInitializer.node());
    }
    this->write(ByteCodeInstruction::kLoopBegin);
    size_t start = fCode->size();
    if (f.fTest) {
        this->writeExpression(f.fTest.expressionNode());
        this->write(ByteCodeInstruction::kLoopMask);
    }
    this->write(ByteCodeInstruction::kBranchIfAllFalse);
    DeferredLocation endLocation(this);
    this->writeStatement(f.fStatement.statementNode());
    this->write(ByteCodeInstruction::kLoopNext);
    if (f.fNextExpression) {
        this->writeExpression(f.fNextExpression.expressionNode(), true);
    }
    this->write(ByteCodeInstruction::kBranch);
    this->write16(start);
    endLocation.set();
    this->write(ByteCodeInstruction::kLoopEnd);
}

void ByteCodeGenerator::writeIfStatement(const IfStatement& i) {
    this->writeExpression(i.fTest.expressionNode());
    this->write(ByteCodeInstruction::kMaskPush);
    this->write(ByteCodeInstruction::kBranchIfAllFalse);
    DeferredLocation falseLocation(this);
    this->writeStatement(i.fIfTrue.statementNode());
    falseLocation.set();
    if (i.fIfFalse) {
        this->write(ByteCodeInstruction::kMaskNegate);
        this->write(ByteCodeInstruction::kBranchIfAllFalse);
        DeferredLocation endLocation(this);
        this->writeStatement(i.fIfFalse.statementNode());
        endLocation.set();
    }
    this->write(ByteCodeInstruction::kMaskPop);
}

void ByteCodeGenerator::writeReturnStatement(const ReturnStatement& r) {
    this->writeExpression(r.fExpression.expressionNode());
    this->write(ByteCodeInstruction::kReturn);
    this->write8(SlotCount(r.fExpression.expressionNode().fType.typeNode()));
}

void ByteCodeGenerator::writeSwitchStatement(const SwitchStatement& r) {
    // not yet implemented
    abort();
}

void ByteCodeGenerator::writeVarDeclarations(const VarDeclarations& v) {
    for (IRNode::ID declStatement : v.fVars) {
        const VarDeclaration& decl = (VarDeclaration&) declStatement.node();
        // we need to grab the location even if we don't use it, to ensure it
        // has been allocated
        int location = getLocation((Variable&) decl.fVar.node());
        if (decl.fValue) {
            this->writeExpression(decl.fValue.expressionNode());
            int count = SlotCount(decl.fValue.expressionNode().fType.typeNode());
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
    this->write(ByteCodeInstruction::kLoopBegin);
    size_t cond = fCode->size();
    this->writeExpression(w.fTest.expressionNode());
    this->write(ByteCodeInstruction::kLoopMask);
    this->write(ByteCodeInstruction::kBranchIfAllFalse);
    DeferredLocation endLocation(this);
    this->writeStatement(w.fStatement.statementNode());
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
            this->writeExpression(((ExpressionStatement&) s).fExpression.expressionNode(), true);
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
        case Statement::kVarDeclarations_Kind: {
            VarDeclarationsStatement& vd = (VarDeclarationsStatement&) s;
            this->writeVarDeclarations((VarDeclarations&) vd.fDeclaration.node());
            break;
        }
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
    for (IRNode::ID pID : declaration->fParameters) {
        const Variable& p = (Variable&) pID.node();
        int slots = ByteCodeGenerator::SlotCount(p.fType.typeNode());
        fParameters.push_back({ slots, (bool) (p.fModifiers.fFlags & Modifiers::kOut_Flag) });
        fParameterCount += slots;
    }
}

}
