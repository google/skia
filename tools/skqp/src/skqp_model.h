/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef skqp_model_DEFINED
#define skqp_model_DEFINED

#include <cstdint>
#include <string>

#include "SkBitmap.h"

#include "skqp.h"

class SkQPAssetManager;
class SkStreamAsset;

namespace skqp {

struct ModelResult {
    SkBitmap fErrors; // Correct pixels are white, failing pixels scale from black
                      // (1 value off) to red (255 off in some channel).
    sk_sp<SkData> fMinPng;  // original model data
    sk_sp<SkData> fMaxPng;  // original model data
    SkQP::RenderOutcome fOutcome;
    std::string fErrorString;  // if non-empty, an error occured.
};

SkQP::RenderOutcome Check(const SkPixmap& minImg,
                          const SkPixmap& maxImg,
                          const SkPixmap& img,
                          unsigned tolerance,
                          SkBitmap* errorOut);

/**
Check if the given test image matches the expected results.

@param name          the name of the rendering test that produced the image
@param image         the image to be tested.  Should be kRGBA_8888_SkColorType
                     and kUnpremul_SkAlphaType.
@param assetManager  provides model data files
 */

ModelResult CheckAgainstModel(const char* name, const SkPixmap& image,
                              SkQPAssetManager* assetManager);
}
#endif  // skqp_model_DEFINED
