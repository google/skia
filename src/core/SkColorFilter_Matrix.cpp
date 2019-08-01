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

bool SkColorFilter_Matrix::onAppendStages(const SkStageRec& rec,
                                          bool /*shaderIsOpaque*/) const {
    const bool hsla = fDomain == Domain::kHSLA;

    SkRasterPipeline* p = rec.fPipeline;
    if (hsla) { p->append(SkRasterPipeline::rgb_to_hsl); }
                p->append(SkRasterPipeline::matrix_4x5, fMatrix);
    if (hsla) { p->append(SkRasterPipeline::hsl_to_rgb); }
                p->append(SkRasterPipeline::clamp_0);
                p->append(SkRasterPipeline::clamp_1);
    return true;
}

#if SK_SUPPORT_GPU
#include "src/gpu/effects/generated/GrColorMatrixFragmentProcessor.h"
std::unique_ptr<GrFragmentProcessor> SkColorFilter_Matrix::asFragmentProcessor(
        GrRecordingContext*, const GrColorSpaceInfo&) const {
    if (fDomain == Domain::kHSLA) {
        // TODO
        return nullptr;
    }
    return GrColorMatrixFragmentProcessor::Make(fMatrix,
                                                /* premulInput = */ true,
                                                /* clampRGBOutput = */ true,
                                                /* premulOutput = */ true);
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
    return MakeMatrix(cm.fMat, SkColorFilter_Matrix::Domain::kRGBA);
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
