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

// Will only show up in module.d.ts
#define TS_PRIVATE_EXPORT(ts_code)
// Will show up in module.d.ts and index.d.ts. Needs to be preceded by a /** doc
#define TS_EXPORT(ts_code)

#endif  // SKIA_BINDINGS_H
