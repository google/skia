/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "Test.h"

#include "Resources.h"
#include "SkCanvas.h"
#include "SkDocument.h"
#include "SkOSFile.h"
#include "SkOSPath.h"
#include "SkStream.h"

#include "sk_tool_utils.h"

static void test_empty(skiatest::Reporter* reporter) {
    SkDynamicMemoryWStream stream;

    sk_sp<SkDocument> doc(SkDocument::MakePDF(&stream));

    doc->close();

    REPORTER_ASSERT(reporter, stream.bytesWritten() == 0);
}

static void test_abort(skiatest::Reporter* reporter) {
    SkDynamicMemoryWStream stream;
    sk_sp<SkDocument> doc(SkDocument::MakePDF(&stream));

    SkCanvas* canvas = doc->beginPage(100, 100);
    canvas->drawColor(SK_ColorRED);
    doc->endPage();

    doc->abort();

    // Test that only the header is written, not the full document.
    REPORTER_ASSERT(reporter, stream.bytesWritten() < 256);
}

static void test_abortWithFile(skiatest::Reporter* reporter) {
    SkString tmpDir = skiatest::GetTmpDir();

    if (tmpDir.isEmpty()) {
        ERRORF(reporter, "missing tmpDir.");
        return;
    }

    SkString path = SkOSPath::Join(tmpDir.c_str(), "aborted.pdf");
    if (!SkFILEWStream(path.c_str()).isValid()) {
        ERRORF(reporter, "unable to write to: %s", path.c_str());
        return;
    }

    // Make sure doc's destructor is called to flush.
    {
        SkFILEWStream stream(path.c_str());
        sk_sp<SkDocument> doc = SkDocument::MakePDF(&stream);

        SkCanvas* canvas = doc->beginPage(100, 100);
        canvas->drawColor(SK_ColorRED);
        doc->endPage();

        doc->abort();
    }

    FILE* file = fopen(path.c_str(), "r");
    // Test that only the header is written, not the full document.
    char buffer[256];
    REPORTER_ASSERT(reporter, fread(buffer, 1, sizeof(buffer), file) < sizeof(buffer));
    fclose(file);
}

static void test_file(skiatest::Reporter* reporter) {
    SkString tmpDir = skiatest::GetTmpDir();
    if (tmpDir.isEmpty()) {
        ERRORF(reporter, "missing tmpDir.");
        return;
    }

    SkString path = SkOSPath::Join(tmpDir.c_str(), "file.pdf");
    if (!SkFILEWStream(path.c_str()).isValid()) {
        ERRORF(reporter, "unable to write to: %s", path.c_str());
        return;
    }

    {
        SkFILEWStream stream(path.c_str());
        sk_sp<SkDocument> doc = SkDocument::MakePDF(&stream);
        SkCanvas* canvas = doc->beginPage(100, 100);

        canvas->drawColor(SK_ColorRED);
        doc->endPage();
        doc->close();
    }

    FILE* file = fopen(path.c_str(), "r");
    REPORTER_ASSERT(reporter, file != nullptr);
    char header[100];
    REPORTER_ASSERT(reporter, fread(header, 4, 1, file) != 0);
    REPORTER_ASSERT(reporter, strncmp(header, "%PDF", 4) == 0);
    fclose(file);
}

static void test_close(skiatest::Reporter* reporter) {
    SkDynamicMemoryWStream stream;
    sk_sp<SkDocument> doc(SkDocument::MakePDF(&stream));

    SkCanvas* canvas = doc->beginPage(100, 100);
    canvas->drawColor(SK_ColorRED);
    doc->endPage();

    doc->close();

    REPORTER_ASSERT(reporter, stream.bytesWritten() != 0);
}

DEF_TEST(SkPDF_document_tests, reporter) {
    REQUIRE_PDF_DOCUMENT(document_tests, reporter);
    test_empty(reporter);
    test_abort(reporter);
    test_abortWithFile(reporter);
    test_file(reporter);
    test_close(reporter);
}

DEF_TEST(SkPDF_document_skbug_4734, r) {
    REQUIRE_PDF_DOCUMENT(SkPDF_document_skbug_4734, r);
    SkDynamicMemoryWStream stream;
    sk_sp<SkDocument> doc(SkDocument::MakePDF(&stream));
    SkCanvas* canvas = doc->beginPage(64, 64);
    canvas->scale(10000.0f, 10000.0f);
    canvas->translate(20.0f, 10.0f);
    canvas->rotate(30.0f);
    const char text[] = "HELLO";
    canvas->drawString(text, 0, 0, SkPaint());
}

static bool contains(const uint8_t* result, size_t size, const char expectation[]) {
    size_t len = strlen(expectation);
    size_t N = 1 + size - len;
    for (size_t i = 0; i < N; ++i) {
        if (0 == memcmp(result + i, expectation, len)) {
            return true;
        }
    }
    return false;
}

// verify that the PDFA flag does something.
DEF_TEST(SkPDF_pdfa_document, r) {
    REQUIRE_PDF_DOCUMENT(SkPDF_pdfa_document, r);

    SkDocument::PDFMetadata pdfMetadata;
    pdfMetadata.fTitle = "test document";
    pdfMetadata.fCreation.fEnabled = true;
    pdfMetadata.fCreation.fDateTime = {0, 1999, 12, 5, 31, 23, 59, 59};
    pdfMetadata.fPDFA = true;

    SkDynamicMemoryWStream buffer;
    auto doc = SkDocument::MakePDF(&buffer, pdfMetadata);
    doc->beginPage(64, 64)->drawColor(SK_ColorRED);
    doc->close();
    sk_sp<SkData> data(buffer.detachAsData());

    static const char* expectations[] = {
        "sRGB IEC61966-2.1",
        "<dc:title><rdf:Alt><rdf:li xml:lang=\"x-default\">test document",
        "<xmp:CreateDate>1999-12-31T23:59:59+00:00</xmp:CreateDate>",
        "/Subtype /XML",
        "/CreationDate (D:19991231235959+00'00')>>",
    };
    for (const char* expectation : expectations) {
        if (!contains(data->bytes(), data->size(), expectation)) {
            ERRORF(r, "PDFA expectation missing: '%s'.", expectation);
        }
    }
    pdfMetadata.fProducer = "phoney library";
    pdfMetadata.fPDFA = true;
    doc = SkDocument::MakePDF(&buffer, pdfMetadata);
    doc->beginPage(64, 64)->drawColor(SK_ColorRED);
    doc->close();
    data = buffer.detachAsData();

    static const char* moreExpectations[] = {
        "/Producer (phoney library)",
        "/ProductionLibrary (Skia/PDF m",
        "<!-- <skia:ProductionLibrary>Skia/PDF m",
        "<pdf:Producer>phoney library</pdf:Producer>",
    };
    for (const char* expectation : moreExpectations) {
        if (!contains(data->bytes(), data->size(), expectation)) {
            ERRORF(r, "PDFA expectation missing: '%s'.", expectation);
        }
    }
}
