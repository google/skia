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
#include "src/sksl/ir/SkSLVarDeclarationsStatement.h"
#include "src/sksl/ir/SkSLVariable.h"
#include "src/sksl/ir/SkSLVariableReference.h"
#include "src/sksl/ir/SkSLWhileStatement.h"

namespace SkSL {
namespace {

static bool contains_returns_above_limit(const FunctionDefinition& funcDef, int limit) {
    class CountReturnsWithLimit : public ProgramVisitor {
    public:
        CountReturnsWithLimit(const FunctionDefinition& funcDef, int limit) : fLimit(limit) {
            this->visitProgramElement(funcDef);
        }

        bool visitStatement(const Statement& stmt) override {
            switch (stmt.kind()) {
                case Statement::Kind::kReturn:
                    ++fNumReturns;
                    return (fNumReturns > fLimit) || INHERITED::visitStatement(stmt);

                default:
                    return INHERITED::visitStatement(stmt);
            }
        }

        int fNumReturns = 0;
        int fLimit = 0;
        using INHERITED = ProgramVisitor;
    };

    return CountReturnsWithLimit{funcDef, limit}.fNumReturns > limit;
}

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
                case Statement::Kind::kWhile:
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

static int count_returns_in_breakable_constructs(const FunctionDefinition& funcDef) {
    class CountReturnsInBreakableConstructs : public ProgramVisitor {
    public:
        CountReturnsInBreakableConstructs(const FunctionDefinition& funcDef) {
            this->visitProgramElement(funcDef);
        }

        bool visitStatement(const Statement& stmt) override {
            switch (stmt.kind()) {
                case Statement::Kind::kSwitch:
                case Statement::Kind::kWhile:
                case Statement::Kind::kDo:
                case Statement::Kind::kFor: {
                    ++fInsideBreakableConstruct;
                    bool result = INHERITED::visitStatement(stmt);
                    --fInsideBreakableConstruct;
                    return result;
                }

                case Statement::Kind::kReturn:
                    fNumReturns += (fInsideBreakableConstruct > 0) ? 1 : 0;
                    [[fallthrough]];

                default:
                    return INHERITED::visitStatement(stmt);
            }
        }

        int fNumReturns = 0;
        int fInsideBreakableConstruct = 0;
        using INHERITED = ProgramVisitor;
    };

    return CountReturnsInBreakableConstructs{funcDef}.fNumReturns;
}

static bool has_early_return(const FunctionDefinition& funcDef) {
    int returnsAtEndOfControlFlow = count_returns_at_end_of_control_flow(funcDef);
    return contains_returns_above_limit(funcDef, returnsAtEndOfControlFlow);
}

static bool contains_recursive_call(const FunctionDeclaration& funcDecl) {
    class ContainsRecursiveCall : public ProgramVisitor {
    public:
        bool visit(const FunctionDeclaration& funcDecl) {
            fFuncDecl = &funcDecl;
            return funcDecl.fDefinition ? this->visitProgramElement(*funcDecl.fDefinition)
                                        : false;
        }

        bool visitExpression(const Expression& expr) override {
            if (expr.is<FunctionCall>() && expr.as<FunctionCall>().function().matches(*fFuncDecl)) {
                return true;
            }
            return INHERITED::visitExpression(expr);
        }

        bool visitStatement(const Statement& stmt) override {
            if (stmt.is<InlineMarker>() && stmt.as<InlineMarker>().fFuncDecl->matches(*fFuncDecl)) {
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
    if (src->typeKind() == Type::TypeKind::kArray) {
        return symbolTable.takeOwnershipOfSymbol(std::make_unique<Type>(src->name(),
                                                                        src->typeKind(),
                                                                        src->componentType(),
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

bool is_trivial_argument(const Expression& argument) {
    return argument.is<VariableReference>() ||
           (argument.is<Swizzle>() && is_trivial_argument(*argument.as<Swizzle>().fBase)) ||
           (argument.is<FieldAccess>() && is_trivial_argument(*argument.as<FieldAccess>().fBase)) ||
           (argument.is<Constructor>() &&
            argument.as<Constructor>().arguments().size() == 1 &&
            is_trivial_argument(*argument.as<Constructor>().arguments().front())) ||
           (argument.is<IndexExpression>() &&
            argument.as<IndexExpression>().fIndex->is<IntLiteral>() &&
            is_trivial_argument(*argument.as<IndexExpression>().fBase));
}

}  // namespace

void Inliner::ensureScopedBlocks(Statement* inlinedBody, Statement* parentStmt) {
    // No changes necessary if this statement isn't actually a block.
    if (!inlinedBody || !inlinedBody->is<Block>()) {
        return;
    }

    // No changes necessary if the parent statement doesn't require a scope.
    if (!parentStmt || !(parentStmt->is<IfStatement>() || parentStmt->is<ForStatement>() ||
                         parentStmt->is<DoStatement>() || parentStmt->is<WhileStatement>())) {
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

void Inliner::reset(const Context& context, const Program::Settings& settings) {
    fContext = &context;
    fSettings = &settings;
    fInlineVarCounter = 0;
}

String Inliner::uniqueNameForInlineVar(const String& baseName, SymbolTable* symbolTable) {
    // If the base name starts with an underscore, like "_coords", we can't append another
    // underscore, because OpenGL disallows two consecutive underscores anywhere in the string. But
    // in the general case, using the underscore as a splitter reads nicely enough that it's worth
    // putting in this special case.
    const char* splitter = baseName.startsWith("_") ? "" : "_";

    // Append a unique numeric prefix to avoid name overlap. Check the symbol table to make sure
    // we're not reusing an existing name. (Note that within a single compilation pass, this check
    // isn't fully comprehensive, as code isn't always generated in top-to-bottom order.)
    String uniqueName;
    for (;;) {
        uniqueName = String::printf("_%d%s%s", fInlineVarCounter++, splitter, baseName.c_str());
        StringFragment frag{uniqueName.data(), uniqueName.length()};
        if ((*symbolTable)[frag] == nullptr) {
            break;
        }
    }

    return uniqueName;
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

    switch (expression.kind()) {
        case Expression::Kind::kBinary: {
            const BinaryExpression& b = expression.as<BinaryExpression>();
            return std::make_unique<BinaryExpression>(offset,
                                                      expr(b.leftPointer()),
                                                      b.getOperator(),
                                                      expr(b.rightPointer()),
                                                      &b.type());
        }
        case Expression::Kind::kBoolLiteral:
        case Expression::Kind::kIntLiteral:
        case Expression::Kind::kFloatLiteral:
        case Expression::Kind::kNullLiteral:
            return expression.clone();
        case Expression::Kind::kConstructor: {
            const Constructor& constructor = expression.as<Constructor>();
            return std::make_unique<Constructor>(offset, &constructor.type(),
                                                 argList(constructor.arguments()));
        }
        case Expression::Kind::kExternalFunctionCall: {
            const ExternalFunctionCall& externalCall = expression.as<ExternalFunctionCall>();
            return std::make_unique<ExternalFunctionCall>(offset, &externalCall.type(),
                                                          externalCall.function(),
                                                          argList(externalCall.arguments()));
        }
        case Expression::Kind::kExternalValue:
            return expression.clone();
        case Expression::Kind::kFieldAccess: {
            const FieldAccess& f = expression.as<FieldAccess>();
            return std::make_unique<FieldAccess>(expr(f.fBase), f.fFieldIndex, f.fOwnerKind);
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
            return std::make_unique<IndexExpression>(*fContext, expr(idx.fBase), expr(idx.fIndex));
        }
        case Expression::Kind::kPrefix: {
            const PrefixExpression& p = expression.as<PrefixExpression>();
            return std::make_unique<PrefixExpression>(p.fOperator, expr(p.fOperand));
        }
        case Expression::Kind::kPostfix: {
            const PostfixExpression& p = expression.as<PostfixExpression>();
            return std::make_unique<PostfixExpression>(expr(p.fOperand), p.fOperator);
        }
        case Expression::Kind::kSetting:
            return expression.clone();
        case Expression::Kind::kSwizzle: {
            const Swizzle& s = expression.as<Swizzle>();
            return std::make_unique<Swizzle>(*fContext, expr(s.fBase), s.fComponents);
        }
        case Expression::Kind::kTernary: {
            const TernaryExpression& t = expression.as<TernaryExpression>();
            return std::make_unique<TernaryExpression>(offset, expr(t.fTest),
                                                       expr(t.fIfTrue), expr(t.fIfFalse));
        }
        case Expression::Kind::kTypeReference:
            return expression.clone();
        case Expression::Kind::kVariableReference: {
            const VariableReference& v = expression.as<VariableReference>();
            auto varMapIter = varMap->find(v.fVariable);
            if (varMapIter != varMap->end()) {
                return clone_with_ref_kind(*varMapIter->second, v.fRefKind);
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
                                                    const Expression* resultExpr,
                                                    bool haveEarlyReturns,
                                                    const Statement& statement,
                                                    bool isBuiltinCode) {
    auto stmt = [&](const std::unique_ptr<Statement>& s) -> std::unique_ptr<Statement> {
        if (s) {
            return this->inlineStatement(offset, varMap, symbolTableForStatement, resultExpr,
                                         haveEarlyReturns, *s, isBuiltinCode);
        }
        return nullptr;
    };
    auto blockStmts = [&](const Block& block) {
        std::vector<std::unique_ptr<Statement>> result;
        for (const std::unique_ptr<Statement>& child : block.children()) {
            result.push_back(stmt(child));
        }
        return result;
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
    switch (statement.kind()) {
        case Statement::Kind::kBlock: {
            const Block& b = statement.as<Block>();
            return std::make_unique<Block>(offset, blockStmts(b), b.symbolTable(), b.isScope());
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
                                                  expr(f.next()), stmt(f.statement()), f.symbols());
        }
        case Statement::Kind::kIf: {
            const IfStatement& i = statement.as<IfStatement>();
            return std::make_unique<IfStatement>(offset, i.fIsStatic, expr(i.fTest),
                                                 stmt(i.fIfTrue), stmt(i.fIfFalse));
        }
        case Statement::Kind::kInlineMarker:
        case Statement::Kind::kNop:
            return statement.clone();
        case Statement::Kind::kReturn: {
            const ReturnStatement& r = statement.as<ReturnStatement>();
            if (r.fExpression) {
                SkASSERT(resultExpr);
                auto assignment =
                        std::make_unique<ExpressionStatement>(std::make_unique<BinaryExpression>(
                                offset,
                                clone_with_ref_kind(*resultExpr, VariableReference::kWrite_RefKind),
                                Token::Kind::TK_EQ,
                                expr(r.fExpression),
                                &resultExpr->type()));
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
        case Statement::Kind::kSwitch: {
            const SwitchStatement& ss = statement.as<SwitchStatement>();
            std::vector<std::unique_ptr<SwitchCase>> cases;
            for (const auto& sc : ss.fCases) {
                cases.emplace_back(new SwitchCase(offset, expr(sc->fValue),
                                                  stmts(sc->fStatements)));
            }
            return std::make_unique<SwitchStatement>(offset, ss.fIsStatic, expr(ss.fValue),
                                                     std::move(cases), ss.fSymbols);
        }
        case Statement::Kind::kVarDeclaration: {
            const VarDeclaration& decl = statement.as<VarDeclaration>();
            std::vector<std::unique_ptr<Expression>> sizes;
            for (const auto& size : decl.fSizes) {
                sizes.push_back(expr(size));
            }
            std::unique_ptr<Expression> initialValue = expr(decl.fValue);
            const Variable* old = decl.fVar;
            // We assign unique names to inlined variables--scopes hide most of the problems in this
            // regard, but see `InlinerAvoidsVariableNameOverlap` for a counterexample where unique
            // names are important.
            auto name = std::make_unique<String>(
                    this->uniqueNameForInlineVar(String(old->name()), symbolTableForStatement));
            const String* namePtr = symbolTableForStatement->takeOwnershipOfString(std::move(name));
            const Type* typePtr = copy_if_needed(&old->type(), *symbolTableForStatement);
            const Variable* clone = symbolTableForStatement->takeOwnershipOfSymbol(
                    std::make_unique<Variable>(offset,
                                               old->fModifiers,
                                               namePtr->c_str(),
                                               typePtr,
                                               isBuiltinCode,
                                               old->fStorage,
                                               initialValue.get()));
            (*varMap)[old] = std::make_unique<VariableReference>(offset, clone);
            return std::make_unique<VarDeclaration>(clone, std::move(sizes),
                                                    std::move(initialValue));
        }
        case Statement::Kind::kVarDeclarations: {
            const VarDeclarations& decls = *statement.as<VarDeclarationsStatement>().fDeclaration;
            std::vector<std::unique_ptr<Statement>> vars;
            vars.reserve(decls.fVars.size());
            for (const auto& var : decls.fVars) {
                vars.push_back(stmt(var));
            }
            const Type* typePtr = copy_if_needed(&decls.fBaseType, *symbolTableForStatement);
            return std::unique_ptr<Statement>(new VarDeclarationsStatement(
                    std::make_unique<VarDeclarations>(offset, typePtr, std::move(vars))));
        }
        case Statement::Kind::kWhile: {
            const WhileStatement& w = statement.as<WhileStatement>();
            return std::make_unique<WhileStatement>(offset, expr(w.fTest), stmt(w.fStatement));
        }
        default:
            SkASSERT(false);
            return nullptr;
    }
}

Inliner::InlinedCall Inliner::inlineCall(FunctionCall* call,
                                         SymbolTable* symbolTableForCall,
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
    SkASSERT(this->isSafeToInline(call->function().fDefinition));

    std::vector<std::unique_ptr<Expression>>& arguments = call->arguments();
    const int offset = call->fOffset;
    const FunctionDefinition& function = *call->function().fDefinition;
    const bool hasEarlyReturn = has_early_return(function);

    InlinedCall inlinedCall;
    inlinedCall.fInlinedBody = std::make_unique<Block>(offset,
                                                       std::vector<std::unique_ptr<Statement>>{},
                                                       /*symbols=*/nullptr,
                                                       /*isScope=*/false);

    Block& inlinedBody = *inlinedCall.fInlinedBody;
    inlinedBody.children().reserve(1 +                // Inline marker
                                   1 +                // Result variable
                                   arguments.size() + // Function arguments (passing in)
                                   arguments.size() + // Function arguments (copy out-params back)
                                   1);                // Inlined code (Block or do-while loop)

    inlinedBody.children().push_back(std::make_unique<InlineMarker>(call->function()));

    auto makeInlineVar =
            [&](const String& baseName, const Type* type, Modifiers modifiers,
                std::unique_ptr<Expression>* initialValue) -> std::unique_ptr<Expression> {
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
        String uniqueName = this->uniqueNameForInlineVar(baseName, symbolTableForCall);
        const String* namePtr = symbolTableForCall->takeOwnershipOfString(
                std::make_unique<String>(std::move(uniqueName)));
        StringFragment nameFrag{namePtr->c_str(), namePtr->length()};

        // Add our new variable to the symbol table.
        auto newVar = std::make_unique<Variable>(/*offset=*/-1, Modifiers(), nameFrag, type,
                                                 caller->fBuiltin, Variable::kLocal_Storage,
                                                 initialValue->get());
        const Variable* variableSymbol = symbolTableForCall->add(nameFrag, std::move(newVar));

        // Prepare the variable declaration (taking extra care with `out` params to not clobber any
        // initial value).
        std::vector<std::unique_ptr<Statement>> variables;
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
        inlinedBody.children().push_back(std::make_unique<VarDeclarationsStatement>(
                std::make_unique<VarDeclarations>(offset, type, std::move(variables))));

        return std::make_unique<VariableReference>(offset, variableSymbol);
    };

    // Create a variable to hold the result in the extra statements (excepting void).
    std::unique_ptr<Expression> resultExpr;
    if (function.fDeclaration.fReturnType != *fContext->fVoid_Type) {
        std::unique_ptr<Expression> noInitialValue;
        resultExpr = makeInlineVar(String(function.fDeclaration.name()),
                                   &function.fDeclaration.fReturnType,
                                   Modifiers{}, &noInitialValue);
   }

    // Create variables in the extra statements to hold the arguments, and assign the arguments to
    // them.
    VariableRewriteMap varMap;
    std::vector<int> argsToCopyBack;
    for (int i = 0; i < (int) arguments.size(); ++i) {
        const Variable* param = function.fDeclaration.fParameters[i];
        bool isOutParam = param->fModifiers.fFlags & Modifiers::kOut_Flag;

        // If this argument can be inlined trivially (e.g. a swizzle, or a constant array index)...
        if (is_trivial_argument(*arguments[i])) {
            // ... and it's an `out` param, or it isn't written to within the inline function...
            if (isOutParam || !Analysis::StatementWritesToVariable(*function.fBody, *param)) {
                // ... we don't need to copy it at all! We can just use the existing expression.
                varMap[param] = arguments[i]->clone();
                continue;
            }
        }

        if (isOutParam) {
            argsToCopyBack.push_back(i);
        }

        varMap[param] = makeInlineVar(String(param->name()), &arguments[i]->type(),
                                      param->fModifiers, &arguments[i]);
    }

    const Block& body = function.fBody->as<Block>();
    auto inlineBlock = std::make_unique<Block>(offset, std::vector<std::unique_ptr<Statement>>{});
    inlineBlock->children().reserve(body.children().size());
    for (const std::unique_ptr<Statement>& stmt : body.children()) {
        inlineBlock->children().push_back(this->inlineStatement(offset, &varMap, symbolTableForCall,
                                                                resultExpr.get(), hasEarlyReturn,
                                                                *stmt, caller->fBuiltin));
    }
    if (hasEarlyReturn) {
        // Since we output to backends that don't have a goto statement (which would normally be
        // used to perform an early return), we fake it by wrapping the function in a
        // do { } while (false); and then use break statements to jump to the end in order to
        // emulate a goto.
        inlinedBody.children().push_back(std::make_unique<DoStatement>(
                /*offset=*/-1,
                std::move(inlineBlock),
                std::make_unique<BoolLiteral>(*fContext, offset, /*value=*/false)));
    } else {
        // No early returns, so we can just dump the code in. We still need to keep the block so we
        // don't get name conflicts with locals.
        inlinedBody.children().push_back(std::move(inlineBlock));
    }

    // Copy back the values of `out` parameters into their real destinations.
    for (int i : argsToCopyBack) {
        const Variable* p = function.fDeclaration.fParameters[i];
        SkASSERT(varMap.find(p) != varMap.end());
        inlinedBody.children().push_back(
                std::make_unique<ExpressionStatement>(std::make_unique<BinaryExpression>(
                        offset,
                        clone_with_ref_kind(*arguments[i], VariableReference::kWrite_RefKind),
                        Token::Kind::TK_EQ,
                        std::move(varMap[p]),
                        &arguments[i]->type())));
    }

    if (resultExpr != nullptr) {
        // Return our result variable as our replacement expression.
        SkASSERT(resultExpr->as<VariableReference>().fRefKind == VariableReference::kRead_RefKind);
        inlinedCall.fReplacementExpr = std::move(resultExpr);
    } else {
        // It's a void function, so it doesn't actually result in anything, but we have to return
        // something non-null as a standin.
        inlinedCall.fReplacementExpr = std::make_unique<BoolLiteral>(*fContext, offset,
                                                                     /*value=*/false);
    }

    return inlinedCall;
}

bool Inliner::isSafeToInline(const FunctionDefinition* functionDef) {
    SkASSERT(fSettings);

    if (functionDef == nullptr) {
        // Can't inline something if we don't actually have its definition.
        return false;
    }

    if (!fSettings->fCaps || !fSettings->fCaps->canUseDoLoops()) {
        // We don't have do-while loops. We use do-while loops to simulate early returns, so we
        // can't inline functions that have an early return.
        bool hasEarlyReturn = has_early_return(*functionDef);

        // If we didn't detect an early return, there shouldn't be any returns in breakable
        // constructs either.
        SkASSERT(hasEarlyReturn || count_returns_in_breakable_constructs(*functionDef) == 0);
        return !hasEarlyReturn;
    }
    // We have do-while loops, but we don't have any mechanism to simulate early returns within a
    // breakable construct (switch/for/do/while), so we can't inline if there's a return inside one.
    bool hasReturnInBreakableConstruct = (count_returns_in_breakable_constructs(*functionDef) > 0);

    // If we detected returns in breakable constructs, we should also detect an early return.
    SkASSERT(!hasReturnInBreakableConstruct || has_early_return(*functionDef));
    return !hasReturnInBreakableConstruct;
}

// A candidate function for inlining, containing everything that `inlineCall` needs.
struct InlineCandidate {
    SymbolTable* fSymbols;                        // the SymbolTable of the candidate
    std::unique_ptr<Statement>* fParentStmt;      // the parent Statement of the enclosing stmt
    std::unique_ptr<Statement>* fEnclosingStmt;   // the Statement containing the candidate
    std::unique_ptr<Expression>* fCandidateExpr;  // the candidate FunctionCall to be inlined
    FunctionDefinition* fEnclosingFunction;       // the Function containing the candidate
    bool fIsLargeFunction;                        // does candidate exceed the inline threshold?
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
    std::vector<SymbolTable*> fSymbolTableStack;
    // A stack of "enclosing" statements--these would be suitable for the inliner to use for adding
    // new instructions. Not all statements are suitable (e.g. a for-loop's initializer). The
    // inliner might replace a statement with a block containing the statement.
    std::vector<std::unique_ptr<Statement>*> fEnclosingStmtStack;
    // The function that we're currently processing (i.e. inlining into).
    FunctionDefinition* fEnclosingFunction = nullptr;

    void visit(Program& program, InlineCandidateList* candidateList) {
        fCandidateList = candidateList;
        fSymbolTableStack.push_back(program.fSymbols.get());

        for (ProgramElement& pe : program) {
            this->visitProgramElement(&pe);
        }

        fSymbolTableStack.pop_back();
        fCandidateList = nullptr;
    }

    void visitProgramElement(ProgramElement* pe) {
        switch (pe->kind()) {
            case ProgramElement::Kind::kFunction: {
                FunctionDefinition& funcDef = pe->as<FunctionDefinition>();
                fEnclosingFunction = &funcDef;
                this->visitStatement(&funcDef.fBody);
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
                    fSymbolTableStack.push_back(block.symbolTable().get());
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
                    fSymbolTableStack.push_back(forStmt.symbols().get());
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
                this->visitExpression(&ifStmt.fTest);
                this->visitStatement(&ifStmt.fIfTrue);
                this->visitStatement(&ifStmt.fIfFalse);
                break;
            }
            case Statement::Kind::kReturn: {
                ReturnStatement& returnStmt = (*stmt)->as<ReturnStatement>();
                this->visitExpression(&returnStmt.fExpression);
                break;
            }
            case Statement::Kind::kSwitch: {
                SwitchStatement& switchStmt = (*stmt)->as<SwitchStatement>();
                if (switchStmt.fSymbols) {
                    fSymbolTableStack.push_back(switchStmt.fSymbols.get());
                }

                this->visitExpression(&switchStmt.fValue);
                for (std::unique_ptr<SwitchCase>& switchCase : switchStmt.fCases) {
                    // The switch-case's fValue cannot be a FunctionCall; skip it.
                    for (std::unique_ptr<Statement>& caseBlock : switchCase->fStatements) {
                        this->visitStatement(&caseBlock);
                    }
                }
                break;
            }
            case Statement::Kind::kVarDeclaration: {
                VarDeclaration& varDeclStmt = (*stmt)->as<VarDeclaration>();
                // Don't need to scan the declaration's sizes; those are always IntLiterals.
                this->visitExpression(&varDeclStmt.fValue);
                break;
            }
            case Statement::Kind::kVarDeclarations: {
                VarDeclarationsStatement& varDecls = (*stmt)->as<VarDeclarationsStatement>();
                for (std::unique_ptr<Statement>& varDecl : varDecls.fDeclaration->fVars) {
                    this->visitStatement(&varDecl, /*isViableAsEnclosingStatement=*/false);
                }
                break;
            }
            case Statement::Kind::kWhile: {
                WhileStatement& whileStmt = (*stmt)->as<WhileStatement>();
                // The loop body is a candidate for inlining.
                this->visitStatement(&whileStmt.fStatement);
                // The inliner isn't smart enough to inline the test-expression for a while loop at
                // this time. There are two limitations:
                // - We would need to insert the inlined-body block at the very beginning of the
                //   while loop's inner fStatement. We don't support that today, but it's doable.
                // - The while-loop's built-in test-expression would need to be replaced with a
                //   `true` BoolLiteral, and the loop would be halted via a break statement at the
                //   end of the inlined test-expression. This is again something we don't support
                //   today, but it could be implemented.
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
                this->visitExpression(&binaryExpr.leftPointer());

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
                    this->visitExpression(&binaryExpr.rightPointer());
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
                this->visitExpression(&indexExpr.fBase);
                this->visitExpression(&indexExpr.fIndex);
                break;
            }
            case Expression::Kind::kPostfix: {
                PostfixExpression& postfixExpr = (*expr)->as<PostfixExpression>();
                this->visitExpression(&postfixExpr.fOperand);
                break;
            }
            case Expression::Kind::kPrefix: {
                PrefixExpression& prefixExpr = (*expr)->as<PrefixExpression>();
                this->visitExpression(&prefixExpr.fOperand);
                break;
            }
            case Expression::Kind::kSwizzle: {
                Swizzle& swizzleExpr = (*expr)->as<Swizzle>();
                this->visitExpression(&swizzleExpr.fBase);
                break;
            }
            case Expression::Kind::kTernary: {
                TernaryExpression& ternaryExpr = (*expr)->as<TernaryExpression>();
                // The test expression is a candidate for inlining.
                this->visitExpression(&ternaryExpr.fTest);
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
                                fEnclosingFunction,
                                /*isLargeFunction=*/false});
    }
};

bool Inliner::candidateCanBeInlined(const InlineCandidate& candidate, InlinabilityCache* cache) {
    const FunctionDeclaration& funcDecl =
                                         (*candidate.fCandidateExpr)->as<FunctionCall>().function();

    auto [iter, wasInserted] = cache->insert({&funcDecl, false});
    if (wasInserted) {
        // Recursion is forbidden here to avoid an infinite death spiral of inlining.
        iter->second = this->isSafeToInline(funcDecl.fDefinition) &&
                       !contains_recursive_call(funcDecl);
    }

    return iter->second;
}

bool Inliner::isLargeFunction(const FunctionDefinition* functionDef) {
    return Analysis::NodeCountExceeds(*functionDef, fSettings->fInlineThreshold);
}

bool Inliner::isLargeFunction(const InlineCandidate& candidate, LargeFunctionCache* cache) {
    const FunctionDeclaration& funcDecl =
                                         (*candidate.fCandidateExpr)->as<FunctionCall>().function();

    auto [iter, wasInserted] = cache->insert({&funcDecl, false});
    if (wasInserted) {
        iter->second = this->isLargeFunction(funcDecl.fDefinition);
    }

    return iter->second;
}

void Inliner::buildCandidateList(Program& program, InlineCandidateList* candidateList) {
    // This is structured much like a ProgramVisitor, but does not actually use ProgramVisitor.
    // The analyzer needs to keep track of the `unique_ptr<T>*` of statements and expressions so
    // that they can later be replaced, and ProgramVisitor does not provide this; it only provides a
    // `const T&`.
    InlineCandidateAnalyzer analyzer;
    analyzer.visit(program, candidateList);

    // Remove candidates that are not safe to inline.
    std::vector<InlineCandidate>& candidates = candidateList->fCandidates;
    InlinabilityCache cache;
    candidates.erase(std::remove_if(candidates.begin(),
                                    candidates.end(),
                                    [&](const InlineCandidate& candidate) {
                                        return !this->candidateCanBeInlined(candidate, &cache);
                                    }),
                     candidates.end());

    // Determine whether each candidate function exceeds our inlining size threshold or not. These
    // can still be valid candidates if they are only called one time, so we don't remove them from
    // the candidate list, but they will not be inlined if they're called more than once.
    LargeFunctionCache largeFunctionCache;
    for (InlineCandidate& candidate : candidates) {
        candidate.fIsLargeFunction = this->isLargeFunction(candidate, &largeFunctionCache);
    }
}

bool Inliner::analyze(Program& program) {
    InlineCandidateList candidateList;
    this->buildCandidateList(program, &candidateList);

    // Inline the candidates where we've determined that it's safe to do so.
    std::unordered_set<const std::unique_ptr<Statement>*> enclosingStmtSet;
    bool madeChanges = false;
    for (const InlineCandidate& candidate : candidateList.fCandidates) {
        FunctionCall& funcCall = (*candidate.fCandidateExpr)->as<FunctionCall>();
        const FunctionDeclaration* funcDecl = &funcCall.function();

        // If the function is large, not marked `inline`, and is called more than once, it's a bad
        // idea to inline it.
        if (candidate.fIsLargeFunction &&
            !(funcDecl->fModifiers.fFlags & Modifiers::kInline_Flag) &&
            funcDecl->fCallCount.load() > 1) {
            continue;
        }

        // Inlining two expressions using the same enclosing statement in the same inlining pass
        // does not work properly. If this happens, skip it; we'll get it in the next pass.
        auto [unusedIter, inserted] = enclosingStmtSet.insert(candidate.fEnclosingStmt);
        if (!inserted) {
            continue;
        }

        // Convert the function call to its inlined equivalent.
        InlinedCall inlinedCall = this->inlineCall(&funcCall, candidate.fSymbols,
                                                   &candidate.fEnclosingFunction->fDeclaration);
        if (inlinedCall.fInlinedBody) {
            // Ensure that the inlined body has a scope if it needs one.
            this->ensureScopedBlocks(inlinedCall.fInlinedBody.get(), candidate.fParentStmt->get());

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
        *candidate.fCandidateExpr = std::move(inlinedCall.fReplacementExpr);
        madeChanges = true;

        // Note that nothing was destroyed except for the FunctionCall. All other nodes should
        // remain valid.
    }

    return madeChanges;
}

}  // namespace SkSL
