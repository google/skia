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
#include "SkImageFilter.h"
#include "SkMaskFilterBase.h"
#include "SkPaintPriv.h"
#include "SkPath.h"
#include "SkPathEffect.h"
#include "SkPicture.h"
#include "SkReader32.h"
#include "SkRefCnt.h"
#include "SkShaderBase.h"
#include "SkTHash.h"
#include "SkWriteBuffer.h"

class SkData;
class SkImage;
class SkInflator;

#if defined(SK_DEBUG) && defined(SK_BUILD_FOR_MAC)
    #define DEBUG_NON_DETERMINISTIC_ASSERT
#endif

class SkReadBuffer {
public:
    SkReadBuffer();
    SkReadBuffer(const void* data, size_t size);
    virtual ~SkReadBuffer();

    virtual SkReadBuffer* clone(const void* data, size_t size) const {
        return new SkReadBuffer(data, size);
    }

    enum Version {
        /*
        kFilterLevelIsEnum_Version         = 23,
        kGradientFlippedFlag_Version       = 24,
        kDashWritesPhaseIntervals_Version  = 25,
        kColorShaderNoBool_Version         = 26,
        kNoUnitMappers_Version             = 27,
        kNoMoreBitmapFlatten_Version       = 28,
        kSimplifyLocalMatrix_Version       = 30,
        kImageFilterUniqueID_Version       = 31,
        kRemoveAndroidPaintOpts_Version    = 32,
        kFlattenCreateProc_Version         = 33,
        kRemoveColorTableAlpha_Version     = 36,
        kDropShadowMode_Version            = 37,
        kPictureImageFilterResolution_Version = 38,
        kPictureImageFilterLevel_Version   = 39,
        kImageFilterNoUniqueID_Version     = 40,
        kBitmapSourceFilterQuality_Version = 41,
        kPictureShaderHasPictureBool_Version = 42,
        kHasDrawImageOpCodes_Version       = 43,
        kAnnotationsMovedToCanvas_Version  = 44,
        kLightingShaderWritesInvNormRotation = 45,
        kBlurMaskFilterWritesOccluder      = 47,
        kGradientShaderFloatColor_Version  = 49,
        kXfermodeToBlendMode_Version       = 50,
        kXfermodeToBlendMode2_Version      = 51,
        kTextBlobImplicitRunCount_Version  = 52,
        kComposeShaderCanLerp_Version      = 54,
        kNoModesInMergeImageFilter_Verison = 55,
         */
        kTileModeInBlurImageFilter_Version = 56,
        kTileInfoInSweepGradient_Version   = 57,
        k2PtConicalNoFlip_Version          = 58,
        kRemovePictureImageFilterLocalSpace = 59,
        kRemoveHeaderFlags_Version         = 60,
        kTwoColorDrawShadow_Version        = 61,
        kDontNegateImageSize_Version       = 62,
        kStoreImageBounds_Version          = 63,
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

    // strings -- the caller is responsible for freeing the string contents
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
    virtual bool readPaint(SkPaint* paint) { return SkPaintPriv::Unflatten(paint, *this); }

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

    /**
     *  For an input flattenable (specified by name), set a custom factory proc
     *  to use when unflattening.  Will make a copy of |name|.
     *
     *  If the global registry already has a default factory for the flattenable,
     *  this will override that factory.  If a custom factory has already been
     *  set for the flattenable, this will override that factory.
     *
     *  Custom factories can be removed by calling setCustomFactory("...", nullptr).
     */
    void setCustomFactory(const SkString& name, SkFlattenable::Factory factory) {
        fCustomFactory.set(name, factory);
    }

    void setDeserialProcs(const SkDeserialProcs& procs);

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

    SkInflator* getInflator() const { return fInflator; }
    void setInflator(SkInflator* inf) { fInflator = inf; }

    // Utilities that mark the buffer invalid if the requested value is out-of-range

    // If the read value is outside of the range, validate(false) is called, and min
    // is returned, else the value is returned.
    int32_t checkInt(int min, int max);

    template <typename T> T checkRange(T min, T max) {
        return static_cast<T>(this->checkInt(static_cast<int32_t>(min),
                                             static_cast<int32_t>(max)));
    }

    SkFilterQuality checkFilterQuality();

protected:
    /**
     *  Allows subclass to check if we are using factories for expansion
     *  of flattenables.
     */
    int factoryCount() { return fFactoryCount; }

    /**
     *  Checks if a custom factory has been set for a given flattenable.
     *  Returns the custom factory if it exists, or nullptr otherwise.
     */
    SkFlattenable::Factory getCustomFactory(const SkString& name) {
        SkFlattenable::Factory* factoryPtr = fCustomFactory.find(name);
        return factoryPtr ? *factoryPtr : nullptr;
    }

    SkReader32 fReader;

    // Only used if we do not have an fFactoryArray.
    SkTHashMap<uint32_t, SkString> fFlattenableDict;

private:
    void setInvalid();
    bool readArray(void* value, size_t size, size_t elementSize);
    void setMemory(const void*, size_t);

    int fVersion;

    void* fMemoryPtr;

    sk_sp<SkTypeface>* fTFArray;
    int                fTFCount;

    SkFlattenable::Factory* fFactoryArray;
    int                     fFactoryCount;

    // Only used if we do not have an fFactoryArray.
    SkTHashMap<SkString, SkFlattenable::Factory> fCustomFactory;

    SkDeserialProcs fProcs;
    friend class SkPicturePriv;

#ifdef DEBUG_NON_DETERMINISTIC_ASSERT
    // Debugging counter to keep track of how many bitmaps we
    // have decoded.
    int fDecodedBitmapIndex;
#endif // DEBUG_NON_DETERMINISTIC_ASSERT

    SkInflator* fInflator = nullptr;

    static bool IsPtrAlign4(const void* ptr) {
        return SkIsAlign4((uintptr_t)ptr);
    }

    bool fError = false;
};

#endif // SkReadBuffer_DEFINED
