/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkImageInfo.h"
#include "src/core/SkReadPixelsRec.h"
#include "src/core/SkWritePixelsRec.h"
#include "tests/Test.h"

#include <climits>
#include <cstddef>
#include <cstdint>
#include <vector>

DEF_TEST(ReadPixelsRec_trim, reporter) {
    constexpr int W = 100;
    constexpr int H = 100;
    const SkImageInfo info = SkImageInfo::MakeN32Premul(W, H);
    const size_t rowBytes = info.minRowBytes();
    std::vector<char> storage(H * rowBytes);
    void* pixels = storage.data();

    {
        skiatest::ReporterContext ctx(reporter, "Normal valid trim");
        SkReadPixelsRec rec(info, pixels, rowBytes, 0, 0);
        REPORTER_ASSERT(reporter, rec.trim(W, H));
        REPORTER_ASSERT(reporter, rec.fPixels == pixels);
        REPORTER_ASSERT(reporter, rec.fInfo.width() == W);
        REPORTER_ASSERT(reporter, rec.fInfo.height() == H);
    }

    {
        skiatest::ReporterContext ctx(reporter, "Trim with negative x, y");
        SkReadPixelsRec rec(info, pixels, rowBytes, -10, -10);
        REPORTER_ASSERT(reporter, rec.trim(W, H));
        // fPixels should be adjusted: pixels + 10 * rowBytes + 10 * 4
        REPORTER_ASSERT(reporter, rec.fPixels ==
                                  (char*)pixels + 10 * rowBytes + 10 * info.bytesPerPixel());
        REPORTER_ASSERT(reporter, rec.fInfo.width() == W - 10);
        REPORTER_ASSERT(reporter, rec.fInfo.height() == H - 10);
        REPORTER_ASSERT(reporter, rec.fX == 0);
        REPORTER_ASSERT(reporter, rec.fY == 0);
    }

    {
        skiatest::ReporterContext ctx(reporter, "Trim with x, y partially outside");
        SkReadPixelsRec rec(info, pixels, rowBytes, 50, 50);
        REPORTER_ASSERT(reporter, rec.trim(W, H));
        REPORTER_ASSERT(reporter, rec.fPixels == pixels);
        REPORTER_ASSERT(reporter, rec.fInfo.width() == 50);
        REPORTER_ASSERT(reporter, rec.fInfo.height() == 50);
        REPORTER_ASSERT(reporter, rec.fX == 50);
        REPORTER_ASSERT(reporter, rec.fY == 50);
    }

    {
        skiatest::ReporterContext ctx(reporter, "Trim with x, y completely outside (positive)");
        SkReadPixelsRec rec(info, pixels, rowBytes, 150, 150);
        REPORTER_ASSERT(reporter, !rec.trim(W, H));
    }

    {
        skiatest::ReporterContext ctx(reporter, "Trim with x, y completely outside (negative)");
        SkReadPixelsRec rec(info, pixels, rowBytes, -150, -150);
        REPORTER_ASSERT(reporter, !rec.trim(W, H));
    }

    {
        skiatest::ReporterContext ctx(reporter, "Explicitly trigger y_offset overflow");
        // arbitrarily large value that will overflow when multiplied by hugeY
        size_t hugeRowBytes = static_cast<size_t>(INT_MAX) >> 13;
        int hugeY = INT_MIN;
        SkReadPixelsRec rec(info, pixels, hugeRowBytes, 0, hugeY);
        REPORTER_ASSERT(reporter, !rec.trim(W, H));
    }

    {
        if (sizeof(size_t) == 4) {
            skiatest::ReporterContext ctx(reporter, "Explicitly trigger x_offset overflow");
            int hugeX = INT_MIN;
            SkReadPixelsRec rec(info, pixels, rowBytes, hugeX, 0);
            REPORTER_ASSERT(reporter, !rec.trim(W, H));
        }
    }

    {
        skiatest::ReporterContext ctx(reporter, "Explicitly trigger total overflow");
        SkReadPixelsRec rec(info, pixels, SIZE_MAX, -1, -1);
        REPORTER_ASSERT(reporter, !rec.trim(W, H));
    }

    {
        skiatest::ReporterContext ctx(reporter, "Trim with large rowBytes (initial check)");
        SkImageInfo hugeInfo = SkImageInfo::MakeN32Premul(1 << 30, 1);
        SkReadPixelsRec rec(hugeInfo, pixels, 100, 0, 0);
        REPORTER_ASSERT(reporter, !rec.trim(W, H));
    }

    {
        skiatest::ReporterContext ctx(reporter, "Invalid ImageInfo (width <= 0)");
        SkReadPixelsRec rec(SkImageInfo::MakeN32Premul(-1, 100), pixels, rowBytes, 0, 0);
        REPORTER_ASSERT(reporter, !rec.trim(W, H));
    }

    {
        skiatest::ReporterContext ctx(reporter, "rowBytes too small");
        SkReadPixelsRec rec(info, pixels, info.minRowBytes() - 1, 0, 0);
        REPORTER_ASSERT(reporter, !rec.trim(W, H));
    }
}

DEF_TEST(WritePixelsRec_trim, reporter) {
    constexpr int W = 100;
    constexpr int H = 100;
    const SkImageInfo info = SkImageInfo::MakeN32Premul(W, H);
    const size_t rowBytes = info.minRowBytes();
    std::vector<char> storage(H * rowBytes);
    const void* pixels = storage.data();

    {
        skiatest::ReporterContext ctx(reporter, "Normal valid trim");
        SkWritePixelsRec rec(info, pixels, rowBytes, 0, 0);
        REPORTER_ASSERT(reporter, rec.trim(W, H));
        REPORTER_ASSERT(reporter, rec.fPixels == pixels);
        REPORTER_ASSERT(reporter, rec.fInfo.width() == W);
        REPORTER_ASSERT(reporter, rec.fInfo.height() == H);
    }

    {
        skiatest::ReporterContext ctx(reporter, "Trim with negative x, y");
        SkWritePixelsRec rec(info, pixels, rowBytes, -10, -10);
        REPORTER_ASSERT(reporter, rec.trim(W, H));
        // fPixels should be adjusted: pixels + 10 * rowBytes + 10 * 4
        REPORTER_ASSERT(reporter, rec.fPixels ==
                                  (const char*)pixels + 10 * rowBytes + 10 * info.bytesPerPixel());
        REPORTER_ASSERT(reporter, rec.fInfo.width() == W - 10);
        REPORTER_ASSERT(reporter, rec.fInfo.height() == H - 10);
        REPORTER_ASSERT(reporter, rec.fX == 0);
        REPORTER_ASSERT(reporter, rec.fY == 0);
    }

    {
        skiatest::ReporterContext ctx(reporter, "Trim with x, y partially outside");
        SkWritePixelsRec rec(info, pixels, rowBytes, 50, 50);
        REPORTER_ASSERT(reporter, rec.trim(W, H));
        REPORTER_ASSERT(reporter, rec.fPixels == pixels);
        REPORTER_ASSERT(reporter, rec.fInfo.width() == 50);
        REPORTER_ASSERT(reporter, rec.fInfo.height() == 50);
        REPORTER_ASSERT(reporter, rec.fX == 50);
        REPORTER_ASSERT(reporter, rec.fY == 50);
    }

    {
        skiatest::ReporterContext ctx(reporter, "Trim with x, y completely outside (positive)");
        SkWritePixelsRec rec(info, pixels, rowBytes, 150, 150);
        REPORTER_ASSERT(reporter, !rec.trim(W, H));
    }

    {
        skiatest::ReporterContext ctx(reporter, "Trim with x, y completely outside (negative)");
        SkWritePixelsRec rec(info, pixels, rowBytes, -150, -150);
        REPORTER_ASSERT(reporter, !rec.trim(W, H));
    }

    {
        skiatest::ReporterContext ctx(reporter, "Explicitly trigger y_offset overflow");
        // arbitrarily large value that will overflow when multiplied by hugeY
        size_t hugeRowBytes = static_cast<size_t>(INT_MAX) >> 13;
        int hugeY = INT_MIN;
        SkWritePixelsRec rec(info, pixels, hugeRowBytes, 0, hugeY);
        REPORTER_ASSERT(reporter, !rec.trim(W, H));
    }

    {
        if (sizeof(size_t) == 4) {
            skiatest::ReporterContext ctx(reporter, "Explicitly trigger x_offset overflow");
            int hugeX = INT_MIN;
            SkWritePixelsRec rec(info, pixels, rowBytes, hugeX, 0);
            REPORTER_ASSERT(reporter, !rec.trim(W, H));
        }
    }

    {
        skiatest::ReporterContext ctx(reporter, "Explicitly trigger total overflow");
        SkWritePixelsRec rec(info, pixels, SIZE_MAX, -1, -1);
        REPORTER_ASSERT(reporter, !rec.trim(W, H));
    }

    {
        skiatest::ReporterContext ctx(reporter, "Trim with large rowBytes (initial check)");
        SkImageInfo hugeInfo = SkImageInfo::MakeN32Premul(1 << 30, 1);
        SkWritePixelsRec rec(hugeInfo, pixels, 100, 0, 0);
        REPORTER_ASSERT(reporter, !rec.trim(W, H));
    }

    {
        skiatest::ReporterContext ctx(reporter, "Invalid ImageInfo (width <= 0)");
        SkWritePixelsRec rec(SkImageInfo::MakeN32Premul(-1, 100), pixels, rowBytes, 0, 0);
        REPORTER_ASSERT(reporter, !rec.trim(W, H));
    }

    {
        skiatest::ReporterContext ctx(reporter, "rowBytes too small");
        SkWritePixelsRec rec(info, pixels, info.minRowBytes() - 1, 0, 0);
        REPORTER_ASSERT(reporter, !rec.trim(W, H));
    }
}
