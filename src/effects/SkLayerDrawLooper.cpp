#include "SkCanvas.h"
#include "SkLayerDrawLooper.h"
#include "SkPaint.h"

SkLayerDrawLooper::SkLayerDrawLooper() {
    fRecs = NULL;
    fCount = 0;
}

SkLayerDrawLooper::~SkLayerDrawLooper() {
    Rec* rec = fRecs;
    while (rec) {
        Rec* next = rec->fNext;
        SkDELETE(rec);
        rec = next;
    }
}
    
SkPaint* SkLayerDrawLooper::addLayer(SkScalar dx, SkScalar dy) {
    fCount += 1;

    Rec* rec = SkNEW(Rec);
    rec->fNext = fRecs;
    rec->fOffset.set(dx, dy);
    fRecs = rec;

    return &rec->fPaint;
}

void SkLayerDrawLooper::init(SkCanvas* canvas, SkPaint* paint) {
    fIter.fSavedPaint = *paint;
    fIter.fPaint = paint;
    fIter.fCanvas = canvas;
    fIter.fRec = fRecs;
    canvas->save(SkCanvas::kMatrix_SaveFlag);
}

bool SkLayerDrawLooper::next() {
    Rec* rec = fIter.fRec;
    if (rec) {
        *fIter.fPaint = rec->fPaint;
        fIter.fCanvas->restore();
        fIter.fCanvas->save(SkCanvas::kMatrix_SaveFlag);
        fIter.fCanvas->translate(rec->fOffset.fX, rec->fOffset.fY);

        fIter.fRec = rec->fNext;
        return true;
    }
    return false;
}

void SkLayerDrawLooper::restore() {
    fIter.fCanvas->restore();
    *fIter.fPaint = fIter.fSavedPaint;
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
        buffer.writeScalar(rec->fOffset.fX);
        buffer.writeScalar(rec->fOffset.fY);
        rec->fPaint.flatten(buffer);
        rec = rec->fNext;
    }
}

SkLayerDrawLooper::SkLayerDrawLooper(SkFlattenableReadBuffer& buffer)
        : INHERITED(buffer) {
    fRecs = NULL;
    fCount = 0;
    
    int count = buffer.readInt();

    for (int i = 0; i < count; i++) {
        SkScalar dx = buffer.readScalar();
        SkScalar dy = buffer.readScalar();
        this->addLayer(dx, dy)->unflatten(buffer);
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
