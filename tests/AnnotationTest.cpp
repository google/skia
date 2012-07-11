
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "Test.h"
#include "SkAnnotation.h"
#include "SkData.h"
#include "SkCanvas.h"

static void test_nodraw(skiatest::Reporter* reporter) {
    SkBitmap bm;
    bm.setConfig(SkBitmap::kARGB_8888_Config, 10, 10);
    bm.allocPixels();
    bm.eraseColor(0);

    SkCanvas canvas(bm);
    SkRect r = SkRect::MakeWH(SkIntToScalar(10), SkIntToScalar(10));

    SkAutoDataUnref data(SkData::NewWithCString("http://www.gooogle.com"));

    REPORTER_ASSERT(reporter, 0 == *bm.getAddr32(0, 0));
    SkAnnotateRectWithURL(&canvas, r, data.get());
    REPORTER_ASSERT(reporter, 0 == *bm.getAddr32(0, 0));
}

static void TestAnnotation(skiatest::Reporter* reporter) {
    test_nodraw(reporter);
}

#include "TestClassDef.h"
DEFINE_TESTCLASS("Annotation", AnnotationClass, TestAnnotation)
