#include "SkPodofoParsedPDF.h"

#include "SkPdfPodofoTokenizer.h"
#include "SkPdfHeaders_autogen.h"
#include "SkPdfMapper_autogen.h"
#include "SkPdfBasics.h"
#include "SkPdfParser.h"

#include "podofo.h"

SkPodofoParsedPDF::SkPodofoParsedPDF(const char* path) : fDoc(new PoDoFo::PdfMemDocument(path))
                                                       , fMapper(new SkPdfMapper(this)) {}

SkPodofoParsedPDF::~SkPodofoParsedPDF() {
    delete fDoc;
    delete fMapper;
}

int SkPodofoParsedPDF::pages() const {
    return fDoc->GetPageCount();
}

double SkPodofoParsedPDF::width(int page) const {
    PoDoFo::PdfRect rect = fDoc->GetPage(page)->GetMediaBox();
    return rect.GetWidth() + rect.GetLeft();
}

double SkPodofoParsedPDF::height(int page) const {
    PoDoFo::PdfRect rect = fDoc->GetPage(page)->GetMediaBox();
    return rect.GetHeight() + rect.GetBottom();
}

const SkPdfResourceDictionary* SkPodofoParsedPDF::pageResources(int page) const {
    SkPdfPageObjectDictionary* pg = NULL;
    SkPdfObject* obj = make(fDoc->GetPage(page)->GetObject());
    fMapper->mapPageObjectDictionary(obj, &pg);
    return pg ? pg->Resources() : NULL;
}

SkRect SkPodofoParsedPDF::MediaBox(int page) const {
    PoDoFo::PdfRect rect = fDoc->GetPage(page)->GetMediaBox();
    SkRect skrect = SkRect::MakeLTRB(SkDoubleToScalar(rect.GetLeft()),
                                     SkDoubleToScalar(rect.GetBottom()),
                                     SkDoubleToScalar(rect.GetLeft() + rect.GetWidth()),
                                     SkDoubleToScalar(rect.GetBottom() + rect.GetHeight()));
    return skrect;
}


SkPdfPodofoTokenizer* SkPodofoParsedPDF::tokenizerOfPage(int page) const {
    PoDoFo::PdfContentsTokenizer* t = new PoDoFo::PdfContentsTokenizer(fDoc->GetPage(page));
    return new SkPdfPodofoTokenizer(this, t);
}

SkPdfPodofoTokenizer* SkPodofoParsedPDF::tokenizerOfStream(const SkPdfStream* stream) const {
    if (stream == NULL) {
        return NULL;
    }

    char* buffer = NULL;
    long len = 0;
    stream->GetFilteredCopy(&buffer, &len);
    return tokenizerOfBuffer(buffer, len);
}

SkPdfPodofoTokenizer* SkPodofoParsedPDF::tokenizerOfBuffer(char* buffer, size_t len) const {
    PoDoFo::PdfContentsTokenizer* t = new PoDoFo::PdfContentsTokenizer(buffer, len);
    return new SkPdfPodofoTokenizer(this, t);
}

size_t SkPodofoParsedPDF::objects() const {
    return fDoc->GetObjects().GetSize();
}

const SkPdfObject* SkPodofoParsedPDF::object(int i) const {
    PoDoFo::PdfVecObjects& objects = (PoDoFo::PdfVecObjects&)fDoc->GetObjects();
    return make(objects[i]);
}

SkPdfObject* SkPodofoParsedPDF::make(PoDoFo::PdfObject* obj) const {
    return new SkPdfObject(this, obj);
}

const SkPdfObject* SkPodofoParsedPDF::make(const PoDoFo::PdfObject* obj) const {
    return new SkPdfObject(this, obj);
}

const SkPdfMapper* SkPodofoParsedPDF::mapper() const {
    return fMapper;
}

SkPdfNumber* SkPodofoParsedPDF::createNumber(double number) const {
    return new SkPdfNumber(this, new PoDoFo::PdfObject(PoDoFo::PdfVariant(number)));
}

SkPdfInteger* SkPodofoParsedPDF::createInteger(int value) const {
    return new SkPdfInteger(this, new PoDoFo::PdfObject(PoDoFo::PdfVariant((PoDoFo::pdf_int64)value)));
}

SkPdfString* SkPodofoParsedPDF::createString(char* sz, size_t len) const {
    // TODO(edisonn): NYI
    return NULL;
}

PdfContext* gPdfContext = NULL;

void SkPodofoParsedPDF::drawPage(int page, SkCanvas* canvas) const {
    SkPdfPodofoTokenizer* tokenizer = tokenizerOfPage(page);

    PdfContext pdfContext(this);
    pdfContext.fOriginalMatrix = SkMatrix::I();
    pdfContext.fGraphicsState.fResources = pageResources(page);

    gPdfContext = &pdfContext;

    // TODO(edisonn): get matrix stuff right.
    // TODO(edisonn): add DPI/scale/zoom.
    SkScalar z = SkIntToScalar(0);
    SkRect rect = MediaBox(page);
    SkScalar w = rect.width();
    SkScalar h = rect.height();

    SkPoint pdfSpace[4] = {SkPoint::Make(z, z), SkPoint::Make(w, z), SkPoint::Make(w, h), SkPoint::Make(z, h)};
//                SkPoint skiaSpace[4] = {SkPoint::Make(z, h), SkPoint::Make(w, h), SkPoint::Make(w, z), SkPoint::Make(z, z)};

    // TODO(edisonn): add flag for this app to create sourunding buffer zone
    // TODO(edisonn): add flagg for no clipping.
    // Use larger image to make sure we do not draw anything outside of page
    // could be used in tests.

#ifdef PDF_DEBUG_3X
    SkPoint skiaSpace[4] = {SkPoint::Make(w+z, h+h), SkPoint::Make(w+w, h+h), SkPoint::Make(w+w, h+z), SkPoint::Make(w+z, h+z)};
#else
    SkPoint skiaSpace[4] = {SkPoint::Make(z, h), SkPoint::Make(w, h), SkPoint::Make(w, z), SkPoint::Make(z, z)};
#endif
    //SkPoint pdfSpace[2] = {SkPoint::Make(z, z), SkPoint::Make(w, h)};
    //SkPoint skiaSpace[2] = {SkPoint::Make(w, z), SkPoint::Make(z, h)};

    //SkPoint pdfSpace[2] = {SkPoint::Make(z, z), SkPoint::Make(z, h)};
    //SkPoint skiaSpace[2] = {SkPoint::Make(z, h), SkPoint::Make(z, z)};

    //SkPoint pdfSpace[3] = {SkPoint::Make(z, z), SkPoint::Make(z, h), SkPoint::Make(w, h)};
    //SkPoint skiaSpace[3] = {SkPoint::Make(z, h), SkPoint::Make(z, z), SkPoint::Make(w, 0)};

    SkAssertResult(pdfContext.fOriginalMatrix.setPolyToPoly(pdfSpace, skiaSpace, 4));
    SkTraceMatrix(pdfContext.fOriginalMatrix, "Original matrix");


    pdfContext.fGraphicsState.fMatrix = pdfContext.fOriginalMatrix;
    pdfContext.fGraphicsState.fMatrixTm = pdfContext.fGraphicsState.fMatrix;
    pdfContext.fGraphicsState.fMatrixTlm = pdfContext.fGraphicsState.fMatrix;

    canvas->setMatrix(pdfContext.fOriginalMatrix);

#ifndef PDF_DEBUG_NO_PAGE_CLIPING
    canvas->clipRect(SkRect::MakeXYWH(z, z, w, h), SkRegion::kIntersect_Op, true);
#endif

// erase with red before?
//        SkPaint paint;
//        paint.setColor(SK_ColorRED);
//        canvas->drawRect(rect, paint);

    PdfMainLooper looper(NULL, tokenizer, &pdfContext, canvas);
    looper.loop();

    delete tokenizer;

    canvas->flush();
}

// TODO(edisonn): move in trace util.
#include "SkMatrix.h"
#include "SkRect.h"

#ifdef PDF_TRACE
void SkTraceMatrix(const SkMatrix& matrix, const char* sz) {
    printf("SkMatrix %s ", sz);
    for (int i = 0 ; i < 9 ; i++) {
        printf("%f ", SkScalarToDouble(matrix.get(i)));
    }
    printf("\n");
}

void SkTraceRect(const SkRect& rect, const char* sz) {
    printf("SkRect %s ", sz);
    printf("x = %f ", SkScalarToDouble(rect.x()));
    printf("y = %f ", SkScalarToDouble(rect.y()));
    printf("w = %f ", SkScalarToDouble(rect.width()));
    printf("h = %f ", SkScalarToDouble(rect.height()));
    printf("\n");
}
#endif

