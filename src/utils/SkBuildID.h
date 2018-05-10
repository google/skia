/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkBuildID_DEFINED
#define SkBuildID_DEFINED

#include <cinttypes>

/** Returns a different value every time this function is recompiled,
    based on __DATE__ and __TIME__. */
uint32_t SkBuildID();

#endif  // SkBuildID_DEFINED
