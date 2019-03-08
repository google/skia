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
#include "SkString.h"
#include "SkTestSVGTypeface.h"

static void export_ttx(sk_sp<SkTestSVGTypeface> typeface, SkString prefix) {
    SkFILEWStream cbdt((SkString(prefix) += "cbdt.ttx").c_str());
    typeface->exportTtxCbdt(&cbdt);
    cbdt.flush();
    cbdt.fsync();

    SkFILEWStream sbix((SkString(prefix) += "sbix.ttx").c_str());
    typeface->exportTtxSbix(&sbix);
    sbix.flush();
    sbix.fsync();

    SkFILEWStream colr((SkString(prefix) += "colr.ttx").c_str());
    typeface->exportTtxColr(&colr);
    colr.flush();
    colr.fsync();
}

int main(int argc, char** argv) {
    SkCommandLineFlags::Parse(argc, argv);

    export_ttx(SkTestSVGTypeface::Default(), SkString());
    export_ttx(SkTestSVGTypeface::Planets(), SkString("planet"));

    return 0;
}
