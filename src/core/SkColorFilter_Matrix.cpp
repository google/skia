/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkRefCnt.h"
#include "include/core/SkString.h"
#include "include/core/SkUnPreMultiply.h"
#include "include/effects/SkColorMatrix.h"
#include "include/private/SkColorData.h"
#include "include/private/SkNx.h"
#include "src/core/SkColorFilter_Matrix.h"
#include "src/core/SkRasterPipeline.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkWriteBuffer.h"

static uint16_t ComputeFlags(const float matrix[20]) {
    const float* srcA = matrix + 15;

    return SkScalarNearlyZero (srcA[0])
        && SkScalarNearlyZero (srcA[1])
        && SkScalarNearlyZero (srcA[2])
        && SkScalarNearlyEqual(srcA[3], 1)
        && SkScalarNearlyZero (srcA[4])
            ? SkColorFilter::kAlphaUnchanged_Flag : 0;
}

SkColorFilter_Matrix::SkColorFilter_Matrix(const float array[20], Domain domain)
    : fFlags(ComputeFlags(array))
    , fDomain(domain) {
    memcpy(fMatrix, array, 20 * sizeof(float));
}

uint32_t SkColorFilter_Matrix::getFlags() const {
    return this->INHERITED::getFlags() | fFlags;
}

void SkColorFilter_Matrix::flatten(SkWriteBuffer& buffer) const {
    SkASSERT(sizeof(fMatrix)/sizeof(float) == 20);
    buffer.writeScalarArray(fMatrix, 20);

    // RGBA flag
    buffer.writeBool(fDomain == Domain::kRGBA);
}

sk_sp<SkFlattenable> SkColorFilter_Matrix::CreateProc(SkReadBuffer& buffer) {
    float matrix[20];
    if (!buffer.readScalarArray(matrix, 20)) {
        return nullptr;
    }

    auto   is_rgba = buffer.isVersionLT(SkPicturePriv::kMatrixColorFilterDomain_Version) ||
                     buffer.readBool();
    return is_rgba ? SkColorFilters::Matrix(matrix)
                   : SkColorFilters::HSLAMatrix(matrix);
}

bool SkColorFilter_Matrix::onAsAColorMatrix(float matrix[20]) const {
    if (matrix) {
        memcpy(matrix, fMatrix, 20 * sizeof(float));
    }
    return true;
}

bool SkColorFilter_Matrix::onAppendStages(const SkStageRec& rec, bool shaderIsOpaque) const {
    const bool willStayOpaque = shaderIsOpaque && (fFlags & kAlphaUnchanged_Flag),
                         hsla = fDomain == Domain::kHSLA;

    SkRasterPipeline* p = rec.fPipeline;
    if (!shaderIsOpaque) { p->append(SkRasterPipeline::unpremul); }
    if (           hsla) { p->append(SkRasterPipeline::rgb_to_hsl); }
    if (           true) { p->append(SkRasterPipeline::matrix_4x5, fMatrix); }
    if (           hsla) { p->append(SkRasterPipeline::hsl_to_rgb); }
    if (           true) { p->append(SkRasterPipeline::clamp_0); }
    if (           true) { p->append(SkRasterPipeline::clamp_1); }
    if (!willStayOpaque) { p->append(SkRasterPipeline::premul); }
    return true;
}

bool SkColorFilter_Matrix::program(skvm::Builder* p,
                                   SkColorSpace* /*dstCS*/,
                                   skvm::Uniforms* uniforms,
                                   skvm::F32* r, skvm::F32* g, skvm::F32* b, skvm::F32* a) const {
    // TODO: specialize generated code on the 0/1 values of fMatrix?
    if (fDomain == Domain::kRGBA) {
        // Unpremul.
        skvm::F32 invA = p->div(p->splat(1.0f), *a),
                  inf  = p->bit_cast(p->splat(0x7f800000));

        // If *a is 0, so are *r,*g,*b, so set invA to 0 to avoid 0*inf=NaN (instead 0*0 = 0).
        invA = p->bit_cast(p->bit_and(p->lt(invA, inf),
                                      p->bit_cast(invA)));
        *r = p->mul(*r, invA);
        *g = p->mul(*g, invA);
        *b = p->mul(*b, invA);

        // Apply matrix.
        skvm::Builder::Uniform u = uniforms->pushF(fMatrix, 20);
        auto m = [&](int i) { return p->uniformF(u.ptr, u.offset + 4*i); };

        skvm::F32 rgba[4];
        for (int j = 0; j < 4; j++) {
            rgba[j] =        m(4+j*5);
            rgba[j] = p->mad(m(3+j*5), *a, rgba[j]);
            rgba[j] = p->mad(m(2+j*5), *b, rgba[j]);
            rgba[j] = p->mad(m(1+j*5), *g, rgba[j]);
            rgba[j] = p->mad(m(0+j*5), *r, rgba[j]);
        }
        *r = rgba[0];
        *g = rgba[1];
        *b = rgba[2];
        *a = rgba[3];

        // Premul.
        *r = p->mul(*r, *a);
        *g = p->mul(*g, *a);
        *b = p->mul(*b, *a);

        return true;
    }
    return false;
}

#if SK_SUPPORT_GPU
#include "src/gpu/effects/generated/GrColorMatrixFragmentProcessor.h"
#include "src/gpu/effects/generated/GrHSLToRGBFilterEffect.h"
#include "src/gpu/effects/generated/GrRGBToHSLFilterEffect.h"
std::unique_ptr<GrFragmentProcessor> SkColorFilter_Matrix::asFragmentProcessor(
        GrRecordingContext*, const GrColorInfo&) const {
    switch (fDomain) {
        case Domain::kRGBA:
            return GrColorMatrixFragmentProcessor::Make(fMatrix,
                                                        /* premulInput = */    true,
                                                        /* clampRGBOutput = */ true,
                                                        /* premulOutput = */   true);
        case Domain::kHSLA: {
            std::unique_ptr<GrFragmentProcessor> series[] = {
                GrRGBToHSLFilterEffect::Make(),
                GrColorMatrixFragmentProcessor::Make(fMatrix,
                                                     /* premulInput = */    false,
                                                     /* clampRGBOutput = */ false,
                                                     /* premulOutput = */   false),
                GrHSLToRGBFilterEffect::Make(),
            };
            return GrFragmentProcessor::RunInSeries(series, SK_ARRAY_COUNT(series));
        }
    }

    SkUNREACHABLE;
}

#endif

///////////////////////////////////////////////////////////////////////////////

static sk_sp<SkColorFilter> MakeMatrix(const float array[20],
                                       SkColorFilter_Matrix::Domain domain) {
    return sk_floats_are_finite(array, 20)
        ? sk_make_sp<SkColorFilter_Matrix>(array, domain)
        : nullptr;
}

sk_sp<SkColorFilter> SkColorFilters::Matrix(const float array[20]) {
    return MakeMatrix(array, SkColorFilter_Matrix::Domain::kRGBA);
}

sk_sp<SkColorFilter> SkColorFilters::Matrix(const SkColorMatrix& cm) {
    return MakeMatrix(cm.fMat.data(), SkColorFilter_Matrix::Domain::kRGBA);
}

sk_sp<SkColorFilter> SkColorFilters::HSLAMatrix(const float array[20]) {
    return MakeMatrix(array, SkColorFilter_Matrix::Domain::kHSLA);
}

void SkColorFilter_Matrix::RegisterFlattenables() {
    SK_REGISTER_FLATTENABLE(SkColorFilter_Matrix);

    // This subclass was removed 4/2019
    SkFlattenable::Register("SkColorMatrixFilterRowMajor255",
                            [](SkReadBuffer& buffer) -> sk_sp<SkFlattenable> {
        float matrix[20];
        if (buffer.readScalarArray(matrix, 20)) {
            matrix[ 4] *= (1.0f/255);
            matrix[ 9] *= (1.0f/255);
            matrix[14] *= (1.0f/255);
            matrix[19] *= (1.0f/255);
            return SkColorFilters::Matrix(matrix);
        }
        return nullptr;
    });
}
