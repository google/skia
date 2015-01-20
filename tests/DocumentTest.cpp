#include "Test.h"

#include "SkCanvas.h"
#include "SkDocument.h"
#include "SkOSFile.h"
#include "SkStream.h"

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
    REPORTER_ASSERT(reporter, file != NULL);
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
    test_empty(reporter);
    test_abort(reporter);
    test_abortWithFile(reporter);
    test_file(reporter);
    test_close(reporter);
}
