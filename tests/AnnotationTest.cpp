/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkAnnotation.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkData.h"
#include "include/core/SkDocument.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkStream.h"
#include "include/core/SkTypes.h"
#include "include/docs/SkPDFDocument.h"
#include "tests/Test.h"

#include <string.h>
#include <memory>

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
    auto doc = SkPDF::MakeDocument(&outStream);
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

DEF_TEST(Annotation_PdfDefineNamedDestination, reporter) {
    REQUIRE_PDF_DOCUMENT(Annotation_PdfNamedDestination, reporter);
    SkDynamicMemoryWStream outStream;
    auto doc = SkPDF::MakeDocument(&outStream);
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

#if defined(SK_XML)
    #include "include/svg/SkSVGCanvas.h"

    DEF_TEST(Annotation_SvgLink, reporter) {
        SkDynamicMemoryWStream outStream;
        SkRect bounds = SkRect::MakeIWH(400, 400);
        std::unique_ptr<SkCanvas> canvas = SkSVGCanvas::Make(bounds, &outStream);

        SkRect r = SkRect::MakeXYWH(SkIntToScalar(72), SkIntToScalar(72), SkIntToScalar(288),
                                    SkIntToScalar(72));
        sk_sp<SkData> data(SkData::MakeWithCString("http://www.gooogle.com"));
        SkAnnotateRectWithURL(canvas.get(), r, data.get());

        sk_sp<SkData> out = outStream.detachAsData();
        const char* rawOutput = (const char*)out->data();

        REPORTER_ASSERT(reporter,
            ContainsString(rawOutput, out->size(), "a xlink:href=\"http://www.gooogle.com\""));
    }

    DEF_TEST(Annotation_SvgLinkNamedDestination, reporter) {
        SkDynamicMemoryWStream outStream;
        SkRect bounds = SkRect::MakeIWH(400, 400);
        std::unique_ptr<SkCanvas> canvas = SkSVGCanvas::Make(bounds, &outStream);

        SkRect r = SkRect::MakeXYWH(SkIntToScalar(72), SkIntToScalar(72), SkIntToScalar(288),
                                    SkIntToScalar(72));
        sk_sp<SkData> data(SkData::MakeWithCString("http://www.gooogle.com/#NamedDestination"));
        SkAnnotateLinkToDestination(canvas.get(), r, data.get());

        sk_sp<SkData> out = outStream.detachAsData();
        const char* rawOutput = (const char*)out->data();

        REPORTER_ASSERT(reporter,
            ContainsString(rawOutput, out->size(),
                           "a xlink:href=\"http://www.gooogle.com/#NamedDestination\""));
    }
#endif
