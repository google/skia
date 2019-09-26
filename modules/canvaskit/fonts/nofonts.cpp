/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/core/SkTypes.h"
struct SkEmbeddedResource { const uint8_t* d; const size_t s; };
static const SkEmbeddedResource header[] = {};
static const int header_count = 0;
struct SkEmbeddedHeader {const SkEmbeddedResource* e; const int c;};
extern "C" const SkEmbeddedHeader SK_EMBEDDED_FONTS = { header, header_count };
