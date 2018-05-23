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
    sk_sp<SkColorFilter> onMakeComposed(sk_sp<SkColorFilter>) const override;

#if SK_SUPPORT_GPU
    std::unique_ptr<GrFragmentProcessor> asFragmentProcessor(
            GrContext*, const GrColorSpaceInfo&) const override;
#endif

    Factory getFactory() const override { return CreateProc; }

protected:
    void flatten(SkWriteBuffer&) const override;

private:
    void onAppendStages(SkRasterPipeline*, SkColorSpace*, SkArenaAlloc*,
                        bool shaderIsOpaque) const override;
    static sk_sp<SkFlattenable> CreateProc(SkReadBuffer&);
    friend class SkFlattenable::PrivateInitializer;

    SkScalar        fMatrix[20];
    float           fTranspose[20]; // for Sk4s
    uint32_t        fFlags;

    void initState();

    typedef SkColorFilter INHERITED;
};

#endif
