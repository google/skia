/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkReadBuffer_DEFINED
#define SkReadBuffer_DEFINED

#include "SkColorFilter.h"
#include "SkSerialProcs.h"
#include "SkDrawLooper.h"
#include "SkFont.h"
#include "SkImageFilter.h"
#include "SkMaskFilterBase.h"
#include "SkMixerBase.h"
#include "SkPaintPriv.h"
#include "SkPath.h"
#include "SkPathEffect.h"
#include "SkPicture.h"
#include "SkReader32.h"
#include "SkRefCnt.h"
#include "SkShaderBase.h"
#include "SkWriteBuffer.h"

class SkData;
class SkImage;

#ifndef SK_DISABLE_READBUFFER

class SkReadBuffer {
public:
    SkReadBuffer();
    SkReadBuffer(const void* data, size_t size);

    enum Version {
        kTileModeInBlurImageFilter_Version = 56,
        kTileInfoInSweepGradient_Version   = 57,
        k2PtConicalNoFlip_Version          = 58,
        kRemovePictureImageFilterLocalSpace = 59,
        kRemoveHeaderFlags_Version         = 60,
        kTwoColorDrawShadow_Version        = 61,
        kDontNegateImageSize_Version       = 62,
        kStoreImageBounds_Version          = 63,
        kRemoveOccluderFromBlurMaskFilter  = 64,
        kFloat4PaintColor_Version          = 65,
        kSaveBehind_Version                = 66,
        kSerializeFonts_Version            = 67,
        kPaintDoesntSerializeFonts_Version = 68,
    };

    /**
     *  Returns true IFF the version is older than the specified version.
     */
    bool isVersionLT(Version targetVersion) const {
        SkASSERT(targetVersion > 0);
        return fVersion > 0 && fVersion < targetVersion;
    }

    uint32_t getVersion() const { return fVersion; }

    /** This may be called at most once; most clients of SkReadBuffer should not mess with it. */
    void setVersion(int version) {
        SkASSERT(0 == fVersion || version == fVersion);
        fVersion = version;
    }

    size_t size() const { return fReader.size(); }
    size_t offset() const { return fReader.offset(); }
    bool eof() { return fReader.eof(); }
    const void* skip(size_t size);
    const void* skip(size_t count, size_t size);    // does safe multiply
    size_t available() const { return fReader.available(); }

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
    void readMatrix(SkMatrix* matrix);
    void readIRect(SkIRect* rect);
    void readRect(SkRect* rect);
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
    sk_sp<SkColorFilter> readColorFilter() { return this->readFlattenable<SkColorFilter>(); }
    sk_sp<SkDrawLooper> readDrawLooper() { return this->readFlattenable<SkDrawLooper>(); }
    sk_sp<SkImageFilter> readImageFilter() { return this->readFlattenable<SkImageFilter>(); }
    sk_sp<SkMaskFilter> readMaskFilter() { return this->readFlattenable<SkMaskFilterBase>(); }
    sk_sp<SkPathEffect> readPathEffect() { return this->readFlattenable<SkPathEffect>(); }
    sk_sp<SkShader> readShader() { return this->readFlattenable<SkShaderBase>(); }
    sk_sp<SkMixer> readMixer() { return this->readFlattenable<SkMixerBase>(); }

    // Reads SkAlign4(bytes), but will only copy bytes into the buffer.
    bool readPad32(void* buffer, size_t bytes);

    // binary data and arrays
    bool readByteArray(void* value, size_t size);
    bool readColorArray(SkColor* colors, size_t size);
    bool readColor4fArray(SkColor4f* colors, size_t size);
    bool readIntArray(int32_t* values, size_t size);
    bool readPointArray(SkPoint* points, size_t size);
    bool readScalarArray(SkScalar* values, size_t size);

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
        return this->validate(n <= (fReader.available() / sizeof(T)));
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

private:
    const char* readString(size_t* length);

    void setInvalid();
    bool readArray(void* value, size_t size, size_t elementSize);
    void setMemory(const void*, size_t);

    SkReader32 fReader;

    // Only used if we do not have an fFactoryArray.
    SkTHashMap<uint32_t, SkFlattenable::Factory> fFlattenableDict;

    int fVersion;

    sk_sp<SkTypeface>* fTFArray;
    int                fTFCount;

    SkFlattenable::Factory* fFactoryArray;
    int                     fFactoryCount;

    SkDeserialProcs fProcs;

    static bool IsPtrAlign4(const void* ptr) {
        return SkIsAlign4((uintptr_t)ptr);
    }

    bool fError = false;
};

#else // #ifndef SK_DISABLE_READBUFFER

class SkReadBuffer {
public:
    SkReadBuffer() {}
    SkReadBuffer(const void*, size_t) {}

    enum Version {
        kTileModeInBlurImageFilter_Version = 56,
        kTileInfoInSweepGradient_Version   = 57,
        k2PtConicalNoFlip_Version          = 58,
        kRemovePictureImageFilterLocalSpace = 59,
        kRemoveHeaderFlags_Version         = 60,
        kTwoColorDrawShadow_Version        = 61,
        kDontNegateImageSize_Version       = 62,
        kStoreImageBounds_Version          = 63,
        kRemoveOccluderFromBlurMaskFilter  = 64,
        kFloat4PaintColor_Version          = 65,
        kSaveBehind_Version                = 66,
        kSerializeFonts_Version            = 67,
        kPaintDoesntSerializeFonts_Version = 68,
    };

    bool isVersionLT(Version) const { return false; }
    uint32_t getVersion() const { return 0xffffffff; }
    void     setVersion(int) {}

    size_t size() const { return 0; }
    size_t offset() const { return 0; }
    bool eof() { return true; }
    size_t available() const { return 0; }

    const void* skip(size_t)         { return nullptr; }
    const void* skip(size_t, size_t) { return nullptr; }
    template <typename T> const T* skipT()       { return nullptr; }
    template <typename T> const T* skipT(size_t) { return nullptr; }

    bool     readBool()   { return 0; }
    SkColor  readColor()  { return 0; }
    int32_t  readInt()    { return 0; }
    SkScalar readScalar() { return 0; }
    uint32_t readUInt()   { return 0; }
    int32_t  read32()     { return 0; }

    template <typename T> T read32LE(T max) { return max; }

    uint8_t  peekByte()   { return 0; }

    void readColor4f(SkColor4f* out) { *out = SkColor4f{0,0,0,0}; }
    void readPoint  (SkPoint*   out) { *out = SkPoint{0,0};       }
    void readPoint3 (SkPoint3*  out) { *out = SkPoint3{0,0,0};    }
    void readMatrix (SkMatrix*  out) { *out = SkMatrix::I();      }
    void readIRect  (SkIRect*   out) { *out = SkIRect{0,0,0,0};   }
    void readRect   (SkRect*    out) { *out = SkRect{0,0,0,0};    }
    void readRRect  (SkRRect*   out) { *out = SkRRect();          }
    void readRegion (SkRegion*  out) { *out = SkRegion();         }
    void readString (SkString*  out) { *out = SkString();         }
    void readPath   (SkPath*    out) { *out = SkPath();           }
    SkReadPaintResult readPaint  (SkPaint*   out, SkFont* font) {
        *out = SkPaint();
        if (font) {
            *font = SkFont();
        }
        return kFailed_ReadPaint;
    }

    SkPoint readPoint() { return {0,0}; }

    SkFlattenable* readFlattenable(SkFlattenable::Type) { return nullptr; }

    template <typename T> sk_sp<T> readFlattenable() { return nullptr; }
    sk_sp<SkColorFilter> readColorFilter() { return nullptr; }
    sk_sp<SkDrawLooper>  readDrawLooper()  { return nullptr; }
    sk_sp<SkImageFilter> readImageFilter() { return nullptr; }
    sk_sp<SkMaskFilter>  readMaskFilter()  { return nullptr; }
    sk_sp<SkPathEffect>  readPathEffect()  { return nullptr; }
    sk_sp<SkShader>      readShader()      { return nullptr; }
    sk_sp<SkMixer>       readMixer()       { return nullptr; }

    bool readPad32       (void*,      size_t) { return false; }
    bool readByteArray   (void*,      size_t) { return false; }
    bool readColorArray  (SkColor*,   size_t) { return false; }
    bool readColor4fArray(SkColor4f*, size_t) { return false; }
    bool readIntArray    (int32_t*,   size_t) { return false; }
    bool readPointArray  (SkPoint*,   size_t) { return false; }
    bool readScalarArray (SkScalar*,  size_t) { return false; }

    sk_sp<SkData> readByteArrayAsData() { return nullptr; }
    uint32_t getArrayCount() { return 0; }

    sk_sp<SkImage>    readImage()    { return nullptr; }
    sk_sp<SkTypeface> readTypeface() { return nullptr; }

    bool validate(bool)                                 { return false; }
    template <typename T> bool validateCanReadN(size_t) { return false; }
    bool isValid() const                                { return false; }
    bool validateIndex(int, int)                        { return false; }

    int32_t checkInt(int min, int)               { return min; }
    template <typename T> T checkRange(T min, T) { return min; }

    SkFilterQuality checkFilterQuality() { return SkFilterQuality::kNone_SkFilterQuality; }

    void setTypefaceArray(sk_sp<SkTypeface>[], int)        {}
    void setFactoryPlayback(SkFlattenable::Factory[], int) {}
    void setDeserialProcs(const SkDeserialProcs&)          {}

    const SkDeserialProcs& getDeserialProcs() const {
        static const SkDeserialProcs procs;
        return procs;
    }
};

#endif // #ifndef SK_DISABLE_READBUFFER

#endif // SkReadBuffer_DEFINED
