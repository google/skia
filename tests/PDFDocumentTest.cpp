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
#include "SkPixelSerializer.h"

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
        return;  // TODO(edisonn): unfortunatelly this pattern is used in other
                 // tests, but if GetTmpDir() starts returning and empty dir
                 // allways, then all these tests will be disabled.
    }

    SkString path = SkOSPath::Join(tmpDir.c_str(), "aborted.pdf");

    // Make sure doc's destructor is called to flush.
    {
        sk_sp<SkDocument> doc(SkDocument::MakePDF(path.c_str()));

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
        return;  // TODO(edisonn): unfortunatelly this pattern is used in other
                 // tests, but if GetTmpDir() starts returning and empty dir
                 // allways, then all these tests will be disabled.
    }

    SkString path = SkOSPath::Join(tmpDir.c_str(), "file.pdf");

    sk_sp<SkDocument> doc(SkDocument::MakePDF(path.c_str()));

    SkCanvas* canvas = doc->beginPage(100, 100);

    canvas->drawColor(SK_ColorRED);
    doc->endPage();
    doc->close();

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

namespace {
class JPEGSerializer final : public SkPixelSerializer {
    bool onUseEncodedData(const void*, size_t) override { return true; }
    SkData* onEncode(const SkPixmap& pixmap) override {
        return sk_tool_utils::EncodeImageToData(pixmap, SkEncodedImageFormat::kJPEG, 85).release();
    }
};
}  // namespace

size_t count_bytes(const SkBitmap& bm, bool useDCT) {
    SkDynamicMemoryWStream stream;
    sk_sp<SkDocument> doc;
    if (useDCT) {
        doc = SkDocument::MakePDF(&stream, SK_ScalarDefaultRasterDPI,
                                  SkDocument::PDFMetadata(),
                                  sk_make_sp<JPEGSerializer>(), false);
    } else {
        doc = SkDocument::MakePDF(&stream);
    }
    SkCanvas* canvas = doc->beginPage(64, 64);
    canvas->drawBitmap(bm, 0, 0);
    doc->endPage();
    doc->close();
    return stream.bytesWritten();
}

DEF_TEST(SkPDF_document_dct_encoder, r) {
    REQUIRE_PDF_DOCUMENT(SkPDF_document_dct_encoder, r);
    SkBitmap bm;
    if (GetResourceAsBitmap("mandrill_64.png", &bm)) {
        // Lossy encoding works better on photographs.
        REPORTER_ASSERT(r, count_bytes(bm, true) < count_bytes(bm, false));
    }
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

    SkDynamicMemoryWStream buffer;
    auto doc = SkDocument::MakePDF(&buffer, SK_ScalarDefaultRasterDPI,
                                   pdfMetadata, nullptr, /* pdfa = */ true);
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
    doc = SkDocument::MakePDF(&buffer, SK_ScalarDefaultRasterDPI,
                              pdfMetadata, nullptr, /* pdfa = */ true);
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
