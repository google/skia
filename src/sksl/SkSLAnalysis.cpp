/*
 * Copyright 2020 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLAnalysis.h"

#include "include/private/SkFloatingPoint.h"
#include "include/private/SkSLModifiers.h"
#include "include/private/SkSLProgramElement.h"
#include "include/private/SkSLSampleUsage.h"
#include "include/private/SkSLStatement.h"
#include "include/sksl/SkSLErrorReporter.h"
#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLConstantFolder.h"
#include "src/sksl/analysis/SkSLProgramVisitor.h"
#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLProgram.h"
#include "src/sksl/transform/SkSLProgramWriter.h"

// ProgramElements
#include "src/sksl/ir/SkSLExtension.h"
#include "src/sksl/ir/SkSLFunctionDefinition.h"
#include "src/sksl/ir/SkSLInterfaceBlock.h"
#include "src/sksl/ir/SkSLVarDeclarations.h"

// Statements
#include "src/sksl/ir/SkSLBlock.h"
#include "src/sksl/ir/SkSLBreakStatement.h"
#include "src/sksl/ir/SkSLContinueStatement.h"
#include "src/sksl/ir/SkSLDiscardStatement.h"
#include "src/sksl/ir/SkSLDoStatement.h"
#include "src/sksl/ir/SkSLExpressionStatement.h"
#include "src/sksl/ir/SkSLForStatement.h"
#include "src/sksl/ir/SkSLIfStatement.h"
#include "src/sksl/ir/SkSLNop.h"
#include "src/sksl/ir/SkSLReturnStatement.h"
#include "src/sksl/ir/SkSLSwitchStatement.h"

// Expressions
#include "src/sksl/ir/SkSLBinaryExpression.h"
#include "src/sksl/ir/SkSLChildCall.h"
#include "src/sksl/ir/SkSLConstructor.h"
#include "src/sksl/ir/SkSLConstructorDiagonalMatrix.h"
#include "src/sksl/ir/SkSLConstructorMatrixResize.h"
#include "src/sksl/ir/SkSLExternalFunctionCall.h"
#include "src/sksl/ir/SkSLExternalFunctionReference.h"
#include "src/sksl/ir/SkSLFieldAccess.h"
#include "src/sksl/ir/SkSLFunctionCall.h"
#include "src/sksl/ir/SkSLFunctionReference.h"
#include "src/sksl/ir/SkSLIndexExpression.h"
#include "src/sksl/ir/SkSLInlineMarker.h"
#include "src/sksl/ir/SkSLLiteral.h"
#include "src/sksl/ir/SkSLPostfixExpression.h"
#include "src/sksl/ir/SkSLPrefixExpression.h"
#include "src/sksl/ir/SkSLSetting.h"
#include "src/sksl/ir/SkSLSwizzle.h"
#include "src/sksl/ir/SkSLTernaryExpression.h"
#include "src/sksl/ir/SkSLTypeReference.h"
#include "src/sksl/ir/SkSLVariableReference.h"

namespace SkSL {

namespace {

// Visitor that determines the merged SampleUsage for a given child in the program.
class MergeSampleUsageVisitor : public ProgramVisitor {
public:
    MergeSampleUsageVisitor(const Context& context,
                            const Variable& child,
                            bool writesToSampleCoords)
            : fContext(context), fChild(child), fWritesToSampleCoords(writesToSampleCoords) {}

    SampleUsage visit(const Program& program) {
        fUsage = SampleUsage(); // reset to none
        INHERITED::visit(program);
        return fUsage;
    }

    int elidedSampleCoordCount() const { return fElidedSampleCoordCount; }

protected:
    const Context& fContext;
    const Variable& fChild;
    const bool fWritesToSampleCoords;
    SampleUsage fUsage;
    int fElidedSampleCoordCount = 0;

    bool visitExpression(const Expression& e) override {
        // Looking for child(...)
        if (e.is<ChildCall>() && &e.as<ChildCall>().child() == &fChild) {
            // Determine the type of call at this site, and merge it with the accumulated state
            const ExpressionArray& arguments = e.as<ChildCall>().arguments();
            SkASSERT(arguments.size() >= 1);

            const Expression* maybeCoords = arguments[0].get();
            if (maybeCoords->type() == *fContext.fTypes.fFloat2) {
                // If the coords are a direct reference to the program's sample-coords, and those
                // coords are never modified, we can conservatively turn this into PassThrough
                // sampling. In all other cases, we consider it Explicit.
                if (!fWritesToSampleCoords && maybeCoords->is<VariableReference>() &&
                    maybeCoords->as<VariableReference>().variable()->modifiers().fLayout.fBuiltin ==
                            SK_MAIN_COORDS_BUILTIN) {
                    fUsage.merge(SampleUsage::PassThrough());
                    ++fElidedSampleCoordCount;
                } else {
                    fUsage.merge(SampleUsage::Explicit());
                }
            } else {
                // child(inputColor) or child(srcColor, dstColor) -> PassThrough
                fUsage.merge(SampleUsage::PassThrough());
            }
        }

        return INHERITED::visitExpression(e);
    }

    using INHERITED = ProgramVisitor;
};

// Visitor that searches through the program for references to a particular builtin variable
class BuiltinVariableVisitor : public ProgramVisitor {
public:
    BuiltinVariableVisitor(int builtin) : fBuiltin(builtin) {}

    bool visitExpression(const Expression& e) override {
        if (e.is<VariableReference>()) {
            const VariableReference& var = e.as<VariableReference>();
            return var.variable()->modifiers().fLayout.fBuiltin == fBuiltin;
        }
        return INHERITED::visitExpression(e);
    }

    int fBuiltin;

    using INHERITED = ProgramVisitor;
};

// Visitor that searches for child calls from a function other than main()
class SampleOutsideMainVisitor : public ProgramVisitor {
public:
    SampleOutsideMainVisitor() {}

    bool visitExpression(const Expression& e) override {
        if (e.is<ChildCall>()) {
            return true;
        }
        return INHERITED::visitExpression(e);
    }

    bool visitProgramElement(const ProgramElement& p) override {
        return p.is<FunctionDefinition>() &&
               !p.as<FunctionDefinition>().declaration().isMain() &&
               INHERITED::visitProgramElement(p);
    }

    using INHERITED = ProgramVisitor;
};

// Visitor that counts the number of nodes visited
class NodeCountVisitor : public ProgramVisitor {
public:
    NodeCountVisitor(int limit) : fLimit(limit) {}

    int visit(const Statement& s) {
        this->visitStatement(s);
        return fCount;
    }

    bool visitExpression(const Expression& e) override {
        ++fCount;
        return (fCount >= fLimit) || INHERITED::visitExpression(e);
    }

    bool visitProgramElement(const ProgramElement& p) override {
        ++fCount;
        return (fCount >= fLimit) || INHERITED::visitProgramElement(p);
    }

    bool visitStatement(const Statement& s) override {
        ++fCount;
        return (fCount >= fLimit) || INHERITED::visitStatement(s);
    }

private:
    int fCount = 0;
    int fLimit;

    using INHERITED = ProgramVisitor;
};

class VariableWriteVisitor : public ProgramVisitor {
public:
    VariableWriteVisitor(const Variable* var)
        : fVar(var) {}

    bool visit(const Statement& s) {
        return this->visitStatement(s);
    }

    bool visitExpression(const Expression& e) override {
        if (e.is<VariableReference>()) {
            const VariableReference& ref = e.as<VariableReference>();
            if (ref.variable() == fVar &&
                (ref.refKind() == VariableReference::RefKind::kWrite ||
                 ref.refKind() == VariableReference::RefKind::kReadWrite ||
                 ref.refKind() == VariableReference::RefKind::kPointer)) {
                return true;
            }
        }
        return INHERITED::visitExpression(e);
    }

private:
    const Variable* fVar;

    using INHERITED = ProgramVisitor;
};

// If a caller doesn't care about errors, we can use this trivial reporter that just counts up.
class TrivialErrorReporter : public ErrorReporter {
public:
    ~TrivialErrorReporter() override { this->reportPendingErrors({}); }
    void handleError(skstd::string_view, PositionInfo) override {}
};

// This isn't actually using ProgramVisitor, because it only considers a subset of the fields for
// any given expression kind. For instance, when indexing an array (e.g. `x[1]`), we only want to
// know if the base (`x`) is assignable; the index expression (`1`) doesn't need to be.
class IsAssignableVisitor {
public:
    IsAssignableVisitor(ErrorReporter* errors) : fErrors(errors) {}

    bool visit(Expression& expr, Analysis::AssignmentInfo* info) {
        int oldErrorCount = fErrors->errorCount();
        this->visitExpression(expr);
        if (info) {
            info->fAssignedVar = fAssignedVar;
        }
        return fErrors->errorCount() == oldErrorCount;
    }

    void visitExpression(Expression& expr) {
        switch (expr.kind()) {
            case Expression::Kind::kVariableReference: {
                VariableReference& varRef = expr.as<VariableReference>();
                const Variable* var = varRef.variable();
                if (var->modifiers().fFlags & (Modifiers::kConst_Flag | Modifiers::kUniform_Flag)) {
                    fErrors->error(expr.fLine,
                                   "cannot modify immutable variable '" + var->name() + "'");
                } else {
                    SkASSERT(fAssignedVar == nullptr);
                    fAssignedVar = &varRef;
                }
                break;
            }
            case Expression::Kind::kFieldAccess:
                this->visitExpression(*expr.as<FieldAccess>().base());
                break;

            case Expression::Kind::kSwizzle: {
                const Swizzle& swizzle = expr.as<Swizzle>();
                this->checkSwizzleWrite(swizzle);
                this->visitExpression(*swizzle.base());
                break;
            }
            case Expression::Kind::kIndex:
                this->visitExpression(*expr.as<IndexExpression>().base());
                break;

            case Expression::Kind::kPoison:
                break;

            default:
                fErrors->error(expr.fLine, "cannot assign to this expression");
                break;
        }
    }

private:
    void checkSwizzleWrite(const Swizzle& swizzle) {
        int bits = 0;
        for (int8_t idx : swizzle.components()) {
            SkASSERT(idx >= SwizzleComponent::X && idx <= SwizzleComponent::W);
            int bit = 1 << idx;
            if (bits & bit) {
                fErrors->error(swizzle.fLine,
                               "cannot write to the same swizzle field more than once");
                break;
            }
            bits |= bit;
        }
    }

    ErrorReporter* fErrors;
    VariableReference* fAssignedVar = nullptr;

    using INHERITED = ProgramVisitor;
};

}  // namespace

////////////////////////////////////////////////////////////////////////////////
// Analysis

SampleUsage Analysis::GetSampleUsage(const Program& program,
                                     const Variable& child,
                                     bool writesToSampleCoords,
                                     int* elidedSampleCoordCount) {
    MergeSampleUsageVisitor visitor(*program.fContext, child, writesToSampleCoords);
    SampleUsage result = visitor.visit(program);
    if (elidedSampleCoordCount) {
        *elidedSampleCoordCount += visitor.elidedSampleCoordCount();
    }
    return result;
}

bool Analysis::ReferencesBuiltin(const Program& program, int builtin) {
    BuiltinVariableVisitor visitor(builtin);
    return visitor.visit(program);
}

bool Analysis::ReferencesSampleCoords(const Program& program) {
    return Analysis::ReferencesBuiltin(program, SK_MAIN_COORDS_BUILTIN);
}

bool Analysis::ReferencesFragCoords(const Program& program) {
    return Analysis::ReferencesBuiltin(program, SK_FRAGCOORD_BUILTIN);
}

bool Analysis::CallsSampleOutsideMain(const Program& program) {
    SampleOutsideMainVisitor visitor;
    return visitor.visit(program);
}

bool Analysis::DetectVarDeclarationWithoutScope(const Statement& stmt, ErrorReporter* errors) {
    // A variable declaration can create either a lone VarDeclaration or an unscoped Block
    // containing multiple VarDeclaration statements. We need to detect either case.
    const Variable* var;
    if (stmt.is<VarDeclaration>()) {
        // The single-variable case. No blocks at all.
        var = &stmt.as<VarDeclaration>().var();
    } else if (stmt.is<Block>()) {
        // The multiple-variable case: an unscoped, non-empty block...
        const Block& block = stmt.as<Block>();
        if (block.isScope() || block.children().empty()) {
            return false;
        }
        // ... holding a variable declaration.
        const Statement& innerStmt = *block.children().front();
        if (!innerStmt.is<VarDeclaration>()) {
            return false;
        }
        var = &innerStmt.as<VarDeclaration>().var();
    } else {
        // This statement wasn't a variable declaration. No problem.
        return false;
    }

    // Report an error.
    SkASSERT(var);
    if (errors) {
        errors->error(stmt.fLine, "variable '" + var->name() + "' must be created in a scope");
    }
    return true;
}

int Analysis::NodeCountUpToLimit(const FunctionDefinition& function, int limit) {
    return NodeCountVisitor{limit}.visit(*function.body());
}

bool Analysis::StatementWritesToVariable(const Statement& stmt, const Variable& var) {
    return VariableWriteVisitor(&var).visit(stmt);
}

bool Analysis::IsAssignable(Expression& expr, AssignmentInfo* info, ErrorReporter* errors) {
    TrivialErrorReporter trivialErrors;
    return IsAssignableVisitor{errors ? errors : &trivialErrors}.visit(expr, info);
}

bool Analysis::UpdateVariableRefKind(Expression* expr,
                                     VariableReference::RefKind kind,
                                     ErrorReporter* errors) {
    Analysis::AssignmentInfo info;
    if (!Analysis::IsAssignable(*expr, &info, errors)) {
        return false;
    }
    if (!info.fAssignedVar) {
        if (errors) {
            errors->error(expr->fLine, "can't assign to expression '" + expr->description() + "'");
        }
        return false;
    }
    info.fAssignedVar->setRefKind(kind);
    return true;
}

bool Analysis::IsTrivialExpression(const Expression& expr) {
    return expr.is<Literal>() ||
           expr.is<VariableReference>() ||
           (expr.is<Swizzle>() &&
            IsTrivialExpression(*expr.as<Swizzle>().base())) ||
           (expr.is<FieldAccess>() &&
            IsTrivialExpression(*expr.as<FieldAccess>().base())) ||
           (expr.isAnyConstructor() &&
            expr.asAnyConstructor().argumentSpan().size() == 1 &&
            IsTrivialExpression(*expr.asAnyConstructor().argumentSpan().front())) ||
           (expr.isAnyConstructor() &&
            expr.isConstantOrUniform()) ||
           (expr.is<IndexExpression>() &&
            expr.as<IndexExpression>().index()->isIntLiteral() &&
            IsTrivialExpression(*expr.as<IndexExpression>().base()));
}

bool Analysis::IsSameExpressionTree(const Expression& left, const Expression& right) {
    if (left.kind() != right.kind() || left.type() != right.type()) {
        return false;
    }

    // This isn't a fully exhaustive list of expressions by any stretch of the imagination; for
    // instance, `x[y+1] = x[y+1]` isn't detected because we don't look at BinaryExpressions.
    // Since this is intended to be used for optimization purposes, handling the common cases is
    // sufficient.
    switch (left.kind()) {
        case Expression::Kind::kLiteral:
            return left.as<Literal>().value() == right.as<Literal>().value();

        case Expression::Kind::kConstructorArray:
        case Expression::Kind::kConstructorArrayCast:
        case Expression::Kind::kConstructorCompound:
        case Expression::Kind::kConstructorCompoundCast:
        case Expression::Kind::kConstructorDiagonalMatrix:
        case Expression::Kind::kConstructorMatrixResize:
        case Expression::Kind::kConstructorScalarCast:
        case Expression::Kind::kConstructorStruct:
        case Expression::Kind::kConstructorSplat: {
            if (left.kind() != right.kind()) {
                return false;
            }
            const AnyConstructor& leftCtor = left.asAnyConstructor();
            const AnyConstructor& rightCtor = right.asAnyConstructor();
            const auto leftSpan = leftCtor.argumentSpan();
            const auto rightSpan = rightCtor.argumentSpan();
            if (leftSpan.size() != rightSpan.size()) {
                return false;
            }
            for (size_t index = 0; index < leftSpan.size(); ++index) {
                if (!IsSameExpressionTree(*leftSpan[index], *rightSpan[index])) {
                    return false;
                }
            }
            return true;
        }
        case Expression::Kind::kFieldAccess:
            return left.as<FieldAccess>().fieldIndex() == right.as<FieldAccess>().fieldIndex() &&
                   IsSameExpressionTree(*left.as<FieldAccess>().base(),
                                        *right.as<FieldAccess>().base());

        case Expression::Kind::kIndex:
            return IsSameExpressionTree(*left.as<IndexExpression>().index(),
                                        *right.as<IndexExpression>().index()) &&
                   IsSameExpressionTree(*left.as<IndexExpression>().base(),
                                        *right.as<IndexExpression>().base());

        case Expression::Kind::kSwizzle:
            return left.as<Swizzle>().components() == right.as<Swizzle>().components() &&
                   IsSameExpressionTree(*left.as<Swizzle>().base(), *right.as<Swizzle>().base());

        case Expression::Kind::kVariableReference:
            return left.as<VariableReference>().variable() ==
                   right.as<VariableReference>().variable();

        default:
            return false;
    }
}

class ES2IndexingVisitor : public ProgramVisitor {
public:
    ES2IndexingVisitor(ErrorReporter& errors) : fErrors(errors) {}

    bool visitStatement(const Statement& s) override {
        if (s.is<ForStatement>()) {
            const ForStatement& f = s.as<ForStatement>();
            SkASSERT(f.initializer() && f.initializer()->is<VarDeclaration>());
            const Variable* var = &f.initializer()->as<VarDeclaration>().var();
            auto [iter, inserted] = fLoopIndices.insert(var);
            SkASSERT(inserted);
            bool result = this->visitStatement(*f.statement());
            fLoopIndices.erase(iter);
            return result;
        }
        return INHERITED::visitStatement(s);
    }

    bool visitExpression(const Expression& e) override {
        if (e.is<IndexExpression>()) {
            const IndexExpression& i = e.as<IndexExpression>();
            if (!Analysis::IsConstantIndexExpression(*i.index(), &fLoopIndices)) {
                fErrors.error(i.fLine, "index expression must be constant");
                return true;
            }
        }
        return INHERITED::visitExpression(e);
    }

    using ProgramVisitor::visitProgramElement;

private:
    ErrorReporter& fErrors;
    std::set<const Variable*> fLoopIndices;
    using INHERITED = ProgramVisitor;
};

void Analysis::ValidateIndexingForES2(const ProgramElement& pe, ErrorReporter& errors) {
    ES2IndexingVisitor visitor(errors);
    visitor.visitProgramElement(pe);
}

void Analysis::VerifyStaticTestsAndExpressions(const Program& program) {
    class TestsAndExpressions : public ProgramVisitor {
    public:
        TestsAndExpressions(const Context& ctx) : fContext(ctx) {}

        using ProgramVisitor::visitProgramElement;

        bool visitStatement(const Statement& stmt) override {
            if (!fContext.fConfig->fSettings.fPermitInvalidStaticTests) {
                switch (stmt.kind()) {
                    case Statement::Kind::kIf:
                        if (stmt.as<IfStatement>().isStatic()) {
                            fContext.fErrors->error(stmt.fLine, "static if has non-static test");
                        }
                        break;

                    case Statement::Kind::kSwitch:
                        if (stmt.as<SwitchStatement>().isStatic()) {
                            fContext.fErrors->error(stmt.fLine,
                                                    "static switch has non-static test");
                        }
                        break;

                    default:
                        break;
                }
            }
            return INHERITED::visitStatement(stmt);
        }

        bool visitExpression(const Expression& expr) override {
            switch (expr.kind()) {
                case Expression::Kind::kFunctionCall: {
                    const FunctionDeclaration& decl = expr.as<FunctionCall>().function();
                    if (!decl.isBuiltin() && !decl.definition()) {
                        fContext.fErrors->error(expr.fLine, "function '" + decl.description() +
                                                            "' is not defined");
                    }
                    break;
                }
                case Expression::Kind::kExternalFunctionReference:
                case Expression::Kind::kFunctionReference:
                case Expression::Kind::kMethodReference:
                case Expression::Kind::kTypeReference:
                    SkDEBUGFAIL("invalid reference-expr, should have been reported by coerce()");
                    fContext.fErrors->error(expr.fLine, "invalid expression");
                    break;
                default:
                    if (expr.type() == *fContext.fTypes.fInvalid) {
                        fContext.fErrors->error(expr.fLine, "invalid expression");
                    }
                    break;
            }
            return INHERITED::visitExpression(expr);
        }

    private:
        using INHERITED = ProgramVisitor;
        const Context& fContext;
    };

    // Check all of the program's owned elements. (Built-in elements are assumed to be valid.)
    TestsAndExpressions visitor{*program.fContext};
    for (const std::unique_ptr<ProgramElement>& element : program.fOwnedElements) {
        visitor.visitProgramElement(*element);
    }
}

////////////////////////////////////////////////////////////////////////////////
// ProgramVisitor

bool ProgramVisitor::visit(const Program& program) {
    for (const ProgramElement* pe : program.elements()) {
        if (this->visitProgramElement(*pe)) {
            return true;
        }
    }
    return false;
}

template <typename T> bool TProgramVisitor<T>::visitExpression(typename T::Expression& e) {
    switch (e.kind()) {
        case Expression::Kind::kCodeString:
        case Expression::Kind::kExternalFunctionReference:
        case Expression::Kind::kFunctionReference:
        case Expression::Kind::kLiteral:
        case Expression::Kind::kMethodReference:
        case Expression::Kind::kPoison:
        case Expression::Kind::kSetting:
        case Expression::Kind::kTypeReference:
        case Expression::Kind::kVariableReference:
            // Leaf expressions return false
            return false;

        case Expression::Kind::kBinary: {
            auto& b = e.template as<BinaryExpression>();
            return (b.left() && this->visitExpressionPtr(b.left())) ||
                   (b.right() && this->visitExpressionPtr(b.right()));
        }
        case Expression::Kind::kChildCall: {
            // We don't visit the child variable itself, just the arguments
            auto& c = e.template as<ChildCall>();
            for (auto& arg : c.arguments()) {
                if (arg && this->visitExpressionPtr(arg)) { return true; }
            }
            return false;
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
            auto& c = e.asAnyConstructor();
            for (auto& arg : c.argumentSpan()) {
                if (this->visitExpressionPtr(arg)) { return true; }
            }
            return false;
        }
        case Expression::Kind::kExternalFunctionCall: {
            auto& c = e.template as<ExternalFunctionCall>();
            for (auto& arg : c.arguments()) {
                if (this->visitExpressionPtr(arg)) { return true; }
            }
            return false;
        }
        case Expression::Kind::kFieldAccess:
            return this->visitExpressionPtr(e.template as<FieldAccess>().base());

        case Expression::Kind::kFunctionCall: {
            auto& c = e.template as<FunctionCall>();
            for (auto& arg : c.arguments()) {
                if (arg && this->visitExpressionPtr(arg)) { return true; }
            }
            return false;
        }
        case Expression::Kind::kIndex: {
            auto& i = e.template as<IndexExpression>();
            return this->visitExpressionPtr(i.base()) || this->visitExpressionPtr(i.index());
        }
        case Expression::Kind::kPostfix:
            return this->visitExpressionPtr(e.template as<PostfixExpression>().operand());

        case Expression::Kind::kPrefix:
            return this->visitExpressionPtr(e.template as<PrefixExpression>().operand());

        case Expression::Kind::kSwizzle: {
            auto& s = e.template as<Swizzle>();
            return s.base() && this->visitExpressionPtr(s.base());
        }

        case Expression::Kind::kTernary: {
            auto& t = e.template as<TernaryExpression>();
            return this->visitExpressionPtr(t.test()) ||
                   (t.ifTrue() && this->visitExpressionPtr(t.ifTrue())) ||
                   (t.ifFalse() && this->visitExpressionPtr(t.ifFalse()));
        }
        default:
            SkUNREACHABLE;
    }
}

template <typename T> bool TProgramVisitor<T>::visitStatement(typename T::Statement& s) {
    switch (s.kind()) {
        case Statement::Kind::kBreak:
        case Statement::Kind::kContinue:
        case Statement::Kind::kDiscard:
        case Statement::Kind::kInlineMarker:
        case Statement::Kind::kNop:
            // Leaf statements just return false
            return false;

        case Statement::Kind::kBlock:
            for (auto& stmt : s.template as<Block>().children()) {
                if (stmt && this->visitStatementPtr(stmt)) {
                    return true;
                }
            }
            return false;

        case Statement::Kind::kSwitchCase: {
            auto& sc = s.template as<SwitchCase>();
            if (sc.value() && this->visitExpressionPtr(sc.value())) {
                return true;
            }
            return this->visitStatementPtr(sc.statement());
        }
        case Statement::Kind::kDo: {
            auto& d = s.template as<DoStatement>();
            return this->visitExpressionPtr(d.test()) || this->visitStatementPtr(d.statement());
        }
        case Statement::Kind::kExpression:
            return this->visitExpressionPtr(s.template as<ExpressionStatement>().expression());

        case Statement::Kind::kFor: {
            auto& f = s.template as<ForStatement>();
            return (f.initializer() && this->visitStatementPtr(f.initializer())) ||
                   (f.test() && this->visitExpressionPtr(f.test())) ||
                   (f.next() && this->visitExpressionPtr(f.next())) ||
                   this->visitStatementPtr(f.statement());
        }
        case Statement::Kind::kIf: {
            auto& i = s.template as<IfStatement>();
            return (i.test() && this->visitExpressionPtr(i.test())) ||
                   (i.ifTrue() && this->visitStatementPtr(i.ifTrue())) ||
                   (i.ifFalse() && this->visitStatementPtr(i.ifFalse()));
        }
        case Statement::Kind::kReturn: {
            auto& r = s.template as<ReturnStatement>();
            return r.expression() && this->visitExpressionPtr(r.expression());
        }
        case Statement::Kind::kSwitch: {
            auto& sw = s.template as<SwitchStatement>();
            if (this->visitExpressionPtr(sw.value())) {
                return true;
            }
            for (auto& c : sw.cases()) {
                if (this->visitStatementPtr(c)) {
                    return true;
                }
            }
            return false;
        }
        case Statement::Kind::kVarDeclaration: {
            auto& v = s.template as<VarDeclaration>();
            return v.value() && this->visitExpressionPtr(v.value());
        }
        default:
            SkUNREACHABLE;
    }
}

template <typename T> bool TProgramVisitor<T>::visitProgramElement(typename T::ProgramElement& pe) {
    switch (pe.kind()) {
        case ProgramElement::Kind::kExtension:
        case ProgramElement::Kind::kFunctionPrototype:
        case ProgramElement::Kind::kInterfaceBlock:
        case ProgramElement::Kind::kModifiers:
        case ProgramElement::Kind::kStructDefinition:
            // Leaf program elements just return false by default
            return false;

        case ProgramElement::Kind::kFunction:
            return this->visitStatementPtr(pe.template as<FunctionDefinition>().body());

        case ProgramElement::Kind::kGlobalVar:
            return this->visitStatementPtr(pe.template as<GlobalVarDeclaration>().declaration());

        default:
            SkUNREACHABLE;
    }
}

template class TProgramVisitor<ProgramVisitorTypes>;
template class TProgramVisitor<ProgramWriterTypes>;

}  // namespace SkSL
