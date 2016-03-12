/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkPDFDocument_DEFINED
#define SkPDFDocument_DEFINED

#include "SkDocument.h"

sk_sp<SkDocument> SkPDFMakeDocument(
        SkWStream* stream,
        void (*doneProc)(SkWStream*, bool),
        SkScalar rasterDpi,
        SkPixelSerializer* jpegEncoder);

#endif  // SkPDFDocument_DEFINED
