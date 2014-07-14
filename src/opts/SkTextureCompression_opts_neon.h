/*
 * Copyright 2014 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkTextureCompression_opts_neon_h_
#define SkTextureCompression_opts_neon_h_

bool CompressA8toR11EAC_NEON(uint8_t* dst, const uint8_t* src,
                             int width, int height, int rowBytes);

#endif  // SkTextureCompression_opts_neon_h_
