/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLInliner.h"

#include <limits.h>
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
#include "src/sksl/ir/SkSLInlineMarker.h"
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
#include "src/sksl/ir/SkSLVariable.h"
#include "src/sksl/ir/SkSLVariableReference.h"

namespace SkSL {
namespace {

static constexpr int kInlinedStatementLimit = 2500;

static int count_returns_at_end_of_control_flow(const FunctionDefinition& funcDef) {
    class CountReturnsAtEndOfControlFlow : public ProgramVisitor {
    public:
        CountReturnsAtEndOfControlFlow(const FunctionDefinition& funcDef) {
            this->visitProgramElement(funcDef);
        }

        bool visitStatement(const Statement& stmt) override {
            switch (stmt.kind()) {
                case Statement::Kind::kBlock: {
                    // Check only the last statement of a block.
                    const auto& block = stmt.as<Block>();
                    return block.children().size() &&
                           this->visitStatement(*block.children().back());
                }
                case Statement::Kind::kSwitch:
                case Statement::Kind::kDo:
                case Statement::Kind::kFor:
                    // Don't introspect switches or loop structures at all.
                    return false;

                case Statement::Kind::kReturn:
                    ++fNumReturns;
                    [[fallthrough]];

                default:
                    return INHERITED::visitStatement(stmt);
            }
        }

        int fNumReturns = 0;
        using INHERITED = ProgramVisitor;
    };

    return CountReturnsAtEndOfControlFlow{funcDef}.fNumReturns;
}

static int count_returns_in_continuable_constructs(const FunctionDefinition& funcDef) {
    class CountReturnsInContinuableConstructs : public ProgramVisitor {
    public:
        CountReturnsInContinuableConstructs(const FunctionDefinition& funcDef) {
            this->visitProgramElement(funcDef);
        }

        bool visitStatement(const Statement& stmt) override {
            switch (stmt.kind()) {
                case Statement::Kind::kDo:
                case Statement::Kind::kFor: {
                    ++fInsideContinuableConstruct;
                    bool result = INHERITED::visitStatement(stmt);
                    --fInsideContinuableConstruct;
                    return result;
                }

                case Statement::Kind::kReturn:
                    fNumReturns += (fInsideContinuableConstruct > 0) ? 1 : 0;
                    [[fallthrough]];

                default:
                    return INHERITED::visitStatement(stmt);
            }
        }

        int fNumReturns = 0;
        int fInsideContinuableConstruct = 0;
        using INHERITED = ProgramVisitor;
    };

    return CountReturnsInContinuableConstructs{funcDef}.fNumReturns;
}

static bool contains_recursive_call(const FunctionDeclaration& funcDecl) {
    class ContainsRecursiveCall : public ProgramVisitor {
    public:
        bool visit(const FunctionDeclaration& funcDecl) {
            fFuncDecl = &funcDecl;
            return funcDecl.definition() ? this->visitProgramElement(*funcDecl.definition())
                                         : false;
        }

        bool visitExpression(const Expression& expr) override {
            if (expr.is<FunctionCall>() && expr.as<FunctionCall>().function().matches(*fFuncDecl)) {
                return true;
            }
            return INHERITED::visitExpression(expr);
        }

        bool visitStatement(const Statement& stmt) override {
            if (stmt.is<InlineMarker>() &&
                stmt.as<InlineMarker>().function().matches(*fFuncDecl)) {
                return true;
            }
            return INHERITED::visitStatement(stmt);
        }

        const FunctionDeclaration* fFuncDecl;
        using INHERITED = ProgramVisitor;
    };

    return ContainsRecursiveCall{}.visit(funcDecl);
}

static const Type* copy_if_needed(const Type* src, SymbolTable& symbolTable) {
    if (src->isArray()) {
        const Type* innerType = copy_if_needed(&src->componentType(), symbolTable);
        return symbolTable.takeOwnershipOfSymbol(Type::MakeArrayType(src->name(), *innerType,
                                                                     src->columns()));
    }
    return src;
}

static std::unique_ptr<Statement>* find_parent_statement(
        const std::vector<std::unique_ptr<Statement>*>& stmtStack) {
    SkASSERT(!stmtStack.empty());

    // Walk the statement stack from back to front, ignoring the last element (which is the
    // enclosing statement).
    auto iter = stmtStack.rbegin();
    ++iter;

    // Anything counts as a parent statement other than a scopeless Block.
    for (; iter != stmtStack.rend(); ++iter) {
        std::unique_ptr<Statement>* stmt = *iter;
        if (!(*stmt)->is<Block>() || (*stmt)->as<Block>().isScope()) {
            return stmt;
        }
    }

    // There wasn't any parent statement to be found.
    return nullptr;
}

std::unique_ptr<Expression> clone_with_ref_kind(const Expression& expr,
                                                VariableReference::RefKind refKind) {
    std::unique_ptr<Expression> clone = expr.clone();
    class SetRefKindInExpression : public ProgramWriter {
    public:
        SetRefKindInExpression(VariableReference::RefKind refKind) : fRefKind(refKind) {}
        bool visitExpression(Expression& expr) override {
            if (expr.is<VariableReference>()) {
                expr.as<VariableReference>().setRefKind(fRefKind);
            }
            return INHERITED::visitExpression(expr);
        }

    private:
        VariableReference::RefKind fRefKind;

        using INHERITED = ProgramWriter;
    };

    SetRefKindInExpression{refKind}.visitExpression(*clone);
    return clone;
}

class CountReturnsWithLimit : public ProgramVisitor {
public:
    CountReturnsWithLimit(const FunctionDefinition& funcDef, int limit) : fLimit(limit) {
        this->visitProgramElement(funcDef);
    }

    bool visitStatement(const Statement& stmt) override {
        switch (stmt.kind()) {
            case Statement::Kind::kReturn: {
                ++fNumReturns;
                fDeepestReturn = std::max(fDeepestReturn, fScopedBlockDepth);
                return (fNumReturns >= fLimit) || INHERITED::visitStatement(stmt);
            }
            case Statement::Kind::kBlock: {
                int depthIncrement = stmt.as<Block>().isScope() ? 1 : 0;
                fScopedBlockDepth += depthIncrement;
                bool result = INHERITED::visitStatement(stmt);
                fScopedBlockDepth -= depthIncrement;
                return result;
            }
            default:
                return INHERITED::visitStatement(stmt);
        }
    }

    int fNumReturns = 0;
    int fDeepestReturn = 0;
    int fLimit = 0;
    int fScopedBlockDepth = 0;
    using INHERITED = ProgramVisitor;
};

}  // namespace

Inliner::ReturnComplexity Inliner::GetReturnComplexity(const FunctionDefinition& funcDef) {
    int returnsAtEndOfControlFlow = count_returns_at_end_of_control_flow(funcDef);
    CountReturnsWithLimit counter{funcDef, returnsAtEndOfControlFlow + 1};

    if (counter.fNumReturns > returnsAtEndOfControlFlow) {
        return ReturnComplexity::kEarlyReturns;
    }
    if (counter.fNumReturns > 1 || counter.fDeepestReturn > 1) {
        return ReturnComplexity::kScopedReturns;
    }
    return ReturnComplexity::kSingleTopLevelReturn;
}

void Inliner::ensureScopedBlocks(Statement* inlinedBody, Statement* parentStmt) {
    // No changes necessary if this statement isn't actually a block.
    if (!inlinedBody || !inlinedBody->is<Block>()) {
        return;
    }

    // No changes necessary if the parent statement doesn't require a scope.
    if (!parentStmt || !(parentStmt->is<IfStatement>() || parentStmt->is<ForStatement>() ||
                         parentStmt->is<DoStatement>())) {
        return;
    }

    Block& block = inlinedBody->as<Block>();

    // The inliner will create inlined function bodies as a Block containing multiple statements,
    // but no scope. Normally, this is fine, but if this block is used as the statement for a
    // do/for/if/while, this isn't actually possible to represent textually; a scope must be added
    // for the generated code to match the intent. In the case of Blocks nested inside other Blocks,
    // we add the scope to the outermost block if needed. Zero-statement blocks have similar
    // issues--if we don't represent the Block textually somehow, we run the risk of accidentally
    // absorbing the following statement into our loop--so we also add a scope to these.
    for (Block* nestedBlock = &block;; ) {
        if (nestedBlock->isScope()) {
            // We found an explicit scope; all is well.
            return;
        }
        if (nestedBlock->children().size() != 1) {
            // We found a block with multiple (or zero) statements, but no scope? Let's add a scope
            // to the outermost block.
            block.setIsScope(true);
            return;
        }
        if (!nestedBlock->children()[0]->is<Block>()) {
            // This block has exactly one thing inside, and it's not another block. No need to scope
            // it.
            return;
        }
        // We have to go deeper.
        nestedBlock = &nestedBlock->children()[0]->as<Block>();
    }
}

void Inliner::reset(ModifiersPool* modifiers, const Program::Settings* settings) {
    fModifiers = modifiers;
    fSettings = settings;
    fInlineVarCounter = 0;
    fInlinedStatementCounter = 0;
}

String Inliner::uniqueNameForInlineVar(String baseName, SymbolTable* symbolTable) {
    // The inliner runs more than once, so the base name might already have a prefix like "_123_x".
    // Let's strip that prefix off to make the generated code easier to read.
    if (baseName.startsWith("_")) {
        // Determine if we have a string of digits.
        int offset = 1;
        while (isdigit(baseName[offset])) {
            ++offset;
        }
        // If we found digits, another underscore, and anything else, that's the inliner prefix.
        // Strip it off.
        if (offset > 1 && baseName[offset] == '_' && baseName[offset + 1] != '\0') {
            baseName.erase(0, offset + 1);
        } else {
            // This name doesn't contain an inliner prefix, but it does start with an underscore.
            // OpenGL disallows two consecutive underscores anywhere in the string, and we'll be
            // adding one as part of the inliner prefix, so strip the leading underscore.
            baseName.erase(0, 1);
        }
    }

    // Append a unique numeric prefix to avoid name overlap. Check the symbol table to make sure
    // we're not reusing an existing name. (Note that within a single compilation pass, this check
    // isn't fully comprehensive, as code isn't always generated in top-to-bottom order.)
    String uniqueName;
    for (;;) {
        uniqueName = String::printf("_%d_%s", fInlineVarCounter++, baseName.c_str());
        StringFragment frag{uniqueName.data(), uniqueName.length()};
        if ((*symbolTable)[frag] == nullptr) {
            break;
        }
    }

    return uniqueName;
}

std::unique_ptr<Expression> Inliner::inlineExpression(int offset,
                                                      VariableRewriteMap* varMap,
                                                      SymbolTable* symbolTableForExpression,
                                                      const Expression& expression) {
    auto expr = [&](const std::unique_ptr<Expression>& e) -> std::unique_ptr<Expression> {
        if (e) {
            return this->inlineExpression(offset, varMap, symbolTableForExpression, *e);
        }
        return nullptr;
    };
    auto argList = [&](const ExpressionArray& originalArgs) -> ExpressionArray {
        ExpressionArray args;
        args.reserve_back(originalArgs.size());
        for (const std::unique_ptr<Expression>& arg : originalArgs) {
            args.push_back(expr(arg));
        }
        return args;
    };

    switch (expression.kind()) {
        case Expression::Kind::kBinary: {
            const BinaryExpression& b = expression.as<BinaryExpression>();
            return std::make_unique<BinaryExpression>(offset,
                                                      expr(b.left()),
                                                      b.getOperator(),
                                                      expr(b.right()),
                                                      &b.type());
        }
        case Expression::Kind::kBoolLiteral:
        case Expression::Kind::kIntLiteral:
        case Expression::Kind::kFloatLiteral:
        case Expression::Kind::kNullLiteral:
            return expression.clone();
        case Expression::Kind::kConstructor: {
            const Constructor& constructor = expression.as<Constructor>();
            const Type* type = copy_if_needed(&constructor.type(), *symbolTableForExpression);
            return std::make_unique<Constructor>(offset, type, argList(constructor.arguments()));
        }
        case Expression::Kind::kExternalFunctionCall: {
            const ExternalFunctionCall& externalCall = expression.as<ExternalFunctionCall>();
            return std::make_unique<ExternalFunctionCall>(offset, &externalCall.function(),
                                                          argList(externalCall.arguments()));
        }
        case Expression::Kind::kExternalValue:
            return expression.clone();
        case Expression::Kind::kFieldAccess: {
            const FieldAccess& f = expression.as<FieldAccess>();
            return std::make_unique<FieldAccess>(expr(f.base()), f.fieldIndex(), f.ownerKind());
        }
        case Expression::Kind::kFunctionCall: {
            const FunctionCall& funcCall = expression.as<FunctionCall>();
            return std::make_unique<FunctionCall>(offset, &funcCall.type(), &funcCall.function(),
                                                  argList(funcCall.arguments()));
        }
        case Expression::Kind::kFunctionReference:
            return expression.clone();
        case Expression::Kind::kIndex: {
            const IndexExpression& idx = expression.as<IndexExpression>();
            return std::make_unique<IndexExpression>(*fContext, expr(idx.base()),
                                                     expr(idx.index()));
        }
        case Expression::Kind::kPrefix: {
            const PrefixExpression& p = expression.as<PrefixExpression>();
            return std::make_unique<PrefixExpression>(p.getOperator(), expr(p.operand()));
        }
        case Expression::Kind::kPostfix: {
            const PostfixExpression& p = expression.as<PostfixExpression>();
            return std::make_unique<PostfixExpression>(expr(p.operand()), p.getOperator());
        }
        case Expression::Kind::kSetting:
            return expression.clone();
        case Expression::Kind::kSwizzle: {
            const Swizzle& s = expression.as<Swizzle>();
            return std::make_unique<Swizzle>(*fContext, expr(s.base()), s.components());
        }
        case Expression::Kind::kTernary: {
            const TernaryExpression& t = expression.as<TernaryExpression>();
            return std::make_unique<TernaryExpression>(offset, expr(t.test()),
                                                       expr(t.ifTrue()), expr(t.ifFalse()));
        }
        case Expression::Kind::kTypeReference:
            return expression.clone();
        case Expression::Kind::kVariableReference: {
            const VariableReference& v = expression.as<VariableReference>();
            auto varMapIter = varMap->find(v.variable());
            if (varMapIter != varMap->end()) {
                return clone_with_ref_kind(*varMapIter->second, v.refKind());
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
                                                    std::unique_ptr<Expression>* resultExpr,
                                                    ReturnComplexity returnComplexity,
                                                    const Statement& statement,
                                                    bool isBuiltinCode) {
    auto stmt = [&](const std::unique_ptr<Statement>& s) -> std::unique_ptr<Statement> {
        if (s) {
            return this->inlineStatement(offset, varMap, symbolTableForStatement, resultExpr,
                                         returnComplexity, *s, isBuiltinCode);
        }
        return nullptr;
    };
    auto blockStmts = [&](const Block& block) {
        StatementArray result;
        result.reserve_back(block.children().size());
        for (const std::unique_ptr<Statement>& child : block.children()) {
            result.push_back(stmt(child));
        }
        return result;
    };
    auto stmts = [&](const StatementArray& ss) {
        StatementArray result;
        result.reserve_back(ss.size());
        for (const auto& s : ss) {
            result.push_back(stmt(s));
        }
        return result;
    };
    auto expr = [&](const std::unique_ptr<Expression>& e) -> std::unique_ptr<Expression> {
        if (e) {
            return this->inlineExpression(offset, varMap, symbolTableForStatement, *e);
        }
        return nullptr;
    };

    ++fInlinedStatementCounter;

    switch (statement.kind()) {
        case Statement::Kind::kBlock: {
            const Block& b = statement.as<Block>();
            return std::make_unique<Block>(offset, blockStmts(b),
                                           SymbolTable::WrapIfBuiltin(b.symbolTable()),
                                           b.isScope());
        }

        case Statement::Kind::kBreak:
        case Statement::Kind::kContinue:
        case Statement::Kind::kDiscard:
            return statement.clone();

        case Statement::Kind::kDo: {
            const DoStatement& d = statement.as<DoStatement>();
            return std::make_unique<DoStatement>(offset, stmt(d.statement()), expr(d.test()));
        }
        case Statement::Kind::kExpression: {
            const ExpressionStatement& e = statement.as<ExpressionStatement>();
            return std::make_unique<ExpressionStatement>(expr(e.expression()));
        }
        case Statement::Kind::kFor: {
            const ForStatement& f = statement.as<ForStatement>();
            // need to ensure initializer is evaluated first so that we've already remapped its
            // declarations by the time we evaluate test & next
            std::unique_ptr<Statement> initializer = stmt(f.initializer());
            return std::make_unique<ForStatement>(offset, std::move(initializer), expr(f.test()),
                                                  expr(f.next()), stmt(f.statement()),
                                                  SymbolTable::WrapIfBuiltin(f.symbols()));
        }
        case Statement::Kind::kIf: {
            const IfStatement& i = statement.as<IfStatement>();
            return std::make_unique<IfStatement>(offset, i.isStatic(), expr(i.test()),
                                                 stmt(i.ifTrue()), stmt(i.ifFalse()));
        }
        case Statement::Kind::kInlineMarker:
        case Statement::Kind::kNop:
            return statement.clone();
        case Statement::Kind::kReturn: {
            const ReturnStatement& r = statement.as<ReturnStatement>();
            if (!r.expression()) {
                if (returnComplexity >= ReturnComplexity::kEarlyReturns) {
                    // This function doesn't return a value, but has early returns, so we've wrapped
                    // it in a for loop. Use a continue to jump to the end of the loop and "leave"
                    // the function.
                    return std::make_unique<ContinueStatement>(offset);
                } else {
                    // This function doesn't exit early or return a value. A return statement at the
                    // end is a no-op and can be treated as such.
                    return std::make_unique<Nop>();
                }
            }

            // For a function that only contains a single top-level return, we don't need to store
            // the result in a variable at all. Just move the return value right into the result
            // expression.
            SkASSERT(resultExpr);
            SkASSERT(*resultExpr);
            if (returnComplexity <= ReturnComplexity::kSingleTopLevelReturn) {
                *resultExpr = expr(r.expression());
                return std::make_unique<Nop>();
            }

            // For more complex functions, assign their result into a variable.
            auto assignment =
                    std::make_unique<ExpressionStatement>(std::make_unique<BinaryExpression>(
                            offset,
                            clone_with_ref_kind(**resultExpr, VariableReference::RefKind::kWrite),
                            Token::Kind::TK_EQ,
                            expr(r.expression()),
                            &resultExpr->get()->type()));

            // Early returns are wrapped in a for loop; we need to synthesize a continue statement
            // to "leave" the function.
            if (returnComplexity >= ReturnComplexity::kEarlyReturns) {
                StatementArray block;
                block.reserve_back(2);
                block.push_back(std::move(assignment));
                block.push_back(std::make_unique<ContinueStatement>(offset));
                return std::make_unique<Block>(offset, std::move(block), /*symbols=*/nullptr,
                                               /*isScope=*/true);
            }
            // Functions without early returns aren't wrapped in a for loop and don't need to worry
            // about breaking out of the control flow.
            return std::move(assignment);

        }
        case Statement::Kind::kSwitch: {
            const SwitchStatement& ss = statement.as<SwitchStatement>();
            std::vector<std::unique_ptr<SwitchCase>> cases;
            cases.reserve(ss.cases().size());
            for (const std::unique_ptr<SwitchCase>& sc : ss.cases()) {
                cases.push_back(std::make_unique<SwitchCase>(offset, expr(sc->value()),
                                                             stmts(sc->statements())));
            }
            return std::make_unique<SwitchStatement>(offset, ss.isStatic(), expr(ss.value()),
                                                     std::move(cases),
                                                     SymbolTable::WrapIfBuiltin(ss.symbols()));
        }
        case Statement::Kind::kVarDeclaration: {
            const VarDeclaration& decl = statement.as<VarDeclaration>();
            std::unique_ptr<Expression> initialValue = expr(decl.value());
            int arraySize = decl.arraySize();
            const Variable& old = decl.var();
            // We assign unique names to inlined variables--scopes hide most of the problems in this
            // regard, but see `InlinerAvoidsVariableNameOverlap` for a counterexample where unique
            // names are important.
            auto name = std::make_unique<String>(
                    this->uniqueNameForInlineVar(String(old.name()), symbolTableForStatement));
            const String* namePtr = symbolTableForStatement->takeOwnershipOfString(std::move(name));
            const Type* baseTypePtr = copy_if_needed(&decl.baseType(), *symbolTableForStatement);
            const Type* typePtr = copy_if_needed(&old.type(), *symbolTableForStatement);
            const Variable* clone = symbolTableForStatement->takeOwnershipOfSymbol(
                    std::make_unique<Variable>(offset,
                                               &old.modifiers(),
                                               namePtr->c_str(),
                                               typePtr,
                                               isBuiltinCode,
                                               old.storage(),
                                               initialValue.get()));
            (*varMap)[&old] = std::make_unique<VariableReference>(offset, clone);
            return std::make_unique<VarDeclaration>(clone, baseTypePtr, arraySize,
                                                    std::move(initialValue));
        }
        default:
            SkASSERT(false);
            return nullptr;
    }
}

Inliner::InlineVariable Inliner::makeInlineVariable(const String& baseName,
                                                    const Type* type,
                                                    SymbolTable* symbolTable,
                                                    Modifiers modifiers,
                                                    bool isBuiltinCode,
                                                    std::unique_ptr<Expression>* initialValue) {
    // $floatLiteral or $intLiteral aren't real types that we can use for scratch variables, so
    // replace them if they ever appear here. If this happens, we likely forgot to coerce a type
    // somewhere during compilation.
    if (type == fContext->fFloatLiteral_Type.get()) {
        SkDEBUGFAIL("found a $floatLiteral type while inlining");
        type = fContext->fFloat_Type.get();
    } else if (type == fContext->fIntLiteral_Type.get()) {
        SkDEBUGFAIL("found an $intLiteral type while inlining");
        type = fContext->fInt_Type.get();
    }

    // Provide our new variable with a unique name, and add it to our symbol table.
    const String* namePtr = symbolTable->takeOwnershipOfString(
            std::make_unique<String>(this->uniqueNameForInlineVar(baseName, symbolTable)));
    StringFragment nameFrag{namePtr->c_str(), namePtr->length()};

    // Create our new variable and add it to the symbol table.
    InlineVariable result;
    result.fVarSymbol =
            symbolTable->add(std::make_unique<Variable>(/*offset=*/-1,
                                                        fModifiers->addToPool(Modifiers()),
                                                        nameFrag,
                                                        type,
                                                        isBuiltinCode,
                                                        Variable::Storage::kLocal,
                                                        initialValue->get()));

    // Prepare the variable declaration (taking extra care with `out` params to not clobber any
    // initial value).
    if (*initialValue && (modifiers.fFlags & Modifiers::kOut_Flag)) {
        result.fVarDecl = std::make_unique<VarDeclaration>(result.fVarSymbol, type, /*arraySize=*/0,
                                                           (*initialValue)->clone());
    } else {
        result.fVarDecl = std::make_unique<VarDeclaration>(result.fVarSymbol, type, /*arraySize=*/0,
                                                           std::move(*initialValue));
    }
    return result;
}

Inliner::InlinedCall Inliner::inlineCall(FunctionCall* call,
                                         std::shared_ptr<SymbolTable> symbolTable,
                                         const FunctionDeclaration* caller) {
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
    SkASSERT(this->isSafeToInline(call->function().definition()));

    ExpressionArray& arguments = call->arguments();
    const int offset = call->fOffset;
    const FunctionDefinition& function = *call->function().definition();
    const ReturnComplexity returnComplexity = GetReturnComplexity(function);
    bool hasEarlyReturn = (returnComplexity >= ReturnComplexity::kEarlyReturns);

    InlinedCall inlinedCall;
    inlinedCall.fInlinedBody = std::make_unique<Block>(offset, StatementArray{},
                                                       /*symbols=*/nullptr,
                                                       /*isScope=*/false);

    Block& inlinedBody = *inlinedCall.fInlinedBody;
    inlinedBody.children().reserve_back(
            1 +                 // Inline marker
            1 +                 // Result variable
            arguments.size() +  // Function arguments (passing in)
            arguments.size() +  // Function arguments (copy out-params back)
            1);                 // Block for inlined code

    inlinedBody.children().push_back(std::make_unique<InlineMarker>(&call->function()));

    // Create a variable to hold the result in the extra statements (excepting void).
    std::unique_ptr<Expression> resultExpr;
    if (function.declaration().returnType() != *fContext->fVoid_Type) {
        std::unique_ptr<Expression> noInitialValue;
        InlineVariable var = this->makeInlineVariable(function.declaration().name(),
                                                      &function.declaration().returnType(),
                                                      symbolTable.get(), Modifiers{},
                                                      caller->isBuiltin(), &noInitialValue);
        inlinedBody.children().push_back(std::move(var.fVarDecl));
        resultExpr = std::make_unique<VariableReference>(/*offset=*/-1, var.fVarSymbol);
   }

    // Create variables in the extra statements to hold the arguments, and assign the arguments to
    // them.
    VariableRewriteMap varMap;
    std::vector<int> argsToCopyBack;
    for (int i = 0; i < (int) arguments.size(); ++i) {
        const Variable* param = function.declaration().parameters()[i];
        bool isOutParam = param->modifiers().fFlags & Modifiers::kOut_Flag;

        // If this argument can be inlined trivially (e.g. a swizzle, or a constant array index)...
        if (Analysis::IsTrivialExpression(*arguments[i])) {
            // ... and it's an `out` param, or it isn't written to within the inline function...
            if (isOutParam || !Analysis::StatementWritesToVariable(*function.body(), *param)) {
                // ... we don't need to copy it at all! We can just use the existing expression.
                varMap[param] = arguments[i]->clone();
                continue;
            }
        }
        if (isOutParam) {
            argsToCopyBack.push_back(i);
        }
        InlineVariable var = this->makeInlineVariable(param->name(), &arguments[i]->type(),
                                                      symbolTable.get(), param->modifiers(),
                                                      caller->isBuiltin(), &arguments[i]);
        inlinedBody.children().push_back(std::move(var.fVarDecl));
        varMap[param] = std::make_unique<VariableReference>(/*offset=*/-1, var.fVarSymbol);
    }

    const Block& body = function.body()->as<Block>();
    StatementArray* inlineStatements;

    if (hasEarlyReturn) {
        // Since we output to backends that don't have a goto statement (which would normally be
        // used to perform an early return), we fake it by wrapping the function in a single-
        // iteration for loop, and use a continue statement to jump to the end of the loop
        // prematurely.

        // int _1_loop = 0;
        symbolTable = std::make_shared<SymbolTable>(std::move(symbolTable), caller->isBuiltin());
        const Type* intType = fContext->fInt_Type.get();
        std::unique_ptr<Expression> initialValue = std::make_unique<IntLiteral>(/*offset=*/-1,
                                                                                /*value=*/0,
                                                                                intType);
        InlineVariable loopVar = this->makeInlineVariable("loop", intType, symbolTable.get(),
                                                          Modifiers{}, caller->isBuiltin(),
                                                          &initialValue);

        // _1_loop < 1;
        std::unique_ptr<Expression> test = std::make_unique<BinaryExpression>(
                /*offset=*/-1,
                std::make_unique<VariableReference>(/*offset=*/-1, loopVar.fVarSymbol),
                Token::Kind::TK_LT,
                std::make_unique<IntLiteral>(/*offset=*/-1, /*value=*/1, intType),
                fContext->fBool_Type.get());

        // _1_loop++
        std::unique_ptr<Expression> increment = std::make_unique<PostfixExpression>(
                std::make_unique<VariableReference>(/*offset=*/-1, loopVar.fVarSymbol,
                                                    VariableReference::RefKind::kReadWrite),
                Token::Kind::TK_PLUSPLUS);

        // {...}
        auto innerBlock = std::make_unique<Block>(offset, StatementArray{},
                                                  /*symbols=*/nullptr, /*isScope=*/true);
        inlineStatements = &innerBlock->children();

        // for (int _1_loop = 0; _1_loop < 1; _1_loop++) {...}
        inlinedBody.children().push_back(std::make_unique<ForStatement>(/*offset=*/-1,
                                                                        std::move(loopVar.fVarDecl),
                                                                        std::move(test),
                                                                        std::move(increment),
                                                                        std::move(innerBlock),
                                                                        symbolTable));
    } else {
        // No early returns, so we can just dump the code into our existing scopeless block.
        inlineStatements = &inlinedBody.children();
    }

    inlineStatements->reserve_back(body.children().size() + argsToCopyBack.size());
    for (const std::unique_ptr<Statement>& stmt : body.children()) {
        inlineStatements->push_back(this->inlineStatement(offset, &varMap, symbolTable.get(),
                                                          &resultExpr, returnComplexity, *stmt,
                                                          caller->isBuiltin()));
    }

    // Copy back the values of `out` parameters into their real destinations.
    for (int i : argsToCopyBack) {
        const Variable* p = function.declaration().parameters()[i];
        SkASSERT(varMap.find(p) != varMap.end());
        inlineStatements->push_back(
                std::make_unique<ExpressionStatement>(std::make_unique<BinaryExpression>(
                        offset,
                        clone_with_ref_kind(*arguments[i], VariableReference::RefKind::kWrite),
                        Token::Kind::TK_EQ,
                        std::move(varMap[p]),
                        &arguments[i]->type())));
    }

    if (resultExpr != nullptr) {
        // Return our result variable as our replacement expression.
        inlinedCall.fReplacementExpr = std::move(resultExpr);
    } else {
        // It's a void function, so it doesn't actually result in anything, but we have to return
        // something non-null as a standin.
        inlinedCall.fReplacementExpr = std::make_unique<BoolLiteral>(*fContext,
                                                                     offset,
                                                                     /*value=*/false);
    }

    return inlinedCall;
}

bool Inliner::isSafeToInline(const FunctionDefinition* functionDef) {
    SkASSERT(fSettings);

    // A threshold of zero indicates that the inliner is completely disabled, so we can just return.
    if (fSettings->fInlineThreshold <= 0) {
        return false;
    }

    // Enforce a limit on inlining to avoid pathological cases. (inliner/ExponentialGrowth.sksl)
    if (fInlinedStatementCounter >= kInlinedStatementLimit) {
        return false;
    }

    if (functionDef == nullptr) {
        // Can't inline something if we don't actually have its definition.
        return false;
    }

    // We don't have any mechanism to simulate early returns within a construct that supports
    // continues (for/do/while), so we can't inline if there's a return inside one.
    bool hasReturnInContinuableConstruct =
            (count_returns_in_continuable_constructs(*functionDef) > 0);
    return !hasReturnInContinuableConstruct;
}

// A candidate function for inlining, containing everything that `inlineCall` needs.
struct InlineCandidate {
    std::shared_ptr<SymbolTable> fSymbols;        // the SymbolTable of the candidate
    std::unique_ptr<Statement>* fParentStmt;      // the parent Statement of the enclosing stmt
    std::unique_ptr<Statement>* fEnclosingStmt;   // the Statement containing the candidate
    std::unique_ptr<Expression>* fCandidateExpr;  // the candidate FunctionCall to be inlined
    FunctionDefinition* fEnclosingFunction;       // the Function containing the candidate
};

struct InlineCandidateList {
    std::vector<InlineCandidate> fCandidates;
};

class InlineCandidateAnalyzer {
public:
    // A list of all the inlining candidates we found during analysis.
    InlineCandidateList* fCandidateList;

    // A stack of the symbol tables; since most nodes don't have one, expected to be shallower than
    // the enclosing-statement stack.
    std::vector<std::shared_ptr<SymbolTable>> fSymbolTableStack;
    // A stack of "enclosing" statements--these would be suitable for the inliner to use for adding
    // new instructions. Not all statements are suitable (e.g. a for-loop's initializer). The
    // inliner might replace a statement with a block containing the statement.
    std::vector<std::unique_ptr<Statement>*> fEnclosingStmtStack;
    // The function that we're currently processing (i.e. inlining into).
    FunctionDefinition* fEnclosingFunction = nullptr;

    void visit(const std::vector<std::unique_ptr<ProgramElement>>& elements,
               std::shared_ptr<SymbolTable> symbols,
               InlineCandidateList* candidateList) {
        fCandidateList = candidateList;
        fSymbolTableStack.push_back(symbols);

        for (const std::unique_ptr<ProgramElement>& pe : elements) {
            this->visitProgramElement(pe.get());
        }

        fSymbolTableStack.pop_back();
        fCandidateList = nullptr;
    }

    void visitProgramElement(ProgramElement* pe) {
        switch (pe->kind()) {
            case ProgramElement::Kind::kFunction: {
                FunctionDefinition& funcDef = pe->as<FunctionDefinition>();
                fEnclosingFunction = &funcDef;
                this->visitStatement(&funcDef.body());
                break;
            }
            default:
                // The inliner can't operate outside of a function's scope.
                break;
        }
    }

    void visitStatement(std::unique_ptr<Statement>* stmt,
                        bool isViableAsEnclosingStatement = true) {
        if (!*stmt) {
            return;
        }

        size_t oldEnclosingStmtStackSize = fEnclosingStmtStack.size();
        size_t oldSymbolStackSize = fSymbolTableStack.size();

        if (isViableAsEnclosingStatement) {
            fEnclosingStmtStack.push_back(stmt);
        }

        switch ((*stmt)->kind()) {
            case Statement::Kind::kBreak:
            case Statement::Kind::kContinue:
            case Statement::Kind::kDiscard:
            case Statement::Kind::kInlineMarker:
            case Statement::Kind::kNop:
                break;

            case Statement::Kind::kBlock: {
                Block& block = (*stmt)->as<Block>();
                if (block.symbolTable()) {
                    fSymbolTableStack.push_back(block.symbolTable());
                }

                for (std::unique_ptr<Statement>& stmt : block.children()) {
                    this->visitStatement(&stmt);
                }
                break;
            }
            case Statement::Kind::kDo: {
                DoStatement& doStmt = (*stmt)->as<DoStatement>();
                // The loop body is a candidate for inlining.
                this->visitStatement(&doStmt.statement());
                // The inliner isn't smart enough to inline the test-expression for a do-while
                // loop at this time. There are two limitations:
                // - We would need to insert the inlined-body block at the very end of the do-
                //   statement's inner fStatement. We don't support that today, but it's doable.
                // - We cannot inline the test expression if the loop uses `continue` anywhere; that
                //   would skip over the inlined block that evaluates the test expression. There
                //   isn't a good fix for this--any workaround would be more complex than the cost
                //   of a function call. However, loops that don't use `continue` would still be
                //   viable candidates for inlining.
                break;
            }
            case Statement::Kind::kExpression: {
                ExpressionStatement& expr = (*stmt)->as<ExpressionStatement>();
                this->visitExpression(&expr.expression());
                break;
            }
            case Statement::Kind::kFor: {
                ForStatement& forStmt = (*stmt)->as<ForStatement>();
                if (forStmt.symbols()) {
                    fSymbolTableStack.push_back(forStmt.symbols());
                }

                // The initializer and loop body are candidates for inlining.
                this->visitStatement(&forStmt.initializer(),
                                     /*isViableAsEnclosingStatement=*/false);
                this->visitStatement(&forStmt.statement());

                // The inliner isn't smart enough to inline the test- or increment-expressions
                // of a for loop loop at this time. There are a handful of limitations:
                // - We would need to insert the test-expression block at the very beginning of the
                //   for-loop's inner fStatement, and the increment-expression block at the very
                //   end. We don't support that today, but it's doable.
                // - The for-loop's built-in test-expression would need to be dropped entirely,
                //   and the loop would be halted via a break statement at the end of the inlined
                //   test-expression. This is again something we don't support today, but it could
                //   be implemented.
                // - We cannot inline the increment-expression if the loop uses `continue` anywhere;
                //   that would skip over the inlined block that evaluates the increment expression.
                //   There isn't a good fix for this--any workaround would be more complex than the
                //   cost of a function call. However, loops that don't use `continue` would still
                //   be viable candidates for increment-expression inlining.
                break;
            }
            case Statement::Kind::kIf: {
                IfStatement& ifStmt = (*stmt)->as<IfStatement>();
                this->visitExpression(&ifStmt.test());
                this->visitStatement(&ifStmt.ifTrue());
                this->visitStatement(&ifStmt.ifFalse());
                break;
            }
            case Statement::Kind::kReturn: {
                ReturnStatement& returnStmt = (*stmt)->as<ReturnStatement>();
                this->visitExpression(&returnStmt.expression());
                break;
            }
            case Statement::Kind::kSwitch: {
                SwitchStatement& switchStmt = (*stmt)->as<SwitchStatement>();
                if (switchStmt.symbols()) {
                    fSymbolTableStack.push_back(switchStmt.symbols());
                }

                this->visitExpression(&switchStmt.value());
                for (const std::unique_ptr<SwitchCase>& switchCase : switchStmt.cases()) {
                    // The switch-case's fValue cannot be a FunctionCall; skip it.
                    for (std::unique_ptr<Statement>& caseBlock : switchCase->statements()) {
                        this->visitStatement(&caseBlock);
                    }
                }
                break;
            }
            case Statement::Kind::kVarDeclaration: {
                VarDeclaration& varDeclStmt = (*stmt)->as<VarDeclaration>();
                // Don't need to scan the declaration's sizes; those are always IntLiterals.
                this->visitExpression(&varDeclStmt.value());
                break;
            }
            default:
                SkUNREACHABLE;
        }

        // Pop our symbol and enclosing-statement stacks.
        fSymbolTableStack.resize(oldSymbolStackSize);
        fEnclosingStmtStack.resize(oldEnclosingStmtStackSize);
    }

    void visitExpression(std::unique_ptr<Expression>* expr) {
        if (!*expr) {
            return;
        }

        switch ((*expr)->kind()) {
            case Expression::Kind::kBoolLiteral:
            case Expression::Kind::kDefined:
            case Expression::Kind::kExternalValue:
            case Expression::Kind::kFieldAccess:
            case Expression::Kind::kFloatLiteral:
            case Expression::Kind::kFunctionReference:
            case Expression::Kind::kIntLiteral:
            case Expression::Kind::kNullLiteral:
            case Expression::Kind::kSetting:
            case Expression::Kind::kTypeReference:
            case Expression::Kind::kVariableReference:
                // Nothing to scan here.
                break;

            case Expression::Kind::kBinary: {
                BinaryExpression& binaryExpr = (*expr)->as<BinaryExpression>();
                this->visitExpression(&binaryExpr.left());

                // Logical-and and logical-or binary expressions do not inline the right side,
                // because that would invalidate short-circuiting. That is, when evaluating
                // expressions like these:
                //    (false && x())   // always false
                //    (true || y())    // always true
                // It is illegal for side-effects from x() or y() to occur. The simplest way to
                // enforce that rule is to avoid inlining the right side entirely. However, it is
                // safe for other types of binary expression to inline both sides.
                Token::Kind op = binaryExpr.getOperator();
                bool shortCircuitable = (op == Token::Kind::TK_LOGICALAND ||
                                         op == Token::Kind::TK_LOGICALOR);
                if (!shortCircuitable) {
                    this->visitExpression(&binaryExpr.right());
                }
                break;
            }
            case Expression::Kind::kConstructor: {
                Constructor& constructorExpr = (*expr)->as<Constructor>();
                for (std::unique_ptr<Expression>& arg : constructorExpr.arguments()) {
                    this->visitExpression(&arg);
                }
                break;
            }
            case Expression::Kind::kExternalFunctionCall: {
                ExternalFunctionCall& funcCallExpr = (*expr)->as<ExternalFunctionCall>();
                for (std::unique_ptr<Expression>& arg : funcCallExpr.arguments()) {
                    this->visitExpression(&arg);
                }
                break;
            }
            case Expression::Kind::kFunctionCall: {
                FunctionCall& funcCallExpr = (*expr)->as<FunctionCall>();
                for (std::unique_ptr<Expression>& arg : funcCallExpr.arguments()) {
                    this->visitExpression(&arg);
                }
                this->addInlineCandidate(expr);
                break;
            }
            case Expression::Kind::kIndex:{
                IndexExpression& indexExpr = (*expr)->as<IndexExpression>();
                this->visitExpression(&indexExpr.base());
                this->visitExpression(&indexExpr.index());
                break;
            }
            case Expression::Kind::kPostfix: {
                PostfixExpression& postfixExpr = (*expr)->as<PostfixExpression>();
                this->visitExpression(&postfixExpr.operand());
                break;
            }
            case Expression::Kind::kPrefix: {
                PrefixExpression& prefixExpr = (*expr)->as<PrefixExpression>();
                this->visitExpression(&prefixExpr.operand());
                break;
            }
            case Expression::Kind::kSwizzle: {
                Swizzle& swizzleExpr = (*expr)->as<Swizzle>();
                this->visitExpression(&swizzleExpr.base());
                break;
            }
            case Expression::Kind::kTernary: {
                TernaryExpression& ternaryExpr = (*expr)->as<TernaryExpression>();
                // The test expression is a candidate for inlining.
                this->visitExpression(&ternaryExpr.test());
                // The true- and false-expressions cannot be inlined, because we are only allowed to
                // evaluate one side.
                break;
            }
            default:
                SkUNREACHABLE;
        }
    }

    void addInlineCandidate(std::unique_ptr<Expression>* candidate) {
        fCandidateList->fCandidates.push_back(
                InlineCandidate{fSymbolTableStack.back(),
                                find_parent_statement(fEnclosingStmtStack),
                                fEnclosingStmtStack.back(),
                                candidate,
                                fEnclosingFunction});
    }
};

static const FunctionDeclaration& candidate_func(const InlineCandidate& candidate) {
    return (*candidate.fCandidateExpr)->as<FunctionCall>().function();
}

bool Inliner::candidateCanBeInlined(const InlineCandidate& candidate, InlinabilityCache* cache) {
    const FunctionDeclaration& funcDecl = candidate_func(candidate);
    auto [iter, wasInserted] = cache->insert({&funcDecl, false});
    if (wasInserted) {
        // Recursion is forbidden here to avoid an infinite death spiral of inlining.
        iter->second = this->isSafeToInline(funcDecl.definition()) &&
                       !contains_recursive_call(funcDecl);
    }

    return iter->second;
}

int Inliner::getFunctionSize(const FunctionDeclaration& funcDecl, FunctionSizeCache* cache) {
    auto [iter, wasInserted] = cache->insert({&funcDecl, 0});
    if (wasInserted) {
        iter->second = Analysis::NodeCountUpToLimit(*funcDecl.definition(),
                                                    fSettings->fInlineThreshold);
    }
    return iter->second;
}

void Inliner::buildCandidateList(const std::vector<std::unique_ptr<ProgramElement>>& elements,
                                 std::shared_ptr<SymbolTable> symbols, ProgramUsage* usage,
                                 InlineCandidateList* candidateList) {
    // This is structured much like a ProgramVisitor, but does not actually use ProgramVisitor.
    // The analyzer needs to keep track of the `unique_ptr<T>*` of statements and expressions so
    // that they can later be replaced, and ProgramVisitor does not provide this; it only provides a
    // `const T&`.
    InlineCandidateAnalyzer analyzer;
    analyzer.visit(elements, symbols, candidateList);

    // Early out if there are no inlining candidates.
    std::vector<InlineCandidate>& candidates = candidateList->fCandidates;
    if (candidates.empty()) {
        return;
    }

    // Remove candidates that are not safe to inline.
    InlinabilityCache cache;
    candidates.erase(std::remove_if(candidates.begin(),
                                    candidates.end(),
                                    [&](const InlineCandidate& candidate) {
                                        return !this->candidateCanBeInlined(candidate, &cache);
                                    }),
                     candidates.end());

    // If the inline threshold is unlimited, or if we have no candidates left, our candidate list is
    // complete.
    if (fSettings->fInlineThreshold == INT_MAX || candidates.empty()) {
        return;
    }

    // Remove candidates on a per-function basis if the effect of inlining would be to make more
    // than `inlineThreshold` nodes. (i.e. if Func() would be inlined six times and its size is
    // 10 nodes, it should be inlined if the inlineThreshold is 60 or higher.)
    FunctionSizeCache functionSizeCache;
    FunctionSizeCache candidateTotalCost;
    for (InlineCandidate& candidate : candidates) {
        const FunctionDeclaration& fnDecl = candidate_func(candidate);
        candidateTotalCost[&fnDecl] += this->getFunctionSize(fnDecl, &functionSizeCache);
    }

    candidates.erase(
            std::remove_if(candidates.begin(),
                           candidates.end(),
                           [&](const InlineCandidate& candidate) {
                               const FunctionDeclaration& fnDecl = candidate_func(candidate);
                               if (fnDecl.modifiers().fFlags & Modifiers::kInline_Flag) {
                                   // Functions marked `inline` ignore size limitations.
                                   return false;
                               }
                               if (usage->get(fnDecl) == 1) {
                                   // If a function is only used once, it's cost-free to inline.
                                   return false;
                               }
                               if (candidateTotalCost[&fnDecl] <= fSettings->fInlineThreshold) {
                                   // We won't exceed the inline threshold by inlining this.
                                   return false;
                               }
                               // Inlining this function will add too many IRNodes.
                               return true;
                           }),
            candidates.end());
}

bool Inliner::analyze(const std::vector<std::unique_ptr<ProgramElement>>& elements,
                      std::shared_ptr<SymbolTable> symbols,
                      ProgramUsage* usage) {
    // A threshold of zero indicates that the inliner is completely disabled, so we can just return.
    if (fSettings->fInlineThreshold <= 0) {
        return false;
    }

    // Enforce a limit on inlining to avoid pathological cases. (inliner/ExponentialGrowth.sksl)
    if (fInlinedStatementCounter >= kInlinedStatementLimit) {
        return false;
    }

    InlineCandidateList candidateList;
    this->buildCandidateList(elements, symbols, usage, &candidateList);

    // Inline the candidates where we've determined that it's safe to do so.
    std::unordered_set<const std::unique_ptr<Statement>*> enclosingStmtSet;
    bool madeChanges = false;
    for (const InlineCandidate& candidate : candidateList.fCandidates) {
        FunctionCall& funcCall = (*candidate.fCandidateExpr)->as<FunctionCall>();

        // Inlining two expressions using the same enclosing statement in the same inlining pass
        // does not work properly. If this happens, skip it; we'll get it in the next pass.
        auto [unusedIter, inserted] = enclosingStmtSet.insert(candidate.fEnclosingStmt);
        if (!inserted) {
            continue;
        }

        // Convert the function call to its inlined equivalent.
        InlinedCall inlinedCall = this->inlineCall(&funcCall, candidate.fSymbols,
                                                   &candidate.fEnclosingFunction->declaration());
        if (inlinedCall.fInlinedBody) {
            // Ensure that the inlined body has a scope if it needs one.
            this->ensureScopedBlocks(inlinedCall.fInlinedBody.get(), candidate.fParentStmt->get());

            // Add references within the inlined body
            usage->add(inlinedCall.fInlinedBody.get());

            // Move the enclosing statement to the end of the unscoped Block containing the inlined
            // function, then replace the enclosing statement with that Block.
            // Before:
            //     fInlinedBody = Block{ stmt1, stmt2, stmt3 }
            //     fEnclosingStmt = stmt4
            // After:
            //     fInlinedBody = null
            //     fEnclosingStmt = Block{ stmt1, stmt2, stmt3, stmt4 }
            inlinedCall.fInlinedBody->children().push_back(std::move(*candidate.fEnclosingStmt));
            *candidate.fEnclosingStmt = std::move(inlinedCall.fInlinedBody);
        }

        // Replace the candidate function call with our replacement expression.
        usage->replace(candidate.fCandidateExpr->get(), inlinedCall.fReplacementExpr.get());
        *candidate.fCandidateExpr = std::move(inlinedCall.fReplacementExpr);
        madeChanges = true;

        // Stop inlining if we've reached our hard cap on new statements.
        if (fInlinedStatementCounter >= kInlinedStatementLimit) {
            break;
        }

        // Note that nothing was destroyed except for the FunctionCall. All other nodes should
        // remain valid.
    }

    return madeChanges;
}

}  // namespace SkSL
