/*
 * Copyright 2014 Google Inc.
 * Copyright 2015 Xamarin Inc.
 * Copyright 2017 Microsoft Corporation. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef sk_document_DEFINED
#define sk_document_DEFINED

#include "include/c/sk_types.h"

SK_C_PLUS_PLUS_BEGIN_GUARD

SK_C_API void sk_document_unref(sk_document_t* document);

SK_C_API sk_document_t* sk_document_create_pdf_from_stream(sk_wstream_t* stream);
SK_C_API sk_document_t* sk_document_create_pdf_from_stream_with_metadata(sk_wstream_t* stream, const sk_document_pdf_metadata_t* metadata);

SK_C_API sk_document_t* sk_document_create_xps_from_stream(sk_wstream_t* stream, float dpi);

SK_C_API sk_canvas_t* sk_document_begin_page(sk_document_t* document, float width, float height, const sk_rect_t* content);
SK_C_API void sk_document_end_page(sk_document_t* document);
SK_C_API void sk_document_close(sk_document_t* document);
SK_C_API void sk_document_abort(sk_document_t* document);

SK_C_PLUS_PLUS_END_GUARD

#endif
