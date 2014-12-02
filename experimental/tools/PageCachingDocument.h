/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef PageCachingDocument_DEFINED
#define PageCachingDocument_DEFINED

#include "SkScalar.h"

class SkBitmap;
class SkData;
class SkDocument;
class SkWStream;

SkDocument* CreatePageCachingDocument(
    SkWStream* stream,
    void (*done)(SkWStream*, bool) = NULL,
    SkData* (*encoder)(size_t*, const SkBitmap&) = NULL,
    SkScalar rasterDpi = 72.0);
#endif  // PageCachingDocument_DEFINED
