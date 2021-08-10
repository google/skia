/*
 * Copyright 2020 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLAnalysis.h"

#include "include/private/SkSLModifiers.h"
#include "include/private/SkSLProgramElement.h"
#include "include/private/SkSLSampleUsage.h"
#include "include/private/SkSLStatement.h"
#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLErrorReporter.h"
#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLProgram.h"

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
#include "src/sksl/ir/SkSLBoolLiteral.h"
#include "src/sksl/ir/SkSLConstructor.h"
#include "src/sksl/ir/SkSLConstructorDiagonalMatrix.h"
#include "src/sksl/ir/SkSLConstructorMatrixResize.h"
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
    return f.intrinsicKind() == k_sample_IntrinsicKind && fc.arguments().size() >= 1 &&
           fc.arguments()[0]->is<VariableReference>() &&
           fc.arguments()[0]->as<VariableReference>().variable() == &fp;
}

// Visitor that determines the merged SampleUsage for a given child 'fp' in the program.
class MergeSampleUsageVisitor : public ProgramVisitor {
public:
    MergeSampleUsageVisitor(const Context& context, const Variable& fp, bool writesToSampleCoords)
            : fContext(context), fFP(fp), fWritesToSampleCoords(writesToSampleCoords) {}

    SampleUsage visit(const Program& program) {
        fUsage = SampleUsage(); // reset to none
        INHERITED::visit(program);
        return fUsage;
    }

    int elidedSampleCoordCount() const { return fElidedSampleCoordCount; }

protected:
    const Context& fContext;
    const Variable& fFP;
    const bool fWritesToSampleCoords;
    SampleUsage fUsage;
    int fElidedSampleCoordCount = 0;

    bool visitExpression(const Expression& e) override {
        // Looking for sample(fp, ...)
        if (e.is<FunctionCall>()) {
            const FunctionCall& fc = e.as<FunctionCall>();
            if (is_sample_call_to_fp(fc, fFP)) {
                // Determine the type of call at this site, and merge it with the accumulated state
                if (fc.arguments().size() >= 2) {
                    const Expression* coords = fc.arguments()[1].get();
                    if (coords->type() == *fContext.fTypes.fFloat2) {
                        // If the coords are a direct reference to the program's sample-coords,
                        // and those coords are never modified, we can conservatively turn this
                        // into PassThrough sampling. In all other cases, we consider it Explicit.
                        if (!fWritesToSampleCoords && coords->is<VariableReference>() &&
                            coords->as<VariableReference>()
                                            .variable()
                                            ->modifiers()
                                            .fLayout.fBuiltin == SK_MAIN_COORDS_BUILTIN) {
                            fUsage.merge(SampleUsage::PassThrough());
                            ++fElidedSampleCoordCount;
                        } else {
                            fUsage.merge(SampleUsage::Explicit());
                        }
                    } else {
                        // sample(fp, half4 inputColor) -> PassThrough
                        fUsage.merge(SampleUsage::PassThrough());
                    }
                } else {
                    // sample(fp) -> PassThrough
                    fUsage.merge(SampleUsage::PassThrough());
                }
                // NOTE: we don't return true here just because we found a sample call. We need to
                // process the entire program and merge across all encountered calls.
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

// Visitor that searches for calls to sample() from a function other than main()
class SampleOutsideMainVisitor : public ProgramVisitor {
public:
    SampleOutsideMainVisitor() {}

    bool visitExpression(const Expression& e) override {
        if (e.is<FunctionCall>()) {
            const FunctionDeclaration& f = e.as<FunctionCall>().function();
            if (f.intrinsicKind() == k_sample_IntrinsicKind) {
                return true;
            }
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
    void handleError(const char*, dsl::PositionInfo) override { ++fErrorCount; }
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
        }
        return fErrors->errorCount() == oldErrorCount;
    }

    void visitExpression(Expression& expr) {
        switch (expr.kind()) {
            case Expression::Kind::kVariableReference: {
                VariableReference& varRef = expr.as<VariableReference>();
                const Variable* var = varRef.variable();
                if (var->modifiers().fFlags & (Modifiers::kConst_Flag | Modifiers::kUniform_Flag)) {
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
                this->checkSwizzleWrite(swizzle);
                this->visitExpression(*swizzle.base());
                break;
            }
            case Expression::Kind::kIndex:
                this->visitExpression(*expr.as<IndexExpression>().base());
                break;

            default:
                fErrors->error(expr.fOffset, "cannot assign to this expression");
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
                fErrors->error(swizzle.fOffset,
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
                                     const Variable& fp,
                                     bool writesToSampleCoords,
                                     int* elidedSampleCoordCount) {
    MergeSampleUsageVisitor visitor(*program.fContext, fp, writesToSampleCoords);
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

bool Analysis::DetectStaticRecursion(SkSpan<std::unique_ptr<ProgramElement>> programElements,
                                     ErrorReporter& errors) {
    using Function = const FunctionDeclaration;
    using CallSet = std::unordered_set<Function*>;
    using CallGraph = std::unordered_map<Function*, CallSet>;

    class CallGraphVisitor : public ProgramVisitor {
    public:
        CallGraphVisitor(CallGraph* calls) : fCallGraph(calls), fCurrentFunctionCalls(nullptr) {}

        bool visitExpression(const Expression& e) override {
            if (e.is<FunctionCall>()) {
                fCurrentFunctionCalls->insert(&e.as<FunctionCall>().function());
            }
            return INHERITED::visitExpression(e);
        }

        bool visitProgramElement(const ProgramElement& p) override {
            if (p.is<FunctionDefinition>()) {
                Function* fn = &p.as<FunctionDefinition>().declaration();
                SkASSERT(fCallGraph->count(fn) == 0);

                SkASSERT(fCurrentFunctionCalls == nullptr);
                CallSet currentFunctionCalls;
                fCurrentFunctionCalls = &currentFunctionCalls;

                INHERITED::visitProgramElement(p);

                fCurrentFunctionCalls = nullptr;
                fCallGraph->insert({fn, std::move(currentFunctionCalls)});
            }
            return false;
        }

        CallGraph* fCallGraph;
        CallSet*   fCurrentFunctionCalls;

        using INHERITED = ProgramVisitor;
    };

    CallGraph callGraph;
    CallGraphVisitor visitor{&callGraph};
    for (const auto& pe : programElements) {
        visitor.visitProgramElement(*pe);
    }

    class CycleFinder {
    public:
        CycleFinder(CallGraph* calls) : fCallGraph(calls) {}

        bool containsCycle() {
            for (const auto& [caller, callees] : *fCallGraph) {
                SkASSERT(fStack.empty());
                if (this->dfsHelper(caller)) {
                    return true;
                }
            }
            return false;
        }

        const std::vector<Function*>& cycle() const { return fStack; }

    private:
        bool dfsHelper(Function* fn) {
            SkASSERT(std::find(fStack.begin(), fStack.end(), fn) == fStack.end());

            auto iter = fCallGraph->find(fn);
            if (iter != fCallGraph->end()) {
                fStack.push_back(fn);

                for (Function* calledFn : iter->second) {
                    auto it = std::find(fStack.begin(), fStack.end(), calledFn);
                    if (it != fStack.end()) {
                        // Cycle detected. It includes the functions from 'it' to the end of fStack
                        fStack.erase(fStack.begin(), it);
                        return true;
                    }
                    if (this->dfsHelper(calledFn)) {
                        return true;
                    }
                }

                fStack.pop_back();
            }

            return false;
        }

        const CallGraph*       fCallGraph;
        std::vector<Function*> fStack;
    };

    CycleFinder cycleFinder{&callGraph};
    if (cycleFinder.containsCycle()) {
        // Get the description of each function participating in the cycle
        std::vector<String> fnNames;
        for (Function* fn : cycleFinder.cycle()) {
            fnNames.push_back(fn->description());
        }

        // Find the lexicographically first function description, so we generate stable errors
        std::vector<String>::iterator cycleStart = std::min_element(fnNames.begin(), fnNames.end());
        ptrdiff_t startIndex = std::distance(fnNames.begin(), cycleStart);

        // Construct a list of the functions participating in the cycle (including the "start"
        // at both the beginning and end):
        String cycleDescription;
        for (size_t i = 0; i <= fnNames.size(); ++i) {
            cycleDescription += "\n\t" + fnNames[(i + startIndex) % fnNames.size()];
        }

        // Go back to the original data to find the offset of the cycle start's declaration
        Function* cycleStartFn = cycleFinder.cycle()[startIndex];
        errors.error(cycleStartFn->fOffset,
                     "potential recursion (function call cycle) not allowed:" + cycleDescription);
        return true;
    }
    return false;
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

void Analysis::UpdateRefKind(Expression* expr, VariableRefKind refKind) {
    class RefKindWriter : public ProgramWriter {
    public:
        RefKindWriter(VariableReference::RefKind refKind) : fRefKind(refKind) {}

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

    RefKindWriter{refKind}.visitExpression(*expr);
}

bool Analysis::MakeAssignmentExpr(Expression* expr,
                                  VariableReference::RefKind kind,
                                  ErrorReporter* errors) {
    Analysis::AssignmentInfo info;
    if (!Analysis::IsAssignable(*expr, &info, errors)) {
        return false;
    }
    if (!info.fAssignedVar) {
        errors->error(expr->fOffset, "can't assign to expression '" + expr->description() + "'");
        return false;
    }
    info.fAssignedVar->setRefKind(kind);
    return true;
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
           (expr.isAnyConstructor() &&
            expr.asAnyConstructor().argumentSpan().size() == 1 &&
            IsTrivialExpression(*expr.asAnyConstructor().argumentSpan().front())) ||
           (expr.isAnyConstructor() &&
            expr.isConstantOrUniform()) ||
           (expr.is<IndexExpression>() &&
            expr.as<IndexExpression>().index()->is<IntLiteral>() &&
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
        case Expression::Kind::kIntLiteral:
            return left.as<IntLiteral>().value() == right.as<IntLiteral>().value();

        case Expression::Kind::kFloatLiteral:
            return left.as<FloatLiteral>().value() == right.as<FloatLiteral>().value();

        case Expression::Kind::kBoolLiteral:
            return left.as<BoolLiteral>().value() == right.as<BoolLiteral>().value();

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

static bool get_constant_value(const Expression& expr, double* val) {
    const Expression* valExpr = expr.getConstantSubexpression(0);
    if (!valExpr) {
        return false;
    }
    if (valExpr->is<IntLiteral>()) {
        *val = static_cast<double>(valExpr->as<IntLiteral>().value());
        return true;
    }
    if (valExpr->is<FloatLiteral>()) {
        *val = static_cast<double>(valExpr->as<FloatLiteral>().value());
        return true;
    }
    SkDEBUGFAILF("unexpected constant type (%s)", expr.type().description().c_str());
    return false;
}

static const char* invalid_for_ES2(int offset,
                                   const Statement* loopInitializer,
                                   const Expression* loopTest,
                                   const Expression* loopNext,
                                   const Statement* loopStatement,
                                   Analysis::UnrollableLoopInfo& loopInfo) {
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
    if (!get_constant_value(*initDecl.value(), &loopInfo.fStart)) {
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
    if (!get_constant_value(*cond.right(), &loopEnd)) {
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
            if (!get_constant_value(*next.right(), &loopInfo.fDelta)) {
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
    constexpr int kMaxUnrollableLoopLength = 128;
    loopInfo.fCount = 0;

    double val = loopInfo.fStart;
    auto evalCond = [&]() {
        switch (cond.getOperator().kind()) {
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

bool Analysis::ForLoopIsValidForES2(int offset,
                                    const Statement* loopInitializer,
                                    const Expression* loopTest,
                                    const Expression* loopNext,
                                    const Statement* loopStatement,
                                    Analysis::UnrollableLoopInfo* outLoopInfo,
                                    ErrorReporter* errors) {
    UnrollableLoopInfo ignored,
                       *loopInfo = outLoopInfo ? outLoopInfo : &ignored;
    if (const char* msg = invalid_for_ES2(
                offset, loopInitializer, loopTest, loopNext, loopStatement, *loopInfo)) {
        if (errors) {
            errors->error(offset, msg);
        }
        return false;
    }
    return true;
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
            case Expression::Kind::kBoolLiteral:
            case Expression::Kind::kIntLiteral:
            case Expression::Kind::kFloatLiteral:
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

            // These are completely disallowed in SkSL constant-(index)-expressions. GLSL allows
            // calls to built-in functions where the arguments are all constant-expressions, but
            // we don't guarantee that behavior. (skbug.com/10835)
            case Expression::Kind::kExternalFunctionCall:
            case Expression::Kind::kFunctionCall:
                return true;

            case Expression::Kind::kPoison:
                return true;

            // These should never appear in final IR
            case Expression::Kind::kExternalFunctionReference:
            case Expression::Kind::kFunctionReference:
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
            ConstantExpressionVisitor indexerInvalid(&fLoopIndices);
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
        case Expression::Kind::kBoolLiteral:
        case Expression::Kind::kExternalFunctionReference:
        case Expression::Kind::kFloatLiteral:
        case Expression::Kind::kFunctionReference:
        case Expression::Kind::kIntLiteral:
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
