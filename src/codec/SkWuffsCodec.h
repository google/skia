/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkWuffsCodec_DEFINED
#define SkWuffsCodec_DEFINED

#include "include/codec/SkCodec.h"

// These functions' types match DecoderProc in SkCodec.cpp.
bool                     SkWuffsCodec_IsFormat(const void*, size_t);
std::unique_ptr<SkCodec> SkWuffsCodec_MakeFromStream(std::unique_ptr<SkStream>, SkCodec::Result*);

#endif  // SkWuffsCodec_DEFINED
