/*
 * Copyright 2016 Xamarin Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// EXPERIMENTAL EXPERIMENTAL EXPERIMENTAL EXPERIMENTAL
// DO NOT USE -- FOR INTERNAL TESTING ONLY

#ifndef sk_document_DEFINED
#define sk_document_DEFINED

#include "sk_types.h"

SK_C_PLUS_PLUS_BEGIN_GUARD

SK_API void sk_document_unref(sk_document_t* document);

SK_API sk_document_t* sk_document_create_pdf_from_stream(sk_wstream_t* stream, float dpi);
SK_API sk_document_t* sk_document_create_pdf_from_filename(const char* path, float dpi);

//SK_API sk_document_t* sk_document_create_xps_from_stream(sk_wstream_t* stream, float dpi);
//SK_API sk_document_t* sk_document_create_xps_from_filename(const char* path, float dpi);

SK_API sk_canvas_t* sk_document_begin_page(sk_document_t* document, float width, float height, const sk_rect_t* content);
SK_API void sk_document_end_page(sk_document_t* document);
SK_API bool sk_document_close(sk_document_t* document);
SK_API void sk_document_abort(sk_document_t* document);

// TODO: setMetadata

SK_C_PLUS_PLUS_END_GUARD

#endif
