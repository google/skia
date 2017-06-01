/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkBmpBaseCodec.h"
#include "../private/SkMalloc.h"

SkBmpBaseCodec::~SkBmpBaseCodec() {}

SkBmpBaseCodec::SkBmpBaseCodec(int width, int height, const SkEncodedInfo& info, SkStream* stream,
                               uint16_t bitsPerPixel, SkCodec::SkScanlineOrder rowOrder)
    : INHERITED(width, height, info, stream, bitsPerPixel, rowOrder)
    , fSrcBuffer(sk_malloc_flags(this->srcRowBytes(), 0))
{}
