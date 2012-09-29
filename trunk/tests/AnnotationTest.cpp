
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
#include "SkPDFDevice.h"
#include "SkPDFDocument.h"

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

struct testCase {
    SkPDFDocument::Flags flags;
    bool expectAnnotations;
};

static void test_pdf_link_annotations(skiatest::Reporter* reporter) {
    SkISize size = SkISize::Make(612, 792);
    SkMatrix initialTransform;
    initialTransform.reset();
    SkPDFDevice device(size, size, initialTransform);
    SkCanvas canvas(&device);

    SkRect r = SkRect::MakeXYWH(SkIntToScalar(72), SkIntToScalar(72),
                                SkIntToScalar(288), SkIntToScalar(72));
    SkAutoDataUnref data(SkData::NewWithCString("http://www.gooogle.com"));
    SkAnnotateRectWithURL(&canvas, r, data.get());

    testCase tests[] = {{(SkPDFDocument::Flags)0, true},
                        {SkPDFDocument::kNoLinks_Flags, false}};
    for (size_t testNum = 0; testNum < SK_ARRAY_COUNT(tests); testNum++) {
        SkPDFDocument doc(tests[testNum].flags);
        doc.appendPage(&device);
        SkDynamicMemoryWStream outStream;
        doc.emitPDF(&outStream);
        SkAutoDataUnref out(outStream.copyToData());
        const char* rawOutput = (const char*)out->data();

        bool found = false;
        for (size_t i = 0; i < out->size() - 8; i++) {
            if (rawOutput[i + 0] == '/' &&
                rawOutput[i + 1] == 'A' &&
                rawOutput[i + 2] == 'n' &&
                rawOutput[i + 3] == 'n' &&
                rawOutput[i + 4] == 'o' &&
                rawOutput[i + 5] == 't' &&
                rawOutput[i + 6] == 's' &&
                rawOutput[i + 7] == ' ') {
                found = true;
                break;
            }
        }
        REPORTER_ASSERT(reporter, found == tests[testNum].expectAnnotations);
    }
}

static void TestAnnotation(skiatest::Reporter* reporter) {
    test_nodraw(reporter);
    test_pdf_link_annotations(reporter);
}

#include "TestClassDef.h"
DEFINE_TESTCLASS("Annotation", AnnotationClass, TestAnnotation)
