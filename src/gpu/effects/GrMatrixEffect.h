/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrMatrixEffect_DEFINED
#define GrMatrixEffect_DEFINED

#include "include/core/SkM44.h"
#include "include/core/SkTypes.h"

#include "src/gpu/GrFragmentProcessor.h"

class GrMatrixEffect : public GrFragmentProcessor {
public:
    static std::unique_ptr<GrFragmentProcessor> Make(const SkMatrix& matrix,
                                                     std::unique_ptr<GrFragmentProcessor> child);

    std::unique_ptr<GrFragmentProcessor> clone() const override;
    const char* name() const override { return "MatrixEffect"; }

private:
    GrMatrixEffect(const GrMatrixEffect& src);

    GrMatrixEffect(SkMatrix matrix, std::unique_ptr<GrFragmentProcessor> child)
            : INHERITED(kGrMatrixEffect_ClassID, ProcessorOptimizationFlags(child.get()))
            , fMatrix(matrix) {
        SkASSERT(child);
        this->registerChild(std::move(child),
                            SkSL::SampleUsage::UniformMatrix(matrix.hasPerspective()));
    }

    std::unique_ptr<ProgramImpl> onMakeProgramImpl() const override;
    void onAddToKey(const GrShaderCaps&, GrProcessorKeyBuilder*) const override;
    bool onIsEqual(const GrFragmentProcessor&) const override;
    SkPMColor4f constantOutputForConstantInput(const SkPMColor4f& inputColor) const override {
        return ConstantOutputForConstantInput(this->childProcessor(0), inputColor);
    }

    SkMatrix fMatrix;

    GR_DECLARE_FRAGMENT_PROCESSOR_TEST
    using INHERITED = GrFragmentProcessor;
};
#endif
