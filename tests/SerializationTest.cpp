/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkOrderedWriteBuffer.h"
#include "SkValidatingReadBuffer.h"
#include "Test.h"

static void Tests(skiatest::Reporter* reporter) {
    {
        static const uint32_t arraySize = 512;
        unsigned char data[arraySize] = {0};
        SkOrderedWriteBuffer writer(1024);
        writer.setFlags(SkOrderedWriteBuffer::kValidation_Flag);
        writer.writeByteArray(data, arraySize);
        uint32_t bytesWritten = writer.bytesWritten();
        // This should write the length (in 4 bytes) and the array
        REPORTER_ASSERT(reporter, (4 + arraySize) == bytesWritten);

        unsigned char dataWritten[1024];
        writer.writeToMemory(dataWritten);

        // Make sure this fails when it should
        SkValidatingReadBuffer buffer(dataWritten, bytesWritten);
        unsigned char dataRead[arraySize];
        bool success = buffer.readByteArray(dataRead, 256);
        // This should have failed, since 256 < sizeInBytes
        REPORTER_ASSERT(reporter, !success);

        // Make sure this succeeds when it should
        SkValidatingReadBuffer buffer2(dataWritten, bytesWritten);
        success = buffer2.readByteArray(dataRead, arraySize);
        // This should have succeeded, since there are enough bytes to read this
        REPORTER_ASSERT(reporter, success);
    }

    {
        static const uint32_t arraySize = 64;
        SkColor data[arraySize];
        SkOrderedWriteBuffer writer(1024);
        writer.setFlags(SkOrderedWriteBuffer::kValidation_Flag);
        writer.writeColorArray(data, arraySize);
        uint32_t bytesWritten = writer.bytesWritten();
        // This should write the length (in 4 bytes) and the array
        REPORTER_ASSERT(reporter, (4 + arraySize * sizeof(SkColor)) == bytesWritten);

        unsigned char dataWritten[1024];
        writer.writeToMemory(dataWritten);

        // Make sure this fails when it should
        SkValidatingReadBuffer buffer(dataWritten, bytesWritten);
        SkColor dataRead[arraySize];
        bool success = buffer.readColorArray(dataRead, 32);
        // This should have failed, since 256 < sizeInBytes
        REPORTER_ASSERT(reporter, !success);

        // Make sure this succeeds when it should
        SkValidatingReadBuffer buffer2(dataWritten, bytesWritten);
        success = buffer2.readColorArray(dataRead, arraySize);
        // This should have succeeded, since there are enough bytes to read this
        REPORTER_ASSERT(reporter, success);
    }

    {
        static const uint32_t arraySize = 64;
        int32_t data[arraySize];
        SkOrderedWriteBuffer writer(1024);
        writer.setFlags(SkOrderedWriteBuffer::kValidation_Flag);
        writer.writeIntArray(data, arraySize);
        uint32_t bytesWritten = writer.bytesWritten();
        // This should write the length (in 4 bytes) and the array
        REPORTER_ASSERT(reporter, (4 + arraySize * sizeof(int32_t)) == bytesWritten);

        unsigned char dataWritten[1024];
        writer.writeToMemory(dataWritten);

        // Make sure this fails when it should
        SkValidatingReadBuffer buffer(dataWritten, bytesWritten);
        int32_t dataRead[arraySize];
        bool success = buffer.readIntArray(dataRead, 32);
        // This should have failed, since 256 < sizeInBytes
        REPORTER_ASSERT(reporter, !success);

        // Make sure this succeeds when it should
        SkValidatingReadBuffer buffer2(dataWritten, bytesWritten);
        success = buffer2.readIntArray(dataRead, arraySize);
        // This should have succeeded, since there are enough bytes to read this
        REPORTER_ASSERT(reporter, success);
    }

    {
        static const uint32_t arraySize = 64;
        SkPoint data[arraySize];
        SkOrderedWriteBuffer writer(1024);
        writer.setFlags(SkOrderedWriteBuffer::kValidation_Flag);
        writer.writePointArray(data, arraySize);
        uint32_t bytesWritten = writer.bytesWritten();
        // This should write the length (in 4 bytes) and the array
        REPORTER_ASSERT(reporter, (4 + arraySize * sizeof(SkPoint)) == bytesWritten);

        unsigned char dataWritten[1024];
        writer.writeToMemory(dataWritten);

        // Make sure this fails when it should
        SkValidatingReadBuffer buffer(dataWritten, bytesWritten);
        SkPoint dataRead[arraySize];
        bool success = buffer.readPointArray(dataRead, 32);
        // This should have failed, since 256 < sizeInBytes
        REPORTER_ASSERT(reporter, !success);

        // Make sure this succeeds when it should
        SkValidatingReadBuffer buffer2(dataWritten, bytesWritten);
        success = buffer2.readPointArray(dataRead, arraySize);
        // This should have succeeded, since there are enough bytes to read this
        REPORTER_ASSERT(reporter, success);
    }

    {
        static const uint32_t arraySize = 64;
        SkScalar data[arraySize];
        SkOrderedWriteBuffer writer(1024);
        writer.setFlags(SkOrderedWriteBuffer::kValidation_Flag);
        writer.writeScalarArray(data, arraySize);
        uint32_t bytesWritten = writer.bytesWritten();
        // This should write the length (in 4 bytes) and the array
        REPORTER_ASSERT(reporter, (4 + arraySize * sizeof(SkScalar)) == bytesWritten);

        unsigned char dataWritten[1024];
        writer.writeToMemory(dataWritten);

        // Make sure this fails when it should
        SkValidatingReadBuffer buffer(dataWritten, bytesWritten);
        SkScalar dataRead[arraySize];
        bool success = buffer.readScalarArray(dataRead, 32);
        // This should have failed, since 256 < sizeInBytes
        REPORTER_ASSERT(reporter, !success);

        // Make sure this succeeds when it should
        SkValidatingReadBuffer buffer2(dataWritten, bytesWritten);
        success = buffer2.readScalarArray(dataRead, arraySize);
        // This should have succeeded, since there are enough bytes to read this
        REPORTER_ASSERT(reporter, success);
    }
}

#include "TestClassDef.h"
DEFINE_TESTCLASS("Serialization", SerializationClass, Tests)
