/*
 * Copyright (C) 2011 Google Inc.
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

#include <stdlib.h>
#include <string.h>

#include "Test.h"
#include "SkFlate.h"
#include "SkStream.h"

// A memory stream that reports zero size with the standard call, like
// an unseekable file stream would.
class SkZeroSizeMemStream : public SkMemoryStream {
public:
    virtual size_t read(void* buffer, size_t size) {
        if (buffer == NULL && size == 0)
            return 0;
        if (buffer == NULL && size == kGetSizeKey)
            size = 0;
        return SkMemoryStream::read(buffer, size);
    }

    static const size_t kGetSizeKey = 0xDEADBEEF;
};

static void TestFlate(skiatest::Reporter* reporter, SkMemoryStream* testStream,
                      size_t dataSize) {
    if (testStream == NULL)
      return;

    SkMemoryStream testData(dataSize);
    uint8_t* data = (uint8_t*)testData.getMemoryBase();
    srand(0);  // Make data deterministic.
    for (size_t i = 0; i < dataSize; i++)
        data[i] = rand() & 0xFF;

    testStream->setMemory(testData.getMemoryBase(), dataSize, true);
    SkDynamicMemoryWStream compressed;
    bool status = SkFlate::Deflate(testStream, &compressed);
    REPORTER_ASSERT(reporter, status);

    // Check that the input data wasn't changed.
    size_t inputSize = testStream->getLength();
    if (inputSize == 0)
        inputSize = testStream->read(NULL, SkZeroSizeMemStream::kGetSizeKey);
    REPORTER_ASSERT(reporter, testData.getLength() == inputSize);
    REPORTER_ASSERT(reporter, memcmp(testData.getMemoryBase(),
                                     testStream->getMemoryBase(),
                                     testData.getLength()) == 0);

    // Assume there are two test sizes, big and small.
    if (dataSize < 1024)
      REPORTER_ASSERT(reporter, compressed.getOffset() < 1024);
    else
      REPORTER_ASSERT(reporter, compressed.getOffset() > 1024);

    testStream->setMemory(compressed.getStream(), compressed.getOffset(), true);
    SkDynamicMemoryWStream uncompressed;
    status = SkFlate::Inflate(testStream, &uncompressed);
    REPORTER_ASSERT(reporter, status);

    // Check that the input data wasn't changed.
    inputSize = testStream->getLength();
    if (inputSize == 0)
        inputSize = testStream->read(NULL, SkZeroSizeMemStream::kGetSizeKey);
    REPORTER_ASSERT(reporter, compressed.getOffset() == inputSize);
    REPORTER_ASSERT(reporter, memcmp(testStream->getMemoryBase(),
                                     compressed.getStream(),
                                     compressed.getOffset()) == 0);

    // Check that the uncompressed data matches the source data.
    REPORTER_ASSERT(reporter, testData.getLength() == uncompressed.getOffset());
    REPORTER_ASSERT(reporter, memcmp(testData.getMemoryBase(),
                                     uncompressed.getStream(),
                                     testData.getLength()) == 0);
}

static void TestFlateCompression(skiatest::Reporter* reporter) {
    TestFlate(reporter, NULL, 0);
#if defined(SK_ZLIB_INCLUDE) && !defined(SK_DEBUG)
    REPORTER_ASSERT(reporter, SkFlate::HaveFlate());

    SkMemoryStream memStream;
    TestFlate(reporter, &memStream, 512);
    TestFlate(reporter, &memStream, 10240);

    SkZeroSizeMemStream fileStream;
    TestFlate(reporter, &fileStream, 512);
    TestFlate(reporter, &fileStream, 10240);
#endif
}

#include "TestClassDef.h"
DEFINE_TESTCLASS("Flate", FlateTestClass, TestFlateCompression)
