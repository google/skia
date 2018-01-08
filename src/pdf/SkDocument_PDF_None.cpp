/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkDocument.h"

#ifdef SK_SUPPORT_LEGACY_REFCNT_DOCUMENT
sk_sp<SkDocument> SkDocument::MakePDF(SkWStream* stream, const PDFMetadata& metadata) {
    return nullptr;
}

sk_sp<SkDocument> SkDocument::MakePDF(SkWStream* stream) {
    return nullptr;
}
#else
std::unique_ptr<SkDocument> SkDocument::MakePDF(SkWStream* stream, const PDFMetadata& metadata) {
    return nullptr;
}

std::unique_ptr<SkDocument> SkDocument::MakePDF(SkWStream* stream) {
    return nullptr;
}
#endif
