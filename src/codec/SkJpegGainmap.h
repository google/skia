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
struct SkGainmapInfo;

#include <memory>

/*
 * Implementation of onGetGainmap that detects Multi-Picture Format based gainmaps.
 */
bool SkJpegGetMultiPictureGainmap(sk_sp<const SkData> decoderMpfMetadata,
                                  SkJpegSourceMgr* decoderSource,
                                  SkGainmapInfo* outInfo,
                                  std::unique_ptr<SkStream>* outGainmapImageStream);

/*
 * Implementation of onGetGainmap that detects JpegR based gainmaps.
 */
bool SkJpegGetJpegRGainmap(sk_sp<const SkData> xmpMetadata,
                           SkJpegSourceMgr* decoderSource,
                           SkGainmapInfo* outInfo,
                           std::unique_ptr<SkStream>* outGainmapImageStream);

/*
 * Implementation of onGetGainmap that detects HDRGM based gainmaps and converts them to
 * SkGainmapInfo.
 */
bool SkJpegGetHDRGMGainmapInfo(sk_sp<const SkData> xmpMetadata, SkGainmapInfo* outGainmapInfo);

#endif
