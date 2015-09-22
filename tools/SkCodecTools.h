/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkCodecTools_DEFINED
#define SkCodecTools_DEFINED

inline float get_scale_from_sample_size(uint32_t sampleSize) {
    return 1.0f / (float) sampleSize;
}

#endif // SkCodecTools_DEFINED
