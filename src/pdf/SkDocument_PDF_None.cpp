/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkDocument.h"

using SkDoc = decltype(SkDocument::MakePDF(nullptr));
SkDoc SkDocument::MakePDF(SkWStream* stream, const PDFMetadata& metadata) {
    return nullptr;
}

SkDoc SkDocument::MakePDF(SkWStream* stream) {
    return nullptr;
}
