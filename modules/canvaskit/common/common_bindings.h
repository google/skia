/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef common_bindings_DEFINED
#define common_bindings_DEFINED

#include <emscripten.h>
#include <emscripten/bind.h>

#include "include/core/SkColor.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkImageInfo.h"

using namespace emscripten;

// Self-documenting types
using JSArray = emscripten::val;
using JSObject = emscripten::val;
using JSString = emscripten::val;
using SkPathOrNull = emscripten::val;
using Uint8Array = emscripten::val;
using Float32Array = emscripten::val;

// Surface creation structs and helpers
struct SimpleImageInfo {
    int width;
    int height;
    SkColorType colorType;
    SkAlphaType alphaType;
    sk_sp<SkColorSpace> colorSpace;

    static SkImageInfo toSkImageInfo(const SimpleImageInfo& sii) {
      return SkImageInfo::Make(
        sii.width, sii.height, sii.colorType, sii.alphaType, sii.colorSpace);
    }

    static SimpleImageInfo toSimpleImageInfo(const SkImageInfo& ii) {
      return (SimpleImageInfo){ii.width(), ii.height(), ii.colorType(), ii.alphaType()};
    }
};

#endif