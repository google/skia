/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPDFDocument.h"

#ifdef SK_SUPPORT_LEGACY_DOCUMENT_FACTORY
sk_sp<SkDocument> SkDocument::MakePDF(SkWStream* stream, const PDFMetadata& metadata) {
    return nullptr;
}

sk_sp<SkDocument> SkDocument::MakePDF(SkWStream* stream) {
    return nullptr;
}
#endif  // SK_SUPPORT_LEGACY_DOCUMENT_FACTORY

sk_sp<SkDocument> SkPDF::MakeDocument(SkWStream*, const SkPDF::Metadata&) { return nullptr; }

