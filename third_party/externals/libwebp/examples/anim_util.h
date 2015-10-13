// Copyright 2015 Google Inc. All Rights Reserved.
//
// Use of this source code is governed by a BSD-style license
// that can be found in the COPYING file in the root of the source
// tree. An additional intellectual property rights grant can be found
// in the file PATENTS. All contributing project authors may
// be found in the AUTHORS file in the root of the source tree.
// -----------------------------------------------------------------------------
//
// Utilities for animated images

#ifndef WEBP_EXAMPLES_ANIM_UTIL_H_
#define WEBP_EXAMPLES_ANIM_UTIL_H_

#ifdef HAVE_CONFIG_H
#include "webp/config.h"
#endif

#include <vector>

#include "webp/types.h"

struct DecodedFrame {
  std::vector<uint8_t> rgba;  // Decoded and reconstructed full frame.
  int duration;          // Frame duration in milliseconds.
  bool is_key_frame;     // True if this frame is a key-frame.
};

struct AnimatedImage {
  uint32_t canvas_width;
  uint32_t canvas_height;
  uint32_t bgcolor;
  uint32_t loop_count;
  std::vector<DecodedFrame> frames;
};

// Read animated image file into 'AnimatedImage' struct.
// If 'dump_frames' is true, dump frames to 'dump_folder'.
bool ReadAnimatedImage(const char filename[], AnimatedImage* const image,
                       bool dump_frames, const char dump_folder[]);

// Given two RGBA buffers, calculate max pixel difference and PSNR.
// If 'premultiply' is true, R/G/B values will be pre-multiplied by the
// transparency before comparison.
void GetDiffAndPSNR(const uint8_t rgba1[], const uint8_t rgba2[],
                    uint32_t width, uint32_t height, bool premultiply,
                    int* const max_diff, double* const psnr);

#endif  // WEBP_EXAMPLES_ANIM_UTIL_H_
