#include "SkCanvas.h"
#include "SkColor.h"
#include "SkLayerDrawLooper.h"
#include "SkPaint.h"
#include "SkUnPreMultiply.h"

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

void SkLayerDrawLooper::ApplyInfo(SkPaint* dst, const SkPaint& src,
                                  const LayerInfo& info) {

    uint32_t mask = info.fFlagsMask;
    dst->setFlags((dst->getFlags() & ~mask) | (src.getFlags() & mask));

    dst->setColor(xferColor(src.getColor(), dst->getColor(), info.fColorMode));

    BitFlags bits = info.fPaintBits;

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

    // we never copy these
#if 0
    dst->setFlags(src.getFlags());
    dst->setTypeface(src.getTypeface());
    dst->setTextSize(src.getTextSize());
    dst->setTextScaleX(src.getTextScaleX());
    dst->setTextSkewX(src.getTextSkewX());
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

void SkLayerDrawLooper::flatten(SkFlattenableWriteBuffer& buffer) {
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
        buffer.writeInt(rec->fInfo.fPaintBits);
        buffer.writeInt(rec->fInfo.fColorMode);
        buffer.writeScalar(rec->fInfo.fOffset.fX);
        buffer.writeScalar(rec->fInfo.fOffset.fY);
        buffer.writeBool(rec->fInfo.fPostTranslate);
        rec->fPaint.flatten(buffer);
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
        info.fPaintBits = buffer.readInt();
        info.fColorMode = (SkXfermode::Mode)buffer.readInt();
        info.fOffset.fX = buffer.readScalar();
        info.fOffset.fY = buffer.readScalar();
        info.fPostTranslate = buffer.readBool();
        this->addLayer(info)->unflatten(buffer);
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

///////////////////////////////////////////////////////////////////////////////

static SkFlattenable::Registrar gReg("SkLayerDrawLooper",
                                     SkLayerDrawLooper::CreateProc);
