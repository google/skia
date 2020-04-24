/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPngPriv_DEFINED
#define SkPngPriv_DEFINED

#include "include/core/SkTypes.h"

// We store kAlpha_8 images as GrayAlpha in png. Our private signal is significant bits for gray.
// If that is set to 1, we assume the gray channel can be ignored, and we output just alpha.
// We tried 0 at first, but png doesn't like a 0 sigbit for a channel it expects, hence we chose 1.

static constexpr int kGraySigBit_GrayAlphaIsJustAlpha = 1;

#endif
