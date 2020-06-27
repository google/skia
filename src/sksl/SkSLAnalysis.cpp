/*
 * Copyright 2020 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/SkSLAnalysis.h"
#include "src/sksl/SkSLVisitor.h"

namespace SkSL {

namespace {
    static bool is_sample_call_to_fp(const FunctionCall& fc, const Variable& fp) {
        const FunctionDeclaration& f = fc.fFunction;
        return f.fBuiltin && f.fName == "sample" && fc.fArguments.size() >= 1 &&
               fc.fArguments[0]->fKind == Expression::kVariableReference_Kind &&
               &((VariableReference&) *fc.fArguments[0]).fVariable == &fp;
    }

    // Visitor that determines the merged SampleMatrix for a given child 'fp' in the program.
    class MergeSampleMatrixVisitor : public ProgramVisitor {
    public:
        MergeSampleMatrixVisitor(const Variable& fp) : fFP(fp) {}

        SampleMatrix visit(const Program& program) {
            fMatrix = SampleMatrix(); // reset to none
            this->INHERITED::visit(program);
            return fMatrix;
        }

    protected:
        const Variable& fFP;
        SampleMatrix fMatrix;

        bool visitFunctionCall(const FunctionCall& fc) override {
            // Looking for sample(fp, inColor?, float3x3)
            if (is_sample_call_to_fp(fc, fFP) && fc.fArguments.size() >= 2 &&
                fc.fArguments.back()->fType == *this->program().fContext->fFloat3x3_Type) {
                // Determine the type of matrix for this call site, then merge it with the
                // previously accumulated matrix state.
                if (fc.fArguments.back()->isConstantOrUniform()) {
                    if (fc.fArguments.back()->fKind == Expression::Kind::kVariableReference_Kind ||
                        fc.fArguments.back()->fKind == Expression::Kind::kConstructor_Kind) {
                        // FIXME if this is a constant, we should parse the float3x3 constructor and
                        // determine if the resulting matrix introduces perspective.
                        fMatrix.merge(SampleMatrix::MakeConstUniform(
                                fc.fArguments.back()->description()));
                    } else {
                        // FIXME this is really to workaround a restriction of the downstream code
                        // that relies on the SampleMatrix's fExpression to identify uniform names.
                        // Once they are tracked separately, any constant/uniform expression can
                        // work, but right now this avoids issues from '0.5 * matrix' that is both
                        // a constant AND a uniform.
                        fMatrix.merge(SampleMatrix::MakeVariable());
                    }
                } else {
                    fMatrix.merge(SampleMatrix::MakeVariable());
                }
                // NOTE: we don't return true here just because we found a sample matrix usage,
                // we need to process the entire program and merge across all encountered calls.
            }

            return this->INHERITED::visitFunctionCall(fc);
        }

        typedef ProgramVisitor INHERITED;
    };

    // Visitor that searches a program for sample() calls with the given 'fp' as the argument
    // and returns true if explicit float2 coords were passed to that call site.
    class ExplicitCoordsVisitor : public ProgramVisitor {
    public:
        ExplicitCoordsVisitor(const Variable& fp) : fFP(fp) {}

    protected:
        bool visitFunctionCall(const FunctionCall& fc) override {
            // Looking for sample(fp, inColor?, float2)
            if (is_sample_call_to_fp(fc, fFP) && fc.fArguments.size() >= 2 &&
                fc.fArguments.back()->fType == *this->program().fContext->fFloat2_Type) {
                return true;
            }
            return this->INHERITED::visitFunctionCall(fc);
        }

        const Variable& fFP;

        typedef ProgramVisitor INHERITED;
    };

    // Visitor that searches through a main function of the program for reference to the
    // sample coordinates provided by the parent FP or main program.
    class SampleCoordsVisitor : public ProgramVisitor {
    protected:
        // Only bother recursing through the main function for the sample coord builtin
        bool visitFunctionDefinition(const FunctionDefinition& func) override {
            if (func.fDeclaration.fName == "main") {
                return this->INHERITED::visitFunctionDefinition(func);
            } else {
                // No recursion, but returning false will allow visitor to continue to siblings
                return false;
            }
        }

        bool visitVarReference(const VariableReference& var) override {
            // For SkRuntimeEffects
            return var.fVariable.fModifiers.fLayout.fBuiltin == SK_MAIN_COORDS_BUILTIN;
        }

        bool visitIndex(const IndexExpression& index) override {
            // For .fp files that use sk_TransformedCoords2D[0] for the time being
            if (index.fBase->fKind == Expression::kVariableReference_Kind) {
                const VariableReference& base = (const VariableReference&) *index.fBase;
                if (base.fVariable.fModifiers.fLayout.fBuiltin == SK_TRANSFORMEDCOORDS2D_BUILTIN) {
                    SkASSERT(index.fIndex->fKind == Expression::kIntLiteral_Kind &&
                             ((IntLiteral&) *index.fIndex).fValue == 0);
                    return true;
                }
            }
            return this->INHERITED::visitIndex(index);
        }

        typedef ProgramVisitor INHERITED;
    };
}


SampleMatrix Analysis::GetSampleMatrix(const Program& program, const Variable& fp) {
    MergeSampleMatrixVisitor visitor(fp);
    return visitor.visit(program);
}

bool Analysis::IsExplicitlySampled(const Program& program, const Variable& fp) {
    ExplicitCoordsVisitor visitor(fp);
    return visitor.visit(program);
}

bool Analysis::ReferencesSampleCoords(const Program& program) {
    SampleCoordsVisitor visitor;
    return visitor.visit(program);
}

}
