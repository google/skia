/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// running create_test_font_color generates ./<cbdt|sbix|cpal>.ttx
// which are read by fonttools ttx to produce native fonts.

#include "SkCommandLineFlags.h"
#include "SkRefCnt.h"
#include "SkStream.h"
#include "SkTestSVGTypeface.h"

int main(int argc, char** argv) {
    SkCommandLineFlags::Parse(argc, argv);

    sk_sp<SkTestSVGTypeface> typeface = SkTestSVGTypeface::Default();

    SkFILEWStream cbdt("cbdt.ttx");
    typeface->exportTtxCbdt(&cbdt);
    cbdt.flush();
    cbdt.fsync();

    SkFILEWStream sbix("sbix.ttx");
    typeface->exportTtxSbix(&sbix);
    sbix.flush();
    sbix.fsync();

    SkFILEWStream colr("colr.ttx");
    typeface->exportTtxColr(&colr);
    colr.flush();
    colr.fsync();

    return 0;
}
