/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkPDFRasterizer_DEFINED
#define SkPDFRasterizer_DEFINED

#include "SkBitmap.h"
#include "SkStream.h"

bool SkPopplerRasterizePDF(SkStream* pdf, SkBitmap* output);

#endif
