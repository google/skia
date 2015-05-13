/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Resources.h"
#include "SkBitmapSource.h"
#include "SkCanvas.h"
#include "SkFixed.h"
#include "SkFontDescriptor.h"
#include "SkMallocPixelRef.h"
#include "SkOSFile.h"
#include "SkPictureRecorder.h"
#include "SkTableColorFilter.h"
#include "SkTemplates.h"
#include "SkTypeface.h"
#include "SkWriteBuffer.h"
#include "SkValidatingReadBuffer.h"
#include "SkXfermodeImageFilter.h"
#include "Test.h"

static const uint32_t kArraySize = 64;
static const int kBitmapSize = 256;

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
    static void Write(SkWriteBuffer& writer, const T* flattenable) {
        writer.writeFlattenable(flattenable);
    }
    static void Read(SkValidatingReadBuffer& reader, T** flattenable) {
        *flattenable = (T*)reader.readFlattenable(T::GetFlattenableType());
    }
};

template<> struct SerializationUtils<SkMatrix> {
    static void Write(SkWriteBuffer& writer, const SkMatrix* matrix) {
        writer.writeMatrix(*matrix);
    }
    static void Read(SkValidatingReadBuffer& reader, SkMatrix* matrix) {
        reader.readMatrix(matrix);
    }
};

template<> struct SerializationUtils<SkPath> {
    static void Write(SkWriteBuffer& writer, const SkPath* path) {
        writer.writePath(*path);
    }
    static void Read(SkValidatingReadBuffer& reader, SkPath* path) {
        reader.readPath(path);
    }
};

template<> struct SerializationUtils<SkRegion> {
    static void Write(SkWriteBuffer& writer, const SkRegion* region) {
        writer.writeRegion(*region);
    }
    static void Read(SkValidatingReadBuffer& reader, SkRegion* region) {
        reader.readRegion(region);
    }
};

template<> struct SerializationUtils<SkString> {
    static void Write(SkWriteBuffer& writer, const SkString* string) {
        writer.writeString(string->c_str());
    }
    static void Read(SkValidatingReadBuffer& reader, SkString* string) {
        reader.readString(string);
    }
};

template<> struct SerializationUtils<unsigned char> {
    static void Write(SkWriteBuffer& writer, unsigned char* data, uint32_t arraySize) {
        writer.writeByteArray(data, arraySize);
    }
    static bool Read(SkValidatingReadBuffer& reader, unsigned char* data, uint32_t arraySize) {
        return reader.readByteArray(data, arraySize);
    }
};

template<> struct SerializationUtils<SkColor> {
    static void Write(SkWriteBuffer& writer, SkColor* data, uint32_t arraySize) {
        writer.writeColorArray(data, arraySize);
    }
    static bool Read(SkValidatingReadBuffer& reader, SkColor* data, uint32_t arraySize) {
        return reader.readColorArray(data, arraySize);
    }
};

template<> struct SerializationUtils<int32_t> {
    static void Write(SkWriteBuffer& writer, int32_t* data, uint32_t arraySize) {
        writer.writeIntArray(data, arraySize);
    }
    static bool Read(SkValidatingReadBuffer& reader, int32_t* data, uint32_t arraySize) {
        return reader.readIntArray(data, arraySize);
    }
};

template<> struct SerializationUtils<SkPoint> {
    static void Write(SkWriteBuffer& writer, SkPoint* data, uint32_t arraySize) {
        writer.writePointArray(data, arraySize);
    }
    static bool Read(SkValidatingReadBuffer& reader, SkPoint* data, uint32_t arraySize) {
        return reader.readPointArray(data, arraySize);
    }
};

template<> struct SerializationUtils<SkScalar> {
    static void Write(SkWriteBuffer& writer, SkScalar* data, uint32_t arraySize) {
        writer.writeScalarArray(data, arraySize);
    }
    static bool Read(SkValidatingReadBuffer& reader, SkScalar* data, uint32_t arraySize) {
        return reader.readScalarArray(data, arraySize);
    }
};

template<typename T, bool testInvalid> struct SerializationTestUtils {
    static void InvalidateData(unsigned char* data) {}
};

template<> struct SerializationTestUtils<SkString, true> {
    static void InvalidateData(unsigned char* data) {
        data[3] |= 0x80; // Reverse sign of 1st integer
    }
};

template<typename T, bool testInvalid>
static void TestObjectSerializationNoAlign(T* testObj, skiatest::Reporter* reporter) {
    SkWriteBuffer writer(SkWriteBuffer::kValidation_Flag);
    SerializationUtils<T>::Write(writer, testObj);
    size_t bytesWritten = writer.bytesWritten();
    REPORTER_ASSERT(reporter, SkAlign4(bytesWritten) == bytesWritten);

    unsigned char dataWritten[1024];
    writer.writeToMemory(dataWritten);

    SerializationTestUtils<T, testInvalid>::InvalidateData(dataWritten);

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
    REPORTER_ASSERT(reporter, buffer2.isValid() == !testInvalid);
    // Note: This following test should always succeed, regardless of whether the buffer is valid,
    // since if it is invalid, it will simply skip to the end, as if it had read the whole buffer.
    REPORTER_ASSERT(reporter, static_cast<size_t>(peekAfter - peekBefore) == bytesWritten);
}

template<typename T>
static void TestObjectSerialization(T* testObj, skiatest::Reporter* reporter) {
    TestObjectSerializationNoAlign<T, false>(testObj, reporter);
    TestAlignment(testObj, reporter);
}

template<typename T>
static T* TestFlattenableSerialization(T* testObj, bool shouldSucceed,
                                       skiatest::Reporter* reporter) {
    SkWriteBuffer writer(SkWriteBuffer::kValidation_Flag);
    SerializationUtils<T>::Write(writer, testObj);
    size_t bytesWritten = writer.bytesWritten();
    REPORTER_ASSERT(reporter, SkAlign4(bytesWritten) == bytesWritten);

    unsigned char dataWritten[4096];
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
        REPORTER_ASSERT(reporter, obj2);
    } else {
        // If the deserialization was supposed to fail, make sure it did
        REPORTER_ASSERT(reporter, !buffer.isValid());
        REPORTER_ASSERT(reporter, NULL == obj2);
    }

    return obj2; // Return object to perform further validity tests on it
}

template<typename T>
static void TestArraySerialization(T* data, skiatest::Reporter* reporter) {
    SkWriteBuffer writer(SkWriteBuffer::kValidation_Flag);
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
    SkAutoTUnref<SkBitmapSource> validBitmapSource(SkBitmapSource::Create(validBitmap));
    SkAutoTUnref<SkBitmapSource> invalidBitmapSource(SkBitmapSource::Create(invalidBitmap));
    SkAutoTUnref<SkXfermode> mode(SkXfermode::Create(SkXfermode::kSrcOver_Mode));
    SkAutoTUnref<SkXfermodeImageFilter> xfermodeImageFilter(
        SkXfermodeImageFilter::Create(mode, invalidBitmapSource, validBitmapSource));

    SkAutoTUnref<SkImageFilter> deserializedFilter(
        TestFlattenableSerialization<SkImageFilter>(
            xfermodeImageFilter, shouldSucceed, reporter));

    // Try to render a small bitmap using the invalid deserialized filter
    // to make sure we don't crash while trying to render it
    if (shouldSucceed) {
        SkBitmap bitmap;
        bitmap.allocN32Pixels(24, 24);
        SkCanvas canvas(bitmap);
        canvas.clear(0x00000000);
        SkPaint paint;
        paint.setImageFilter(deserializedFilter);
        canvas.clipRect(SkRect::MakeXYWH(0, 0, SkIntToScalar(24), SkIntToScalar(24)));
        canvas.drawBitmap(bitmap, 0, 0, &paint);
    }
}

static void TestXfermodeSerialization(skiatest::Reporter* reporter) {
    for (size_t i = 0; i <= SkXfermode::kLastMode; ++i) {
        if (i == SkXfermode::kSrcOver_Mode) {
            // skip SrcOver, as it is allowed to return NULL from Create()
            continue;
        }
        SkAutoTUnref<SkXfermode> mode(SkXfermode::Create(static_cast<SkXfermode::Mode>(i)));
        REPORTER_ASSERT(reporter, mode.get());
        SkAutoTUnref<SkXfermode> copy(
            TestFlattenableSerialization<SkXfermode>(mode.get(), true, reporter));
    }
}

static void TestColorFilterSerialization(skiatest::Reporter* reporter) {
    uint8_t table[256];
    for (int i = 0; i < 256; ++i) {
        table[i] = (i * 41) % 256;
    }
    SkAutoTUnref<SkColorFilter> colorFilter(SkTableColorFilter::Create(table));
    SkAutoTUnref<SkColorFilter> copy(
        TestFlattenableSerialization<SkColorFilter>(colorFilter.get(), true, reporter));
}

static SkBitmap draw_picture(SkPicture& picture) {
     SkBitmap bitmap;
     bitmap.allocN32Pixels(SkScalarCeilToInt(picture.cullRect().width()), 
                           SkScalarCeilToInt(picture.cullRect().height()));
     SkCanvas canvas(bitmap);
     picture.playback(&canvas);
     return bitmap;
}

static void compare_bitmaps(skiatest::Reporter* reporter,
                            const SkBitmap& b1, const SkBitmap& b2) {
    REPORTER_ASSERT(reporter, b1.width() == b2.width());
    REPORTER_ASSERT(reporter, b1.height() == b2.height());
    SkAutoLockPixels autoLockPixels1(b1);
    SkAutoLockPixels autoLockPixels2(b2);

    if ((b1.width() != b2.width()) ||
        (b1.height() != b2.height())) {
        return;
    }

    int pixelErrors = 0;
    for (int y = 0; y < b2.height(); ++y) {
        for (int x = 0; x < b2.width(); ++x) {
            if (b1.getColor(x, y) != b2.getColor(x, y))
                ++pixelErrors;
        }
    }
    REPORTER_ASSERT(reporter, 0 == pixelErrors);
}
static void serialize_and_compare_typeface(SkTypeface* typeface, const char* text,
                                           skiatest::Reporter* reporter)
{
    // Create a paint with the typeface.
    SkPaint paint;
    paint.setColor(SK_ColorGRAY);
    paint.setTextSize(SkIntToScalar(30));
    paint.setTypeface(typeface);

    // Paint some text.
    SkPictureRecorder recorder;
    SkIRect canvasRect = SkIRect::MakeWH(kBitmapSize, kBitmapSize);
    SkCanvas* canvas = recorder.beginRecording(SkIntToScalar(canvasRect.width()), 
                                               SkIntToScalar(canvasRect.height()), 
                                               NULL, 0);
    canvas->drawColor(SK_ColorWHITE);
    canvas->drawText(text, 2, 24, 32, paint);
    SkAutoTUnref<SkPicture> picture(recorder.endRecording());

    // Serlialize picture and create its clone from stream.
    SkDynamicMemoryWStream stream;
    picture->serialize(&stream);
    SkAutoTDelete<SkStream> inputStream(stream.detachAsStream());
    SkAutoTUnref<SkPicture> loadedPicture(SkPicture::CreateFromStream(inputStream.get()));

    // Draw both original and clone picture and compare bitmaps -- they should be identical.
    SkBitmap origBitmap = draw_picture(*picture);
    SkBitmap destBitmap = draw_picture(*loadedPicture);
    compare_bitmaps(reporter, origBitmap, destBitmap);
}

static void TestPictureTypefaceSerialization(skiatest::Reporter* reporter) {
    {
        // Load typeface from file to test CreateFromFile with index.
        SkString filename = GetResourcePath("/fonts/test.ttc");
        SkAutoTUnref<SkTypeface> typeface(SkTypeface::CreateFromFile(filename.c_str(), 1));
        if (!typeface) {
            SkDebugf("Could not run fontstream test because test.ttc not found.");
        } else {
            serialize_and_compare_typeface(typeface, "A!", reporter);
        }
    }

    {
        // Load typeface as stream to create with axis settings.
        SkAutoTDelete<SkStreamAsset> distortable(GetResourceAsStream("/fonts/Distortable.ttf"));
        if (!distortable) {
            SkDebugf("Could not run fontstream test because Distortable.ttf not found.");
        } else {
            SkFixed axis = SK_FixedSqrt2;
            SkAutoTUnref<SkTypeface> typeface(SkTypeface::CreateFromFontData(
                new SkFontData(distortable.detach(), 0, &axis, 1)));
            if (!typeface) {
                SkDebugf("Could not run fontstream test because Distortable.ttf not created.");
            } else {
                serialize_and_compare_typeface(typeface, "abc", reporter);
            }
        }
    }
}

static void setup_bitmap_for_canvas(SkBitmap* bitmap) {
    bitmap->allocN32Pixels(kBitmapSize, kBitmapSize);
}

static void make_checkerboard_bitmap(SkBitmap& bitmap) {
    setup_bitmap_for_canvas(&bitmap);

    SkCanvas canvas(bitmap);
    canvas.clear(0x00000000);
    SkPaint darkPaint;
    darkPaint.setColor(0xFF804020);
    SkPaint lightPaint;
    lightPaint.setColor(0xFF244484);
    const int i = kBitmapSize / 8;
    const SkScalar f = SkIntToScalar(i);
    for (int y = 0; y < kBitmapSize; y += i) {
        for (int x = 0; x < kBitmapSize; x += i) {
            canvas.save();
            canvas.translate(SkIntToScalar(x), SkIntToScalar(y));
            canvas.drawRect(SkRect::MakeXYWH(0, 0, f, f), darkPaint);
            canvas.drawRect(SkRect::MakeXYWH(f, 0, f, f), lightPaint);
            canvas.drawRect(SkRect::MakeXYWH(0, f, f, f), lightPaint);
            canvas.drawRect(SkRect::MakeXYWH(f, f, f, f), darkPaint);
            canvas.restore();
        }
    }
}

static void draw_something(SkCanvas* canvas) {
    SkPaint paint;
    SkBitmap bitmap;
    make_checkerboard_bitmap(bitmap);

    canvas->save();
    canvas->scale(0.5f, 0.5f);
    canvas->drawBitmap(bitmap, 0, 0, NULL);
    canvas->restore();

    paint.setAntiAlias(true);

    paint.setColor(SK_ColorRED);
    canvas->drawCircle(SkIntToScalar(kBitmapSize/2), SkIntToScalar(kBitmapSize/2), SkIntToScalar(kBitmapSize/3), paint);
    paint.setColor(SK_ColorBLACK);
    paint.setTextSize(SkIntToScalar(kBitmapSize/3));
    canvas->drawText("Picture", 7, SkIntToScalar(kBitmapSize/2), SkIntToScalar(kBitmapSize/4), paint);
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

    // Test xfermode serialization
    {
        TestXfermodeSerialization(reporter);
    }

    // Test color filter serialization
    {
        TestColorFilterSerialization(reporter);
    }

    // Test string serialization
    {
        SkString string("string");
        TestObjectSerializationNoAlign<SkString, false>(&string, reporter);
        TestObjectSerializationNoAlign<SkString, true>(&string, reporter);
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
        SkImageInfo info = SkImageInfo::MakeN32Premul(kBitmapSize, kBitmapSize);

        SkBitmap validBitmap;
        validBitmap.setInfo(info);

        // Create a bitmap with a really large height
        SkBitmap invalidBitmap;
        invalidBitmap.setInfo(info.makeWH(info.width(), 1000000000));

        // The deserialization should succeed, and the rendering shouldn't crash,
        // even when the device fails to initialize, due to its size
        TestBitmapSerialization(validBitmap, invalidBitmap, true, reporter);
    }

    // Test simple SkPicture serialization
    {
        SkPictureRecorder recorder;
        draw_something(recorder.beginRecording(SkIntToScalar(kBitmapSize),
                                               SkIntToScalar(kBitmapSize),
                                               NULL, 0));
        SkAutoTUnref<SkPicture> pict(recorder.endRecording());

        // Serialize picture
        SkWriteBuffer writer(SkWriteBuffer::kValidation_Flag);
        pict->flatten(writer);
        size_t size = writer.bytesWritten();
        SkAutoTMalloc<unsigned char> data(size);
        writer.writeToMemory(static_cast<void*>(data.get()));

        // Deserialize picture
        SkValidatingReadBuffer reader(static_cast<void*>(data.get()), size);
        SkAutoTUnref<SkPicture> readPict(
            SkPicture::CreateFromBuffer(reader));
        REPORTER_ASSERT(reporter, readPict.get());
    }

    TestPictureTypefaceSerialization(reporter);
}
