/*
 * Copyright 2016 Xamarin Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkDocument.h"

#include "sk_document.h"

#include "sk_types_priv.h"

void sk_document_unref(sk_document_t* document) {
    AsDocument(document)->unref();
}

sk_document_t* sk_document_create_pdf_from_stream(sk_wstream_t* stream, float dpi) {
    return ToDocument(SkDocument::CreatePDF(AsWStream(stream), dpi));
}

sk_document_t* sk_document_create_pdf_from_filename(const char* path, float dpi) {
    return ToDocument(SkDocument::CreatePDF(path, dpi));
}

//sk_document_t* sk_document_create_xps_from_stream(sk_wstream_t* stream, float dpi) {
//    return ToDocument(SkDocument::CreateXPS(AsWStream(stream), dpi));
//}
//
//sk_document_t* sk_document_create_xps_from_filename(const char* path, float dpi) {
//    return ToDocument(SkDocument::CreateXPS(path, dpi));
//}

sk_canvas_t* sk_document_begin_page(sk_document_t* document, float width, float height, const sk_rect_t* content) {
    return ToCanvas(AsDocument(document)->beginPage(width, height, AsRect(content)));
}

void sk_document_end_page(sk_document_t* document) {
    AsDocument(document)->endPage();
}

bool sk_document_close(sk_document_t* document) {
    return AsDocument(document)->close();
}

void sk_document_abort(sk_document_t* document) {
    AsDocument(document)->abort();
}
