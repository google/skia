/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLIRGenerator.h"

#include "limits.h"
#include <unordered_set>

#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLParser.h"
#include "src/sksl/SkSLExternalValueSymbol.h"
#include "src/sksl/ir/SkSLAppendStage.h"
#include "src/sksl/ir/SkSLBinaryExpression.h"
#include "src/sksl/ir/SkSLBoolLiteral.h"
#include "src/sksl/ir/SkSLBreakStatement.h"
#include "src/sksl/ir/SkSLConstructor.h"
#include "src/sksl/ir/SkSLContinueStatement.h"
#include "src/sksl/ir/SkSLDiscardStatement.h"
#include "src/sksl/ir/SkSLDoStatement.h"
#include "src/sksl/ir/SkSLEnum.h"
#include "src/sksl/ir/SkSLExpressionStatement.h"
#include "src/sksl/ir/SkSLExtension.h"
#include "src/sksl/ir/SkSLExternalFunctionCall.h"
#include "src/sksl/ir/SkSLExternalValueReference.h"
#include "src/sksl/ir/SkSLField.h"
#include "src/sksl/ir/SkSLFieldAccess.h"
#include "src/sksl/ir/SkSLFloatLiteral.h"
#include "src/sksl/ir/SkSLForStatement.h"
#include "src/sksl/ir/SkSLFunctionCall.h"
#include "src/sksl/ir/SkSLFunctionDeclaration.h"
#include "src/sksl/ir/SkSLFunctionDefinition.h"
#include "src/sksl/ir/SkSLFunctionReference.h"
#include "src/sksl/ir/SkSLIfStatement.h"
#include "src/sksl/ir/SkSLIndexExpression.h"
#include "src/sksl/ir/SkSLIntLiteral.h"
#include "src/sksl/ir/SkSLInterfaceBlock.h"
#include "src/sksl/ir/SkSLLayout.h"
#include "src/sksl/ir/SkSLModifiersDeclaration.h"
#include "src/sksl/ir/SkSLNullLiteral.h"
#include "src/sksl/ir/SkSLPostfixExpression.h"
#include "src/sksl/ir/SkSLPrefixExpression.h"
#include "src/sksl/ir/SkSLReturnStatement.h"
#include "src/sksl/ir/SkSLSection.h"
#include "src/sksl/ir/SkSLSetting.h"
#include "src/sksl/ir/SkSLSwitchCase.h"
#include "src/sksl/ir/SkSLSwitchStatement.h"
#include "src/sksl/ir/SkSLSwizzle.h"
#include "src/sksl/ir/SkSLTernaryExpression.h"
#include "src/sksl/ir/SkSLTypeReference.h"
#include "src/sksl/ir/SkSLUnresolvedFunction.h"
#include "src/sksl/ir/SkSLVarDeclarations.h"
#include "src/sksl/ir/SkSLVarDeclarationsStatement.h"
#include "src/sksl/ir/SkSLVariable.h"
#include "src/sksl/ir/SkSLVariableReference.h"
#include "src/sksl/ir/SkSLWhileStatement.h"

namespace SkSL {

class AutoSymbolTable {
public:
    AutoSymbolTable(IRGenerator* ir)
        : fIR(ir)
        , fPrevious(fIR->fSymbolTable) {
        fIR->pushSymbolTable();
    }

    ~AutoSymbolTable() {
        fIR->popSymbolTable();
        SkASSERT(fPrevious == fIR->fSymbolTable);
    }

    IRGenerator* fIR;
    std::shared_ptr<SymbolTable> fPrevious;
};

class AutoLoopLevel {
public:
    AutoLoopLevel(IRGenerator* ir)
        : fIR(ir) {
        fIR->fLoopLevel++;
    }

    ~AutoLoopLevel() {
        fIR->fLoopLevel--;
    }

    IRGenerator* fIR;
};

class AutoSwitchLevel {
public:
    AutoSwitchLevel(IRGenerator* ir)
        : fIR(ir) {
        fIR->fSwitchLevel++;
    }

    ~AutoSwitchLevel() {
        fIR->fSwitchLevel--;
    }

    IRGenerator* fIR;
};

IRGenerator::IRGenerator(ErrorReporter& errorReporter)
    : fContext(this)
    , fErrors(errorReporter)
    , fSymbolTable(new SymbolTable(this))
    , fLoopLevel(0)
    , fSwitchLevel(0)
    , fTmpCount(0) {}

IRGenerator::~IRGenerator() {
    for (int i = fLegacyNodes.size() - 1; i >= 0; --i) {
        fLegacyNodes[i] = nullptr;
    }
}

void IRGenerator::pushSymbolTable() {
    fSymbolTable.reset(new SymbolTable(std::move(fSymbolTable), this));
}

void IRGenerator::popSymbolTable() {
    fSymbolTable = fSymbolTable->fParent;
}

static void fill_caps(const SKSL_CAPS_CLASS& caps,
                      std::unordered_map<String, Program::Settings::Value>* capsMap) {
#define CAP(name) \
    capsMap->insert(std::make_pair(String(#name), Program::Settings::Value(caps.name())))
    CAP(fbFetchSupport);
    CAP(fbFetchNeedsCustomOutput);
    CAP(flatInterpolationSupport);
    CAP(noperspectiveInterpolationSupport);
    CAP(sampleVariablesSupport);
    CAP(externalTextureSupport);
    CAP(mustEnableAdvBlendEqs);
    CAP(mustEnableSpecificAdvBlendEqs);
    CAP(mustDeclareFragmentShaderOutput);
    CAP(mustDoOpBetweenFloorAndAbs);
    CAP(atan2ImplementedAsAtanYOverX);
    CAP(canUseAnyFunctionInShader);
    CAP(floatIs32Bits);
    CAP(integerSupport);
#undef CAP
}

void IRGenerator::start(const Program::Settings* settings,
                        std::vector<IRNode::ID>* inherited) {
    if (fStarted) {
        this->popSymbolTable();
    }
    fSettings = settings;
    fCapsMap.clear();
    if (settings->fCaps) {
        fill_caps(*settings->fCaps, &fCapsMap);
    } else {
        fCapsMap.insert(std::make_pair(String("integerSupport"),
                                       Program::Settings::Value(true)));
    }
    this->pushSymbolTable();
    fInvocations = -1;
    fInputs.reset();
    fSkPerVertex = IRNode::ID();
    fRTAdjust = IRNode::ID();
    fRTAdjustInterfaceBlock = IRNode::ID();
    if (inherited) {
        for (IRNode::ID e : *inherited) {
            if (((ProgramElement&) e.node()).fKind == ProgramElement::kInterfaceBlock_Kind) {
                InterfaceBlock& intf = (InterfaceBlock&) e.node();
                if (((Variable&) intf.fVariable.node()).fName == Compiler::PERVERTEX_NAME) {
                    SkASSERT(!fSkPerVertex);
                    fSkPerVertex = intf.fVariable;
                }
            }
        }
    }
}

IRNode::ID IRGenerator::createNode(IRNode* node) {
    fLegacyNodes.emplace_back(node);
    return IRNode::ID(node);
}

IRNode::ID IRGenerator::convertExtension(int offset, StringFragment name) {
    return this->createNode(new Extension(offset, name));
}

void IRGenerator::finish() {
    this->popSymbolTable();
    fSettings = nullptr;
}

IRNode::ID IRGenerator::convertStatement(const ASTNode& statement) {
    switch (statement.fKind) {
        case ASTNode::Kind::kBlock:
            return this->convertBlock(statement);
        case ASTNode::Kind::kVarDeclarations:
            return this->convertVarDeclarationStatement(statement);
        case ASTNode::Kind::kIf:
            return this->convertIf(statement);
        case ASTNode::Kind::kFor:
            return this->convertFor(statement);
        case ASTNode::Kind::kWhile:
            return this->convertWhile(statement);
        case ASTNode::Kind::kDo:
            return this->convertDo(statement);
        case ASTNode::Kind::kSwitch:
            return this->convertSwitch(statement);
        case ASTNode::Kind::kReturn:
            return this->convertReturn(statement);
        case ASTNode::Kind::kBreak:
            return this->convertBreak(statement);
        case ASTNode::Kind::kContinue:
            return this->convertContinue(statement);
        case ASTNode::Kind::kDiscard:
            return this->convertDiscard(statement);
        default:
            // it's an expression
            IRNode::ID result = this->convertExpressionStatement(statement);
            if (fRTAdjust && Program::kGeometry_Kind == fKind) {
                SkASSERT(result.statementNode().fKind == Statement::kExpression_Kind);
                Expression& expr =
                                ((ExpressionStatement&) result.node()).fExpression.expressionNode();
                if (expr.fKind == Expression::kFunctionCall_Kind) {
                    FunctionCall& fc = (FunctionCall&) expr;
                    FunctionDeclaration& f = (FunctionDeclaration&) fc.fFunction.node();
                    if (f.fBuiltin && f.fName == "EmitVertex") {
                        std::vector<IRNode::ID> statements;
                        statements.push_back(getNormalizeSkPositionCode());
                        statements.push_back(result);
                        return this->createNode(new Block(this, statement.fOffset,
                                                          std::move(statements), fSymbolTable));
                    }
                }
            }
            return result;
    }
}

IRNode::ID IRGenerator::convertBlock(const ASTNode& block) {
    SkASSERT(block.fKind == ASTNode::Kind::kBlock);
    AutoSymbolTable table(this);
    std::vector<IRNode::ID> statements;
    for (const auto& child : block) {
        IRNode::ID statement = this->convertStatement(child);
        if (!statement) {
            return IRNode::ID();
        }
        statements.push_back(statement);
    }
    return this->createNode(new Block(this, block.fOffset, std::move(statements), fSymbolTable));
}

IRNode::ID IRGenerator::convertVarDeclarationStatement(const ASTNode& s) {
    SkASSERT(s.fKind == ASTNode::Kind::kVarDeclarations);
    auto decl = this->convertVarDeclarations(s, Variable::kLocal_Storage);
    if (!decl) {
        return IRNode::ID();
    }
    return this->createNode(new VarDeclarationsStatement(this, decl));
}

IRNode::ID IRGenerator::convertVarDeclarations(const ASTNode& decls, Variable::Storage storage) {
    SkASSERT(decls.fKind == ASTNode::Kind::kVarDeclarations);
    auto iter = decls.begin();
    const Modifiers& modifiers = iter++->getModifiers();
    const ASTNode& rawType = *(iter++);
    std::vector<IRNode::ID> variables;
    IRNode::ID baseType = this->convertType(rawType);
    if (!baseType) {
        return IRNode::ID();
    }
    if (fKind != Program::kFragmentProcessor_Kind && (modifiers.fFlags & Modifiers::kIn_Flag) &&
        baseType.typeNode().kind() == Type::Kind::kMatrix_Kind) {
        fErrors.error(decls.fOffset, "'in' variables may not have matrix type");
    }
    if (modifiers.fLayout.fWhen.fLength && fKind != Program::kFragmentProcessor_Kind &&
        fKind != Program::kPipelineStage_Kind) {
        fErrors.error(decls.fOffset, "'when' is only permitted within fragment processors");
    }
    if (modifiers.fLayout.fKey) {
        if (fKind != Program::kFragmentProcessor_Kind && fKind != Program::kPipelineStage_Kind) {
            fErrors.error(decls.fOffset, "'key' is only permitted within fragment processors");
        }
        if ((modifiers.fFlags & Modifiers::kUniform_Flag) != 0) {
            fErrors.error(decls.fOffset, "'key' is not permitted on 'uniform' variables");
        }
    }
    for (; iter != decls.end(); ++iter) {
        const ASTNode& varDecl = *iter;
        if (modifiers.fLayout.fLocation == 0 && modifiers.fLayout.fIndex == 0 &&
            (modifiers.fFlags & Modifiers::kOut_Flag) && fKind == Program::kFragment_Kind &&
            varDecl.getVarData().fName != "sk_FragColor") {
            fErrors.error(varDecl.fOffset,
                          "out location=0, index=0 is reserved for sk_FragColor");
        }
        const ASTNode::VarData& varData = varDecl.getVarData();
        StringFragment name = varData.fName;
        IRNode::ID type = baseType;
        std::vector<IRNode::ID> sizes;
        auto iter = varDecl.begin();
        for (size_t i = 0; i < varData.fSizeCount; ++i, ++iter) {
            const ASTNode& rawSize = *iter;
            if (rawSize) {
                auto sizeID = this->coerce(this->convertExpression(rawSize), fContext.fInt_Type);
                if (!sizeID) {
                    return IRNode::ID();
                }
                String name(type.typeNode().fName);
                int64_t count;
                Expression& size = sizeID.expressionNode();
                if (size.fKind == Expression::kIntLiteral_Kind) {
                    count = ((IntLiteral&) size).fValue;
                    if (count <= 0) {
                        fErrors.error(size.fOffset, "array size must be positive");
                    }
                    name += "[" + to_string(count) + "]";
                } else {
                    count = -1;
                    name += "[]";
                }
                type = this->createNode(new Type(this, name, Type::kArray_Kind, type, (int) count));
                sizes.push_back(sizeID);
            } else {
                type = this->createNode(new Type(this, type.typeNode().name() + "[]",
                                                 Type::kArray_Kind, type, -1));
                sizes.push_back(IRNode::ID());
            }
        }
        IRNode::ID var = this->createNode(new Variable(this, varDecl.fOffset, modifiers,
                                                       name, type, storage, IRNode::ID()));
        if (name == Compiler::RTADJUST_NAME) {
            SkASSERT(!fRTAdjust);
            SkASSERT(type == fContext.fFloat4_Type);
            fRTAdjust = var;
        }
        IRNode::ID value;
        if (iter != varDecl.end()) {
            value = this->convertExpression(*iter);
            if (!value) {
                return IRNode::ID();
            }
            value = this->coerce(value, type);
            if (!value) {
                return IRNode::ID();
            }
            ((Variable&) var.node()).fWriteCount = 1;
            ((Variable&) var.node()).fInitialValue = value;
        }
        if (storage == Variable::kGlobal_Storage && name == "sk_FragColor" &&
            (*fSymbolTable)[name]) {
            // already defined, ignore
        } else if (storage == Variable::kGlobal_Storage && (*fSymbolTable)[name] &&
                   ((Symbol&) (*fSymbolTable)[name].node()).fSymbolKind == Symbol::kVariable_Kind &&
                   ((Variable&) (*fSymbolTable)[name].node()).fModifiers.fLayout.fBuiltin >= 0) {
            // already defined, just update the modifiers
            Variable& old = (Variable&) (*fSymbolTable)[name].node();
            old.fModifiers = ((Variable&) var.node()).fModifiers;
        } else {
            variables.push_back(this->createNode(new VarDeclaration(this, var, std::move(sizes),
                                                                    value)));
            fSymbolTable->add(name, var);
        }
    }
    return this->createNode(new VarDeclarations(this, decls.fOffset, baseType,
                                                std::move(variables)));
}

IRNode::ID IRGenerator::convertModifiersDeclaration(const ASTNode& m) {
    SkASSERT(m.fKind == ASTNode::Kind::kModifiers);
    Modifiers modifiers = m.getModifiers();
    if (modifiers.fLayout.fInvocations != -1) {
        if (fKind != Program::kGeometry_Kind) {
            fErrors.error(m.fOffset, "'invocations' is only legal in geometry shaders");
            return IRNode::ID();
        }
        fInvocations = modifiers.fLayout.fInvocations;
        if (fSettings->fCaps && !fSettings->fCaps->gsInvocationsSupport()) {
            modifiers.fLayout.fInvocations = -1;
            SkASSERT((*fSymbolTable)["sk_InvocationID"]);
            Variable& invocationId = (Variable&) (*fSymbolTable)["sk_InvocationID"].node();
            invocationId.fModifiers.fFlags = 0;
            invocationId.fModifiers.fLayout.fBuiltin = -1;
            if (modifiers.fLayout.description() == "") {
                return IRNode::ID();
            }
        }
    }
    if (modifiers.fLayout.fMaxVertices != -1 && fInvocations > 0 && fSettings->fCaps &&
        !fSettings->fCaps->gsInvocationsSupport()) {
        modifiers.fLayout.fMaxVertices *= fInvocations;
    }
    return this->createNode(new ModifiersDeclaration(this, modifiers));
}

IRNode::ID IRGenerator::convertIf(const ASTNode& n) {
    SkASSERT(n.fKind == ASTNode::Kind::kIf);
    auto iter = n.begin();
    IRNode::ID test = this->coerce(this->convertExpression(*(iter++)), fContext.fBool_Type);
    if (!test) {
        return IRNode::ID();
    }
    IRNode::ID ifTrue = this->convertStatement(*(iter++));
    if (!ifTrue) {
        return IRNode::ID();
    }
    IRNode::ID ifFalse;
    if (iter != n.end()) {
        ifFalse = this->convertStatement(*(iter++));
        if (!ifFalse) {
            return IRNode::ID();
        }
    }
    if (test.expressionNode().fKind == Expression::kBoolLiteral_Kind) {
        // static boolean value, fold down to a single branch
        if (((BoolLiteral&) test.node()).fValue) {
            return ifTrue;
        } else if (ifFalse) {
            return ifFalse;
        } else {
            // False & no else clause. Not an error, so don't return null!
            return this->createNode(new Block(this, n.fOffset, std::vector<IRNode::ID>(),
                                              fSymbolTable));
        }
    }
    return this->createNode(new IfStatement(this, n.fOffset, n.getBool(), test, ifTrue, ifFalse));
}

IRNode::ID IRGenerator::convertFor(const ASTNode& f) {
    SkASSERT(f.fKind == ASTNode::Kind::kFor);
    AutoLoopLevel level(this);
    AutoSymbolTable table(this);
    IRNode::ID initializer;
    auto iter = f.begin();
    if (*iter) {
        initializer = this->convertStatement(*iter);
        if (!initializer) {
            return IRNode::ID();
        }
    }
    ++iter;
    IRNode::ID test;
    if (*iter) {
        test = this->coerce(this->convertExpression(*iter), fContext.fBool_Type);
        if (!test) {
            return IRNode::ID();
        }
    }
    ++iter;
    IRNode::ID next;
    if (*iter) {
        next = this->convertExpression(*iter);
        if (!next) {
            return IRNode::ID();
        }
        this->checkValid(next.expressionNode());
    }
    ++iter;
    IRNode::ID statement = this->convertStatement(*iter);
    if (!statement) {
        return IRNode::ID();
    }
    return this->createNode(new ForStatement(this, f.fOffset, initializer, test, next, statement,
                                             fSymbolTable));
}

IRNode::ID IRGenerator::convertWhile(const ASTNode& w) {
    SkASSERT(w.fKind == ASTNode::Kind::kWhile);
    AutoLoopLevel level(this);
    auto iter = w.begin();
    IRNode::ID test = this->coerce(this->convertExpression(*(iter++)), fContext.fBool_Type);
    if (!test) {
        return IRNode::ID();
    }
    IRNode::ID statement = this->convertStatement(*(iter++));
    if (!statement) {
        return IRNode::ID();
    }
    return this->createNode(new WhileStatement(this, w.fOffset, test, statement));
}

IRNode::ID IRGenerator::convertDo(const ASTNode& d) {
    SkASSERT(d.fKind == ASTNode::Kind::kDo);
    AutoLoopLevel level(this);
    auto iter = d.begin();
    IRNode::ID statement = this->convertStatement(*(iter++));
    if (!statement) {
        return IRNode::ID();
    }
    IRNode::ID test = this->coerce(this->convertExpression(*(iter++)), fContext.fBool_Type);
    if (!test) {
        return IRNode::ID();
    }
    return this->createNode(new DoStatement(this, d.fOffset, statement, test));
}

IRNode::ID IRGenerator::convertSwitch(const ASTNode& s) {
    SkASSERT(s.fKind == ASTNode::Kind::kSwitch);
    AutoSwitchLevel level(this);
    auto iter = s.begin();
    IRNode::ID valueID = this->convertExpression(*(iter++));
    if (!valueID) {
        return IRNode::ID();
    }
    if (valueID.expressionNode().fType != fContext.fUInt_Type &&
        valueID.expressionNode().fType.typeNode().kind() != Type::kEnum_Kind) {
        valueID = this->coerce(valueID, fContext.fInt_Type);
        if (!valueID) {
            return IRNode::ID();
        }
    }
    AutoSymbolTable table(this);
    std::unordered_set<int> caseValues;
    std::vector<IRNode::ID> cases;
    for (; iter != s.end(); ++iter) {
        const ASTNode& c = *iter;
        SkASSERT(c.fKind == ASTNode::Kind::kSwitchCase);
        IRNode::ID caseValue;
        auto childIter = c.begin();
        if (*childIter) {
            caseValue = this->convertExpression(*childIter);
            if (!caseValue) {
                return IRNode::ID();
            }
            caseValue = this->coerce(caseValue, valueID.expressionNode().fType);
            if (!caseValue) {
                return IRNode::ID();
            }
            if (!caseValue.node().isConstant()) {
                fErrors.error(caseValue.node().fOffset, "case value must be a constant");
                return IRNode::ID();
            }
            int64_t v;
            this->getConstantInt(caseValue.expressionNode(), &v);
            if (caseValues.find(v) != caseValues.end()) {
                fErrors.error(caseValue.node().fOffset, "duplicate case value");
            }
            caseValues.insert(v);
        }
        ++childIter;
        std::vector<IRNode::ID> statements;
        for (; childIter != c.end(); ++childIter) {
            IRNode::ID converted = this->convertStatement(*childIter);
            if (!converted) {
                return IRNode::ID();
            }
            statements.push_back(converted);
        }
        cases.emplace_back(new SwitchCase(this, c.fOffset, caseValue, std::move(statements)));
    }
    return this->createNode(new SwitchStatement(this, s.fOffset, s.getBool(), valueID,
                                                std::move(cases), fSymbolTable));
}

IRNode::ID IRGenerator::convertExpressionStatement(const ASTNode& s) {
    IRNode::ID e = this->convertExpression(s);
    if (!e) {
        return IRNode::ID();
    }
    this->checkValid(e.expressionNode());
    return this->createNode(new ExpressionStatement(this, e));
}

IRNode::ID IRGenerator::convertReturn(const ASTNode& r) {
    SkASSERT(r.fKind == ASTNode::Kind::kReturn);
    SkASSERT(fCurrentFunction);
    // early returns from a vertex main function will bypass the sk_Position normalization, so
    // SkASSERT that we aren't doing that. It is of course possible to fix this by adding a
    // normalization before each return, but it will probably never actually be necessary.
    SkASSERT(Program::kVertex_Kind != fKind || !fRTAdjust ||
             ((FunctionDeclaration&) fCurrentFunction.node()).fName != "main");
    IRNode::ID returnType = ((FunctionDeclaration&) fCurrentFunction.node()).fReturnType;
    if (r.begin() != r.end()) {
        IRNode::ID result = this->convertExpression(*r.begin());
        if (!result) {
            return IRNode::ID();
        }
        if (returnType == fContext.fVoid_Type) {
            fErrors.error(result.node().fOffset, "may not return a value from a void function");
        } else {
            result = this->coerce(result, returnType);
            if (!result) {
                return IRNode::ID();
            }
        }
        return this->createNode(new ReturnStatement(this, result));
    } else {
        if (returnType != fContext.fVoid_Type) {
            fErrors.error(r.fOffset, "expected function to return '" +
                                     returnType.node().description() + "'");
        }
        return this->createNode(new ReturnStatement(this, r.fOffset));
    }
}

IRNode::ID IRGenerator::convertBreak(const ASTNode& b) {
    SkASSERT(b.fKind == ASTNode::Kind::kBreak);
    if (fLoopLevel > 0 || fSwitchLevel > 0) {
        return this->createNode(new BreakStatement(this, b.fOffset));
    } else {
        fErrors.error(b.fOffset, "break statement must be inside a loop or switch");
        return IRNode::ID();
    }
}

IRNode::ID IRGenerator::convertContinue(const ASTNode& c) {
    SkASSERT(c.fKind == ASTNode::Kind::kContinue);
    if (fLoopLevel > 0) {
        return this->createNode(new ContinueStatement(this, c.fOffset));
    } else {
        fErrors.error(c.fOffset, "continue statement must be inside a loop");
        return IRNode::ID();
    }
}

IRNode::ID IRGenerator::convertDiscard(const ASTNode& d) {
    SkASSERT(d.fKind == ASTNode::Kind::kDiscard);
    return this->createNode(new DiscardStatement(this, d.fOffset));
}

IRNode::ID IRGenerator::applyInvocationIDWorkaround(IRNode::ID main) {
    Layout invokeLayout;
    Modifiers invokeModifiers(invokeLayout, Modifiers::kHasSideEffects_Flag);
    const char* name = "_invoke";
    IRNode::ID invokeDecl = this->createNode(new FunctionDeclaration(this,
                                                                     -1,
                                                                     invokeModifiers,
                                                                     name,
                                                                     std::vector<IRNode::ID>(),
                                                                     fContext.fVoid_Type));
    fProgramElements->push_back(this->createNode(new FunctionDefinition(this, -1, invokeDecl,
                                                                        std::move(main))));
    fSymbolTable->add(name, invokeDecl);

    std::vector<std::unique_ptr<VarDeclaration>> variables;
    IRNode::ID loopIdx = (*fSymbolTable)["sk_InvocationID"];
    SkASSERT(loopIdx);
    IRNode::ID test = this->createNode(new BinaryExpression(this, -1,
                                         this->createNode(new VariableReference(this, -1, loopIdx)),
                                         Token::LT,
                                         this->createNode(new IntLiteral(this, -1, fInvocations)),
                                         fContext.fBool_Type));
    IRNode::ID next = this->createNode(new PostfixExpression(this,
                                        this->createNode(new VariableReference(this, -1, loopIdx,
                                                                               kReadWrite_RefKind)),
                                        Token::PLUSPLUS));
    ASTNode endPrimitiveID(&fFile->fNodes, -1, ASTNode::Kind::kIdentifier, "EndPrimitive");
    IRNode::ID endPrimitive = this->convertExpression(endPrimitiveID);
    SkASSERT(endPrimitive);

    std::vector<IRNode::ID> loopBody;
    std::vector<IRNode::ID> invokeArgs;
    loopBody.push_back(this->createNode(new ExpressionStatement(this,
                                                   this->callFunction(-1,
                                                                      invokeDecl,
                                                                      std::vector<IRNode::ID>()))));
    loopBody.push_back(this->createNode(new ExpressionStatement(this,
                                                 this->callExpression(-1,
                                                                      std::move(endPrimitive),
                                                                      std::vector<IRNode::ID>()))));
    IRNode::ID assignment(new BinaryExpression(this, -1,
                    this->createNode(new VariableReference(this, -1, loopIdx)),
                    Token::EQ,
                    this->createNode(new IntLiteral(this, -1, 0)),
                    fContext.fInt_Type));
    IRNode::ID initializer(new ExpressionStatement(this, assignment));
    IRNode::ID loop = this->createNode(new ForStatement(this,
                                                        -1,
                                                        initializer,
                                                        test,
                                                        next,
                                                        this->createNode(new Block(this, -1,
                                                                                   loopBody)),
                                                        fSymbolTable));
    return this->createNode(new Block(this, -1, { loop }));
}

IRNode::ID IRGenerator::getNormalizeSkPositionCode() {
    // sk_Position = float4(sk_Position.xy * rtAdjust.xz + sk_Position.ww * rtAdjust.yw,
    //                      0,
    //                      sk_Position.w);
    SkASSERT(fSkPerVertex && fRTAdjust);
    #define REF(var) this->createNode(new VariableReference(this, -1, var, kRead_RefKind))
    #define FIELD(var, idx) this->createNode( \
              new FieldAccess(this, REF(var), idx, FieldAccess::kAnonymousInterfaceBlock_OwnerKind))
    #define POS this->createNode(new FieldAccess(this, REF(fSkPerVertex), 0, \
                                                   FieldAccess::kAnonymousInterfaceBlock_OwnerKind))
    #define ADJUST (fRTAdjustInterfaceBlock ? \
                    FIELD(fRTAdjustInterfaceBlock, fRTAdjustFieldIndex) : \
                    REF(fRTAdjust))
    #define SWIZZLE(expr, ...) this->createNode(new Swizzle(this, expr, { __VA_ARGS__ }))
    #define OP(left, op, right) this->createNode( \
                                   new BinaryExpression(this, -1, left, op, right, \
                                                        fContext.fFloat2_Type))
    std::vector<IRNode::ID> children;
    children.push_back(OP(OP(SWIZZLE(POS, 0, 1), Token::STAR, SWIZZLE(ADJUST, 0, 2)),
                          Token::PLUS,
                          OP(SWIZZLE(POS, 3, 3), Token::STAR, SWIZZLE(ADJUST, 1, 3))));
    children.push_back(this->createNode(new FloatLiteral(this, -1, 0.0)));
    children.push_back(SWIZZLE(POS, 3));
    IRNode::ID result = OP(POS, Token::EQ, this->createNode(new Constructor(this,
                                                                            -1,
                                                                            fContext.fFloat4_Type,
                                                                            std::move(children))));
    return this->createNode(new ExpressionStatement(this, std::move(result)));
}

void IRGenerator::convertFunction(const ASTNode& f) {
    auto iter = f.begin();
    IRNode::ID returnType = this->convertType(*(iter++));
    if (!returnType) {
        return;
    }
    const ASTNode::FunctionData& fd = f.getFunctionData();
    std::vector<IRNode::ID> parameters;
    for (size_t i = 0; i < fd.fParameterCount; ++i) {
        const ASTNode& param = *(iter++);
        SkASSERT(param.fKind == ASTNode::Kind::kParameter);
        ASTNode::ParameterData pd = param.getParameterData();
        auto paramIter = param.begin();
        IRNode::ID type = this->convertType(*(paramIter++));
        if (!type) {
            return;
        }
        for (int j = (int) pd.fSizeCount; j >= 1; j--) {
            int size = (param.begin() + j)->getInt();
            String name = type.typeNode().name() + "[" + to_string(size) + "]";
            type = this->createNode(new Type(this, std::move(name), Type::kArray_Kind, type, size));
        }
        StringFragment name = pd.fName;
        IRNode::ID var = this->createNode(new Variable(this, param.fOffset, pd.fModifiers, name,
                                                       type, Variable::kParameter_Storage,
                                                       IRNode::ID()));
        parameters.push_back(var);
    }

    if (fd.fName == "main") {
        switch (fKind) {
            case Program::kPipelineStage_Kind: {
                bool valid;
                switch (parameters.size()) {
                    case 3:
                        valid = ((Variable&) parameters[0].node()).fType == fContext.fInt_Type &&
                                ((Variable&) parameters[0].node()).fModifiers.fFlags == 0 &&
                                ((Variable&) parameters[1].node()).fType == fContext.fInt_Type &&
                                ((Variable&) parameters[1].node()).fModifiers.fFlags == 0 &&
                                ((Variable&) parameters[2].node()).fType == fContext.fHalf4_Type &&
                                ((Variable&) parameters[2].node()).fModifiers.fFlags ==
                                (Modifiers::kIn_Flag | Modifiers::kOut_Flag);
                        break;
                    case 1:
                        valid = ((Variable&) parameters[0].node()).fType == fContext.fHalf4_Type &&
                                ((Variable&) parameters[0].node()).fModifiers.fFlags ==
                                (Modifiers::kIn_Flag | Modifiers::kOut_Flag);
                        break;
                    default:
                        valid = false;
                }
                if (!valid) {
                    fErrors.error(f.fOffset, "pipeline stage 'main' must be declared main(int, "
                                             "int, inout half4) or main(inout half4)");
                    return;
                }
                break;
            }
            case Program::kGeneric_Kind:
                break;
            default:
                if (parameters.size()) {
                    fErrors.error(f.fOffset, "shader 'main' must have zero parameters");
                }
        }
    }

    // find existing declaration
    IRNode::ID decl;
    auto entry = (*fSymbolTable)[fd.fName];
    if (entry) {
        std::vector<IRNode::ID> functions;
        switch (((Symbol&) entry.node()).fSymbolKind) {
            case Symbol::kUnresolvedFunction_Kind:
                functions = ((UnresolvedFunction&) entry.node()).fFunctions;
                break;
            case Symbol::kFunctionDeclaration_Kind:
                functions.push_back(entry);
                break;
            default:
                fErrors.error(f.fOffset, "symbol '" + fd.fName + "' was already defined");
                return;
        }
        for (IRNode::ID otherID : functions) {
            FunctionDeclaration& other = (FunctionDeclaration&) otherID.node();
            SkASSERT(other.fName == fd.fName);
            if (parameters.size() == other.fParameters.size()) {
                bool match = true;
                for (size_t i = 0; i < parameters.size(); i++) {
                    if (((Variable&) parameters[i].node()).fType !=
                        ((Variable&) other.fParameters[i].node()).fType) {
                        match = false;
                        break;
                    }
                }
                if (match) {
                    if (returnType != other.fReturnType) {
                        FunctionDeclaration newDecl(this, f.fOffset, fd.fModifiers, fd.fName,
                                                    parameters, returnType);
                        fErrors.error(f.fOffset, "functions '" + newDecl.description() +
                                                 "' and '" + other.description() +
                                                 "' differ only in return type");
                        return;
                    }
                    decl = otherID;
                    for (size_t i = 0; i < parameters.size(); i++) {
                        if (((Variable&) parameters[i].node()).fModifiers !=
                            ((Variable&) other.fParameters[i].node()).fModifiers) {
                            fErrors.error(f.fOffset, "modifiers on parameter " +
                                                     to_string((uint64_t) i + 1) +
                                                     " differ between declaration and "
                                                     "definition");
                            return;
                        }
                    }
                    if (other.fDefined) {
                        fErrors.error(f.fOffset, "duplicate definition of " + other.description());
                    }
                    break;
                }
            }
        }
    }
    if (!decl) {
        // couldn't find an existing declaration
        IRNode::ID newDecl = this->createNode(new FunctionDeclaration(this,
                                                                      f.fOffset,
                                                                      fd.fModifiers,
                                                                      fd.fName,
                                                                      parameters,
                                                                      returnType));
        decl = newDecl;
        fSymbolTable->add(fd.fName, newDecl);
    }
    if (iter != f.end()) {
        // compile body
        SkASSERT(!fCurrentFunction);
        fCurrentFunction = decl;
        ((FunctionDeclaration&) decl.node()).fDefined = true;
        std::shared_ptr<SymbolTable> old = fSymbolTable;
        AutoSymbolTable table(this);
        if (fd.fName == "main" && fKind == Program::kPipelineStage_Kind) {
            if (parameters.size() == 3) {
                ((Variable&) parameters[0].node()).fModifiers.fLayout.fBuiltin = SK_MAIN_X_BUILTIN;
                ((Variable&) parameters[1].node()).fModifiers.fLayout.fBuiltin = SK_MAIN_Y_BUILTIN;
                ((Variable&) parameters[2].node()).fModifiers.fLayout.fBuiltin =
                                                                                SK_OUTCOLOR_BUILTIN;
            } else {
                SkASSERT(parameters.size() == 1);
                ((Variable&) parameters[0].node()).fModifiers.fLayout.fBuiltin =
                                                                                SK_OUTCOLOR_BUILTIN;
            }
        }
        for (size_t i = 0; i < parameters.size(); i++) {
            fSymbolTable->add(((Variable&) parameters[i].node()).fName,
                              ((FunctionDeclaration&) decl.node()).fParameters[i]);
        }
        bool needInvocationIDWorkaround = fInvocations != -1 && fd.fName == "main" &&
                                          fSettings->fCaps &&
                                          !fSettings->fCaps->gsInvocationsSupport();
        SkASSERT(!fExtraVars.size());
        IRNode::ID body = this->convertBlock(*iter);
        for (IRNode::ID v : fExtraVars) {
            Block& block = (Block&) body.node();
            block.fStatements.insert(block.fStatements.begin(), v);
        }
        fExtraVars.clear();
        fCurrentFunction = IRNode::ID();
        if (!body) {
            return;
        }
        if (needInvocationIDWorkaround) {
            body = this->applyInvocationIDWorkaround(std::move(body));
        }
        // conservatively assume all user-defined functions have side effects
        ((FunctionDeclaration&) decl.node()).fModifiers.fFlags |= Modifiers::kHasSideEffects_Flag;
        if (Program::kVertex_Kind == fKind && fd.fName == "main" && fRTAdjust) {
            Block& block = (Block&) body.node();
            block.fStatements.insert(block.fStatements.end(), this->getNormalizeSkPositionCode());
        }
        fProgramElements->push_back(this->createNode(new FunctionDefinition(this, f.fOffset, decl,
                                                                            body)));
    }
}

IRNode::ID IRGenerator::convertInterfaceBlock(const ASTNode& intf) {
    SkASSERT(intf.fKind == ASTNode::Kind::kInterfaceBlock);
    ASTNode::InterfaceBlockData id = intf.getInterfaceBlockData();
    std::shared_ptr<SymbolTable> old = fSymbolTable;
    this->pushSymbolTable();
    std::shared_ptr<SymbolTable> symbols = fSymbolTable;
    std::vector<Type::Field> fields;
    bool haveRuntimeArray = false;
    bool foundRTAdjust = false;
    auto iter = intf.begin();
    for (size_t i = 0; i < id.fDeclarationCount; ++i) {
        IRNode::ID decl = this->convertVarDeclarations(*(iter++),
                                                       Variable::kInterfaceBlock_Storage);
        if (!decl) {
            return IRNode::ID();
        }
        for (IRNode::ID stmt : ((VarDeclarations&) decl.node()).fVars) {
            VarDeclaration& vd = (VarDeclaration&) stmt.node();
            if (haveRuntimeArray) {
                fErrors.error(decl.node().fOffset,
                              "only the last entry in an interface block may be a runtime-sized "
                              "array");
            }
            Variable& var = (Variable&) vd.fVar.node();
            if (vd.fVar == fRTAdjust) {
                foundRTAdjust = true;
                SkASSERT(var.fType == fContext.fFloat4_Type);
                fRTAdjustFieldIndex = fields.size();
            }
            fields.push_back(Type::Field(var.fModifiers, var.fName, var.fType));
            if (vd.fValue) {
                fErrors.error(decl.node().fOffset,
                              "initializers are not permitted on interface block fields");
            }
            if (var.fModifiers.fFlags & (Modifiers::kIn_Flag |
                                         Modifiers::kOut_Flag |
                                         Modifiers::kUniform_Flag |
                                         Modifiers::kBuffer_Flag |
                                         Modifiers::kConst_Flag)) {
                fErrors.error(decl.node().fOffset,
                              "interface block fields may not have storage qualifiers");
            }
            const Type& varType = var.fType.typeNode();
            if (varType.kind() == Type::kArray_Kind && varType.columns() == -1) {
                haveRuntimeArray = true;
            }
        }
    }
    this->popSymbolTable();
    IRNode::ID type = this->createNode(new Type(this, intf.fOffset, id.fTypeName, fields));
    std::vector<IRNode::ID> sizes;
    for (size_t i = 0; i < id.fSizeCount; ++i) {
        const ASTNode& size = *(iter++);
        if (size) {
            IRNode::ID converted = this->convertExpression(size);
            if (!converted) {
                return IRNode::ID();
            }
            String name = type.typeNode().fName;
            int64_t count;
            if (converted.expressionNode().fKind == Expression::kIntLiteral_Kind) {
                count = ((IntLiteral&) converted.node()).fValue;
                if (count <= 0) {
                    fErrors.error(converted.node().fOffset, "array size must be positive");
                }
                name += "[" + to_string(count) + "]";
            } else {
                count = -1;
                name += "[]";
            }
            type = this->createNode(new Type(this, name, Type::kArray_Kind, type, (int) count));
            sizes.push_back(converted);
        } else {
            type = this->createNode(new Type(this, type.typeNode().name() + "[]", Type::kArray_Kind,
                                             type, -1));
            sizes.push_back(IRNode::ID());
        }
    }
    IRNode::ID var = this->createNode(new Variable(
                                         this,
                                         intf.fOffset,
                                         id.fModifiers,
                                         id.fInstanceName.fLength ? id.fInstanceName : id.fTypeName,
                                         type,
                                         Variable::kGlobal_Storage,
                                         IRNode::ID()));
    if (foundRTAdjust) {
        fRTAdjustInterfaceBlock = var;
    }
    if (id.fInstanceName.fLength) {
        old->add(id.fInstanceName, var);
    } else {
        for (size_t i = 0; i < fields.size(); i++) {
            old->add(fields[i].fName, this->createNode(new Field(intf.fOffset, var, (int) i)));
        }
    }
    return this->createNode(new InterfaceBlock(this,
                                               intf.fOffset,
                                               var,
                                               id.fTypeName,
                                               id.fInstanceName,
                                               std::move(sizes),
                                               symbols));
}

void IRGenerator::getConstantInt(const Expression& value, int64_t* out) {
    switch (value.fKind) {
        case Expression::kIntLiteral_Kind:
            *out = ((const IntLiteral&) value).fValue;
            break;
        case Expression::kVariableReference_Kind: {
            Variable& var = (Variable&) ((VariableReference&) value).fVariable.node();
            if ((var.fModifiers.fFlags & Modifiers::kConst_Flag) &&
                var.fInitialValue) {
                this->getConstantInt(var.fInitialValue.expressionNode(), out);
            }
            break;
        }
        default:
            fErrors.error(value.fOffset, "expected a constant int");
    }
}

void IRGenerator::convertEnum(const ASTNode& e) {
    SkASSERT(e.fKind == ASTNode::Kind::kEnum);
    std::vector<IRNode::ID> variables;
    int64_t currentValue = 0;
    Layout layout;
    ASTNode enumType(e.fNodes, e.fOffset, ASTNode::Kind::kType,
                     ASTNode::TypeData(e.getString(), false, false));
    IRNode::ID type = this->convertType(enumType);
    Modifiers modifiers(layout, Modifiers::kConst_Flag);
    std::shared_ptr<SymbolTable> symbols(new SymbolTable(fSymbolTable, this));
    fSymbolTable = symbols;
    for (auto iter = e.begin(); iter != e.end(); ++iter) {
        const ASTNode& child = *iter;
        SkASSERT(child.fKind == ASTNode::Kind::kEnumCase);
        IRNode::ID value;
        if (child.begin() != child.end()) {
            value = this->convertExpression(*child.begin());
            if (!value) {
                fSymbolTable = symbols->fParent;
                return;
            }
            this->getConstantInt(value.expressionNode(), &currentValue);
        }
        value = this->createNode(new IntLiteral(this, e.fOffset, currentValue));
        ++currentValue;
        IRNode::ID var = this->createNode(new Variable(this, e.fOffset, modifiers,
                                                       child.getString(), type,
                                                       Variable::kGlobal_Storage, value));
        variables.push_back(var);
        symbols->add(child.getString(), var);
    }
    fProgramElements->push_back(this->createNode(new Enum(this, e.fOffset, e.getString(),
                                                          symbols)));
    fSymbolTable = symbols->fParent;
}

IRNode::ID IRGenerator::convertType(const ASTNode& type) {
    ASTNode::TypeData td = type.getTypeData();
    IRNode::ID result = (*fSymbolTable)[td.fName];
    if (result && ((Symbol&) result.node()).fSymbolKind == Symbol::kType_Kind) {
        if (td.fIsNullable) {
            if (result == fContext.fFragmentProcessor_Type) {
                if (type.begin() != type.end()) {
                    fErrors.error(type.fOffset, "type '" + td.fName + "' may not be used in "
                                                "an array");
                }
                result = this->createNode(new Type(this, String(result.typeNode().fName) + "?",
                                                   Type::kNullable_Kind,
                                                   result));
            } else {
                fErrors.error(type.fOffset, "type '" + td.fName + "' may not be nullable");
            }
        }
        for (const auto& size : type) {
            String name(result.typeNode().fName);
            name += "[";
            if (size) {
                name += to_string(size.getInt());
            }
            name += "]";
            result = this->createNode(new Type(this, name, Type::kArray_Kind, result,
                                               size ? size.getInt() : 0));
        }
        return result;
    }
    fErrors.error(type.fOffset, "unknown type '" + td.fName + "'");
    return IRNode::ID();
}

IRNode::ID IRGenerator::convertExpression(const ASTNode& expr) {
    switch (expr.fKind) {
        case ASTNode::Kind::kBinary:
            return this->convertBinaryExpression(expr);
        case ASTNode::Kind::kBool:
            return this->createNode(new BoolLiteral(this, expr.fOffset, expr.getBool()));
        case ASTNode::Kind::kCall:
            return this->convertCallExpression(expr);
        case ASTNode::Kind::kField:
            return this->convertFieldExpression(expr);
        case ASTNode::Kind::kFloat:
            return this->createNode(new FloatLiteral(this, expr.fOffset, expr.getFloat()));
        case ASTNode::Kind::kIdentifier:
            return this->convertIdentifier(expr);
        case ASTNode::Kind::kIndex:
            return this->convertIndexExpression(expr);
        case ASTNode::Kind::kInt:
            return this->createNode(new IntLiteral(this, expr.fOffset, expr.getInt()));
        case ASTNode::Kind::kNull:
            return this->createNode(new NullLiteral(this, expr.fOffset));
        case ASTNode::Kind::kPostfix:
            return this->convertPostfixExpression(expr);
        case ASTNode::Kind::kPrefix:
            return this->convertPrefixExpression(expr);
        case ASTNode::Kind::kTernary:
            return this->convertTernaryExpression(expr);
        default:
            ABORT("unsupported expression: %s\n", expr.description().c_str());
    }
}

IRNode::ID IRGenerator::convertIdentifier(const ASTNode& identifier) {
    SkASSERT(identifier.fKind == ASTNode::Kind::kIdentifier);
    IRNode::ID result = (*fSymbolTable)[identifier.getString()];
    if (!result) {
        fErrors.error(identifier.fOffset, "unknown identifier '" + identifier.getString() + "'");
        return IRNode::ID();
    }
    switch (((Symbol&) result.node()).fSymbolKind) {
        case Symbol::kFunctionDeclaration_Kind: {
            std::vector<IRNode::ID> f = { result };
            return this->createNode(new FunctionReference(this, identifier.fOffset, f));
        }
        case Symbol::kUnresolvedFunction_Kind: {
            const UnresolvedFunction& f = (const UnresolvedFunction&) result.node();
            return this->createNode(new FunctionReference(this, identifier.fOffset, f.fFunctions));
        }
        case Symbol::kVariable_Kind: {
            const Variable& var = (const Variable&) result.node();
            switch (var.fModifiers.fLayout.fBuiltin) {
                case SK_WIDTH_BUILTIN:
                    fInputs.fRTWidth = true;
                    break;
                case SK_HEIGHT_BUILTIN:
                    fInputs.fRTHeight = true;
                    break;
#ifndef SKSL_STANDALONE
                case SK_FRAGCOORD_BUILTIN:
                    if (var.fModifiers.fLayout.fBuiltin == SK_FRAGCOORD_BUILTIN) {
                        fInputs.fFlipY = true;
                        if (fSettings->fFlipY &&
                            (!fSettings->fCaps ||
                             !fSettings->fCaps->fragCoordConventionsExtensionString())) {
                            fInputs.fRTHeight = true;
                        }
                    }
#endif
            }
            // default to kRead_RefKind; this will be corrected later if the variable is written to
            return this->createNode(new VariableReference(this, identifier.fOffset, result,
                                                          kRead_RefKind));
        }
        case Symbol::kField_Kind: {
            const Field& field = (const Field&) result.node();
            IRNode::ID base = this->createNode(new VariableReference(this, identifier.fOffset,
                                                                     field.fOwner, kRead_RefKind));
            return this->createNode(new FieldAccess(
                                                  this,
                                                  base,
                                                  field.fFieldIndex,
                                                  FieldAccess::kAnonymousInterfaceBlock_OwnerKind));
        }
        case Symbol::kType_Kind: {
            return this->createNode(new TypeReference(this, identifier.fOffset, result));
        }
        case Symbol::kExternal_Kind: {
            return this->createNode(new ExternalValueReference(this, identifier.fOffset,
                                                   &((ExternalValueSymbol&) result.node()).fValue));
        }
        default:
            ABORT("unsupported symbol type %d\n", ((Symbol&) result.node()).fSymbolKind);
    }
}

IRNode::ID IRGenerator::convertSection(const ASTNode& s) {
    ASTNode::SectionData section = s.getSectionData();
    return this->createNode(new Section(this, s.fOffset, section.fName, section.fArgument,
                                        section.fText));
}

IRNode::ID IRGenerator::coerce(IRNode::ID exprID, IRNode::ID type) {
    if (!exprID) {
        return IRNode::ID();
    }
    Expression& expr = exprID.expressionNode();
    if (expr.fType == type) {
        return exprID;
    }
    this->checkValid(expr);
    if (!expr.fType) {
        return IRNode::ID();
    }
    if (expr.coercionCost(type.typeNode()) == INT_MAX) {
        fErrors.error(expr.fOffset, "expected '" + type.node().description() + "', but found '" +
                                        expr.fType.node().description() + "'");
        return IRNode::ID();
    }
    if (type.typeNode().kind() == Type::kScalar_Kind) {
        std::vector<IRNode::ID> args;
        args.push_back(exprID);
        IRNode::ID ctor;
        if (type == fContext.fFloatLiteral_Type) {
            ctor = this->convertIdentifier(ASTNode(&fFile->fNodes, -1, ASTNode::Kind::kIdentifier,
                                                   "float"));
        } else if (type == fContext.fIntLiteral_Type) {
            ctor = this->convertIdentifier(ASTNode(&fFile->fNodes, -1, ASTNode::Kind::kIdentifier,
                                                   "int"));
        } else {
            ctor = this->convertIdentifier(ASTNode(&fFile->fNodes, -1, ASTNode::Kind::kIdentifier,
                                                   type.typeNode().fName));
        }
        if (!ctor) {
            printf("error, null identifier: %s\n", String(type.typeNode().fName).c_str());
        }
        SkASSERT(ctor);
        return this->callExpression(-1, ctor, std::move(args));
    }
    if (expr.fKind == Expression::kNullLiteral_Kind) {
        SkASSERT(type.typeNode().kind() == Type::kNullable_Kind);
        return this->createNode(new NullLiteral(this, expr.fOffset, type));
    }
    return this->createNode(new Constructor(this, -1, type, { exprID }));
}

static bool is_matrix_multiply(const Type& left, const Type& right) {
    if (left.kind() == Type::kMatrix_Kind) {
        return right.kind() == Type::kMatrix_Kind || right.kind() == Type::kVector_Kind;
    }
    return left.kind() == Type::kVector_Kind && right.kind() == Type::kMatrix_Kind;
}

/**
 * Determines the operand and result types of a binary expression. Returns true if the expression is
 * legal, false otherwise. If false, the values of the out parameters are undefined.
 */
static bool determine_binary_type(const Context& context,
                                  Token::Kind op,
                                  IRNode::ID leftID,
                                  IRNode::ID rightID,
                                  IRNode::ID* outLeftType,
                                  IRNode::ID* outRightType,
                                  IRNode::ID* outResultType,
                                  bool tryFlipped) {
    Type& left = leftID.typeNode();
    Type& right = rightID.typeNode();
    bool isLogical;
    bool validMatrixOrVectorOp;
    switch (op) {
        case Token::EQ:
            *outLeftType = leftID;
            *outRightType = leftID;
            *outResultType = leftID;
            return right.canCoerceTo(left);
        case Token::EQEQ: // fall through
        case Token::NEQ:
            if (right.canCoerceTo(left)) {
                *outLeftType = leftID;
                *outRightType = leftID;
                *outResultType = context.fBool_Type;
                return true;
            } if (left.canCoerceTo(right)) {
                *outLeftType = rightID;
                *outRightType = rightID;
                *outResultType = context.fBool_Type;
                return true;
            }
            return false;
        case Token::LT:   // fall through
        case Token::GT:   // fall through
        case Token::LTEQ: // fall through
        case Token::GTEQ:
            isLogical = true;
            validMatrixOrVectorOp = false;
            break;
        case Token::LOGICALOR: // fall through
        case Token::LOGICALAND: // fall through
        case Token::LOGICALXOR: // fall through
        case Token::LOGICALOREQ: // fall through
        case Token::LOGICALANDEQ: // fall through
        case Token::LOGICALXOREQ:
            *outLeftType = context.fBool_Type;
            *outRightType = context.fBool_Type;
            *outResultType = context.fBool_Type;
            return left.canCoerceTo(context.fBool_Type.typeNode()) &&
                   right.canCoerceTo(context.fBool_Type.typeNode());
        case Token::STAREQ:
            if (left.kind() == Type::kScalar_Kind) {
                *outLeftType = leftID;
                *outRightType = leftID;
                *outResultType = leftID;
                return right.canCoerceTo(left);
            }
            // fall through
        case Token::STAR:
            if (is_matrix_multiply(left, right)) {
                // determine final component type
                if (determine_binary_type(context, Token::STAR, left.componentType(),
                                          right.componentType(), outLeftType, outRightType,
                                          outResultType, false)) {
                    *outLeftType = outResultType->typeNode().toCompound(left.columns(),
                                                                        left.rows());
                    *outRightType = outResultType->typeNode().toCompound(right.columns(),
                                                                         right.rows());
                    int leftColumns = left.columns();
                    int leftRows = left.rows();
                    int rightColumns;
                    int rightRows;
                    if (right.kind() == Type::kVector_Kind) {
                        // matrix * vector treats the vector as a column vector, so we need to
                        // transpose it
                        rightColumns = right.rows();
                        rightRows = right.columns();
                        SkASSERT(rightColumns == 1);
                    } else {
                        rightColumns = right.columns();
                        rightRows = right.rows();
                    }
                    if (rightColumns > 1) {
                        *outResultType = outResultType->typeNode().toCompound(rightColumns,
                                                                              leftRows);
                    } else {
                        // result was a column vector, transpose it back to a row
                        *outResultType = outResultType->typeNode().toCompound(leftRows,
                                                                              rightColumns);
                    }
                    return leftColumns == rightRows;
                } else {
                    return false;
                }
            }
            isLogical = false;
            validMatrixOrVectorOp = true;
            break;
        case Token::PLUSEQ:
        case Token::MINUSEQ:
        case Token::SLASHEQ:
        case Token::PERCENTEQ:
        case Token::SHLEQ:
        case Token::SHREQ:
            if (left.kind() == Type::kScalar_Kind) {
                *outLeftType = leftID;
                *outRightType = leftID;
                *outResultType = leftID;
                return right.canCoerceTo(left);
            }
            // fall through
        case Token::PLUS:    // fall through
        case Token::MINUS:   // fall through
        case Token::SLASH:   // fall through
            isLogical = false;
            validMatrixOrVectorOp = true;
            break;
        case Token::COMMA:
            *outLeftType = leftID;
            *outRightType = rightID;
            *outResultType = rightID;
            return true;
        default:
            isLogical = false;
            validMatrixOrVectorOp = false;
    }
    bool isVectorOrMatrix = left.kind() == Type::kVector_Kind || left.kind() == Type::kMatrix_Kind;
    if (left.kind() == Type::kScalar_Kind && right.kind() == Type::kScalar_Kind &&
            right.canCoerceTo(left)) {
        if (left.priority() > right.priority()) {
            *outLeftType = leftID;
            *outRightType = leftID;
        } else {
            *outLeftType = rightID;
            *outRightType = rightID;
        }
        if (isLogical) {
            *outResultType = context.fBool_Type;
        } else {
            *outResultType = leftID;
        }
        return true;
    }
    if (right.canCoerceTo(left) && isVectorOrMatrix && validMatrixOrVectorOp) {
        *outLeftType = leftID;
        *outRightType = leftID;
        if (isLogical) {
            *outResultType = context.fBool_Type;
        } else {
            *outResultType = leftID;
        }
        return true;
    }
    if ((left.kind() == Type::kVector_Kind || left.kind() == Type::kMatrix_Kind) &&
        (right.kind() == Type::kScalar_Kind)) {
        if (determine_binary_type(context, op, left.componentType(), rightID, outLeftType,
                                  outRightType, outResultType, false)) {
            *outLeftType = outLeftType->typeNode().toCompound(left.columns(), left.rows());
            if (!isLogical) {
                *outResultType = outResultType->typeNode().toCompound(left.columns(), left.rows());
            }
            return true;
        }
        return false;
    }
    if (tryFlipped) {
        return determine_binary_type(context, op, rightID, leftID, outRightType, outLeftType,
                                     outResultType, false);
    }
    return false;
}

IRNode::ID IRGenerator::shortCircuitBoolean(IRNode::ID leftID, Token::Kind op, IRNode::ID rightID) {
    Expression& left = leftID.expressionNode();
    Expression& right = rightID.expressionNode();
    SkASSERT(left.fKind == Expression::kBoolLiteral_Kind);
    bool leftVal = ((BoolLiteral&) left).fValue;
    if (op == Token::LOGICALAND) {
        // (true && expr) -> (expr) and (false && expr) -> (false)
        return leftVal ? right.clone()
                       : this->createNode(new BoolLiteral(this, left.fOffset, false));
    } else if (op == Token::LOGICALOR) {
    // (true || expr) -> (true) and (false || expr) -> (expr)
        return leftVal ? this->createNode(new BoolLiteral(this, left.fOffset, true))
                       : right.clone();
    } else {
        // Can't short circuit XOR
        return IRNode::ID();
    }
}

IRNode::ID IRGenerator::constantFold(IRNode::ID leftID,
                                     Token::Kind op,
                                     IRNode::ID rightID) {
    Expression& left = leftID.expressionNode();
    Expression& right = rightID.expressionNode();
    // If the left side is a constant boolean literal, the right side does not need to be constant
    // for short circuit optimizations to allow the constant to be folded.
    if (left.fKind == Expression::kBoolLiteral_Kind && !right.isConstant()) {
        return this->shortCircuitBoolean(leftID, op, rightID);
    } else if (right.fKind == Expression::kBoolLiteral_Kind && !left.isConstant()) {
        // There aren't side effects in SKSL within expressions, so (left OP right) is equivalent to
        // (right OP left) for short-circuit optimizations
        return this->shortCircuitBoolean(rightID, op, leftID);
    }

    // Other than the short-circuit cases above, constant folding requires both sides to be constant
    if (!left.isConstant() || !right.isConstant()) {
        return IRNode::ID();
    }
    // Note that we expressly do not worry about precision and overflow here -- we use the maximum
    // precision to calculate the results and hope the result makes sense. The plan is to move the
    // Skia caps into SkSL, so we have access to all of them including the precisions of the various
    // types, which will let us be more intelligent about this.
    if (left.fKind == Expression::kBoolLiteral_Kind &&
        right.fKind == Expression::kBoolLiteral_Kind) {
        bool leftVal  = ((BoolLiteral&) left).fValue;
        bool rightVal = ((BoolLiteral&) right).fValue;
        bool result;
        switch (op) {
            case Token::LOGICALAND: result = leftVal && rightVal; break;
            case Token::LOGICALOR:  result = leftVal || rightVal; break;
            case Token::LOGICALXOR: result = leftVal ^  rightVal; break;
            default: return IRNode::ID();
        }
        return this->createNode(new BoolLiteral(this, left.fOffset, result));
    }
    #define RESULT(t, op) this->createNode(new t ## Literal(this, left.fOffset, \
                                                            leftVal op rightVal))
    if (left.fKind == Expression::kIntLiteral_Kind && right.fKind == Expression::kIntLiteral_Kind) {
        int64_t leftVal  = ((IntLiteral&) left).fValue;
        int64_t rightVal = ((IntLiteral&) right).fValue;
        switch (op) {
            case Token::PLUS:       return RESULT(Int, +);
            case Token::MINUS:      return RESULT(Int, -);
            case Token::STAR:       return RESULT(Int, *);
            case Token::SLASH:
                if (rightVal) {
                    return RESULT(Int, /);
                }
                fErrors.error(right.fOffset, "division by zero");
                return IRNode::ID();
            case Token::PERCENT:
                if (rightVal) {
                    return RESULT(Int, %);
                }
                fErrors.error(right.fOffset, "division by zero");
                return IRNode::ID();
            case Token::BITWISEAND: return RESULT(Int,  &);
            case Token::BITWISEOR:  return RESULT(Int,  |);
            case Token::BITWISEXOR: return RESULT(Int,  ^);
            case Token::EQEQ:       return RESULT(Bool, ==);
            case Token::NEQ:        return RESULT(Bool, !=);
            case Token::GT:         return RESULT(Bool, >);
            case Token::GTEQ:       return RESULT(Bool, >=);
            case Token::LT:         return RESULT(Bool, <);
            case Token::LTEQ:       return RESULT(Bool, <=);
            case Token::SHL:
                if (rightVal >= 0 && rightVal <= 31) {
                    return RESULT(Int,  <<);
                }
                fErrors.error(right.fOffset, "shift value out of range");
                return IRNode::ID();
            case Token::SHR:
                if (rightVal >= 0 && rightVal <= 31) {
                    return RESULT(Int,  >>);
                }
                fErrors.error(right.fOffset, "shift value out of range");
                return IRNode::ID();

            default:
                return IRNode::ID();
        }
    }
    if (left.fKind == Expression::kFloatLiteral_Kind &&
        right.fKind == Expression::kFloatLiteral_Kind) {
        double leftVal  = ((FloatLiteral&) left).fValue;
        double rightVal = ((FloatLiteral&) right).fValue;
        switch (op) {
            case Token::PLUS:       return RESULT(Float, +);
            case Token::MINUS:      return RESULT(Float, -);
            case Token::STAR:       return RESULT(Float, *);
            case Token::SLASH:
                if (rightVal) {
                    return RESULT(Float, /);
                }
                fErrors.error(right.fOffset, "division by zero");
                return IRNode::ID();
            case Token::EQEQ:       return RESULT(Bool, ==);
            case Token::NEQ:        return RESULT(Bool, !=);
            case Token::GT:         return RESULT(Bool, >);
            case Token::GTEQ:       return RESULT(Bool, >=);
            case Token::LT:         return RESULT(Bool, <);
            case Token::LTEQ:       return RESULT(Bool, <=);
            default:                return IRNode::ID();
        }
    }
    if (left.fType.typeNode().kind() == Type::kVector_Kind &&
        left.fType.typeNode().componentType().typeNode().isFloat() &&
        left.fType == right.fType) {
        std::vector<IRNode::ID> args;
        #define RETURN_VEC_COMPONENTWISE_RESULT(op)                                         \
            for (int i = 0; i < left.fType.typeNode().columns(); i++) {                     \
                float value = left.getFVecComponent(i) op                                   \
                              right.getFVecComponent(i);                                    \
                args.emplace_back(new FloatLiteral(this, -1, value));                       \
            }                                                                               \
            return this->createNode(new Constructor(this, -1, left.fType, std::move(args)))
        switch (op) {
            case Token::EQEQ:
                return this->createNode(new BoolLiteral(this, -1, left.compareConstant(right)));
            case Token::NEQ:
                return this->createNode(new BoolLiteral(this, -1, !left.compareConstant(right)));
            case Token::PLUS:  RETURN_VEC_COMPONENTWISE_RESULT(+);
            case Token::MINUS: RETURN_VEC_COMPONENTWISE_RESULT(-);
            case Token::STAR:  RETURN_VEC_COMPONENTWISE_RESULT(*);
            case Token::SLASH:
                for (int i = 0; i < left.fType.typeNode().columns(); i++) {
                    SKSL_FLOAT rvalue = right.getFVecComponent(i);
                    if (rvalue == 0.0) {
                        fErrors.error(right.fOffset, "division by zero");
                        return IRNode::ID();
                    }
                    float value = left.getFVecComponent(i) / rvalue;
                    args.emplace_back(new FloatLiteral(this, -1, value));
                }
                return this->createNode(new Constructor(this, -1, left.fType, std::move(args)));
            default: return IRNode::ID();
        }
    }
    if (left.fType.typeNode().kind() == Type::kMatrix_Kind &&
        right.fType.typeNode().kind() == Type::kMatrix_Kind &&
        left.fKind == right.fKind) {
        switch (op) {
            case Token::EQEQ:
                return this->createNode(new BoolLiteral(this, -1, left.compareConstant(right)));
            case Token::NEQ:
                return this->createNode(new BoolLiteral(this, -1, !left.compareConstant(right)));
            default:
                return IRNode::ID();
        }
    }
    #undef RESULT
    return IRNode::ID();
}

IRNode::ID IRGenerator::convertBinaryExpression(const ASTNode& expression) {
    SkASSERT(expression.fKind == ASTNode::Kind::kBinary);
    auto iter = expression.begin();
    IRNode::ID leftID = this->convertExpression(*(iter++));
    if (!leftID) {
        return IRNode::ID();
    }
    IRNode::ID rightID = this->convertExpression(*(iter++));
    if (!rightID) {
        return IRNode::ID();
    }
    Expression& left = leftID.expressionNode();
    Expression& right = rightID.expressionNode();
    IRNode::ID leftType;
    IRNode::ID rightType;
    IRNode::ID resultType;
    IRNode::ID rawLeftType;
    if (left.fKind == Expression::kIntLiteral_Kind && right.fType.typeNode().isInteger()) {
        rawLeftType = right.fType;
    } else {
        rawLeftType = left.fType;
    }
    IRNode::ID rawRightType;
    if (right.fKind == Expression::kIntLiteral_Kind && left.fType.typeNode().isInteger()) {
        rawRightType = left.fType;
    } else {
        rawRightType = right.fType;
    }
    Token::Kind op = expression.getToken().fKind;
    if (!determine_binary_type(fContext, op, rawLeftType, rawRightType, &leftType, &rightType,
                               &resultType, !Compiler::IsAssignment(op))) {
        fErrors.error(expression.fOffset, String("type mismatch: '") +
                                          Compiler::OperatorName(expression.getToken().fKind) +
                                          "' cannot operate on '" +
                                          left.fType.node().description() +
                                          "', '" + right.fType.node().description() + "'");
        return IRNode::ID();
    }
    if (Compiler::IsAssignment(op)) {
        this->setRefKind(left, op != Token::EQ ? kReadWrite_RefKind : kWrite_RefKind);
    }
    leftID = this->coerce(leftID, leftType);
    rightID = this->coerce(rightID, rightType);
    if (!leftID || !rightID) {
        return IRNode::ID();
    }
    IRNode::ID result = this->constantFold(leftID, op, rightID);
    if (!result) {
        result = this->createNode(new BinaryExpression(this,
                                                       expression.fOffset,
                                                       leftID,
                                                       op,
                                                       rightID,
                                                       resultType));
    }
    return result;
}

void IRGenerator::destroy(IRNode::ID id) {
    switch (id.expressionNode().fKind) {
        case Expression::kVariableReference_Kind: {
            VariableReference& vr = (VariableReference&) id.node();
            if (vr.fRefKind != kRead_RefKind) {
                ((Variable&) vr.fVariable.node()).fWriteCount--;
            }
            if (vr.fRefKind != kWrite_RefKind) {
                ((Variable&) vr.fVariable.node()).fReadCount--;
            }
        }
        default:
            break;
    }
}

IRNode::ID IRGenerator::convertTernaryExpression(const ASTNode& node) {
    SkASSERT(node.fKind == ASTNode::Kind::kTernary);
    auto iter = node.begin();
    IRNode::ID test = this->coerce(this->convertExpression(*(iter++)), fContext.fBool_Type);
    if (!test) {
        return IRNode::ID();
    }
    IRNode::ID ifTrue = this->convertExpression(*(iter++));
    if (!ifTrue) {
        return IRNode::ID();
    }
    IRNode::ID ifFalse = this->convertExpression(*(iter++));
    if (!ifFalse) {
        return IRNode::ID();
    }
    IRNode::ID trueType;
    IRNode::ID falseType;
    IRNode::ID resultType;
    if (!determine_binary_type(fContext, Token::EQEQ, ifTrue.expressionNode().fType,
                               ifFalse.expressionNode().fType, &trueType, &falseType, &resultType,
                               true) || trueType != falseType) {
        fErrors.error(node.fOffset, "ternary operator result mismatch: '" +
                                    ifTrue.expressionNode().fType.node().description() + "', '" +
                                    ifFalse.expressionNode().fType.node().description() + "'");
        return IRNode::ID();
    }
    ifTrue = this->coerce(ifTrue, trueType);
    if (!ifTrue) {
        return IRNode::ID();
    }
    ifFalse = this->coerce(ifFalse, falseType);
    if (!ifFalse) {
        return IRNode::ID();
    }
    if (test.expressionNode().fKind == Expression::kBoolLiteral_Kind) {
        // static boolean test, just return one of the branches
        if (((BoolLiteral&) test.node()).fValue) {
            this->destroy(ifFalse);
            return ifTrue;
        } else {
            this->destroy(ifTrue);
            return ifFalse;
        }
    }
    return this->createNode(new TernaryExpression(this,
                                                  node.fOffset,
                                                  std::move(test),
                                                  std::move(ifTrue),
                                                  std::move(ifFalse)));
}

IRNode::ID IRGenerator::callFunction(int offset, IRNode::ID functionID,
                                     std::vector<IRNode::ID> arguments) {
    FunctionDeclaration& function = (FunctionDeclaration&) functionID.node();
    if (function.fParameters.size() != arguments.size()) {
        String msg = "call to '" + function.fName + "' expected " +
                                 to_string((uint64_t) function.fParameters.size()) +
                                 " argument";
        if (function.fParameters.size() != 1) {
            msg += "s";
        }
        msg += ", but found " + to_string((uint64_t) arguments.size());
        fErrors.error(offset, msg);
        return IRNode::ID();
    }
    std::vector<IRNode::ID> types;
    IRNode::ID returnType;
    if (!function.determineFinalTypes(arguments, &types, &returnType)) {
        String msg = "no match for " + function.fName + "(";
        String separator;
        for (size_t i = 0; i < arguments.size(); i++) {
            msg += separator;
            separator = ", ";
            msg += arguments[i].expressionNode().fType.node().description();
        }
        msg += ")";
        fErrors.error(offset, msg);
        return IRNode::ID();
    }
    for (size_t i = 0; i < arguments.size(); i++) {
        arguments[i] = this->coerce(arguments[i], types[i]);
        if (!arguments[i]) {
            return IRNode::ID();
        }
        Variable& param = (Variable&) function.fParameters[i].node();
        if (arguments[i] && (param.fModifiers.fFlags & Modifiers::kOut_Flag)) {
            this->setRefKind(arguments[i].expressionNode(),
                             param.fModifiers.fFlags & Modifiers::kIn_Flag ? kReadWrite_RefKind
                                                                           : kPointer_RefKind);
        }
    }
    return this->createNode(new FunctionCall(this, offset, returnType, functionID,
                                             std::move(arguments)));
}

/**
 * Determines the cost of coercing the arguments of a function to the required types. Cost has no
 * particular meaning other than "lower costs are preferred". Returns INT_MAX if the call is not
 * valid.
 */
int IRGenerator::callCost(const FunctionDeclaration& function,
                          const std::vector<IRNode::ID>& arguments) {
    if (function.fParameters.size() != arguments.size()) {
        return INT_MAX;
    }
    int total = 0;
    std::vector<IRNode::ID> types;
    IRNode::ID ignored;
    if (!function.determineFinalTypes(arguments, &types, &ignored)) {
        return INT_MAX;
    }
    for (size_t i = 0; i < arguments.size(); i++) {
        int cost = arguments[i].expressionNode().coercionCost(types[i].typeNode());
        if (cost != INT_MAX) {
            total += cost;
        } else {
            return INT_MAX;
        }
    }
    return total;
}

IRNode::ID IRGenerator::callExpression(int offset, IRNode::ID functionValue,
                                       std::vector<IRNode::ID> arguments) {
    switch (functionValue.expressionNode().fKind) {
        case Expression::kTypeReference_Kind:
            return this->convertConstructor(offset,
                                            ((TypeReference&) functionValue.node()).fValue,
                                            std::move(arguments));
        case Expression::kExternalValue_Kind: {
            ExternalValue& v = ((ExternalValueReference&) functionValue.node()).fValue;
            if (!v.canCall()) {
                fErrors.error(offset, "this external value is not a function");
                return IRNode::ID();
            }
            int count = v.callParameterCount();
            if (count != (int) arguments.size()) {
                fErrors.error(offset, "external function expected " + to_string(count) +
                                      " arguments, but found " + to_string((int) arguments.size()));
                return IRNode::ID();
            }
            static constexpr int PARAMETER_MAX = 16;
            SkASSERT(count < PARAMETER_MAX);
            IRNode::ID types[PARAMETER_MAX];
            v.getCallParameterTypes(types);
            for (int i = 0; i < count; ++i) {
                arguments[i] = this->coerce(arguments[i], types[i]);
                if (!arguments[i]) {
                    return IRNode::ID();
                }
            }
            return IRNode::ID(new ExternalFunctionCall(this, offset, v.callReturnType(), &v,
                                                       std::move(arguments)));
        }
        case Expression::kFunctionReference_Kind: {
            FunctionReference& ref = (FunctionReference&) functionValue.node();
            int bestCost = INT_MAX;
            IRNode::ID best;
            if (ref.fFunctions.size() > 1) {
                for (IRNode::ID f : ref.fFunctions) {
                    int cost = this->callCost((FunctionDeclaration&) f.node(), arguments);
                    if (cost < bestCost) {
                        bestCost = cost;
                        best = f;
                    }
                }
                if (best) {
                    return this->callFunction(offset, best, std::move(arguments));
                }
                String msg = "no match for " +
                             ((FunctionDeclaration&) *ref.fFunctions[0].fNode).fName + "(";
                String separator;
                for (size_t i = 0; i < arguments.size(); i++) {
                    msg += separator;
                    separator = ", ";
                    msg += arguments[i].expressionNode().fType.node().description();
                }
                msg += ")";
                fErrors.error(offset, msg);
                return IRNode::ID();
            }
            return this->callFunction(offset, ref.fFunctions[0], std::move(arguments));
        }
        default:
            fErrors.error(offset, "'" + functionValue.node().description() + "' is not a function");
            return IRNode::ID();
    }
}

IRNode::ID IRGenerator::convertNumberConstructor(int offset, IRNode::ID typeID,
                                                 std::vector<IRNode::ID> args) {
    const Type& type = typeID.typeNode();
    SkASSERT(type.isNumber());
    if (args.size() != 1) {
        fErrors.error(offset, "invalid arguments to '" + type.description() +
                              "' constructor, (expected exactly 1 argument, but found " +
                              to_string((uint64_t) args.size()) + ")");
        return IRNode::ID();
    }
    if (typeID == args[0].expressionNode().fType) {
        return args[0];
    }
    if (type.isFloat() && args.size() == 1 &&
        args[0].expressionNode().fKind == Expression::kFloatLiteral_Kind) {
        double value = ((FloatLiteral&) args[0].node()).fValue;
        return this->createNode(new FloatLiteral(this, offset, value, typeID));
    }
    if (type.isFloat() && args.size() == 1 &&
        args[0].expressionNode().fKind == Expression::kIntLiteral_Kind) {
        int64_t value = ((IntLiteral&) args[0].node()).fValue;
        return this->createNode(new FloatLiteral(this, offset, (double) value, typeID));
    }
    if (args[0].expressionNode().fKind == Expression::kIntLiteral_Kind &&
        (type == fContext.fInt_Type || type == fContext.fUInt_Type)) {
        return this->createNode(new IntLiteral(this, offset, ((IntLiteral&) args[0].node()).fValue,
                                               typeID));
    }
    if (args[0].expressionNode().fType == fContext.fBool_Type) {
        IRNode::ID zero(new IntLiteral(this, offset, 0));
        IRNode::ID one(new IntLiteral(this, offset, 1));
        return this->createNode(new TernaryExpression(this, offset, args[0],
                                                      this->coerce(one, typeID),
                                                      this->coerce(zero, typeID)));
    }
    if (!args[0].expressionNode().fType.typeNode().isNumber()) {
        fErrors.error(offset, "invalid argument to '" + type.description() +
                              "' constructor (expected a number or bool, but found '" +
                              args[0].expressionNode().fType.node().description() + "')");
        return IRNode::ID();
    }
    return this->createNode(new Constructor(this, offset, typeID, std::move(args)));
}

int component_count(const Type& type) {
    switch (type.kind()) {
        case Type::kVector_Kind:
            return type.columns();
        case Type::kMatrix_Kind:
            return type.columns() * type.rows();
        default:
            return 1;
    }
}

IRNode::ID IRGenerator::convertCompoundConstructor(int offset, IRNode::ID type,
                                                   std::vector<IRNode::ID> args) {
    SkASSERT(type.typeNode().kind() == Type::kVector_Kind ||
             type.typeNode().kind() == Type::kMatrix_Kind);
    if (type.typeNode().kind() == Type::kMatrix_Kind && args.size() == 1 &&
        args[0].expressionNode().fType.typeNode().kind() == Type::kMatrix_Kind) {
        // matrix from matrix is always legal
        return this->createNode(new Constructor(this, offset, type, std::move(args)));
    }
    int actual = 0;
    int expected = type.typeNode().rows() * type.typeNode().columns();
    if (args.size() != 1 ||
        expected != component_count(args[0].expressionNode().fType.typeNode()) ||
        type.typeNode().componentType().typeNode().isNumber() !=
        args[0].expressionNode().fType.typeNode().componentType().typeNode().isNumber()) {
        for (size_t i = 0; i < args.size(); i++) {
            if (args[i].expressionNode().fType.typeNode().kind() == Type::kVector_Kind) {
                if (type.typeNode().componentType().typeNode().isNumber() !=
                    args[i].expressionNode().fType.typeNode().componentType().typeNode().isNumber()) {
                    fErrors.error(offset, "'" +
                                          args[i].expressionNode().fType.node().description() +
                                          "' is not a valid parameter to '" +
                                          type.node().description() +
                                          "' constructor");
                    return IRNode::ID();
                }
                actual += args[i].expressionNode().fType.typeNode().columns();
            } else if (args[i].expressionNode().fType.typeNode().kind() == Type::kScalar_Kind) {
                actual += 1;
                if (type.typeNode().kind() != Type::kScalar_Kind) {
                    args[i] = this->coerce(args[i], type.typeNode().componentType());
                    if (!args[i]) {
                        return IRNode::ID();
                    }
                }
            } else {
                fErrors.error(offset, "'" + args[i].expressionNode().fType.node().description() +
                                      "' is not a valid parameter to '" +
                                      type.node().description() + "' constructor");
                return IRNode::ID();
            }
        }
        if (actual != 1 && actual != expected) {
            fErrors.error(offset, "invalid arguments to '" + type.node().description() +
                                  "' constructor (expected " + to_string(expected) +
                                  " scalars, but found " + to_string(actual) + ")");
            return IRNode::ID();
        }
    }
    return this->createNode(new Constructor(this, offset, type, std::move(args)));
}

IRNode::ID IRGenerator::convertConstructor(int offset, IRNode::ID type,
                                           std::vector<IRNode::ID> args) {
    // FIXME: add support for structs
    Type::Kind kind = type.typeNode().kind();
    if (args.size() == 1 && args[0].expressionNode().fType == type) {
        // argument is already the right type, just return it
        return args[0];
    }
    if (type.typeNode().isNumber()) {
        return this->convertNumberConstructor(offset, type, std::move(args));
    } else if (kind == Type::kArray_Kind) {
        IRNode::ID base = type.typeNode().componentType();
        for (size_t i = 0; i < args.size(); i++) {
            args[i] = this->coerce(args[i], base);
            if (!args[i]) {
                return IRNode::ID();
            }
        }
        return this->createNode(new Constructor(this, offset, type, std::move(args)));
    } else if (kind == Type::kVector_Kind || kind == Type::kMatrix_Kind) {
        return this->convertCompoundConstructor(offset, type, std::move(args));
    } else {
        fErrors.error(offset, "cannot construct '" + type.node().description() + "'");
        return IRNode::ID();
    }
}

IRNode::ID IRGenerator::convertPrefixExpression(const ASTNode& expression) {
    SkASSERT(expression.fKind == ASTNode::Kind::kPrefix);
    IRNode::ID baseID = this->convertExpression(*expression.begin());
    if (!baseID) {
        return IRNode::ID();
    }
    Expression& base = baseID.expressionNode();
    switch (expression.getToken().fKind) {
        case Token::PLUS:
            if (!base.fType.typeNode().isNumber() &&
                base.fType.typeNode().kind() != Type::kVector_Kind &&
                base.fType != fContext.fFloatLiteral_Type) {
                fErrors.error(expression.fOffset,
                              "'+' cannot operate on '" + base.fType.node().description() + "'");
                return IRNode::ID();
            }
            return baseID;
        case Token::MINUS:
            if (base.fKind == Expression::kIntLiteral_Kind) {
                return this->createNode(new IntLiteral(this, base.fOffset,
                                                       -((IntLiteral&) base).fValue));
            }
            if (base.fKind == Expression::kFloatLiteral_Kind) {
                double value = -((FloatLiteral&) base).fValue;
                return this->createNode(new FloatLiteral(this, base.fOffset, value));
            }
            if (!base.fType.typeNode().isNumber() &&
                base.fType.typeNode().kind() != Type::kVector_Kind) {
                fErrors.error(expression.fOffset,
                              "'-' cannot operate on '" + base.fType.node().description() + "'");
                return IRNode::ID();
            }
            return this->createNode(new PrefixExpression(this, Token::MINUS, baseID));
        case Token::PLUSPLUS:
            if (!base.fType.typeNode().isNumber()) {
                fErrors.error(expression.fOffset,
                              String("'") + Compiler::OperatorName(expression.getToken().fKind) +
                              "' cannot operate on '" + base.fType.node().description() + "'");
                return IRNode::ID();
            }
            this->setRefKind(base, kReadWrite_RefKind);
            break;
        case Token::MINUSMINUS:
            if (!base.fType.typeNode().isNumber()) {
                fErrors.error(expression.fOffset,
                              String("'") + Compiler::OperatorName(expression.getToken().fKind) +
                              "' cannot operate on '" + base.fType.node().description() + "'");
                return IRNode::ID();
            }
            this->setRefKind(base, kReadWrite_RefKind);
            break;
        case Token::LOGICALNOT:
            if (base.fType != fContext.fBool_Type) {
                fErrors.error(expression.fOffset,
                              String("'") + Compiler::OperatorName(expression.getToken().fKind) +
                              "' cannot operate on '" + base.fType.node().description() + "'");
                return IRNode::ID();
            }
            if (base.fKind == Expression::kBoolLiteral_Kind) {
                return this->createNode(new BoolLiteral(this, base.fOffset,
                                                        !((BoolLiteral&) base).fValue));
            }
            break;
        case Token::BITWISENOT:
            if (base.fType != fContext.fInt_Type) {
                fErrors.error(expression.fOffset,
                              String("'") + Compiler::OperatorName(expression.getToken().fKind) +
                              "' cannot operate on '" + base.fType.node().description() + "'");
                return IRNode::ID();
            }
            break;
        default:
            ABORT("unsupported prefix operator\n");
    }
    return this->createNode(new PrefixExpression(this, expression.getToken().fKind, baseID));
}

IRNode::ID IRGenerator::convertIndex(IRNode::ID baseID, const ASTNode& index) {
    {
        Expression& base = baseID.expressionNode();
        if (base.fKind == Expression::kTypeReference_Kind) {
            if (index.fKind == ASTNode::Kind::kInt) {
                IRNode::ID oldType = ((TypeReference&) base).fValue;
                SKSL_INT size = index.getInt();
                IRNode::ID newType = this->createNode(new Type(this, oldType.typeNode().name() +
                                                               "[" + to_string(size) + "]",
                                                               Type::kArray_Kind, oldType, size));
                return this->createNode(new TypeReference(this, base.fOffset, newType));

            } else {
                fErrors.error(base.fOffset, "array size must be a constant");
                return IRNode::ID();
            }
        }
        switch (base.fType.typeNode().kind()) {
            case Type::kArray_Kind:
            case Type::kMatrix_Kind:
            case Type::kVector_Kind:
                break;
            default:
                fErrors.error(base.fOffset, "expected array, but found '" +
                                            base.fType.node().description() + "'");
                return IRNode::ID();
        }
    }
    IRNode::ID converted = this->convertExpression(index);
    if (!converted) {
        return IRNode::ID();
    }
    if (converted.expressionNode().fType != fContext.fUInt_Type) {
        converted = this->coerce(converted, fContext.fInt_Type);
        if (!converted) {
            return IRNode::ID();
        }
    }
    return this->createNode(new IndexExpression(this, baseID, converted));
}

IRNode::ID IRGenerator::convertField(IRNode::ID baseID, StringFragment field) {
    Expression& base = baseID.expressionNode();
    if (base.fKind == Expression::kExternalValue_Kind) {
        ExternalValue& ev = ((ExternalValueReference&) base).fValue;
        ExternalValue* result = ev.getChild(String(field).c_str());
        if (!result) {
            fErrors.error(base.fOffset, "external value does not have a child named '" + field +
                                        "'");
            return IRNode::ID();
        }
        return this->createNode(new ExternalValueReference(this, base.fOffset, result));
    }
    auto fields = base.fType.typeNode().fields();
    for (size_t i = 0; i < fields.size(); i++) {
        if (fields[i].fName == field) {
            return this->createNode(new FieldAccess(this, baseID, (int) i));
        }
    }
    fErrors.error(base.fOffset, "type '" + base.fType.node().description() + "' does not have a "
                                "field named '" + field + "");
    return IRNode::ID();
}

IRNode::ID IRGenerator::convertSwizzle(IRNode::ID baseID, StringFragment fields) {
    Expression& base = baseID.expressionNode();
    if (base.fType.typeNode().kind() != Type::kVector_Kind) {
        fErrors.error(base.fOffset, "cannot swizzle type '" + base.fType.node().description() +
                                    "'");
        return IRNode::ID();
    }
    std::vector<int> swizzleComponents;
    for (size_t i = 0; i < fields.fLength; i++) {
        switch (fields[i]) {
            case '0':
                if (i != fields.fLength - 1) {
                    fErrors.error(base.fOffset,
                                  "only the last swizzle component can be a constant");
                }
                swizzleComponents.push_back(SKSL_SWIZZLE_0);
                break;
            case '1':
                if (i != fields.fLength - 1) {
                    fErrors.error(base.fOffset,
                                  "only the last swizzle component can be a constant");
                }
                swizzleComponents.push_back(SKSL_SWIZZLE_1);
                break;
            case 'x': // fall through
            case 'r': // fall through
            case 's':
                swizzleComponents.push_back(0);
                break;
            case 'y': // fall through
            case 'g': // fall through
            case 't':
                if (base.fType.typeNode().columns() >= 2) {
                    swizzleComponents.push_back(1);
                    break;
                }
                // fall through
            case 'z': // fall through
            case 'b': // fall through
            case 'p':
                if (base.fType.typeNode().columns() >= 3) {
                    swizzleComponents.push_back(2);
                    break;
                }
                // fall through
            case 'w': // fall through
            case 'a': // fall through
            case 'q':
                if (base.fType.typeNode().columns() >= 4) {
                    swizzleComponents.push_back(3);
                    break;
                }
                // fall through
            default:
                fErrors.error(base.fOffset, String::printf("invalid swizzle component '%c'",
                                                            fields[i]));
                return IRNode::ID();
        }
    }
    SkASSERT(swizzleComponents.size() > 0);
    if (swizzleComponents.size() > 4) {
        fErrors.error(base.fOffset, "too many components in swizzle mask '" + fields + "'");
        return IRNode::ID();
    }
    return this->createNode(new Swizzle(this, baseID, swizzleComponents));
}

IRNode::ID IRGenerator::getCap(int offset, String name) {
    auto found = fCapsMap.find(name);
    if (found == fCapsMap.end()) {
        fErrors.error(offset, "unknown capability flag '" + name + "'");
        return IRNode::ID();
    }
    String fullName = "sk_Caps." + name;
    return this->createNode(new Setting(this, offset, fullName,
                                        found->second.literal(this, offset)));
}

IRNode::ID IRGenerator::getArg(int offset, String name) {
    auto found = fSettings->fArgs.find(name);
    if (found == fSettings->fArgs.end()) {
        return IRNode::ID();
    }
    String fullName = "sk_Args." + name;
    return this->createNode(new Setting(this, offset, fullName,
                                        found->second.literal(this, offset)));
}

IRNode::ID IRGenerator::convertTypeField(int offset, IRNode::ID type, StringFragment field) {
    IRNode::ID result;
    for (IRNode::ID eID : *fProgramElements) {
        ProgramElement& e = (ProgramElement&) eID.node();
        if (e.fKind == ProgramElement::kEnum_Kind &&
            type.typeNode().name() == ((Enum&) e).fTypeName) {
            std::shared_ptr<SymbolTable> old = fSymbolTable;
            fSymbolTable = ((Enum&) e).fSymbols;
            result = convertIdentifier(ASTNode(&fFile->fNodes, offset, ASTNode::Kind::kIdentifier,
                                               field));
            fSymbolTable = old;
        }
    }
    if (!result) {
        fErrors.error(offset, "type '" + type.typeNode().fName + "' does not have a field named '" +
                              field + "'");
    }
    return result;
}

IRNode::ID IRGenerator::convertAppend(int offset, const std::vector<ASTNode>& args) {
#ifndef SKSL_STANDALONE
    if (args.size() < 2) {
        fErrors.error(offset, "'append' requires at least two arguments");
        return IRNode::ID();
    }
    IRNode::ID pipeline = this->convertExpression(args[0]);
    if (!pipeline) {
        return IRNode::ID();
    }
    if (pipeline.expressionNode().fType != fContext.fSkRasterPipeline_Type) {
        fErrors.error(offset, "first argument of 'append' must have type 'SkRasterPipeline'");
        return IRNode::ID();
    }
    if (ASTNode::Kind::kIdentifier != args[1].fKind) {
        fErrors.error(offset, "'" + args[1].description() + "' is not a valid stage");
        return IRNode::ID();
    }
    StringFragment name = args[1].getString();
    SkRasterPipeline::StockStage stage = SkRasterPipeline::premul;
    std::vector<IRNode::ID> stageArgs;
    stageArgs.push_back(std::move(pipeline));
    for (size_t i = 2; i < args.size(); ++i) {
        IRNode::ID arg = this->convertExpression(args[i]);
        if (!arg) {
            return IRNode::ID();
        }
        stageArgs.push_back(std::move(arg));
    }
    size_t expectedArgs = 0;
    // FIXME use a map
    if ("premul" == name) {
        stage = SkRasterPipeline::premul;
    }
    else if ("unpremul" == name) {
        stage = SkRasterPipeline::unpremul;
    }
    else if ("clamp_0" == name) {
        stage = SkRasterPipeline::clamp_0;
    }
    else if ("clamp_1" == name) {
        stage = SkRasterPipeline::clamp_1;
    }
    else if ("matrix_4x5" == name) {
        expectedArgs = 1;
        stage = SkRasterPipeline::matrix_4x5;
        if (stageArgs.size() == 1 &&
            stageArgs[0].expressionNode().fType.typeNode().fName != "float[20]") {
            fErrors.error(offset, "pipeline stage '" + name + "' expected a float[20] argument");
            return IRNode::ID();
        }
    }
    else {
        bool found = false;
        for (IRNode::ID e : *fProgramElements) {
            if (((ProgramElement&) e.node()).fKind == ProgramElement::kFunction_Kind) {
                const FunctionDefinition& f = (const FunctionDefinition&) e.node();
                if (((FunctionDeclaration&) f.fDeclaration.node()).fName == name) {
                    stage = SkRasterPipeline::callback;
                    std::vector<IRNode::ID> functions = { f.fDeclaration };
                    stageArgs.emplace_back(this->createNode(new FunctionReference(this, offset,
                                                                                  functions)));
                    found = true;
                    break;
                }
            }
        }
        if (!found) {
            fErrors.error(offset, "'" + name + "' is not a valid pipeline stage");
            return IRNode::ID();
        }
    }
    if (args.size() != expectedArgs + 2) {
        fErrors.error(offset, "pipeline stage '" + name + "' expected an additional argument " +
                              "count of " + to_string((int) expectedArgs) + ", but found " +
                              to_string((int) args.size() - 1));
        return IRNode::ID();
    }
    return this->createNode(new AppendStage(this, offset, stage, std::move(stageArgs)));
#else
    SkASSERT(false);
    return IRNode::ID();
#endif
}

IRNode::ID IRGenerator::convertIndexExpression(const ASTNode& index) {
    SkASSERT(index.fKind == ASTNode::Kind::kIndex);
    auto iter = index.begin();
    IRNode::ID base = this->convertExpression(*(iter++));
    if (!base) {
        return IRNode::ID();
    }
    if (iter != index.end()) {
        return this->convertIndex(std::move(base), *(iter++));
    } else if (base.expressionNode().fKind == Expression::kTypeReference_Kind) {
        IRNode::ID oldType = ((TypeReference&) base.node()).fValue;
        IRNode::ID newType = this->createNode(new Type(this, oldType.typeNode().name() + "[]",
                                                       Type::kArray_Kind, oldType,  -1));
        return this->createNode(new TypeReference(this, base.node().fOffset, newType));
    }
    fErrors.error(index.fOffset, "'[]' must follow a type name");
    return IRNode::ID();
}

IRNode::ID IRGenerator::convertCallExpression(const ASTNode& callNode) {
    SkASSERT(callNode.fKind == ASTNode::Kind::kCall);
    auto iter = callNode.begin();
    IRNode::ID base = this->convertExpression(*(iter++));
    if (!base) {
        return IRNode::ID();
    }
    std::vector<IRNode::ID> arguments;
    for (; iter != callNode.end(); ++iter) {
        IRNode::ID converted = this->convertExpression(*iter);
        if (!converted) {
            return IRNode::ID();
        }
        arguments.push_back(converted);
    }
    return this->callExpression(callNode.fOffset, base, std::move(arguments));
}

IRNode::ID IRGenerator::convertFieldExpression(const ASTNode& fieldNode) {
    IRNode::ID base = this->convertExpression(*fieldNode.begin());
    if (!base) {
        return IRNode::ID();
    }
    StringFragment field = fieldNode.getString();
    if (base.expressionNode().fType == fContext.fSkCaps_Type) {
        return this->getCap(fieldNode.fOffset, field);
    }
    if (base.expressionNode().fType == fContext.fSkArgs_Type) {
        return this->getArg(fieldNode.fOffset, field);
    }
    if (base.expressionNode().fKind == Expression::kTypeReference_Kind) {
        return this->convertTypeField(base.node().fOffset, ((TypeReference&) base.node()).fValue,
                                      field);
    }
    if (base.expressionNode().fKind == Expression::kExternalValue_Kind) {
        return this->convertField(base, field);
    }
    switch (base.expressionNode().fType.typeNode().kind()) {
        case Type::kVector_Kind:
            return this->convertSwizzle(base, field);
        case Type::kOther_Kind:
        case Type::kStruct_Kind:
            return this->convertField(base, field);
        default:
            fErrors.error(base.node().fOffset, "cannot swizzle value of type '" +
                                               base.expressionNode().fType.node().description() +
                                               "'");
            return IRNode::ID();
    }
}

IRNode::ID IRGenerator::convertPostfixExpression(const ASTNode& expression) {
    IRNode::ID base = this->convertExpression(*expression.begin());
    if (!base) {
        return IRNode::ID();
    }
    if (!base.expressionNode().fType.typeNode().isNumber()) {
        fErrors.error(expression.fOffset,
                      "'" + String(Compiler::OperatorName(expression.getToken().fKind)) +
                      "' cannot operate on '" + base.expressionNode().fType.node().description() +
                      "'");
        return IRNode::ID();
    }
    this->setRefKind(base.expressionNode(), kReadWrite_RefKind);
    return this->createNode(new PostfixExpression(this, base, expression.getToken().fKind));
}

void IRGenerator::checkValid(const Expression& expr) {
    switch (expr.fKind) {
        case Expression::kFunctionReference_Kind:
            fErrors.error(expr.fOffset, "expected '(' to begin function call");
            break;
        case Expression::kTypeReference_Kind:
            fErrors.error(expr.fOffset, "expected '(' to begin constructor invocation");
            break;
        default:
            if (!expr.fType) {
                fErrors.error(expr.fOffset, "invalid expression");
            }
    }
}

bool IRGenerator::checkSwizzleWrite(const Swizzle& swizzle) {
    int bits = 0;
    for (int idx : swizzle.fComponents) {
        if (idx < 0) {
            fErrors.error(swizzle.fOffset, "cannot write to a swizzle mask containing a constant");
            return false;
        }
        SkASSERT(idx <= 3);
        int bit = 1 << idx;
        if (bits & bit) {
            fErrors.error(swizzle.fOffset,
                          "cannot write to the same swizzle field more than once");
            return false;
        }
        bits |= bit;
    }
    return true;
}

void IRGenerator::setRefKind(const Expression& expr, RefKind kind) {
    switch (expr.fKind) {
        case Expression::kVariableReference_Kind: {
            const Variable& var = (Variable&) ((VariableReference&) expr).fVariable.node();
            if (var.fModifiers.fFlags & (Modifiers::kConst_Flag | Modifiers::kUniform_Flag)) {
                fErrors.error(expr.fOffset,
                              "cannot modify immutable variable '" + var.fName + "'");
            }
            ((VariableReference&) expr).setRefKind(kind);
            break;
        }
        case Expression::kFieldAccess_Kind:
            this->setRefKind(((FieldAccess&) expr).fBase.expressionNode(), kind);
            break;
        case Expression::kSwizzle_Kind: {
            const Swizzle& swizzle = (Swizzle&) expr;
            this->checkSwizzleWrite(swizzle);
            this->setRefKind(swizzle.fBase.expressionNode(), kind);
            break;
        }
        case Expression::kIndex_Kind:
            this->setRefKind(((IndexExpression&) expr).fBase.expressionNode(), kind);
            break;
        case Expression::kTernary_Kind: {
            TernaryExpression& t = (TernaryExpression&) expr;
            this->setRefKind(t.fIfTrue.expressionNode(), kind);
            this->setRefKind(t.fIfFalse.expressionNode(), kind);
            break;
        }
        case Expression::kExternalValue_Kind: {
            const ExternalValue& v = ((ExternalValueReference&) expr).fValue;
            if (!v.canWrite()) {
                fErrors.error(expr.fOffset,
                              String("cannot modify immutable external value '") + v.fName + "'");
            }
            break;
        }
        default:
            fErrors.error(expr.fOffset, "cannot assign to '" + expr.description() + "'");
            break;
    }
}

void IRGenerator::convertProgram(Program::Kind kind,
                                 const char* text,
                                 size_t length,
                                 SymbolTable& types,
                                 std::vector<IRNode::ID>* out) {
    fKind = kind;
    fProgramElements = out;
    Parser parser(text, length, types, this);
    fFile = parser.file();
    if (fErrors.errorCount()) {
        return;
    }
    SkASSERT(fFile);
    for (const auto& decl : fFile->root()) {
        switch (decl.fKind) {
            case ASTNode::Kind::kVarDeclarations: {
                IRNode::ID s = this->convertVarDeclarations(decl, Variable::kGlobal_Storage);
                if (s) {
                    fProgramElements->push_back(s);
                }
                break;
            }
            case ASTNode::Kind::kEnum: {
                this->convertEnum(decl);
                break;
            }
            case ASTNode::Kind::kFunction: {
                this->convertFunction(decl);
                break;
            }
            case ASTNode::Kind::kModifiers: {
                IRNode::ID f = this->convertModifiersDeclaration(decl);
                if (f) {
                    fProgramElements->push_back(f);
                }
                break;
            }
            case ASTNode::Kind::kInterfaceBlock: {
                IRNode::ID i = this->convertInterfaceBlock(decl);
                if (i) {
                    fProgramElements->push_back(i);
                }
                break;
            }
            case ASTNode::Kind::kExtension: {
                IRNode::ID e = this->convertExtension(decl.fOffset, decl.getString());
                if (e) {
                    fProgramElements->push_back(e);
                }
                break;
            }
            case ASTNode::Kind::kSection: {
                IRNode::ID s = this->convertSection(decl);
                if (s) {
                    fProgramElements->push_back(s);
                }
                break;
            }
            default:
                ABORT("unsupported declaration: %s\n", decl.description().c_str());
        }
    }
}


}
