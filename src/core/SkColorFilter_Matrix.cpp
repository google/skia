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
                                   skvm::Arg uniforms, SkTDArray<uint32_t>* buf,
                                   skvm::I32* r, skvm::I32* g, skvm::I32* b, skvm::I32* a) const {
    // TODO: specialize generated code on the 0/1 values of fMatrix?
    if (fDomain == Domain::kRGBA) {
        // Convert to [0,1] floats.
        skvm::F32 R = p->mul(p->to_f32(*r), p->splat(1/255.0f)),
                  G = p->mul(p->to_f32(*g), p->splat(1/255.0f)),
                  B = p->mul(p->to_f32(*b), p->splat(1/255.0f)),
                  A = p->mul(p->to_f32(*a), p->splat(1/255.0f));

        // Unpremul.
        skvm::F32 invA = p->select(p->eq(A, p->splat(0.0f)), p->splat(0.0f)
                                                           , p->div(p->splat(1.0f), A));
        R = p->mul(R, invA);
        G = p->mul(G, invA);
        B = p->mul(B, invA);

        // Apply matrix.
        const size_t offset = buf->bytes();
        memcpy(buf->append(20), &fMatrix, sizeof(fMatrix));
        auto m = [&](int i) { return p->bit_cast(p->uniform32(uniforms, offset + 4*i)); };

        skvm::F32 rgba[4];
        for (int j = 0; j < 4; j++) {
            rgba[j] = p->mad(m(0+j*5), R,
                      p->mad(m(1+j*5), G,
                      p->mad(m(2+j*5), B,
                      p->mad(m(3+j*5), A,
                             m(4+j*5)))));
        }

        // Clamp back to bytes.
        R = p->mad(rgba[0], p->splat(255.0f), p->splat(0.5f));
        G = p->mad(rgba[1], p->splat(255.0f), p->splat(0.5f));
        B = p->mad(rgba[2], p->splat(255.0f), p->splat(0.5f));
        A = p->mad(rgba[3], p->splat(255.0f), p->splat(0.5f));

        *r = p->max(p->splat(0), p->min(p->to_i32(R), p->splat(255)));
        *g = p->max(p->splat(0), p->min(p->to_i32(G), p->splat(255)));
        *b = p->max(p->splat(0), p->min(p->to_i32(B), p->splat(255)));
        *a = p->max(p->splat(0), p->min(p->to_i32(A), p->splat(255)));

        // Premul.
        *r = p->scale_unorm8(*r, *a);
        *g = p->scale_unorm8(*g, *a);
        *b = p->scale_unorm8(*b, *a);
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
