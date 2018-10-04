/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCanvas.h"
#include "SkPDFDocument.h"

#ifdef SK_SUPPORT_LEGACY_DOCUMENT_FACTORY
sk_sp<SkDocument> SkDocument::MakePDF(SkWStream* stream) {
    return nullptr;
}
#endif  // SK_SUPPORT_LEGACY_DOCUMENT_FACTORY

sk_sp<SkDocument> SkPDF::MakeDocument(SkWStream*, const SkPDF::Metadata&) { return nullptr; }

void SkPDF::SetNodeId(SkCanvas* c, int n) {
    c->drawAnnotation({0, 0, 0, 0}, "PDF_Node_Key", SkData::MakeWithCopy(&n, sizeof(n)).get());
}
