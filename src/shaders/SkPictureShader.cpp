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
#include "src/base/SkArenaAlloc.h"
#include "src/core/SkImageInfoPriv.h"
#include "src/core/SkImagePriv.h"
#include "src/core/SkMatrixPriv.h"
#include "src/core/SkMatrixProvider.h"
#include "src/core/SkMatrixUtils.h"
#include "src/core/SkPicturePriv.h"
#include "src/core/SkRasterPipeline.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkResourceCache.h"
#include "src/core/SkVM.h"
#include "src/shaders/SkBitmapProcShader.h"
#include "src/shaders/SkImageShader.h"
#include "src/shaders/SkLocalMatrixShader.h"

#if defined(SK_GANESH)
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

#if defined(SK_GRAPHITE)
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/KeyContext.h"
#include "src/gpu/graphite/KeyHelpers.h"
#include "src/gpu/graphite/PaintParamsKey.h"
#include "src/gpu/graphite/RecorderPriv.h"
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
    SkSize         tileScale;      // Additional scale factors to apply when sampling image.
    SkMatrix       matrixForDraw;  // Matrix used to produce an image from the picture
    SkImageInfo    imageInfo;
    SkSurfaceProps props;

    static CachedImageInfo Make(const SkRect& bounds,
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

            // Scale down the tile size if larger than maxTextureSize for GPU path
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

        return {true,
                tileScale,
                SkMatrix::RectToRect(bounds, SkRect::MakeIWH(tileSize.width(), tileSize.height())),
                SkImageInfo::Make(tileSize, imgCT, kPremul_SkAlphaType, imgCS),
                props};
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
        image = info.makeImage(SkSurface::MakeRaster(info.imageInfo, &info.props), fPicture.get());
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

bool SkPictureShader::appendStages(const SkStageRec& rec, const MatrixRec& mRec) const {
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

skvm::Color SkPictureShader::program(skvm::Builder* p,
                                     skvm::Coord device,
                                     skvm::Coord local,
                                     skvm::Color paint,
                                     const MatrixRec& mRec,
                                     const SkColorInfo& dst,
                                     skvm::Uniforms* uniforms,
                                     SkArenaAlloc* alloc) const {
    // TODO: We'll need additional plumbing to get the correct props from our callers.
    SkSurfaceProps props{};

    // Keep bitmapShader alive by using alloc instead of stack memory
    auto& bitmapShader = *alloc->make<sk_sp<SkShader>>();
    bitmapShader = this->rasterShader(mRec.totalMatrix(), dst.colorType(), dst.colorSpace(), props);
    if (!bitmapShader) {
        return {};
    }

    return as_SB(bitmapShader)->program(p, device, local, paint, mRec, dst, uniforms, alloc);
}

/////////////////////////////////////////////////////////////////////////////////////////

#ifdef SK_ENABLE_LEGACY_SHADERCONTEXT
SkShaderBase::Context* SkPictureShader::onMakeContext(const ContextRec& rec, SkArenaAlloc* alloc)
const {
    const auto& vm     = *rec.fMatrix;
    const auto* lm     = rec.fLocalMatrix;
    const auto  totalM = lm ? SkMatrix::Concat(vm, *lm) : vm;
    sk_sp<SkShader> bitmapShader = this->rasterShader(totalM, rec.fDstColorType,
                                                      rec.fDstColorSpace, rec.fProps);
    if (!bitmapShader) {
        return nullptr;
    }

    return as_SB(bitmapShader)->makeContext(rec, alloc);
}
#endif

/////////////////////////////////////////////////////////////////////////////////////////

#if defined(SK_GANESH)

#include "src/gpu/ganesh/GrProxyProvider.h"

std::unique_ptr<GrFragmentProcessor> SkPictureShader::asFragmentProcessor(
        const GrFPArgs& args, const MatrixRec& mRec) const {
    auto ctx = args.fContext;
    SkColorType dstColorType = GrColorTypeToSkColorType(args.fDstColorInfo->colorType());
    if (dstColorType == kUnknown_SkColorType) {
        dstColorType = kRGBA_8888_SkColorType;
    }

    auto dstCS = ref_or_srgb(args.fDstColorInfo->colorSpace());

    auto info = CachedImageInfo::Make(fTile,
                                      mRec.totalMatrix(),
                                      dstColorType,
                                      dstCS.get(),
                                      ctx->priv().caps()->maxTextureSize(),
                                      args.fSurfaceProps);
    if (!info.success) {
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
                                                                skgpu::Budgeted::kYes,
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
    auto fp = GrTextureEffect::Make(std::move(view),
                                    kPremul_SkAlphaType,
                                    SkMatrix::I(),
                                    sampler,
                                    *ctx->priv().caps());
    SkMatrix scale = SkMatrix::Scale(info.tileScale.width(), info.tileScale.height());
    bool success;
    std::tie(success, fp) = mRec.apply(std::move(fp), scale);
    return success ? std::move(fp) : nullptr;
}
#endif

#if defined(SK_GRAPHITE)
void SkPictureShader::addToKey(const skgpu::graphite::KeyContext& keyContext,
                               skgpu::graphite::PaintParamsKeyBuilder* builder,
                               skgpu::graphite::PipelineDataGatherer* gatherer) const {

    using namespace skgpu::graphite;

    Recorder* recorder = keyContext.recorder();
    const Caps* caps = recorder->priv().caps();

    // TODO: We'll need additional plumbing to get the correct props from our callers. In
    // particular we'll need to expand the keyContext to have the surfaceProps, the dstColorType
    // and dstColorSpace.
    SkSurfaceProps props{};

    SkMatrix totalM = keyContext.local2Dev().asM33();
    if (keyContext.localMatrix()) {
        totalM.preConcat(*keyContext.localMatrix());
    }
    CachedImageInfo info = CachedImageInfo::Make(fTile,
                                                 totalM,
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
                                                                skgpu::Mipmapped::kNo, &info.props),
                                        fPicture.get());
    if (!img) {
        SolidColorShaderBlock::BeginBlock(keyContext, builder, gatherer, {1, 0, 0, 1});
        builder->endBlock();
        return;
    }

    const auto shaderLM = SkMatrix::Scale(1.f/info.tileScale.width(), 1.f/info.tileScale.height());
    sk_sp<SkShader> shader = img->makeShader(fTmx, fTmy, SkSamplingOptions(fFilter), &shaderLM);
    if (!shader) {
        SolidColorShaderBlock::BeginBlock(keyContext, builder, gatherer, {1, 0, 0, 1});
        builder->endBlock();
        return;
    }

    as_SB(shader)->addToKey(keyContext, builder, gatherer);
}
#endif // SK_GRAPHITE
