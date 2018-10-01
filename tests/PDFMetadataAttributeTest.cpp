/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCanvas.h"
#include "SkData.h"
#include "SkPDFDocument.h"
#include "SkStream.h"
#include "Test.h"

DEF_TEST(SkPDF_Metadata, r) {
    REQUIRE_PDF_DOCUMENT(SkPDF_Metadata, r);
    SkTime::DateTime now;
    SkTime::GetDateTime(&now);
    SkPDF::Metadata metadata;
    metadata.fTitle = "A1";
    metadata.fAuthor = "A2";
    metadata.fSubject = "A3";
    metadata.fKeywords = "A4";
    metadata.fCreator = "A5";
    metadata.fCreation = now;
    metadata.fModified = now;

    SkDynamicMemoryWStream pdf;
    sk_sp<SkDocument> doc = SkPDF::MakeDocument(&pdf, metadata);
    doc->beginPage(612.0f, 792.0f);
    doc->close();
    sk_sp<SkData> data = pdf.detachAsData();
    static const char* expectations[] = {
        "/Title (A1)",
        "/Author (A2)",
        "/Subject (A3)",
        "/Keywords (A4)",
        "/Creator (A5)",
        "/Producer (Skia/PDF ",
        "/CreationDate (D:",
        "/ModDate (D:"
    };
    const uint8_t* bytes = data->bytes();
    for (const char* expectation : expectations) {
        size_t len = strlen(expectation);
        bool found = false;
        size_t N = 1 + data->size() - len;
        for (size_t i = 0; i < N; ++i) {
            if (0 == memcmp(bytes + i, expectation, len)) {
                found = true;
                break;
            }
        }
        if (!found) {
            ERRORF(r, "expectation missing: '%s'.", expectation);
        }
    }
}

static SkMatrix rotation_matrix(SkPDF::Rotation rot, SkSize size) {
    SkMatrix matrix;
    SkScalar x = 0.5f * size.width(),
             y = 0.5f * size.height(),
             r = -90.0f * (uint8_t)rot;
    switch (rot) {
        case SkPDF::Rotation::kPortrait:          matrix.setIdentity();      break;
        case SkPDF::Rotation::kLandscape:         matrix.setRotate(r, y, y); break;
        case SkPDF::Rotation::kInvertedPortrait:  matrix.setRotate(r, x, y); break;
        case SkPDF::Rotation::kInvertedLandscape: matrix.setRotate(r, x, x); break;
    }
    return matrix;
}

DEF_TEST(SkPDF_Rotation, r) {
    REQUIRE_PDF_DOCUMENT(SkPDF_Rotation, r);
    constexpr float W = 8.5f * 72;
    constexpr float H = 11   * 72;
    constexpr float M = 0.25 * 72;
    SkPDF::Rotation rotations[] = {
        SkPDF::Rotation::kPortrait,
        SkPDF::Rotation::kLandscape,
        SkPDF::Rotation::kInvertedPortrait,
        SkPDF::Rotation::kInvertedLandscape,
    };
    // extra_cflags = [ "-DSK_PDF_TEST_ROTATION=\"/tmp/skpdf_test_rotation.pdf\"" ]
    #ifdef SK_PDF_TEST_ROTATION
    SkFILEWStream pdf(SK_PDF_TEST_ROTATION);
    #else
    SkDynamicMemoryWStream pdf;
    #endif
    sk_sp<SkDocument> doc = SkPDF::MakeDocument(&pdf);
    for (auto rotation : rotations) {
        SkCanvas* canvas = doc->beginPage(W, H);
        //SkPDF::SetCropBoxForPage(canvas, SkRect::MakeWH(W, H).makeInset(M / 2, M / 2));
        SkPDF::SetRotationForPage(canvas, rotation);
        // SetRotation just changes how a page is displayed on a screen.  To
        // draw rotated text, clients should apply a matrix to the canvas to
        // "rotate it back":
        canvas->concat(rotation_matrix(rotation, {W, H})); // draw in expected way.

        // Some test content.  The rectangle should be centered, it should be
        // clipped to the edge of the page, the text should readable upright in
        // a viewer.
        SkPaint paint;
        SkString text = SkStringPrintf("rotation = %d", (int)rotation);
        canvas->drawString(text, 2 * M, 2 * M + paint.getFontSpacing(), paint);
        SkSize size = (uint8_t)rotation % 2 == 0 ? SkSize{W, H} : SkSize{H, W};
        SkRect rect = SkRect::MakeSize(size).makeInset(M, M);
        SkPath area;
        area.addRect(rect);
        area.toggleInverseFillType();
        SkPaint gray;
        gray.setColor(SK_ColorLTGRAY);
        canvas->drawPath(area, gray);
    }
}

