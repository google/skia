#ifndef __DEFINED__SkPdfBasics
#define __DEFINED__SkPdfBasics

#include "SkCanvas.h"
#include "SkPaint.h"
#include "SkPdfConfig.h"

#include <iostream>
#include <cstdio>
#include <map>
#include <stack>

class SkPdfFont;
class SkPdfDoc;
class SkPdfObject;
class SkPdfResourceDictionary;

class SkPodofoParsedPDF;

// TODO(edisonn): better class design.
struct SkPdfColorOperator {
    std::string fColorSpace;  // TODO(edisonn): use SkString
    SkColor fColor;
    double fOpacity;  // ca or CA
    // TODO(edisonn): add here other color space options.

    void setRGBColor(SkColor color) {
        // TODO(edisonn): ASSERT DeviceRGB is the color space.
        fColor = color;
    }
    // TODO(edisonn): double check the default values for all fields.
    SkPdfColorOperator() : fColor(SK_ColorBLACK), fOpacity(1) {}

    void applyGraphicsState(SkPaint* paint) {
        paint->setColor(SkColorSetA(fColor, fOpacity * 255));
    }
};

// TODO(edisonn): better class design.
struct SkPdfGraphicsState {
    SkMatrix            fMatrix;
    SkMatrix            fMatrixTm;
    SkMatrix            fMatrixTlm;

    double              fCurPosX;
    double              fCurPosY;

    double              fCurFontSize;
    bool                fTextBlock;
    SkPdfFont*          fSkFont;
    SkPath              fPath;
    bool                fPathClosed;

    // Clip that is applied after the drawing is done!!!
    bool                fHasClipPathToApply;
    SkPath              fClipPath;

    SkPdfColorOperator  fStroking;
    SkPdfColorOperator  fNonStroking;

    double              fLineWidth;
    double              fTextLeading;
    double              fWordSpace;
    double              fCharSpace;

    const SkPdfResourceDictionary* fResources;

    SkBitmap            fSMask;

    SkPdfGraphicsState() {
        fCurPosX      = 0.0;
        fCurPosY      = 0.0;
        fCurFontSize  = 0.0;
        fTextBlock    = false;
        fMatrix       = SkMatrix::I();
        fMatrixTm     = SkMatrix::I();
        fMatrixTlm    = SkMatrix::I();
        fPathClosed   = true;
        fLineWidth    = 0;
        fTextLeading  = 0;
        fWordSpace    = 0;
        fCharSpace    = 0;
        fHasClipPathToApply = false;
        fResources    = NULL;
        fSkFont       = NULL;
    }

    void applyGraphicsState(SkPaint* paint, bool stroking) {
        if (stroking) {
            fStroking.applyGraphicsState(paint);
        } else {
            fNonStroking.applyGraphicsState(paint);
        }

        // TODO(edisonn): get this from pdfContext->options,
        // or pdfContext->addPaintOptions(&paint);
        paint->setAntiAlias(true);

        // TODO(edisonn): dashing, miter, ...
        paint->setStrokeWidth(SkDoubleToScalar(fLineWidth));
    }
};

// TODO(edisonn): better class design.
// TODO(edisonn): could we remove it?
// TODO(edisonn): rename to SkPdfInlineImage
struct SkPdfInlineImage {
    std::map<std::string, std::string> fKeyValuePairs;
    std::string fImageData;
};

// TODO(edisonn): better class design.
// TODO(edisonn): rename to SkPdfContext
struct PdfContext {
    std::stack<SkPdfObject*>        fObjectStack;
    std::stack<SkPdfGraphicsState>  fStateStack;
    SkPdfGraphicsState              fGraphicsState;
    const SkPodofoParsedPDF*        fPdfDoc;
    SkMatrix                        fOriginalMatrix;

    SkPdfInlineImage                fInlineImage;

    PdfContext(const SkPodofoParsedPDF* doc) :  fPdfDoc(doc) {}

};

// TODO(edisonn): temporary code, to report how much of the PDF we actually think we rendered.
// TODO(edisonn): rename to SkPdfResult
enum PdfResult {
    kOK_PdfResult,
    kPartial_PdfResult,
    kNYI_PdfResult,
    kIgnoreError_PdfResult,
    kError_PdfResult,
    kUnsupported_PdfResult,

    kCount_PdfResult
};

#endif  // __DEFINED__SkPdfBasics
