
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkCanvas.h"
#include "SkColor.h"
#include "SkFlattenableBuffers.h"
#include "SkLayerDrawLooper.h"
#include "SkPaint.h"
#include "SkString.h"
#include "SkStringUtils.h"
#include "SkUnPreMultiply.h"

SK_DEFINE_INST_COUNT(SkLayerDrawLooper)

SkLayerDrawLooper::LayerInfo::LayerInfo() {
    fFlagsMask = 0;                     // ignore our paint flags
    fPaintBits = 0;                     // ignore our paint fields
    fColorMode = SkXfermode::kDst_Mode; // ignore our color
    fOffset.set(0, 0);
    fPostTranslate = false;
}

SkLayerDrawLooper::SkLayerDrawLooper()
        : fRecs(NULL),
          fCount(0),
          fCurrRec(NULL) {
}

SkLayerDrawLooper::~SkLayerDrawLooper() {
    Rec* rec = fRecs;
    while (rec) {
        Rec* next = rec->fNext;
        SkDELETE(rec);
        rec = next;
    }
}

SkPaint* SkLayerDrawLooper::addLayer(const LayerInfo& info) {
    fCount += 1;

    Rec* rec = SkNEW(Rec);
    rec->fNext = fRecs;
    rec->fInfo = info;
    fRecs = rec;

    return &rec->fPaint;
}

void SkLayerDrawLooper::addLayer(SkScalar dx, SkScalar dy) {
    LayerInfo info;

    info.fOffset.set(dx, dy);
    (void)this->addLayer(info);
}

void SkLayerDrawLooper::init(SkCanvas* canvas) {
    fCurrRec = fRecs;
    canvas->save(SkCanvas::kMatrix_SaveFlag);
}

static SkColor xferColor(SkColor src, SkColor dst, SkXfermode::Mode mode) {
    switch (mode) {
        case SkXfermode::kSrc_Mode:
            return src;
        case SkXfermode::kDst_Mode:
            return dst;
        default: {
            SkPMColor pmS = SkPreMultiplyColor(src);
            SkPMColor pmD = SkPreMultiplyColor(dst);
            SkPMColor result = SkXfermode::GetProc(mode)(pmS, pmD);
            return SkUnPreMultiply::PMColorToColor(result);
        }
    }
}

// Even with kEntirePaint_Bits, we always ensure that the master paint's
// text-encoding is respected, since that controls how we interpret the
// text/length parameters of a draw[Pos]Text call.
void SkLayerDrawLooper::ApplyInfo(SkPaint* dst, const SkPaint& src,
                                  const LayerInfo& info) {

    uint32_t mask = info.fFlagsMask;
    dst->setFlags((dst->getFlags() & ~mask) | (src.getFlags() & mask));
    dst->setColor(xferColor(src.getColor(), dst->getColor(), info.fColorMode));

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
        dst->setPathEffect(src.getPathEffect());
    }
    if (bits & kMaskFilter_Bit) {
        dst->setMaskFilter(src.getMaskFilter());
    }
    if (bits & kShader_Bit) {
        dst->setShader(src.getShader());
    }
    if (bits & kColorFilter_Bit) {
        dst->setColorFilter(src.getColorFilter());
    }
    if (bits & kXfermode_Bit) {
        dst->setXfermode(src.getXfermode());
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

bool SkLayerDrawLooper::next(SkCanvas* canvas, SkPaint* paint) {
    canvas->restore();
    if (NULL == fCurrRec) {
        return false;
    }

    ApplyInfo(paint, fCurrRec->fPaint, fCurrRec->fInfo);

    canvas->save(SkCanvas::kMatrix_SaveFlag);
    if (fCurrRec->fInfo.fPostTranslate) {
        postTranslate(canvas, fCurrRec->fInfo.fOffset.fX,
                      fCurrRec->fInfo.fOffset.fY);
    } else {
        canvas->translate(fCurrRec->fInfo.fOffset.fX, fCurrRec->fInfo.fOffset.fY);
    }
    fCurrRec = fCurrRec->fNext;

    return true;
}

SkLayerDrawLooper::Rec* SkLayerDrawLooper::Rec::Reverse(Rec* head) {
    Rec* rec = head;
    Rec* prev = NULL;
    while (rec) {
        Rec* next = rec->fNext;
        rec->fNext = prev;
        prev = rec;
        rec = next;
    }
    return prev;
}

///////////////////////////////////////////////////////////////////////////////

void SkLayerDrawLooper::flatten(SkFlattenableWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);

#ifdef SK_DEBUG
    {
        Rec* rec = fRecs;
        int count = 0;
        while (rec) {
            rec = rec->fNext;
            count += 1;
        }
        SkASSERT(count == fCount);
    }
#endif

    buffer.writeInt(fCount);

    Rec* rec = fRecs;
    for (int i = 0; i < fCount; i++) {
        buffer.writeInt(rec->fInfo.fFlagsMask);
        buffer.writeInt(rec->fInfo.fPaintBits);
        buffer.writeInt(rec->fInfo.fColorMode);
        buffer.writePoint(rec->fInfo.fOffset);
        buffer.writeBool(rec->fInfo.fPostTranslate);
        buffer.writePaint(rec->fPaint);
        rec = rec->fNext;
    }
}

SkLayerDrawLooper::SkLayerDrawLooper(SkFlattenableReadBuffer& buffer)
        : INHERITED(buffer),
          fRecs(NULL),
          fCount(0),
          fCurrRec(NULL) {
    int count = buffer.readInt();

    for (int i = 0; i < count; i++) {
        LayerInfo info;
        info.fFlagsMask = buffer.readInt();
        info.fPaintBits = buffer.readInt();
        info.fColorMode = (SkXfermode::Mode)buffer.readInt();
        buffer.readPoint(&info.fOffset);
        info.fPostTranslate = buffer.readBool();
        buffer.readPaint(this->addLayer(info));
    }
    SkASSERT(count == fCount);

    // we're in reverse order, so fix it now
    fRecs = Rec::Reverse(fRecs);

#ifdef SK_DEBUG
    {
        Rec* rec = fRecs;
        int n = 0;
        while (rec) {
            rec = rec->fNext;
            n += 1;
        }
        SkASSERT(count == n);
    }
#endif
}

#ifdef SK_DEVELOPER
void SkLayerDrawLooper::toString(SkString* str) const {
    str->appendf("SkLayerDrawLooper (%d): ", fCount);

    Rec* rec = fRecs;
    for (int i = 0; i < fCount; i++) {
        str->appendf("%d: ", i);

        str->append("flagsMask: (");
        if (0 == rec->fInfo.fFlagsMask) {
            str->append("None");
        } else {
            bool needSeparator = false;
            SkAddFlagToString(str, SkToBool(SkPaint::kAntiAlias_Flag & rec->fInfo.fFlagsMask),
                              "AntiAlias", &needSeparator);
            SkAddFlagToString(str, SkToBool(SkPaint::kFilterBitmap_Flag & rec->fInfo.fFlagsMask),
                              "FilterBitmap", &needSeparator);
            SkAddFlagToString(str, SkToBool(SkPaint::kDither_Flag & rec->fInfo.fFlagsMask),
                              "Dither", &needSeparator);
            SkAddFlagToString(str, SkToBool(SkPaint::kUnderlineText_Flag & rec->fInfo.fFlagsMask),
                              "UnderlineText", &needSeparator);
            SkAddFlagToString(str, SkToBool(SkPaint::kStrikeThruText_Flag & rec->fInfo.fFlagsMask),
                              "StrikeThruText", &needSeparator);
            SkAddFlagToString(str, SkToBool(SkPaint::kFakeBoldText_Flag & rec->fInfo.fFlagsMask),
                              "FakeBoldText", &needSeparator);
            SkAddFlagToString(str, SkToBool(SkPaint::kLinearText_Flag & rec->fInfo.fFlagsMask),
                              "LinearText", &needSeparator);
            SkAddFlagToString(str, SkToBool(SkPaint::kSubpixelText_Flag & rec->fInfo.fFlagsMask),
                              "SubpixelText", &needSeparator);
            SkAddFlagToString(str, SkToBool(SkPaint::kDevKernText_Flag & rec->fInfo.fFlagsMask),
                              "DevKernText", &needSeparator);
            SkAddFlagToString(str, SkToBool(SkPaint::kLCDRenderText_Flag & rec->fInfo.fFlagsMask),
                              "LCDRenderText", &needSeparator);
            SkAddFlagToString(str, SkToBool(SkPaint::kEmbeddedBitmapText_Flag & rec->fInfo.fFlagsMask),
                              "EmbeddedBitmapText", &needSeparator);
            SkAddFlagToString(str, SkToBool(SkPaint::kAutoHinting_Flag & rec->fInfo.fFlagsMask),
                              "Autohinted", &needSeparator);
            SkAddFlagToString(str, SkToBool(SkPaint::kVerticalText_Flag & rec->fInfo.fFlagsMask),
                              "VerticalText", &needSeparator);
            SkAddFlagToString(str, SkToBool(SkPaint::kGenA8FromLCD_Flag & rec->fInfo.fFlagsMask),
                              "GenA8FromLCD", &needSeparator);
        }
        str->append(") ");

        str->append("paintBits: (");
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

        static const char* gModeStrings[SkXfermode::kLastMode+1] = {
            "kClear", "kSrc", "kDst", "kSrcOver", "kDstOver", "kSrcIn", "kDstIn",
            "kSrcOut", "kDstOut", "kSrcATop", "kDstATop", "kXor", "kPlus",
            "kMultiply", "kScreen", "kOverlay", "kDarken", "kLighten", "kColorDodge",
            "kColorBurn", "kHardLight", "kSoftLight", "kDifference", "kExclusion"
        };

        str->appendf("mode: %s ", gModeStrings[rec->fInfo.fColorMode]);

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
