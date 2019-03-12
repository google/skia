/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkColorMatrixFilter_DEFINED
#define SkColorMatrixFilter_DEFINED

#include "SkFlattenable.h"
#include "SkColorFilter.h"

class SkColorMatrixFilterRowMajor255 : public SkColorFilter {
public:
    SkColorMatrixFilterRowMajor255() {}
    explicit SkColorMatrixFilterRowMajor255(const SkScalar array[20]);

    /** Creates a color matrix filter that returns the same value in all four channels. */
    static sk_sp<SkColorFilter> MakeSingleChannelOutput(const SkScalar row[5]);

    uint32_t getFlags() const override;
    bool asColorMatrix(SkScalar matrix[20]) const override;

#if SK_SUPPORT_GPU
    std::unique_ptr<GrFragmentProcessor> asFragmentProcessor(
            GrRecordingContext*, const GrColorSpaceInfo&) const override;
#endif

protected:
    void flatten(SkWriteBuffer&) const override;

private:
    SK_FLATTENABLE_HOOKS(SkColorMatrixFilterRowMajor255)

    void onAppendStages(SkRasterPipeline*, SkColorSpace*, SkArenaAlloc*,
                        bool shaderIsOpaque) const override;

    SkScalar        fMatrix[20];
    float           fTranspose[20]; // for Sk4s
    uint32_t        fFlags;

    void initState();

    typedef SkColorFilter INHERITED;
};

#endif
