/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/docs/SkPDFDocument.h"

sk_sp<SkDocument> SkPDF::MakeDocument(SkWStream*, const SkPDF::Metadata&) { return nullptr; }

void SkPDF::SetNodeId(SkCanvas* c, int n) {
    c->drawAnnotation({0, 0, 0, 0}, "PDF_Node_Key", SkData::MakeWithCopy(&n, sizeof(n)).get());
}
