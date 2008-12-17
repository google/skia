#include "SkBlurDrawLooper.h"
#include "SkBlurMaskFilter.h"
#include "SkCanvas.h"
#include "SkPaint.h"
#include "SkMaskFilter.h"

SkBlurDrawLooper::SkBlurDrawLooper(SkScalar radius, SkScalar dx, SkScalar dy,
                                   SkColor color)
    : fDx(dx), fDy(dy), fBlurColor(color)
{
    if (radius > 0)
        fBlur = SkBlurMaskFilter::Create(radius,
                                         SkBlurMaskFilter::kNormal_BlurStyle);
    else
        fBlur = NULL;
}

SkBlurDrawLooper::SkBlurDrawLooper(SkFlattenableReadBuffer& buffer)
{
    fDx = buffer.readScalar();
    fDy = buffer.readScalar();
    fBlurColor = buffer.readU32();
    fBlur = static_cast<SkMaskFilter*>(buffer.readFlattenable());
}

SkBlurDrawLooper::~SkBlurDrawLooper()
{
    fBlur->safeUnref();
}

void SkBlurDrawLooper::flatten(SkFlattenableWriteBuffer& buffer)
{
    buffer.writeScalar(fDx);
    buffer.writeScalar(fDy);
    buffer.write32(fBlurColor);
    buffer.writeFlattenable(fBlur);
}

void SkBlurDrawLooper::init(SkCanvas* canvas, SkPaint* paint)
{
    // we do nothing if a maskfilter is already installed
    if (paint->getMaskFilter() != NULL)
        fState = kDone;
    else
    {
        fState = kBeforeEdge;
        fPaint = paint;
        fCanvas = canvas;
        fSaveCount = canvas->getSaveCount();
    }
}

bool SkBlurDrawLooper::next()
{
    switch (fState) {
    case kBeforeEdge:
        fSavedColor = fPaint->getColor();
        fPaint->setColor(fBlurColor);
        fPaint->setMaskFilter(fBlur);
        fCanvas->save(SkCanvas::kMatrix_SaveFlag);
        fCanvas->translate(fDx, fDy);
        fState = kAfterEdge;
        return true;
    case kAfterEdge:
        fPaint->setColor(fSavedColor);
        fPaint->setMaskFilter(NULL);
        fCanvas->restore(); // to remove the translate we did earlier
        fState = kDone;
        return true;
    default:
        SkASSERT(kDone == fState);
        return false;
    }
}

void SkBlurDrawLooper::restore()
{
    if (kAfterEdge == fState)
    {
        fPaint->setColor(fSavedColor);
        fPaint->setMaskFilter(NULL);
        fCanvas->restore(); // to remove the translate we did earlier
        fState = kDone;
    }
}

///////////////////////////////////////////////////////////////////////////////

static SkFlattenable::Registrar gReg("SkBlurDrawLooper",
                                     SkBlurDrawLooper::CreateProc);

