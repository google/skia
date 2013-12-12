/*
 * Copyright 2010 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"
#include "TestClassDef.h"
#include "SkBitmap.h"
#include "SkCanvas.h"
#include "SkData.h"
#include "SkFlate.h"
#include "SkImageEncoder.h"
#include "SkMatrix.h"
#include "SkPDFCatalog.h"
#include "SkPDFDevice.h"
#include "SkPDFStream.h"
#include "SkPDFTypes.h"
#include "SkScalar.h"
#include "SkStream.h"
#include "SkTypes.h"

class SkPDFTestDict : public SkPDFDict {
public:
  virtual void getResources(const SkTSet<SkPDFObject*>& knownResourceObjects,
                            SkTSet<SkPDFObject*>* newResourceObjects) {
        for (int i = 0; i < fResources.count(); i++) {
            newResourceObjects->add(fResources[i]);
            fResources[i]->ref();
        }
    }

    void addResource(SkPDFObject* object) {
        fResources.append(1, &object);
    }

private:
    SkTDArray<SkPDFObject*> fResources;
};

#define DUMMY_TEXT "DCT compessed stream."

static SkData* encode_to_dct_data(size_t* pixelRefOffset, const SkBitmap& bitmap) {
    *pixelRefOffset = 0;
    return SkData::NewWithProc(DUMMY_TEXT, sizeof(DUMMY_TEXT) - 1, NULL, NULL);
}

static bool stream_equals(const SkDynamicMemoryWStream& stream, size_t offset,
                          const void* buffer, size_t len) {
    SkAutoDataUnref data(stream.copyToData());
    if (offset + len > data->size()) {
        return false;
    }
    return memcmp(data->bytes() + offset, buffer, len) == 0;
}

static bool stream_contains(const SkDynamicMemoryWStream& stream,
                            const char* buffer) {
    SkAutoDataUnref data(stream.copyToData());
    int len = strlen(buffer);  // our buffer does not have EOSs.

    for (int offset = 0 ; offset < (int)data->size() - len; offset++) {
        if (memcmp(data->bytes() + offset, buffer, len) == 0) {
            return true;
        }
    }

    return false;
}

static void CheckObjectOutput(skiatest::Reporter* reporter, SkPDFObject* obj,
                              const char* expectedData, size_t expectedSize,
                              bool indirect, bool compression) {
    SkPDFDocument::Flags docFlags = (SkPDFDocument::Flags) 0;
    if (!compression) {
        docFlags = SkTBitOr(docFlags, SkPDFDocument::kFavorSpeedOverSize_Flags);
    }
    SkPDFCatalog catalog(docFlags);
    size_t directSize = obj->getOutputSize(&catalog, false);
    REPORTER_ASSERT(reporter, directSize == expectedSize);

    SkDynamicMemoryWStream buffer;
    obj->emit(&buffer, &catalog, false);
    REPORTER_ASSERT(reporter, directSize == buffer.getOffset());
    REPORTER_ASSERT(reporter, stream_equals(buffer, 0, expectedData,
                                            directSize));

    if (indirect) {
        // Indirect output.
        static char header[] = "1 0 obj\n";
        static size_t headerLen = strlen(header);
        static char footer[] = "\nendobj\n";
        static size_t footerLen = strlen(footer);

        catalog.addObject(obj, false);

        size_t indirectSize = obj->getOutputSize(&catalog, true);
        REPORTER_ASSERT(reporter,
                        indirectSize == directSize + headerLen + footerLen);

        buffer.reset();
        obj->emit(&buffer, &catalog, true);
        REPORTER_ASSERT(reporter, indirectSize == buffer.getOffset());
        REPORTER_ASSERT(reporter, stream_equals(buffer, 0, header, headerLen));
        REPORTER_ASSERT(reporter, stream_equals(buffer, headerLen, expectedData,
                                                directSize));
        REPORTER_ASSERT(reporter, stream_equals(buffer, headerLen + directSize,
                                                footer, footerLen));
    }
}

static void SimpleCheckObjectOutput(skiatest::Reporter* reporter,
                                    SkPDFObject* obj,
                                    const char* expectedResult) {
    CheckObjectOutput(reporter, obj, expectedResult,
                      strlen(expectedResult), true, false);
}

static void TestPDFStream(skiatest::Reporter* reporter) {
    char streamBytes[] = "Test\nFoo\tBar";
    SkAutoTUnref<SkMemoryStream> streamData(new SkMemoryStream(
        streamBytes, strlen(streamBytes), true));
    SkAutoTUnref<SkPDFStream> stream(new SkPDFStream(streamData.get()));
    SimpleCheckObjectOutput(
        reporter, stream.get(),
        "<</Length 12\n>> stream\nTest\nFoo\tBar\nendstream");
    stream->insert("Attribute", new SkPDFInt(42))->unref();
    SimpleCheckObjectOutput(reporter, stream.get(),
                            "<</Length 12\n/Attribute 42\n>> stream\n"
                                "Test\nFoo\tBar\nendstream");

    if (SkFlate::HaveFlate()) {
        char streamBytes2[] = "This is a longer string, so that compression "
                              "can do something with it. With shorter strings, "
                              "the short circuit logic cuts in and we end up "
                              "with an uncompressed string.";
        SkAutoDataUnref streamData2(SkData::NewWithCopy(streamBytes2,
                                                        strlen(streamBytes2)));
        SkAutoTUnref<SkPDFStream> stream(new SkPDFStream(streamData2.get()));

        SkDynamicMemoryWStream compressedByteStream;
        SkFlate::Deflate(streamData2.get(), &compressedByteStream);
        SkAutoDataUnref compressedData(compressedByteStream.copyToData());

        // Check first without compression.
        SkDynamicMemoryWStream expectedResult1;
        expectedResult1.writeText("<</Length 167\n>> stream\n");
        expectedResult1.writeText(streamBytes2);
        expectedResult1.writeText("\nendstream");
        SkAutoDataUnref expectedResultData1(expectedResult1.copyToData());
        CheckObjectOutput(reporter, stream.get(),
                          (const char*) expectedResultData1->data(),
                          expectedResultData1->size(), true, false);

        // Then again with compression.
        SkDynamicMemoryWStream expectedResult2;
        expectedResult2.writeText("<</Filter /FlateDecode\n/Length 116\n"
                                 ">> stream\n");
        expectedResult2.write(compressedData->data(), compressedData->size());
        expectedResult2.writeText("\nendstream");
        SkAutoDataUnref expectedResultData2(expectedResult2.copyToData());
        CheckObjectOutput(reporter, stream.get(),
                          (const char*) expectedResultData2->data(),
                          expectedResultData2->size(), true, true);
    }
}

static void TestCatalog(skiatest::Reporter* reporter) {
    SkPDFCatalog catalog((SkPDFDocument::Flags)0);
    SkAutoTUnref<SkPDFInt> int1(new SkPDFInt(1));
    SkAutoTUnref<SkPDFInt> int2(new SkPDFInt(2));
    SkAutoTUnref<SkPDFInt> int3(new SkPDFInt(3));
    int1.get()->ref();
    SkAutoTUnref<SkPDFInt> int1Again(int1.get());

    catalog.addObject(int1.get(), false);
    catalog.addObject(int2.get(), false);
    catalog.addObject(int3.get(), false);

    REPORTER_ASSERT(reporter, catalog.getObjectNumberSize(int1.get()) == 3);
    REPORTER_ASSERT(reporter, catalog.getObjectNumberSize(int2.get()) == 3);
    REPORTER_ASSERT(reporter, catalog.getObjectNumberSize(int3.get()) == 3);

    SkDynamicMemoryWStream buffer;
    catalog.emitObjectNumber(&buffer, int1.get());
    catalog.emitObjectNumber(&buffer, int2.get());
    catalog.emitObjectNumber(&buffer, int3.get());
    catalog.emitObjectNumber(&buffer, int1Again.get());
    char expectedResult[] = "1 02 03 01 0";
    REPORTER_ASSERT(reporter, stream_equals(buffer, 0, expectedResult,
                                            strlen(expectedResult)));
}

static void TestObjectRef(skiatest::Reporter* reporter) {
    SkAutoTUnref<SkPDFInt> int1(new SkPDFInt(1));
    SkAutoTUnref<SkPDFInt> int2(new SkPDFInt(2));
    SkAutoTUnref<SkPDFObjRef> int2ref(new SkPDFObjRef(int2.get()));

    SkPDFCatalog catalog((SkPDFDocument::Flags)0);
    catalog.addObject(int1.get(), false);
    catalog.addObject(int2.get(), false);
    REPORTER_ASSERT(reporter, catalog.getObjectNumberSize(int1.get()) == 3);
    REPORTER_ASSERT(reporter, catalog.getObjectNumberSize(int2.get()) == 3);

    char expectedResult[] = "2 0 R";
    SkDynamicMemoryWStream buffer;
    int2ref->emitObject(&buffer, &catalog, false);
    REPORTER_ASSERT(reporter, buffer.getOffset() == strlen(expectedResult));
    REPORTER_ASSERT(reporter, stream_equals(buffer, 0, expectedResult,
                                            buffer.getOffset()));
}

static void TestSubstitute(skiatest::Reporter* reporter) {
    SkAutoTUnref<SkPDFTestDict> proxy(new SkPDFTestDict());
    SkAutoTUnref<SkPDFTestDict> stub(new SkPDFTestDict());
    SkAutoTUnref<SkPDFInt> int33(new SkPDFInt(33));
    SkAutoTUnref<SkPDFDict> stubResource(new SkPDFDict());
    SkAutoTUnref<SkPDFInt> int44(new SkPDFInt(44));

    stub->insert("Value", int33.get());
    stubResource->insert("InnerValue", int44.get());
    stub->addResource(stubResource.get());

    SkPDFCatalog catalog((SkPDFDocument::Flags)0);
    catalog.addObject(proxy.get(), false);
    catalog.setSubstitute(proxy.get(), stub.get());

    SkDynamicMemoryWStream buffer;
    proxy->emit(&buffer, &catalog, false);
    catalog.emitSubstituteResources(&buffer, false);

    char objectResult[] = "2 0 obj\n<</Value 33\n>>\nendobj\n";
    REPORTER_ASSERT(
        reporter,
        catalog.setFileOffset(proxy.get(), 0) == strlen(objectResult));

    char expectedResult[] =
        "<</Value 33\n>>1 0 obj\n<</InnerValue 44\n>>\nendobj\n";
    REPORTER_ASSERT(reporter, buffer.getOffset() == strlen(expectedResult));
    REPORTER_ASSERT(reporter, stream_equals(buffer, 0, expectedResult,
                                            buffer.getOffset()));
}

// Create a bitmap that would be very eficiently compressed in a ZIP.
static void setup_bitmap(SkBitmap* bitmap, int width, int height) {
    bitmap->setConfig(SkBitmap::kARGB_8888_Config, width, height);
    bitmap->allocPixels();
    bitmap->eraseColor(SK_ColorWHITE);
}

static void TestImage(skiatest::Reporter* reporter, const SkBitmap& bitmap,
                      const char* expected, bool useDCTEncoder) {
    SkISize pageSize = SkISize::Make(bitmap.width(), bitmap.height());
    SkAutoTUnref<SkPDFDevice> dev(new SkPDFDevice(pageSize, pageSize, SkMatrix::I()));

    if (useDCTEncoder) {
        dev->setDCTEncoder(encode_to_dct_data);
    }

    SkCanvas c(dev);
    c.drawBitmap(bitmap, 0, 0, NULL);

    SkPDFDocument doc;
    doc.appendPage(dev);

    SkDynamicMemoryWStream stream;
    doc.emitPDF(&stream);

    REPORTER_ASSERT(reporter, stream_contains(stream, expected));
}

static void TestUncompressed(skiatest::Reporter* reporter) {
    SkBitmap bitmap;
    setup_bitmap(&bitmap, 1, 1);
    TestImage(reporter, bitmap,
              "/Subtype /Image\n"
              "/Width 1\n"
              "/Height 1\n"
              "/ColorSpace /DeviceRGB\n"
              "/BitsPerComponent 8\n"
              "/Length 3\n"
              ">> stream",
              true);
}

static void TestFlateDecode(skiatest::Reporter* reporter) {
    if (!SkFlate::HaveFlate()) {
        return;
    }
    SkBitmap bitmap;
    setup_bitmap(&bitmap, 10, 10);
    TestImage(reporter, bitmap,
              "/Subtype /Image\n"
              "/Width 10\n"
              "/Height 10\n"
              "/ColorSpace /DeviceRGB\n"
              "/BitsPerComponent 8\n"
              "/Filter /FlateDecode\n"
              "/Length 13\n"
              ">> stream",
              false);
}

static void TestDCTDecode(skiatest::Reporter* reporter) {
    SkBitmap bitmap;
    setup_bitmap(&bitmap, 32, 32);
    TestImage(reporter, bitmap,
              "/Subtype /Image\n"
              "/Width 32\n"
              "/Height 32\n"
              "/ColorSpace /DeviceRGB\n"
              "/BitsPerComponent 8\n"
              "/Filter /DCTDecode\n"
              "/ColorTransform 0\n"
              "/Length 21\n"
              ">> stream",
              true);
}

static void TestImages(skiatest::Reporter* reporter) {
    TestUncompressed(reporter);
    TestFlateDecode(reporter);
    TestDCTDecode(reporter);
}

// This test used to assert without the fix submitted for
// http://code.google.com/p/skia/issues/detail?id=1083.
// SKP files might have invalid glyph ids. This test ensures they are ignored,
// and there is no assert on input data in Debug mode.
static void test_issue1083() {
    SkISize pageSize = SkISize::Make(100, 100);
    SkAutoTUnref<SkPDFDevice> dev(new SkPDFDevice(pageSize, pageSize, SkMatrix::I()));

    SkCanvas c(dev);
    SkPaint paint;
    paint.setTextEncoding(SkPaint::kGlyphID_TextEncoding);

    uint16_t glyphID = 65000;
    c.drawText(&glyphID, 2, 0, 0, paint);

    SkPDFDocument doc;
    doc.appendPage(dev);

    SkDynamicMemoryWStream stream;
    doc.emitPDF(&stream);
}

DEF_TEST(PDFPrimitives, reporter) {
    SkAutoTUnref<SkPDFInt> int42(new SkPDFInt(42));
    SimpleCheckObjectOutput(reporter, int42.get(), "42");

    SkAutoTUnref<SkPDFScalar> realHalf(new SkPDFScalar(SK_ScalarHalf));
    SimpleCheckObjectOutput(reporter, realHalf.get(), "0.5");

#if defined(SK_SCALAR_IS_FLOAT)
    SkAutoTUnref<SkPDFScalar> bigScalar(new SkPDFScalar(110999.75f));
#if !defined(SK_ALLOW_LARGE_PDF_SCALARS)
    SimpleCheckObjectOutput(reporter, bigScalar.get(), "111000");
#else
    SimpleCheckObjectOutput(reporter, bigScalar.get(), "110999.75");

    SkAutoTUnref<SkPDFScalar> biggerScalar(new SkPDFScalar(50000000.1));
    SimpleCheckObjectOutput(reporter, biggerScalar.get(), "50000000");

    SkAutoTUnref<SkPDFScalar> smallestScalar(new SkPDFScalar(1.0/65536));
    SimpleCheckObjectOutput(reporter, smallestScalar.get(), "0.00001526");
#endif
#endif

    SkAutoTUnref<SkPDFString> stringSimple(
        new SkPDFString("test ) string ( foo"));
    SimpleCheckObjectOutput(reporter, stringSimple.get(),
                            "(test \\) string \\( foo)");
    SkAutoTUnref<SkPDFString> stringComplex(
        new SkPDFString("\ttest ) string ( foo"));
    SimpleCheckObjectOutput(reporter, stringComplex.get(),
                            "<0974657374202920737472696E67202820666F6F>");

    SkAutoTUnref<SkPDFName> name(new SkPDFName("Test name\twith#tab"));
    const char expectedResult[] = "/Test#20name#09with#23tab";
    CheckObjectOutput(reporter, name.get(), expectedResult,
                      strlen(expectedResult), false, false);

    SkAutoTUnref<SkPDFName> escapedName(new SkPDFName("A#/%()<>[]{}B"));
    const char escapedNameExpected[] = "/A#23#2F#25#28#29#3C#3E#5B#5D#7B#7DB";
    CheckObjectOutput(reporter, escapedName.get(), escapedNameExpected,
                      strlen(escapedNameExpected), false, false);

    // Test that we correctly handle characters with the high-bit set.
    const unsigned char highBitCString[] = {0xDE, 0xAD, 'b', 'e', 0xEF, 0};
    SkAutoTUnref<SkPDFName> highBitName(
        new SkPDFName((const char*)highBitCString));
    const char highBitExpectedResult[] = "/#DE#ADbe#EF";
    CheckObjectOutput(reporter, highBitName.get(), highBitExpectedResult,
                      strlen(highBitExpectedResult), false, false);

    SkAutoTUnref<SkPDFArray> array(new SkPDFArray);
    SimpleCheckObjectOutput(reporter, array.get(), "[]");
    array->append(int42.get());
    SimpleCheckObjectOutput(reporter, array.get(), "[42]");
    array->append(realHalf.get());
    SimpleCheckObjectOutput(reporter, array.get(), "[42 0.5]");
    SkAutoTUnref<SkPDFInt> int0(new SkPDFInt(0));
    array->append(int0.get());
    SimpleCheckObjectOutput(reporter, array.get(), "[42 0.5 0]");
    SkAutoTUnref<SkPDFInt> int1(new SkPDFInt(1));
    array->setAt(0, int1.get());
    SimpleCheckObjectOutput(reporter, array.get(), "[1 0.5 0]");

    SkAutoTUnref<SkPDFDict> dict(new SkPDFDict);
    SimpleCheckObjectOutput(reporter, dict.get(), "<<>>");
    SkAutoTUnref<SkPDFName> n1(new SkPDFName("n1"));
    dict->insert(n1.get(), int42.get());
    SimpleCheckObjectOutput(reporter, dict.get(), "<</n1 42\n>>");
    SkAutoTUnref<SkPDFName> n2(new SkPDFName("n2"));
    SkAutoTUnref<SkPDFName> n3(new SkPDFName("n3"));
    dict->insert(n2.get(), realHalf.get());
    dict->insert(n3.get(), array.get());
    SimpleCheckObjectOutput(reporter, dict.get(),
                            "<</n1 42\n/n2 0.5\n/n3 [1 0.5 0]\n>>");

    TestPDFStream(reporter);

    TestCatalog(reporter);

    TestObjectRef(reporter);

    TestSubstitute(reporter);

    test_issue1083();

    TestImages(reporter);
}
