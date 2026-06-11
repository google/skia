/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// running create_test_font_color generates ./<cbdt|cbdt_fmt*|ebdt_fmt*|sbix|cpal>.ttx
// which are read by fonttools ttx to produce native fonts.
// cbdt.ttx produces PNG format (17) bitmap fonts.
// cbdt_fmt{1,2,6,7}.ttx produce non-PNG alpha bitmap fonts.
// ebdt_fmt{1,2,6,7}.ttx produce monochrome embedded bitmap fonts.

#include "include/core/SkRefCnt.h"
#include "include/core/SkStream.h"
#include "include/core/SkString.h"
#include "tools/flags/CommandLineFlags.h"
#include "tools/fonts/TestSVGTypeface.h"

#if defined(SK_ENABLE_SVG)

static void export_ttx(sk_sp<TestSVGTypeface> typeface,
                       SkString               prefix,
                       SkSpan<unsigned>       cbdtStrikeSizes,
                       SkSpan<unsigned>       sbixStrikeSizes) {
    SkFILEWStream cbdt((SkString(prefix) += "cbdt.ttx").c_str());
    typeface->exportTtxCbdt(&cbdt, cbdtStrikeSizes);
    cbdt.flush();
    cbdt.fsync();

    // Non-PNG bitmap data formats: 1 (byte-aligned Small), 2 (bit-aligned
    // Small), 6 (byte-aligned Big), 7 (bit-aligned Big).
    constexpr TestSVGTypeface::BitmapDataFormat kBitmapFormats[] = {
        TestSVGTypeface::BitmapDataFormat::kSmallByteAligned,
        TestSVGTypeface::BitmapDataFormat::kSmallBitAligned,
        TestSVGTypeface::BitmapDataFormat::kBigByteAligned,
        TestSVGTypeface::BitmapDataFormat::kBigBitAligned,
    };
    for (TestSVGTypeface::BitmapDataFormat fmt : kBitmapFormats) {
        SkString fname(prefix);
        fname += "cbdt_fmt";
        fname.appendS32(static_cast<int>(fmt));
        fname += ".ttx";
        SkFILEWStream cbdtAlpha(fname.c_str());
        typeface->exportTtxCbdtAlpha(&cbdtAlpha, cbdtStrikeSizes, fmt);
        cbdtAlpha.flush();
        cbdtAlpha.fsync();
    }

    for (TestSVGTypeface::BitmapDataFormat fmt : kBitmapFormats) {
        SkString fname(prefix);
        fname += "ebdt_fmt";
        fname.appendS32(static_cast<int>(fmt));
        fname += ".ttx";
        SkFILEWStream ebdt(fname.c_str());
        typeface->exportTtxEbdt(&ebdt, cbdtStrikeSizes, fmt);
        ebdt.flush();
        ebdt.fsync();
    }

    SkFILEWStream sbix((SkString(prefix) += "sbix.ttx").c_str());
    typeface->exportTtxSbix(&sbix, sbixStrikeSizes);
    sbix.flush();
    sbix.fsync();

    SkFILEWStream colr((SkString(prefix) += "colr.ttx").c_str());
    typeface->exportTtxColr(&colr);
    colr.flush();
    colr.fsync();
}

int main(int argc, char** argv) {
    CommandLineFlags::Parse(argc, argv);

    // Most of the time use these sizes.
    unsigned usual[] = { 16, 64, 128 };

    // But the planet font cannot get very big in the size limited cbdt format.
    unsigned small[] = { 8, 16 };

    export_ttx(TestSVGTypeface::Default(), SkString(), SkSpan(usual), SkSpan(usual));
    export_ttx(TestSVGTypeface::Planets(), SkString("planet"), SkSpan(small), SkSpan(usual));

    return 0;
}

#else

int main(int argc, char** argv) {
    SkDebugf("compile with SVG enabled\n");
    return 1;
}

#endif // SK_ENABLE_SVG
