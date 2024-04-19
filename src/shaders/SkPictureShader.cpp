/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/shaders/SkPictureShader.h"

#include "include/core/SkAlphaType.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkColorType.h"
#include "include/core/SkImage.h"
#include "include/core/SkPoint.h"
#include "include/core/SkSamplingOptions.h"
#include "include/core/SkScalar.h"
#include "include/core/SkShader.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTileMode.h"
#include "include/private/base/SkDebug.h"
#include "include/private/base/SkFloatingPoint.h"
#include "src/base/SkArenaAlloc.h"
#include "src/core/SkEffectPriv.h"
#include "src/core/SkImageInfoPriv.h"
#include "src/core/SkMatrixPriv.h"
#include "src/core/SkPicturePriv.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkResourceCache.h"
#include "src/core/SkWriteBuffer.h"
#include "src/shaders/SkLocalMatrixShader.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <utility>
class SkDiscardableMemory;

sk_sp<SkShader> SkPicture::makeShader(SkTileMode tmx, SkTileMode tmy, SkFilterMode filter,
                                      const SkMatrix* localMatrix, const SkRect* tile) const {
    if (localMatrix && !localMatrix->invert(nullptr)) {
        return nullptr;
    }
    return SkPictureShader::Make(sk_ref_sp(this), tmx, tmy, filter, localMatrix, tile);
}

namespace {
static unsigned gImageFromPictureKeyNamespaceLabel;

struct ImageFromPictureKey : public SkResourceCache::Key {
public:
    ImageFromPictureKey(SkColorSpace* colorSpace, SkColorType colorType,
                        uint32_t pictureID, const SkRect& subset,
                        SkSize scale, const SkSurfaceProps& surfaceProps)
        : fColorSpaceXYZHash(colorSpace->toXYZD50Hash())
        , fColorSpaceTransferFnHash(colorSpace->transferFnHash())
        , fColorType(static_cast<uint32_t>(colorType))
        , fSubset(subset)
        , fScale(scale)
        , fSurfaceProps(surfaceProps)
    {
        static const size_t keySize = sizeof(fColorSpaceXYZHash) +
                                      sizeof(fColorSpaceTransferFnHash) +
                                      sizeof(fColorType) +
                                      sizeof(fSubset) +
                                      sizeof(fScale) +
                                      sizeof(fSurfaceProps);
        // This better be packed.
        SkASSERT(sizeof(uint32_t) * (&fEndOfStruct - &fColorSpaceXYZHash) == keySize);
        this->init(&gImageFromPictureKeyNamespaceLabel,
                   SkPicturePriv::MakeSharedID(pictureID),
                   keySize);
    }

private:
    uint32_t       fColorSpaceXYZHash;
    uint32_t       fColorSpaceTransferFnHash;
    uint32_t       fColorType;
    SkRect         fSubset;
    SkSize         fScale;
    SkSurfaceProps fSurfaceProps;

    SkDEBUGCODE(uint32_t fEndOfStruct;)
};

struct ImageFromPictureRec : public SkResourceCache::Rec {
    ImageFromPictureRec(const ImageFromPictureKey& key, sk_sp<SkImage> image)
        : fKey(key)
        , fImage(std::move(image)) {}

    ImageFromPictureKey fKey;
    sk_sp<SkImage>  fImage;

    const Key& getKey() const override { return fKey; }
    size_t bytesUsed() const override {
        // Just the record overhead -- the actual pixels are accounted by SkImage_Lazy.
        return sizeof(fKey) + (size_t)fImage->width() * fImage->height() * 4;
    }
    const char* getCategory() const override { return "bitmap-shader"; }
    SkDiscardableMemory* diagnostic_only_getDiscardable() const override { return nullptr; }

    static bool Visitor(const SkResourceCache::Rec& baseRec, void* contextShader) {
        const ImageFromPictureRec& rec = static_cast<const ImageFromPictureRec&>(baseRec);
        sk_sp<SkImage>* result = reinterpret_cast<sk_sp<SkImage>*>(contextShader);

        *result = rec.fImage;
        return true;
    }
};

} // namespace

SkPictureShader::SkPictureShader(sk_sp<SkPicture> picture,
                                 SkTileMode tmx,
                                 SkTileMode tmy,
                                 SkFilterMode filter,
                                 const SkRect* tile)
        : fPicture(std::move(picture))
        , fTile(tile ? *tile : fPicture->cullRect())
        , fTmx(tmx)
        , fTmy(tmy)
        , fFilter(filter) {}

sk_sp<SkShader> SkPictureShader::Make(sk_sp<SkPicture> picture, SkTileMode tmx, SkTileMode tmy,
                                      SkFilterMode filter, const SkMatrix* lm, const SkRect* tile) {
    if (!picture || picture->cullRect().isEmpty() || (tile && tile->isEmpty())) {
        return SkShaders::Empty();
    }
    return SkLocalMatrixShader::MakeWrapped<SkPictureShader>(lm,
                                                             std::move(picture),
                                                             tmx, tmy,
                                                             filter,
                                                             tile);
}

sk_sp<SkFlattenable> SkPictureShader::CreateProc(SkReadBuffer& buffer) {
    SkMatrix lm;
    if (buffer.isVersionLT(SkPicturePriv::Version::kNoShaderLocalMatrix)) {
        buffer.readMatrix(&lm);
    }
    auto tmx = buffer.read32LE(SkTileMode::kLastTileMode);
    auto tmy = buffer.read32LE(SkTileMode::kLastTileMode);
    SkRect tile = buffer.readRect();

    sk_sp<SkPicture> picture;

    SkFilterMode filter = SkFilterMode::kNearest;
    if (buffer.isVersionLT(SkPicturePriv::kNoFilterQualityShaders_Version)) {
        if (buffer.isVersionLT(SkPicturePriv::kPictureShaderFilterParam_Version)) {
            bool didSerialize = buffer.readBool();
            if (didSerialize) {
                picture = SkPicturePriv::MakeFromBuffer(buffer);
            }
        } else {
            unsigned legacyFilter = buffer.read32();
            if (legacyFilter <= (unsigned)SkFilterMode::kLast) {
                filter = (SkFilterMode)legacyFilter;
            }
            picture = SkPicturePriv::MakeFromBuffer(buffer);
        }
    } else {
        filter = buffer.read32LE(SkFilterMode::kLast);
        picture = SkPicturePriv::MakeFromBuffer(buffer);
    }
    return SkPictureShader::Make(picture, tmx, tmy, filter, &lm, &tile);
}

void SkPictureShader::flatten(SkWriteBuffer& buffer) const {
    buffer.write32((unsigned)fTmx);
    buffer.write32((unsigned)fTmy);
    buffer.writeRect(fTile);
    buffer.write32((unsigned)fFilter);
    SkPicturePriv::Flatten(fPicture, buffer);
}

static sk_sp<SkColorSpace> ref_or_srgb(SkColorSpace* cs) {
    return cs ? sk_ref_sp(cs) : SkColorSpace::MakeSRGB();
}

SkPictureShader::CachedImageInfo SkPictureShader::CachedImageInfo::Make(
        const SkRect& bounds,
        const SkMatrix& totalM,
        SkColorType dstColorType,
        SkColorSpace* dstColorSpace,
        const int maxTextureSize,
        const SkSurfaceProps& propsIn) {
    SkSurfaceProps props = propsIn.cloneWithPixelGeometry(kUnknown_SkPixelGeometry);

    const SkSize scaledSize = [&]() {
        SkSize size;
        // Use a rotation-invariant scale
        if (!totalM.decomposeScale(&size, nullptr)) {
            SkPoint center = {bounds.centerX(), bounds.centerY()};
            SkScalar area = SkMatrixPriv::DifferentialAreaScale(totalM, center);
            if (!SkIsFinite(area) || SkScalarNearlyZero(area)) {
                size = {1, 1};  // ill-conditioned matrix
            } else {
                size.fWidth = size.fHeight = SkScalarSqrt(area);
            }
        }
        size.fWidth *= bounds.width();
        size.fHeight *= bounds.height();

        // Clamp the tile size to about 4M pixels
        static const SkScalar kMaxTileArea = 2048 * 2048;
        SkScalar tileArea = size.width() * size.height();
        if (tileArea > kMaxTileArea) {
            SkScalar clampScale = SkScalarSqrt(kMaxTileArea / tileArea);
            size.set(size.width() * clampScale, size.height() * clampScale);
        }

        // Scale down the tile size if larger than maxTextureSize for GPU path
        // or it should fail on create texture
        if (maxTextureSize) {
            if (size.width() > maxTextureSize || size.height() > maxTextureSize) {
                SkScalar downScale = maxTextureSize / std::max(size.width(), size.height());
                size.set(SkScalarFloorToScalar(size.width() * downScale),
                         SkScalarFloorToScalar(size.height() * downScale));
            }
        }
        return size;
    }();

    const SkISize tileSize = scaledSize.toCeil();
    if (tileSize.isEmpty()) {
        return {false, {}, {}, {}, {}};
    }

    const SkSize tileScale = {tileSize.width() / bounds.width(),
                              tileSize.height() / bounds.height()};
    auto imgCS = ref_or_srgb(dstColorSpace);
    const SkColorType imgCT = SkColorTypeMaxBitsPerChannel(dstColorType) <= 8
                                      ? kRGBA_8888_SkColorType
                                      : kRGBA_F16Norm_SkColorType;

    return {true,
            tileScale,
            SkMatrix::RectToRect(bounds, SkRect::MakeIWH(tileSize.width(), tileSize.height())),
            SkImageInfo::Make(tileSize, imgCT, kPremul_SkAlphaType, imgCS),
            props};
}

sk_sp<SkImage> SkPictureShader::CachedImageInfo::makeImage(sk_sp<SkSurface> surf,
                                                           const SkPicture* pict) const {
    if (!surf) {
        return nullptr;
    }
    auto canvas = surf->getCanvas();
    canvas->concat(matrixForDraw);
    canvas->drawPicture(pict);
    return surf->makeImageSnapshot();
}

// Returns a cached image shader, which wraps a single picture tile at the given
// CTM/local matrix.  Also adjusts the local matrix for tile scaling.
sk_sp<SkShader> SkPictureShader::rasterShader(const SkMatrix& totalM,
                                              SkColorType dstColorType,
                                              SkColorSpace* dstColorSpace,
                                              const SkSurfaceProps& propsIn) const {
    const int maxTextureSize_NotUsedForCPU = 0;
    CachedImageInfo info = CachedImageInfo::Make(fTile,
                                                 totalM,
                                                 dstColorType, dstColorSpace,
                                                 maxTextureSize_NotUsedForCPU,
                                                 propsIn);
    if (!info.success) {
        return nullptr;
    }

    ImageFromPictureKey key(info.imageInfo.colorSpace(), info.imageInfo.colorType(),
                            fPicture->uniqueID(), fTile, info.tileScale, info.props);

    sk_sp<SkImage> image;
    if (!SkResourceCache::Find(key, ImageFromPictureRec::Visitor, &image)) {
        image = info.makeImage(SkSurfaces::Raster(info.imageInfo, &info.props), fPicture.get());
        if (!image) {
            return nullptr;
        }

        SkResourceCache::Add(new ImageFromPictureRec(key, image));
        SkPicturePriv::AddedToCache(fPicture.get());
    }
    // Scale the image to the original picture size.
    auto lm = SkMatrix::Scale(1.f/info.tileScale.width(), 1.f/info.tileScale.height());
    return image->makeShader(fTmx, fTmy, SkSamplingOptions(fFilter), &lm);
}

bool SkPictureShader::appendStages(const SkStageRec& rec, const SkShaders::MatrixRec& mRec) const {
    // Keep bitmapShader alive by using alloc instead of stack memory
    auto& bitmapShader = *rec.fAlloc->make<sk_sp<SkShader>>();
    // We don't check whether the total local matrix is valid here because we have to assume *some*
    // mapping to make an image. It could be wildly wrong if there is a runtime shader transforming
    // the coordinates in a manner we don't know about here. However, that is a fundamental problem
    // with the technique of converting a picture to an image to implement this shader.
    bitmapShader = this->rasterShader(mRec.totalMatrix(),
                                      rec.fDstColorType,
                                      rec.fDstCS,
                                      rec.fSurfaceProps);
    if (!bitmapShader) {
        return false;
    }
    return as_SB(bitmapShader)->appendStages(rec, mRec);
}

/////////////////////////////////////////////////////////////////////////////////////////

#ifdef SK_ENABLE_LEGACY_SHADERCONTEXT
SkShaderBase::Context* SkPictureShader::onMakeContext(const ContextRec& rec,
                                                      SkArenaAlloc* alloc) const {
    sk_sp<SkShader> bitmapShader = this->rasterShader(
            rec.fMatrixRec.totalMatrix(), rec.fDstColorType, rec.fDstColorSpace, rec.fProps);
    if (!bitmapShader) {
        return nullptr;
    }

    return as_SB(bitmapShader)->makeContext(rec, alloc);
}
#endif
