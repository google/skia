
/*
 * Copyright 2010 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "Test.h"
#include "SkData.h"
#include "SkFlate.h"
#include "SkPDFCatalog.h"
#include "SkPDFStream.h"
#include "SkPDFTypes.h"
#include "SkScalar.h"
#include "SkStream.h"
#include "SkTypes.h"

class SkPDFTestDict : public SkPDFDict {
public:
    void getResources(SkTDArray<SkPDFObject*>* resourceList) {
        resourceList->setReserve(resourceList->count() + fResources.count());
        for (int i = 0; i < fResources.count(); i++) {
            resourceList->push(fResources[i]);
            fResources[i]->ref();
        }
    }

    void addResource(SkPDFObject* object) {
        fResources.append(1, &object);
    }

private:
    SkTDArray<SkPDFObject*> fResources;
};

static bool stream_equals(const SkDynamicMemoryWStream& stream, size_t offset,
                          const void* buffer, size_t len) {
    SkAutoDataUnref data(stream.copyToData());
    if (offset + len > data.size()) {
        return false;
    }
    return memcmp(data.bytes() + offset, buffer, len) == 0;
}

static void CheckObjectOutput(skiatest::Reporter* reporter, SkPDFObject* obj,
                              const char* expectedData, size_t expectedSize,
                              bool indirect, bool compression) {
    SkPDFDocument::Flags docFlags = (SkPDFDocument::Flags) 0;
    if (!compression) {
        docFlags = SkTBitOr(docFlags, SkPDFDocument::kNoCompression_Flag);
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
    SkRefPtr<SkMemoryStream> streamData = new SkMemoryStream(
        streamBytes, strlen(streamBytes), true);
    streamData->unref();  // SkRefPtr and new both took a reference.
    SkRefPtr<SkPDFStream> stream = new SkPDFStream(streamData.get());
    stream->unref();  // SkRefPtr and new both took a reference.
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
        SkRefPtr<SkPDFStream> stream = new SkPDFStream(streamData2.get());
        stream->unref();  // SkRefPtr and new both took a reference.

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
                          (const char*) expectedResultData1.data(),
                          expectedResultData1.size(), true, false);

        // Then again with compression.
        SkDynamicMemoryWStream expectedResult2;
        expectedResult2.writeText("<</Filter /FlateDecode\n/Length 116\n"
                                 ">> stream\n");
        expectedResult2.write(compressedData.data(), compressedData.size());
        expectedResult2.writeText("\nendstream");
        SkAutoDataUnref expectedResultData2(expectedResult2.copyToData());
        CheckObjectOutput(reporter, stream.get(),
                          (const char*) expectedResultData2.data(),
                          expectedResultData2.size(), true, true);
    }
}

static void TestCatalog(skiatest::Reporter* reporter) {
    SkPDFCatalog catalog((SkPDFDocument::Flags)0);
    SkRefPtr<SkPDFInt> int1 = new SkPDFInt(1);
    int1->unref();  // SkRefPtr and new both took a reference.
    SkRefPtr<SkPDFInt> int2 = new SkPDFInt(2);
    int2->unref();  // SkRefPtr and new both took a reference.
    SkRefPtr<SkPDFInt> int3 = new SkPDFInt(3);
    int3->unref();  // SkRefPtr and new both took a reference.
    SkRefPtr<SkPDFInt> int1Again(int1.get());

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
    SkRefPtr<SkPDFInt> int1 = new SkPDFInt(1);
    int1->unref();  // SkRefPtr and new both took a reference.
    SkRefPtr<SkPDFInt> int2 = new SkPDFInt(2);
    int2->unref();  // SkRefPtr and new both took a reference.
    SkRefPtr<SkPDFObjRef> int2ref = new SkPDFObjRef(int2.get());
    int2ref->unref();  // SkRefPtr and new both took a reference.

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
    SkRefPtr<SkPDFTestDict> proxy = new SkPDFTestDict();
    proxy->unref();  // SkRefPtr and new both took a reference.
    SkRefPtr<SkPDFTestDict> stub = new SkPDFTestDict();
    stub->unref();  // SkRefPtr and new both took a reference.
    SkRefPtr<SkPDFInt> int33 = new SkPDFInt(33);
    int33->unref();  // SkRefPtr and new both took a reference.
    SkRefPtr<SkPDFDict> stubResource = new SkPDFDict();
    stubResource->unref();  // SkRefPtr and new both took a reference.
    SkRefPtr<SkPDFInt> int44 = new SkPDFInt(44);
    int44->unref();  // SkRefPtr and new both took a reference.

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

static void TestPDFPrimitives(skiatest::Reporter* reporter) {
    SkRefPtr<SkPDFInt> int42 = new SkPDFInt(42);
    int42->unref();  // SkRefPtr and new both took a reference.
    SimpleCheckObjectOutput(reporter, int42.get(), "42");

    SkRefPtr<SkPDFScalar> realHalf = new SkPDFScalar(SK_ScalarHalf);
    realHalf->unref();  // SkRefPtr and new both took a reference.
    SimpleCheckObjectOutput(reporter, realHalf.get(), "0.5");

#if defined(SK_SCALAR_IS_FLOAT)
    SkRefPtr<SkPDFScalar> bigScalar = new SkPDFScalar(110999.75f);
    bigScalar->unref();  // SkRefPtr and new both took a reference.
#if !defined(SK_ALLOW_LARGE_PDF_SCALARS)
    SimpleCheckObjectOutput(reporter, bigScalar.get(), "111000");
#else
    SimpleCheckObjectOutput(reporter, bigScalar.get(), "110999.75");

    SkRefPtr<SkPDFScalar> biggerScalar = new SkPDFScalar(50000000.1);
    biggerScalar->unref();  // SkRefPtr and new both took a reference.
    SimpleCheckObjectOutput(reporter, biggerScalar.get(), "50000000");

    SkRefPtr<SkPDFScalar> smallestScalar = new SkPDFScalar(1.0/65536);
    smallestScalar->unref();  // SkRefPtr and new both took a reference.
    SimpleCheckObjectOutput(reporter, smallestScalar.get(), "0.00001526");
#endif
#endif

    SkRefPtr<SkPDFString> stringSimple = new SkPDFString("test ) string ( foo");
    stringSimple->unref();  // SkRefPtr and new both took a reference.
    SimpleCheckObjectOutput(reporter, stringSimple.get(),
                            "(test \\) string \\( foo)");
    SkRefPtr<SkPDFString> stringComplex =
        new SkPDFString("\ttest ) string ( foo");
    stringComplex->unref();  // SkRefPtr and new both took a reference.
    SimpleCheckObjectOutput(reporter, stringComplex.get(),
                            "<0974657374202920737472696E67202820666F6F>");

    SkRefPtr<SkPDFName> name = new SkPDFName("Test name\twith#tab");
    name->unref();  // SkRefPtr and new both took a reference.
    const char expectedResult[] = "/Test#20name#09with#23tab";
    CheckObjectOutput(reporter, name.get(), expectedResult,
                      strlen(expectedResult), false, false);

    // Test that we correctly handle characters with the high-bit set.
    const unsigned char highBitCString[] = {0xDE, 0xAD, 'b', 'e', 0xEF, 0};
    SkRefPtr<SkPDFName> highBitName = new SkPDFName((const char*)highBitCString);
    highBitName->unref();  // SkRefPtr and new both took a reference.
    const char highBitExpectedResult[] = "/#DE#ADbe#EF";
    CheckObjectOutput(reporter, highBitName.get(), highBitExpectedResult,
                      strlen(highBitExpectedResult), false, false);

    SkRefPtr<SkPDFArray> array = new SkPDFArray;
    array->unref();  // SkRefPtr and new both took a reference.
    SimpleCheckObjectOutput(reporter, array.get(), "[]");
    array->append(int42.get());
    SimpleCheckObjectOutput(reporter, array.get(), "[42]");
    array->append(realHalf.get());
    SimpleCheckObjectOutput(reporter, array.get(), "[42 0.5]");
    SkRefPtr<SkPDFInt> int0 = new SkPDFInt(0);
    int0->unref();  // SkRefPtr and new both took a reference.
    array->append(int0.get());
    SimpleCheckObjectOutput(reporter, array.get(), "[42 0.5 0]");
    SkRefPtr<SkPDFInt> int1 = new SkPDFInt(1);
    int1->unref();  // SkRefPtr and new both took a reference.
    array->setAt(0, int1.get());
    SimpleCheckObjectOutput(reporter, array.get(), "[1 0.5 0]");

    SkRefPtr<SkPDFDict> dict = new SkPDFDict;
    dict->unref();  // SkRefPtr and new both took a reference.
    SimpleCheckObjectOutput(reporter, dict.get(), "<<>>");
    SkRefPtr<SkPDFName> n1 = new SkPDFName("n1");
    n1->unref();  // SkRefPtr and new both took a reference.
    dict->insert(n1.get(), int42.get());
    SimpleCheckObjectOutput(reporter, dict.get(), "<</n1 42\n>>");
    SkRefPtr<SkPDFName> n2 = new SkPDFName("n2");
    n2->unref();  // SkRefPtr and new both took a reference.
    SkRefPtr<SkPDFName> n3 = new SkPDFName("n3");
    n3->unref();  // SkRefPtr and new both took a reference.
    dict->insert(n2.get(), realHalf.get());
    dict->insert(n3.get(), array.get());
    SimpleCheckObjectOutput(reporter, dict.get(),
                            "<</n1 42\n/n2 0.5\n/n3 [1 0.5 0]\n>>");

    TestPDFStream(reporter);

    TestCatalog(reporter);

    TestObjectRef(reporter);

    TestSubstitute(reporter);
}

#include "TestClassDef.h"
DEFINE_TESTCLASS("PDFPrimitives", PDFPrimitivesTestClass, TestPDFPrimitives)
