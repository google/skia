/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef skqp_model_DEFINED
#define skqp_model_DEFINED

#include <string>

#include "SkBitmap.h"

class SkQPAssetManager;
class SkStreamAsset;

namespace skqp {

struct ModelResult {
    SkBitmap fErrors;
    std::unique_ptr<SkStreamAsset> fMinPng;
    std::unique_ptr<SkStreamAsset> fMaxPng;
    int fMaxError = 0;
    int fBadPixelCount = 0;
    std::string fErrorString;
};

ModelResult CheckAgainstModel(const char* name, const SkPixmap& image, SkQPAssetManager*);
}
#endif  // skqp_model_DEFINED
