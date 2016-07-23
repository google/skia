/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBlurDrawLooper.h"
#include "SkBlurMask.h"     // just for SkBlurMask::ConvertRadiusToSigma
#include "SkBlurMaskFilter.h"
#include "SkCanvas.h"
#include "SkColorFilter.h"
#include "SkReadBuffer.h"
#include "SkWriteBuffer.h"
#include "SkMaskFilter.h"
#include "SkPaint.h"
#include "SkString.h"
#include "SkStringUtils.h"

SkBlurDrawLooper::SkBlurDrawLooper(SkColor color, SkScalar sigma,
                                   SkScalar dx, SkScalar dy, uint32_t flags) {
    this->init(sigma, dx, dy, color, flags);
}

// only call from constructor
void SkBlurDrawLooper::initEffects() {
    SkASSERT(fBlurFlags <= kAll_BlurFlag);
    if (fSigma > 0) {
        uint32_t flags = fBlurFlags & kIgnoreTransform_BlurFlag ?
                            SkBlurMaskFilter::kIgnoreTransform_BlurFlag :
                            SkBlurMaskFilter::kNone_BlurFlag;

        flags |= fBlurFlags & kHighQuality_BlurFlag ?
                    SkBlurMaskFilter::kHighQuality_BlurFlag :
                    SkBlurMaskFilter::kNone_BlurFlag;

        fBlur = SkBlurMaskFilter::Make(kNormal_SkBlurStyle, fSigma, flags);
    } else {
        fBlur = nullptr;
    }

    if (fBlurFlags & kOverrideColor_BlurFlag) {
        // Set alpha to 1 for the override since transparency will already
        // be baked into the blurred mask.
        SkColor opaqueColor = SkColorSetA(fBlurColor, 255);
        //The SrcIn xfer mode will multiply 'color' by the incoming alpha
        fColorFilter = SkColorFilter::MakeModeFilter(opaqueColor, SkXfermode::kSrcIn_Mode);
    } else {
        fColorFilter = nullptr;
    }
}

void SkBlurDrawLooper::init(SkScalar sigma, SkScalar dx, SkScalar dy,
                            SkColor color, uint32_t flags) {
    fSigma = sigma;
    fDx = dx;
    fDy = dy;
    fBlurColor = color;
    fBlurFlags = flags;

    this->initEffects();
}

sk_sp<SkFlattenable> SkBlurDrawLooper::CreateProc(SkReadBuffer& buffer) {
    const SkColor color = buffer.readColor();
    const SkScalar sigma = buffer.readScalar();
    const SkScalar dx = buffer.readScalar();
    const SkScalar dy = buffer.readScalar();
    const uint32_t flags = buffer.read32();
    return Make(color, sigma, dx, dy, flags);
}

void SkBlurDrawLooper::flatten(SkWriteBuffer& buffer) const {
    buffer.writeColor(fBlurColor);
    buffer.writeScalar(fSigma);
    buffer.writeScalar(fDx);
    buffer.writeScalar(fDy);
    buffer.write32(fBlurFlags);
}

bool SkBlurDrawLooper::asABlurShadow(BlurShadowRec* rec) const {
    if (fSigma <= 0 || (fBlurFlags & fBlurFlags & kIgnoreTransform_BlurFlag)) {
        return false;
    }

    if (rec) {
        rec->fSigma = fSigma;
        rec->fColor = fBlurColor;
        rec->fOffset.set(fDx, fDy);
        rec->fStyle = kNormal_SkBlurStyle;
        rec->fQuality = (fBlurFlags & kHighQuality_BlurFlag) ?
                        kHigh_SkBlurQuality : kLow_SkBlurQuality;
    }
    return true;
}

////////////////////////////////////////////////////////////////////////////////////////

SkDrawLooper::Context* SkBlurDrawLooper::createContext(SkCanvas*, void* storage) const {
    return new (storage) BlurDrawLooperContext(this);
}

SkBlurDrawLooper::BlurDrawLooperContext::BlurDrawLooperContext(
        const SkBlurDrawLooper* looper)
    : fLooper(looper), fState(SkBlurDrawLooper::kBeforeEdge) {}

bool SkBlurDrawLooper::BlurDrawLooperContext::next(SkCanvas* canvas,
                                                   SkPaint* paint) {
    switch (fState) {
        case kBeforeEdge:
            // we do nothing if a maskfilter is already installed
            if (paint->getMaskFilter()) {
                fState = kDone;
                return false;
            }
#ifdef SK_BUILD_FOR_ANDROID
            SkColor blurColor;
            blurColor = fLooper->fBlurColor;
            if (SkColorGetA(blurColor) == 255) {
                blurColor = SkColorSetA(blurColor, paint->getAlpha());
            }
            paint->setColor(blurColor);
#else
            paint->setColor(fLooper->fBlurColor);
#endif
            paint->setMaskFilter(fLooper->fBlur);
            paint->setColorFilter(fLooper->fColorFilter);
            canvas->save();
            if (fLooper->fBlurFlags & kIgnoreTransform_BlurFlag) {
                SkMatrix transform(canvas->getTotalMatrix());
                transform.postTranslate(fLooper->fDx, fLooper->fDy);
                canvas->setMatrix(transform);
            } else {
                canvas->translate(fLooper->fDx, fLooper->fDy);
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

#ifndef SK_IGNORE_TO_STRING
void SkBlurDrawLooper::toString(SkString* str) const {
    str->append("SkBlurDrawLooper: ");

    str->append("dx: ");
    str->appendScalar(fDx);

    str->append(" dy: ");
    str->appendScalar(fDy);

    str->append(" color: ");
    str->appendHex(fBlurColor);

    str->append(" flags: (");
    if (kNone_BlurFlag == fBlurFlags) {
        str->append("None");
    } else {
        bool needsSeparator = false;
        SkAddFlagToString(str, SkToBool(kIgnoreTransform_BlurFlag & fBlurFlags), "IgnoreTransform",
                          &needsSeparator);
        SkAddFlagToString(str, SkToBool(kOverrideColor_BlurFlag & fBlurFlags), "OverrideColor",
                          &needsSeparator);
        SkAddFlagToString(str, SkToBool(kHighQuality_BlurFlag & fBlurFlags), "HighQuality",
                          &needsSeparator);
    }
    str->append(")");

    // TODO: add optional "fBlurFilter->toString(str);" when SkMaskFilter::toString is added
    // alternatively we could cache the radius in SkBlurDrawLooper and just add it here
}
#endif
