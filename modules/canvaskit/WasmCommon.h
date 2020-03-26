/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef WasmCommon_DEFINED
#define WasmCommon_DEFINED

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

// A struct used for binding the TypedArray  colors passed to to canvaskit functions.
// Canvaskit functions returning colors return a Float32Array, which looks the same
// on the javascript side.
struct SimpleColor4f {
	// A sensible but noticeable default value to let you know you've called the
	// default constructor.
    float r = 1.0;
    float g = 0.0;
    float b = 1.0;
    float a = 1.0;

    SkColor4f toSkColor4f() const {
      return SkColor4f({r, g, b, a});
    };
    SkColor toSkColor() const {
      return toSkColor4f().toSkColor();
    };
};

#endif
