/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef WasmAliases_DEFINED
#define WasmAliases_DEFINED

#include <emscripten.h>
#include <emscripten/bind.h>
#include "include/core/SkColor.h"

using namespace emscripten;

// Self-documenting types
using JSArray = emscripten::val;
using JSObject = emscripten::val;
using JSString = emscripten::val;
using SkPathOrNull = emscripten::val;
using Uint8Array = emscripten::val;
using Float32Array = emscripten::val;

struct SimpleColor4f {
    float r, g, b, a;

    SkColor4f toSkColor4f() const {
      return SkColor4f({r, g, b, a});
    };
    SkColor toSkColor() const {
      return toSkColor4f().toSkColor();
    };
};

#endif
