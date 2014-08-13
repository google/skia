/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkData.h"
#include "SkFlate.h"
#include "SkRandom.h"
#include "SkStream.h"
#include "Test.h"

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

// Returns a deterministic data of the given size.
static SkData* new_test_data(size_t dataSize) {
    SkAutoTMalloc<uint8_t> testBuffer(dataSize);
    SkRandom random(0);
    for (size_t i = 0; i < dataSize; ++i) {
        testBuffer[i] = random.nextU() & 0xFF;
    }
    return SkData::NewFromMalloc(testBuffer.detach(), dataSize);
}

static void TestFlate(skiatest::Reporter* reporter, SkMemoryStream* testStream,
                      size_t dataSize) {
    SkASSERT(testStream != NULL);

    SkAutoDataUnref testData(new_test_data(dataSize));
    SkASSERT(testData->size() == dataSize);

    testStream->setMemory(testData->data(), dataSize, /*copyData=*/ true);
    SkDynamicMemoryWStream compressed;
    bool deflateSuccess = SkFlate::Deflate(testStream, &compressed);
    REPORTER_ASSERT(reporter, deflateSuccess);

    // Check that the input data wasn't changed.
    size_t inputSize = testStream->getLength();
    if (inputSize == 0) {
        inputSize = testStream->read(NULL, SkZeroSizeMemStream::kGetSizeKey);
    }
    REPORTER_ASSERT(reporter, dataSize == inputSize);
    if (dataSize == inputSize) {
        REPORTER_ASSERT(reporter, memcmp(testData->data(),
                                         testStream->getMemoryBase(),
                                         dataSize) == 0);
    }

    // Assume there are two test sizes, big and small.
    if (dataSize < 1024) {
        REPORTER_ASSERT(reporter, compressed.getOffset() < 1024);
    } else {
        REPORTER_ASSERT(reporter, compressed.getOffset() > 1024);
    }

    SkAutoDataUnref compressedData(compressed.copyToData());
    testStream->setData(compressedData.get());

    SkDynamicMemoryWStream uncompressed;
    bool inflateSuccess = SkFlate::Inflate(testStream, &uncompressed);
    REPORTER_ASSERT(reporter, inflateSuccess);

    // Check that the input data wasn't changed.
    inputSize = testStream->getLength();
    if (inputSize == 0) {
        inputSize = testStream->read(NULL, SkZeroSizeMemStream::kGetSizeKey);
    }
    REPORTER_ASSERT(reporter, compressedData->size() == inputSize);
    if (compressedData->size() == inputSize) {
        REPORTER_ASSERT(reporter, memcmp(testStream->getMemoryBase(),
                                         compressedData->data(),
                                         compressedData->size()) == 0);
    }

    // Check that the uncompressed data matches the source data.
    SkAutoDataUnref uncompressedData(uncompressed.copyToData());
    REPORTER_ASSERT(reporter, dataSize == uncompressedData->size());
    if (dataSize == uncompressedData->size()) {
        REPORTER_ASSERT(reporter, memcmp(testData->data(),
                                         uncompressedData->data(),
                                         dataSize) == 0);
    }
}

DEF_TEST(Flate, reporter) {
#ifdef SK_HAS_ZLIB
    REPORTER_ASSERT(reporter, SkFlate::HaveFlate());
#endif
    if (SkFlate::HaveFlate()) {
        SkMemoryStream memStream;
        TestFlate(reporter, &memStream, 512);
        TestFlate(reporter, &memStream, 10240);

        SkZeroSizeMemStream fileStream;
        TestFlate(reporter, &fileStream, 512);
        TestFlate(reporter, &fileStream, 10240);
    }
}
