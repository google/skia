/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/codec/SkCodec.h"
#include "include/core/SkBitmap.h"
#include "tests/Test.h"
#include "tools/Resources.h"
#include "tools/ToolUtils.h"

DEF_TEST(WebpCodecBlend, r) {
    const char* path = "images/blendBG.webp";
    auto codec = SkCodec::MakeFromData(GetResourceAsData(path));
    if (!codec) {
        ERRORF(r, "Failed to open/decode %s", path);
        return;
    }

    // Previously, a bug in SkWebpCodec resulted in different output depending
    // on whether kPremul or kOpaque SkAlphaType was passed to getPixels().
    // Decode each frame twice, once with kPremul and once with kOpaque if the
    // frame is opaque, and verify they look the same.
    auto premulInfo = codec->getInfo().makeAlphaType(kPremul_SkAlphaType);
    SkBitmap premulBm, changeBm;
    premulBm.allocPixels(premulInfo);
    changeBm.allocPixels(premulInfo);   // The SkBitmap's SkAlphaType is unrelated to the bug.

    for (int i = 0; i < codec->getFrameCount(); i++) {
        SkCodec::Options options;
        options.fFrameIndex = i;
        auto result = codec->getPixels(premulBm.pixmap(), &options);
        if (result != SkCodec::kSuccess) {
            ERRORF(r, "Failed to decode %s frame %i (premul) - error %s", path, i,
                   SkCodec::ResultToString(result));
            return;
        }

        SkCodec::FrameInfo frameInfo;
        if (!codec->getFrameInfo(i, &frameInfo)) {
            ERRORF(r, "Failed to getFrameInfo for %s frame %i", path, i);
            return;
        }

        auto alphaType = frameInfo.fAlphaType == kOpaque_SkAlphaType ? kOpaque_SkAlphaType
                                                                     : kPremul_SkAlphaType;
        result = codec->getPixels(premulInfo.makeAlphaType(alphaType), changeBm.getPixels(),
                                  changeBm.rowBytes(), &options);
        if (result != SkCodec::kSuccess) {
            ERRORF(r, "Failed to decode %s frame %i (change) - error %s", path, i,
                   SkCodec::ResultToString(result));
            return;
        }

        REPORTER_ASSERT(r, ToolUtils::equal_pixels(premulBm, changeBm), "%s frame %i does not match"
                        " with mismatched SkAlphaType", path, i);
    }
}
