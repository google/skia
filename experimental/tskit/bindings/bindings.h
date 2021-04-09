/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SKIA_BINDINGS_H
#define SKIA_BINDINGS_H

#include <emscripten.h>
#include <emscripten/bind.h>
using namespace emscripten;

// The following two macros allow for the generation of various support files to create
// Canvaskit. The code inside the parentheses should be the Typescript declaration of whatever
// the following line or lines of code are describing. There are 3 types of files created, the
// ambient namespace files (e.g. core.d.ts; the public and private JS functions exposed by embind),
// externs.js (used to tell the Closure compiler not to minify certain names in the interface
// code) and the API Summary doc (index.d.ts). Types declared with TS_PRIVATE_EXPORT will
// only appear in the first two; TS_EXPORT will show up in all three.
//
// Because TS_EXPORT will show up in the public API docs, it is required that all TS_EXPORT
// declarations are preceded by docs starting with /** that will be copied into the final API
// summary doc, otherwise the generation step will fail.
//
// The declarations will be normal TS, with the exception of having a ClassName:: as a prefix if
// we are exposing a method on a class. This lets us properly group methods together.
//
// As an example:
//
//  TS_PRIVATE_EXPORT("_privateFunction(x: number, y: number): number")
//  function("_privateFunction", optional_override([](int x, int y)->size_t {
//      return x * y;
//  }));
//
//  /** See SkCanvas.h for more on this class */
//  class_<SkCanvas>("Canvas")
//  /**
//   * Draw the given paint using the current matrix and cli.
//   * @param p a paint to draw.
//   */
//  TS_EXPORT("Canvas::drawPaint(p: Paint): void")
//  .function("drawPaint", &SkCanvas::drawPaint)
#define TS_PRIVATE_EXPORT(ts_code)
#define TS_EXPORT(ts_code)

#endif  // SKIA_BINDINGS_H
