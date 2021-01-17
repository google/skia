/*
 * Copyright 2020 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLAnalysis.h"

#include "include/private/SkSLSampleUsage.h"
#include "src/sksl/SkSLErrorReporter.h"
#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLProgram.h"
#include "src/sksl/ir/SkSLProgramElement.h"
#include "src/sksl/ir/SkSLStatement.h"

// ProgramElements
#include "src/sksl/ir/SkSLEnum.h"
#include "src/sksl/ir/SkSLExtension.h"
#include "src/sksl/ir/SkSLFunctionDefinition.h"
#include "src/sksl/ir/SkSLInterfaceBlock.h"
#include "src/sksl/ir/SkSLModifiers.h"
#include "src/sksl/ir/SkSLSection.h"
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
#include "src/sksl/ir/SkSLBoolLiteral.h"
#include "src/sksl/ir/SkSLConstructor.h"
#include "src/sksl/ir/SkSLExternalFunctionCall.h"
#include "src/sksl/ir/SkSLExternalFunctionReference.h"
#include "src/sksl/ir/SkSLFieldAccess.h"
#include "src/sksl/ir/SkSLFloatLiteral.h"
#include "src/sksl/ir/SkSLFunctionCall.h"
#include "src/sksl/ir/SkSLFunctionReference.h"
#include "src/sksl/ir/SkSLIndexExpression.h"
#include "src/sksl/ir/SkSLInlineMarker.h"
#include "src/sksl/ir/SkSLIntLiteral.h"
#include "src/sksl/ir/SkSLPostfixExpression.h"
#include "src/sksl/ir/SkSLPrefixExpression.h"
#include "src/sksl/ir/SkSLSetting.h"
#include "src/sksl/ir/SkSLSwizzle.h"
#include "src/sksl/ir/SkSLTernaryExpression.h"
#include "src/sksl/ir/SkSLTypeReference.h"
#include "src/sksl/ir/SkSLVariableReference.h"

namespace SkSL {

namespace {

static bool is_sample_call_to_fp(const FunctionCall& fc, const Variable& fp) {
    const FunctionDeclaration& f = fc.function();
    return f.isBuiltin() && f.name() == "sample" && fc.arguments().size() >= 1 &&
           fc.arguments()[0]->is<VariableReference>() &&
           fc.arguments()[0]->as<VariableReference>().variable() == &fp;
}

// Visitor that determines the merged SampleUsage for a given child 'fp' in the program.
class MergeSampleUsageVisitor : public ProgramVisitor {
public:
    MergeSampleUsageVisitor(const Context& context, const Variable& fp)
            : fContext(context), fFP(fp) {}

    SampleUsage visit(const Program& program) {
        fUsage = SampleUsage(); // reset to none
        INHERITED::visit(program);
        return fUsage;
    }

protected:
    const Context& fContext;
    const Variable& fFP;
    SampleUsage fUsage;

    bool visitExpression(const Expression& e) override {
        // Looking for sample(fp, inColor?, ...)
        if (e.kind() == Expression::Kind::kFunctionCall) {
            const FunctionCall& fc = e.as<FunctionCall>();
            if (is_sample_call_to_fp(fc, fFP)) {
                // Determine the type of call at this site, and merge it with the accumulated state
                const Expression* lastArg = fc.arguments().back().get();

                if (lastArg->type() == *fContext.fTypes.fFloat2) {
                    fUsage.merge(SampleUsage::Explicit());
                } else if (lastArg->type() == *fContext.fTypes.fFloat3x3) {
                    // Determine the type of matrix for this call site
                    if (lastArg->isConstantOrUniform()) {
                        if (lastArg->kind() == Expression::Kind::kVariableReference ||
                            lastArg->kind() == Expression::Kind::kConstructor) {
                            // FIXME if this is a constant, we should parse the float3x3 constructor
                            // and determine if the resulting matrix introduces perspective.
                            fUsage.merge(SampleUsage::UniformMatrix(lastArg->description()));
                        } else {
                            // FIXME this is really to workaround a restriction of the downstream
                            // code that relies on the SampleUsage's fExpression to identify uniform
                            // names. Once they are tracked separately, any uniform expression can
                            // work, but right now this avoids issues from '0.5 * matrix' that is
                            // both a constant AND a uniform.
                            fUsage.merge(SampleUsage::VariableMatrix());
                        }
                    } else {
                        fUsage.merge(SampleUsage::VariableMatrix());
                    }
                } else {
                    // The only other signatures do pass-through sampling
                    fUsage.merge(SampleUsage::PassThrough());
                }
                // NOTE: we don't return true here just because we found a sample call. We need to
                //  process the entire program and merge across all encountered calls.
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

class ProgramUsageVisitor : public ProgramVisitor {
public:
    ProgramUsageVisitor(ProgramUsage* usage, int delta) : fUsage(usage), fDelta(delta) {}

    bool visitExpression(const Expression& e) override {
        if (e.is<FunctionCall>()) {
            const FunctionDeclaration* f = &e.as<FunctionCall>().function();
            fUsage->fCallCounts[f] += fDelta;
            SkASSERT(fUsage->fCallCounts[f] >= 0);
        } else if (e.is<VariableReference>()) {
            const VariableReference& ref = e.as<VariableReference>();
            ProgramUsage::VariableCounts& counts = fUsage->fVariableCounts[ref.variable()];
            switch (ref.refKind()) {
                case VariableRefKind::kRead:
                    counts.fRead += fDelta;
                    break;
                case VariableRefKind::kWrite:
                    counts.fWrite += fDelta;
                    break;
                case VariableRefKind::kReadWrite:
                case VariableRefKind::kPointer:
                    counts.fRead += fDelta;
                    counts.fWrite += fDelta;
                    break;
            }
            SkASSERT(counts.fRead >= 0 && counts.fWrite >= 0);
        }
        return INHERITED::visitExpression(e);
    }

    using ProgramVisitor::visitProgramElement;
    using ProgramVisitor::visitStatement;

    ProgramUsage* fUsage;
    int fDelta;
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
    void error(int offset, String) override { ++fErrorCount; }
    int errorCount() override { return fErrorCount; }

private:
    int fErrorCount = 0;
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
            info->fIsSwizzled = fIsSwizzled;
        }
        return fErrors->errorCount() == oldErrorCount;
    }

    void visitExpression(Expression& expr) {
        switch (expr.kind()) {
            case Expression::Kind::kVariableReference: {
                VariableReference& varRef = expr.as<VariableReference>();
                const Variable* var = varRef.variable();
                if (var->modifiers().fFlags & (Modifiers::kConst_Flag | Modifiers::kUniform_Flag |
                                               Modifiers::kVarying_Flag)) {
                    fErrors->error(expr.fOffset,
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
                fIsSwizzled = true;
                this->checkSwizzleWrite(swizzle);
                this->visitExpression(*swizzle.base());
                break;
            }
            case Expression::Kind::kIndex: {
                Expression& inner = *expr.as<IndexExpression>().base();
                fIsSwizzled |= inner.type().isVector();
                this->visitExpression(inner);
                break;
            }
            default:
                fErrors->error(expr.fOffset, "cannot assign to this expression");
                break;
        }
    }

private:
    void checkSwizzleWrite(const Swizzle& swizzle) {
        int bits = 0;
        for (int idx : swizzle.components()) {
            SkASSERT(idx <= 3);
            int bit = 1 << idx;
            if (bits & bit) {
                fErrors->error(swizzle.fOffset,
                               "cannot write to the same swizzle field more than once");
                break;
            }
            bits |= bit;
        }
    }

    ErrorReporter* fErrors;
    VariableReference* fAssignedVar = nullptr;
    bool fIsSwizzled = false;

    using INHERITED = ProgramVisitor;
};

}  // namespace

////////////////////////////////////////////////////////////////////////////////
// Analysis

SampleUsage Analysis::GetSampleUsage(const Program& program, const Variable& fp) {
    MergeSampleUsageVisitor visitor(*program.fContext, fp);
    return visitor.visit(program);
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

int Analysis::NodeCountUpToLimit(const FunctionDefinition& function, int limit) {
    return NodeCountVisitor{limit}.visit(*function.body());
}

std::unique_ptr<ProgramUsage> Analysis::GetUsage(const Program& program) {
    auto usage = std::make_unique<ProgramUsage>();
    ProgramUsageVisitor addRefs(usage.get(), /*delta=*/+1);
    addRefs.visit(program);
    return usage;
}

std::unique_ptr<ProgramUsage> Analysis::GetUsage(const LoadedModule& module) {
    auto usage = std::make_unique<ProgramUsage>();
    ProgramUsageVisitor addRefs(usage.get(), /*delta=*/+1);
    for (const auto& element : module.fElements) {
        addRefs.visitProgramElement(*element);
    }
    return usage;
}

ProgramUsage::VariableCounts ProgramUsage::get(const Variable& v) const {
    VariableCounts result = { 0, v.initialValue() ? 1 : 0 };
    if (const VariableCounts* counts = fVariableCounts.find(&v)) {
        result.fRead += counts->fRead;
        result.fWrite += counts->fWrite;
    }
    return result;
}

bool ProgramUsage::isDead(const Variable& v) const {
    const Modifiers& modifiers = v.modifiers();
    VariableCounts counts = this->get(v);
    if ((v.storage() != Variable::Storage::kLocal && counts.fRead) ||
        (modifiers.fFlags & (Modifiers::kIn_Flag | Modifiers::kOut_Flag | Modifiers::kUniform_Flag |
                             Modifiers::kVarying_Flag))) {
        return false;
    }
    return !counts.fWrite || (!counts.fRead && !(modifiers.fFlags &
                                                 (Modifiers::kPLS_Flag | Modifiers::kPLSOut_Flag)));
}

int ProgramUsage::get(const FunctionDeclaration& f) const {
    const int* count = fCallCounts.find(&f);
    return count ? *count : 0;
}

void ProgramUsage::replace(const Expression* oldExpr, const Expression* newExpr) {
    if (oldExpr) {
        ProgramUsageVisitor subRefs(this, /*delta=*/-1);
        subRefs.visitExpression(*oldExpr);
    }
    if (newExpr) {
        ProgramUsageVisitor addRefs(this, /*delta=*/+1);
        addRefs.visitExpression(*newExpr);
    }
}

void ProgramUsage::add(const Statement* stmt) {
    ProgramUsageVisitor addRefs(this, /*delta=*/+1);
    addRefs.visitStatement(*stmt);
}

void ProgramUsage::remove(const Expression* expr) {
    ProgramUsageVisitor subRefs(this, /*delta=*/-1);
    subRefs.visitExpression(*expr);
}

void ProgramUsage::remove(const Statement* stmt) {
    ProgramUsageVisitor subRefs(this, /*delta=*/-1);
    subRefs.visitStatement(*stmt);
}

void ProgramUsage::remove(const ProgramElement& element) {
    ProgramUsageVisitor subRefs(this, /*delta=*/-1);
    subRefs.visitProgramElement(element);
}

bool Analysis::StatementWritesToVariable(const Statement& stmt, const Variable& var) {
    return VariableWriteVisitor(&var).visit(stmt);
}

bool Analysis::IsAssignable(Expression& expr, AssignmentInfo* info, ErrorReporter* errors) {
    TrivialErrorReporter trivialErrors;
    return IsAssignableVisitor{errors ? errors : &trivialErrors}.visit(expr, info);
}

bool Analysis::IsTrivialExpression(const Expression& expr) {
    return expr.is<IntLiteral>() ||
           expr.is<FloatLiteral>() ||
           expr.is<BoolLiteral>() ||
           expr.is<VariableReference>() ||
           (expr.is<Swizzle>() &&
            IsTrivialExpression(*expr.as<Swizzle>().base())) ||
           (expr.is<FieldAccess>() &&
            IsTrivialExpression(*expr.as<FieldAccess>().base())) ||
           (expr.is<Constructor>() &&
            expr.as<Constructor>().arguments().size() == 1 &&
            IsTrivialExpression(*expr.as<Constructor>().arguments().front())) ||
           (expr.is<Constructor>() &&
            expr.isConstantOrUniform()) ||
           (expr.is<IndexExpression>() &&
            expr.as<IndexExpression>().index()->is<IntLiteral>() &&
            IsTrivialExpression(*expr.as<IndexExpression>().base()));
}

static const char* invalid_for_ES2(const ForStatement& loop,
                                   Analysis::UnrollableLoopInfo& loopInfo) {
    auto getConstant = [&](const std::unique_ptr<Expression>& expr, double* val) {
        if (!expr->isCompileTimeConstant()) {
            return false;
        }
        if (!expr->type().isNumber()) {
            SkDEBUGFAIL("unexpected constant type");
            return false;
        }

        *val = expr->type().isInteger() ? static_cast<double>(expr->getConstantInt())
                                        : static_cast<double>(expr->getConstantFloat());
        return true;
    };

    //
    // init_declaration has the form: type_specifier identifier = constant_expression
    //
    if (!loop.initializer()) {
        return "missing init declaration";
    }
    if (!loop.initializer()->is<VarDeclaration>()) {
        return "invalid init declaration";
    }
    const VarDeclaration& initDecl = loop.initializer()->as<VarDeclaration>();
    if (!initDecl.baseType().isNumber()) {
        return "invalid type for loop index";
    }
    if (initDecl.arraySize() != 0) {
        return "invalid type for loop index";
    }
    if (!initDecl.value()) {
        return "missing loop index initializer";
    }
    if (!getConstant(initDecl.value(), &loopInfo.fStart)) {
        return "loop index initializer must be a constant expression";
    }

    loopInfo.fIndex = &initDecl.var();

    auto is_loop_index = [&](const std::unique_ptr<Expression>& expr) {
        return expr->is<VariableReference>() &&
               expr->as<VariableReference>().variable() == loopInfo.fIndex;
    };

    //
    // condition has the form: loop_index relational_operator constant_expression
    //
    if (!loop.test()) {
        return "missing condition";
    }
    if (!loop.test()->is<BinaryExpression>()) {
        return "invalid condition";
    }
    const BinaryExpression& cond = loop.test()->as<BinaryExpression>();
    if (!is_loop_index(cond.left())) {
        return "expected loop index on left hand side of condition";
    }
    // relational_operator is one of: > >= < <= == or !=
    switch (cond.getOperator()) {
        case Token::Kind::TK_GT:
        case Token::Kind::TK_GTEQ:
        case Token::Kind::TK_LT:
        case Token::Kind::TK_LTEQ:
        case Token::Kind::TK_EQEQ:
        case Token::Kind::TK_NEQ:
            break;
        default:
            return "invalid relational operator";
    }
    double loopEnd = 0;
    if (!getConstant(cond.right(), &loopEnd)) {
        return "loop index must be compared with a constant expression";
    }

    //
    // expression has one of the following forms:
    //   loop_index++
    //   loop_index--
    //   loop_index += constant_expression
    //   loop_index -= constant_expression
    // The spec doesn't mention prefix increment and decrement, but there is some consensus that
    // it's an oversight, so we allow those as well.
    //
    if (!loop.next()) {
        return "missing loop expression";
    }
    switch (loop.next()->kind()) {
        case Expression::Kind::kBinary: {
            const BinaryExpression& next = loop.next()->as<BinaryExpression>();
            if (!is_loop_index(next.left())) {
                return "expected loop index in loop expression";
            }
            if (!getConstant(next.right(), &loopInfo.fDelta)) {
                return "loop index must be modified by a constant expression";
            }
            switch (next.getOperator()) {
                case Token::Kind::TK_PLUSEQ:                                      break;
                case Token::Kind::TK_MINUSEQ: loopInfo.fDelta = -loopInfo.fDelta; break;
                default:
                    return "invalid operator in loop expression";
            }
        } break;
        case Expression::Kind::kPrefix: {
            const PrefixExpression& next = loop.next()->as<PrefixExpression>();
            if (!is_loop_index(next.operand())) {
                return "expected loop index in loop expression";
            }
            switch (next.getOperator()) {
                case Token::Kind::TK_PLUSPLUS:   loopInfo.fDelta =  1; break;
                case Token::Kind::TK_MINUSMINUS: loopInfo.fDelta = -1; break;
                default:
                    return "invalid operator in loop expression";
            }
        } break;
        case Expression::Kind::kPostfix: {
            const PostfixExpression& next = loop.next()->as<PostfixExpression>();
            if (!is_loop_index(next.operand())) {
                return "expected loop index in loop expression";
            }
            switch (next.getOperator()) {
                case Token::Kind::TK_PLUSPLUS:   loopInfo.fDelta =  1; break;
                case Token::Kind::TK_MINUSMINUS: loopInfo.fDelta = -1; break;
                default:
                    return "invalid operator in loop expression";
            }
        } break;
        default:
            return "invalid loop expression";
    }

    //
    // Within the body of the loop, the loop index is not statically assigned to, nor is it used as
    // argument to a function 'out' or 'inout' parameter.
    //
    if (Analysis::StatementWritesToVariable(*loop.statement(), initDecl.var())) {
        return "loop index must not be modified within body of the loop";
    }

    // Finally, compute the iteration count, based on the bounds, and the termination operator.
    constexpr int kMaxUnrollableLoopLength = 128;
    loopInfo.fCount = 0;

    double val = loopInfo.fStart;
    auto evalCond = [&]() {
        switch (cond.getOperator()) {
            case Token::Kind::TK_GT:   return val >  loopEnd;
            case Token::Kind::TK_GTEQ: return val >= loopEnd;
            case Token::Kind::TK_LT:   return val <  loopEnd;
            case Token::Kind::TK_LTEQ: return val <= loopEnd;
            case Token::Kind::TK_EQEQ: return val == loopEnd;
            case Token::Kind::TK_NEQ:  return val != loopEnd;
            default: SkUNREACHABLE;
        }
    };

    for (loopInfo.fCount = 0; loopInfo.fCount <= kMaxUnrollableLoopLength; ++loopInfo.fCount) {
        if (!evalCond()) {
            break;
        }
        val += loopInfo.fDelta;
    }

    if (loopInfo.fCount > kMaxUnrollableLoopLength) {
        return "loop must guarantee termination in fewer iterations";
    }

    return nullptr;  // All checks pass
}

bool Analysis::ForLoopIsValidForES2(const ForStatement& loop,
                                    Analysis::UnrollableLoopInfo* outLoopInfo,
                                    ErrorReporter* errors) {
    UnrollableLoopInfo ignored,
                       *loopInfo = outLoopInfo ? outLoopInfo : &ignored;
    if (const char* msg = invalid_for_ES2(loop, *loopInfo)) {
        if (errors) {
            errors->error(loop.fOffset, msg);
        }
        return false;
    }
    return true;
}

class ES2IndexExpressionVisitor : public ProgramVisitor {
public:
    ES2IndexExpressionVisitor(const std::set<const Variable*>* loopIndices)
            : fLoopIndices(loopIndices) {}

    bool visitExpression(const Expression& e) override {
        // A constant-(index)-expression is one of...
        switch (e.kind()) {
            // ... a literal value
            case Expression::Kind::kBoolLiteral:
            case Expression::Kind::kIntLiteral:
            case Expression::Kind::kFloatLiteral:
                return false;

            // ... loop indices as defined in section 4. Today, SkSL allows no other variables,
            // but GLSL allows 'const' variables, because it requires that const variables be
            // initialized with constant-expressions. We could support that usage by checking
            // for variables that are const, and whose initializing expression also pass our
            // checks. For now, we'll be conservative. (skbug.com/10837)
            case Expression::Kind::kVariableReference:
                return fLoopIndices->find(e.as<VariableReference>().variable()) ==
                       fLoopIndices->end();

            // ... expressions composed of both of the above
            case Expression::Kind::kBinary:
            case Expression::Kind::kConstructor:
            case Expression::Kind::kFieldAccess:
            case Expression::Kind::kIndex:
            case Expression::Kind::kPrefix:
            case Expression::Kind::kPostfix:
            case Expression::Kind::kSwizzle:
            case Expression::Kind::kTernary:
                return INHERITED::visitExpression(e);

            // These are completely disallowed in SkSL constant-index-expressions. GLSL allows
            // calls to built-in functions where the arguments are all constant-expressions, but
            // we don't guarantee that behavior. (skbug.com/10835)
            case Expression::Kind::kExternalFunctionCall:
            case Expression::Kind::kFunctionCall:
                return true;

            // These should never appear in final IR
            case Expression::Kind::kDefined:
            case Expression::Kind::kExternalFunctionReference:
            case Expression::Kind::kFunctionReference:
            case Expression::Kind::kSetting:
            case Expression::Kind::kTypeReference:
            default:
                SkDEBUGFAIL("Unexpected expression type");
                return true;
        }
    }

private:
    const std::set<const Variable*>* fLoopIndices;
    using INHERITED = ProgramVisitor;
};

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
            ES2IndexExpressionVisitor indexerInvalid(&fLoopIndices);
            if (indexerInvalid.visitExpression(*i.index())) {
                fErrors.error(i.fOffset, "index expression must be constant");
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

template <typename PROG, typename EXPR, typename STMT, typename ELEM>
bool TProgramVisitor<PROG, EXPR, STMT, ELEM>::visitExpression(EXPR e) {
    switch (e.kind()) {
        case Expression::Kind::kBoolLiteral:
        case Expression::Kind::kDefined:
        case Expression::Kind::kExternalFunctionReference:
        case Expression::Kind::kFloatLiteral:
        case Expression::Kind::kFunctionReference:
        case Expression::Kind::kIntLiteral:
        case Expression::Kind::kSetting:
        case Expression::Kind::kTypeReference:
        case Expression::Kind::kVariableReference:
            // Leaf expressions return false
            return false;

        case Expression::Kind::kBinary: {
            auto& b = e.template as<BinaryExpression>();
            return (b.left() && this->visitExpression(*b.left())) ||
                   (b.right() && this->visitExpression(*b.right()));
        }
        case Expression::Kind::kConstructor: {
            auto& c = e.template as<Constructor>();
            for (auto& arg : c.arguments()) {
                if (this->visitExpression(*arg)) { return true; }
            }
            return false;
        }
        case Expression::Kind::kExternalFunctionCall: {
            auto& c = e.template as<ExternalFunctionCall>();
            for (auto& arg : c.arguments()) {
                if (this->visitExpression(*arg)) { return true; }
            }
            return false;
        }
        case Expression::Kind::kFieldAccess:
            return this->visitExpression(*e.template as<FieldAccess>().base());

        case Expression::Kind::kFunctionCall: {
            auto& c = e.template as<FunctionCall>();
            for (auto& arg : c.arguments()) {
                if (arg && this->visitExpression(*arg)) { return true; }
            }
            return false;
        }
        case Expression::Kind::kIndex: {
            auto& i = e.template as<IndexExpression>();
            return this->visitExpression(*i.base()) || this->visitExpression(*i.index());
        }
        case Expression::Kind::kPostfix:
            return this->visitExpression(*e.template as<PostfixExpression>().operand());

        case Expression::Kind::kPrefix:
            return this->visitExpression(*e.template as<PrefixExpression>().operand());

        case Expression::Kind::kSwizzle: {
            auto& s = e.template as<Swizzle>();
            return s.base() && this->visitExpression(*s.base());
        }

        case Expression::Kind::kTernary: {
            auto& t = e.template as<TernaryExpression>();
            return this->visitExpression(*t.test()) ||
                   (t.ifTrue() && this->visitExpression(*t.ifTrue())) ||
                   (t.ifFalse() && this->visitExpression(*t.ifFalse()));
        }
        default:
            SkUNREACHABLE;
    }
}

template <typename PROG, typename EXPR, typename STMT, typename ELEM>
bool TProgramVisitor<PROG, EXPR, STMT, ELEM>::visitStatement(STMT s) {
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
                if (stmt && this->visitStatement(*stmt)) {
                    return true;
                }
            }
            return false;

        case Statement::Kind::kDo: {
            auto& d = s.template as<DoStatement>();
            return this->visitExpression(*d.test()) || this->visitStatement(*d.statement());
        }
        case Statement::Kind::kExpression:
            return this->visitExpression(*s.template as<ExpressionStatement>().expression());

        case Statement::Kind::kFor: {
            auto& f = s.template as<ForStatement>();
            return (f.initializer() && this->visitStatement(*f.initializer())) ||
                   (f.test() && this->visitExpression(*f.test())) ||
                   (f.next() && this->visitExpression(*f.next())) ||
                   this->visitStatement(*f.statement());
        }
        case Statement::Kind::kIf: {
            auto& i = s.template as<IfStatement>();
            return (i.test() && this->visitExpression(*i.test())) ||
                   (i.ifTrue() && this->visitStatement(*i.ifTrue())) ||
                   (i.ifFalse() && this->visitStatement(*i.ifFalse()));
        }
        case Statement::Kind::kReturn: {
            auto& r = s.template as<ReturnStatement>();
            return r.expression() && this->visitExpression(*r.expression());
        }
        case Statement::Kind::kSwitch: {
            auto& sw = s.template as<SwitchStatement>();
            if (this->visitExpression(*sw.value())) {
                return true;
            }
            for (const auto& c : sw.cases()) {
                if (c->value() && this->visitExpression(*c->value())) {
                    return true;
                }
                for (auto& st : c->statements()) {
                    if (st && this->visitStatement(*st)) {
                        return true;
                    }
                }
            }
            return false;
        }
        case Statement::Kind::kVarDeclaration: {
            auto& v = s.template as<VarDeclaration>();
            return v.value() && this->visitExpression(*v.value());
        }
        default:
            SkUNREACHABLE;
    }
}

template <typename PROG, typename EXPR, typename STMT, typename ELEM>
bool TProgramVisitor<PROG, EXPR, STMT, ELEM>::visitProgramElement(ELEM pe) {
    switch (pe.kind()) {
        case ProgramElement::Kind::kEnum:
        case ProgramElement::Kind::kExtension:
        case ProgramElement::Kind::kFunctionPrototype:
        case ProgramElement::Kind::kInterfaceBlock:
        case ProgramElement::Kind::kModifiers:
        case ProgramElement::Kind::kSection:
        case ProgramElement::Kind::kStructDefinition:
            // Leaf program elements just return false by default
            return false;

        case ProgramElement::Kind::kFunction:
            return this->visitStatement(*pe.template as<FunctionDefinition>().body());

        case ProgramElement::Kind::kGlobalVar:
            if (this->visitStatement(*pe.template as<GlobalVarDeclaration>().declaration())) {
                return true;
            }
            return false;

        default:
            SkUNREACHABLE;
    }
}

template class TProgramVisitor<const Program&, const Expression&,
                               const Statement&, const ProgramElement&>;
template class TProgramVisitor<Program&, Expression&, Statement&, ProgramElement&>;

}  // namespace SkSL
