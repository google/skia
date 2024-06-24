/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/image/SkImage_Lazy.h"

#include "include/core/SkBitmap.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkData.h"
#include "include/core/SkImageGenerator.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkSize.h"
#include "include/core/SkSurface.h"
#include "include/core/SkYUVAInfo.h"
#include "src/core/SkBitmapCache.h"
#include "src/core/SkCachedData.h"
#include "src/core/SkNextID.h"
#include "src/core/SkResourceCache.h"
#include "src/core/SkYUVPlanesCache.h"

#include <utility>

class SkSurfaceProps;

enum SkColorType : int;

sk_sp<SharedGenerator> SharedGenerator::Make(std::unique_ptr<SkImageGenerator> gen) {
    return gen ? sk_sp<SharedGenerator>(new SharedGenerator(std::move(gen))) : nullptr;
}

SharedGenerator::SharedGenerator(std::unique_ptr<SkImageGenerator> gen)
        : fGenerator(std::move(gen)) {
    SkASSERT(fGenerator);
}

const SkImageInfo& SharedGenerator::getInfo() const { return fGenerator->getInfo(); }

bool SharedGenerator::isTextureGenerator() { return fGenerator->isTextureGenerator(); }

///////////////////////////////////////////////////////////////////////////////

SkImage_Lazy::Validator::Validator(sk_sp<SharedGenerator> gen, const SkColorType* colorType,
                                   sk_sp<SkColorSpace> colorSpace)
        : fSharedGenerator(std::move(gen)) {
    if (!fSharedGenerator) {
        return;
    }

    // The following generator accessors are safe without acquiring the mutex (const getters).
    // TODO: refactor to use a ScopedGenerator instead, for clarity.
    fInfo = fSharedGenerator->fGenerator->getInfo();
    if (fInfo.isEmpty()) {
        fSharedGenerator.reset();
        return;
    }

    fUniqueID = fSharedGenerator->fGenerator->uniqueID();

    if (colorType && (*colorType == fInfo.colorType())) {
        colorType = nullptr;
    }

    if (colorType || colorSpace) {
        if (colorType) {
            fInfo = fInfo.makeColorType(*colorType);
        }
        if (colorSpace) {
            fInfo = fInfo.makeColorSpace(colorSpace);
        }
        fUniqueID = SkNextID::ImageID();
    }
}

///////////////////////////////////////////////////////////////////////////////

// Helper for exclusive access to a shared generator.
class SkImage_Lazy::ScopedGenerator {
public:
    ScopedGenerator(const sk_sp<SharedGenerator>& gen)
      : fSharedGenerator(gen)
      , fAutoAcquire(gen->fMutex) {}

    SkImageGenerator* operator->() const {
        fSharedGenerator->fMutex.assertHeld();
        return fSharedGenerator->fGenerator.get();
    }

    operator SkImageGenerator*() const {
        fSharedGenerator->fMutex.assertHeld();
        return fSharedGenerator->fGenerator.get();
    }

private:
    const sk_sp<SharedGenerator>& fSharedGenerator;
    SkAutoMutexExclusive          fAutoAcquire;
};

///////////////////////////////////////////////////////////////////////////////

SkImage_Lazy::SkImage_Lazy(Validator* validator)
    : SkImage_Base(validator->fInfo, validator->fUniqueID)
    , fSharedGenerator(std::move(validator->fSharedGenerator))
{
    SkASSERT(fSharedGenerator);
}

bool SkImage_Lazy::getROPixels(GrDirectContext* ctx, SkBitmap* bitmap,
                               SkImage::CachingHint chint) const {
    auto check_output_bitmap = [bitmap]() {
        SkASSERT(bitmap->isImmutable());
        SkASSERT(bitmap->getPixels());
        (void)bitmap;
    };

    auto desc = SkBitmapCacheDesc::Make(this);
    if (SkBitmapCache::Find(desc, bitmap)) {
        check_output_bitmap();
        return true;
    }

    if (SkImage::kAllow_CachingHint == chint) {
        SkPixmap pmap;
        SkBitmapCache::RecPtr cacheRec = SkBitmapCache::Alloc(desc, this->imageInfo(), &pmap);
        if (!cacheRec) {
            return false;
        }
        bool success = false;
        {   // make sure ScopedGenerator goes out of scope before we try readPixelsProxy
            success = ScopedGenerator(fSharedGenerator)->getPixels(pmap);
        }
        if (!success && !this->readPixelsProxy(ctx, pmap)) {
            return false;
        }
        SkBitmapCache::Add(std::move(cacheRec), bitmap);
        this->notifyAddedToRasterCache();
    } else {
        if (!bitmap->tryAllocPixels(this->imageInfo())) {
            return false;
        }
        bool success = false;
        {   // make sure ScopedGenerator goes out of scope before we try readPixelsProxy
            success = ScopedGenerator(fSharedGenerator)->getPixels(bitmap->pixmap());
        }
        if (!success && !this->readPixelsProxy(ctx, bitmap->pixmap())) {
            return false;
        }
        bitmap->setImmutable();
    }
    check_output_bitmap();
    return true;
}

sk_sp<SharedGenerator> SkImage_Lazy::generator() const {
    return fSharedGenerator;
}

bool SkImage_Lazy::onIsProtected() const {
    ScopedGenerator generator(fSharedGenerator);
    return generator->isProtected();
}

bool SkImage_Lazy::onReadPixels(GrDirectContext* dContext,
                                const SkImageInfo& dstInfo,
                                void* dstPixels,
                                size_t dstRB,
                                int srcX,
                                int srcY,
                                CachingHint chint) const {
    SkBitmap bm;
    if (this->getROPixels(dContext, &bm, chint)) {
        return bm.readPixels(dstInfo, dstPixels, dstRB, srcX, srcY);
    }
    return false;
}

sk_sp<SkData> SkImage_Lazy::onRefEncoded() const {
    // check that we aren't a subset or colortype/etc modification of the original
    if (fSharedGenerator->fGenerator->uniqueID() == this->uniqueID()) {
        ScopedGenerator generator(fSharedGenerator);
        return generator->refEncodedData();
    }
    return nullptr;
}

bool SkImage_Lazy::isValid(GrRecordingContext* context) const {
    ScopedGenerator generator(fSharedGenerator);
    return generator->isValid(context);
}


sk_sp<SkImage> SkImage_Lazy::onMakeSubset(GrDirectContext*, const SkIRect& subset) const {
    // neither picture-backed nor codec-backed lazy images need the context to do readbacks.
    // The subclass for cross-context images *does* use the direct context.
    auto pixels = this->makeRasterImage(nullptr);
    return pixels ? pixels->makeSubset(nullptr, subset) : nullptr;
}

sk_sp<SkImage> SkImage_Lazy::onMakeSubset(skgpu::graphite::Recorder*,
                                          const SkIRect& subset,
                                          RequiredProperties props) const {
    // TODO: can we do this more efficiently, by telling the generator we want to
    //       "realize" a subset?
    sk_sp<SkImage> nonLazyImg = this->makeRasterImage(nullptr);
    if (!nonLazyImg) {
        return nullptr;
    }
    return nonLazyImg->makeSubset(nullptr, subset, props);
}

sk_sp<SkSurface> SkImage_Lazy::onMakeSurface(skgpu::graphite::Recorder*,
                                             const SkImageInfo& info) const {
    const SkSurfaceProps* props = nullptr;
    const size_t rowBytes = 0;
    return SkSurfaces::Raster(info, rowBytes, props);
}

sk_sp<SkImage> SkImage_Lazy::onMakeColorTypeAndColorSpace(SkColorType targetCT,
                                                          sk_sp<SkColorSpace> targetCS,
                                                          GrDirectContext*) const {
    SkAutoMutexExclusive autoAquire(fOnMakeColorTypeAndSpaceMutex);
    if (fOnMakeColorTypeAndSpaceResult &&
        targetCT == fOnMakeColorTypeAndSpaceResult->colorType() &&
        SkColorSpace::Equals(targetCS.get(), fOnMakeColorTypeAndSpaceResult->colorSpace())) {
        return fOnMakeColorTypeAndSpaceResult;
    }
    Validator validator(fSharedGenerator, &targetCT, targetCS);
    sk_sp<SkImage> result = validator ? sk_sp<SkImage>(new SkImage_Lazy(&validator)) : nullptr;
    if (result) {
        fOnMakeColorTypeAndSpaceResult = result;
    }
    return result;
}

sk_sp<SkImage> SkImage_Lazy::onReinterpretColorSpace(sk_sp<SkColorSpace> newCS) const {
    // TODO: The correct thing is to clone the generator, and modify its color space. That's hard,
    // because we don't have a clone method, and generator is public (and derived-from by clients).
    // So do the simple/inefficient thing here, and fallback to raster when this is called.

    // We allocate the bitmap with the new color space, then generate the image using the original.
    SkBitmap bitmap;
    if (bitmap.tryAllocPixels(this->imageInfo().makeColorSpace(std::move(newCS)))) {
        SkPixmap pixmap = bitmap.pixmap();
        pixmap.setColorSpace(this->refColorSpace());
        if (ScopedGenerator(fSharedGenerator)->getPixels(pixmap)) {
            bitmap.setImmutable();
            return bitmap.asImage();
        }
    }
    return nullptr;
}

sk_sp<SkCachedData> SkImage_Lazy::getPlanes(
        const SkYUVAPixmapInfo::SupportedDataTypes& supportedDataTypes,
        SkYUVAPixmaps* yuvaPixmaps) const {
    ScopedGenerator generator(fSharedGenerator);

    sk_sp<SkCachedData> data(SkYUVPlanesCache::FindAndRef(generator->uniqueID(), yuvaPixmaps));

    if (data) {
        SkASSERT(yuvaPixmaps->isValid());
        SkASSERT(yuvaPixmaps->yuvaInfo().dimensions() == this->dimensions());
        return data;
    }
    SkYUVAPixmapInfo yuvaPixmapInfo;
    if (!generator->queryYUVAInfo(supportedDataTypes, &yuvaPixmapInfo) ||
        yuvaPixmapInfo.yuvaInfo().dimensions() != this->dimensions()) {
        return nullptr;
    }
    data.reset(SkResourceCache::NewCachedData(yuvaPixmapInfo.computeTotalBytes()));
    SkYUVAPixmaps tempPixmaps = SkYUVAPixmaps::FromExternalMemory(yuvaPixmapInfo,
                                                                  data->writable_data());
    SkASSERT(tempPixmaps.isValid());
    if (!generator->getYUVAPlanes(tempPixmaps)) {
        return nullptr;
    }
    // Decoding is done, cache the resulting YUV planes
    *yuvaPixmaps = tempPixmaps;
    SkYUVPlanesCache::Add(this->uniqueID(), data.get(), *yuvaPixmaps);
    return data;
}

void SkImage_Lazy::addUniqueIDListener(sk_sp<SkIDChangeListener> listener) const {
    fUniqueIDListeners.add(std::move(listener));
}

// TODO(kjlubick) move SharedGenerate to SkImage_Lazy.h and this to SkImage_LazyFactories
namespace SkImages {

sk_sp<SkImage> DeferredFromGenerator(std::unique_ptr<SkImageGenerator> generator) {
    SkImage_Lazy::Validator validator(
            SharedGenerator::Make(std::move(generator)), nullptr, nullptr);

    return validator ? sk_make_sp<SkImage_Lazy>(&validator) : nullptr;
}

}  // namespace SkImages
