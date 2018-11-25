/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkArenaAlloc.h"
#include "SkBlurDrawLooper.h"
#include "SkBlurMaskFilter.h"
#include "SkCanvas.h"
#include "SkColorSpaceXformer.h"
#include "SkColor.h"
#include "SkMaskFilterBase.h"
#include "SkReadBuffer.h"
#include "SkWriteBuffer.h"
#include "SkLayerDrawLooper.h"
#include "SkString.h"
#include "SkStringUtils.h"
#include "SkUnPreMultiply.h"
#include "SkXfermodePriv.h"

SkLayerDrawLooper::LayerInfo::LayerInfo() {
    fPaintBits = 0;                     // ignore our paint fields
    fColorMode = SkBlendMode::kDst;     // ignore our color
    fOffset.set(0, 0);
    fPostTranslate = false;
}

SkLayerDrawLooper::SkLayerDrawLooper()
        : fRecs(nullptr),
          fCount(0) {
}

SkLayerDrawLooper::~SkLayerDrawLooper() {
    Rec* rec = fRecs;
    while (rec) {
        Rec* next = rec->fNext;
        delete rec;
        rec = next;
    }
}

SkLayerDrawLooper::Context*
SkLayerDrawLooper::makeContext(SkCanvas* canvas, SkArenaAlloc* alloc) const {
    canvas->save();
    return alloc->make<LayerDrawLooperContext>(this);
}

static SkColor xferColor(SkColor src, SkColor dst, SkBlendMode mode) {
    switch (mode) {
        case SkBlendMode::kSrc:
            return src;
        case SkBlendMode::kDst:
            return dst;
        default: {
            SkPMColor pmS = SkPreMultiplyColor(src);
            SkPMColor pmD = SkPreMultiplyColor(dst);
            SkXfermode::Peek(mode)->xfer32(&pmD, &pmS, 1, nullptr);
            return SkUnPreMultiply::PMColorToColor(pmD);
        }
    }
}

// Even with kEntirePaint_Bits, we always ensure that the master paint's
// text-encoding is respected, since that controls how we interpret the
// text/length parameters of a draw[Pos]Text call.
void SkLayerDrawLooper::LayerDrawLooperContext::ApplyInfo(
        SkPaint* dst, const SkPaint& src, const LayerInfo& info) {
    SkColor srcColor = src.getColor();
#ifdef SK_BUILD_FOR_ANDROID_FRAMEWORK
    // The framework may respect the alpha value on the original paint.
    // Match this legacy behavior.
    if (SkColorGetA(srcColor) == 255) {
        srcColor = SkColorSetA(srcColor, dst->getAlpha());
    }
#endif
    dst->setColor(xferColor(srcColor, dst->getColor(), (SkBlendMode)info.fColorMode));

    BitFlags bits = info.fPaintBits;
    SkPaint::TextEncoding encoding = dst->getTextEncoding();

    if (0 == bits) {
        return;
    }
    if (kEntirePaint_Bits == bits) {
        // we've already computed these, so save it from the assignment
        uint32_t f = dst->getFlags();
        SkColor c = dst->getColor();
        *dst = src;
        dst->setFlags(f);
        dst->setColor(c);
        dst->setTextEncoding(encoding);
        return;
    }

    if (bits & kStyle_Bit) {
        dst->setStyle(src.getStyle());
        dst->setStrokeWidth(src.getStrokeWidth());
        dst->setStrokeMiter(src.getStrokeMiter());
        dst->setStrokeCap(src.getStrokeCap());
        dst->setStrokeJoin(src.getStrokeJoin());
    }

    if (bits & kTextSkewX_Bit) {
        dst->setTextSkewX(src.getTextSkewX());
    }

    if (bits & kPathEffect_Bit) {
        dst->setPathEffect(src.refPathEffect());
    }
    if (bits & kMaskFilter_Bit) {
        dst->setMaskFilter(src.refMaskFilter());
    }
    if (bits & kShader_Bit) {
        dst->setShader(src.refShader());
    }
    if (bits & kColorFilter_Bit) {
        dst->setColorFilter(src.refColorFilter());
    }
    if (bits & kXfermode_Bit) {
        dst->setBlendMode(src.getBlendMode());
    }

    // we don't override these
#if 0
    dst->setTypeface(src.getTypeface());
    dst->setTextSize(src.getTextSize());
    dst->setTextScaleX(src.getTextScaleX());
    dst->setRasterizer(src.getRasterizer());
    dst->setLooper(src.getLooper());
    dst->setTextEncoding(src.getTextEncoding());
    dst->setHinting(src.getHinting());
#endif
}

// Should we add this to canvas?
static void postTranslate(SkCanvas* canvas, SkScalar dx, SkScalar dy) {
    SkMatrix m = canvas->getTotalMatrix();
    m.postTranslate(dx, dy);
    canvas->setMatrix(m);
}

SkLayerDrawLooper::LayerDrawLooperContext::LayerDrawLooperContext(
        const SkLayerDrawLooper* looper) : fCurrRec(looper->fRecs) {}

bool SkLayerDrawLooper::LayerDrawLooperContext::next(SkCanvas* canvas,
                                                     SkPaint* paint) {
    canvas->restore();
    if (nullptr == fCurrRec) {
        return false;
    }

    ApplyInfo(paint, fCurrRec->fPaint, fCurrRec->fInfo);

    canvas->save();
    if (fCurrRec->fInfo.fPostTranslate) {
        postTranslate(canvas, fCurrRec->fInfo.fOffset.fX,
                      fCurrRec->fInfo.fOffset.fY);
    } else {
        canvas->translate(fCurrRec->fInfo.fOffset.fX,
                          fCurrRec->fInfo.fOffset.fY);
    }
    fCurrRec = fCurrRec->fNext;

    return true;
}

bool SkLayerDrawLooper::asABlurShadow(BlurShadowRec* bsRec) const {
    if (fCount != 2) {
        return false;
    }
    const Rec* rec = fRecs;

    // bottom layer needs to be just blur(maskfilter)
    if ((rec->fInfo.fPaintBits & ~kMaskFilter_Bit)) {
        return false;
    }
    if (SkBlendMode::kSrc != (SkBlendMode)rec->fInfo.fColorMode) {
        return false;
    }
    const SkMaskFilter* mf = rec->fPaint.getMaskFilter();
    if (nullptr == mf) {
        return false;
    }
    SkMaskFilterBase::BlurRec maskBlur;
    if (!as_MFB(mf)->asABlur(&maskBlur)) {
        return false;
    }

    rec = rec->fNext;
    // top layer needs to be "plain"
    if (rec->fInfo.fPaintBits) {
        return false;
    }
    if (SkBlendMode::kDst != (SkBlendMode)rec->fInfo.fColorMode) {
        return false;
    }
    if (!rec->fInfo.fOffset.equals(0, 0)) {
        return false;
    }

    if (bsRec) {
        bsRec->fSigma = maskBlur.fSigma;
        bsRec->fOffset = fRecs->fInfo.fOffset;
        bsRec->fColor = fRecs->fPaint.getColor();
        bsRec->fStyle = maskBlur.fStyle;
        bsRec->fQuality = maskBlur.fQuality;
    }
    return true;
}

sk_sp<SkDrawLooper> SkLayerDrawLooper::onMakeColorSpace(SkColorSpaceXformer* xformer) const {
    if (!fCount) {
        return sk_ref_sp(const_cast<SkLayerDrawLooper*>(this));
    }

    auto looper = sk_sp<SkLayerDrawLooper>(new SkLayerDrawLooper());
    looper->fCount = fCount;

    Rec* oldRec = fRecs;
    Rec* newTopRec = new Rec();
    newTopRec->fInfo = oldRec->fInfo;
    newTopRec->fPaint = xformer->apply(oldRec->fPaint);
    newTopRec->fNext = nullptr;

    Rec* prevNewRec = newTopRec;
    oldRec = oldRec->fNext;
    while (oldRec) {
        Rec* newRec = new Rec();
        newRec->fInfo = oldRec->fInfo;
        newRec->fPaint = xformer->apply(oldRec->fPaint);
        newRec->fNext = nullptr;
        prevNewRec->fNext = newRec;

        prevNewRec = newRec;
        oldRec = oldRec->fNext;
    }

    looper->fRecs = newTopRec;
    return std::move(looper);
}

///////////////////////////////////////////////////////////////////////////////

void SkLayerDrawLooper::flatten(SkWriteBuffer& buffer) const {
    buffer.writeInt(fCount);

    Rec* rec = fRecs;
    for (int i = 0; i < fCount; i++) {
        // Legacy "flagsmask" field -- now ignored, remove when we bump version
        buffer.writeInt(0);

        buffer.writeInt(rec->fInfo.fPaintBits);
        buffer.writeInt((int)rec->fInfo.fColorMode);
        buffer.writePoint(rec->fInfo.fOffset);
        buffer.writeBool(rec->fInfo.fPostTranslate);
        buffer.writePaint(rec->fPaint);
        rec = rec->fNext;
    }
}

sk_sp<SkFlattenable> SkLayerDrawLooper::CreateProc(SkReadBuffer& buffer) {
    int count = buffer.readInt();

    Builder builder;
    for (int i = 0; i < count; i++) {
        LayerInfo info;
        // Legacy "flagsmask" field -- now ignored, remove when we bump version
        (void)buffer.readInt();

        info.fPaintBits = buffer.readInt();
        info.fColorMode = (SkBlendMode)buffer.readInt();
        buffer.readPoint(&info.fOffset);
        info.fPostTranslate = buffer.readBool();
        buffer.readPaint(builder.addLayerOnTop(info));
    }
    return builder.detach();
}

#ifndef SK_IGNORE_TO_STRING
void SkLayerDrawLooper::toString(SkString* str) const {
    str->appendf("SkLayerDrawLooper (%d): ", fCount);

    Rec* rec = fRecs;
    for (int i = 0; i < fCount; i++) {
        str->appendf("%d: paintBits: (", i);
        if (0 == rec->fInfo.fPaintBits) {
            str->append("None");
        } else if (kEntirePaint_Bits == rec->fInfo.fPaintBits) {
            str->append("EntirePaint");
        } else {
            bool needSeparator = false;
            SkAddFlagToString(str, SkToBool(kStyle_Bit & rec->fInfo.fPaintBits), "Style",
                              &needSeparator);
            SkAddFlagToString(str, SkToBool(kTextSkewX_Bit & rec->fInfo.fPaintBits), "TextSkewX",
                              &needSeparator);
            SkAddFlagToString(str, SkToBool(kPathEffect_Bit & rec->fInfo.fPaintBits), "PathEffect",
                              &needSeparator);
            SkAddFlagToString(str, SkToBool(kMaskFilter_Bit & rec->fInfo.fPaintBits), "MaskFilter",
                              &needSeparator);
            SkAddFlagToString(str, SkToBool(kShader_Bit & rec->fInfo.fPaintBits), "Shader",
                              &needSeparator);
            SkAddFlagToString(str, SkToBool(kColorFilter_Bit & rec->fInfo.fPaintBits), "ColorFilter",
                              &needSeparator);
            SkAddFlagToString(str, SkToBool(kXfermode_Bit & rec->fInfo.fPaintBits), "Xfermode",
                              &needSeparator);
        }
        str->append(") ");

        static const char* gModeStrings[(int)SkBlendMode::kLastMode+1] = {
            "kClear", "kSrc", "kDst", "kSrcOver", "kDstOver", "kSrcIn", "kDstIn",
            "kSrcOut", "kDstOut", "kSrcATop", "kDstATop", "kXor", "kPlus",
            "kMultiply", "kScreen", "kOverlay", "kDarken", "kLighten", "kColorDodge",
            "kColorBurn", "kHardLight", "kSoftLight", "kDifference", "kExclusion"
        };

        str->appendf("mode: %s ", gModeStrings[(int)rec->fInfo.fColorMode]);

        str->append("offset: (");
        str->appendScalar(rec->fInfo.fOffset.fX);
        str->append(", ");
        str->appendScalar(rec->fInfo.fOffset.fY);
        str->append(") ");

        str->append("postTranslate: ");
        if (rec->fInfo.fPostTranslate) {
            str->append("true ");
        } else {
            str->append("false ");
        }

        rec->fPaint.toString(str);
        rec = rec->fNext;
    }
}
#endif

SkLayerDrawLooper::Builder::Builder()
        : fRecs(nullptr),
          fTopRec(nullptr),
          fCount(0) {
}

SkLayerDrawLooper::Builder::~Builder() {
    Rec* rec = fRecs;
    while (rec) {
        Rec* next = rec->fNext;
        delete rec;
        rec = next;
    }
}

SkPaint* SkLayerDrawLooper::Builder::addLayer(const LayerInfo& info) {
    fCount += 1;

    Rec* rec = new Rec;
    rec->fNext = fRecs;
    rec->fInfo = info;
    fRecs = rec;
    if (nullptr == fTopRec) {
        fTopRec = rec;
    }

    return &rec->fPaint;
}

void SkLayerDrawLooper::Builder::addLayer(SkScalar dx, SkScalar dy) {
    LayerInfo info;

    info.fOffset.set(dx, dy);
    (void)this->addLayer(info);
}

SkPaint* SkLayerDrawLooper::Builder::addLayerOnTop(const LayerInfo& info) {
    fCount += 1;

    Rec* rec = new Rec;
    rec->fNext = nullptr;
    rec->fInfo = info;
    if (nullptr == fRecs) {
        fRecs = rec;
    } else {
        SkASSERT(fTopRec);
        fTopRec->fNext = rec;
    }
    fTopRec = rec;

    return &rec->fPaint;
}

sk_sp<SkDrawLooper> SkLayerDrawLooper::Builder::detach() {
    SkLayerDrawLooper* looper = new SkLayerDrawLooper;
    looper->fCount = fCount;
    looper->fRecs = fRecs;

    fCount = 0;
    fRecs = nullptr;
    fTopRec = nullptr;

    return sk_sp<SkDrawLooper>(looper);
}

sk_sp<SkDrawLooper> SkBlurDrawLooper::Make(SkColor color, SkScalar sigma, SkScalar dx, SkScalar dy)
{
    sk_sp<SkMaskFilter> blur = nullptr;
    if (sigma > 0.0f) {
        blur = SkBlurMaskFilter::Make(kNormal_SkBlurStyle, sigma, SkBlurMaskFilter::kNone_BlurFlag);
    }

    SkLayerDrawLooper::Builder builder;

    // First layer
    SkLayerDrawLooper::LayerInfo defaultLayer;
    builder.addLayer(defaultLayer);

    // Blur layer
    SkLayerDrawLooper::LayerInfo blurInfo;
    blurInfo.fColorMode = SkBlendMode::kSrc;
    blurInfo.fPaintBits = SkLayerDrawLooper::kMaskFilter_Bit;
    blurInfo.fOffset = SkVector::Make(dx, dy);
    SkPaint* paint = builder.addLayer(blurInfo);
    paint->setMaskFilter(std::move(blur));
    paint->setColor(color);

    return builder.detach();
}
