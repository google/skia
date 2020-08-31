/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLInliner.h"

#include "limits.h"
#include <memory>
#include <unordered_set>

#include "src/sksl/SkSLAnalysis.h"
#include "src/sksl/ir/SkSLBinaryExpression.h"
#include "src/sksl/ir/SkSLBoolLiteral.h"
#include "src/sksl/ir/SkSLBreakStatement.h"
#include "src/sksl/ir/SkSLConstructor.h"
#include "src/sksl/ir/SkSLContinueStatement.h"
#include "src/sksl/ir/SkSLDiscardStatement.h"
#include "src/sksl/ir/SkSLDoStatement.h"
#include "src/sksl/ir/SkSLEnum.h"
#include "src/sksl/ir/SkSLExpressionStatement.h"
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
#include "src/sksl/ir/SkSLNop.h"
#include "src/sksl/ir/SkSLNullLiteral.h"
#include "src/sksl/ir/SkSLPostfixExpression.h"
#include "src/sksl/ir/SkSLPrefixExpression.h"
#include "src/sksl/ir/SkSLReturnStatement.h"
#include "src/sksl/ir/SkSLSetting.h"
#include "src/sksl/ir/SkSLSwitchCase.h"
#include "src/sksl/ir/SkSLSwitchStatement.h"
#include "src/sksl/ir/SkSLSwizzle.h"
#include "src/sksl/ir/SkSLTernaryExpression.h"
#include "src/sksl/ir/SkSLUnresolvedFunction.h"
#include "src/sksl/ir/SkSLVarDeclarations.h"
#include "src/sksl/ir/SkSLVarDeclarationsStatement.h"
#include "src/sksl/ir/SkSLVariable.h"
#include "src/sksl/ir/SkSLVariableReference.h"
#include "src/sksl/ir/SkSLWhileStatement.h"

namespace SkSL {
namespace {

static int count_all_returns(const FunctionDefinition& funcDef) {
    class CountAllReturns : public ProgramVisitor {
    public:
        CountAllReturns(const FunctionDefinition& funcDef) {
            this->visitProgramElement(funcDef);
        }

        bool visitStatement(const Statement& stmt) override {
            switch (stmt.fKind) {
                case Statement::kReturn_Kind:
                    ++fNumReturns;
                    [[fallthrough]];

                default:
                    return this->INHERITED::visitStatement(stmt);
            }
        }

        int fNumReturns = 0;
        using INHERITED = ProgramVisitor;
    };

    return CountAllReturns{funcDef}.fNumReturns;
}

static int count_returns_at_end_of_control_flow(const FunctionDefinition& funcDef) {
    class CountReturnsAtEndOfControlFlow : public ProgramVisitor {
    public:
        CountReturnsAtEndOfControlFlow(const FunctionDefinition& funcDef) {
            this->visitProgramElement(funcDef);
        }

        bool visitStatement(const Statement& stmt) override {
            switch (stmt.fKind) {
                case Statement::kBlock_Kind: {
                    // Check only the last statement of a block.
                    const auto& blockStmts = stmt.as<Block>().fStatements;
                    return (blockStmts.size() > 0) ? this->visitStatement(*blockStmts.back())
                                                   : false;
                }
                case Statement::kSwitch_Kind:
                case Statement::kWhile_Kind:
                case Statement::kDo_Kind:
                case Statement::kFor_Kind:
                    // Don't introspect switches or loop structures at all.
                    return false;

                case Statement::kReturn_Kind:
                    ++fNumReturns;
                    [[fallthrough]];

                default:
                    return this->INHERITED::visitStatement(stmt);
            }
        }

        int fNumReturns = 0;
        using INHERITED = ProgramVisitor;
    };

    return CountReturnsAtEndOfControlFlow{funcDef}.fNumReturns;
}

static int count_returns_in_breakable_constructs(const FunctionDefinition& funcDef) {
    class CountReturnsInBreakableConstructs : public ProgramVisitor {
    public:
        CountReturnsInBreakableConstructs(const FunctionDefinition& funcDef) {
            this->visitProgramElement(funcDef);
        }

        bool visitStatement(const Statement& stmt) override {
            switch (stmt.fKind) {
                case Statement::kSwitch_Kind:
                case Statement::kWhile_Kind:
                case Statement::kDo_Kind:
                case Statement::kFor_Kind: {
                    ++fInsideBreakableConstruct;
                    bool result = this->INHERITED::visitStatement(stmt);
                    --fInsideBreakableConstruct;
                    return result;
                }

                case Statement::kReturn_Kind:
                    fNumReturns += (fInsideBreakableConstruct > 0) ? 1 : 0;
                    [[fallthrough]];

                default:
                    return this->INHERITED::visitStatement(stmt);
            }
        }

        int fNumReturns = 0;
        int fInsideBreakableConstruct = 0;
        using INHERITED = ProgramVisitor;
    };

    return CountReturnsInBreakableConstructs{funcDef}.fNumReturns;
}

static bool has_early_return(const FunctionDefinition& funcDef) {
    int returnCount = count_all_returns(funcDef);
    if (returnCount == 0) {
        return false;
    }

    int returnsAtEndOfControlFlow = count_returns_at_end_of_control_flow(funcDef);
    return returnCount > returnsAtEndOfControlFlow;
}

static const Type* copy_if_needed(const Type* src, SymbolTable& symbolTable) {
    if (src->kind() == Type::kArray_Kind) {
        return symbolTable.takeOwnershipOfSymbol(std::make_unique<Type>(*src));
    }
    return src;
}

}  // namespace

void Inliner::reset(const Context& context, const Program::Settings& settings) {
    fContext = &context;
    fSettings = &settings;
    fInlineVarCounter = 0;
}

std::unique_ptr<Expression> Inliner::inlineExpression(int offset,
                                                      VariableRewriteMap* varMap,
                                                      const Expression& expression) {
    auto expr = [&](const std::unique_ptr<Expression>& e) -> std::unique_ptr<Expression> {
        if (e) {
            return this->inlineExpression(offset, varMap, *e);
        }
        return nullptr;
    };
    auto argList = [&](const std::vector<std::unique_ptr<Expression>>& originalArgs)
            -> std::vector<std::unique_ptr<Expression>> {
        std::vector<std::unique_ptr<Expression>> args;
        args.reserve(originalArgs.size());
        for (const std::unique_ptr<Expression>& arg : originalArgs) {
            args.push_back(expr(arg));
        }
        return args;
    };

    switch (expression.fKind) {
        case Expression::kBinary_Kind: {
            const BinaryExpression& b = expression.as<BinaryExpression>();
            return std::make_unique<BinaryExpression>(offset,
                                                      expr(b.fLeft),
                                                      b.fOperator,
                                                      expr(b.fRight),
                                                      b.fType);
        }
        case Expression::kBoolLiteral_Kind:
        case Expression::kIntLiteral_Kind:
        case Expression::kFloatLiteral_Kind:
        case Expression::kNullLiteral_Kind:
            return expression.clone();
        case Expression::kConstructor_Kind: {
            const Constructor& constructor = expression.as<Constructor>();
            return std::make_unique<Constructor>(offset, constructor.fType,
                                                 argList(constructor.fArguments));
        }
        case Expression::kExternalFunctionCall_Kind: {
            const ExternalFunctionCall& externalCall = expression.as<ExternalFunctionCall>();
            return std::make_unique<ExternalFunctionCall>(offset, externalCall.fType,
                                                          externalCall.fFunction,
                                                          argList(externalCall.fArguments));
        }
        case Expression::kExternalValue_Kind:
            return expression.clone();
        case Expression::kFieldAccess_Kind: {
            const FieldAccess& f = expression.as<FieldAccess>();
            return std::make_unique<FieldAccess>(expr(f.fBase), f.fFieldIndex, f.fOwnerKind);
        }
        case Expression::kFunctionCall_Kind: {
            const FunctionCall& funcCall = expression.as<FunctionCall>();
            return std::make_unique<FunctionCall>(offset, funcCall.fType, funcCall.fFunction,
                                                  argList(funcCall.fArguments));
        }
        case Expression::kIndex_Kind: {
            const IndexExpression& idx = expression.as<IndexExpression>();
            return std::make_unique<IndexExpression>(*fContext, expr(idx.fBase), expr(idx.fIndex));
        }
        case Expression::kPrefix_Kind: {
            const PrefixExpression& p = expression.as<PrefixExpression>();
            return std::make_unique<PrefixExpression>(p.fOperator, expr(p.fOperand));
        }
        case Expression::kPostfix_Kind: {
            const PostfixExpression& p = expression.as<PostfixExpression>();
            return std::make_unique<PostfixExpression>(expr(p.fOperand), p.fOperator);
        }
        case Expression::kSetting_Kind:
            return expression.clone();
        case Expression::kSwizzle_Kind: {
            const Swizzle& s = expression.as<Swizzle>();
            return std::make_unique<Swizzle>(*fContext, expr(s.fBase), s.fComponents);
        }
        case Expression::kTernary_Kind: {
            const TernaryExpression& t = expression.as<TernaryExpression>();
            return std::make_unique<TernaryExpression>(offset, expr(t.fTest),
                                                       expr(t.fIfTrue), expr(t.fIfFalse));
        }
        case Expression::kVariableReference_Kind: {
            const VariableReference& v = expression.as<VariableReference>();
            auto found = varMap->find(&v.fVariable);
            if (found != varMap->end()) {
                return std::make_unique<VariableReference>(offset, *found->second, v.fRefKind);
            }
            return v.clone();
        }
        default:
            SkASSERT(false);
            return nullptr;
    }
}

std::unique_ptr<Statement> Inliner::inlineStatement(int offset,
                                                    VariableRewriteMap* varMap,
                                                    SymbolTable* symbolTableForStatement,
                                                    const Variable* returnVar,
                                                    bool haveEarlyReturns,
                                                    const Statement& statement) {
    auto stmt = [&](const std::unique_ptr<Statement>& s) -> std::unique_ptr<Statement> {
        if (s) {
            return this->inlineStatement(offset, varMap, symbolTableForStatement, returnVar,
                                         haveEarlyReturns, *s);
        }
        return nullptr;
    };
    auto stmts = [&](const std::vector<std::unique_ptr<Statement>>& ss) {
        std::vector<std::unique_ptr<Statement>> result;
        for (const auto& s : ss) {
            result.push_back(stmt(s));
        }
        return result;
    };
    auto expr = [&](const std::unique_ptr<Expression>& e) -> std::unique_ptr<Expression> {
        if (e) {
            return this->inlineExpression(offset, varMap, *e);
        }
        return nullptr;
    };
    switch (statement.fKind) {
        case Statement::kBlock_Kind: {
            const Block& b = statement.as<Block>();
            return std::make_unique<Block>(offset, stmts(b.fStatements), b.fSymbols, b.fIsScope);
        }

        case Statement::kBreak_Kind:
        case Statement::kContinue_Kind:
        case Statement::kDiscard_Kind:
            return statement.clone();

        case Statement::kDo_Kind: {
            const DoStatement& d = statement.as<DoStatement>();
            return std::make_unique<DoStatement>(offset, stmt(d.fStatement), expr(d.fTest));
        }
        case Statement::kExpression_Kind: {
            const ExpressionStatement& e = statement.as<ExpressionStatement>();
            return std::make_unique<ExpressionStatement>(expr(e.fExpression));
        }
        case Statement::kFor_Kind: {
            const ForStatement& f = statement.as<ForStatement>();
            // need to ensure initializer is evaluated first so that we've already remapped its
            // declarations by the time we evaluate test & next
            std::unique_ptr<Statement> initializer = stmt(f.fInitializer);
            return std::make_unique<ForStatement>(offset, std::move(initializer), expr(f.fTest),
                                                  expr(f.fNext), stmt(f.fStatement), f.fSymbols);
        }
        case Statement::kIf_Kind: {
            const IfStatement& i = statement.as<IfStatement>();
            return std::make_unique<IfStatement>(offset, i.fIsStatic, expr(i.fTest),
                                                 stmt(i.fIfTrue), stmt(i.fIfFalse));
        }
        case Statement::kNop_Kind:
            return statement.clone();
        case Statement::kReturn_Kind: {
            const ReturnStatement& r = statement.as<ReturnStatement>();
            if (r.fExpression) {
                auto assignment = std::make_unique<ExpressionStatement>(
                        std::make_unique<BinaryExpression>(
                            offset,
                            std::make_unique<VariableReference>(offset, *returnVar,
                                                                VariableReference::kWrite_RefKind),
                            Token::Kind::TK_EQ,
                            expr(r.fExpression),
                            returnVar->fType));
                if (haveEarlyReturns) {
                    std::vector<std::unique_ptr<Statement>> block;
                    block.push_back(std::move(assignment));
                    block.emplace_back(new BreakStatement(offset));
                    return std::make_unique<Block>(offset, std::move(block), /*symbols=*/nullptr,
                                                   /*isScope=*/true);
                } else {
                    return std::move(assignment);
                }
            } else {
                if (haveEarlyReturns) {
                    return std::make_unique<BreakStatement>(offset);
                } else {
                    return std::make_unique<Nop>();
                }
            }
        }
        case Statement::kSwitch_Kind: {
            const SwitchStatement& ss = statement.as<SwitchStatement>();
            std::vector<std::unique_ptr<SwitchCase>> cases;
            for (const auto& sc : ss.fCases) {
                cases.emplace_back(new SwitchCase(offset, expr(sc->fValue),
                                                  stmts(sc->fStatements)));
            }
            return std::make_unique<SwitchStatement>(offset, ss.fIsStatic, expr(ss.fValue),
                                                     std::move(cases), ss.fSymbols);
        }
        case Statement::kVarDeclaration_Kind: {
            const VarDeclaration& decl = statement.as<VarDeclaration>();
            std::vector<std::unique_ptr<Expression>> sizes;
            for (const auto& size : decl.fSizes) {
                sizes.push_back(expr(size));
            }
            std::unique_ptr<Expression> initialValue = expr(decl.fValue);
            const Variable* old = decl.fVar;
            // need to copy the var name in case the originating function is discarded and we lose
            // its symbols
            std::unique_ptr<String> name(new String(old->fName));
            const String* namePtr = symbolTableForStatement->takeOwnershipOfString(std::move(name));
            const Type* typePtr = copy_if_needed(&old->fType, *symbolTableForStatement);
            const Variable* clone = symbolTableForStatement->takeOwnershipOfSymbol(
                    std::make_unique<Variable>(offset,
                                               old->fModifiers,
                                               namePtr->c_str(),
                                               *typePtr,
                                               old->fStorage,
                                               initialValue.get()));
            (*varMap)[old] = clone;
            return std::make_unique<VarDeclaration>(clone, std::move(sizes),
                                                    std::move(initialValue));
        }
        case Statement::kVarDeclarations_Kind: {
            const VarDeclarations& decls = *statement.as<VarDeclarationsStatement>().fDeclaration;
            std::vector<std::unique_ptr<VarDeclaration>> vars;
            for (const auto& var : decls.fVars) {
                vars.emplace_back(&stmt(var).release()->as<VarDeclaration>());
            }
            const Type* typePtr = copy_if_needed(&decls.fBaseType, *symbolTableForStatement);
            return std::unique_ptr<Statement>(new VarDeclarationsStatement(
                    std::make_unique<VarDeclarations>(offset, typePtr, std::move(vars))));
        }
        case Statement::kWhile_Kind: {
            const WhileStatement& w = statement.as<WhileStatement>();
            return std::make_unique<WhileStatement>(offset, expr(w.fTest), stmt(w.fStatement));
        }
        default:
            SkASSERT(false);
            return nullptr;
    }
}

Inliner::InlinedCall Inliner::inlineCall(std::unique_ptr<FunctionCall> call,
                                         SymbolTable* symbolTableForCall) {
    // Inlining is more complicated here than in a typical compiler, because we have to have a
    // high-level IR and can't just drop statements into the middle of an expression or even use
    // gotos.
    //
    // Since we can't insert statements into an expression, we run the inline function as extra
    // statements before the statement we're currently processing, relying on a lack of execution
    // order guarantees. Since we can't use gotos (which are normally used to replace return
    // statements), we wrap the whole function in a loop and use break statements to jump to the
    // end.
    SkASSERT(fSettings);
    SkASSERT(fContext);
    SkASSERT(call);
    SkASSERT(this->isSafeToInline(*call, /*inlineThreshold=*/INT_MAX));

    int offset = call->fOffset;
    std::vector<std::unique_ptr<Expression>>& arguments = call->fArguments;
    const FunctionDefinition& function = *call->fFunction.fDefinition;
    InlinedCall inlinedCall;
    std::vector<std::unique_ptr<Statement>> inlinedBody;

    auto makeInlineVar = [&](const String& baseName, const Type& type, Modifiers modifiers,
                             std::unique_ptr<Expression>* initialValue) -> const Variable* {
        // If the base name starts with an underscore, like "_coords", we can't append another
        // underscore, because some OpenGL platforms error out when they see two consecutive
        // underscores (anywhere in the string!). But in the general case, using the underscore as
        // a splitter reads nicely enough that it's worth putting in this special case.
        const char* splitter = baseName.startsWith("_") ? "_X" : "_";

        // Append a unique numeric prefix to avoid name overlap. Check the symbol table to make sure
        // we're not reusing an existing name. (Note that within a single compilation pass, this
        // check isn't fully comprehensive, as code isn't always generated in top-to-bottom order.)
        String uniqueName;
        for (;;) {
            uniqueName = String::printf("_%d%s%s", fInlineVarCounter++, splitter, baseName.c_str());
            StringFragment frag{uniqueName.data(), uniqueName.length()};
            if ((*symbolTableForCall)[frag] == nullptr) {
                break;
            }
        }

        // Add our new variable's name to the symbol table.
        const String* namePtr = symbolTableForCall->takeOwnershipOfString(
                std::make_unique<String>(std::move(uniqueName)));
        StringFragment nameFrag{namePtr->c_str(), namePtr->length()};

        // Add our new variable to the symbol table.
        auto newVar = std::make_unique<Variable>(/*offset=*/-1, Modifiers(), nameFrag, type,
                                                 Variable::kLocal_Storage, initialValue->get());
        const Variable* variableSymbol = symbolTableForCall->add(nameFrag, std::move(newVar));

        // Prepare the variable declaration (taking extra care with `out` params to not clobber any
        // initial value).
        std::vector<std::unique_ptr<VarDeclaration>> variables;
        if (initialValue && (modifiers.fFlags & Modifiers::kOut_Flag)) {
            variables.push_back(std::make_unique<VarDeclaration>(
                    variableSymbol, /*sizes=*/std::vector<std::unique_ptr<Expression>>{},
                    (*initialValue)->clone()));
        } else {
            variables.push_back(std::make_unique<VarDeclaration>(
                    variableSymbol, /*sizes=*/std::vector<std::unique_ptr<Expression>>{},
                    std::move(*initialValue)));
        }

        // Add the new variable-declaration statement to our block of extra statements.
        inlinedBody.push_back(std::make_unique<VarDeclarationsStatement>(
                std::make_unique<VarDeclarations>(offset, &type, std::move(variables))));

        return variableSymbol;
    };

    // Create a variable to hold the result in the extra statements (excepting void).
    const Variable* resultVar = nullptr;
    if (function.fDeclaration.fReturnType != *fContext->fVoid_Type) {
        std::unique_ptr<Expression> noInitialValue;
        resultVar = makeInlineVar(String(function.fDeclaration.fName),
                                  function.fDeclaration.fReturnType, Modifiers{}, &noInitialValue);
    }

    // Create variables in the extra statements to hold the arguments, and assign the arguments to
    // them.
    VariableRewriteMap varMap;
    for (int i = 0; i < (int) arguments.size(); ++i) {
        const Variable* param = function.fDeclaration.fParameters[i];

        if (arguments[i]->fKind == Expression::kVariableReference_Kind) {
            // The argument is just a variable, so we only need to copy it if it's an out parameter
            // or it's written to within the function.
            if ((param->fModifiers.fFlags & Modifiers::kOut_Flag) ||
                !Analysis::StatementWritesToVariable(*function.fBody, *param)) {
                varMap[param] = &arguments[i]->as<VariableReference>().fVariable;
                continue;
            }
        }

        varMap[param] = makeInlineVar(String(param->fName), arguments[i]->fType, param->fModifiers,
                                      &arguments[i]);
    }

    const Block& body = function.fBody->as<Block>();
    bool hasEarlyReturn = has_early_return(function);
    auto inlineBlock = std::make_unique<Block>(offset, std::vector<std::unique_ptr<Statement>>{});
    inlineBlock->fStatements.reserve(body.fStatements.size());
    for (const std::unique_ptr<Statement>& stmt : body.fStatements) {
        inlineBlock->fStatements.push_back(this->inlineStatement(
                offset, &varMap, symbolTableForCall, resultVar, hasEarlyReturn, *stmt));
    }
    if (hasEarlyReturn) {
        // Since we output to backends that don't have a goto statement (which would normally be
        // used to perform an early return), we fake it by wrapping the function in a
        // do { } while (false); and then use break statements to jump to the end in order to
        // emulate a goto.
        inlinedBody.push_back(std::make_unique<DoStatement>(
                /*offset=*/-1,
                std::move(inlineBlock),
                std::make_unique<BoolLiteral>(*fContext, offset, /*value=*/false)));
    } else {
        // No early returns, so we can just dump the code in. We need to use a block so we don't get
        // name conflicts with locals.
        inlinedBody.push_back(std::move(inlineBlock));
    }

    // Copy the values of `out` parameters into their destinations.
    for (size_t i = 0; i < arguments.size(); ++i) {
        const Variable* p = function.fDeclaration.fParameters[i];
        if (p->fModifiers.fFlags & Modifiers::kOut_Flag) {
            SkASSERT(varMap.find(p) != varMap.end());
            if (arguments[i]->fKind == Expression::kVariableReference_Kind &&
                &arguments[i]->as<VariableReference>().fVariable == varMap[p]) {
                // we didn't create a temporary for this parameter, so there's nothing to copy back
                // out
                continue;
            }
            auto varRef = std::make_unique<VariableReference>(offset, *varMap[p]);
            inlinedBody.push_back(std::make_unique<ExpressionStatement>(
                    std::make_unique<BinaryExpression>(offset,
                                                       arguments[i]->clone(),
                                                       Token::Kind::TK_EQ,
                                                       std::move(varRef),
                                                       arguments[i]->fType)));
        }
    }

    if (function.fDeclaration.fReturnType != *fContext->fVoid_Type) {
        // Return a reference to the result variable as our replacement expression.
        inlinedCall.fReplacementExpr = std::make_unique<VariableReference>(offset, *resultVar);
    } else {
        // It's a void function, so it doesn't actually result in anything, but we have to return
        // something non-null as a standin.
        inlinedCall.fReplacementExpr = std::make_unique<BoolLiteral>(*fContext, offset,
                                                                     /*value=*/false);
    }

    switch (inlinedBody.size()) {
        case 0:
            break;
        case 1:
            inlinedCall.fInlinedBody = std::move(inlinedBody.front());
            break;
        default:
            inlinedCall.fInlinedBody = std::make_unique<Block>(offset, std::move(inlinedBody),
                                                               /*symbols=*/nullptr,
                                                               /*isScope=*/false);
            break;
    }

    return inlinedCall;
}

bool Inliner::isSafeToInline(const FunctionCall& functionCall,
                             int inlineThreshold) {
    SkASSERT(fSettings);

    if (functionCall.fFunction.fDefinition == nullptr) {
        // Can't inline something if we don't actually have its definition.
        return false;
    }
    const FunctionDefinition& functionDef = *functionCall.fFunction.fDefinition;
    if (inlineThreshold < INT_MAX) {
        if (!(functionDef.fDeclaration.fModifiers.fFlags & Modifiers::kInline_Flag) &&
            Analysis::NodeCount(functionDef) >= inlineThreshold) {
            // The function exceeds our maximum inline size and is not flagged 'inline'.
            return false;
        }
    }
    if (!fSettings->fCaps || !fSettings->fCaps->canUseDoLoops()) {
        // We don't have do-while loops. We use do-while loops to simulate early returns, so we
        // can't inline functions that have an early return.
        bool hasEarlyReturn = has_early_return(functionDef);

        // If we didn't detect an early return, there shouldn't be any returns in breakable
        // constructs either.
        SkASSERT(hasEarlyReturn || count_returns_in_breakable_constructs(functionDef) == 0);
        return !hasEarlyReturn;
    }
    // We have do-while loops, but we don't have any mechanism to simulate early returns within a
    // breakable construct (switch/for/do/while), so we can't inline if there's a return inside one.
    bool hasReturnInBreakableConstruct = (count_returns_in_breakable_constructs(functionDef) > 0);

    // If we detected returns in breakable constructs, we should also detect an early return.
    SkASSERT(!hasReturnInBreakableConstruct || has_early_return(functionDef));
    return !hasReturnInBreakableConstruct;
}

}  // namespace SkSL
