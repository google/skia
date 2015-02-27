/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkDocument.h"
#include "SkPDFCanon.h"
#include "SkPDFDocument.h"
#include "SkPDFDevice.h"

class SkDocument_PDF : public SkDocument {
public:
    SkDocument_PDF(SkWStream* stream,
                   void (*doneProc)(SkWStream*, bool),
                   SkScalar rasterDpi)
        : SkDocument(stream, doneProc)
        , fDoc(SkNEW(SkPDFDocument))
        , fRasterDpi(rasterDpi) {}

    virtual ~SkDocument_PDF() {
        // subclasses must call close() in their destructors
        this->close();
    }

protected:
    virtual SkCanvas* onBeginPage(SkScalar width, SkScalar height,
                                  const SkRect& trimBox) SK_OVERRIDE {
        SkASSERT(!fCanvas.get());
        SkASSERT(!fDevice.get());

        SkISize pageSize = SkISize::Make(
                SkScalarRoundToInt(width), SkScalarRoundToInt(height));
        fDevice.reset(SkPDFDevice::Create(pageSize, fRasterDpi, &fCanon));
        fCanvas.reset(SkNEW_ARGS(SkCanvas, (fDevice)));
        fCanvas->clipRect(trimBox);
        fCanvas->translate(trimBox.x(), trimBox.y());
        return fCanvas.get();
    }

    void onEndPage() SK_OVERRIDE {
        SkASSERT(fCanvas.get());
        SkASSERT(fDevice.get());

        fCanvas->flush();
        fDoc->appendPage(fDevice.get());

        fCanvas.reset(NULL);
        fDevice.reset(NULL);
    }

    bool onClose(SkWStream* stream) SK_OVERRIDE {
        SkASSERT(!fCanvas.get());
        SkASSERT(!fDevice.get());

        bool success = fDoc->emitPDF(stream);
        fDoc.free();
        fCanon.reset();
        return success;
    }

    void onAbort() SK_OVERRIDE {
        fDoc.free();
        fCanon.reset();
    }

private:
    SkPDFCanon fCanon;
    SkAutoTDelete<SkPDFDocument> fDoc;
    SkAutoTUnref<SkPDFDevice> fDevice;
    SkAutoTUnref<SkCanvas> fCanvas;
    SkScalar fRasterDpi;
};

///////////////////////////////////////////////////////////////////////////////

SkDocument* SkDocument::CreatePDF(SkWStream* stream, SkScalar dpi) {
    return stream ? SkNEW_ARGS(SkDocument_PDF, (stream, NULL, dpi)) : NULL;
}

static void delete_wstream(SkWStream* stream, bool aborted) {
    SkDELETE(stream);
}

SkDocument* SkDocument::CreatePDF(const char path[], SkScalar dpi) {
    SkFILEWStream* stream = SkNEW_ARGS(SkFILEWStream, (path));
    if (!stream->isValid()) {
        SkDELETE(stream);
        return NULL;
    }
    return SkNEW_ARGS(SkDocument_PDF, (stream, delete_wstream, dpi));
}
