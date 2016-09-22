/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkAnnotation.h"
#include "SkCanvas.h"
#include "SkData.h"
#include "SkDocument.h"
#include "SkStream.h"
#include "Test.h"

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
    bm.allocN32Pixels(10, 10);
    bm.eraseColor(SK_ColorTRANSPARENT);

    SkCanvas canvas(bm);
    SkRect r = SkRect::MakeWH(SkIntToScalar(10), SkIntToScalar(10));

    sk_sp<SkData> data(SkData::MakeWithCString("http://www.gooogle.com"));

    REPORTER_ASSERT(reporter, 0 == *bm.getAddr32(0, 0));
    SkAnnotateRectWithURL(&canvas, r, data.get());
    REPORTER_ASSERT(reporter, 0 == *bm.getAddr32(0, 0));
}

DEF_TEST(Annotation_PdfLink, reporter) {
    REQUIRE_PDF_DOCUMENT(Annotation_PdfLink, reporter);
    SkDynamicMemoryWStream outStream;
    sk_sp<SkDocument> doc(SkDocument::MakePDF(&outStream));
    SkCanvas* canvas = doc->beginPage(612.0f, 792.0f);
    REPORTER_ASSERT(reporter, canvas);

    SkRect r = SkRect::MakeXYWH(SkIntToScalar(72), SkIntToScalar(72),
                                SkIntToScalar(288), SkIntToScalar(72));
    sk_sp<SkData> data(SkData::MakeWithCString("http://www.gooogle.com"));
    SkAnnotateRectWithURL(canvas, r, data.get());

    doc->close();
    sk_sp<SkData> out = outStream.detachAsData();
    const char* rawOutput = (const char*)out->data();

    REPORTER_ASSERT(reporter, ContainsString(rawOutput, out->size(), "/Annots "));
}

DEF_TEST(Annotation_NamedDestination, reporter) {
    REQUIRE_PDF_DOCUMENT(Annotation_NamedDestination, reporter);
    SkDynamicMemoryWStream outStream;
    sk_sp<SkDocument> doc(SkDocument::MakePDF(&outStream));
    SkCanvas* canvas = doc->beginPage(612.0f, 792.0f);
    REPORTER_ASSERT(reporter, canvas);

    SkPoint p = SkPoint::Make(SkIntToScalar(72), SkIntToScalar(72));
    sk_sp<SkData> data(SkData::MakeWithCString("example"));
    SkAnnotateNamedDestination(canvas, p, data.get());

    doc->close();
    sk_sp<SkData> out = outStream.detachAsData();
    const char* rawOutput = (const char*)out->data();

    REPORTER_ASSERT(reporter,
        ContainsString(rawOutput, out->size(), "/example "));
}
