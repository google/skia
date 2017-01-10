/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkConservativeInfo_DEFINED
#define SkConservativeInfo_DEFINED

class GrCaps;
class SkColorSpace;

#include "SkImageInfo.h"

namespace SkConservativeInfo {

enum Format {
    kLegacy_Format,    // Input format, with any color space stripped out
    kAsIs_Format,      // Input format, with no modification
    kLinearF16_Format, // Half float RGBA with linear gamma
    kSRGB8888_Format,  // sRGB bytes

    kNumFormats,
};

Format ChooseFormat(const SkImageInfo& info, SkColorSpace* dstColorSpace, const GrCaps* grCaps);
SkImageInfo Make(const SkImageInfo& info, Format format);
SkImageInfo Make(const SkImageInfo& info, SkColorSpace* dstColorSpace, const GrCaps* caps);

// Convenience API, assumes that we want color correct behavior without GPU limitations.
SkImageInfo Make(const SkImageInfo& info);

};

#endif
