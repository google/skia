/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkColorMatrixFilter_DEFINED
#define SkColorMatrixFilter_DEFINED

#include "SkColorFilter.h"

class SK_API SkColorMatrixFilter : public SkColorFilter {
public:
    explicit SkColorMatrixFilter(const float colMajor[20]);

    void filterSpan(const SkPMColor src[], int count, SkPMColor[]) const override;
    void filterSpan4f(const SkPM4f src[], int count, SkPM4f[]) const override;
    uint32_t getFlags() const override;
    bool asColorMatrix(float matrix[20]) const override;
    sk_sp<SkColorFilter> makeComposed(sk_sp<SkColorFilter>) const override;

#if SK_SUPPORT_GPU
    sk_sp<GrFragmentProcessor> asFragmentProcessor(GrContext*, SkColorSpace*) const override;
#endif

    SK_TO_STRING_OVERRIDE()

    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkColorMatrixFilter)

protected:
    void flatten(SkWriteBuffer&) const override;

private:
    bool onAppendStages(SkRasterPipeline*, SkColorSpace*, SkArenaAlloc*,
                        bool shaderIsOpaque) const override;

    float       fColMajor[20];
    uint32_t    fFlags;

    void initState();

    typedef SkColorFilter INHERITED;
};

#endif
