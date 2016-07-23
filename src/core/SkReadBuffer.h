/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkReadBuffer_DEFINED
#define SkReadBuffer_DEFINED

#include "SkColorFilter.h"
#include "SkData.h"
#include "SkDrawLooper.h"
#include "SkImageFilter.h"
#include "SkMaskFilter.h"
#include "SkPath.h"
#include "SkPathEffect.h"
#include "SkPicture.h"
#include "SkRasterizer.h"
#include "SkReadBuffer.h"
#include "SkReader32.h"
#include "SkRefCnt.h"
#include "SkShader.h"
#include "SkTHash.h"
#include "SkWriteBuffer.h"
#include "SkXfermode.h"

class SkBitmap;
class SkImage;

#if defined(SK_DEBUG) && defined(SK_BUILD_FOR_MAC)
    #define DEBUG_NON_DETERMINISTIC_ASSERT
#endif

class SkReadBuffer {
public:
    SkReadBuffer();
    SkReadBuffer(const void* data, size_t size);
    SkReadBuffer(SkStream* stream);
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
        */
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
    };

    /**
     *  Returns true IFF the version is older than the specified version.
     */
    bool isVersionLT(Version targetVersion) const {
        SkASSERT(targetVersion > 0);
        return fVersion > 0 && fVersion < targetVersion;
    }

    /** This may be called at most once; most clients of SkReadBuffer should not mess with it. */
    void setVersion(int version) {
        SkASSERT(0 == fVersion || version == fVersion);
        fVersion = version;
    }

    enum Flags {
        kCrossProcess_Flag  = 1 << 0,
        kScalarIsFloat_Flag = 1 << 1,
        kPtrIs64Bit_Flag    = 1 << 2,
        kValidation_Flag    = 1 << 3,
    };

    void setFlags(uint32_t flags) { fFlags = flags; }
    uint32_t getFlags() const { return fFlags; }

    bool isCrossProcess() const {
        return this->isValidating() || SkToBool(fFlags & kCrossProcess_Flag);
    }
    bool isScalarFloat() const { return SkToBool(fFlags & kScalarIsFloat_Flag); }
    bool isPtr64Bit() const { return SkToBool(fFlags & kPtrIs64Bit_Flag); }
    bool isValidating() const { return SkToBool(fFlags & kValidation_Flag); }

    SkReader32* getReader32() { return &fReader; }

    size_t size() { return fReader.size(); }
    size_t offset() { return fReader.offset(); }
    bool eof() { return fReader.eof(); }
    virtual const void* skip(size_t size) { return fReader.skip(size); }

    // primitives
    virtual bool readBool();
    virtual SkColor readColor();
    virtual int32_t readInt();
    virtual SkScalar readScalar();
    virtual uint32_t readUInt();
    virtual int32_t read32();

    // peek
    virtual uint8_t peekByte();

    // strings -- the caller is responsible for freeing the string contents
    virtual void readString(SkString* string);

    // common data structures
    virtual void readPoint(SkPoint* point);
    SkPoint readPoint() { SkPoint p; this->readPoint(&p); return p; }
    virtual void readMatrix(SkMatrix* matrix);
    virtual void readIRect(SkIRect* rect);
    virtual void readRect(SkRect* rect);
    virtual void readRRect(SkRRect* rrect);
    virtual void readRegion(SkRegion* region);

    virtual void readPath(SkPath* path);
    void readPaint(SkPaint* paint) { paint->unflatten(*this); }

    virtual SkFlattenable* readFlattenable(SkFlattenable::Type);
    template <typename T> sk_sp<T> readFlattenable() {
        return sk_sp<T>((T*)this->readFlattenable(T::GetFlattenableType()));
    }
    sk_sp<SkColorFilter> readColorFilter() { return this->readFlattenable<SkColorFilter>(); }
    sk_sp<SkDrawLooper> readDrawLooper() { return this->readFlattenable<SkDrawLooper>(); }
    sk_sp<SkImageFilter> readImageFilter() { return this->readFlattenable<SkImageFilter>(); }
    sk_sp<SkMaskFilter> readMaskFilter() { return this->readFlattenable<SkMaskFilter>(); }
    sk_sp<SkPathEffect> readPathEffect() { return this->readFlattenable<SkPathEffect>(); }
    sk_sp<SkRasterizer> readRasterizer() { return this->readFlattenable<SkRasterizer>(); }
    sk_sp<SkShader> readShader() { return this->readFlattenable<SkShader>(); }
    sk_sp<SkXfermode> readXfermode() { return this->readFlattenable<SkXfermode>(); }

    // binary data and arrays
    virtual bool readByteArray(void* value, size_t size);
    virtual bool readColorArray(SkColor* colors, size_t size);
    virtual bool readIntArray(int32_t* values, size_t size);
    virtual bool readPointArray(SkPoint* points, size_t size);
    virtual bool readScalarArray(SkScalar* values, size_t size);

    sk_sp<SkData> readByteArrayAsData() {
        size_t len = this->getArrayCount();
        if (!this->validateAvailable(len)) {
            return SkData::MakeEmpty();
        }
        void* buffer = sk_malloc_throw(len);
        this->readByteArray(buffer, len);
        return SkData::MakeFromMalloc(buffer, len);
    }

    // helpers to get info about arrays and binary data
    virtual uint32_t getArrayCount();

    /**
     *  Returns false if the bitmap could not be completely read. In that case, it will be set
     *  to have width/height, but no pixels.
     */
    bool readBitmap(SkBitmap* bitmap);

    SkImage* readImage();

    virtual SkTypeface* readTypeface();

    void setTypefaceArray(SkTypeface* array[], int count) {
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

    /**
     *  Provide a function to decode an SkBitmap from encoded data. Only used if the writer
     *  encoded the SkBitmap. If the proper decoder cannot be used, a red bitmap with the
     *  appropriate size will be used.
     */
    void setBitmapDecoder(SkPicture::InstallPixelRefProc bitmapDecoder) {
        fBitmapDecoder = bitmapDecoder;
    }

    // Default impelementations don't check anything.
    virtual bool validate(bool isValid) { return isValid; }
    virtual bool isValid() const { return true; }
    virtual bool validateAvailable(size_t size) { return true; }
    bool validateIndex(int index, int count) {
        return this->validate(index >= 0 && index < count);
    }

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
    bool readArray(void* value, size_t size, size_t elementSize);

    uint32_t fFlags;
    int fVersion;

    void* fMemoryPtr;

    SkTypeface** fTFArray;
    int        fTFCount;

    SkFlattenable::Factory* fFactoryArray;
    int                     fFactoryCount;

    // Only used if we do not have an fFactoryArray.
    SkTHashMap<SkString, SkFlattenable::Factory> fCustomFactory;

    SkPicture::InstallPixelRefProc fBitmapDecoder;

#ifdef DEBUG_NON_DETERMINISTIC_ASSERT
    // Debugging counter to keep track of how many bitmaps we
    // have decoded.
    int fDecodedBitmapIndex;
#endif // DEBUG_NON_DETERMINISTIC_ASSERT
};

#endif // SkReadBuffer_DEFINED
