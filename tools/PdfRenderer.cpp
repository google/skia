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

void PdfRenderer::init(SkPicture* pict, SkWStream* stream) {
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
    fCanvas.reset(this->setupCanvas(stream, pict->width(), pict->height()));
}

SkCanvas* PdfRenderer::setupCanvas(SkWStream* stream, int width, int height) {
    fPdfDoc.reset(SkDocument::CreatePDF(stream, NULL, fEncoder));

    SkCanvas* canvas = fPdfDoc->beginPage(SkIntToScalar(width), SkIntToScalar(height));
    canvas->ref();

    return canvas;
}

void PdfRenderer::end() {
    fPicture = NULL;
    fCanvas.reset(NULL);
    fPdfDoc.reset(NULL);
}

bool SimplePdfRenderer::render() {
    SkASSERT(fCanvas.get() != NULL);
    SkASSERT(fPicture != NULL);
    if (NULL == fCanvas.get() || NULL == fPicture) {
        return false;
    }

    fCanvas->drawPicture(*fPicture);
    fCanvas->flush();

    return fPdfDoc->close();
}

}
