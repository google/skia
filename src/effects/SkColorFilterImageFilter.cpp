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
#include "SkTableColorFilter.h"
#include "SkWriteBuffer.h"

namespace {

void mult_color_matrix(SkScalar a[20], SkScalar b[20], SkScalar out[20]) {
    for (int j = 0; j < 4; ++j) {
        for (int i = 0; i < 5; ++i) {
            out[i+j*5] = 4 == i ? a[4+j*5] : 0;
            for (int k = 0; k < 4; ++k)
                out[i+j*5] += a[k+j*5] * b[i+k*5];
        }
    }
}

// Combines the two lookup tables so that making a lookup using OUT has
// the same effect as making a lookup through B then A.
void combine_color_tables(const uint8_t a[4 * 256],
                          const uint8_t b[4 * 256],
                          uint8_t out[4 * 256]) {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 256; j++) {
            out[i * 256 + j] = a[i * 256 + b[i * 256 + j]];
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
        SkImageFilter* input, const CropRect* cropRect, uint32_t uniqueID) {
    if (NULL == cf) {
        return NULL;
    }

    SkColorFilter* inputColorFilter;
    if (input && input->asColorFilter(&inputColorFilter) && inputColorFilter) {
        SkAutoUnref autoUnref(inputColorFilter);

        // Try to collapse two consecutive matrix filters
        SkScalar colorMatrix[20], inputMatrix[20];
        if (cf->asColorMatrix(colorMatrix) && inputColorFilter->asColorMatrix(inputMatrix)
                                           && !matrix_needs_clamping(inputMatrix)) {
            SkScalar combinedMatrix[20];
            mult_color_matrix(colorMatrix, inputMatrix, combinedMatrix);
            SkAutoTUnref<SkColorFilter> newCF(SkColorMatrixFilter::Create(combinedMatrix));
            return SkNEW_ARGS(SkColorFilterImageFilter, (newCF, input->getInput(0), cropRect, 0));
        }

        // Try to collapse two consecutive table filters
        SkBitmap colorTable, inputTable;
        if (cf->asComponentTable(&colorTable) && inputColorFilter->asComponentTable(&inputTable)) {
            uint8_t combinedTable[4 * 256];
            SkAutoLockPixels colorLock(colorTable);
            SkAutoLockPixels inputLock(inputTable);

            combine_color_tables(colorTable.getAddr8(0, 0), inputTable.getAddr8(0, 0),
                                 combinedTable);
            SkAutoTUnref<SkColorFilter> newCF(SkTableColorFilter::CreateARGB(
                        &combinedTable[256 * 0],
                        &combinedTable[256 * 1],
                        &combinedTable[256 * 2],
                        &combinedTable[256 * 3])
            );

            return SkNEW_ARGS(SkColorFilterImageFilter, (newCF, input->getInput(0), cropRect, 0));
        }
    }

    return SkNEW_ARGS(SkColorFilterImageFilter, (cf, input, cropRect, uniqueID));
}

SkColorFilterImageFilter::SkColorFilterImageFilter(SkColorFilter* cf,
        SkImageFilter* input, const CropRect* cropRect, uint32_t uniqueID)
    : INHERITED(1, &input, cropRect, uniqueID), fColorFilter(SkRef(cf)) {
}

SkFlattenable* SkColorFilterImageFilter::CreateProc(SkReadBuffer& buffer) {
    SK_IMAGEFILTER_UNFLATTEN_COMMON(common, 1);
    SkAutoTUnref<SkColorFilter> cf(buffer.readColorFilter());
    return Create(cf, common.getInput(0), &common.cropRect(), common.uniqueID());
}

void SkColorFilterImageFilter::flatten(SkWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    buffer.writeFlattenable(fColorFilter);
}

SkColorFilterImageFilter::~SkColorFilterImageFilter() {
    fColorFilter->unref();
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

#ifndef SK_IGNORE_TO_STRING
void SkColorFilterImageFilter::toString(SkString* str) const {
    str->appendf("SkColorFilterImageFilter: (");

    str->appendf("input: (");

    if (this->getInput(0)) {
        this->getInput(0)->toString(str);
    }

    str->appendf(") color filter: ");
    fColorFilter->toString(str);

    str->append(")");
}
#endif
