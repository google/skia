/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkColorFilter_Matrix_DEFINED
#define SkColorFilter_Matrix_DEFINED

#include "src/core/SkColorFilterBase.h"

class SkColorFilter_Matrix : public SkColorFilterBase {
public:
    enum class Domain : uint8_t { kRGBA, kHSLA };

    explicit SkColorFilter_Matrix(const float array[20], Domain);

    bool onIsAlphaUnchanged() const override { return fAlphaIsUnchanged; }

#if SK_SUPPORT_GPU
    GrFPResult asFragmentProcessor(std::unique_ptr<GrFragmentProcessor> inputFP,
                                   GrRecordingContext*, const GrColorInfo&) const override;
#endif

    static void RegisterFlattenables();

private:
    void flatten(SkWriteBuffer&) const override;
    bool onAsAColorMatrix(float matrix[20]) const override;

    SK_FLATTENABLE_HOOKS(SkColorFilter_Matrix)

    bool onAppendStages(const SkStageRec& rec, bool shaderIsOpaque) const override;
    skvm::Color onProgram(skvm::Builder*, skvm::Color,
                          SkColorSpace* dstCS,
                          skvm::Uniforms* uniforms, SkArenaAlloc*) const override;

    float   fMatrix[20];
    bool    fAlphaIsUnchanged;
    Domain  fDomain;

    using INHERITED = SkColorFilterBase;
};

#endif
