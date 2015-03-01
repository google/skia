/*
 * Copyright 2010 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmap.h"
#include "SkCanvas.h"
#include "SkData.h"
#include "SkDocument.h"
#include "SkFlate.h"
#include "SkImageEncoder.h"
#include "SkMatrix.h"
#include "SkPDFCanon.h"
#include "SkPDFCatalog.h"
#include "SkPDFDevice.h"
#include "SkPDFStream.h"
#include "SkPDFTypes.h"
#include "SkReadBuffer.h"
#include "SkScalar.h"
#include "SkStream.h"
#include "SkTypes.h"
#include "Test.h"

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

static bool stream_equals(const SkDynamicMemoryWStream& stream, size_t offset,
                          const void* buffer, size_t len) {
    SkAutoDataUnref data(stream.copyToData());
    if (offset + len > data->size()) {
        return false;
    }
    return memcmp(data->bytes() + offset, buffer, len) == 0;
}

static void emit_object(SkPDFObject* object,
                        SkWStream* stream,
                        SkPDFCatalog* catalog,
                        bool indirect) {
    SkPDFObject* realObject = catalog->getSubstituteObject(object);
    if (indirect) {
        stream->writeDecAsText(catalog->getObjectNumber(object));
        stream->writeText(" 0 obj\n");  // Generation number is always 0.
        realObject->emitObject(stream, catalog);
        stream->writeText("\nendobj\n");
    } else {
        realObject->emitObject(stream, catalog);
    }
}

static size_t get_output_size(SkPDFObject* object,
                              SkPDFCatalog* catalog,
                              bool indirect) {
    SkDynamicMemoryWStream buffer;
    emit_object(object, &buffer, catalog, indirect);
    return buffer.getOffset();
}

static void CheckObjectOutput(skiatest::Reporter* reporter, SkPDFObject* obj,
                              const char* expectedData, size_t expectedSize,
                              bool indirect) {
    SkPDFCatalog catalog;
    size_t directSize = get_output_size(obj, &catalog, false);
    REPORTER_ASSERT(reporter, directSize == expectedSize);

    SkDynamicMemoryWStream buffer;
    emit_object(obj, &buffer, &catalog, false);
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

        size_t indirectSize = get_output_size(obj, &catalog, true);
        REPORTER_ASSERT(reporter,
                        indirectSize == directSize + headerLen + footerLen);

        buffer.reset();
        emit_object(obj, &buffer, &catalog, true);
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
                      strlen(expectedResult), true);
}

static void TestPDFStream(skiatest::Reporter* reporter) {
    char streamBytes[] = "Test\nFoo\tBar";
    SkAutoTDelete<SkMemoryStream> streamData(new SkMemoryStream(
        streamBytes, strlen(streamBytes), true));
    SkAutoTUnref<SkPDFStream> stream(new SkPDFStream(streamData.get()));
    SimpleCheckObjectOutput(
        reporter, stream.get(),
        "<</Length 12\n>> stream\nTest\nFoo\tBar\nendstream");
    stream->insert("Attribute", new SkPDFInt(42))->unref();
    SimpleCheckObjectOutput(reporter, stream.get(),
                            "<</Length 12\n/Attribute 42\n>> stream\n"
                                "Test\nFoo\tBar\nendstream");

#ifndef SK_NO_FLATE
    {
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

        SkDynamicMemoryWStream expected;
        expected.writeText("<</Filter /FlateDecode\n/Length 116\n"
                                 ">> stream\n");
        expected.write(compressedData->data(), compressedData->size());
        expected.writeText("\nendstream");
        SkAutoDataUnref expectedResultData2(expected.copyToData());
        CheckObjectOutput(reporter, stream.get(),
                          (const char*) expectedResultData2->data(),
                          expectedResultData2->size(), true);
    }
#endif  // SK_NO_FLATE
}

static void TestCatalog(skiatest::Reporter* reporter) {
    SkPDFCatalog catalog;
    SkAutoTUnref<SkPDFInt> int1(new SkPDFInt(1));
    SkAutoTUnref<SkPDFInt> int2(new SkPDFInt(2));
    SkAutoTUnref<SkPDFInt> int3(new SkPDFInt(3));
    int1.get()->ref();
    SkAutoTUnref<SkPDFInt> int1Again(int1.get());

    catalog.addObject(int1.get(), false);
    catalog.addObject(int2.get(), false);
    catalog.addObject(int3.get(), false);

    REPORTER_ASSERT(reporter, catalog.getObjectNumber(int1.get()) == 1);
    REPORTER_ASSERT(reporter, catalog.getObjectNumber(int2.get()) == 2);
    REPORTER_ASSERT(reporter, catalog.getObjectNumber(int3.get()) == 3);
    REPORTER_ASSERT(reporter, catalog.getObjectNumber(int1Again.get()) == 1);
}

static void TestObjectRef(skiatest::Reporter* reporter) {
    SkAutoTUnref<SkPDFInt> int1(new SkPDFInt(1));
    SkAutoTUnref<SkPDFInt> int2(new SkPDFInt(2));
    SkAutoTUnref<SkPDFObjRef> int2ref(new SkPDFObjRef(int2.get()));

    SkPDFCatalog catalog;
    catalog.addObject(int1.get(), false);
    catalog.addObject(int2.get(), false);
    REPORTER_ASSERT(reporter, catalog.getObjectNumber(int1.get()) == 1);
    REPORTER_ASSERT(reporter, catalog.getObjectNumber(int2.get()) == 2);

    char expectedResult[] = "2 0 R";
    SkDynamicMemoryWStream buffer;
    int2ref->emitObject(&buffer, &catalog);
    REPORTER_ASSERT(reporter, buffer.getOffset() == strlen(expectedResult));
    REPORTER_ASSERT(reporter, stream_equals(buffer, 0, expectedResult,
                                            buffer.getOffset()));
}

static void TestSubstitute(skiatest::Reporter* reporter) {
    SkAutoTUnref<SkPDFTestDict> proxy(new SkPDFTestDict());
    SkAutoTUnref<SkPDFTestDict> stub(new SkPDFTestDict());

    proxy->insert("Value", new SkPDFInt(33))->unref();
    stub->insert("Value", new SkPDFInt(44))->unref();

    SkPDFCatalog catalog;
    catalog.addObject(proxy.get(), false);
    catalog.setSubstitute(proxy.get(), stub.get());

    REPORTER_ASSERT(reporter, stub.get() == catalog.getSubstituteObject(proxy));
    REPORTER_ASSERT(reporter, proxy.get() != catalog.getSubstituteObject(stub));
}

// This test used to assert without the fix submitted for
// http://code.google.com/p/skia/issues/detail?id=1083.
// SKP files might have invalid glyph ids. This test ensures they are ignored,
// and there is no assert on input data in Debug mode.
static void test_issue1083() {
    SkDynamicMemoryWStream outStream;
    SkAutoTUnref<SkDocument> doc(SkDocument::CreatePDF(&outStream));
    SkCanvas* canvas = doc->beginPage(100.0f, 100.0f);
    SkPaint paint;
    paint.setTextEncoding(SkPaint::kGlyphID_TextEncoding);

    uint16_t glyphID = 65000;
    canvas->drawText(&glyphID, 2, 0, 0, paint);

    doc->close();
}

DEF_TEST(PDFPrimitives, reporter) {
    SkAutoTUnref<SkPDFInt> int42(new SkPDFInt(42));
    SimpleCheckObjectOutput(reporter, int42.get(), "42");

    SkAutoTUnref<SkPDFScalar> realHalf(new SkPDFScalar(SK_ScalarHalf));
    SimpleCheckObjectOutput(reporter, realHalf.get(), "0.5");

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
                      strlen(expectedResult), false);

    SkAutoTUnref<SkPDFName> escapedName(new SkPDFName("A#/%()<>[]{}B"));
    const char escapedNameExpected[] = "/A#23#2F#25#28#29#3C#3E#5B#5D#7B#7DB";
    CheckObjectOutput(reporter, escapedName.get(), escapedNameExpected,
                      strlen(escapedNameExpected), false);

    // Test that we correctly handle characters with the high-bit set.
    const unsigned char highBitCString[] = {0xDE, 0xAD, 'b', 'e', 0xEF, 0};
    SkAutoTUnref<SkPDFName> highBitName(
        new SkPDFName((const char*)highBitCString));
    const char highBitExpectedResult[] = "/#DE#ADbe#EF";
    CheckObjectOutput(reporter, highBitName.get(), highBitExpectedResult,
                      strlen(highBitExpectedResult), false);

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
}

namespace {

class DummyImageFilter : public SkImageFilter {
public:
    DummyImageFilter(bool visited = false) : SkImageFilter(0, NULL), fVisited(visited) {}
    ~DummyImageFilter() SK_OVERRIDE {}
    virtual bool onFilterImage(Proxy*, const SkBitmap& src, const Context&,
                               SkBitmap* result, SkIPoint* offset) const SK_OVERRIDE {
        fVisited = true;
        offset->fX = offset->fY = 0;
        *result = src;
        return true;
    }
    SK_TO_STRING_OVERRIDE()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(DummyImageFilter)
    bool visited() const { return fVisited; }

private:
    mutable bool fVisited;
};

SkFlattenable* DummyImageFilter::CreateProc(SkReadBuffer& buffer) {
    SK_IMAGEFILTER_UNFLATTEN_COMMON(common, 0);
    bool visited = buffer.readBool();
    return SkNEW_ARGS(DummyImageFilter, (visited));
}

#ifndef SK_IGNORE_TO_STRING
void DummyImageFilter::toString(SkString* str) const {
    str->appendf("DummyImageFilter: (");
    str->append(")");
}
#endif

};

// Check that PDF rendering of image filters successfully falls back to
// CPU rasterization.
DEF_TEST(PDFImageFilter, reporter) {
    SkDynamicMemoryWStream stream;
    SkAutoTUnref<SkDocument> doc(SkDocument::CreatePDF(&stream));
    SkCanvas* canvas = doc->beginPage(100.0f, 100.0f);

    SkAutoTUnref<DummyImageFilter> filter(new DummyImageFilter());

    // Filter just created; should be unvisited.
    REPORTER_ASSERT(reporter, !filter->visited());
    SkPaint paint;
    paint.setImageFilter(filter.get());
    canvas->drawRect(SkRect::MakeWH(100, 100), paint);
    doc->close();

    // Filter was used in rendering; should be visited.
    REPORTER_ASSERT(reporter, filter->visited());
}
