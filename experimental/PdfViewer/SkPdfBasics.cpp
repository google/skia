#include "SkPdfBasics.h"
#include "SkPdfNativeTokenizer.h"

#include "SkDashPathEffect.h"

PdfContext::PdfContext(SkNativeParsedPDF* doc)
    : fPdfDoc(doc)
    , fTmpPageAllocator(new SkPdfAllocator()) {
}

PdfContext::~PdfContext() {
    delete fTmpPageAllocator;
}

void SkPdfGraphicsState::applyGraphicsState(SkPaint* paint, bool stroking) {
    if (stroking) {
        fStroking.applyGraphicsState(paint);
    } else {
        fNonStroking.applyGraphicsState(paint);
    }

    // TODO(edisonn): get this from pdfContext->options,
    // or pdfContext->addPaintOptions(&paint);
    paint->setAntiAlias(true);

    // TODO(edisonn): miter, ...
    if (stroking) {
        paint->setStrokeWidth(SkDoubleToScalar(fLineWidth));
        // TODO(edisonn): perf, two sets of allocs, create SkDashPathEffect constr that takes ownership
        // of the intervals
        if (fDashArrayLength > 0 && fDashPhase > 0) {
            paint->setPathEffect(new SkDashPathEffect(fDashArray, fDashArrayLength, fDashPhase))->unref();
        }
    }
}
