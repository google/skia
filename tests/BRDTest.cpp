/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// Make sure SkUserConfig.h is included so #defines are available on
// Android.
#include "include/core/SkTypes.h"
#ifdef SK_ENABLE_ANDROID_UTILS
#include "client_utils/android/BitmapRegionDecoder.h"
#include "include/codec/SkAndroidCodec.h"
#include "include/codec/SkCodec.h"
#include "tests/Test.h"
#include "tools/Resources.h"

DEF_TEST(BRD_types, r) {
    static const struct {
        const char* name;
        bool supported;
    } gRec[] = {
        { "images/arrow.png", true },
        { "images/box.gif", false },
        { "images/baby_tux.webp", true },
        { "images/brickwork-texture.jpg", true },
        { "images/color_wheel.ico", false },
#ifdef SK_CODEC_DECODES_RAW
        { "images/sample_1mp.dng", false },
#endif
        { "images/mandrill.wbmp", false },
        { "images/randPixels.bmp", false },
    };

    for (const auto& rec : gRec) {
        auto data = GetResourceAsData(rec.name);
        if (!data) return;

        REPORTER_ASSERT(r, SkCodec::MakeFromData(data) != nullptr);
        REPORTER_ASSERT(r, SkAndroidCodec::MakeFromData(data) != nullptr);

        auto brd = android::skia::BitmapRegionDecoder::Make(data);
        if (rec.supported) {
            if (!brd) ERRORF(r, "Failed to create BRD from %s", rec.name);
        } else {
            if (brd) ERRORF(r, "Should *not* create BRD from %s", rec.name);
        }
    }
}
#endif // SK_ENABLE_ANDROID_UTILS
