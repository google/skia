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

DEF_TEST(SkPDF_MetadataAttribute, r) {
    REQUIRE_PDF_DOCUMENT(SkPDF_MetadataAttribute, r);
    SkDynamicMemoryWStream pdf;
    SkAutoTUnref<SkDocument> doc(SkDocument::CreatePDF(&pdf));
    SkTArray<SkDocument::Attribute> info;
    info.emplace_back(SkString("Title"), SkString("A1"));
    info.emplace_back(SkString("Author"), SkString("A2"));
    info.emplace_back(SkString("Subject"), SkString("A3"));
    info.emplace_back(SkString("Keywords"), SkString("A4"));
    info.emplace_back(SkString("Creator"), SkString("A5"));
    SkTime::DateTime now;
    SkTime::GetDateTime(&now);
    doc->setMetadata(&info[0], info.count(), &now, &now);
    doc->beginPage(612.0f, 792.0f);
    doc->close();
    SkAutoTUnref<SkData> data(pdf.copyToData());
    static const char* expectations[] = {
        "/Title (A1)",
        "/Author (A2)",
        "/Subject (A3)",
        "/Keywords (A4)",
        "/Creator (A5)",
        "/Producer (Skia/PDF)",
        "/CreationDate (D:",
        "/ModDate (D:"
    };
    for (const char* expectation : expectations) {
        bool found = false;
        size_t N = 1 + data->size() - strlen(expectation);
        for (size_t i = 0; i < N; ++i) {
            if (0 == memcmp(data->bytes() + i,
                             expectation, strlen(expectation))) {
                found = true;
                break;
            }
        }
        if (!found) {
            ERRORF(r, "expectation missing: '%s'.", expectation);
        }
    }
}
