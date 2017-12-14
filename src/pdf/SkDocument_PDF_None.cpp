/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkDocument.h"

sk_sp<SkDocument> SkDocument::MakePDF(SkWStream* stream, const PDFMetadata& metadata) {
    return nullptr;
}

sk_sp<SkDocument> SkDocument::MakePDF(SkWStream* stream) {
    return nullptr;
}

#ifdef SK_SUPPORT_LEGACY_PDF_PIXELSERIALIZER
sk_sp<SkDocument> SkDocument::MakePDF(SkWStream* stream,
                                      SkScalar dpi,
                                      const PDFMetadata& metadata,
                                      sk_sp<SkPixelSerializer> jpegEncoder,
                                      bool pdfa) {
    return nullptr;
}
sk_sp<SkDocument> SkDocument::MakePDF(SkWStream* stream, SkScalar dpi) {
    return nullptr;
}
sk_sp<SkDocument> SkDocument::MakePDF(const char path[], SkScalar dpi) {
    return nullptr;
}
#endif
