/*
 * Copyright 2023 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkJpegXmp_codec_DEFINED
#define SkJpegXmp_codec_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/private/SkXmp.h"

class SkData;

#include <memory>
#include <vector>

// Find and parse all XMP metadata, given a list of all APP1 segment parameters.
std::unique_ptr<SkXmp> SkJpegMakeXmp(const std::vector<sk_sp<SkData>>& decoderApp1Params);

#endif
