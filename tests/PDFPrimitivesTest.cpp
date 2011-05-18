/*
 * Copyright (C) 2010 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <string>

#include "Test.h"
#include "SkPDFCatalog.h"
#include "SkPDFStream.h"
#include "SkPDFTypes.h"
#include "SkScalar.h"
#include "SkStream.h"

static void CheckObjectOutput(skiatest::Reporter* reporter, SkPDFObject* obj,
                              const std::string& representation,
                              bool indirect) {
    size_t directSize = obj->getOutputSize(NULL, false);
    REPORTER_ASSERT(reporter, directSize == representation.size());

    SkDynamicMemoryWStream buffer;
    obj->emitObject(&buffer, NULL, false);
    REPORTER_ASSERT(reporter, directSize == buffer.getOffset());
    REPORTER_ASSERT(reporter, memcmp(buffer.getStream(), representation.c_str(),
                                     directSize) == 0);

    if (indirect) {
        // Indirect output.
        static char header[] = "1 0 obj\n";
        static size_t headerLen = strlen(header);
        static char footer[] = "\nendobj\n";
        static size_t footerLen = strlen(footer);

        SkPDFCatalog catalog;
        catalog.addObject(obj, false);

        size_t indirectSize = obj->getOutputSize(&catalog, true);
        REPORTER_ASSERT(reporter,
                        indirectSize == directSize + headerLen + footerLen);

        buffer.reset();
        obj->emitObject(&buffer, &catalog, true);
        REPORTER_ASSERT(reporter, indirectSize == buffer.getOffset());
        REPORTER_ASSERT(reporter, memcmp(buffer.getStream(), header,
                                         headerLen) == 0);
        REPORTER_ASSERT(reporter,
                        memcmp(buffer.getStream() + headerLen,
                               representation.c_str(), directSize) == 0);
        REPORTER_ASSERT(reporter,
                        memcmp(buffer.getStream() + headerLen + directSize,
                               footer, footerLen) == 0);
    }
}

static void TestCatalog(skiatest::Reporter* reporter) {
    SkPDFCatalog catalog;
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
    REPORTER_ASSERT(reporter, memcmp(buffer.getStream(), expectedResult,
                                     strlen(expectedResult)) == 0);
}

static void TestObjectRef(skiatest::Reporter* reporter) {
    SkRefPtr<SkPDFInt> int1 = new SkPDFInt(1);
    int1->unref();  // SkRefPtr and new both took a reference.
    SkRefPtr<SkPDFInt> int2 = new SkPDFInt(2);
    int2->unref();  // SkRefPtr and new both took a reference.
    SkRefPtr<SkPDFObjRef> int2ref = new SkPDFObjRef(int2.get());
    int2ref->unref();  // SkRefPtr and new both took a reference.

    SkPDFCatalog catalog;
    catalog.addObject(int1.get(), false);
    catalog.addObject(int2.get(), false);
    REPORTER_ASSERT(reporter, catalog.getObjectNumberSize(int1.get()) == 3);
    REPORTER_ASSERT(reporter, catalog.getObjectNumberSize(int2.get()) == 3);

    char expectedResult[] = "2 0 R";
    SkDynamicMemoryWStream buffer;
    int2ref->emitObject(&buffer, &catalog, false);
    REPORTER_ASSERT(reporter, buffer.getOffset() == strlen(expectedResult));
    REPORTER_ASSERT(reporter, memcmp(buffer.getStream(), expectedResult,
                                     buffer.getOffset()) == 0);
}

static void TestPDFPrimitives(skiatest::Reporter* reporter) {
    SkRefPtr<SkPDFInt> int42 = new SkPDFInt(42);
    int42->unref();  // SkRefPtr and new both took a reference.
    CheckObjectOutput(reporter, int42.get(), "42", true);

    SkRefPtr<SkPDFScalar> realHalf = new SkPDFScalar(SK_ScalarHalf);
    realHalf->unref();  // SkRefPtr and new both took a reference.
    CheckObjectOutput(reporter, realHalf.get(), "0.5", true);

#if defined(SK_SCALAR_IS_FLOAT)
    SkRefPtr<SkPDFScalar> bigScalar = new SkPDFScalar(110999.75);
    bigScalar->unref();  // SkRefPtr and new both took a reference.
#if !defined(SK_ALLOW_LARGE_PDF_SCALARS)
    CheckObjectOutput(reporter, bigScalar.get(), "111000", true);
#else
    CheckObjectOutput(reporter, bigScalar.get(), "110999.75", true);

    SkRefPtr<SkPDFScalar> biggerScalar = new SkPDFScalar(50000000.1);
    biggerScalar->unref();  // SkRefPtr and new both took a reference.
    CheckObjectOutput(reporter, biggerScalar.get(), "50000000", true);

    SkRefPtr<SkPDFScalar> smallestScalar = new SkPDFScalar(1.0/65536);
    smallestScalar->unref();  // SkRefPtr and new both took a reference.
    CheckObjectOutput(reporter, smallestScalar.get(), "0.00001526", true);
#endif
#endif

    SkRefPtr<SkPDFString> stringSimple = new SkPDFString("test ) string ( foo");
    stringSimple->unref();  // SkRefPtr and new both took a reference.
    CheckObjectOutput(reporter, stringSimple.get(), "(test \\) string \\( foo)",
                      true);
    SkRefPtr<SkPDFString> stringComplex =
        new SkPDFString("\ttest ) string ( foo");
    stringComplex->unref();  // SkRefPtr and new both took a reference.
    CheckObjectOutput(reporter, stringComplex.get(),
                      "<0974657374202920737472696E67202820666F6F>", true);

    SkRefPtr<SkPDFName> name = new SkPDFName("Test name\twith#tab");
    name->unref();  // SkRefPtr and new both took a reference.
    CheckObjectOutput(reporter, name.get(), "/Test#20name#09with#23tab", false);

    SkRefPtr<SkPDFArray> array = new SkPDFArray;
    array->unref();  // SkRefPtr and new both took a reference.
    CheckObjectOutput(reporter, array.get(), "[]", true);
    array->append(int42.get());
    CheckObjectOutput(reporter, array.get(), "[42]", true);
    array->append(realHalf.get());
    CheckObjectOutput(reporter, array.get(), "[42 0.5]", true);
    SkRefPtr<SkPDFInt> int0 = new SkPDFInt(0);
    int0->unref();  // SkRefPtr and new both took a reference.
    array->append(int0.get());
    CheckObjectOutput(reporter, array.get(), "[42 0.5 0]", true);
    SkRefPtr<SkPDFInt> int1 = new SkPDFInt(1);
    int1->unref();  // SkRefPtr and new both took a reference.
    array->setAt(0, int1.get());
    CheckObjectOutput(reporter, array.get(), "[1 0.5 0]", true);

    SkRefPtr<SkPDFDict> dict = new SkPDFDict;
    dict->unref();  // SkRefPtr and new both took a reference.
    CheckObjectOutput(reporter, dict.get(), "<<>>", true);
    SkRefPtr<SkPDFName> n1 = new SkPDFName("n1");
    n1->unref();  // SkRefPtr and new both took a reference.
    dict->insert(n1.get(), int42.get());
    CheckObjectOutput(reporter, dict.get(), "<</n1 42\n>>", true);
    SkRefPtr<SkPDFName> n2 = new SkPDFName("n2");
    n2->unref();  // SkRefPtr and new both took a reference.
    SkRefPtr<SkPDFName> n3 = new SkPDFName("n3");
    n3->unref();  // SkRefPtr and new both took a reference.
    dict->insert(n2.get(), realHalf.get());
    dict->insert(n3.get(), array.get());
    CheckObjectOutput(reporter, dict.get(),
                      "<</n1 42\n/n2 0.5\n/n3 [1 0.5 0]\n>>", true);

    char streamBytes[] = "Test\nFoo\tBar";
    SkRefPtr<SkMemoryStream> streamData = new SkMemoryStream(
        streamBytes, strlen(streamBytes), true);
    streamData->unref();  // SkRefPtr and new both took a reference.
    SkRefPtr<SkPDFStream> stream = new SkPDFStream(streamData.get());
    stream->unref();  // SkRefPtr and new both took a reference.
    CheckObjectOutput(reporter, stream.get(),
                      "<</Length 12\n>> stream\nTest\nFoo\tBar\nendstream",
                      true);
    stream->insert(n1.get(), int42.get());
    CheckObjectOutput(reporter, stream.get(),
                      "<</Length 12\n/n1 42\n>> stream\nTest\nFoo\tBar"
                      "\nendstream",
                      true);

    TestCatalog(reporter);

    TestObjectRef(reporter);
}

#include "TestClassDef.h"
DEFINE_TESTCLASS("PDFPrimitives", PDFPrimitivesTestClass, TestPDFPrimitives)
