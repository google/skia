/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkImage_Lazy_DEFINED
#define SkImage_Lazy_DEFINED

#include "include/core/SkColorSpace.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageGenerator.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkTypes.h"
#include "include/core/SkYUVAPixmaps.h"
#include "include/private/SkIDChangeListener.h"
#include "include/private/base/SkMutex.h"
#include "src/image/SkImage_Base.h"

#include <cstddef>
#include <cstdint>
#include <memory>

class GrDirectContext;
class GrRecordingContext;
class SharedGenerator;
class SkBitmap;
class SkCachedData;
class SkData;
class SkPixmap;
class SkSurface;
enum SkColorType : int;
struct SkIRect;

namespace skgpu { namespace graphite { class Recorder; } }

class SkImage_Lazy : public SkImage_Base {
public:
    struct Validator {
        Validator(sk_sp<SharedGenerator>, const SkColorType*, sk_sp<SkColorSpace>);

        explicit operator bool() const { return fSharedGenerator.get(); }

        sk_sp<SharedGenerator> fSharedGenerator;
        SkImageInfo            fInfo;
        sk_sp<SkColorSpace>    fColorSpace;
        uint32_t               fUniqueID;
    };

    SkImage_Lazy(Validator* validator);

    // From SkImage.h
    bool isValid(GrRecordingContext*) const override;

    // From SkImage_Base.h
    bool onHasMipmaps() const override {
        // TODO: Should we defer to the generator? The generator interface currently doesn't have
        // a way to provide content for levels other than via SkImageGenerator::generateTexture().
        return false;
    }
    bool onIsProtected() const override;

    bool onReadPixels(GrDirectContext*, const SkImageInfo&, void*, size_t, int srcX, int srcY,
                      CachingHint) const override;
    sk_sp<SkData> onRefEncoded() const override;
    sk_sp<SkImage> onMakeSubset(GrDirectContext*, const SkIRect&) const override;
    sk_sp<SkImage> onMakeSubset(skgpu::graphite::Recorder*,
                                const SkIRect&,
                                RequiredProperties) const override;

    sk_sp<SkSurface> onMakeSurface(skgpu::graphite::Recorder*, const SkImageInfo&) const override;

    bool getROPixels(GrDirectContext*, SkBitmap*, CachingHint) const override;
    SkImage_Base::Type type() const override { return SkImage_Base::Type::kLazy; }
    sk_sp<SkImage> onMakeColorTypeAndColorSpace(SkColorType, sk_sp<SkColorSpace>,
                                                GrDirectContext*) const override;
    sk_sp<SkImage> onReinterpretColorSpace(sk_sp<SkColorSpace>) const final;

    void addUniqueIDListener(sk_sp<SkIDChangeListener>) const;
    sk_sp<SkCachedData> getPlanes(const SkYUVAPixmapInfo::SupportedDataTypes& supportedDataTypes,
                                  SkYUVAPixmaps* pixmaps) const;


    // Be careful with this. You need to acquire the mutex, as the generator might be shared
    // among several images.
    sk_sp<SharedGenerator> generator() const;
protected:
    virtual bool readPixelsProxy(GrDirectContext*, const SkPixmap&) const { return false; }

private:

    class ScopedGenerator;

    // Note that this->imageInfo() is not necessarily the info from the generator. It may be
    // cropped by onMakeSubset and its color type/space may be changed by
    // onMakeColorTypeAndColorSpace.
    sk_sp<SharedGenerator> fSharedGenerator;

    // Repeated calls to onMakeColorTypeAndColorSpace will result in a proliferation of unique IDs
    // and SkImage_Lazy instances. Cache the result of the last successful call.
    mutable SkMutex        fOnMakeColorTypeAndSpaceMutex;
    mutable sk_sp<SkImage> fOnMakeColorTypeAndSpaceResult;
    // When the SkImage_Lazy goes away, we will iterate over all the listeners to inform them
    // of the unique ID's demise. This is used to remove cached textures from GrContext.
    mutable SkIDChangeListener::List fUniqueIDListeners;
};

// Ref-counted tuple(SkImageGenerator, SkMutex) which allows sharing one generator among N images
class SharedGenerator final : public SkNVRefCnt<SharedGenerator> {
public:
    static sk_sp<SharedGenerator> Make(std::unique_ptr<SkImageGenerator> gen);

    // This is thread safe.  It is a const field set in the constructor.
    const SkImageInfo& getInfo() const;

    bool isTextureGenerator();

    std::unique_ptr<SkImageGenerator> fGenerator;
    SkMutex                           fMutex;

private:
    explicit SharedGenerator(std::unique_ptr<SkImageGenerator> gen);
};

#endif
