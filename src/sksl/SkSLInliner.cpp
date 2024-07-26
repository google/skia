/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLInliner.h"

#ifndef SK_ENABLE_OPTIMIZE_SIZE

#include "include/core/SkSpan.h"
#include "include/core/SkTypes.h"
#include "include/private/base/SkTArray.h"
#include "src/base/SkEnumBitMask.h"
#include "src/sksl/SkSLAnalysis.h"
#include "src/sksl/SkSLDefines.h"
#include "src/sksl/SkSLErrorReporter.h"
#include "src/sksl/SkSLOperator.h"
#include "src/sksl/SkSLPosition.h"
#include "src/sksl/analysis/SkSLProgramUsage.h"
#include "src/sksl/ir/SkSLBinaryExpression.h"
#include "src/sksl/ir/SkSLBreakStatement.h"
#include "src/sksl/ir/SkSLChildCall.h"
#include "src/sksl/ir/SkSLConstructor.h"
#include "src/sksl/ir/SkSLConstructorArray.h"
#include "src/sksl/ir/SkSLConstructorArrayCast.h"
#include "src/sksl/ir/SkSLConstructorCompound.h"
#include "src/sksl/ir/SkSLConstructorCompoundCast.h"
#include "src/sksl/ir/SkSLConstructorDiagonalMatrix.h"
#include "src/sksl/ir/SkSLConstructorMatrixResize.h"
#include "src/sksl/ir/SkSLConstructorScalarCast.h"
#include "src/sksl/ir/SkSLConstructorSplat.h"
#include "src/sksl/ir/SkSLConstructorStruct.h"
#include "src/sksl/ir/SkSLContinueStatement.h"
#include "src/sksl/ir/SkSLDiscardStatement.h"
#include "src/sksl/ir/SkSLDoStatement.h"
#include "src/sksl/ir/SkSLEmptyExpression.h"
#include "src/sksl/ir/SkSLExpressionStatement.h"
#include "src/sksl/ir/SkSLFieldAccess.h"
#include "src/sksl/ir/SkSLForStatement.h"
#include "src/sksl/ir/SkSLFunctionCall.h"
#include "src/sksl/ir/SkSLFunctionDeclaration.h"
#include "src/sksl/ir/SkSLFunctionDefinition.h"
#include "src/sksl/ir/SkSLIRNode.h"
#include "src/sksl/ir/SkSLIfStatement.h"
#include "src/sksl/ir/SkSLIndexExpression.h"
#include "src/sksl/ir/SkSLModifierFlags.h"
#include "src/sksl/ir/SkSLNop.h"
#include "src/sksl/ir/SkSLPostfixExpression.h"
#include "src/sksl/ir/SkSLPrefixExpression.h"
#include "src/sksl/ir/SkSLProgramElement.h"
#include "src/sksl/ir/SkSLReturnStatement.h"
#include "src/sksl/ir/SkSLSetting.h"
#include "src/sksl/ir/SkSLStatement.h"
#include "src/sksl/ir/SkSLSwitchCase.h"
#include "src/sksl/ir/SkSLSwitchStatement.h"
#include "src/sksl/ir/SkSLSwizzle.h"
#include "src/sksl/ir/SkSLSymbolTable.h"
#include "src/sksl/ir/SkSLTernaryExpression.h"
#include "src/sksl/ir/SkSLType.h"
#include "src/sksl/ir/SkSLVarDeclarations.h"
#include "src/sksl/ir/SkSLVariable.h"
#include "src/sksl/ir/SkSLVariableReference.h"
#include "src/sksl/transform/SkSLTransform.h"

#include <algorithm>
#include <climits>
#include <cstddef>
#include <memory>
#include <string>
#include <string_view>
#include <utility>

using namespace skia_private;

namespace SkSL {
namespace {

static constexpr int kInlinedStatementLimit = 2500;

static bool is_scopeless_block(Statement* stmt) {
    return stmt->is<Block>() && !stmt->as<Block>().isScope();
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
        if (!is_scopeless_block(stmt->get())) {
            return stmt;
        }
    }

    // There wasn't any parent statement to be found.
    return nullptr;
}

std::unique_ptr<Expression> clone_with_ref_kind(const Expression& expr,
                                                VariableReference::RefKind refKind,
                                                Position pos) {
    std::unique_ptr<Expression> clone = expr.clone(pos);
    Analysis::UpdateVariableRefKind(clone.get(), refKind);
    return clone;
}

}  // namespace

const Variable* Inliner::RemapVariable(const Variable* variable,
                                       const VariableRewriteMap* varMap) {
    std::unique_ptr<Expression>* remap = varMap->find(variable);
    if (!remap) {
        SkDEBUGFAILF("rewrite map does not contain variable '%.*s'",
                     (int)variable->name().size(), variable->name().data());
        return variable;
    }
    Expression* expr = remap->get();
    SkASSERT(expr);
    if (!expr->is<VariableReference>()) {
        SkDEBUGFAILF("rewrite map contains non-variable replacement for '%.*s'",
                     (int)variable->name().size(), variable->name().data());
        return variable;
    }
    return expr->as<VariableReference>().variable();
}

void Inliner::ensureScopedBlocks(Statement* inlinedBody, Statement* parentStmt) {
    // No changes necessary if this statement isn't actually a block.
    if (!inlinedBody || !inlinedBody->is<Block>()) {
        return;
    }

    // No changes necessary if the parent statement doesn't require a scope.
    if (!parentStmt || !(parentStmt->is<IfStatement>() || parentStmt->is<ForStatement>() ||
                         parentStmt->is<DoStatement>() || is_scopeless_block(parentStmt))) {
        return;
    }

    Block& block = inlinedBody->as<Block>();

    // The inliner will create inlined function bodies as a Block containing multiple statements,
    // but no scope. Normally, this is fine, but if this block is used as the statement for a
    // do/for/if/while, the block needs to be scoped for the generated code to match the intent.
    // In the case of Blocks nested inside other Blocks, we add the scope to the outermost block if
    // needed.
    for (Block* nestedBlock = &block;; ) {
        if (nestedBlock->isScope()) {
            // We found an explicit scope; all is well.
            return;
        }
        if (nestedBlock->children().size() == 1 && nestedBlock->children()[0]->is<Block>()) {
            // This block wraps another unscoped block; we need to go deeper.
            nestedBlock = &nestedBlock->children()[0]->as<Block>();
            continue;
        }
        // We found a block containing real statements (not just more blocks), but no scope.
        // Let's add a scope to the outermost block.
        block.setBlockKind(Block::Kind::kBracedScope);
        return;
    }
}

std::unique_ptr<Expression> Inliner::inlineExpression(Position pos,
                                                      VariableRewriteMap* varMap,
                                                      SymbolTable* symbolTableForExpression,
                                                      const Expression& expression) {
    auto expr = [&](const std::unique_ptr<Expression>& e) -> std::unique_ptr<Expression> {
        if (e) {
            return this->inlineExpression(pos, varMap, symbolTableForExpression, *e);
        }
        return nullptr;
    };
    auto argList = [&](const ExpressionArray& originalArgs) -> ExpressionArray {
        ExpressionArray args;
        args.reserve_exact(originalArgs.size());
        for (const std::unique_ptr<Expression>& arg : originalArgs) {
            args.push_back(expr(arg));
        }
        return args;
    };
    auto childRemap = [&](const Variable& var) -> const Variable& {
        // If our variable remapping table contains the passed-in variable...
        if (std::unique_ptr<Expression>* remap = varMap->find(&var)) {
            // ... the remapped expression _must_ be another variable reference.
            // SkSL doesn't allow opaque types to participate in complex expressions.
            if ((*remap)->is<VariableReference>()) {
                const VariableReference& remappedRef = (*remap)->as<VariableReference>();
                return *remappedRef.variable();
            } else {
                SkDEBUGFAILF("Child effect '%.*s' remaps to unexpected expression '%s'",
                             (int)var.name().size(), var.name().data(),
                             (*remap)->description().c_str());
            }
        }

        // There's no remapping for this; return it as-is.
        return var;
    };

    switch (expression.kind()) {
        case Expression::Kind::kBinary: {
            const BinaryExpression& binaryExpr = expression.as<BinaryExpression>();
            return BinaryExpression::Make(*fContext,
                                          pos,
                                          expr(binaryExpr.left()),
                                          binaryExpr.getOperator(),
                                          expr(binaryExpr.right()));
        }
        case Expression::Kind::kEmpty:
            return expression.clone(pos);
        case Expression::Kind::kLiteral:
            return expression.clone(pos);
        case Expression::Kind::kChildCall: {
            const ChildCall& childCall = expression.as<ChildCall>();
            return ChildCall::Make(*fContext,
                                   pos,
                                   childCall.type().clone(*fContext, symbolTableForExpression),
                                   childRemap(childCall.child()),
                                   argList(childCall.arguments()));
        }
        case Expression::Kind::kConstructorArray: {
            const ConstructorArray& ctor = expression.as<ConstructorArray>();
            return ConstructorArray::Make(*fContext,
                                          pos,
                                          *ctor.type().clone(*fContext, symbolTableForExpression),
                                          argList(ctor.arguments()));
        }
        case Expression::Kind::kConstructorArrayCast: {
            const ConstructorArrayCast& ctor = expression.as<ConstructorArrayCast>();
            return ConstructorArrayCast::Make(
                    *fContext,
                    pos,
                    *ctor.type().clone(*fContext, symbolTableForExpression),
                    expr(ctor.argument()));
        }
        case Expression::Kind::kConstructorCompound: {
            const ConstructorCompound& ctor = expression.as<ConstructorCompound>();
            return ConstructorCompound::Make(
                    *fContext,
                    pos,
                    *ctor.type().clone(*fContext, symbolTableForExpression),
                    argList(ctor.arguments()));
        }
        case Expression::Kind::kConstructorCompoundCast: {
            const ConstructorCompoundCast& ctor = expression.as<ConstructorCompoundCast>();
            return ConstructorCompoundCast::Make(
                    *fContext,
                    pos,
                    *ctor.type().clone(*fContext, symbolTableForExpression),
                    expr(ctor.argument()));
        }
        case Expression::Kind::kConstructorDiagonalMatrix: {
            const ConstructorDiagonalMatrix& ctor = expression.as<ConstructorDiagonalMatrix>();
            return ConstructorDiagonalMatrix::Make(
                    *fContext,
                    pos,
                    *ctor.type().clone(*fContext, symbolTableForExpression),
                    expr(ctor.argument()));
        }
        case Expression::Kind::kConstructorMatrixResize: {
            const ConstructorMatrixResize& ctor = expression.as<ConstructorMatrixResize>();
            return ConstructorMatrixResize::Make(
                    *fContext,
                    pos,
                    *ctor.type().clone(*fContext, symbolTableForExpression),
                    expr(ctor.argument()));
        }
        case Expression::Kind::kConstructorScalarCast: {
            const ConstructorScalarCast& ctor = expression.as<ConstructorScalarCast>();
            return ConstructorScalarCast::Make(
                    *fContext,
                    pos,
                    *ctor.type().clone(*fContext, symbolTableForExpression),
                    expr(ctor.argument()));
        }
        case Expression::Kind::kConstructorSplat: {
            const ConstructorSplat& ctor = expression.as<ConstructorSplat>();
            return ConstructorSplat::Make(*fContext,
                                          pos,
                                          *ctor.type().clone(*fContext, symbolTableForExpression),
                                          expr(ctor.argument()));
        }
        case Expression::Kind::kConstructorStruct: {
            const ConstructorStruct& ctor = expression.as<ConstructorStruct>();
            return ConstructorStruct::Make(*fContext,
                                           pos,
                                           *ctor.type().clone(*fContext, symbolTableForExpression),
                                           argList(ctor.arguments()));
        }
        case Expression::Kind::kFieldAccess: {
            const FieldAccess& f = expression.as<FieldAccess>();
            return FieldAccess::Make(*fContext, pos, expr(f.base()), f.fieldIndex(), f.ownerKind());
        }
        case Expression::Kind::kFunctionCall: {
            const FunctionCall& funcCall = expression.as<FunctionCall>();
            return FunctionCall::Make(*fContext,
                                      pos,
                                      funcCall.type().clone(*fContext, symbolTableForExpression),
                                      funcCall.function(),
                                      argList(funcCall.arguments()));
        }
        case Expression::Kind::kFunctionReference:
            return expression.clone(pos);
        case Expression::Kind::kIndex: {
            const IndexExpression& idx = expression.as<IndexExpression>();
            return IndexExpression::Make(*fContext, pos, expr(idx.base()), expr(idx.index()));
        }
        case Expression::Kind::kMethodReference:
            return expression.clone(pos);
        case Expression::Kind::kPrefix: {
            const PrefixExpression& p = expression.as<PrefixExpression>();
            return PrefixExpression::Make(*fContext, pos, p.getOperator(), expr(p.operand()));
        }
        case Expression::Kind::kPostfix: {
            const PostfixExpression& p = expression.as<PostfixExpression>();
            return PostfixExpression::Make(*fContext, pos, expr(p.operand()), p.getOperator());
        }
        case Expression::Kind::kSetting: {
            const Setting& s = expression.as<Setting>();
            return Setting::Make(*fContext, pos, s.capsPtr());
        }
        case Expression::Kind::kSwizzle: {
            const Swizzle& s = expression.as<Swizzle>();
            return Swizzle::Make(*fContext, pos, expr(s.base()), s.components());
        }
        case Expression::Kind::kTernary: {
            const TernaryExpression& t = expression.as<TernaryExpression>();
            return TernaryExpression::Make(*fContext, pos, expr(t.test()),
                                           expr(t.ifTrue()), expr(t.ifFalse()));
        }
        case Expression::Kind::kTypeReference:
            return expression.clone(pos);
        case Expression::Kind::kVariableReference: {
            const VariableReference& v = expression.as<VariableReference>();
            std::unique_ptr<Expression>* remap = varMap->find(v.variable());
            if (remap) {
                return clone_with_ref_kind(**remap, v.refKind(), pos);
            }
            return expression.clone(pos);
        }
        default:
            SkDEBUGFAILF("unsupported expression: %s", expression.description().c_str());
            return nullptr;
    }
}

std::unique_ptr<Statement> Inliner::inlineStatement(Position pos,
                                                    VariableRewriteMap* varMap,
                                                    SymbolTable* symbolTableForStatement,
                                                    std::unique_ptr<Expression>* resultExpr,
                                                    Analysis::ReturnComplexity returnComplexity,
                                                    const Statement& statement,
                                                    const ProgramUsage& usage,
                                                    bool isBuiltinCode) {
    auto stmt = [&](const std::unique_ptr<Statement>& s) -> std::unique_ptr<Statement> {
        if (s) {
            return this->inlineStatement(pos, varMap, symbolTableForStatement, resultExpr,
                                         returnComplexity, *s, usage, isBuiltinCode);
        }
        return nullptr;
    };
    auto expr = [&](const std::unique_ptr<Expression>& e) -> std::unique_ptr<Expression> {
        if (e) {
            return this->inlineExpression(pos, varMap, symbolTableForStatement, *e);
        }
        return nullptr;
    };
    auto variableModifiers = [&](const Variable& variable,
                                 const Expression* initialValue) -> ModifierFlags {
        return Transform::AddConstToVarModifiers(variable, initialValue, &usage);
    };
    auto makeWithChildSymbolTable = [&](auto callback) -> std::unique_ptr<Statement> {
        SymbolTable* origSymbolTable = symbolTableForStatement;
        auto childSymbols = std::make_unique<SymbolTable>(origSymbolTable, isBuiltinCode);
        symbolTableForStatement = childSymbols.get();

        std::unique_ptr<Statement> stmt = callback(std::move(childSymbols));

        symbolTableForStatement = origSymbolTable;
        return stmt;
    };

    ++fInlinedStatementCounter;

    switch (statement.kind()) {
        case Statement::Kind::kBlock:
            return makeWithChildSymbolTable([&](std::unique_ptr<SymbolTable> symbolTable) {
                const Block& block = statement.as<Block>();
                StatementArray statements;
                statements.reserve_exact(block.children().size());
                for (const std::unique_ptr<Statement>& child : block.children()) {
                    statements.push_back(stmt(child));
                }
                return Block::Make(pos,
                                   std::move(statements),
                                   block.blockKind(),
                                   std::move(symbolTable));
            });

        case Statement::Kind::kBreak:
            return BreakStatement::Make(pos);

        case Statement::Kind::kContinue:
            return ContinueStatement::Make(pos);

        case Statement::Kind::kDiscard:
            return DiscardStatement::Make(*fContext, pos);

        case Statement::Kind::kDo: {
            const DoStatement& d = statement.as<DoStatement>();
            return DoStatement::Make(*fContext, pos, stmt(d.statement()), expr(d.test()));
        }
        case Statement::Kind::kExpression: {
            const ExpressionStatement& e = statement.as<ExpressionStatement>();
            return ExpressionStatement::Make(*fContext, expr(e.expression()));
        }
        case Statement::Kind::kFor:
            return makeWithChildSymbolTable([&](std::unique_ptr<SymbolTable> symbolTable) {
                const ForStatement& f = statement.as<ForStatement>();
                // We need to ensure `initializer` is evaluated first, so that we've already
                // remapped its declaration by the time we evaluate `test` and `next`.
                std::unique_ptr<Statement> initializerStmt = stmt(f.initializer());
                std::unique_ptr<Expression> testExpr = expr(f.test());
                std::unique_ptr<Expression> nextExpr = expr(f.next());
                std::unique_ptr<Statement> bodyStmt = stmt(f.statement());

                std::unique_ptr<LoopUnrollInfo> unrollInfo;
                if (f.unrollInfo()) {
                    // The for loop's unroll-info points to the Variable in the initializer as the
                    // index. This variable has been rewritten into a clone by the inliner, so we
                    // need to update the loop-unroll info to point to the clone.
                    unrollInfo = std::make_unique<LoopUnrollInfo>(*f.unrollInfo());
                    unrollInfo->fIndex = RemapVariable(unrollInfo->fIndex, varMap);
                }

                return ForStatement::Make(*fContext, pos, ForLoopPositions{},
                                          std::move(initializerStmt),
                                          std::move(testExpr),
                                          std::move(nextExpr),
                                          std::move(bodyStmt),
                                          std::move(unrollInfo),
                                          std::move(symbolTable));
            });

        case Statement::Kind::kIf: {
            const IfStatement& i = statement.as<IfStatement>();
            return IfStatement::Make(*fContext, pos, expr(i.test()),
                                     stmt(i.ifTrue()), stmt(i.ifFalse()));
        }
        case Statement::Kind::kNop:
            return Nop::Make();

        case Statement::Kind::kReturn: {
            const ReturnStatement& r = statement.as<ReturnStatement>();
            if (!r.expression()) {
                // This function doesn't return a value. We won't inline functions with early
                // returns, so a return statement is a no-op and can be treated as such.
                return Nop::Make();
            }

            // If a function only contains a single return, and it doesn't reference variables from
            // inside an Block's scope, we don't need to store the result in a variable at all. Just
            // replace the function-call expression with the function's return expression.
            SkASSERT(resultExpr);
            if (returnComplexity <= Analysis::ReturnComplexity::kSingleSafeReturn) {
                *resultExpr = expr(r.expression());
                return Nop::Make();
            }

            // For more complex functions, we assign their result into a variable. We refuse to
            // inline anything with early returns, so this should be safe to do; that is, on this
            // control path, this is the last statement that will occur.
            SkASSERT(*resultExpr);
            return ExpressionStatement::Make(
                    *fContext,
                    BinaryExpression::Make(
                            *fContext,
                            pos,
                            clone_with_ref_kind(**resultExpr, VariableRefKind::kWrite, pos),
                            Operator::Kind::EQ,
                            expr(r.expression())));
        }
        case Statement::Kind::kSwitch: {
            const SwitchStatement& ss = statement.as<SwitchStatement>();
            return SwitchStatement::Make(*fContext, pos, expr(ss.value()), stmt(ss.caseBlock()));
        }
        case Statement::Kind::kSwitchCase: {
            const SwitchCase& sc = statement.as<SwitchCase>();
            return sc.isDefault() ? SwitchCase::MakeDefault(pos, stmt(sc.statement()))
                                  : SwitchCase::Make(pos, sc.value(), stmt(sc.statement()));
        }
        case Statement::Kind::kVarDeclaration: {
            const VarDeclaration& decl = statement.as<VarDeclaration>();
            std::unique_ptr<Expression> initialValue = expr(decl.value());
            const Variable* variable = decl.var();

            // We assign unique names to inlined variables--scopes hide most of the problems in this
            // regard, but see `InlinerAvoidsVariableNameOverlap` for a counterexample where unique
            // names are important.
            const std::string* name = symbolTableForStatement->takeOwnershipOfString(
                    fMangler.uniqueName(variable->name(), symbolTableForStatement));
            auto clonedVar =
                    Variable::Make(pos,
                                   variable->modifiersPosition(),
                                   variable->layout(),
                                   variableModifiers(*variable, initialValue.get()),
                                   variable->type().clone(*fContext, symbolTableForStatement),
                                   name->c_str(),
                                   /*mangledName=*/"",
                                   isBuiltinCode,
                                   variable->storage());
            varMap->set(variable, VariableReference::Make(pos, clonedVar.get()));
            std::unique_ptr<Statement> result =
                    VarDeclaration::Make(*fContext,
                                         clonedVar.get(),
                                         decl.baseType().clone(*fContext, symbolTableForStatement),
                                         decl.arraySize(),
                                         std::move(initialValue));
            symbolTableForStatement->add(*fContext, std::move(clonedVar));
            return result;
        }
        default:
            SkASSERT(false);
            return nullptr;
    }
}

static bool argument_needs_scratch_variable(const Expression* arg,
                                            const Variable* param,
                                            const ProgramUsage& usage) {
    // If the parameter isn't written to within the inline function ...
    const ProgramUsage::VariableCounts& paramUsage = usage.get(*param);
    if (!paramUsage.fWrite) {
        // ... and it can be inlined trivially (e.g. a swizzle, or a constant array index),
        // or it is any expression without side effects that is only accessed at most once...
        if ((paramUsage.fRead > 1) ? Analysis::IsTrivialExpression(*arg)
                                   : !Analysis::HasSideEffects(*arg)) {
            // ... we don't need to copy it at all! We can just use the existing expression.
            return false;
        }
    }
    // We need a scratch variable.
    return true;
}

Inliner::InlinedCall Inliner::inlineCall(const FunctionCall& call,
                                         SymbolTable* symbolTable,
                                         const ProgramUsage& usage,
                                         const FunctionDeclaration* caller) {
    using ScratchVariable = Variable::ScratchVariable;

    // Inlining is more complicated here than in a typical compiler, because we have to have a
    // high-level IR and can't just drop statements into the middle of an expression or even use
    // gotos.
    //
    // Since we can't insert statements into an expression, we run the inline function as extra
    // statements before the statement we're currently processing, relying on a lack of execution
    // order guarantees.
    SkASSERT(fContext);
    SkASSERT(this->isSafeToInline(call.function().definition(), usage));

    const ExpressionArray& arguments = call.arguments();
    const Position pos = call.fPosition;
    const FunctionDefinition& function = *call.function().definition();
    const Block& body = function.body()->as<Block>();
    const Analysis::ReturnComplexity returnComplexity = Analysis::GetReturnComplexity(function);

    StatementArray inlineStatements;
    int expectedStmtCount = 1 +                      // Result variable
                            arguments.size() +       // Function argument temp-vars
                            body.children().size();  // Inlined code

    inlineStatements.reserve_exact(expectedStmtCount);

    std::unique_ptr<Expression> resultExpr;
    if (returnComplexity > Analysis::ReturnComplexity::kSingleSafeReturn &&
        !function.declaration().returnType().isVoid()) {
        // Create a variable to hold the result in the extra statements. We don't need to do this
        // for void-return functions, or in cases that are simple enough that we can just replace
        // the function-call node with the result expression.
        ScratchVariable var = Variable::MakeScratchVariable(*fContext,
                                                            fMangler,
                                                            function.declaration().name(),
                                                            &function.declaration().returnType(),
                                                            symbolTable,
                                                            /*initialValue=*/nullptr);
        inlineStatements.push_back(std::move(var.fVarDecl));
        resultExpr = VariableReference::Make(Position(), var.fVarSymbol);
    }

    // Create variables in the extra statements to hold the arguments, and assign the arguments to
    // them.
    VariableRewriteMap varMap;
    for (int i = 0; i < arguments.size(); ++i) {
        const Expression* arg = arguments[i].get();
        const Variable* param = function.declaration().parameters()[i];
        if (!argument_needs_scratch_variable(arg, param, usage)) {
            varMap.set(param, arg->clone());
            continue;
        }
        ScratchVariable var = Variable::MakeScratchVariable(*fContext,
                                                            fMangler,
                                                            param->name(),
                                                            &arg->type(),
                                                            symbolTable,
                                                            arg->clone());
        inlineStatements.push_back(std::move(var.fVarDecl));
        varMap.set(param, VariableReference::Make(Position(), var.fVarSymbol));
    }

    for (const std::unique_ptr<Statement>& stmt : body.children()) {
        inlineStatements.push_back(this->inlineStatement(pos, &varMap, symbolTable,
                                                         &resultExpr, returnComplexity, *stmt,
                                                         usage, caller->isBuiltin()));
    }

    SkASSERT(inlineStatements.size() <= expectedStmtCount);

    // Wrap all of the generated statements in a block. We need a real Block here, because we need
    // to add another child statement to the Block later.
    InlinedCall inlinedCall;
    inlinedCall.fInlinedBody = Block::MakeBlock(pos, std::move(inlineStatements),
                                                Block::Kind::kUnbracedBlock);
    if (resultExpr) {
        // Return our result expression as-is.
        inlinedCall.fReplacementExpr = std::move(resultExpr);
    } else if (function.declaration().returnType().isVoid()) {
        // It's a void function, so its result is the empty expression.
        inlinedCall.fReplacementExpr = EmptyExpression::Make(pos, *fContext);
    } else {
        // It's a non-void function, but it never created a result expression--that is, it never
        // returned anything on any path! This should have been detected in the function finalizer.
        // Still, discard our output and generate an error.
        SkDEBUGFAIL("inliner found non-void function that fails to return a value on any path");
        fContext->fErrors->error(function.fPosition, "inliner found non-void function '" +
                                                     std::string(function.declaration().name()) +
                                                     "' that fails to return a value on any path");
        inlinedCall = {};
    }

    return inlinedCall;
}

bool Inliner::isSafeToInline(const FunctionDefinition* functionDef, const ProgramUsage& usage) {
    // A threshold of zero indicates that the inliner is completely disabled, so we can just return.
    if (this->settings().fInlineThreshold <= 0) {
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

    if (functionDef->declaration().modifierFlags().isNoInline()) {
        // Refuse to inline functions decorated with `noinline`.
        return false;
    }

    for (const Variable* param : functionDef->declaration().parameters()) {
        // We don't allow inlining functions with parameters that are written-to, if they...
        // - are `out` parameters (see skia:11326 for rationale.)
        // - are arrays or structures (introducing temporary copies is non-trivial)
        if ((param->modifierFlags() & ModifierFlag::kOut) ||
            param->type().isArray() ||
            param->type().isStruct()) {
            ProgramUsage::VariableCounts counts = usage.get(*param);
            if (counts.fWrite > 0) {
                return false;
            }
        }
    }

    // We don't have a mechanism to simulate early returns, so we can't inline if there is one.
    return Analysis::GetReturnComplexity(*functionDef) < Analysis::ReturnComplexity::kEarlyReturns;
}

// A candidate function for inlining, containing everything that `inlineCall` needs.
struct InlineCandidate {
    SymbolTable* fSymbols;                        // the SymbolTable of the candidate
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
    std::vector<SymbolTable*> fSymbolTableStack;
    // A stack of "enclosing" statements--these would be suitable for the inliner to use for adding
    // new instructions. Not all statements are suitable (e.g. a for-loop's initializer). The
    // inliner might replace a statement with a block containing the statement.
    std::vector<std::unique_ptr<Statement>*> fEnclosingStmtStack;
    // The function that we're currently processing (i.e. inlining into).
    FunctionDefinition* fEnclosingFunction = nullptr;

    void visit(const std::vector<std::unique_ptr<ProgramElement>>& elements,
               SymbolTable* symbols,
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

                // If this function has parameter names that would shadow globally-scoped names, we
                // don't scan it for inline candidates, because it's too late to mangle the names.
                bool foundShadowingParameterName = false;
                for (const Variable* param : funcDef.declaration().parameters()) {
                    if (fSymbolTableStack.front()->find(param->name())) {
                        foundShadowingParameterName = true;
                        break;
                    }
                }

                if (!foundShadowingParameterName) {
                    fEnclosingFunction = &funcDef;
                    this->visitStatement(&funcDef.body());
                }
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

        Analysis::SymbolTableStackBuilder scopedStackBuilder(stmt->get(), &fSymbolTableStack);
        // If this statement contains symbols that would shadow globally-scoped names, we don't look
        // for any inline candidates, because it's too late to mangle the names.
        if (scopedStackBuilder.foundSymbolTable() &&
            fSymbolTableStack.back()->wouldShadowSymbolsFrom(fSymbolTableStack.front())) {
            return;
        }

        size_t oldEnclosingStmtStackSize = fEnclosingStmtStack.size();

        if (isViableAsEnclosingStatement) {
            fEnclosingStmtStack.push_back(stmt);
        }

        switch ((*stmt)->kind()) {
            case Statement::Kind::kBreak:
            case Statement::Kind::kContinue:
            case Statement::Kind::kDiscard:
            case Statement::Kind::kNop:
                break;

            case Statement::Kind::kBlock: {
                Block& block = (*stmt)->as<Block>();
                for (std::unique_ptr<Statement>& blockStmt : block.children()) {
                    this->visitStatement(&blockStmt);
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
                this->visitExpression(&switchStmt.value());
                for (const std::unique_ptr<Statement>& switchCase : switchStmt.cases()) {
                    // The switch-case's fValue cannot be a FunctionCall; skip it.
                    this->visitStatement(&switchCase->as<SwitchCase>().statement());
                }
                break;
            }
            case Statement::Kind::kVarDeclaration: {
                VarDeclaration& varDeclStmt = (*stmt)->as<VarDeclaration>();
                // Don't need to scan the declaration's sizes; those are always literals.
                this->visitExpression(&varDeclStmt.value());
                break;
            }
            default:
                SkUNREACHABLE;
        }

        // Pop our symbol and enclosing-statement stacks.
        fEnclosingStmtStack.resize(oldEnclosingStmtStackSize);
    }

    void visitExpression(std::unique_ptr<Expression>* expr) {
        if (!*expr) {
            return;
        }

        switch ((*expr)->kind()) {
            case Expression::Kind::kFieldAccess:
            case Expression::Kind::kFunctionReference:
            case Expression::Kind::kLiteral:
            case Expression::Kind::kMethodReference:
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
                Operator op = binaryExpr.getOperator();
                bool shortCircuitable = (op.kind() == Operator::Kind::LOGICALAND ||
                                         op.kind() == Operator::Kind::LOGICALOR);
                if (!shortCircuitable) {
                    this->visitExpression(&binaryExpr.right());
                }
                break;
            }
            case Expression::Kind::kChildCall: {
                ChildCall& childCallExpr = (*expr)->as<ChildCall>();
                for (std::unique_ptr<Expression>& arg : childCallExpr.arguments()) {
                    this->visitExpression(&arg);
                }
                break;
            }
            case Expression::Kind::kConstructorArray:
            case Expression::Kind::kConstructorArrayCast:
            case Expression::Kind::kConstructorCompound:
            case Expression::Kind::kConstructorCompoundCast:
            case Expression::Kind::kConstructorDiagonalMatrix:
            case Expression::Kind::kConstructorMatrixResize:
            case Expression::Kind::kConstructorScalarCast:
            case Expression::Kind::kConstructorSplat:
            case Expression::Kind::kConstructorStruct: {
                AnyConstructor& constructorExpr = (*expr)->asAnyConstructor();
                for (std::unique_ptr<Expression>& arg : constructorExpr.argumentSpan()) {
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
            case Expression::Kind::kIndex: {
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

bool Inliner::functionCanBeInlined(const FunctionDeclaration& funcDecl,
                                   const ProgramUsage& usage,
                                   InlinabilityCache* cache) {
    if (const bool* cachedInlinability = cache->find(&funcDecl)) {
        return *cachedInlinability;
    }
    bool inlinability = this->isSafeToInline(funcDecl.definition(), usage);
    cache->set(&funcDecl, inlinability);
    return inlinability;
}

bool Inliner::candidateCanBeInlined(const InlineCandidate& candidate,
                                    const ProgramUsage& usage,
                                    InlinabilityCache* cache) {
    // Check the cache to see if this function is safe to inline.
    const FunctionDeclaration& funcDecl = candidate_func(candidate);
    if (!this->functionCanBeInlined(funcDecl, usage, cache)) {
        return false;
    }

    // Even if the function is safe, the arguments we are passing may not be. In particular, we
    // can't make copies of opaque values, so we need to reject inline candidates that would need to
    // do this. Every call has different arguments, so this part is not cacheable. (skia:13824)
    const FunctionCall& call = candidate.fCandidateExpr->get()->as<FunctionCall>();
    const ExpressionArray& arguments = call.arguments();
    for (int i = 0; i < arguments.size(); ++i) {
        const Expression* arg = arguments[i].get();
        if (arg->type().isOpaque()) {
            const Variable* param = funcDecl.parameters()[i];
            if (argument_needs_scratch_variable(arg, param, usage)) {
                return false;
            }
        }
    }

    return true;
}

int Inliner::getFunctionSize(const FunctionDeclaration& funcDecl, FunctionSizeCache* cache) {
    if (const int* cachedSize = cache->find(&funcDecl)) {
        return *cachedSize;
    }
    int size = Analysis::NodeCountUpToLimit(*funcDecl.definition(),
                                            this->settings().fInlineThreshold);
    cache->set(&funcDecl, size);
    return size;
}

void Inliner::buildCandidateList(const std::vector<std::unique_ptr<ProgramElement>>& elements,
                                 SymbolTable* symbols,
                                 ProgramUsage* usage,
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
                                        return !this->candidateCanBeInlined(
                                                candidate, *usage, &cache);
                                    }),
                     candidates.end());

    // If the inline threshold is unlimited, or if we have no candidates left, our candidate list is
    // complete.
    if (this->settings().fInlineThreshold == INT_MAX || candidates.empty()) {
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

    candidates.erase(std::remove_if(candidates.begin(), candidates.end(),
                        [&](const InlineCandidate& candidate) {
                            const FunctionDeclaration& fnDecl = candidate_func(candidate);
                            if (fnDecl.modifierFlags().isInline()) {
                                // Functions marked `inline` ignore size limitations.
                                return false;
                            }
                            if (usage->get(fnDecl) == 1) {
                                // If a function is only used once, it's cost-free to inline.
                                return false;
                            }
                            if (candidateTotalCost[&fnDecl] <= this->settings().fInlineThreshold) {
                                // We won't exceed the inline threshold by inlining this.
                                return false;
                            }
                            // Inlining this function will add too many IRNodes.
                            return true;
                        }),
         candidates.end());
}

bool Inliner::analyze(const std::vector<std::unique_ptr<ProgramElement>>& elements,
                      SymbolTable* symbols,
                      ProgramUsage* usage) {
    // A threshold of zero indicates that the inliner is completely disabled, so we can just return.
    if (this->settings().fInlineThreshold <= 0) {
        return false;
    }

    // Enforce a limit on inlining to avoid pathological cases. (inliner/ExponentialGrowth.sksl)
    if (fInlinedStatementCounter >= kInlinedStatementLimit) {
        return false;
    }

    InlineCandidateList candidateList;
    this->buildCandidateList(elements, symbols, usage, &candidateList);

    // Inline the candidates where we've determined that it's safe to do so.
    using StatementRemappingTable = THashMap<std::unique_ptr<Statement>*,
                                             std::unique_ptr<Statement>*>;
    StatementRemappingTable statementRemappingTable;

    bool madeChanges = false;
    for (const InlineCandidate& candidate : candidateList.fCandidates) {
        const FunctionCall& funcCall = (*candidate.fCandidateExpr)->as<FunctionCall>();

        // Convert the function call to its inlined equivalent.
        InlinedCall inlinedCall = this->inlineCall(funcCall, candidate.fSymbols, *usage,
                                                   &candidate.fEnclosingFunction->declaration());

        // Stop if an error was detected during the inlining process.
        if (!inlinedCall.fInlinedBody && !inlinedCall.fReplacementExpr) {
            break;
        }

        // Ensure that the inlined body has a scope if it needs one.
        this->ensureScopedBlocks(inlinedCall.fInlinedBody.get(), candidate.fParentStmt->get());

        // Add references within the inlined body
        usage->add(inlinedCall.fInlinedBody.get());

        // Look up the enclosing statement; remap it if necessary.
        std::unique_ptr<Statement>* enclosingStmt = candidate.fEnclosingStmt;
        for (;;) {
            std::unique_ptr<Statement>** remappedStmt = statementRemappingTable.find(enclosingStmt);
            if (!remappedStmt) {
                break;
            }
            enclosingStmt = *remappedStmt;
        }

        // Move the enclosing statement to the end of the unscoped Block containing the inlined
        // function, then replace the enclosing statement with that Block.
        // Before:
        //     fInlinedBody = Block{ stmt1, stmt2, stmt3 }
        //     fEnclosingStmt = stmt4
        // After:
        //     fInlinedBody = null
        //     fEnclosingStmt = Block{ stmt1, stmt2, stmt3, stmt4 }
        inlinedCall.fInlinedBody->children().push_back(std::move(*enclosingStmt));
        *enclosingStmt = std::move(inlinedCall.fInlinedBody);

        // Replace the candidate function call with our replacement expression.
        usage->remove(candidate.fCandidateExpr->get());
        usage->add(inlinedCall.fReplacementExpr.get());
        *candidate.fCandidateExpr = std::move(inlinedCall.fReplacementExpr);
        madeChanges = true;

        // If anything else pointed at our enclosing statement, it's now pointing at a Block
        // containing many other statements as well. Maintain a fix-up table to account for this.
        statementRemappingTable.set(enclosingStmt,&(*enclosingStmt)->as<Block>().children().back());

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

#endif  // SK_ENABLE_OPTIMIZE_SIZE
