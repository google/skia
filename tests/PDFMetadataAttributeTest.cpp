/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkDocument.h"
#include "SkStream.h"
#include "SkData.h"
#include "Test.h"

DEF_TEST(SkPDF_Metadata, r) {
    REQUIRE_PDF_DOCUMENT(SkPDF_Metadata, r);
    SkTime::DateTime now;
    SkTime::GetDateTime(&now);
    SkDocument::PDFMetadata metadata;
    metadata.fTitle = "A1";
    metadata.fAuthor = "A2";
    metadata.fSubject = "A3";
    metadata.fKeywords = "A4";
    metadata.fCreator = "A5";
    metadata.fCreation.fEnabled = true;
    metadata.fCreation.fDateTime = now;
    metadata.fModified.fEnabled = true;
    metadata.fModified.fDateTime = now;

    SkDynamicMemoryWStream pdf;
    sk_sp<SkDocument> doc = SkDocument::MakePDF(&pdf, SK_ScalarDefaultRasterDPI,
                                                metadata, nullptr, false);
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
