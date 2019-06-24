/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/effects/SkDisplacementMapEffect.h"

#include "include/core/SkBitmap.h"
#include "include/core/SkUnPreMultiply.h"
#include "include/private/SkColorData.h"
#include "src/core/SkImageFilterPriv.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkSpecialImage.h"
#include "src/core/SkWriteBuffer.h"
#if SK_SUPPORT_GPU
#include "include/gpu/GrTexture.h"
#include "include/private/GrRecordingContext.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrClip.h"
#include "src/gpu/GrColorSpaceXform.h"
#include "src/gpu/GrCoordTransform.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/GrRenderTargetContext.h"
#include "src/gpu/GrTextureProxy.h"
#include "src/gpu/SkGr.h"
#include "src/gpu/effects/GrTextureDomain.h"
#include "src/gpu/glsl/GrGLSLFragmentProcessor.h"
#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/glsl/GrGLSLProgramDataManager.h"
#include "src/gpu/glsl/GrGLSLUniformHandler.h"
#endif

namespace {

#define kChannelSelectorKeyBits 3  // Max value is 4, so 3 bits are required at most

// Shift values to extract channels from an SkColor (SkColorGetR, SkColorGetG, etc)
const uint8_t gChannelTypeToShift[] = {
     0,  // unknown
    16,  // R
     8,  // G
     0,  // B
    24,  // A
};
struct Extractor {
    Extractor(SkDisplacementMapEffect::ChannelSelectorType typeX,
              SkDisplacementMapEffect::ChannelSelectorType typeY)
        : fShiftX(gChannelTypeToShift[typeX])
        , fShiftY(gChannelTypeToShift[typeY])
    {}

    unsigned fShiftX, fShiftY;

    unsigned getX(SkColor c) const { return (c >> fShiftX) & 0xFF; }
    unsigned getY(SkColor c) const { return (c >> fShiftY) & 0xFF; }
};

void computeDisplacement(Extractor ex, const SkVector& scale, SkBitmap* dst,
                         const SkBitmap& displ, const SkIPoint& offset,
                         const SkBitmap& src,
                         const SkIRect& bounds) {
    static const SkScalar Inv8bit = SkScalarInvert(255);
    const int srcW = src.width();
    const int srcH = src.height();
    const SkVector scaleForColor = SkVector::Make(scale.fX * Inv8bit, scale.fY * Inv8bit);
    const SkVector scaleAdj = SkVector::Make(SK_ScalarHalf - scale.fX * SK_ScalarHalf,
                                             SK_ScalarHalf - scale.fY * SK_ScalarHalf);
    SkPMColor* dstPtr = dst->getAddr32(0, 0);
    for (int y = bounds.top(); y < bounds.bottom(); ++y) {
        const SkPMColor* displPtr = displ.getAddr32(bounds.left() + offset.fX, y + offset.fY);
        for (int x = bounds.left(); x < bounds.right(); ++x, ++displPtr) {
            SkColor c = SkUnPreMultiply::PMColorToColor(*displPtr);

            SkScalar displX = scaleForColor.fX * ex.getX(c) + scaleAdj.fX;
            SkScalar displY = scaleForColor.fY * ex.getY(c) + scaleAdj.fY;
            // Truncate the displacement values
            const int32_t srcX = Sk32_sat_add(x, SkScalarTruncToInt(displX));
            const int32_t srcY = Sk32_sat_add(y, SkScalarTruncToInt(displY));
            *dstPtr++ = ((srcX < 0) || (srcX >= srcW) || (srcY < 0) || (srcY >= srcH)) ?
                      0 : *(src.getAddr32(srcX, srcY));
        }
    }
}

bool channel_selector_type_is_valid(SkDisplacementMapEffect::ChannelSelectorType cst) {
    switch (cst) {
    case SkDisplacementMapEffect::kUnknown_ChannelSelectorType:
    case SkDisplacementMapEffect::kR_ChannelSelectorType:
    case SkDisplacementMapEffect::kG_ChannelSelectorType:
    case SkDisplacementMapEffect::kB_ChannelSelectorType:
    case SkDisplacementMapEffect::kA_ChannelSelectorType:
        return true;
    default:
        break;
    }
    return false;
}

} // end namespace

///////////////////////////////////////////////////////////////////////////////

sk_sp<SkImageFilter> SkDisplacementMapEffect::Make(ChannelSelectorType xChannelSelector,
                                                   ChannelSelectorType yChannelSelector,
                                                   SkScalar scale,
                                                   sk_sp<SkImageFilter> displacement,
                                                   sk_sp<SkImageFilter> color,
                                                   const CropRect* cropRect) {
    if (!channel_selector_type_is_valid(xChannelSelector) ||
        !channel_selector_type_is_valid(yChannelSelector)) {
        return nullptr;
    }

    sk_sp<SkImageFilter> inputs[2] = { std::move(displacement), std::move(color) };
    return sk_sp<SkImageFilter>(new SkDisplacementMapEffect(xChannelSelector,
                                                            yChannelSelector,
                                                            scale, inputs, cropRect));
}

SkDisplacementMapEffect::SkDisplacementMapEffect(ChannelSelectorType xChannelSelector,
                                                 ChannelSelectorType yChannelSelector,
                                                 SkScalar scale,
                                                 sk_sp<SkImageFilter> inputs[2],
                                                 const CropRect* cropRect)
    : INHERITED(inputs, 2, cropRect)
    , fXChannelSelector(xChannelSelector)
    , fYChannelSelector(yChannelSelector)
    , fScale(scale) {
}

SkDisplacementMapEffect::~SkDisplacementMapEffect() {
}

sk_sp<SkFlattenable> SkDisplacementMapEffect::CreateProc(SkReadBuffer& buffer) {
    SK_IMAGEFILTER_UNFLATTEN_COMMON(common, 2);

    ChannelSelectorType xsel = buffer.read32LE(kLast_ChannelSelectorType);
    ChannelSelectorType ysel = buffer.read32LE(kLast_ChannelSelectorType);
    SkScalar scale = buffer.readScalar();

    return Make(xsel, ysel, scale, common.getInput(0), common.getInput(1), &common.cropRect());
}

void SkDisplacementMapEffect::flatten(SkWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    buffer.writeInt((int) fXChannelSelector);
    buffer.writeInt((int) fYChannelSelector);
    buffer.writeScalar(fScale);
}

#if SK_SUPPORT_GPU
class GrDisplacementMapEffect : public GrFragmentProcessor {
public:
    static std::unique_ptr<GrFragmentProcessor> Make(
            SkDisplacementMapEffect::ChannelSelectorType xChannelSelector,
            SkDisplacementMapEffect::ChannelSelectorType yChannelSelector, SkVector scale,
            sk_sp<GrTextureProxy> displacement, const SkIRect& displSubset,
            const SkMatrix& offsetMatrix, sk_sp<GrTextureProxy> color, const SkIRect& colorSubset) {
        return std::unique_ptr<GrFragmentProcessor>(new GrDisplacementMapEffect(
                xChannelSelector, yChannelSelector, scale, std::move(displacement), displSubset,
                offsetMatrix, std::move(color), colorSubset));
    }

    ~GrDisplacementMapEffect() override;

    SkDisplacementMapEffect::ChannelSelectorType xChannelSelector() const {
        return fXChannelSelector;
    }
    SkDisplacementMapEffect::ChannelSelectorType yChannelSelector() const {
        return fYChannelSelector;
    }
    const SkVector& scale() const { return fScale; }

    const char* name() const override { return "DisplacementMap"; }
    const GrTextureDomain& domain() const { return fDomain; }

    std::unique_ptr<GrFragmentProcessor> clone() const override;

private:
    GrDisplacementMapEffect(const GrDisplacementMapEffect&);

    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override;

    void onGetGLSLProcessorKey(const GrShaderCaps& caps, GrProcessorKeyBuilder* b) const override;

    bool onIsEqual(const GrFragmentProcessor&) const override;

    GrDisplacementMapEffect(SkDisplacementMapEffect::ChannelSelectorType xChannelSelector,
                            SkDisplacementMapEffect::ChannelSelectorType yChannelSelector,
                            const SkVector& scale, sk_sp<GrTextureProxy> displacement,
                            const SkIRect& displSubset, const SkMatrix& offsetMatrix,
                            sk_sp<GrTextureProxy> color, const SkIRect& colorSubset);

    const TextureSampler& onTextureSampler(int i) const override {
        return IthTextureSampler(i, fDisplacementSampler, fColorSampler);
    }

    GR_DECLARE_FRAGMENT_PROCESSOR_TEST

    GrCoordTransform            fDisplacementTransform;
    TextureSampler              fDisplacementSampler;
    GrCoordTransform            fColorTransform;
    GrTextureDomain             fDomain;
    TextureSampler              fColorSampler;
    SkDisplacementMapEffect::ChannelSelectorType fXChannelSelector;
    SkDisplacementMapEffect::ChannelSelectorType fYChannelSelector;
    SkVector fScale;

    typedef GrFragmentProcessor INHERITED;
};
#endif

sk_sp<SkSpecialImage> SkDisplacementMapEffect::onFilterImage(SkSpecialImage* source,
                                                             const Context& ctx,
                                                             SkIPoint* offset) const {
    SkIPoint colorOffset = SkIPoint::Make(0, 0);
    sk_sp<SkSpecialImage> color(this->filterInput(1, source, ctx, &colorOffset));
    if (!color) {
        return nullptr;
    }

    SkIPoint displOffset = SkIPoint::Make(0, 0);
    // Creation of the displacement map should happen in a non-colorspace aware context. This
    // texture is a purely mathematical construct, so we want to just operate on the stored
    // values. Consider:
    // User supplies an sRGB displacement map. If we're rendering to a wider gamut, then we could
    // end up filtering the displacement map into that gamut, which has the effect of reducing
    // the amount of displacement that it represents (as encoded values move away from the
    // primaries).
    // With a more complex DAG attached to this input, it's not clear that working in ANY specific
    // color space makes sense, so we ignore color spaces (and gamma) entirely. This may not be
    // ideal, but it's at least consistent and predictable.
    Context displContext(ctx.ctm(), ctx.clipBounds(), ctx.cache(),
                         OutputProperties(kN32_SkColorType, nullptr));
    sk_sp<SkSpecialImage> displ(this->filterInput(0, source, displContext, &displOffset));
    if (!displ) {
        return nullptr;
    }

    const SkIRect srcBounds = SkIRect::MakeXYWH(colorOffset.x(), colorOffset.y(),
                                                color->width(), color->height());

    // Both paths do bounds checking on color pixel access, we don't need to
    // pad the color bitmap to bounds here.
    SkIRect bounds;
    if (!this->applyCropRect(ctx, srcBounds, &bounds)) {
        return nullptr;
    }

    SkIRect displBounds;
    displ = this->applyCropRectAndPad(ctx, displ.get(), &displOffset, &displBounds);
    if (!displ) {
        return nullptr;
    }

    if (!bounds.intersect(displBounds)) {
        return nullptr;
    }

    const SkIRect colorBounds = bounds.makeOffset(-colorOffset.x(), -colorOffset.y());
    // If the offset overflowed (saturated) then we have to abort, as we need their
    // dimensions to be equal. See https://bugs.chromium.org/p/oss-fuzz/issues/detail?id=7209
    if (colorBounds.size() != bounds.size()) {
        return nullptr;
    }

    SkVector scale = SkVector::Make(fScale, fScale);
    ctx.ctm().mapVectors(&scale, 1);

#if SK_SUPPORT_GPU
    if (source->isTextureBacked()) {
        auto context = source->getContext();

        sk_sp<GrTextureProxy> colorProxy(color->asTextureProxyRef(context));
        sk_sp<GrTextureProxy> displProxy(displ->asTextureProxyRef(context));
        if (!colorProxy || !displProxy) {
            return nullptr;
        }
        const auto isProtected = colorProxy->isProtected();

        SkMatrix offsetMatrix = SkMatrix::MakeTrans(SkIntToScalar(colorOffset.fX - displOffset.fX),
                                                    SkIntToScalar(colorOffset.fY - displOffset.fY));
        SkColorSpace* colorSpace = ctx.outputProperties().colorSpace();

        std::unique_ptr<GrFragmentProcessor> fp =
                GrDisplacementMapEffect::Make(fXChannelSelector,
                                              fYChannelSelector,
                                              scale,
                                              std::move(displProxy),
                                              displ->subset(),
                                              offsetMatrix,
                                              std::move(colorProxy),
                                              color->subset());
        fp = GrColorSpaceXformEffect::Make(std::move(fp), color->getColorSpace(),
                                           color->alphaType(), colorSpace);

        GrPaint paint;
        paint.addColorFragmentProcessor(std::move(fp));
        paint.setPorterDuffXPFactory(SkBlendMode::kSrc);
        SkMatrix matrix;
        matrix.setTranslate(-SkIntToScalar(colorBounds.x()), -SkIntToScalar(colorBounds.y()));
        SkColorType colorType = ctx.outputProperties().colorType();
        GrPixelConfig config = SkColorType2GrPixelConfig(colorType);
        GrBackendFormat format =
                context->priv().caps()->getBackendFormatFromColorType(colorType);

        sk_sp<GrRenderTargetContext> renderTargetContext(
                context->priv().makeDeferredRenderTargetContext(
                        format, SkBackingFit::kApprox, bounds.width(), bounds.height(), config,
                        sk_ref_sp(colorSpace), 1, GrMipMapped::kNo, kBottomLeft_GrSurfaceOrigin,
                        nullptr, SkBudgeted::kYes,
                        isProtected ? GrProtected::kYes : GrProtected::kNo));
        if (!renderTargetContext) {
            return nullptr;
        }

        renderTargetContext->drawRect(GrNoClip(), std::move(paint), GrAA::kNo, matrix,
                                      SkRect::Make(colorBounds));

        offset->fX = bounds.left();
        offset->fY = bounds.top();
        return SkSpecialImage::MakeDeferredFromGpu(
                context,
                SkIRect::MakeWH(bounds.width(), bounds.height()),
                kNeedNewImageUniqueID_SpecialImage,
                renderTargetContext->asTextureProxyRef(),
                renderTargetContext->colorSpaceInfo().refColorSpace());
    }
#endif

    SkBitmap colorBM, displBM;

    if (!color->getROPixels(&colorBM) || !displ->getROPixels(&displBM)) {
        return nullptr;
    }

    if ((colorBM.colorType() != kN32_SkColorType) ||
        (displBM.colorType() != kN32_SkColorType)) {
        return nullptr;
    }

    if (!colorBM.getPixels() || !displBM.getPixels()) {
        return nullptr;
    }

    SkImageInfo info = SkImageInfo::MakeN32(bounds.width(), bounds.height(),
                                            colorBM.alphaType());

    SkBitmap dst;
    if (!dst.tryAllocPixels(info)) {
        return nullptr;
    }

    computeDisplacement(Extractor(fXChannelSelector, fYChannelSelector), scale, &dst,
                        displBM, colorOffset - displOffset, colorBM, colorBounds);

    offset->fX = bounds.left();
    offset->fY = bounds.top();
    return SkSpecialImage::MakeFromRaster(SkIRect::MakeWH(bounds.width(), bounds.height()),
                                          dst);
}

SkRect SkDisplacementMapEffect::computeFastBounds(const SkRect& src) const {
    SkRect bounds = this->getColorInput() ? this->getColorInput()->computeFastBounds(src) : src;
    bounds.outset(SkScalarAbs(fScale) * SK_ScalarHalf, SkScalarAbs(fScale) * SK_ScalarHalf);
    return bounds;
}

SkIRect SkDisplacementMapEffect::onFilterNodeBounds(const SkIRect& src, const SkMatrix& ctm,
                                                    MapDirection, const SkIRect* inputRect) const {
    SkVector scale = SkVector::Make(fScale, fScale);
    ctm.mapVectors(&scale, 1);
    return src.makeOutset(SkScalarCeilToInt(SkScalarAbs(scale.fX) * SK_ScalarHalf),
                          SkScalarCeilToInt(SkScalarAbs(scale.fY) * SK_ScalarHalf));
}

SkIRect SkDisplacementMapEffect::onFilterBounds(const SkIRect& src, const SkMatrix& ctm,
                                                MapDirection dir, const SkIRect* inputRect) const {
    // Recurse only into color input.
    if (this->getColorInput()) {
        return this->getColorInput()->filterBounds(src, ctm, dir, inputRect);
    }
    return src;
}

///////////////////////////////////////////////////////////////////////////////

#if SK_SUPPORT_GPU
class GrGLDisplacementMapEffect : public GrGLSLFragmentProcessor {
public:
    void emitCode(EmitArgs&) override;

    static inline void GenKey(const GrProcessor&, const GrShaderCaps&, GrProcessorKeyBuilder*);

protected:
    void onSetData(const GrGLSLProgramDataManager&, const GrFragmentProcessor&) override;

private:
    typedef GrGLSLProgramDataManager::UniformHandle UniformHandle;

    UniformHandle fScaleUni;
    GrTextureDomain::GLDomain fGLDomain;

    typedef GrGLSLFragmentProcessor INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

GrGLSLFragmentProcessor* GrDisplacementMapEffect::onCreateGLSLInstance() const {
    return new GrGLDisplacementMapEffect;
}

void GrDisplacementMapEffect::onGetGLSLProcessorKey(const GrShaderCaps& caps,
                                                    GrProcessorKeyBuilder* b) const {
    GrGLDisplacementMapEffect::GenKey(*this, caps, b);
}

GrDisplacementMapEffect::GrDisplacementMapEffect(
        SkDisplacementMapEffect::ChannelSelectorType xChannelSelector,
        SkDisplacementMapEffect::ChannelSelectorType yChannelSelector,
        const SkVector& scale,
        sk_sp<GrTextureProxy> displacement,
        const SkIRect& displSubset,
        const SkMatrix& offsetMatrix,
        sk_sp<GrTextureProxy> color,
        const SkIRect& colorSubset)
        : INHERITED(kGrDisplacementMapEffect_ClassID,
                    GrFragmentProcessor::kNone_OptimizationFlags)
        , fDisplacementTransform(
                SkMatrix::Concat(SkMatrix::MakeTrans(displSubset.x(), displSubset.y()),
                                 offsetMatrix),
                displacement.get())
        , fDisplacementSampler(displacement)
        , fColorTransform(SkMatrix::MakeTrans(colorSubset.x(), colorSubset.y()), color.get())
        , fDomain(color.get(),
                  GrTextureDomain::MakeTexelDomain(colorSubset,
                                                   GrTextureDomain::kDecal_Mode),
                  GrTextureDomain::kDecal_Mode, GrTextureDomain::kDecal_Mode)
        , fColorSampler(color)
        , fXChannelSelector(xChannelSelector)
        , fYChannelSelector(yChannelSelector)
        , fScale(scale) {
    this->addCoordTransform(&fDisplacementTransform);
    this->addCoordTransform(&fColorTransform);
    this->setTextureSamplerCnt(2);
}

GrDisplacementMapEffect::GrDisplacementMapEffect(const GrDisplacementMapEffect& that)
        : INHERITED(kGrDisplacementMapEffect_ClassID, that.optimizationFlags())
        , fDisplacementTransform(that.fDisplacementTransform)
        , fDisplacementSampler(that.fDisplacementSampler)
        , fColorTransform(that.fColorTransform)
        , fDomain(that.fDomain)
        , fColorSampler(that.fColorSampler)
        , fXChannelSelector(that.fXChannelSelector)
        , fYChannelSelector(that.fYChannelSelector)
        , fScale(that.fScale) {
    this->addCoordTransform(&fDisplacementTransform);
    this->addCoordTransform(&fColorTransform);
    this->setTextureSamplerCnt(2);
}

GrDisplacementMapEffect::~GrDisplacementMapEffect() {}

std::unique_ptr<GrFragmentProcessor> GrDisplacementMapEffect::clone() const {
    return std::unique_ptr<GrFragmentProcessor>(new GrDisplacementMapEffect(*this));
}

bool GrDisplacementMapEffect::onIsEqual(const GrFragmentProcessor& sBase) const {
    const GrDisplacementMapEffect& s = sBase.cast<GrDisplacementMapEffect>();
    return fXChannelSelector == s.fXChannelSelector &&
           fYChannelSelector == s.fYChannelSelector &&
           fScale == s.fScale;
}

///////////////////////////////////////////////////////////////////////////////

GR_DEFINE_FRAGMENT_PROCESSOR_TEST(GrDisplacementMapEffect);

#if GR_TEST_UTILS
std::unique_ptr<GrFragmentProcessor> GrDisplacementMapEffect::TestCreate(GrProcessorTestData* d) {
    int texIdxDispl = d->fRandom->nextBool() ? GrProcessorUnitTest::kSkiaPMTextureIdx :
                                               GrProcessorUnitTest::kAlphaTextureIdx;
    int texIdxColor = d->fRandom->nextBool() ? GrProcessorUnitTest::kSkiaPMTextureIdx :
                                               GrProcessorUnitTest::kAlphaTextureIdx;
    sk_sp<GrTextureProxy> dispProxy = d->textureProxy(texIdxDispl);
    sk_sp<GrTextureProxy> colorProxy = d->textureProxy(texIdxColor);
    static const int kMaxComponent = 4;
    SkDisplacementMapEffect::ChannelSelectorType xChannelSelector =
        static_cast<SkDisplacementMapEffect::ChannelSelectorType>(
                d->fRandom->nextRangeU(1, kMaxComponent));
    SkDisplacementMapEffect::ChannelSelectorType yChannelSelector =
        static_cast<SkDisplacementMapEffect::ChannelSelectorType>(
                d->fRandom->nextRangeU(1, kMaxComponent));
    SkVector scale = SkVector::Make(d->fRandom->nextRangeScalar(0, 100.0f),
                                    d->fRandom->nextRangeScalar(0, 100.0f));
    SkISize colorDimensions;
    colorDimensions.fWidth = d->fRandom->nextRangeU(0, colorProxy->width());
    colorDimensions.fHeight = d->fRandom->nextRangeU(0, colorProxy->height());
    SkIRect dispRect = SkIRect::MakeWH(dispProxy->width(), dispProxy->height());
    return GrDisplacementMapEffect::Make(xChannelSelector, yChannelSelector, scale,
                                         std::move(dispProxy),
                                         dispRect,
                                         SkMatrix::I(),
                                         std::move(colorProxy), SkIRect::MakeSize(colorDimensions));
}

#endif

///////////////////////////////////////////////////////////////////////////////

void GrGLDisplacementMapEffect::emitCode(EmitArgs& args) {
    const GrDisplacementMapEffect& displacementMap = args.fFp.cast<GrDisplacementMapEffect>();
    const GrTextureDomain& domain = displacementMap.domain();

    fScaleUni = args.fUniformHandler->addUniform(kFragment_GrShaderFlag, kHalf2_GrSLType, "Scale");
    const char* scaleUni = args.fUniformHandler->getUniformCStr(fScaleUni);
    const char* dColor = "dColor";
    const char* cCoords = "cCoords";
    const char* nearZero = "1e-6"; // Since 6.10352e-5 is the smallest half float, use
                                   // a number smaller than that to approximate 0, but
                                   // leave room for 32-bit float GPU rounding errors.

    GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;
    fragBuilder->codeAppendf("\t\thalf4 %s = ", dColor);
    fragBuilder->appendTextureLookup(args.fTexSamplers[0], args.fTransformedCoords[0].c_str(),
                                     args.fTransformedCoords[0].getType());
    fragBuilder->codeAppend(";\n");

    // Unpremultiply the displacement
    fragBuilder->codeAppendf(
        "\t\t%s.rgb = (%s.a < %s) ? half3(0.0) : saturate(%s.rgb / %s.a);",
        dColor, dColor, nearZero, dColor, dColor);
    SkString coords2D = fragBuilder->ensureCoords2D(args.fTransformedCoords[1]);
    fragBuilder->codeAppendf("\t\tfloat2 %s = %s + %s*(%s.",
                             cCoords, coords2D.c_str(), scaleUni, dColor);

    switch (displacementMap.xChannelSelector()) {
      case SkDisplacementMapEffect::kR_ChannelSelectorType:
        fragBuilder->codeAppend("r");
        break;
      case SkDisplacementMapEffect::kG_ChannelSelectorType:
        fragBuilder->codeAppend("g");
        break;
      case SkDisplacementMapEffect::kB_ChannelSelectorType:
        fragBuilder->codeAppend("b");
        break;
      case SkDisplacementMapEffect::kA_ChannelSelectorType:
        fragBuilder->codeAppend("a");
        break;
      case SkDisplacementMapEffect::kUnknown_ChannelSelectorType:
      default:
        SkDEBUGFAIL("Unknown X channel selector");
    }

    switch (displacementMap.yChannelSelector()) {
      case SkDisplacementMapEffect::kR_ChannelSelectorType:
        fragBuilder->codeAppend("r");
        break;
      case SkDisplacementMapEffect::kG_ChannelSelectorType:
        fragBuilder->codeAppend("g");
        break;
      case SkDisplacementMapEffect::kB_ChannelSelectorType:
        fragBuilder->codeAppend("b");
        break;
      case SkDisplacementMapEffect::kA_ChannelSelectorType:
        fragBuilder->codeAppend("a");
        break;
      case SkDisplacementMapEffect::kUnknown_ChannelSelectorType:
      default:
        SkDEBUGFAIL("Unknown Y channel selector");
    }
    fragBuilder->codeAppend("-half2(0.5));\t\t");

    fGLDomain.sampleTexture(fragBuilder,
                            args.fUniformHandler,
                            args.fShaderCaps,
                            domain,
                            args.fOutputColor,
                            SkString(cCoords),
                            args.fTexSamplers[1]);
    fragBuilder->codeAppend(";\n");
}

void GrGLDisplacementMapEffect::onSetData(const GrGLSLProgramDataManager& pdman,
                                          const GrFragmentProcessor& proc) {
    const GrDisplacementMapEffect& displacementMap = proc.cast<GrDisplacementMapEffect>();
    GrTextureProxy* proxy = displacementMap.textureSampler(1).proxy();
    GrTexture* colorTex = proxy->peekTexture();

    SkScalar scaleX = displacementMap.scale().fX / colorTex->width();
    SkScalar scaleY = displacementMap.scale().fY / colorTex->height();
    pdman.set2f(fScaleUni, SkScalarToFloat(scaleX),
                proxy->origin() == kTopLeft_GrSurfaceOrigin ?
                SkScalarToFloat(scaleY) : SkScalarToFloat(-scaleY));
    fGLDomain.setData(pdman, displacementMap.domain(), proxy,
                      displacementMap.textureSampler(1).samplerState());
}

void GrGLDisplacementMapEffect::GenKey(const GrProcessor& proc,
                                       const GrShaderCaps&, GrProcessorKeyBuilder* b) {
    const GrDisplacementMapEffect& displacementMap = proc.cast<GrDisplacementMapEffect>();

    uint32_t xKey = displacementMap.xChannelSelector();
    uint32_t yKey = displacementMap.yChannelSelector() << kChannelSelectorKeyBits;

    b->add32(xKey | yKey);
}
#endif
