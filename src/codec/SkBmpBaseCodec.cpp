/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "src/codec/SkBmpBaseCodec.h"

#include "include/core/SkStream.h"
#include "include/private/SkEncodedInfo.h"
#include "include/private/base/SkMalloc.h"

#include <utility>

SkBmpBaseCodec::~SkBmpBaseCodec() {}

SkBmpBaseCodec::SkBmpBaseCodec(SkEncodedInfo&& info, std::unique_ptr<SkStream> stream,
                               uint16_t bitsPerPixel, SkCodec::SkScanlineOrder rowOrder)
    : INHERITED(std::move(info), std::move(stream), bitsPerPixel, rowOrder)
    , fSrcBuffer(sk_malloc_canfail(this->srcRowBytes()))
{}
