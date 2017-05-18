/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkColorMatrixFilter.h"
#include "SkColorSpaceXformer.h"
#if SK_SUPPORT_GPU
    #include "GrFragmentProcessor.h"
#endif

static SkScalar byte_to_scale(U8CPU byte) {
    if (0xFF == byte) {
        // want to get this exact
        return 1;
    } else {
        return byte * 0.00392156862745f;
    }
}

// If we can't reduce to a mode filter in MakeLightingFilter(), this is the general case.
// We operate as a matrix color filter, but remember our input colors in case we're asked
// to onMakeColorSpace() a new filter.
class SkLightingColorFilter : public SkColorFilter {
public:
    SkLightingColorFilter(SkColor mul, SkColor add) : fMul(mul), fAdd(add) {
        SkColorMatrix matrix;
        matrix.setScale(byte_to_scale(SkColorGetR(mul)),
                        byte_to_scale(SkColorGetG(mul)),
                        byte_to_scale(SkColorGetB(mul)),
                        1);
        matrix.postTranslate(SkIntToScalar(SkColorGetR(add)),
                             SkIntToScalar(SkColorGetG(add)),
                             SkIntToScalar(SkColorGetB(add)),
                             0);
        fMatrixFilter = SkColorFilter::MakeMatrixFilterRowMajor255(matrix.fMat);
    }

    // Overriding this method is the class' raison d'etre.
    sk_sp<SkColorFilter> onMakeColorSpace(SkColorSpaceXformer* xformer) const override {
        return sk_make_sp<SkLightingColorFilter>(xformer->apply(fMul), xformer->apply(fAdd));
    }

    // Let fMatrixFilter handle all the other calls directly.
    void filterSpan(const SkPMColor src[], int count, SkPMColor dst[]) const override {
        fMatrixFilter->filterSpan(src, count, dst);
    }
    void filterSpan4f(const SkPM4f src[], int count, SkPM4f dst[]) const override {
        fMatrixFilter->filterSpan4f(src, count, dst);
    }
    uint32_t getFlags() const override {
        return fMatrixFilter->getFlags();
    }
    bool asColorMatrix(SkScalar matrix[20]) const override {
        return fMatrixFilter->asColorMatrix(matrix);
    }
    void onAppendStages(SkRasterPipeline* p, SkColorSpace* cs, SkArenaAlloc* alloc,
                        bool shaderIsOpaque) const override {
        fMatrixFilter->appendStages(p, cs, alloc, shaderIsOpaque);
    }

    // TODO: might want to remember we're a lighting color filter through serialization?
    void flatten(SkWriteBuffer& buf) const override { return fMatrixFilter->flatten(buf); }
    Factory getFactory() const override             { return fMatrixFilter->getFactory(); }

#if SK_SUPPORT_GPU
    sk_sp<GrFragmentProcessor> asFragmentProcessor(GrContext* ctx,
                                                   SkColorSpace* cs) const override {
        return fMatrixFilter->asFragmentProcessor(ctx, cs);
    }
#endif

#ifndef SK_IGNORE_TO_STRING
    void toString(SkString* str) const override { fMatrixFilter->toString(str); }
#endif

private:
    SkColor              fMul, fAdd;
    sk_sp<SkColorFilter> fMatrixFilter;
};

sk_sp<SkColorFilter> SkColorMatrixFilter::MakeLightingFilter(SkColor mul, SkColor add) {
    const SkColor opaqueAlphaMask = SK_ColorBLACK;
    // omit the alpha and compare only the RGB values
    if (0 == (add & ~opaqueAlphaMask)) {
        return SkColorFilter::MakeModeFilter(mul | opaqueAlphaMask, SkBlendMode::kModulate);
    }
    return sk_make_sp<SkLightingColorFilter>(mul, add);
}
