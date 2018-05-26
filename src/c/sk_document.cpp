/*
 * Copyright 2014 Google Inc.
 * Copyright 2015 Xamarin Inc.
 * Copyright 2017 Microsoft Corporation. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkDocument.h"

#include "sk_document.h"

#include "sk_types_priv.h"

void sk_document_unref(sk_document_t* document) {
    SkSafeUnref(AsDocument(document));
}

sk_document_t* sk_document_create_pdf_from_stream(sk_wstream_t* stream, float dpi) {
    return ToDocument(SkDocument::MakePDF(AsWStream(stream), dpi).release());
}

sk_document_t* sk_document_create_pdf_from_stream_with_metadata(sk_wstream_t* stream, float dpi, const sk_document_pdf_metadata_t* cmetadata) {
    SkDocument::PDFMetadata metadata;
    from_c(*cmetadata, &metadata);
    return ToDocument(SkDocument::MakePDF(AsWStream(stream), dpi, metadata, nullptr, false).release());
}

sk_document_t* sk_document_create_pdf_from_filename(const char* path, float dpi) {
    return ToDocument(SkDocument::MakePDF(path, dpi).release());
}

sk_document_t* sk_document_create_xps_from_stream(sk_wstream_t* stream, float dpi) {
    return ToDocument(SkDocument::MakeXPS(AsWStream(stream), dpi).release());
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
