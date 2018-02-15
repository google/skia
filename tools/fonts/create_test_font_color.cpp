/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// running create_test_font_color generates ./tools/<cbdt|sbix|cpal>.ttx
// which are read by fonttools ttx to produce native fonts.

#include "Resources.h"
#include "SkFontStyle.h"
#include "SkOSFile.h"
#include "SkOSPath.h"
#include "SkPaint.h"
#include "SkPath.h"
#include "SkStream.h"
#include "SkTArray.h"
#include "SkTSort.h"
#include "SkTestScalerContext.h"
#include "SkTypeface.h"
#include "SkUtils.h"
#include <stdio.h>


int main(int , char * const []) {
    sk_sp<SkSVGTestTypeface> typeface = SkSVGTestTypeface::Default();
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
