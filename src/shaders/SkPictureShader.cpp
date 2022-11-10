/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/shaders/SkPictureShader.h"

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkImage.h"
#include "include/private/SkImageInfoPriv.h"
#include "src/core/SkArenaAlloc.h"
#include "src/core/SkImagePriv.h"
#include "src/core/SkMatrixPriv.h"
#include "src/core/SkMatrixProvider.h"
#include "src/core/SkMatrixUtils.h"
#include "src/core/SkPicturePriv.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkResourceCache.h"
#include "src/core/SkVM.h"
#include "src/shaders/SkImageShader.h"
#include "src/shaders/SkLocalMatrixShader.h"
#include <atomic>

#if SK_SUPPORT_GPU
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/GrRecordingContext.h"
#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrColorInfo.h"
#include "src/gpu/ganesh/GrFPArgs.h"
#include "src/gpu/ganesh/GrFragmentProcessor.h"
#include "src/gpu/ganesh/GrRecordingContextPriv.h"
#include "src/gpu/ganesh/SkGr.h"
#include "src/gpu/ganesh/effects/GrTextureEffect.h"
#include "src/image/SkImage_Base.h"
#include "src/shaders/SkLocalMatrixShader.h"
#endif

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

struct CachedImageInfo {
    bool           success;
    SkSize         tileScale;
    SkMatrix       matrixForDraw;
    SkImageInfo    imageInfo;
    SkSurfaceProps props;

    static CachedImageInfo Make(const SkRect& bounds,
                                const SkMatrix& viewMatrix,
                                SkMatrix* localMatrix,     // in/out
                                SkColorType dstColorType,
                                SkColorSpace* dstColorSpace,
                                const int maxTextureSize,
                                const SkSurfaceProps& propsIn) {
        const SkMatrix m = SkMatrix::Concat(viewMatrix, *localMatrix);

        SkSurfaceProps props = propsIn.cloneWithPixelGeometry(kUnknown_SkPixelGeometry);

        const SkSize scaledSize = [&]() {
            SkSize size;
            // Use a rotation-invariant scale
            if (!m.decomposeScale(&size, nullptr)) {
                SkPoint center = {bounds.centerX(), bounds.centerY()};
                SkScalar area = SkMatrixPriv::DifferentialAreaScale(m, center);
                if (!SkScalarIsFinite(area) || SkScalarNearlyZero(area)) {
                    size = {1, 1}; // ill-conditioned matrix
                } else {
                    size.fWidth = size.fHeight = SkScalarSqrt(area);
                }
            }
            size.fWidth  *= bounds.width();
            size.fHeight *= bounds.height();

            // Clamp the tile size to about 4M pixels
            static const SkScalar kMaxTileArea = 2048 * 2048;
            SkScalar tileArea = size.width() * size.height();
            if (tileArea > kMaxTileArea) {
                SkScalar clampScale = SkScalarSqrt(kMaxTileArea / tileArea);
                size.set(size.width() * clampScale, size.height() * clampScale);
            }

            // Scale down the tile size if larger than maxTextureSize for GPU Path
            // or it should fail on create texture
            if (maxTextureSize) {
                if (size.width() > maxTextureSize || size.height() > maxTextureSize) {
                    SkScalar downScale = maxTextureSize / std::max(size.width(),
                                                                   size.height());
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

        const SkSize tileScale = {
            tileSize.width() / bounds.width(), tileSize.height() / bounds.height()
        };
        auto imgCS = ref_or_srgb(dstColorSpace);
        const SkColorType imgCT = SkColorTypeMaxBitsPerChannel(dstColorType) <= 8
                                ? kRGBA_8888_SkColorType
                                : kRGBA_F16Norm_SkColorType;

        if (tileScale.width() != 1 || tileScale.height() != 1) {
            localMatrix->preScale(1 / tileScale.width(), 1 / tileScale.height());
        }

        return {
            true,
            tileScale,
            SkMatrix::RectToRect(bounds, SkRect::MakeIWH(tileSize.width(), tileSize.height())),
            SkImageInfo::Make(tileSize.width(), tileSize.height(),
                              imgCT, kPremul_SkAlphaType, imgCS),
            props
        };
    }

    sk_sp<SkImage> makeImage(sk_sp<SkSurface> surf, const SkPicture* pict) const {
        if (!surf) {
            return nullptr;
        }
        auto canvas = surf->getCanvas();
        canvas->concat(matrixForDraw);
        canvas->drawPicture(pict);
        return surf->makeImageSnapshot();
    }
};

// Returns a cached image shader, which wraps a single picture tile at the given
// CTM/local matrix.  Also adjusts the local matrix for tile scaling.
sk_sp<SkShader> SkPictureShader::rasterShader(const SkMatrix& viewMatrix,
                                              SkMatrix* localMatrix,
                                              SkColorType dstColorType,
                                              SkColorSpace* dstColorSpace,
                                              const SkSurfaceProps& propsIn) const {
    const int maxTextureSize_NotUsedForCPU = 0;
    CachedImageInfo info = CachedImageInfo::Make(fTile, viewMatrix, localMatrix,
                                                 dstColorType, dstColorSpace,
                                                 maxTextureSize_NotUsedForCPU, propsIn);
    if (!info.success) {
        return nullptr;
    }

    ImageFromPictureKey key(info.imageInfo.colorSpace(), info.imageInfo.colorType(),
                            fPicture->uniqueID(), fTile, info.tileScale, info.props);

    sk_sp<SkImage> image;
    if (!SkResourceCache::Find(key, ImageFromPictureRec::Visitor, &image)) {
        image = info.makeImage(SkSurface::MakeRaster(info.imageInfo, &info.props), fPicture.get());
        if (!image) {
            return nullptr;
        }

        SkResourceCache::Add(new ImageFromPictureRec(key, image));
        SkPicturePriv::AddedToCache(fPicture.get());
    }
    return image->makeShader(fTmx, fTmy, SkSamplingOptions(fFilter), nullptr);
}

bool SkPictureShader::onAppendStages(const SkStageRec& rec) const {
    auto lm = rec.fLocalM ? *rec.fLocalM : SkMatrix::I();
    // Keep bitmapShader alive by using alloc instead of stack memory
    auto& bitmapShader = *rec.fAlloc->make<sk_sp<SkShader>>();
    bitmapShader = this->rasterShader(rec.fMatrixProvider.localToDevice(), &lm,
                                      rec.fDstColorType, rec.fDstCS, rec.fSurfaceProps);
    if (!bitmapShader) {
        return false;
    }

    SkStageRec localRec = rec;
    localRec.fLocalM = lm.isIdentity() ? nullptr : &lm;

    return as_SB(bitmapShader)->appendStages(localRec);
}

skvm::Color SkPictureShader::onProgram(skvm::Builder* p,
                                       skvm::Coord device, skvm::Coord local, skvm::Color paint,
                                       const SkMatrixProvider& matrices, const SkMatrix* localM,
                                       const SkColorInfo& dst,
                                       skvm::Uniforms* uniforms, SkArenaAlloc* alloc) const {
    SkMatrix lm = localM ? *localM : SkMatrix::I();

    // TODO: We'll need additional plumbing to get the correct props from our callers.
    SkSurfaceProps props{};

    // Keep bitmapShader alive by using alloc instead of stack memory
    auto& bitmapShader = *alloc->make<sk_sp<SkShader>>();
    bitmapShader = this->rasterShader(matrices.localToDevice(), &lm,
                                      dst.colorType(), dst.colorSpace(), props);
    if (!bitmapShader) {
        return {};
    }

    return as_SB(bitmapShader)->program(p, device, local, paint,
                                        matrices, &lm, dst,
                                        uniforms, alloc);
}

/////////////////////////////////////////////////////////////////////////////////////////

#if SK_SUPPORT_GPU

#include "src/gpu/ganesh/GrProxyProvider.h"

std::unique_ptr<GrFragmentProcessor> SkPictureShader::asFragmentProcessor(
        const GrFPArgs& args) const {

    auto ctx = args.fContext;
    auto lm = args.fLocalMatrix ? *args.fLocalMatrix : SkMatrix::I();
    SkColorType dstColorType = GrColorTypeToSkColorType(args.fDstColorInfo->colorType());
    if (dstColorType == kUnknown_SkColorType) {
        dstColorType = kRGBA_8888_SkColorType;
    }

    auto dstCS = ref_or_srgb(args.fDstColorInfo->colorSpace());
    auto viewMatrix = args.fMatrixProvider.localToDevice();
    auto info = CachedImageInfo::Make(fTile, viewMatrix, &lm, dstColorType, dstCS.get(),
                                      ctx->priv().caps()->maxTextureSize(), args.fSurfaceProps);
    SkMatrix inv;
    if (!info.success || !lm.invert(&inv)) {
        return nullptr;
    }

    // Gotta be sure the GPU can support our requested colortype (might be FP16)
    if (!ctx->colorTypeSupportedAsSurface(info.imageInfo.colorType())) {
        info.imageInfo = info.imageInfo.makeColorType(kRGBA_8888_SkColorType);
    }

    static const skgpu::UniqueKey::Domain kDomain = skgpu::UniqueKey::GenerateDomain();
    skgpu::UniqueKey key;
    std::tuple keyData = {
        dstCS->toXYZD50Hash(),
        dstCS->transferFnHash(),
        static_cast<uint32_t>(dstColorType),
        fPicture->uniqueID(),
        fTile,
        info.tileScale,
        info.props
    };
    skgpu::UniqueKey::Builder builder(&key, kDomain, sizeof(keyData)/sizeof(uint32_t),
                                      "Picture Shader Image");
    memcpy(&builder[0], &keyData, sizeof(keyData));
    builder.finish();

    GrProxyProvider* provider = ctx->priv().proxyProvider();
    GrSurfaceProxyView view;
    if (auto proxy = provider->findOrCreateProxyByUniqueKey(key)) {
        view = GrSurfaceProxyView(proxy, kTopLeft_GrSurfaceOrigin, skgpu::Swizzle());
    } else {
        const int msaaSampleCount = 0;
        const bool createWithMips = false;
        auto image = info.makeImage(SkSurface::MakeRenderTarget(ctx,
                                                                SkBudgeted::kYes,
                                                                info.imageInfo,
                                                                msaaSampleCount,
                                                                kTopLeft_GrSurfaceOrigin,
                                                                &info.props,
                                                                createWithMips),
                                    fPicture.get());
        if (!image) {
            return nullptr;
        }
        auto [v, ct] = as_IB(image)->asView(ctx, GrMipmapped::kNo);
        view = std::move(v);
        provider->assignUniqueKeyToProxy(key, view.asTextureProxy());
    }

    const GrSamplerState sampler(static_cast<GrSamplerState::WrapMode>(fTmx),
                                 static_cast<GrSamplerState::WrapMode>(fTmy),
                                 fFilter);

    return GrTextureEffect::Make(
            std::move(view), kPremul_SkAlphaType, inv, sampler, *ctx->priv().caps());
}
#endif

#ifdef SK_GRAPHITE_ENABLED
#include "src/core/SkKeyContext.h"
#include "src/core/SkKeyHelpers.h"

#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/PaintParamsKey.h"
#include "src/gpu/graphite/RecorderPriv.h"

void SkPictureShader::addToKey(const SkKeyContext& keyContext,
                               skgpu::graphite::PaintParamsKeyBuilder* builder,
                               SkPipelineDataGatherer* gatherer) const {

    using namespace skgpu::graphite;

    Recorder* recorder = keyContext.recorder();
    const Caps* caps = recorder->priv().caps();

    // TODO: We'll need additional plumbing to get the correct props from our callers. In
    // particular we'll need to expand the keyContext to have the surfaceProps, the dstColorType
    // and dstColorSpace.
    SkSurfaceProps props{};

    SkMatrix lm = keyContext.localMatrix() ? *keyContext.localMatrix() : SkMatrix::I();
    CachedImageInfo info = CachedImageInfo::Make(fTile,
                                                 keyContext.local2Dev().asM33(),
                                                 &lm,
                                                 /* dstColorType= */ kRGBA_8888_SkColorType,
                                                 /* dstColorSpace= */ nullptr,
                                                 caps->maxTextureSize(),
                                                 props);
    if (!info.success) {
        SolidColorShaderBlock::BeginBlock(keyContext, builder, gatherer, {1, 0, 0, 1});
        builder->endBlock();
        return;
    }

    // TODO: right now we're explicitly not caching here. We could expand the ImageProvider
    // API to include already Graphite-backed images, add a Recorder-local cache or add
    // rendered-picture images to the global cache.
    sk_sp<SkImage> img = info.makeImage(SkSurface::MakeGraphite(recorder, info.imageInfo,
                                                                Mipmapped::kNo, &info.props),
                                        fPicture.get());
    if (!img) {
        SolidColorShaderBlock::BeginBlock(keyContext, builder, gatherer, {1, 0, 0, 1});
        builder->endBlock();
        return;
    }

    SkMatrix newLocal;
    if (info.tileScale.width() != 1 || info.tileScale.height() != 1) {
        newLocal.preScale(1 / info.tileScale.width(), 1 / info.tileScale.height());
    }

    sk_sp<SkShader> shader = img->makeShader(fTmx, fTmy, SkSamplingOptions(fFilter), newLocal);
    if (!shader) {
        SolidColorShaderBlock::BeginBlock(keyContext, builder, gatherer, {1, 0, 0, 1});
        builder->endBlock();
        return;
    }

    as_SB(shader)->addToKey(keyContext, builder, gatherer);
}

#endif // SK_GRAPHITE_ENABLED
