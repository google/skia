/*
 * Copyright 2023 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkJpegGainmap_codec_DEFINED
#define SkJpegGainmap_codec_DEFINED

#include "include/core/SkRefCnt.h"  // IWYU pragma: keep

class SkData;
class SkJpegSourceMgr;
class SkStream;
class SkJpegXmp;
struct SkGainmapInfo;
struct SkJpegMultiPictureParameters;

#include <memory>

/*
 * Implementation of onGetGainmap that detects Multi-Picture Format based gainmaps.
 */
bool SkJpegGetMultiPictureGainmap(const SkJpegMultiPictureParameters* mpParams,
                                  SkJpegSourceMgr* decoderSource,
                                  SkGainmapInfo* outInfo,
                                  std::unique_ptr<SkStream>* outGainmapImageStream);

/*
 * Implementation of onGetGainmap that detects JpegR based gainmaps.
 */
bool SkJpegGetJpegRGainmap(const SkJpegXmp* xmp,
                           SkJpegSourceMgr* decoderSource,
                           SkGainmapInfo* outInfo,
                           std::unique_ptr<SkStream>* outGainmapImageStream);

#endif
