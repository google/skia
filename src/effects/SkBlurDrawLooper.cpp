
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkBlurDrawLooper.h"
#include "SkBlurMaskFilter.h"
#include "SkCanvas.h"
#include "SkPaint.h"
#include "SkMaskFilter.h"
#include "SkColorFilter.h"

SkBlurDrawLooper::SkBlurDrawLooper(SkScalar radius, SkScalar dx, SkScalar dy,
                                   SkColor color, uint32_t flags)
    : fDx(dx), fDy(dy), fBlurColor(color), fBlurFlags(flags), fState(kDone) {

    SkASSERT(flags <= kAll_BlurFlag);
    if (radius > 0) {
        uint32_t blurFlags = flags & kIgnoreTransform_BlurFlag ?
            SkBlurMaskFilter::kIgnoreTransform_BlurFlag :
            SkBlurMaskFilter::kNone_BlurFlag;

        blurFlags |= flags & kHighQuality_BlurFlag ?
            SkBlurMaskFilter::kHighQuality_BlurFlag : 
            SkBlurMaskFilter::kNone_BlurFlag;

        fBlur = SkBlurMaskFilter::Create(radius,
                                         SkBlurMaskFilter::kNormal_BlurStyle,  
                                         blurFlags);
    } else {
        fBlur = NULL;
    }

    if (flags & kOverrideColor_BlurFlag) {
        // Set alpha to 1 for the override since transparency will already
        // be baked into the blurred mask.
        SkColor opaqueColor = SkColorSetA(color, 255);
        //The SrcIn xfer mode will multiply 'color' by the incoming alpha
        fColorFilter = SkColorFilter::CreateModeFilter(opaqueColor,
                                                       SkXfermode::kSrcIn_Mode);
    } else {
        fColorFilter = NULL;
    }
}

SkBlurDrawLooper::SkBlurDrawLooper(SkFlattenableReadBuffer& buffer)
: INHERITED(buffer) {

    fDx = buffer.readScalar();
    fDy = buffer.readScalar();
    fBlurColor = buffer.readU32();
    fBlur = static_cast<SkMaskFilter*>(buffer.readFlattenable());
    fColorFilter = static_cast<SkColorFilter*>(buffer.readFlattenable());
    fBlurFlags = buffer.readU32() & kAll_BlurFlag;
}

SkBlurDrawLooper::~SkBlurDrawLooper() {
    SkSafeUnref(fBlur);
    SkSafeUnref(fColorFilter);
}

void SkBlurDrawLooper::flatten(SkFlattenableWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    buffer.writeScalar(fDx);
    buffer.writeScalar(fDy);
    buffer.write32(fBlurColor);
    buffer.writeFlattenable(fBlur);
    buffer.writeFlattenable(fColorFilter);
    buffer.write32(fBlurFlags);
}

void SkBlurDrawLooper::init(SkCanvas* canvas) {
    fState = kBeforeEdge;
}

bool SkBlurDrawLooper::next(SkCanvas* canvas, SkPaint* paint) {
    switch (fState) {
        case kBeforeEdge:
            // we do nothing if a maskfilter is already installed
            if (paint->getMaskFilter()) {
                fState = kDone;
                return false;
            }
#ifdef SK_BUILD_FOR_ANDROID
            SkColor blurColor;
            blurColor = fBlurColor;
            if (SkColorGetA(blurColor) == 255) {
                blurColor = SkColorSetA(blurColor, paint->getAlpha());
            }
            paint->setColor(blurColor);
#else
            paint->setColor(fBlurColor);
#endif
            paint->setMaskFilter(fBlur);
            paint->setColorFilter(fColorFilter);
            canvas->save(SkCanvas::kMatrix_SaveFlag);
            if (fBlurFlags & kIgnoreTransform_BlurFlag) {
                SkMatrix transform(canvas->getTotalMatrix());
                transform.postTranslate(fDx, fDy);
                canvas->setMatrix(transform);
            } else {
                canvas->translate(fDx, fDy);
            }
            fState = kAfterEdge;
            return true;
        case kAfterEdge:
            canvas->restore();
            fState = kDone;
            return true;
        default:
            SkASSERT(kDone == fState);
            return false;
    }
}

///////////////////////////////////////////////////////////////////////////////

SK_DEFINE_FLATTENABLE_REGISTRAR(SkBlurDrawLooper)

