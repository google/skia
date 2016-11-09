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
    called to complete the document.  You must call
    sk_document_unref() on the returned document.
*/
SK_API sk_document_t* sk_document_new_pdf(sk_writer_proc writer, void* context);

/**
    Increment the reference count on the given sk_document_t.
    Must be balanced by a call to sk_document_unref().
*/
SK_API void sk_document_ref(sk_document_t*);

/**
    Decrement the reference count. If the reference count is 1 before
    the decrement, then release both the memory holding the
    sk_document_t and any other resources it may be managing.  New
    sk_document_t are created with a reference count of 1.
*/
SK_API void sk_document_unref(sk_document_t*);

/**
    Begin a new page for the document, returning the canvas that will
    draw into the page. The document owns this canvas, and it will go
    out of scope when sk_document_end_page(), sk_document_close(),
    or sk_document_unref() is called.
*/
SK_API sk_canvas_t* sk_document_begin_page(sk_document_t*, float width, float height);

/**
    Call sk_document_end_page() when the content for the current page
    has been drawn (into the canvas returned by
    sk_document_begin_page()).  After this call the canvas returned by
    begin_page() will be out-of-scope.
*/
SK_API void sk_document_end_page(sk_document_t*);

/**
   Close the document and finish writing the output. After this method
   is called, do not call sk_document_begin_page or
   sk_document_close() again on this document.  After calling
   sk_document_close(), one must still call sk_document_unref() on
   this sk_document_t.
*/
SK_API void sk_document_close(sk_document_t*);

SK_C_PLUS_PLUS_END_GUARD

#endif
