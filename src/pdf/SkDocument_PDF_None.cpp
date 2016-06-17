/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkDocument.h"
sk_sp<SkDocument> SkDocument::MakePDF(SkWStream*,
                                      SkScalar,
                                      const SkDocument::PDFMetadata&,
                                      sk_sp<SkPixelSerializer>,
                                      bool) {
    return nullptr;
}
sk_sp<SkDocument> SkDocument::MakePDF(const char path[], SkScalar) {
    return nullptr;
}
