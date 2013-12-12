
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "Test.h"
#include "TestClassDef.h"
#include "SkAnnotation.h"
#include "SkData.h"
#include "SkCanvas.h"
#include "SkPDFDevice.h"
#include "SkPDFDocument.h"

/** Returns true if data (may contain null characters) contains needle (null
 *  terminated). */
static bool ContainsString(const char* data, size_t dataSize, const char* needle) {
    size_t nSize = strlen(needle);
    for (size_t i = 0; i < dataSize - nSize; i++) {
        if (strncmp(&data[i], needle, nSize) == 0) {
            return true;
        }
    }
    return false;
}

DEF_TEST(Annotation_NoDraw, reporter) {
    SkBitmap bm;
    bm.setConfig(SkBitmap::kARGB_8888_Config, 10, 10);
    bm.allocPixels();
    bm.eraseColor(SK_ColorTRANSPARENT);

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

DEF_TEST(Annotation_PdfLink, reporter) {
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

        REPORTER_ASSERT(reporter,
            ContainsString(rawOutput, out->size(), "/Annots ")
            == tests[testNum].expectAnnotations);
    }
}

DEF_TEST(Annotation_NamedDestination, reporter) {
    SkISize size = SkISize::Make(612, 792);
    SkMatrix initialTransform;
    initialTransform.reset();
    SkPDFDevice device(size, size, initialTransform);
    SkCanvas canvas(&device);

    SkPoint p = SkPoint::Make(SkIntToScalar(72), SkIntToScalar(72));
    SkAutoDataUnref data(SkData::NewWithCString("example"));
    SkAnnotateNamedDestination(&canvas, p, data.get());

    SkPDFDocument doc;
    doc.appendPage(&device);
    SkDynamicMemoryWStream outStream;
    doc.emitPDF(&outStream);
    SkAutoDataUnref out(outStream.copyToData());
    const char* rawOutput = (const char*)out->data();

    REPORTER_ASSERT(reporter,
        ContainsString(rawOutput, out->size(), "/example "));
}
