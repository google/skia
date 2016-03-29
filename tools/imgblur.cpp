/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmap.h"
#include "SkCommandLineFlags.h"
#include "SkCommonFlags.h"
#include "SkData.h"
#include "SkForceLinking.h"
#include "SkImage.h"
#include "SkStream.h"
#include "SkTypes.h"

#include "sk_tool_utils.h"

__SK_FORCE_IMAGE_DECODER_LINKING;

DEFINE_string(in, "input.png", "Input image");
DEFINE_string(out, "blurred.png", "Output image");
DEFINE_double(sigma, 1, "Sigma to be used for blur (> 0.0f)");


// This tool just performs a blur on an input image
// Return codes:
static const int kSuccess = 0;
static const int kError = 1;

int tool_main(int argc, char** argv);
int tool_main(int argc, char** argv) {
    SkCommandLineFlags::SetUsage("Brute force blur of an image.");
    SkCommandLineFlags::Parse(argc, argv);

    if (FLAGS_sigma <= 0) {
        if (!FLAGS_quiet) {
            SkDebugf("Sigma must be greater than zero (it is %f).\n", FLAGS_sigma);
        }
        return kError;
    }

    sk_sp<SkData> data(SkData::MakeFromFileName(FLAGS_in[0]));
    if (nullptr == data) {
        if (!FLAGS_quiet) {
            SkDebugf("Couldn't open file: %s\n", FLAGS_in[0]);
        }
        return kError;
    }

    sk_sp<SkImage> image(SkImage::MakeFromEncoded(data));
    if (!image) {
        if (!FLAGS_quiet) {
            SkDebugf("Couldn't create image for: %s.\n", FLAGS_in[0]);
        }
        return kError;
    }

    SkBitmap src;
    if (!image->asLegacyBitmap(&src, SkImage::kRW_LegacyBitmapMode)) {
        if (!FLAGS_quiet) {
            SkDebugf("Couldn't create bitmap for: %s.\n", FLAGS_in[0]);
        }
        return kError;
    }

    SkBitmap dst = sk_tool_utils::slow_blur(src, (float) FLAGS_sigma);

    if (!SkImageEncoder::EncodeFile(FLAGS_out[0], dst, SkImageEncoder::kPNG_Type, 100)) {
        if (!FLAGS_quiet) {
            SkDebugf("Couldn't write to file: %s\n", FLAGS_out[0]);
        }
        return kError;
    }

    return kSuccess;
}

#if !defined SK_BUILD_FOR_IOS
int main(int argc, char * const argv[]) {
    return tool_main(argc, (char**) argv);
}
#endif
