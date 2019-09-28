/*
 * Copyright 2014 Google Inc.
 * Copyright 2015 Xamarin Inc.
 * Copyright 2017 Microsoft Corporation. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkDocument.h"
#include "include/docs/SkPDFDocument.h"
#include "include/docs/SkXPSDocument.h"

#include "include/c/sk_document.h"

#include "src/c/sk_types_priv.h"

void sk_document_unref(sk_document_t* document) {
    SkSafeUnref(AsDocument(document));
}

sk_document_t* sk_document_create_pdf_from_stream(sk_wstream_t* stream) {
    return ToDocument(SkPDF::MakeDocument(AsWStream(stream)).release());
}

sk_document_t* sk_document_create_pdf_from_stream_with_metadata(sk_wstream_t* stream, const sk_document_pdf_metadata_t* cmetadata) {
    return ToDocument(SkPDF::MakeDocument(AsWStream(stream), AsDocumentPDFMetadata(cmetadata)).release());
}

sk_document_t* sk_document_create_xps_from_stream(sk_wstream_t* stream, float dpi) {
    return ToDocument(SkXPS::MakeDocument(AsWStream(stream), dpi).release());
}

sk_canvas_t* sk_document_begin_page(sk_document_t* document, float width, float height, const sk_rect_t* content) {
    return ToCanvas(AsDocument(document)->beginPage(width, height, AsRect(content)));
}

void sk_document_end_page(sk_document_t* document) {
    AsDocument(document)->endPage();
}

void sk_document_close(sk_document_t* document) {
    AsDocument(document)->close();
}

void sk_document_abort(sk_document_t* document) {
    AsDocument(document)->abort();
}
