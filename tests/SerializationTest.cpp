/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"
#include "TestClassDef.h"
#include "SkBitmapDevice.h"
#include "SkBitmapSource.h"
#include "SkCanvas.h"
#include "SkMallocPixelRef.h"
#include "SkOrderedWriteBuffer.h"
#include "SkValidatingReadBuffer.h"
#include "SkXfermodeImageFilter.h"

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
    // Generic case for flattenables
    static void Write(SkOrderedWriteBuffer& writer, const T* flattenable) {
        writer.writeFlattenable(flattenable);
    }
    static void Read(SkValidatingReadBuffer& reader, T** flattenable) {
        *flattenable = (T*)reader.readFlattenable(T::GetFlattenableType());
    }
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
    REPORTER_ASSERT(reporter, SkAlign4(bytesWritten) == bytesWritten);

    unsigned char dataWritten[1024];
    writer.writeToMemory(dataWritten);

    // Make sure this fails when it should (test with smaller size, but still multiple of 4)
    SkValidatingReadBuffer buffer(dataWritten, bytesWritten - 4);
    T obj;
    SerializationUtils<T>::Read(buffer, &obj);
    REPORTER_ASSERT(reporter, !buffer.isValid());

    // Make sure this succeeds when it should
    SkValidatingReadBuffer buffer2(dataWritten, bytesWritten);
    const unsigned char* peekBefore = static_cast<const unsigned char*>(buffer2.skip(0));
    T obj2;
    SerializationUtils<T>::Read(buffer2, &obj2);
    const unsigned char* peekAfter = static_cast<const unsigned char*>(buffer2.skip(0));
    // This should have succeeded, since there are enough bytes to read this
    REPORTER_ASSERT(reporter, buffer2.isValid());
    REPORTER_ASSERT(reporter, static_cast<size_t>(peekAfter - peekBefore) == bytesWritten);

    TestAlignment(testObj, reporter);
}

template<typename T>
static T* TestFlattenableSerialization(T* testObj, bool shouldSucceed,
                                       skiatest::Reporter* reporter) {
    SkOrderedWriteBuffer writer(1024);
    writer.setFlags(SkOrderedWriteBuffer::kValidation_Flag);
    SerializationUtils<T>::Write(writer, testObj);
    size_t bytesWritten = writer.bytesWritten();
    REPORTER_ASSERT(reporter, SkAlign4(bytesWritten) == bytesWritten);

    unsigned char dataWritten[1024];
    SkASSERT(bytesWritten <= sizeof(dataWritten));
    writer.writeToMemory(dataWritten);

    // Make sure this fails when it should (test with smaller size, but still multiple of 4)
    SkValidatingReadBuffer buffer(dataWritten, bytesWritten - 4);
    T* obj = NULL;
    SerializationUtils<T>::Read(buffer, &obj);
    REPORTER_ASSERT(reporter, !buffer.isValid());
    REPORTER_ASSERT(reporter, NULL == obj);

    // Make sure this succeeds when it should
    SkValidatingReadBuffer buffer2(dataWritten, bytesWritten);
    const unsigned char* peekBefore = static_cast<const unsigned char*>(buffer2.skip(0));
    T* obj2 = NULL;
    SerializationUtils<T>::Read(buffer2, &obj2);
    const unsigned char* peekAfter = static_cast<const unsigned char*>(buffer2.skip(0));
    if (shouldSucceed) {
        // This should have succeeded, since there are enough bytes to read this
        REPORTER_ASSERT(reporter, buffer2.isValid());
        REPORTER_ASSERT(reporter, static_cast<size_t>(peekAfter - peekBefore) == bytesWritten);
        REPORTER_ASSERT(reporter, NULL != obj2);
    } else {
        // If the deserialization was supposed to fail, make sure it did
        REPORTER_ASSERT(reporter, !buffer.isValid());
        REPORTER_ASSERT(reporter, NULL == obj2);
    }

    return obj2; // Return object to perform further validity tests on it
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

static void TestBitmapSerialization(const SkBitmap& validBitmap,
                                    const SkBitmap& invalidBitmap,
                                    bool shouldSucceed,
                                    skiatest::Reporter* reporter) {
    SkBitmapSource validBitmapSource(validBitmap);
    SkBitmapSource invalidBitmapSource(invalidBitmap);
    SkAutoTUnref<SkXfermode> mode(SkXfermode::Create(SkXfermode::kSrcOver_Mode));
    SkXfermodeImageFilter xfermodeImageFilter(mode, &invalidBitmapSource, &validBitmapSource);

    SkAutoTUnref<SkImageFilter> deserializedFilter(
        TestFlattenableSerialization<SkImageFilter>(
            &xfermodeImageFilter, shouldSucceed, reporter));

    // Try to render a small bitmap using the invalid deserialized filter
    // to make sure we don't crash while trying to render it
    if (shouldSucceed) {
        SkBitmap bitmap;
        bitmap.setConfig(SkBitmap::kARGB_8888_Config, 24, 24);
        bitmap.allocPixels();
        SkBitmapDevice device(bitmap);
        SkCanvas canvas(&device);
        canvas.clear(0x00000000);
        SkPaint paint;
        paint.setImageFilter(deserializedFilter);
        canvas.clipRect(SkRect::MakeXYWH(0, 0, SkIntToScalar(24), SkIntToScalar(24)));
        canvas.drawBitmap(bitmap, 0, 0, &paint);
    }
}

DEF_TEST(Serialization, reporter) {
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
        // SkRRect does not initialize anything.
        // An uninitialized SkRRect can be serialized,
        // but will branch on uninitialized data when deserialized.
        SkRRect rrect;
        SkRect rect = SkRect::MakeXYWH(1, 2, 20, 30);
        SkVector corners[4] = { {1, 2}, {2, 3}, {3,4}, {4,5} };
        rrect.setRectRadii(rect, corners);
        TestAlignment(&rrect, reporter);
    }

    // Test readByteArray
    {
        unsigned char data[kArraySize] = { 1, 2, 3 };
        TestArraySerialization(data, reporter);
    }

    // Test readColorArray
    {
        SkColor data[kArraySize] = { SK_ColorBLACK, SK_ColorWHITE, SK_ColorRED };
        TestArraySerialization(data, reporter);
    }

    // Test readIntArray
    {
        int32_t data[kArraySize] = { 1, 2, 4, 8 };
        TestArraySerialization(data, reporter);
    }

    // Test readPointArray
    {
        SkPoint data[kArraySize] = { {6, 7}, {42, 128} };
        TestArraySerialization(data, reporter);
    }

    // Test readScalarArray
    {
        SkScalar data[kArraySize] = { SK_Scalar1, SK_ScalarHalf, SK_ScalarMax };
        TestArraySerialization(data, reporter);
    }

    // Test invalid deserializations
    {
        SkBitmap validBitmap;
        validBitmap.setConfig(SkBitmap::kARGB_8888_Config, 256, 256);

        // Create a bitmap with a really large height
        SkBitmap invalidBitmap;
        invalidBitmap.setConfig(SkBitmap::kARGB_8888_Config, 256, 1000000000);

        // The deserialization should succeed, and the rendering shouldn't crash,
        // even when the device fails to initialize, due to its size
        TestBitmapSerialization(validBitmap, invalidBitmap, true, reporter);

        // Create a bitmap with a pixel ref too small
        SkImageInfo info;
        info.fWidth = 256;
        info.fHeight = 256;
        info.fColorType = kPMColor_SkColorType;
        info.fAlphaType = kPremul_SkAlphaType;

        SkBitmap invalidBitmap2;
        invalidBitmap2.setConfig(info);
        
        // Hack to force invalid, by making the pixelref smaller than its
        // owning bitmap.
        info.fWidth = 32;
        info.fHeight = 1;
        
        invalidBitmap2.setPixelRef(SkMallocPixelRef::NewAllocate(
                        info, invalidBitmap2.rowBytes(), NULL))->unref();

        // The deserialization should detect the pixel ref being too small and fail
        TestBitmapSerialization(validBitmap, invalidBitmap2, false, reporter);
    }
}
