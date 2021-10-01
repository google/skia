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
#include "src/core/SkSafeMath.h"
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

#include <stack>

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

class ProgramUsageVisitor : public ProgramVisitor {
public:
    ProgramUsageVisitor(ProgramUsage* usage, int delta) : fUsage(usage), fDelta(delta) {}

    bool visitProgramElement(const ProgramElement& pe) override {
        if (pe.is<FunctionDefinition>()) {
            for (const Variable* param : pe.as<FunctionDefinition>().declaration().parameters()) {
                // Ensure function-parameter variables exist in the variable usage map. They aren't
                // otherwise declared, but ProgramUsage::get() should be able to find them, even if
                // they are unread and unwritten.
                fUsage->fVariableCounts[param];
            }
        } else if (pe.is<InterfaceBlock>()) {
            // Ensure interface-block variables exist in the variable usage map.
            fUsage->fVariableCounts[&pe.as<InterfaceBlock>().variable()];
        }
        return INHERITED::visitProgramElement(pe);
    }

    bool visitStatement(const Statement& s) override {
        if (s.is<VarDeclaration>()) {
            // Add all declared variables to the usage map (even if never otherwise accessed).
            const VarDeclaration& vd = s.as<VarDeclaration>();
            ProgramUsage::VariableCounts& counts = fUsage->fVariableCounts[&vd.var()];
            counts.fDeclared += fDelta;
            SkASSERT(counts.fDeclared >= 0);
            if (vd.value()) {
                // The initial-value expression, when present, counts as a write.
                counts.fWrite += fDelta;
            }
        }
        return INHERITED::visitStatement(s);
    }

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

class SwitchCaseContainsExit : public ProgramVisitor {
public:
    SwitchCaseContainsExit(bool conditionalExits) : fConditionalExits(conditionalExits) {}

    bool visitStatement(const Statement& stmt) override {
        switch (stmt.kind()) {
            case Statement::Kind::kBlock:
            case Statement::Kind::kSwitchCase:
                return INHERITED::visitStatement(stmt);

            case Statement::Kind::kReturn:
                // Returns are an early exit regardless of the surrounding control structures.
                return fConditionalExits ? fInConditional : !fInConditional;

            case Statement::Kind::kContinue:
                // Continues are an early exit from switches, but not loops.
                return !fInLoop &&
                       (fConditionalExits ? fInConditional : !fInConditional);

            case Statement::Kind::kBreak:
                // Breaks cannot escape from switches or loops.
                return !fInLoop && !fInSwitch &&
                       (fConditionalExits ? fInConditional : !fInConditional);

            case Statement::Kind::kIf: {
                ++fInConditional;
                bool result = INHERITED::visitStatement(stmt);
                --fInConditional;
                return result;
            }

            case Statement::Kind::kFor:
            case Statement::Kind::kDo: {
                // Loops are treated as conditionals because a loop could potentially execute zero
                // times. We don't have a straightforward way to determine that a loop definitely
                // executes at least once.
                ++fInConditional;
                ++fInLoop;
                bool result = INHERITED::visitStatement(stmt);
                --fInLoop;
                --fInConditional;
                return result;
            }

            case Statement::Kind::kSwitch: {
                ++fInSwitch;
                bool result = INHERITED::visitStatement(stmt);
                --fInSwitch;
                return result;
            }

            default:
                return false;
        }
    }

    bool fConditionalExits = false;
    int fInConditional = 0;
    int fInLoop = 0;
    int fInSwitch = 0;
    using INHERITED = ProgramVisitor;
};

class ReturnsOnAllPathsVisitor : public ProgramVisitor {
public:
    bool visitExpression(const Expression& expr) override {
        // We can avoid processing expressions entirely.
        return false;
    }

    bool visitStatement(const Statement& stmt) override {
        switch (stmt.kind()) {
            // Returns, breaks, or continues will stop the scan, so only one of these should ever be
            // true.
            case Statement::Kind::kReturn:
                fFoundReturn = true;
                return true;

            case Statement::Kind::kBreak:
                fFoundBreak = true;
                return true;

            case Statement::Kind::kContinue:
                fFoundContinue = true;
                return true;

            case Statement::Kind::kIf: {
                const IfStatement& i = stmt.as<IfStatement>();
                ReturnsOnAllPathsVisitor trueVisitor;
                ReturnsOnAllPathsVisitor falseVisitor;
                trueVisitor.visitStatement(*i.ifTrue());
                if (i.ifFalse()) {
                    falseVisitor.visitStatement(*i.ifFalse());
                }
                // If either branch leads to a break or continue, we report the entire if as
                // containing a break or continue, since we don't know which side will be reached.
                fFoundBreak    = (trueVisitor.fFoundBreak    || falseVisitor.fFoundBreak);
                fFoundContinue = (trueVisitor.fFoundContinue || falseVisitor.fFoundContinue);
                // On the other hand, we only want to report returns that definitely happen, so we
                // require those to be found on both sides.
                fFoundReturn   = (trueVisitor.fFoundReturn   && falseVisitor.fFoundReturn);
                return fFoundBreak || fFoundContinue || fFoundReturn;
            }
            case Statement::Kind::kFor: {
                const ForStatement& f = stmt.as<ForStatement>();
                // We assume a for/while loop runs for at least one iteration; this isn't strictly
                // guaranteed, but it's better to be slightly over-permissive here than to fail on
                // reasonable code.
                ReturnsOnAllPathsVisitor forVisitor;
                forVisitor.visitStatement(*f.statement());
                // A for loop that contains a break or continue is safe; it won't exit the entire
                // function, just the loop. So we disregard those signals.
                fFoundReturn = forVisitor.fFoundReturn;
                return fFoundReturn;
            }
            case Statement::Kind::kDo: {
                const DoStatement& d = stmt.as<DoStatement>();
                // Do-while blocks are always entered at least once.
                ReturnsOnAllPathsVisitor doVisitor;
                doVisitor.visitStatement(*d.statement());
                // A do-while loop that contains a break or continue is safe; it won't exit the
                // entire function, just the loop. So we disregard those signals.
                fFoundReturn = doVisitor.fFoundReturn;
                return fFoundReturn;
            }
            case Statement::Kind::kBlock:
                // Blocks are definitely entered and don't imply any additional control flow.
                // If the block contains a break, continue or return, we want to keep that.
                return INHERITED::visitStatement(stmt);

            case Statement::Kind::kSwitch: {
                // Switches are the most complex control flow we need to deal with; fortunately we
                // already have good primitives for dissecting them. We need to verify that:
                // - a default case exists, so that every possible input value is covered
                // - every switch-case either (a) returns unconditionally, or
                //                            (b) falls through to another case that does
                const SwitchStatement& s = stmt.as<SwitchStatement>();
                bool foundDefault = false;
                bool fellThrough = false;
                for (const std::unique_ptr<Statement>& switchStmt : s.cases()) {
                    // The default case is indicated by a null value. A switch without a default
                    // case cannot definitively return, as its value might not be in the cases list.
                    const SwitchCase& sc = switchStmt->as<SwitchCase>();
                    if (!sc.value()) {
                        foundDefault = true;
                    }
                    // Scan this switch-case for any exit (break, continue or return).
                    ReturnsOnAllPathsVisitor caseVisitor;
                    caseVisitor.visitStatement(sc);

                    // If we found a break or continue, whether conditional or not, this switch case
                    // can't be called an unconditional return. Switches absorb breaks but not
                    // continues.
                    if (caseVisitor.fFoundContinue) {
                        fFoundContinue = true;
                        return false;
                    }
                    if (caseVisitor.fFoundBreak) {
                        return false;
                    }
                    // We just confirmed that there weren't any breaks or continues. If we didn't
                    // find an unconditional return either, the switch is considered fallen-through.
                    // (There might be a conditional return, but that doesn't count.)
                    fellThrough = !caseVisitor.fFoundReturn;
                }

                // If we didn't find a default case, or the very last case fell through, this switch
                // doesn't meet our criteria.
                if (fellThrough || !foundDefault) {
                    return false;
                }

                // We scanned the entire switch, found a default case, and every section either fell
                // through or contained an unconditional return.
                fFoundReturn = true;
                return true;
            }

            case Statement::Kind::kSwitchCase:
                // Recurse into the switch-case.
                return INHERITED::visitStatement(stmt);

            case Statement::Kind::kDiscard:
            case Statement::Kind::kExpression:
            case Statement::Kind::kInlineMarker:
            case Statement::Kind::kNop:
            case Statement::Kind::kVarDeclaration:
                // None of these statements could contain a return.
                break;
        }

        return false;
    }

    bool fFoundReturn = false;
    bool fFoundBreak = false;
    bool fFoundContinue = false;

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

bool Analysis::CheckProgramUnrolledSize(const Program& program) {
    // We check the size of strict-ES2 programs since SkVM will completely unroll them.
    // Note that we *cannot* safely check the program size of non-ES2 code at this time, as it is
    // allowed to do things we can't measure (e.g. the program can contain a recursive cycle). We
    // could, at best, compute a lower bound.
    const Context& context = *program.fContext;
    SkASSERT(context.fConfig->strictES2Mode());

    // If we decide that expressions are cheaper than statements, or that certain statements are
    // more expensive than others, etc., we can always tweak these ratios as needed. A very rough
    // ballpark estimate is currently good enough for our purposes.
    static constexpr size_t kExpressionCost = 1;
    static constexpr size_t kStatementCost = 1;
    static constexpr size_t kUnknownCost = -1;
    static constexpr size_t kProgramSizeLimit = 100000;
    static constexpr size_t kProgramStackDepthLimit = 50;

    class ProgramSizeVisitor : public ProgramVisitor {
    public:
        ProgramSizeVisitor(const Context& c) : fContext(c) {}

        using ProgramVisitor::visitProgramElement;

        size_t functionSize() const {
            return fFunctionSize;
        }

        bool visitProgramElement(const ProgramElement& pe) override {
            if (pe.is<FunctionDefinition>()) {
                // Check the function-size cache map first. We don't need to visit this function if
                // we already processed it before.
                const FunctionDeclaration* decl = &pe.as<FunctionDefinition>().declaration();
                auto [iter, wasInserted] = fFunctionCostMap.insert({decl, kUnknownCost});
                if (!wasInserted) {
                    // We already have this function in our map. We don't need to check it again.
                    if (iter->second == kUnknownCost) {
                        // If the function is present in the map with an unknown cost, we're
                        // recursively processing it--in other words, we found a cycle in the code.
                        // Unwind our stack into a string.
                        String msg = "\n\t" + decl->description();
                        for (auto unwind = fStack.rbegin(); unwind != fStack.rend(); ++unwind) {
                            msg = "\n\t" + (*unwind)->description() + msg;
                            if (*unwind == decl) {
                                break;
                            }
                        }
                        msg = "potential recursion (function call cycle) not allowed:" + msg;
                        fContext.fErrors->error(pe.fLine, std::move(msg));
                        fFunctionSize = iter->second = 0;
                        return true;
                    }
                    // Set the size to its known value.
                    fFunctionSize = iter->second;
                    return false;
                }

                // If the function-call stack has gotten too deep, stop the analysis.
                if (fStack.size() >= kProgramStackDepthLimit) {
                    String msg = "exceeded max function call depth:";
                    for (auto unwind = fStack.begin(); unwind != fStack.end(); ++unwind) {
                        msg += "\n\t" + (*unwind)->description();
                    }
                    msg += "\n\t" + decl->description();
                    fContext.fErrors->error(pe.fLine, std::move(msg));
                    fFunctionSize = iter->second = 0;
                    return true;
                }

                // Calculate the function cost and store it in our cache.
                fStack.push_back(decl);
                fFunctionSize = 0;
                bool result = INHERITED::visitProgramElement(pe);
                iter->second = fFunctionSize;
                fStack.pop_back();

                return result;
            }

            return INHERITED::visitProgramElement(pe);
        }

        bool visitStatement(const Statement& stmt) override {
            switch (stmt.kind()) {
                case Statement::Kind::kFor: {
                    // We count a for-loop's unrolled size here. We expect that the init statement
                    // will be emitted once, and the next-expr and statement will be repeated in the
                    // output for every iteration of the loop. The test-expr is optimized away
                    // during the unroll and is not counted at all.
                    const ForStatement& forStmt = stmt.as<ForStatement>();
                    bool result = this->visitStatement(*forStmt.initializer());

                    size_t originalFunctionSize = fFunctionSize;
                    fFunctionSize = 0;

                    result = this->visitExpression(*forStmt.next()) ||
                             this->visitStatement(*forStmt.statement()) || result;

                    if (const LoopUnrollInfo* unrollInfo = forStmt.unrollInfo()) {
                        fFunctionSize = SkSafeMath::Mul(fFunctionSize, unrollInfo->fCount);
                    } else {
                        SkDEBUGFAIL("for-loops should always have unroll info in an ES2 program");
                    }

                    fFunctionSize = SkSafeMath::Add(fFunctionSize, originalFunctionSize);
                    return result;
                }

                case Statement::Kind::kExpression:
                    // The cost of an expression-statement is counted in visitExpression. It would
                    // be double-dipping to count it here too.
                    break;

                case Statement::Kind::kInlineMarker:
                case Statement::Kind::kNop:
                case Statement::Kind::kVarDeclaration:
                    // These statements don't directly consume any space in a compiled program.
                    break;

                case Statement::Kind::kDo:
                    SkDEBUGFAIL("encountered a statement that shouldn't exist in an ES2 program");
                    break;

                default:
                    fFunctionSize = SkSafeMath::Add(fFunctionSize, kStatementCost);
                    break;
            }

            bool earlyExit = fFunctionSize > kProgramSizeLimit;
            return earlyExit || INHERITED::visitStatement(stmt);
        }

        bool visitExpression(const Expression& expr) override {
            // Other than function calls, all expressions are assumed to have a fixed unit cost.
            bool earlyExit = false;
            size_t expressionCost = kExpressionCost;

            if (expr.is<FunctionCall>()) {
                // Visit this function call to calculate its size. If we've already sized it, this
                // will retrieve the size from our cache.
                const FunctionCall& call = expr.as<FunctionCall>();
                const FunctionDeclaration* decl = &call.function();
                if (decl->definition() && !decl->isIntrinsic()) {
                    size_t originalFunctionSize = fFunctionSize;
                    fFunctionSize = 0;

                    earlyExit = this->visitProgramElement(*decl->definition());
                    expressionCost = fFunctionSize;

                    fFunctionSize = originalFunctionSize;
                }
            }

            fFunctionSize = SkSafeMath::Add(fFunctionSize, expressionCost);
            return earlyExit || INHERITED::visitExpression(expr);
        }

    private:
        using INHERITED = ProgramVisitor;

        const Context& fContext;
        size_t fFunctionSize = 0;
        std::unordered_map<const FunctionDeclaration*, size_t> fFunctionCostMap;
        std::vector<const FunctionDeclaration*> fStack;
    };

    // Process every function in our program.
    ProgramSizeVisitor visitor{context};
    for (const std::unique_ptr<ProgramElement>& element : program.ownedElements()) {
        if (element->is<FunctionDefinition>()) {
            // Visit every function--we want to detect static recursion and report it as an error,
            // even in unreferenced functions.
            visitor.visitProgramElement(*element);
            // Report an error when main()'s flattened size is larger than our program limit.
            if (visitor.functionSize() > kProgramSizeLimit &&
                element->as<FunctionDefinition>().declaration().isMain()) {
                context.fErrors->error(/*line=*/-1, "program is too large");
            }
        }
    }

    return true;
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

bool Analysis::SwitchCaseContainsUnconditionalExit(Statement& stmt) {
    return SwitchCaseContainsExit{/*conditionalExits=*/false}.visitStatement(stmt);
}

bool Analysis::SwitchCaseContainsConditionalExit(Statement& stmt) {
    return SwitchCaseContainsExit{/*conditionalExits=*/true}.visitStatement(stmt);
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
    const VariableCounts* counts = fVariableCounts.find(&v);
    SkASSERT(counts);
    return *counts;
}

bool ProgramUsage::isDead(const Variable& v) const {
    const Modifiers& modifiers = v.modifiers();
    VariableCounts counts = this->get(v);
    if ((v.storage() != Variable::Storage::kLocal && counts.fRead) ||
        (modifiers.fFlags &
         (Modifiers::kIn_Flag | Modifiers::kOut_Flag | Modifiers::kUniform_Flag))) {
        return false;
    }
    // Consider the variable dead if it's never read and never written (besides the initial-value).
    return !counts.fRead && (counts.fWrite <= (v.initialValue() ? 1 : 0));
}

int ProgramUsage::get(const FunctionDeclaration& f) const {
    const int* count = fCallCounts.find(&f);
    return count ? *count : 0;
}

void ProgramUsage::add(const Expression* expr) {
    ProgramUsageVisitor addRefs(this, /*delta=*/+1);
    addRefs.visitExpression(*expr);
}

void ProgramUsage::add(const Statement* stmt) {
    ProgramUsageVisitor addRefs(this, /*delta=*/+1);
    addRefs.visitStatement(*stmt);
}

void ProgramUsage::add(const ProgramElement& element) {
    ProgramUsageVisitor addRefs(this, /*delta=*/+1);
    addRefs.visitProgramElement(element);
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

static const char* invalid_for_ES2(int line,
                                   const Statement* loopInitializer,
                                   const Expression* loopTest,
                                   const Expression* loopNext,
                                   const Statement* loopStatement,
                                   LoopUnrollInfo& loopInfo) {
    //
    // init_declaration has the form: type_specifier identifier = constant_expression
    //
    if (!loopInitializer) {
        return "missing init declaration";
    }
    if (!loopInitializer->is<VarDeclaration>()) {
        return "invalid init declaration";
    }
    const VarDeclaration& initDecl = loopInitializer->as<VarDeclaration>();
    if (!initDecl.baseType().isNumber()) {
        return "invalid type for loop index";
    }
    if (initDecl.arraySize() != 0) {
        return "invalid type for loop index";
    }
    if (!initDecl.value()) {
        return "missing loop index initializer";
    }
    if (!ConstantFolder::GetConstantValue(*initDecl.value(), &loopInfo.fStart)) {
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
    if (!loopTest) {
        return "missing condition";
    }
    if (!loopTest->is<BinaryExpression>()) {
        return "invalid condition";
    }
    const BinaryExpression& cond = loopTest->as<BinaryExpression>();
    if (!is_loop_index(cond.left())) {
        return "expected loop index on left hand side of condition";
    }
    // relational_operator is one of: > >= < <= == or !=
    switch (cond.getOperator().kind()) {
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
    if (!ConstantFolder::GetConstantValue(*cond.right(), &loopEnd)) {
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
    if (!loopNext) {
        return "missing loop expression";
    }
    switch (loopNext->kind()) {
        case Expression::Kind::kBinary: {
            const BinaryExpression& next = loopNext->as<BinaryExpression>();
            if (!is_loop_index(next.left())) {
                return "expected loop index in loop expression";
            }
            if (!ConstantFolder::GetConstantValue(*next.right(), &loopInfo.fDelta)) {
                return "loop index must be modified by a constant expression";
            }
            switch (next.getOperator().kind()) {
                case Token::Kind::TK_PLUSEQ:                                      break;
                case Token::Kind::TK_MINUSEQ: loopInfo.fDelta = -loopInfo.fDelta; break;
                default:
                    return "invalid operator in loop expression";
            }
        } break;
        case Expression::Kind::kPrefix: {
            const PrefixExpression& next = loopNext->as<PrefixExpression>();
            if (!is_loop_index(next.operand())) {
                return "expected loop index in loop expression";
            }
            switch (next.getOperator().kind()) {
                case Token::Kind::TK_PLUSPLUS:   loopInfo.fDelta =  1; break;
                case Token::Kind::TK_MINUSMINUS: loopInfo.fDelta = -1; break;
                default:
                    return "invalid operator in loop expression";
            }
        } break;
        case Expression::Kind::kPostfix: {
            const PostfixExpression& next = loopNext->as<PostfixExpression>();
            if (!is_loop_index(next.operand())) {
                return "expected loop index in loop expression";
            }
            switch (next.getOperator().kind()) {
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
    if (Analysis::StatementWritesToVariable(*loopStatement, initDecl.var())) {
        return "loop index must not be modified within body of the loop";
    }

    // Finally, compute the iteration count, based on the bounds, and the termination operator.
    static constexpr int kLoopTerminationLimit = 100000;
    loopInfo.fCount = 0;

    auto calculateCount = [](double start, double end, double delta,
                             bool forwards, bool inclusive) -> int {
        if (forwards != (start < end)) {
            // The loop starts in a completed state (the start has already advanced past the end).
            return 0;
        }
        if ((delta == 0.0) || forwards != (delta > 0.0)) {
            // The loop does not progress toward a completed state, and will never terminate.
            return kLoopTerminationLimit;
        }
        double iterations = sk_ieee_double_divide(end - start, delta);
        double count = std::ceil(iterations);
        if (inclusive && (count == iterations)) {
            count += 1.0;
        }
        if (count > kLoopTerminationLimit || !std::isfinite(count)) {
            // The loop runs for more iterations than we can safely unroll.
            return kLoopTerminationLimit;
        }
        return (int)count;
    };

    switch (cond.getOperator().kind()) {
        case Token::Kind::TK_LT:
            loopInfo.fCount = calculateCount(loopInfo.fStart, loopEnd, loopInfo.fDelta,
                                             /*forwards=*/true, /*inclusive=*/false);
            break;

        case Token::Kind::TK_GT:
            loopInfo.fCount = calculateCount(loopInfo.fStart, loopEnd, loopInfo.fDelta,
                                             /*forwards=*/false, /*inclusive=*/false);
            break;

        case Token::Kind::TK_LTEQ:
            loopInfo.fCount = calculateCount(loopInfo.fStart, loopEnd, loopInfo.fDelta,
                                             /*forwards=*/true, /*inclusive=*/true);
            break;

        case Token::Kind::TK_GTEQ:
            loopInfo.fCount = calculateCount(loopInfo.fStart, loopEnd, loopInfo.fDelta,
                                             /*forwards=*/false, /*inclusive=*/true);
            break;

        case Token::Kind::TK_NEQ: {
            float iterations = sk_ieee_double_divide(loopEnd - loopInfo.fStart, loopInfo.fDelta);
            loopInfo.fCount = std::ceil(iterations);
            if (loopInfo.fCount < 0 || loopInfo.fCount != iterations ||
                !std::isfinite(iterations)) {
                // The loop doesn't reach the exact endpoint and so will never terminate.
                loopInfo.fCount = kLoopTerminationLimit;
            }
            break;
        }
        case Token::Kind::TK_EQEQ: {
            if (loopInfo.fStart == loopEnd) {
                // Start and end begin in the same place, so we can run one iteration...
                if (loopInfo.fDelta) {
                    // ... and then they diverge, so the loop terminates.
                    loopInfo.fCount = 1;
                } else {
                    // ... but they never diverge, so the loop runs forever.
                    loopInfo.fCount = kLoopTerminationLimit;
                }
            } else {
                // Start never equals end, so the loop will not run a single iteration.
                loopInfo.fCount = 0;
            }
            break;
        }
        default: SkUNREACHABLE;
    }

    SkASSERT(loopInfo.fCount >= 0);
    if (loopInfo.fCount >= kLoopTerminationLimit) {
        return "loop must guarantee termination in fewer iterations";
    }

    return nullptr;  // All checks pass
}

std::unique_ptr<LoopUnrollInfo> Analysis::GetLoopUnrollInfo(int line,
                                                            const Statement* loopInitializer,
                                                            const Expression* loopTest,
                                                            const Expression* loopNext,
                                                            const Statement* loopStatement,
                                                            ErrorReporter* errors) {
    auto result = std::make_unique<LoopUnrollInfo>();
    if (const char* msg = invalid_for_ES2(line, loopInitializer, loopTest, loopNext,
                                          loopStatement, *result)) {
        result = nullptr;
        if (errors) {
            errors->error(line, msg);
        }
    }
    return result;
}

// Checks for ES2 constant-expression rules, and (optionally) constant-index-expression rules
// (if loopIndices is non-nullptr)
class ConstantExpressionVisitor : public ProgramVisitor {
public:
    ConstantExpressionVisitor(const std::set<const Variable*>* loopIndices)
            : fLoopIndices(loopIndices) {}

    bool visitExpression(const Expression& e) override {
        // A constant-(index)-expression is one of...
        switch (e.kind()) {
            // ... a literal value
            case Expression::Kind::kLiteral:
                return false;

            // ... settings can appear in fragment processors; they will resolve when compiled
            case Expression::Kind::kSetting:
                return false;

            // ... a global or local variable qualified as 'const', excluding function parameters.
            // ... loop indices as defined in section 4. [constant-index-expression]
            case Expression::Kind::kVariableReference: {
                const Variable* v = e.as<VariableReference>().variable();
                if ((v->storage() == Variable::Storage::kGlobal ||
                     v->storage() == Variable::Storage::kLocal) &&
                    (v->modifiers().fFlags & Modifiers::kConst_Flag)) {
                    return false;
                }
                return !fLoopIndices || fLoopIndices->find(v) == fLoopIndices->end();
            }

            // ... expressions composed of both of the above
            case Expression::Kind::kBinary:
            case Expression::Kind::kConstructorArray:
            case Expression::Kind::kConstructorArrayCast:
            case Expression::Kind::kConstructorCompound:
            case Expression::Kind::kConstructorCompoundCast:
            case Expression::Kind::kConstructorDiagonalMatrix:
            case Expression::Kind::kConstructorMatrixResize:
            case Expression::Kind::kConstructorScalarCast:
            case Expression::Kind::kConstructorSplat:
            case Expression::Kind::kConstructorStruct:
            case Expression::Kind::kFieldAccess:
            case Expression::Kind::kIndex:
            case Expression::Kind::kPrefix:
            case Expression::Kind::kPostfix:
            case Expression::Kind::kSwizzle:
            case Expression::Kind::kTernary:
                return INHERITED::visitExpression(e);

            // Function calls are completely disallowed in SkSL constant-(index)-expressions.
            // GLSL does mandate that calling a built-in function where the arguments are all
            // constant-expressions should result in a constant-expression. SkSL handles this by
            // optimizing fully-constant function calls into literals in FunctionCall::Make.
            case Expression::Kind::kFunctionCall:
            case Expression::Kind::kExternalFunctionCall:
            case Expression::Kind::kChildCall:

            // These shouldn't appear in a valid program at all, and definitely aren't
            // constant-index-expressions.
            case Expression::Kind::kPoison:
            case Expression::Kind::kFunctionReference:
            case Expression::Kind::kExternalFunctionReference:
            case Expression::Kind::kMethodReference:
            case Expression::Kind::kTypeReference:
            case Expression::Kind::kCodeString:
                return true;

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
            ConstantExpressionVisitor indexerInvalid(&fLoopIndices);
            if (indexerInvalid.visitExpression(*i.index())) {
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

bool Analysis::IsConstantExpression(const Expression& expr) {
    ConstantExpressionVisitor visitor(/*loopIndices=*/nullptr);
    return !visitor.visitExpression(expr);
}

bool Analysis::CanExitWithoutReturningValue(const FunctionDeclaration& funcDecl,
                                            const Statement& body) {
    if (funcDecl.returnType().isVoid()) {
        return false;
    }
    ReturnsOnAllPathsVisitor visitor;
    visitor.visitStatement(body);
    return !visitor.fFoundReturn;
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
    for (const std::unique_ptr<ProgramElement>& element : program.ownedElements()) {
        visitor.visitProgramElement(*element);
    }
}

void Analysis::EliminateUnreachableCode(std::unique_ptr<Statement>& stmt, ProgramUsage* usage) {
    class UnreachableCodeEliminator : public ProgramWriter {
    public:
        UnreachableCodeEliminator(ProgramUsage* usage)
                : fUsage(usage) {
            fFoundFunctionExit.push(false);
            fFoundLoopExit.push(false);
        }

        bool visitExpressionPtr(std::unique_ptr<Expression>& expr) override {
            // We don't need to look inside expressions at all.
            return false;
        }

        bool visitStatementPtr(std::unique_ptr<Statement>& stmt) override {
            if (fFoundFunctionExit.top() || fFoundLoopExit.top()) {
                // If we already found an exit in this section, anything beyond it is dead code.
                if (!stmt->is<Nop>()) {
                    // Eliminate the dead statement by substituting a Nop.
                    if (fUsage) {
                        fUsage->remove(stmt.get());
                    }
                    stmt = std::make_unique<Nop>();
                }
                return false;
            }

            switch (stmt->kind()) {
                case Statement::Kind::kReturn:
                case Statement::Kind::kDiscard:
                    // We found a function exit on this path.
                    fFoundFunctionExit.top() = true;
                    break;

                case Statement::Kind::kBreak:
                case Statement::Kind::kContinue:
                    // We found a loop exit on this path. Note that we skip over switch statements
                    // completely when eliminating code, so any `break` statement would be breaking
                    // out of a loop, not out of a switch.
                    fFoundLoopExit.top() = true;
                    break;

                case Statement::Kind::kExpression:
                case Statement::Kind::kInlineMarker:
                case Statement::Kind::kNop:
                case Statement::Kind::kVarDeclaration:
                    // These statements don't affect control flow.
                    break;

                case Statement::Kind::kBlock:
                    // Blocks are on the straight-line path and don't affect control flow.
                    return INHERITED::visitStatementPtr(stmt);

                case Statement::Kind::kDo: {
                    // Function-exits are allowed to propagate outside of a do-loop, because it
                    // always executes its body at least once.
                    fFoundLoopExit.push(false);
                    bool result = INHERITED::visitStatementPtr(stmt);
                    fFoundLoopExit.pop();
                    return result;
                }
                case Statement::Kind::kFor: {
                    // Function-exits are not allowed to propagate out, because a for-loop or while-
                    // loop could potentially run zero times.
                    fFoundFunctionExit.push(false);
                    fFoundLoopExit.push(false);
                    bool result = INHERITED::visitStatementPtr(stmt);
                    fFoundLoopExit.pop();
                    fFoundFunctionExit.pop();
                    return result;
                }
                case Statement::Kind::kIf: {
                    // This statement is conditional and encloses two inner sections of code.
                    // If both sides contain a function-exit or loop-exit, that exit is allowed to
                    // propagate out.
                    IfStatement& ifStmt = stmt->as<IfStatement>();

                    fFoundFunctionExit.push(false);
                    fFoundLoopExit.push(false);
                    bool result = (ifStmt.ifTrue() && this->visitStatementPtr(ifStmt.ifTrue()));
                    bool foundFunctionExitOnTrue = fFoundFunctionExit.top();
                    bool foundLoopExitOnTrue = fFoundLoopExit.top();
                    fFoundFunctionExit.pop();
                    fFoundLoopExit.pop();

                    fFoundFunctionExit.push(false);
                    fFoundLoopExit.push(false);
                    result |= (ifStmt.ifFalse() && this->visitStatementPtr(ifStmt.ifFalse()));
                    bool foundFunctionExitOnFalse = fFoundFunctionExit.top();
                    bool foundLoopExitOnFalse = fFoundLoopExit.top();
                    fFoundFunctionExit.pop();
                    fFoundLoopExit.pop();

                    fFoundFunctionExit.top() |= foundFunctionExitOnTrue && foundFunctionExitOnFalse;
                    fFoundLoopExit.top() |= foundLoopExitOnTrue && foundLoopExitOnFalse;
                    return result;
                }
                case Statement::Kind::kSwitch:
                case Statement::Kind::kSwitchCase:
                    // We skip past switch statements entirely when scanning for dead code. Their
                    // control flow is quite complex and we already do a good job of flattening out
                    // switches on constant values.
                    break;
            }

            return false;
        }

        ProgramUsage* fUsage;
        std::stack<bool> fFoundFunctionExit;
        std::stack<bool> fFoundLoopExit;

        using INHERITED = ProgramWriter;
    };

    UnreachableCodeEliminator visitor{usage};
    visitor.visitStatementPtr(stmt);
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
