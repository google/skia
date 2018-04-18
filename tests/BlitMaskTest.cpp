/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBlitMask.h"
#include "SkColorPriv.h"
#include "SkMask.h"
#include "Test.h"

static void test_opaque_dest(skiatest::Reporter* reporter, SkMask::Format format) {
    const auto& row_proc = SkBlitMask::RowFactory(SkColorType::kN32_SkColorType, format,
                                                  static_cast<SkBlitMask::RowFlags>(0));

    SkPMColor src[256],
              dst[256];
    uint8_t    aa[256];

    // Coverage -> [0..255]
    for (size_t i = 0; i < 256; ++i) {
        aa[i] = static_cast<uint8_t>(i);
    }

    // src -> [0..255]
    for (size_t src_a = 0; src_a < 256; ++src_a) {
        memset(src, src_a, sizeof(src));

        // dst -> 0xff (always opaque)
        memset(dst, 0xff, sizeof(dst));

        row_proc(dst, aa, src, 256);

        for (size_t i = 0; i < 256; ++i) {
            REPORTER_ASSERT(reporter, SkGetPackedA32(dst[i]) == 0xff);
        }
    }
}

// Verifies that D32 dest remains opaque for any (src_alpha, coverage) combination.
DEF_TEST(BlitMask_OpaqueD32, reporter) {
    test_opaque_dest(reporter, SkMask::Format::kA8_Format);
}
