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
using TypedArray = emscripten::val;
using Uint8Array = emscripten::val;
using Uint16Array = emscripten::val;
using Uint32Array = emscripten::val;
using Float32Array = emscripten::val;

/**
 *  Create a typed-array (in the JS heap) and initialize it with the provided
 *  data (from the wasm heap). The caller is responsible for matching the type of data
 *  with the specified arrayType.
 *
 *  TODO: can we specialize this on T and provide the correct string?
 *          e.g. T==uint8_t --> "Uint8Array"
 */
template <typename T>
TypedArray MakeTypedArray(int count, const T src[], const char arrayType[]) {
    emscripten::val length = emscripten::val(count);
    emscripten::val jarray = emscripten::val::global(arrayType).new_(count);
    jarray.call<void>("set", val(typed_memory_view(count, src)));
    return jarray;
}

/**
 *  Reads a JS array, and returns a copy of it in the std::vector.
 *
 *  It is up to the caller to ensure the T and arrayType-string match
 *      e.g. T == uint16_t, arrayType == "Uint16Array"
 *
 *  Note, this checks the JS type, and if it does not match, it will still attempt
 *  to return a copy, just by performing a 1-element-at-a-time copy
 *  via emscripten::vecFromJSArray().
 */
template <typename T>
std::vector<T> CopyTypedArray(JSArray src, const char arrayType[]) {
    if (src.instanceof(emscripten::val::global(arrayType))) {
        const size_t len = src["length"].as<size_t>();
        std::vector<T> dst;
        dst.resize(len);
        auto dst_view = emscripten::val(typed_memory_view(len, dst.data()));
        dst_view.call<void>("set", src);
        return dst;
    } else {
        // copies one at a time
        return emscripten::vecFromJSArray<T>(src);
    }
}

#endif
