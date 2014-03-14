/*
 * Copyright 2012 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkColorFilterImageFilter.h"
#include "SkBitmap.h"
#include "SkCanvas.h"
#include "SkColorMatrixFilter.h"
#include "SkDevice.h"
#include "SkColorFilter.h"
#include "SkReadBuffer.h"
#include "SkWriteBuffer.h"

namespace {

void mult_color_matrix(SkScalar a[20], SkScalar b[20], SkScalar out[20]) {
    for (int j = 0; j < 4; ++j) {
        for (int i = 0; i < 5; ++i) {
            out[i+j*5] = 4 == i ? a[4+j*5] : 0;
            for (int k = 0; k < 4; ++k)
                out[i+j*5] += SkScalarMul(a[k+j*5], b[i+k*5]);
        }
    }
}

// To detect if we need to apply clamping after applying a matrix, we check if
// any output component might go outside of [0, 255] for any combination of
// input components in [0..255].
// Each output component is an affine transformation of the input component, so
// the minimum and maximum values are for any combination of minimum or maximum
// values of input components (i.e. 0 or 255).
// E.g. if R' = x*R + y*G + z*B + w*A + t
// Then the maximum value will be for R=255 if x>0 or R=0 if x<0, and the
// minimum value will be for R=0 if x>0 or R=255 if x<0.
// Same goes for all components.
bool component_needs_clamping(SkScalar row[5]) {
    SkScalar maxValue = row[4] / 255;
    SkScalar minValue = row[4] / 255;
    for (int i = 0; i < 4; ++i) {
        if (row[i] > 0)
            maxValue += row[i];
        else
            minValue += row[i];
    }
    return (maxValue > 1) || (minValue < 0);
}

bool matrix_needs_clamping(SkScalar matrix[20]) {
    return component_needs_clamping(matrix)
        || component_needs_clamping(matrix+5)
        || component_needs_clamping(matrix+10)
        || component_needs_clamping(matrix+15);
}

};

SkColorFilterImageFilter* SkColorFilterImageFilter::Create(SkColorFilter* cf,
        SkImageFilter* input, const CropRect* cropRect) {
    SkASSERT(cf);
    SkScalar colorMatrix[20], inputMatrix[20];
    SkColorFilter* inputColorFilter;
    if (input && cf->asColorMatrix(colorMatrix)
              && input->asColorFilter(&inputColorFilter)
              && (NULL != inputColorFilter)) {
        SkAutoUnref autoUnref(inputColorFilter);
        if (inputColorFilter->asColorMatrix(inputMatrix) && !matrix_needs_clamping(inputMatrix)) {
            SkScalar combinedMatrix[20];
            mult_color_matrix(inputMatrix, colorMatrix, combinedMatrix);
            SkAutoTUnref<SkColorFilter> newCF(SkColorMatrixFilter::Create(combinedMatrix));
            return SkNEW_ARGS(SkColorFilterImageFilter, (newCF, input->getInput(0), cropRect));
        }
    }
    return SkNEW_ARGS(SkColorFilterImageFilter, (cf, input, cropRect));
}

SkColorFilterImageFilter::SkColorFilterImageFilter(SkColorFilter* cf,
        SkImageFilter* input, const CropRect* cropRect)
    : INHERITED(input, cropRect), fColorFilter(cf) {
    SkASSERT(cf);
    SkSafeRef(cf);
}

SkColorFilterImageFilter::SkColorFilterImageFilter(SkReadBuffer& buffer)
  : INHERITED(1, buffer) {
    fColorFilter = buffer.readColorFilter();
}

void SkColorFilterImageFilter::flatten(SkWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);

    buffer.writeFlattenable(fColorFilter);
}

SkColorFilterImageFilter::~SkColorFilterImageFilter() {
    SkSafeUnref(fColorFilter);
}

bool SkColorFilterImageFilter::onFilterImage(Proxy* proxy, const SkBitmap& source,
                                             const Context& ctx,
                                             SkBitmap* result,
                                             SkIPoint* offset) const {
    SkBitmap src = source;
    SkIPoint srcOffset = SkIPoint::Make(0, 0);
    if (getInput(0) && !getInput(0)->filterImage(proxy, source, ctx, &src, &srcOffset)) {
        return false;
    }

    SkIRect bounds;
    if (!this->applyCropRect(ctx, src, srcOffset, &bounds)) {
        return false;
    }

    SkAutoTUnref<SkBaseDevice> device(proxy->createDevice(bounds.width(), bounds.height()));
    if (NULL == device.get()) {
        return false;
    }
    SkCanvas canvas(device.get());
    SkPaint paint;

    paint.setXfermodeMode(SkXfermode::kSrc_Mode);
    paint.setColorFilter(fColorFilter);
    canvas.drawSprite(src, srcOffset.fX - bounds.fLeft, srcOffset.fY - bounds.fTop, &paint);

    *result = device.get()->accessBitmap(false);
    offset->fX = bounds.fLeft;
    offset->fY = bounds.fTop;
    return true;
}

bool SkColorFilterImageFilter::asColorFilter(SkColorFilter** filter) const {
    if (!cropRectIsSet()) {
        if (filter) {
            *filter = fColorFilter;
            fColorFilter->ref();
        }
        return true;
    }
    return false;
}
