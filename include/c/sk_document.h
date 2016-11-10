/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

/* EXPERIMENTAL EXPERIMENTAL EXPERIMENTAL EXPERIMENTAL */
/* DO NOT USE -- FOR INTERNAL TESTING ONLY */

#ifndef sk_document_DEFINED
#define sk_document_DEFINED

#include "sk_types.h"

SK_C_PLUS_PLUS_BEGIN_GUARD

/**
    Return a pointer to a new sk_document_t object which will write a
    PDF document to the sk_writer_proc.  sk_document_close() must be
    called to complete the document.

    You must call either sk_document_close() or sk_document_abort()
    on the returned document to release both the memory holding the
    sk_document_t and any other resources it may be managing.
*/
SK_API sk_document_t* sk_document_new_pdf(sk_writer_proc writer, void* context);

/**
    Call sk_document_abort() to stop producing the document immediately.
    The stream output must be ignored, and should not be trusted.
    After calling this function, the sk_document_t pointer is disposed of.
*/
SK_API void sk_document_abort(sk_document_t*);

/**
    Begin a new page for the document, returning the canvas that will
    draw into the page. The document owns this canvas, and it will go
    out of scope when sk_document_close() or sk_document_abort() is
    called, or when sk_document_begin_page() is called again.
*/
SK_API sk_canvas_t* sk_document_begin_page(sk_document_t*, float width, float height);

/**
   Close the document and finish writing the output. After this method
   is called, do not call sk_document_begin_page() or
   sk_document_close() again on this document.
*/
SK_API void sk_document_close(sk_document_t*);

SK_C_PLUS_PLUS_END_GUARD

#endif
