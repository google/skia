/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkData.h"
#include "SkStream.h"
#include "SkDocument.h"

#include "sk_document.h"

namespace {
// adapt a sk_writer_proc into a SkWStream.
struct SkWriterWStream : public SkWStream {
    SkWriterWStream(sk_writer_proc writer, void* context)
        : fWriter(writer), fContext(context), fN(0) {}
    bool write(const void* buffer, size_t n) override {
        if (!fWriter(fContext, buffer, n)) {
            return false;
        }
        fN += n;
        return true;
    }
    size_t bytesWritten() const override { return fN; }
    sk_writer_proc fWriter;
    void* fContext;
    size_t fN;
};
}

struct sk_document_t {
    SkWriterWStream fWriterWStream;
    sk_sp<SkDocument> fDocument;
    sk_document_t(sk_writer_proc writer, void* context)
        : fWriterWStream(writer, context) {
        fDocument = SkDocument::MakePDF(&fWriterWStream);
    }
};

sk_document_t* sk_document_new_pdf(sk_writer_proc writer, void* context) {
    return new sk_document_t(writer, context);
}

sk_canvas_t* sk_document_begin_page(sk_document_t* doc, float w, float h) {
    return (sk_canvas_t*)doc->fDocument->beginPage(w, h);
}

void sk_document_close(sk_document_t* doc) {
    doc->fDocument->close();
    delete doc;
}

void sk_document_abort(sk_document_t* doc) {
    doc->fDocument->abort();
    delete doc;
}

