/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkReadBuffer_DEFINED
#define SkReadBuffer_DEFINED

#include "include/core/SkFont.h"
#include "include/core/SkImageFilter.h"
#include "include/core/SkPath.h"
#include "include/core/SkPathEffect.h"
#include "include/core/SkPicture.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSerialProcs.h"
#include "src/core/SkColorFilterBase.h"
#include "src/core/SkMaskFilterBase.h"
#include "src/core/SkPaintPriv.h"
#include "src/core/SkPicturePriv.h"
#include "src/core/SkWriteBuffer.h"
#include "src/shaders/SkShaderBase.h"

#ifdef SK_SUPPORT_LEGACY_DRAWLOOPER
#include "include/core/SkDrawLooper.h"
#endif

class SkData;
class SkImage;

class SkReadBuffer {
public:
    SkReadBuffer() = default;
    SkReadBuffer(const void* data, size_t size) {
        this->setMemory(data, size);
    }

    void setMemory(const void*, size_t);

    /**
     *  Returns true IFF the version is older than the specified version.
     */
    bool isVersionLT(SkPicturePriv::Version targetVersion) const {
        SkASSERT(targetVersion > 0);
        return fVersion > 0 && fVersion < targetVersion;
    }

    uint32_t getVersion() const { return fVersion; }

    /** This may be called at most once; most clients of SkReadBuffer should not mess with it. */
    void setVersion(int version) {
        SkASSERT(0 == fVersion || version == fVersion);
        fVersion = version;
    }

    size_t size() const { return fStop - fBase; }
    size_t offset() const { return fCurr - fBase; }
    bool eof() { return fCurr >= fStop; }
    const void* skip(size_t size);
    const void* skip(size_t count, size_t size);    // does safe multiply
    size_t available() const { return fStop - fCurr; }

    template <typename T> const T* skipT() {
        return static_cast<const T*>(this->skip(sizeof(T)));
    }
    template <typename T> const T* skipT(size_t count) {
        return static_cast<const T*>(this->skip(count, sizeof(T)));
    }

    // primitives
    bool readBool();
    SkColor readColor();
    int32_t readInt();
    SkScalar readScalar();
    uint32_t readUInt();
    int32_t read32();

    template <typename T> T read32LE(T max) {
        uint32_t value = this->readUInt();
        if (!this->validate(value <= static_cast<uint32_t>(max))) {
            value = 0;
        }
        return static_cast<T>(value);
    }

    // peek
    uint8_t peekByte();

    void readString(SkString* string);

    // common data structures
    void readColor4f(SkColor4f* color);
    void readPoint(SkPoint* point);
    SkPoint readPoint() { SkPoint p; this->readPoint(&p); return p; }
    void readPoint3(SkPoint3* point);
    void read(SkM44*);
    void readMatrix(SkMatrix* matrix);
    void readIRect(SkIRect* rect);
    void readRect(SkRect* rect);
    SkRect readRect();
    void readRRect(SkRRect* rrect);
    void readRegion(SkRegion* region);

    void readPath(SkPath* path);

    SkReadPaintResult readPaint(SkPaint* paint, SkFont* font) {
        return SkPaintPriv::Unflatten(paint, *this, font);
    }

    SkFlattenable* readFlattenable(SkFlattenable::Type);
    template <typename T> sk_sp<T> readFlattenable() {
        return sk_sp<T>((T*)this->readFlattenable(T::GetFlattenableType()));
    }
    sk_sp<SkColorFilter> readColorFilter() { return this->readFlattenable<SkColorFilterBase>(); }
#ifdef SK_SUPPORT_LEGACY_DRAWLOOPER
    sk_sp<SkDrawLooper> readDrawLooper() { return this->readFlattenable<SkDrawLooper>(); }
#endif
    sk_sp<SkImageFilter> readImageFilter() { return this->readFlattenable<SkImageFilter>(); }
    sk_sp<SkMaskFilter> readMaskFilter() { return this->readFlattenable<SkMaskFilterBase>(); }
    sk_sp<SkPathEffect> readPathEffect() { return this->readFlattenable<SkPathEffect>(); }
    sk_sp<SkShader> readShader() { return this->readFlattenable<SkShaderBase>(); }

    // Reads SkAlign4(bytes), but will only copy bytes into the buffer.
    bool readPad32(void* buffer, size_t bytes);

    // binary data and arrays
    bool readByteArray(void* value, size_t size);
    bool readColorArray(SkColor* colors, size_t size);
    bool readColor4fArray(SkColor4f* colors, size_t size);
    bool readIntArray(int32_t* values, size_t size);
    bool readPointArray(SkPoint* points, size_t size);
    bool readScalarArray(SkScalar* values, size_t size);

    const void* skipByteArray(size_t* size);

    sk_sp<SkData> readByteArrayAsData();

    // helpers to get info about arrays and binary data
    uint32_t getArrayCount();

    // If there is a real error (e.g. data is corrupted) this returns null. If the image cannot
    // be created (e.g. it was not originally encoded) then this returns an image that doesn't
    // draw.
    sk_sp<SkImage> readImage();
    sk_sp<SkTypeface> readTypeface();

    void setTypefaceArray(sk_sp<SkTypeface> array[], int count) {
        fTFArray = array;
        fTFCount = count;
    }

    /**
     *  Call this with a pre-loaded array of Factories, in the same order as
     *  were created/written by the writer. SkPicture uses this.
     */
    void setFactoryPlayback(SkFlattenable::Factory array[], int count) {
        fFactoryArray = array;
        fFactoryCount = count;
    }

    void setDeserialProcs(const SkDeserialProcs& procs);
    const SkDeserialProcs& getDeserialProcs() const { return fProcs; }

    /**
     *  If isValid is false, sets the buffer to be "invalid". Returns true if the buffer
     *  is still valid.
     */
    bool validate(bool isValid) {
        if (!isValid) {
            this->setInvalid();
        }
        return !fError;
    }

    /**
     * Helper function to do a preflight check before a large allocation or read.
     * Returns true if there is enough bytes in the buffer to read n elements of T.
     * If not, the buffer will be "invalid" and false will be returned.
     */
    template <typename T>
    bool validateCanReadN(size_t n) {
        return this->validate(n <= (this->available() / sizeof(T)));
    }

    bool isValid() const { return !fError; }
    bool validateIndex(int index, int count) {
        return this->validate(index >= 0 && index < count);
    }

    // Utilities that mark the buffer invalid if the requested value is out-of-range

    // If the read value is outside of the range, validate(false) is called, and min
    // is returned, else the value is returned.
    int32_t checkInt(int min, int max);

    template <typename T> T checkRange(T min, T max) {
        return static_cast<T>(this->checkInt(static_cast<int32_t>(min),
                                             static_cast<int32_t>(max)));
    }

    SkFilterQuality checkFilterQuality();

    SkSamplingOptions readSampling();

private:
    const char* readString(size_t* length);

    void setInvalid();
    bool readArray(void* value, size_t size, size_t elementSize);
    bool isAvailable(size_t size) const { return size <= this->available(); }

    sk_sp<SkImage> readImage_preV78();

    // These are always 4-byte aligned
    const char* fCurr = nullptr;  // current position within buffer
    const char* fStop = nullptr;  // end of buffer
    const char* fBase = nullptr;  // beginning of buffer

    // Only used if we do not have an fFactoryArray.
    SkTHashMap<uint32_t, SkFlattenable::Factory> fFlattenableDict;

    int fVersion = 0;

    sk_sp<SkTypeface>* fTFArray = nullptr;
    int                fTFCount = 0;

    SkFlattenable::Factory* fFactoryArray = nullptr;
    int                     fFactoryCount = 0;

    SkDeserialProcs fProcs;

    static bool IsPtrAlign4(const void* ptr) {
        return SkIsAlign4((uintptr_t)ptr);
    }

    bool fError = false;
};

#endif // SkReadBuffer_DEFINED
