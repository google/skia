/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/shaders/SkImageShader.h"

#include "src/base/SkArenaAlloc.h"
#include "src/core/SkColorSpacePriv.h"
#include "src/core/SkColorSpaceXformSteps.h"
#include "src/core/SkImageInfoPriv.h"
#include "src/core/SkMatrixPriv.h"
#include "src/core/SkMatrixProvider.h"
#include "src/core/SkMipmapAccessor.h"
#include "src/core/SkOpts.h"
#include "src/core/SkRasterPipeline.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkVM.h"
#include "src/core/SkWriteBuffer.h"
#include "src/image/SkImage_Base.h"
#include "src/shaders/SkBitmapProcShader.h"
#include "src/shaders/SkLocalMatrixShader.h"
#include "src/shaders/SkTransformShader.h"

#if defined(SK_GRAPHITE)
#include "src/gpu/graphite/ImageUtils.h"
#include "src/gpu/graphite/Image_Graphite.h"
#include "src/gpu/graphite/KeyContext.h"
#include "src/gpu/graphite/KeyHelpers.h"
#include "src/gpu/graphite/Log.h"
#include "src/gpu/graphite/PaintParamsKey.h"
#include "src/gpu/graphite/ReadWriteSwizzle.h"
#include "src/gpu/graphite/TextureProxyView.h"


static skgpu::graphite::ReadSwizzle swizzle_class_to_read_enum(const skgpu::Swizzle& swizzle) {
    if (swizzle == skgpu::Swizzle::RGBA()) {
        return skgpu::graphite::ReadSwizzle::kRGBA;
    } else if (swizzle == skgpu::Swizzle::RGB1()) {
        return skgpu::graphite::ReadSwizzle::kRGB1;
    } else if (swizzle == skgpu::Swizzle("rrrr")) {
        return skgpu::graphite::ReadSwizzle::kRRRR;
    } else if (swizzle == skgpu::Swizzle("rrr1")) {
        return skgpu::graphite::ReadSwizzle::kRRR1;
    } else if (swizzle == skgpu::Swizzle::BGRA()) {
        return skgpu::graphite::ReadSwizzle::kBGRA;
    } else {
        SKGPU_LOG_W("%s is an unsupported read swizzle. Defaulting to RGBA.\n",
                    swizzle.asString().data());
        return skgpu::graphite::ReadSwizzle::kRGBA;
    }
}
#endif

SkM44 SkImageShader::CubicResamplerMatrix(float B, float C) {
#if 0
    constexpr SkM44 kMitchell = SkM44( 1.f/18.f, -9.f/18.f,  15.f/18.f,  -7.f/18.f,
                                      16.f/18.f,  0.f/18.f, -36.f/18.f,  21.f/18.f,
                                       1.f/18.f,  9.f/18.f,  27.f/18.f, -21.f/18.f,
                                       0.f/18.f,  0.f/18.f,  -6.f/18.f,   7.f/18.f);

    constexpr SkM44 kCatmull = SkM44(0.0f, -0.5f,  1.0f, -0.5f,
                                     1.0f,  0.0f, -2.5f,  1.5f,
                                     0.0f,  0.5f,  2.0f, -1.5f,
                                     0.0f,  0.0f, -0.5f,  0.5f);

    if (B == 1.0f/3 && C == 1.0f/3) {
        return kMitchell;
    }
    if (B == 0 && C == 0.5f) {
        return kCatmull;
    }
#endif
    return SkM44(    (1.f/6)*B, -(3.f/6)*B - C,       (3.f/6)*B + 2*C,    - (1.f/6)*B - C,
                 1 - (2.f/6)*B,              0, -3 + (12.f/6)*B +   C,  2 - (9.f/6)*B - C,
                     (1.f/6)*B,  (3.f/6)*B + C,  3 - (15.f/6)*B - 2*C, -2 + (9.f/6)*B + C,
                             0,              0,                    -C,      (1.f/6)*B + C);
}

/**
 *  We are faster in clamp, so always use that tiling when we can.
 */
static SkTileMode optimize(SkTileMode tm, int dimension) {
    SkASSERT(dimension > 0);
#ifdef SK_BUILD_FOR_ANDROID_FRAMEWORK
    // need to update frameworks/base/libs/hwui/tests/unit/SkiaBehaviorTests.cpp:55 to allow
    // for transforming to clamp.
    return tm;
#else
    // mirror and repeat on a 1px axis are the same as clamping, but decal will still transition to
    // transparent black.
    return (tm != SkTileMode::kDecal && dimension == 1) ? SkTileMode::kClamp : tm;
#endif
}

// TODO: currently this only *always* used in asFragmentProcessor(), which is excluded on no-gpu
// builds. No-gpu builds only use needs_subset() in asserts, so release+no-gpu doesn't use it, which
// can cause builds to fail if unused warnings are treated as errors.
[[maybe_unused]] static bool needs_subset(SkImage* img, const SkRect& subset) {
    return subset != SkRect::Make(img->dimensions());
}

SkImageShader::SkImageShader(sk_sp<SkImage> img,
                             const SkRect& subset,
                             SkTileMode tmx, SkTileMode tmy,
                             const SkSamplingOptions& sampling,
                             bool raw,
                             bool clampAsIfUnpremul)
        : fImage(std::move(img))
        , fSampling(sampling)
        , fTileModeX(optimize(tmx, fImage->width()))
        , fTileModeY(optimize(tmy, fImage->height()))
        , fSubset(subset)
        , fRaw(raw)
        , fClampAsIfUnpremul(clampAsIfUnpremul) {
    // These options should never appear together:
    SkASSERT(!fRaw || !fClampAsIfUnpremul);

    // Bicubic filtering of raw image shaders would add a surprising clamp - so we don't support it
    SkASSERT(!fRaw || !fSampling.useCubic);
}

// just used for legacy-unflattening
enum class LegacyFilterEnum {
    kNone,
    kLow,
    kMedium,
    kHigh,
    // this is the special value for backward compatibility
    kInheritFromPaint,
    // this signals we should use the new SkFilterOptions
    kUseFilterOptions,
    // use cubic and ignore FilterOptions
    kUseCubicResampler,

    kLast = kUseCubicResampler,
};

// fClampAsIfUnpremul is always false when constructed through public APIs,
// so there's no need to read or write it here.

sk_sp<SkFlattenable> SkImageShader::CreateProc(SkReadBuffer& buffer) {
    auto tmx = buffer.read32LE<SkTileMode>(SkTileMode::kLastTileMode);
    auto tmy = buffer.read32LE<SkTileMode>(SkTileMode::kLastTileMode);

    SkSamplingOptions sampling;
    bool readSampling = true;
    if (buffer.isVersionLT(SkPicturePriv::kNoFilterQualityShaders_Version) &&
        !buffer.readBool() /* legacy has_sampling */)
    {
        readSampling = false;
        // we just default to Nearest in sampling
    }
    if (readSampling) {
        sampling = buffer.readSampling();
    }

    SkMatrix localMatrix;
    if (buffer.isVersionLT(SkPicturePriv::Version::kNoShaderLocalMatrix)) {
        buffer.readMatrix(&localMatrix);
    }
    sk_sp<SkImage> img = buffer.readImage();
    if (!img) {
        return nullptr;
    }

    bool raw = buffer.isVersionLT(SkPicturePriv::Version::kRawImageShaders) ? false
                                                                            : buffer.readBool();

    // TODO(skbug.com/12784): Subset is not serialized yet; it's only used by special images so it
    // will never be written to an SKP.

    return raw ? SkImageShader::MakeRaw(std::move(img), tmx, tmy, sampling, &localMatrix)
               : SkImageShader::Make(std::move(img), tmx, tmy, sampling, &localMatrix);
}

void SkImageShader::flatten(SkWriteBuffer& buffer) const {
    buffer.writeUInt((unsigned)fTileModeX);
    buffer.writeUInt((unsigned)fTileModeY);

    buffer.writeSampling(fSampling);

    buffer.writeImage(fImage.get());
    SkASSERT(fClampAsIfUnpremul == false);

    // TODO(skbug.com/12784): Subset is not serialized yet; it's only used by special images so it
    // will never be written to an SKP.
    SkASSERT(!needs_subset(fImage.get(), fSubset));

    buffer.writeBool(fRaw);
}

bool SkImageShader::isOpaque() const {
    return fImage->isOpaque() &&
           fTileModeX != SkTileMode::kDecal && fTileModeY != SkTileMode::kDecal;
}

#ifdef SK_ENABLE_LEGACY_SHADERCONTEXT

static bool legacy_shader_can_handle(const SkMatrix& inv) {
    SkASSERT(!inv.hasPerspective());

    // Scale+translate methods are always present, but affine might not be.
    if (!SkOpts::S32_alpha_D32_filter_DXDY && !inv.isScaleTranslate()) {
        return false;
    }

    // legacy code uses SkFixed 32.32, so ensure the inverse doesn't map device coordinates
    // out of range.
    const SkScalar max_dev_coord = 32767.0f;
    const SkRect src = inv.mapRect(SkRect::MakeWH(max_dev_coord, max_dev_coord));

    // take 1/4 of max signed 32bits so we have room to subtract local values
    const SkScalar max_fixed32dot32 = float(SK_MaxS32) * 0.25f;
    if (!SkRect::MakeLTRB(-max_fixed32dot32, -max_fixed32dot32,
                          +max_fixed32dot32, +max_fixed32dot32).contains(src)) {
        return false;
    }

    // legacy shader impl should be able to handle these matrices
    return true;
}

SkShaderBase::Context* SkImageShader::onMakeContext(const ContextRec& rec,
                                                    SkArenaAlloc* alloc) const {
    SkASSERT(!needs_subset(fImage.get(), fSubset)); // TODO(skbug.com/12784)
    if (fImage->alphaType() == kUnpremul_SkAlphaType) {
        return nullptr;
    }
    if (fImage->colorType() != kN32_SkColorType) {
        return nullptr;
    }
    if (fTileModeX != fTileModeY) {
        return nullptr;
    }
    if (fTileModeX == SkTileMode::kDecal || fTileModeY == SkTileMode::kDecal) {
        return nullptr;
    }

    SkSamplingOptions sampling = fSampling;
    if (sampling.isAniso()) {
        sampling = SkSamplingPriv::AnisoFallback(fImage->hasMipmaps());
    }

    auto supported = [](const SkSamplingOptions& sampling) {
        const std::tuple<SkFilterMode,SkMipmapMode> supported[] = {
            {SkFilterMode::kNearest, SkMipmapMode::kNone},    // legacy None
            {SkFilterMode::kLinear,  SkMipmapMode::kNone},    // legacy Low
            {SkFilterMode::kLinear,  SkMipmapMode::kNearest}, // legacy Medium
        };
        for (auto [f, m] : supported) {
            if (sampling.filter == f && sampling.mipmap == m) {
                return true;
            }
        }
        return false;
    };
    if (sampling.useCubic || !supported(sampling)) {
        return nullptr;
    }

    // SkBitmapProcShader stores bitmap coordinates in a 16bit buffer,
    // so it can't handle bitmaps larger than 65535.
    //
    // We back off another bit to 32767 to make small amounts of
    // intermediate math safe, e.g. in
    //
    //     SkFixed fx = ...;
    //     fx = tile(fx + SK_Fixed1);
    //
    // we want to make sure (fx + SK_Fixed1) never overflows.
    if (fImage-> width() > 32767 ||
        fImage->height() > 32767) {
        return nullptr;
    }

    SkMatrix inv;
    if (!this->computeTotalInverse(*rec.fMatrix, rec.fLocalMatrix, &inv) ||
        !legacy_shader_can_handle(inv)) {
        return nullptr;
    }

    if (!rec.isLegacyCompatible(fImage->colorSpace())) {
        return nullptr;
    }

    return SkBitmapProcLegacyShader::MakeContext(*this, fTileModeX, fTileModeY, sampling,
                                                 as_IB(fImage.get()), rec, alloc);
}
#endif

SkImage* SkImageShader::onIsAImage(SkMatrix* texM, SkTileMode xy[]) const {
    if (texM) {
        *texM = SkMatrix::I();
    }
    if (xy) {
        xy[0] = fTileModeX;
        xy[1] = fTileModeY;
    }
    return const_cast<SkImage*>(fImage.get());
}

sk_sp<SkShader> SkImageShader::Make(sk_sp<SkImage> image,
                                    SkTileMode tmx, SkTileMode tmy,
                                    const SkSamplingOptions& options,
                                    const SkMatrix* localMatrix,
                                    bool clampAsIfUnpremul) {
    SkRect subset = image ? SkRect::Make(image->dimensions()) : SkRect::MakeEmpty();
    return MakeSubset(std::move(image), subset, tmx, tmy, options, localMatrix, clampAsIfUnpremul);
}

sk_sp<SkShader> SkImageShader::MakeRaw(sk_sp<SkImage> image,
                                       SkTileMode tmx, SkTileMode tmy,
                                       const SkSamplingOptions& options,
                                       const SkMatrix* localMatrix) {
    if (options.useCubic) {
        return nullptr;
    }
    if (!image) {
        return SkShaders::Empty();
    }
    auto subset = SkRect::Make(image->dimensions());
    return SkLocalMatrixShader::MakeWrapped<SkImageShader>(localMatrix,
                                                           image,
                                                           subset,
                                                           tmx, tmy,
                                                           options,
                                                           /*raw=*/true,
                                                           /*clampAsIfUnpremul=*/false);
}

sk_sp<SkShader> SkImageShader::MakeSubset(sk_sp<SkImage> image,
                                          const SkRect& subset,
                                          SkTileMode tmx, SkTileMode tmy,
                                          const SkSamplingOptions& options,
                                          const SkMatrix* localMatrix,
                                          bool clampAsIfUnpremul) {
    auto is_unit = [](float x) {
        return x >= 0 && x <= 1;
    };
    if (options.useCubic) {
        if (!is_unit(options.cubic.B) || !is_unit(options.cubic.C)) {
            return nullptr;
        }
    }
    if (!image || subset.isEmpty()) {
        return SkShaders::Empty();
    }

    // Validate subset and check if we can drop it
    if (!SkRect::Make(image->bounds()).contains(subset)) {
        return nullptr;
    }
    // TODO(skbug.com/12784): GPU-only for now since it's only supported in onAsFragmentProcessor()
    SkASSERT(!needs_subset(image.get(), subset) || image->isTextureBacked());
    return SkLocalMatrixShader::MakeWrapped<SkImageShader>(localMatrix,
                                                           std::move(image),
                                                           subset,
                                                           tmx, tmy,
                                                           options,
                                                           /*raw=*/false,
                                                           clampAsIfUnpremul);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

#if defined(SK_GANESH)

#include "src/gpu/ganesh/GrColorInfo.h"
#include "src/gpu/ganesh/GrFPArgs.h"
#include "src/gpu/ganesh/effects/GrBlendFragmentProcessor.h"

std::unique_ptr<GrFragmentProcessor>
SkImageShader::asFragmentProcessor(const GrFPArgs& args, const MatrixRec& mRec) const {
    SkTileMode tileModes[2] = {fTileModeX, fTileModeY};
    const SkRect* subset = needs_subset(fImage.get(), fSubset) ? &fSubset : nullptr;
    auto fp = as_IB(fImage.get())->asFragmentProcessor(args.fContext,
                                                       fSampling,
                                                       tileModes,
                                                       SkMatrix::I(),
                                                       subset);
    if (!fp) {
        return nullptr;
    }

    bool success;
    std::tie(success, fp) = mRec.apply(std::move(fp));
    if (!success) {
        return nullptr;
    }

    if (!fRaw) {
        fp = GrColorSpaceXformEffect::Make(std::move(fp),
                                           fImage->colorSpace(),
                                           fImage->alphaType(),
                                           args.fDstColorInfo->colorSpace(),
                                           kPremul_SkAlphaType);

        if (fImage->isAlphaOnly()) {
            fp = GrBlendFragmentProcessor::Make<SkBlendMode::kDstIn>(std::move(fp), nullptr);
        }
    }

    return fp;
}

#endif

#if defined(SK_GRAPHITE)
void SkImageShader::addToKey(const skgpu::graphite::KeyContext& keyContext,
                             skgpu::graphite::PaintParamsKeyBuilder* builder,
                             skgpu::graphite::PipelineDataGatherer* gatherer) const {
    using namespace skgpu::graphite;

    ImageShaderBlock::ImageData imgData(fSampling, fTileModeX, fTileModeY, fSubset,
                                        ReadSwizzle::kRGBA);

    auto [ imageToDraw, newSampling ] = skgpu::graphite::GetGraphiteBacked(keyContext.recorder(),
                                                                           fImage.get(),
                                                                           fSampling);

    if (imageToDraw) {
        imgData.fSampling = newSampling;
        skgpu::Mipmapped mipmapped = (newSampling.mipmap != SkMipmapMode::kNone)
                                         ? skgpu::Mipmapped::kYes : skgpu::Mipmapped::kNo;

        auto [view, _] = as_IB(imageToDraw)->asView(keyContext.recorder(), mipmapped);
        imgData.fTextureProxy = view.refProxy();
        skgpu::Swizzle readSwizzle = view.swizzle();
        // If the color type is alpha-only, propagate the alpha value to the other channels.
        if (imageToDraw->isAlphaOnly()) {
            readSwizzle = skgpu::Swizzle::Concat(readSwizzle, skgpu::Swizzle("aaaa"));
        }
        imgData.fReadSwizzle = swizzle_class_to_read_enum(readSwizzle);
    }

    if (!fRaw) {
        imgData.fSteps = SkColorSpaceXformSteps(fImage->colorSpace(),
                                                fImage->alphaType(),
                                                keyContext.dstColorInfo().colorSpace(),
                                                keyContext.dstColorInfo().alphaType());

        if (fImage->isAlphaOnly()) {
            SkSpan<const float> constants = skgpu::GetPorterDuffBlendConstants(SkBlendMode::kDstIn);
            // expects dst, src
            PorterDuffBlendShaderBlock::BeginBlock(keyContext, builder, gatherer,
                                                   {constants});

                // dst
                SolidColorShaderBlock::BeginBlock(keyContext, builder, gatherer,
                                                  keyContext.paintColor());
                builder->endBlock();

                // src
                ImageShaderBlock::BeginBlock(keyContext, builder, gatherer, &imgData);
                builder->endBlock();

            builder->endBlock();
            return;
        }
    }

    ImageShaderBlock::BeginBlock(keyContext, builder, gatherer, &imgData);
    builder->endBlock();
}
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////
#include "src/core/SkImagePriv.h"

sk_sp<SkShader> SkMakeBitmapShaderForPaint(const SkPaint& paint, const SkBitmap& src,
                                           SkTileMode tmx, SkTileMode tmy,
                                           const SkSamplingOptions& sampling,
                                           const SkMatrix* localMatrix, SkCopyPixelsMode mode) {
    auto s = SkImageShader::Make(SkMakeImageFromRasterBitmap(src, mode),
                                 tmx, tmy, sampling, localMatrix);
    if (!s) {
        return nullptr;
    }
    if (SkColorTypeIsAlphaOnly(src.colorType()) && paint.getShader()) {
        // Compose the image shader with the paint's shader. Alpha images+shaders should output the
        // texture's alpha multiplied by the shader's color. DstIn (d*sa) will achieve this with
        // the source image and dst shader (MakeBlend takes dst first, src second).
        s = SkShaders::Blend(SkBlendMode::kDstIn, paint.refShader(), std::move(s));
    }
    return s;
}

void SkShaderBase::RegisterFlattenables() { SK_REGISTER_FLATTENABLE(SkImageShader); }

namespace {

struct MipLevelHelper {
    SkPixmap pm;
    SkMatrix inv;
    SkRasterPipeline_GatherCtx* gather;
    SkRasterPipeline_TileCtx* limitX;
    SkRasterPipeline_TileCtx* limitY;
    SkRasterPipeline_DecalTileCtx* decalCtx = nullptr;

    void allocAndInit(SkArenaAlloc* alloc,
                      const SkSamplingOptions& sampling,
                      SkTileMode tileModeX,
                      SkTileMode tileModeY) {
        gather = alloc->make<SkRasterPipeline_GatherCtx>();
        gather->pixels = pm.addr();
        gather->stride = pm.rowBytesAsPixels();
        gather->width = pm.width();
        gather->height = pm.height();

        if (sampling.useCubic) {
            SkImageShader::CubicResamplerMatrix(sampling.cubic.B, sampling.cubic.C)
                    .getColMajor(gather->weights);
        }

        limitX = alloc->make<SkRasterPipeline_TileCtx>();
        limitY = alloc->make<SkRasterPipeline_TileCtx>();
        limitX->scale = pm.width();
        limitX->invScale = 1.0f / pm.width();
        limitY->scale = pm.height();
        limitY->invScale = 1.0f / pm.height();

        // We would like an image that is mapped 1:1 with device pixels but at a half pixel offset
        // to select every pixel from the src image once. Our rasterizer biases upward. That is a
        // rect from 0.5...1.5 fills pixel 1 and not pixel 0. So we make exact integer pixel sample
        // values select the pixel to the left/above the integer value.
        //
        // Note that a mirror mapping between canvas and image space will not have this property -
        // on one side of the image a row/column will be skipped and one repeated on the other side.
        //
        // The GM nearest_half_pixel_image tests both of the above scenarios.
        //
        // The implementation of SkTileMode::kMirror also modifies integer pixel snapping to create
        // consistency when the sample coords are running backwards and must account for gather
        // modification we perform here. The GM mirror_tile tests this.
        if (!sampling.useCubic && sampling.filter == SkFilterMode::kNearest) {
            gather->roundDownAtInteger = true;
            limitX->mirrorBiasDir = limitY->mirrorBiasDir = 1;
        }

        if (tileModeX == SkTileMode::kDecal || tileModeY == SkTileMode::kDecal) {
            decalCtx = alloc->make<SkRasterPipeline_DecalTileCtx>();
            decalCtx->limit_x = limitX->scale;
            decalCtx->limit_y = limitY->scale;

            // When integer sample coords snap left/up then we want the right/bottom edge of the
            // image bounds to be inside the image rather than the left/top edge, that is (0, w]
            // rather than [0, w).
            if (gather->roundDownAtInteger) {
                decalCtx->inclusiveEdge_x = decalCtx->limit_x;
                decalCtx->inclusiveEdge_y = decalCtx->limit_y;
            }
        }
    }
};

}  // namespace

static SkSamplingOptions tweak_sampling(SkSamplingOptions sampling, const SkMatrix& matrix) {
    SkFilterMode filter = sampling.filter;

    // When the matrix is just an integer translate, bilerp == nearest neighbor.
    if (filter == SkFilterMode::kLinear &&
            matrix.getType() <= SkMatrix::kTranslate_Mask &&
            matrix.getTranslateX() == (int)matrix.getTranslateX() &&
            matrix.getTranslateY() == (int)matrix.getTranslateY()) {
        filter = SkFilterMode::kNearest;
    }

    return SkSamplingOptions(filter, sampling.mipmap);
}

bool SkImageShader::appendStages(const SkStageRec& rec, const MatrixRec& mRec) const {
    SkASSERT(!needs_subset(fImage.get(), fSubset));  // TODO(skbug.com/12784)

    // We only support certain sampling options in stages so far
    auto sampling = fSampling;
    if (sampling.isAniso()) {
        sampling = SkSamplingPriv::AnisoFallback(fImage->hasMipmaps());
    }

    SkRasterPipeline* p = rec.fPipeline;
    SkArenaAlloc* alloc = rec.fAlloc;

    SkMatrix baseInv;
    // If the total matrix isn't valid then we will always access the base MIP level.
    if (mRec.totalMatrixIsValid()) {
        if (!mRec.totalInverse(&baseInv)) {
            return false;
        }
        baseInv.normalizePerspective();
    }

    SkASSERT(!sampling.useCubic || sampling.mipmap == SkMipmapMode::kNone);
    auto* access = SkMipmapAccessor::Make(alloc, fImage.get(), baseInv, sampling.mipmap);
    if (!access) {
        return false;
    }

    MipLevelHelper upper;
    std::tie(upper.pm, upper.inv) = access->level();

    if (!sampling.useCubic) {
        // TODO: can tweak_sampling sometimes for cubic too when B=0
        if (mRec.totalMatrixIsValid()) {
            sampling = tweak_sampling(sampling, SkMatrix::Concat(upper.inv, baseInv));
        }
    }

    if (!mRec.apply(rec, upper.inv)) {
        return false;
    }

    upper.allocAndInit(alloc, sampling, fTileModeX, fTileModeY);

    MipLevelHelper lower;
    SkRasterPipeline_MipmapCtx* mipmapCtx = nullptr;
    float lowerWeight = access->lowerWeight();
    if (lowerWeight > 0) {
        std::tie(lower.pm, lower.inv) = access->lowerLevel();
        mipmapCtx = alloc->make<SkRasterPipeline_MipmapCtx>();
        mipmapCtx->lowerWeight = lowerWeight;
        mipmapCtx->scaleX = static_cast<float>(lower.pm.width()) / upper.pm.width();
        mipmapCtx->scaleY = static_cast<float>(lower.pm.height()) / upper.pm.height();

        lower.allocAndInit(alloc, sampling, fTileModeX, fTileModeY);

        p->append(SkRasterPipelineOp::mipmap_linear_init, mipmapCtx);
    }

    const bool decalBothAxes = fTileModeX == SkTileMode::kDecal && fTileModeY == SkTileMode::kDecal;

    auto append_tiling_and_gather = [&](const MipLevelHelper* level) {
        if (decalBothAxes) {
            p->append(SkRasterPipelineOp::decal_x_and_y,  level->decalCtx);
        } else {
            switch (fTileModeX) {
                case SkTileMode::kClamp: /* The gather_xxx stage will clamp for us. */
                    break;
                case SkTileMode::kMirror:
                    p->append(SkRasterPipelineOp::mirror_x, level->limitX);
                    break;
                case SkTileMode::kRepeat:
                    p->append(SkRasterPipelineOp::repeat_x, level->limitX);
                    break;
                case SkTileMode::kDecal:
                    p->append(SkRasterPipelineOp::decal_x, level->decalCtx);
                    break;
            }
            switch (fTileModeY) {
                case SkTileMode::kClamp: /* The gather_xxx stage will clamp for us. */
                    break;
                case SkTileMode::kMirror:
                    p->append(SkRasterPipelineOp::mirror_y, level->limitY);
                    break;
                case SkTileMode::kRepeat:
                    p->append(SkRasterPipelineOp::repeat_y, level->limitY);
                    break;
                case SkTileMode::kDecal:
                    p->append(SkRasterPipelineOp::decal_y, level->decalCtx);
                    break;
            }
        }

        void* ctx = level->gather;
        switch (level->pm.colorType()) {
            case kAlpha_8_SkColorType:      p->append(SkRasterPipelineOp::gather_a8,    ctx); break;
            case kA16_unorm_SkColorType:    p->append(SkRasterPipelineOp::gather_a16,   ctx); break;
            case kA16_float_SkColorType:    p->append(SkRasterPipelineOp::gather_af16,  ctx); break;
            case kRGB_565_SkColorType:      p->append(SkRasterPipelineOp::gather_565,   ctx); break;
            case kARGB_4444_SkColorType:    p->append(SkRasterPipelineOp::gather_4444,  ctx); break;
            case kR8G8_unorm_SkColorType:   p->append(SkRasterPipelineOp::gather_rg88,  ctx); break;
            case kR16G16_unorm_SkColorType: p->append(SkRasterPipelineOp::gather_rg1616,ctx); break;
            case kR16G16_float_SkColorType: p->append(SkRasterPipelineOp::gather_rgf16, ctx); break;
            case kRGBA_8888_SkColorType:    p->append(SkRasterPipelineOp::gather_8888,  ctx); break;

            case kRGBA_1010102_SkColorType:
                p->append(SkRasterPipelineOp::gather_1010102, ctx);
                break;

            case kR16G16B16A16_unorm_SkColorType:
                p->append(SkRasterPipelineOp::gather_16161616, ctx);
                break;

            case kRGBA_F16Norm_SkColorType:
            case kRGBA_F16_SkColorType:     p->append(SkRasterPipelineOp::gather_f16,   ctx); break;
            case kRGBA_F32_SkColorType:     p->append(SkRasterPipelineOp::gather_f32,   ctx); break;

            case kGray_8_SkColorType:       p->append(SkRasterPipelineOp::gather_a8,    ctx);
                                            p->append(SkRasterPipelineOp::alpha_to_gray    ); break;

            case kR8_unorm_SkColorType:     p->append(SkRasterPipelineOp::gather_a8,    ctx);
                                            p->append(SkRasterPipelineOp::alpha_to_red     ); break;

            case kRGB_888x_SkColorType:     p->append(SkRasterPipelineOp::gather_8888,  ctx);
                                            p->append(SkRasterPipelineOp::force_opaque     ); break;

            case kBGRA_1010102_SkColorType:
                p->append(SkRasterPipelineOp::gather_1010102, ctx);
                p->append(SkRasterPipelineOp::swap_rb);
                break;

            case kRGB_101010x_SkColorType:
                p->append(SkRasterPipelineOp::gather_1010102, ctx);
                p->append(SkRasterPipelineOp::force_opaque);
                break;

            case kBGR_101010x_XR_SkColorType:
                SkASSERT(false);
                break;

            case kBGR_101010x_SkColorType:
                p->append(SkRasterPipelineOp::gather_1010102, ctx);
                p->append(SkRasterPipelineOp::force_opaque);
                p->append(SkRasterPipelineOp::swap_rb);
                break;

            case kBGRA_8888_SkColorType:
                p->append(SkRasterPipelineOp::gather_8888, ctx);
                p->append(SkRasterPipelineOp::swap_rb);
                break;

            case kSRGBA_8888_SkColorType:
                p->append(SkRasterPipelineOp::gather_8888, ctx);
                p->append_transfer_function(*skcms_sRGB_TransferFunction());
                break;

            case kUnknown_SkColorType: SkASSERT(false);
        }
        if (level->decalCtx) {
            p->append(SkRasterPipelineOp::check_decal_mask, level->decalCtx);
        }
    };

    auto append_misc = [&] {
        SkColorSpace* cs = upper.pm.colorSpace();
        SkAlphaType   at = upper.pm.alphaType();

        // Color for alpha-only images comes from the paint (already converted to dst color space).
        if (SkColorTypeIsAlphaOnly(upper.pm.colorType()) && !fRaw) {
            p->append_set_rgb(alloc, rec.fPaintColor);

            cs = rec.fDstCS;
            at = kUnpremul_SkAlphaType;
        }

        // Bicubic filtering naturally produces out of range values on both sides of [0,1].
        if (sampling.useCubic) {
            p->append(at == kUnpremul_SkAlphaType || fClampAsIfUnpremul
                          ? SkRasterPipelineOp::clamp_01
                          : SkRasterPipelineOp::clamp_gamut);
        }

        // Transform color space and alpha type to match shader convention (dst CS, premul alpha).
        if (!fRaw) {
            alloc->make<SkColorSpaceXformSteps>(cs, at, rec.fDstCS, kPremul_SkAlphaType)->apply(p);
        }

        return true;
    };

    // Check for fast-path stages.
    // TODO: Could we use the fast-path stages for each level when doing linear mipmap filtering?
    SkColorType ct = upper.pm.colorType();
    if (true
        && (ct == kRGBA_8888_SkColorType || ct == kBGRA_8888_SkColorType)
        && !sampling.useCubic && sampling.filter == SkFilterMode::kLinear
        && sampling.mipmap != SkMipmapMode::kLinear
        && fTileModeX == SkTileMode::kClamp && fTileModeY == SkTileMode::kClamp) {

        p->append(SkRasterPipelineOp::bilerp_clamp_8888, upper.gather);
        if (ct == kBGRA_8888_SkColorType) {
            p->append(SkRasterPipelineOp::swap_rb);
        }
        return append_misc();
    }
    if (true
        && (ct == kRGBA_8888_SkColorType || ct == kBGRA_8888_SkColorType)
        && sampling.useCubic
        && fTileModeX == SkTileMode::kClamp && fTileModeY == SkTileMode::kClamp) {

        p->append(SkRasterPipelineOp::bicubic_clamp_8888, upper.gather);
        if (ct == kBGRA_8888_SkColorType) {
            p->append(SkRasterPipelineOp::swap_rb);
        }
        return append_misc();
    }

    // This context can be shared by both levels when doing linear mipmap filtering
    SkRasterPipeline_SamplerCtx* sampler = alloc->make<SkRasterPipeline_SamplerCtx>();

    auto sample = [&](SkRasterPipelineOp setup_x,
                      SkRasterPipelineOp setup_y,
                      const MipLevelHelper* level) {
        p->append(setup_x, sampler);
        p->append(setup_y, sampler);
        append_tiling_and_gather(level);
        p->append(SkRasterPipelineOp::accumulate, sampler);
    };

    auto sample_level = [&](const MipLevelHelper* level) {
        if (sampling.useCubic) {
            CubicResamplerMatrix(sampling.cubic.B, sampling.cubic.C).getColMajor(sampler->weights);

            p->append(SkRasterPipelineOp::bicubic_setup, sampler);

            sample(SkRasterPipelineOp::bicubic_n3x, SkRasterPipelineOp::bicubic_n3y, level);
            sample(SkRasterPipelineOp::bicubic_n1x, SkRasterPipelineOp::bicubic_n3y, level);
            sample(SkRasterPipelineOp::bicubic_p1x, SkRasterPipelineOp::bicubic_n3y, level);
            sample(SkRasterPipelineOp::bicubic_p3x, SkRasterPipelineOp::bicubic_n3y, level);

            sample(SkRasterPipelineOp::bicubic_n3x, SkRasterPipelineOp::bicubic_n1y, level);
            sample(SkRasterPipelineOp::bicubic_n1x, SkRasterPipelineOp::bicubic_n1y, level);
            sample(SkRasterPipelineOp::bicubic_p1x, SkRasterPipelineOp::bicubic_n1y, level);
            sample(SkRasterPipelineOp::bicubic_p3x, SkRasterPipelineOp::bicubic_n1y, level);

            sample(SkRasterPipelineOp::bicubic_n3x, SkRasterPipelineOp::bicubic_p1y, level);
            sample(SkRasterPipelineOp::bicubic_n1x, SkRasterPipelineOp::bicubic_p1y, level);
            sample(SkRasterPipelineOp::bicubic_p1x, SkRasterPipelineOp::bicubic_p1y, level);
            sample(SkRasterPipelineOp::bicubic_p3x, SkRasterPipelineOp::bicubic_p1y, level);

            sample(SkRasterPipelineOp::bicubic_n3x, SkRasterPipelineOp::bicubic_p3y, level);
            sample(SkRasterPipelineOp::bicubic_n1x, SkRasterPipelineOp::bicubic_p3y, level);
            sample(SkRasterPipelineOp::bicubic_p1x, SkRasterPipelineOp::bicubic_p3y, level);
            sample(SkRasterPipelineOp::bicubic_p3x, SkRasterPipelineOp::bicubic_p3y, level);

            p->append(SkRasterPipelineOp::move_dst_src);
        } else if (sampling.filter == SkFilterMode::kLinear) {
            p->append(SkRasterPipelineOp::bilinear_setup, sampler);

            sample(SkRasterPipelineOp::bilinear_nx, SkRasterPipelineOp::bilinear_ny, level);
            sample(SkRasterPipelineOp::bilinear_px, SkRasterPipelineOp::bilinear_ny, level);
            sample(SkRasterPipelineOp::bilinear_nx, SkRasterPipelineOp::bilinear_py, level);
            sample(SkRasterPipelineOp::bilinear_px, SkRasterPipelineOp::bilinear_py, level);

            p->append(SkRasterPipelineOp::move_dst_src);
        } else {
            append_tiling_and_gather(level);
        }
    };

    sample_level(&upper);

    if (mipmapCtx) {
        p->append(SkRasterPipelineOp::mipmap_linear_update, mipmapCtx);
        sample_level(&lower);
        p->append(SkRasterPipelineOp::mipmap_linear_finish, mipmapCtx);
    }

    return append_misc();
}

skvm::Color SkImageShader::program(skvm::Builder* p,
                                   skvm::Coord device,
                                   skvm::Coord origLocal,
                                   skvm::Color paint,
                                   const MatrixRec& mRec,
                                   const SkColorInfo& dst,
                                   skvm::Uniforms* uniforms,
                                   SkArenaAlloc* alloc) const {
    SkASSERT(!needs_subset(fImage.get(), fSubset));  // TODO(skbug.com/12784)

    auto sampling = fSampling;
    if (sampling.isAniso()) {
        sampling = SkSamplingPriv::AnisoFallback(fImage->hasMipmaps());
    }

    SkMatrix baseInv;
    // If the total matrix isn't valid then we will always access the base MIP level.
    if (mRec.totalMatrixIsValid()) {
        if (!mRec.totalInverse(&baseInv)) {
            return {};
        }
        baseInv.normalizePerspective();
    }

    SkASSERT(!sampling.useCubic || sampling.mipmap == SkMipmapMode::kNone);
    auto* access = SkMipmapAccessor::Make(alloc, fImage.get(), baseInv, sampling.mipmap);
    if (!access) {
        return {};
    }

    SkPixmap upper;
    SkMatrix upperInv;
    std::tie(upper, upperInv) = access->level();

    if (!sampling.useCubic) {
        // TODO: can tweak_sampling sometimes for cubic too when B=0
        if (mRec.totalMatrixIsValid()) {
            sampling = tweak_sampling(sampling, SkMatrix::Concat(upperInv, baseInv));
        }
    }

    SkPixmap lowerPixmap;
    SkMatrix lowerInv;
    SkPixmap* lower = nullptr;
    float lowerWeight = access->lowerWeight();
    if (lowerWeight > 0) {
        std::tie(lowerPixmap, lowerInv) = access->lowerLevel();
        lower = &lowerPixmap;
    }

    skvm::Coord upperLocal = origLocal;
    if (!mRec.apply(p, &upperLocal, uniforms, upperInv).has_value()) {
        return {};
    }

    // We can exploit image opacity to skip work unpacking alpha channels.
    const bool input_is_opaque = SkAlphaTypeIsOpaque(upper.alphaType())
                              || SkColorTypeIsAlwaysOpaque(upper.colorType());

    // Each call to sample() will try to rewrite the same uniforms over and over,
    // so remember where we start and reset back there each time.  That way each
    // sample() call uses the same uniform offsets.

    auto compute_clamp_limit = [&](float limit) {
        // Subtract an ulp so the upper clamp limit excludes limit itself.
        int bits;
        memcpy(&bits, &limit, 4);
        return p->uniformF(uniforms->push(bits-1));
    };

    // Except in the simplest case (no mips, no filtering), we reference uniforms
    // more than once. To avoid adding/registering them multiple times, we pre-load them
    // into a struct (just to logically group them together), based on the "current"
    // pixmap (level of a mipmap).
    //
    struct Uniforms {
        skvm::F32   w, iw, i2w,
                    h, ih, i2h;

        skvm::F32   clamp_w,
                    clamp_h;

        skvm::Uniform addr;
        skvm::I32     rowBytesAsPixels;

        skvm::PixelFormat pixelFormat;  // not a uniform, but needed for each texel sample,
                                        // so we store it here, since it is also dependent on
                                        // the current pixmap (level).
    };

    auto setup_uniforms = [&](const SkPixmap& pm) -> Uniforms {
        skvm::PixelFormat pixelFormat = skvm::SkColorType_to_PixelFormat(pm.colorType());
        return {
            p->uniformF(uniforms->pushF(     pm.width())),
            p->uniformF(uniforms->pushF(1.0f/pm.width())), // iff tileX == kRepeat
            p->uniformF(uniforms->pushF(0.5f/pm.width())), // iff tileX == kMirror

            p->uniformF(uniforms->pushF(     pm.height())),
            p->uniformF(uniforms->pushF(1.0f/pm.height())), // iff tileY == kRepeat
            p->uniformF(uniforms->pushF(0.5f/pm.height())), // iff tileY == kMirror

            compute_clamp_limit(pm. width()),
            compute_clamp_limit(pm.height()),

            uniforms->pushPtr(pm.addr()),
            p->uniform32(uniforms->push(pm.rowBytesAsPixels())),

            pixelFormat,
        };
    };

    auto sample_texel = [&](const Uniforms& u, skvm::F32 sx, skvm::F32 sy) -> skvm::Color {
        // repeat() and mirror() are written assuming they'll be followed by a [0,scale) clamp.
        auto repeat = [&](skvm::F32 v, skvm::F32 S, skvm::F32 I) {
            return v - floor(v * I) * S;
        };
        auto mirror = [&](skvm::F32 v, skvm::F32 S, skvm::F32 I2) {
            // abs( (v-scale) - (2*scale)*floor((v-scale)*(0.5f/scale)) - scale )
            //      {---A---}   {------------------B------------------}
            skvm::F32 A = v - S,
                      B = (S + S) * floor(A * I2);
            return abs(A - B - S);
        };
        switch (fTileModeX) {
            case SkTileMode::kDecal:  /* handled after gather */ break;
            case SkTileMode::kClamp:  /*    we always clamp   */ break;
            case SkTileMode::kRepeat: sx = repeat(sx, u.w, u.iw);  break;
            case SkTileMode::kMirror: sx = mirror(sx, u.w, u.i2w); break;
        }
        switch (fTileModeY) {
            case SkTileMode::kDecal:  /* handled after gather */  break;
            case SkTileMode::kClamp:  /*    we always clamp   */  break;
            case SkTileMode::kRepeat: sy = repeat(sy, u.h, u.ih);  break;
            case SkTileMode::kMirror: sy = mirror(sy, u.h, u.i2h); break;
        }

        // Always clamp sample coordinates to [0,width), [0,height), both for memory
        // safety and to handle the clamps still needed by kClamp, kRepeat, and kMirror.
        skvm::F32 clamped_x = clamp(sx, 0, u.clamp_w),
                  clamped_y = clamp(sy, 0, u.clamp_h);

        // Load pixels from pm.addr()[(int)sx + (int)sy*stride].
        skvm::I32 index = trunc(clamped_x) +
                          trunc(clamped_y) * u.rowBytesAsPixels;
        skvm::Color c = gather(u.pixelFormat, u.addr, index);

        // If we know the image is opaque, jump right to alpha = 1.0f, skipping work to unpack it.
        if (input_is_opaque) {
            c.a = p->splat(1.0f);
        }

        // Mask away any pixels that we tried to sample outside the bounds in kDecal.
        if (fTileModeX == SkTileMode::kDecal || fTileModeY == SkTileMode::kDecal) {
            skvm::I32 mask = p->splat(~0);
            if (fTileModeX == SkTileMode::kDecal) { mask &= (sx == clamped_x); }
            if (fTileModeY == SkTileMode::kDecal) { mask &= (sy == clamped_y); }
            c.r = pun_to_F32(p->bit_and(mask, pun_to_I32(c.r)));
            c.g = pun_to_F32(p->bit_and(mask, pun_to_I32(c.g)));
            c.b = pun_to_F32(p->bit_and(mask, pun_to_I32(c.b)));
            c.a = pun_to_F32(p->bit_and(mask, pun_to_I32(c.a)));
            // Notice that even if input_is_opaque, c.a might now be 0.
        }

        return c;
    };

    auto sample_level = [&](const SkPixmap& pm, skvm::Coord local) {
        const Uniforms u = setup_uniforms(pm);

        if (sampling.useCubic) {
            // All bicubic samples have the same fractional offset (fx,fy) from the center.
            // They're either the 16 corners of a 3x3 grid/ surrounding (x,y) at (0.5,0.5) off-center.
            skvm::F32 fx = fract(local.x + 0.5f),
                      fy = fract(local.y + 0.5f);
            skvm::F32 wx[4],
                      wy[4];

            SkM44 weights = CubicResamplerMatrix(sampling.cubic.B, sampling.cubic.C);

            auto dot = [](const skvm::F32 a[], const skvm::F32 b[]) {
                return a[0]*b[0] + a[1]*b[1] + a[2]*b[2] + a[3]*b[3];
            };
            const skvm::F32 tmpx[] =  { p->splat(1.0f), fx, fx*fx, fx*fx*fx };
            const skvm::F32 tmpy[] =  { p->splat(1.0f), fy, fy*fy, fy*fy*fy };

            for (int row = 0; row < 4; ++row) {
                SkV4 r = weights.row(row);
                skvm::F32 ru[] = {
                    p->uniformF(uniforms->pushF(r[0])),
                    p->uniformF(uniforms->pushF(r[1])),
                    p->uniformF(uniforms->pushF(r[2])),
                    p->uniformF(uniforms->pushF(r[3])),
                };
                wx[row] = dot(ru, tmpx);
                wy[row] = dot(ru, tmpy);
            }

            skvm::Color c;
            c.r = c.g = c.b = c.a = p->splat(0.0f);

            skvm::F32 sy = local.y - 1.5f;
            for (int j = 0; j < 4; j++, sy += 1.0f) {
                skvm::F32 sx = local.x - 1.5f;
                for (int i = 0; i < 4; i++, sx += 1.0f) {
                    skvm::Color s = sample_texel(u, sx,sy);
                    skvm::F32   w = wx[i] * wy[j];

                    c.r += s.r * w;
                    c.g += s.g * w;
                    c.b += s.b * w;
                    c.a += s.a * w;
                }
            }
            return c;
        } else if (sampling.filter == SkFilterMode::kLinear) {
            // Our four sample points are the corners of a logical 1x1 pixel
            // box surrounding (x,y) at (0.5,0.5) off-center.
            skvm::F32 left   = local.x - 0.5f,
                      top    = local.y - 0.5f,
                      right  = local.x + 0.5f,
                      bottom = local.y + 0.5f;

            // The fractional parts of right and bottom are our lerp factors in x and y respectively.
            skvm::F32 fx = fract(right ),
                      fy = fract(bottom);

            return lerp(lerp(sample_texel(u, left,top   ), sample_texel(u, right,top   ), fx),
                        lerp(sample_texel(u, left,bottom), sample_texel(u, right,bottom), fx), fy);
        } else {
            SkASSERT(sampling.filter == SkFilterMode::kNearest);
            // Our rasterizer biases upward. That is a rect from 0.5...1.5 fills pixel 1 and not
            // pixel 0. To make an image that is mapped 1:1 with device pixels but at a half pixel
            // offset select every pixel from the src image once we make exact integer pixel sample
            // values round down not up. Note that a mirror mapping will not have this property.
            local.x = skvm::pun_to_F32(skvm::pun_to_I32(local.x) - 1);
            local.y = skvm::pun_to_F32(skvm::pun_to_I32(local.y) - 1);
            return sample_texel(u, local.x,local.y);
        }
    };

    skvm::Color c = sample_level(upper, upperLocal);
    if (lower) {
        skvm::Coord lowerLocal = origLocal;
        if (!mRec.apply(p, &lowerLocal, uniforms, lowerInv)) {
            return {};
        }
        // lower * weight + upper * (1 - weight)
        c = lerp(c,
                 sample_level(*lower, lowerLocal),
                 p->uniformF(uniforms->pushF(lowerWeight)));
    }

    // If the input is opaque and we're not in decal mode, that means the output is too.
    // Forcing *a to 1.0 here will retroactively skip any work we did to interpolate sample alphas.
    if (input_is_opaque
            && fTileModeX != SkTileMode::kDecal
            && fTileModeY != SkTileMode::kDecal) {
        c.a = p->splat(1.0f);
    }

    // Alpha-only images get their color from the paint (already converted to dst color space).
    SkColorSpace* cs = upper.colorSpace();
    SkAlphaType   at = upper.alphaType();
    if (SkColorTypeIsAlphaOnly(upper.colorType()) && !fRaw) {
        c.r = paint.r;
        c.g = paint.g;
        c.b = paint.b;

        cs = dst.colorSpace();
        at = kUnpremul_SkAlphaType;
    }

    if (sampling.useCubic) {
        // Bicubic filtering naturally produces out of range values on both sides of [0,1].
        c.a = clamp01(c.a);

        skvm::F32 limit = (at == kUnpremul_SkAlphaType || fClampAsIfUnpremul)
                        ? p->splat(1.0f)
                        : c.a;
        c.r = clamp(c.r, 0.0f, limit);
        c.g = clamp(c.g, 0.0f, limit);
        c.b = clamp(c.b, 0.0f, limit);
    }

    return fRaw ? c
                : SkColorSpaceXformSteps{cs, at, dst.colorSpace(), dst.alphaType()}.program(
                          p, uniforms, c);
}
