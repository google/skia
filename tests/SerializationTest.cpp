/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkOrderedWriteBuffer.h"
#include "SkValidatingReadBuffer.h"
#include "Test.h"

static const uint32_t kArraySize = 64;

template<typename T>
static void TestAlignment(T* testObj, skiatest::Reporter* reporter) {
    // Test memory read/write functions directly
    unsigned char dataWritten[1024];
    size_t bytesWrittenToMemory = testObj->writeToMemory(dataWritten);
    REPORTER_ASSERT(reporter, SkAlign4(bytesWrittenToMemory) == bytesWrittenToMemory);
    size_t bytesReadFromMemory = testObj->readFromMemory(dataWritten, bytesWrittenToMemory);
    REPORTER_ASSERT(reporter, SkAlign4(bytesReadFromMemory) == bytesReadFromMemory);
}

template<typename T> struct SerializationUtils {
};

template<> struct SerializationUtils<SkMatrix> {
    static void Write(SkOrderedWriteBuffer& writer, const SkMatrix* matrix) {
        writer.writeMatrix(*matrix);
    }
    static void Read(SkValidatingReadBuffer& reader, SkMatrix* matrix) {
        reader.readMatrix(matrix);
    }
};

template<> struct SerializationUtils<SkPath> {
    static void Write(SkOrderedWriteBuffer& writer, const SkPath* path) {
        writer.writePath(*path);
    }
    static void Read(SkValidatingReadBuffer& reader, SkPath* path) {
        reader.readPath(path);
    }
};

template<> struct SerializationUtils<SkRegion> {
    static void Write(SkOrderedWriteBuffer& writer, const SkRegion* region) {
        writer.writeRegion(*region);
    }
    static void Read(SkValidatingReadBuffer& reader, SkRegion* region) {
        reader.readRegion(region);
    }
};

template<> struct SerializationUtils<unsigned char> {
    static void Write(SkOrderedWriteBuffer& writer, unsigned char* data, uint32_t arraySize) {
        writer.writeByteArray(data, arraySize);
    }
    static bool Read(SkValidatingReadBuffer& reader, unsigned char* data, uint32_t arraySize) {
        return reader.readByteArray(data, arraySize);
    }
};

template<> struct SerializationUtils<SkColor> {
    static void Write(SkOrderedWriteBuffer& writer, SkColor* data, uint32_t arraySize) {
        writer.writeColorArray(data, arraySize);
    }
    static bool Read(SkValidatingReadBuffer& reader, SkColor* data, uint32_t arraySize) {
        return reader.readColorArray(data, arraySize);
    }
};

template<> struct SerializationUtils<int32_t> {
    static void Write(SkOrderedWriteBuffer& writer, int32_t* data, uint32_t arraySize) {
        writer.writeIntArray(data, arraySize);
    }
    static bool Read(SkValidatingReadBuffer& reader, int32_t* data, uint32_t arraySize) {
        return reader.readIntArray(data, arraySize);
    }
};

template<> struct SerializationUtils<SkPoint> {
    static void Write(SkOrderedWriteBuffer& writer, SkPoint* data, uint32_t arraySize) {
        writer.writePointArray(data, arraySize);
    }
    static bool Read(SkValidatingReadBuffer& reader, SkPoint* data, uint32_t arraySize) {
        return reader.readPointArray(data, arraySize);
    }
};

template<> struct SerializationUtils<SkScalar> {
    static void Write(SkOrderedWriteBuffer& writer, SkScalar* data, uint32_t arraySize) {
        writer.writeScalarArray(data, arraySize);
    }
    static bool Read(SkValidatingReadBuffer& reader, SkScalar* data, uint32_t arraySize) {
        return reader.readScalarArray(data, arraySize);
    }
};

template<typename T>
static void TestObjectSerialization(T* testObj, skiatest::Reporter* reporter) {
    SkOrderedWriteBuffer writer(1024);
    writer.setFlags(SkOrderedWriteBuffer::kValidation_Flag);
    SerializationUtils<T>::Write(writer, testObj);
    size_t bytesWritten = writer.bytesWritten();

    unsigned char dataWritten[1024];
    writer.writeToMemory(dataWritten);

    // Make sure this fails when it should
    SkValidatingReadBuffer buffer(dataWritten, bytesWritten - 1);
    const unsigned char* peekBefore = static_cast<const unsigned char*>(buffer.skip(0));
    SerializationUtils<T>::Read(buffer, testObj);
    const unsigned char* peekAfter = static_cast<const unsigned char*>(buffer.skip(0));
    // This should have failed, since the buffer is too small to read a matrix from it
    REPORTER_ASSERT(reporter, peekBefore == peekAfter);

    // Make sure this succeeds when it should
    SkValidatingReadBuffer buffer2(dataWritten, bytesWritten);
    peekBefore = static_cast<const unsigned char*>(buffer2.skip(0));
    SerializationUtils<T>::Read(buffer2, testObj);
    peekAfter = static_cast<const unsigned char*>(buffer2.skip(0));
    // This should have succeeded, since there are enough bytes to read this
    REPORTER_ASSERT(reporter, static_cast<size_t>(peekAfter - peekBefore) == bytesWritten);

    TestAlignment(testObj, reporter);
}

template<typename T>
static void TestArraySerialization(T* data, skiatest::Reporter* reporter) {
    SkOrderedWriteBuffer writer(1024);
    writer.setFlags(SkOrderedWriteBuffer::kValidation_Flag);
    SerializationUtils<T>::Write(writer, data, kArraySize);
    size_t bytesWritten = writer.bytesWritten();
    // This should write the length (in 4 bytes) and the array
    REPORTER_ASSERT(reporter, (4 + kArraySize * sizeof(T)) == bytesWritten);

    unsigned char dataWritten[1024];
    writer.writeToMemory(dataWritten);

    // Make sure this fails when it should
    SkValidatingReadBuffer buffer(dataWritten, bytesWritten);
    T dataRead[kArraySize];
    bool success = SerializationUtils<T>::Read(buffer, dataRead, kArraySize / 2);
    // This should have failed, since the provided size was too small
    REPORTER_ASSERT(reporter, !success);

    // Make sure this succeeds when it should
    SkValidatingReadBuffer buffer2(dataWritten, bytesWritten);
    success = SerializationUtils<T>::Read(buffer2, dataRead, kArraySize);
    // This should have succeeded, since there are enough bytes to read this
    REPORTER_ASSERT(reporter, success);
}

static void Tests(skiatest::Reporter* reporter) {
    // Test matrix serialization
    {
        SkMatrix matrix = SkMatrix::I();
        TestObjectSerialization(&matrix, reporter);
     }

    // Test path serialization
    {
        SkPath path;
        TestObjectSerialization(&path, reporter);
    }

    // Test region serialization
    {
        SkRegion region;
        TestObjectSerialization(&region, reporter);
    }

    // Test rrect serialization
    {
        SkRRect rrect;
        TestAlignment(&rrect, reporter);
    }

    // Test readByteArray
    {
        unsigned char data[kArraySize] = {0};
        TestArraySerialization(data, reporter);
    }

    // Test readColorArray
    {
        SkColor data[kArraySize];
        TestArraySerialization(data, reporter);
    }

    // Test readIntArray
    {
        int32_t data[kArraySize];
        TestArraySerialization(data, reporter);
    }

    // Test readPointArray
    {
        SkPoint data[kArraySize];
        TestArraySerialization(data, reporter);
    }

    // Test readScalarArray
    {
        SkScalar data[kArraySize];
        TestArraySerialization(data, reporter);
    }
}

#include "TestClassDef.h"
DEFINE_TESTCLASS("Serialization", SerializationClass, Tests)
