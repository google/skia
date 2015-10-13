// Copyright 2015 Google Inc. All Rights Reserved.
//
// Use of this source code is governed by a BSD-style license
// that can be found in the COPYING file in the root of the source
// tree. An additional intellectual property rights grant can be found
// in the file PATENTS. All contributing project authors may
// be found in the AUTHORS file in the root of the source tree.
// -----------------------------------------------------------------------------
//
// Checks if given pair of animated GIF/WebP images are identical:
// That is: their reconstructed canvases match pixel-by-pixel and their other
// animation properties (loop count etc) also match.
//
// example: anim_diff foo.gif bar.webp

#include <stdio.h>
#include <stdlib.h>  // for 'strtod'.
#include <string.h>  // for 'strcmp'.

#include <iostream>  // for 'cout'.
#include <sstream>   // for 'ostringstream'.

#include "./anim_util.h"

namespace {

template<typename T>
bool CompareValues(T a, T b, const std::string& output_str) {
  if (a != b) {
    std::cout << output_str << ": " << a << " vs " << b << std::endl;
    return false;
  }
  return true;
}

// Note: As long as frame durations and reconstructed frames are identical, it
// is OK for other aspects like offsets, dispose/blend method to vary.
bool CompareAnimatedImagePair(const AnimatedImage& img1,
                              const AnimatedImage& img2,
                              bool premultiply,
                              double min_psnr) {
  bool ok = true;
  ok = CompareValues(img1.canvas_width, img2.canvas_width,
                     "Canvas width mismatch") && ok;
  ok = CompareValues(img1.canvas_height, img2.canvas_height,
                     "Canvas height mismatch") && ok;
  ok = CompareValues(img1.frames.size(), img2.frames.size(),
                     "Frame count mismatch") && ok;
  if (!ok) return false;  // These are fatal failures, can't proceed.

  const bool is_multi_frame_image = (img1.frames.size() > 1);
  if (is_multi_frame_image) {  // Checks relevant for multi-frame images only.
    ok = CompareValues(img1.loop_count, img2.loop_count,
                       "Loop count mismatch") && ok;
    ok = CompareValues(img1.bgcolor, img2.bgcolor,
                       "Background color mismatch") && ok;
  }

  for (size_t i = 0; i < img1.frames.size(); ++i) {
    if (is_multi_frame_image) {  // Check relevant for multi-frame images only.
      std::ostringstream error_str;
      error_str << "Frame #" << i << ", duration mismatch";
      ok = CompareValues(img1.frames[i].duration, img2.frames[i].duration,
                         error_str.str()) && ok;
    }
    // Pixel-by-pixel comparison.
    const uint8_t* rgba1 = img1.frames[i].rgba.data();
    const uint8_t* rgba2 = img2.frames[i].rgba.data();
    int max_diff;
    double psnr;
    GetDiffAndPSNR(rgba1, rgba2, img1.canvas_width, img1.canvas_height,
                   premultiply, &max_diff, &psnr);
    if (min_psnr > 0.) {
      if (psnr < min_psnr) {
        fprintf(stderr, "Frame #%zu, psnr = %.2lf (min_psnr = %f)\n", i,
                psnr, min_psnr);
        ok = false;
      }
    } else {
      if (max_diff != 0) {
        fprintf(stderr, "Frame #%zu, max pixel diff: %d\n", i, max_diff);
        ok = false;
      }
    }
  }
  return ok;
}

void Help() {
  printf("\nUsage: anim_diff <image1> <image2> [-dump_frames <folder>] "
         "[-min_psnr <float>][-raw_comparison]\n");
}

}  // namespace

int main(int argc, const char* argv[]) {
  bool dump_frames = false;
  const char* dump_folder = NULL;
  double min_psnr = 0.;
  bool got_input1 = false;
  bool got_input2 = false;
  bool premultiply = true;
  const char* files[2];

  if (argc < 3) {
    Help();
    return -1;
  }

  for (int c = 1; c < argc; ++c) {
    bool parse_error = false;
    if (!strcmp(argv[c], "-dump_frames")) {
      if (c < argc - 1) {
        dump_frames = true;
        dump_folder = argv[++c];
      } else {
        parse_error = true;
      }
    } else if (!strcmp(argv[c], "-min_psnr")) {
      if (c < argc - 1) {
        const char* const v = argv[++c];
        char* end = NULL;
        const double d = strtod(v, &end);
        if (end == v) {
          parse_error = true;
          fprintf(stderr, "Error! '%s' is not a floating point number.\n", v);
        }
        min_psnr = d;
      } else {
        parse_error = true;
      }
    } else if (!strcmp(argv[c], "-raw_comparison")) {
      premultiply = false;
    } else {
      if (!got_input1) {
        files[0] = argv[c];
        got_input1 = true;
      } else if (!got_input2) {
        files[1] = argv[c];
        got_input2 = true;
      } else {
        parse_error = true;
      }
    }
    if (parse_error) {
      Help();
      return -1;
    }
  }
  if (!got_input2) {
    Help();
    return -1;
  }

  if (dump_frames) {
    printf("Dumping decoded frames in: %s\n", dump_folder);
  }

  AnimatedImage images[2];
  for (int i = 0; i < 2; ++i) {
    printf("Decoding file: %s\n", files[i]);
    if (!ReadAnimatedImage(files[i], &images[i], dump_frames, dump_folder)) {
      fprintf(stderr, "Error decoding file: %s\n Aborting.\n", files[i]);
      return -2;
    }
  }

  if (!CompareAnimatedImagePair(images[0], images[1],
                                premultiply, min_psnr)) {
    fprintf(stderr, "\nFiles %s and %s differ.\n", files[0], files[1]);
    return -3;
  }

  printf("\nFiles %s and %s are identical.\n", files[0], files[1]);
  return 0;
}
