/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "PdfRenderer.h"
#include "SkCanvas.h"
#include "SkDevice.h"
#include "SkPDFDevice.h"
#include "SkPDFDocument.h"

namespace sk_tools {

void PdfRenderer::init(SkPicture* pict) {
    SkASSERT(NULL == fPicture);
    SkASSERT(NULL == fCanvas.get());
    if (fPicture != NULL || NULL != fCanvas.get()) {
        return;
    }

    SkASSERT(pict != NULL);
    if (NULL == pict) {
        return;
    }

    fPicture = pict;
    fCanvas.reset(this->setupCanvas());
}

SkCanvas* PdfRenderer::setupCanvas() {
    return this->setupCanvas(fPicture->width(), fPicture->height());
}

SkCanvas* PdfRenderer::setupCanvas(int width, int height) {
    SkISize pageSize = SkISize::Make(width, height);
    fPDFDevice = SkNEW_ARGS(SkPDFDevice, (pageSize, pageSize, SkMatrix::I()));
    return SkNEW_ARGS(SkCanvas, (fPDFDevice));
}

void PdfRenderer::end() {
    fPicture = NULL;
    fCanvas.reset(NULL);
    if (fPDFDevice) {
        SkDELETE(fPDFDevice);
        fPDFDevice = NULL;
    }
}

bool PdfRenderer::write(const SkString& path) const {
    SkPDFDocument doc;
    doc.appendPage(fPDFDevice);
    SkFILEWStream stream(path.c_str());
    if (stream.isValid()) {
        doc.emitPDF(&stream);
        return true;
    }
    return false;
}

void SimplePdfRenderer::render() {
    SkASSERT(fCanvas.get() != NULL);
    SkASSERT(fPicture != NULL);
    if (NULL == fCanvas.get() || NULL == fPicture) {
        return;
    }

    fCanvas->drawPicture(*fPicture);
    fCanvas->flush();
}

}
