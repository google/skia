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
#include "SkStream.h"
#include "SkPixelSerializer.h"

static void test_empty(skiatest::Reporter* reporter) {
    SkDynamicMemoryWStream stream;

    SkAutoTUnref<SkDocument> doc(SkDocument::CreatePDF(&stream));

    doc->close();

    REPORTER_ASSERT(reporter, stream.bytesWritten() == 0);
}

static void test_abort(skiatest::Reporter* reporter) {
    SkDynamicMemoryWStream stream;
    SkAutoTUnref<SkDocument> doc(SkDocument::CreatePDF(&stream));

    SkCanvas* canvas = doc->beginPage(100, 100);
    canvas->drawColor(SK_ColorRED);
    doc->endPage();

    doc->abort();

    REPORTER_ASSERT(reporter, stream.bytesWritten() == 0);
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
        SkAutoTUnref<SkDocument> doc(SkDocument::CreatePDF(path.c_str()));

        SkCanvas* canvas = doc->beginPage(100, 100);
        canvas->drawColor(SK_ColorRED);
        doc->endPage();

        doc->abort();
    }

    FILE* file = fopen(path.c_str(), "r");
    // The created file should be empty.
    char buffer[100];
    REPORTER_ASSERT(reporter, fread(buffer, 1, 1, file) == 0);
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

    SkAutoTUnref<SkDocument> doc(SkDocument::CreatePDF(path.c_str()));

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
    SkAutoTUnref<SkDocument> doc(SkDocument::CreatePDF(&stream));

    SkCanvas* canvas = doc->beginPage(100, 100);
    canvas->drawColor(SK_ColorRED);
    doc->endPage();

    doc->close();

    REPORTER_ASSERT(reporter, stream.bytesWritten() != 0);
}

DEF_TEST(document_tests, reporter) {
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
        SkBitmap bm;
        return bm.installPixels(pixmap.info(),
                                pixmap.writable_addr(),
                                pixmap.rowBytes(),
                                pixmap.ctable(),
                                nullptr, nullptr)
            ? SkImageEncoder::EncodeData(bm, SkImageEncoder::kJPEG_Type, 85)
            : nullptr;
    }
};
}  // namespace

size_t count_bytes(const SkBitmap& bm, bool useDCT) {
    SkDynamicMemoryWStream stream;
    SkAutoTUnref<SkDocument> doc;
    if (useDCT) {
        SkAutoTUnref<SkPixelSerializer> serializer(new JPEGSerializer);
        doc.reset(SkDocument::CreatePDF(
                          &stream, SK_ScalarDefaultRasterDPI, serializer));
    } else {
        doc.reset(SkDocument::CreatePDF(&stream));
    }
    SkCanvas* canvas = doc->beginPage(64, 64);
    canvas->drawBitmap(bm, 0, 0);
    doc->endPage();
    doc->close();
    return stream.bytesWritten();
}

DEF_TEST(document_dct_encoder, r) {
    REQUIRE_PDF_DOCUMENT(document_dct_encoder, r);
    SkBitmap bm;
    if (GetResourceAsBitmap("mandrill_64.png", &bm)) {
        // Lossy encoding works better on photographs.
        REPORTER_ASSERT(r, count_bytes(bm, true) < count_bytes(bm, false));
    }
}

DEF_TEST(document_skbug_4734, r) {
    REQUIRE_PDF_DOCUMENT(document_skbug_4734, r);
    SkDynamicMemoryWStream stream;
    SkAutoTUnref<SkDocument> doc(SkDocument::CreatePDF(&stream));
    SkCanvas* canvas = doc->beginPage(64, 64);
    canvas->scale(10000.0f, 10000.0f);
    canvas->translate(20.0f, 10.0f);
    canvas->rotate(30.0f);
    const char text[] = "HELLO";
    canvas->drawText(text, strlen(text), 0, 0, SkPaint());
}
