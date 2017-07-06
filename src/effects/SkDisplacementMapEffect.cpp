/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkDisplacementMapEffect.h"

#include "SkBitmap.h"
#include "SkColorSpaceXformer.h"
#include "SkReadBuffer.h"
#include "SkSpecialImage.h"
#include "SkWriteBuffer.h"
#include "SkUnPreMultiply.h"
#include "SkColorPriv.h"
#if SK_SUPPORT_GPU
#include "GrClip.h"
#include "GrContext.h"
#include "GrCoordTransform.h"
#include "GrRenderTargetContext.h"
#include "GrTexture.h"
#include "GrTextureProxy.h"
#include "SkGr.h"
#include "effects/GrTextureDomain.h"
#include "glsl/GrGLSLColorSpaceXformHelper.h"
#include "glsl/GrGLSLFragmentProcessor.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"
#include "glsl/GrGLSLProgramDataManager.h"
#include "glsl/GrGLSLUniformHandler.h"
#endif

namespace {

#define kChannelSelectorKeyBits 3; // Max value is 4, so 3 bits are required at most

const uint8_t gChannelTypeToShift[] = {
     0,  // unknown
    SK_R32_SHIFT,
    SK_G32_SHIFT,
    SK_B32_SHIFT,
    SK_A32_SHIFT,
};
struct Extractor {
    Extractor(SkDisplacementMapEffect::ChannelSelectorType typeX,
              SkDisplacementMapEffect::ChannelSelectorType typeY)
        : fShiftX(gChannelTypeToShift[typeX])
        , fShiftY(gChannelTypeToShift[typeY])
    {}

    unsigned fShiftX, fShiftY;

    unsigned getX(SkPMColor c) const { return (c >> fShiftX) & 0xFF; }
    unsigned getY(SkPMColor c) const { return (c >> fShiftY) & 0xFF; }
};

static SkPMColor unpremul_pm(SkPMColor c) {
    const U8CPU a = SkGetPackedA32(c);
    if (0 == a) {
        return 0;
    } else if (0xFF == a) {
        return c;
    }
    const unsigned scale = SkUnPreMultiply::GetScale(a);
    return SkPackARGB32NoCheck(a,
                               SkUnPreMultiply::ApplyScale(scale, SkGetPackedR32(c)),
                               SkUnPreMultiply::ApplyScale(scale, SkGetPackedG32(c)),
                               SkUnPreMultiply::ApplyScale(scale, SkGetPackedB32(c)));
}

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
            SkPMColor c = unpremul_pm(*displPtr);

            SkScalar displX = scaleForColor.fX * ex.getX(c) + scaleAdj.fX;
            SkScalar displY = scaleForColor.fY * ex.getY(c) + scaleAdj.fY;
            // Truncate the displacement values
            const int srcX = x + SkScalarTruncToInt(displX);
            const int srcY = y + SkScalarTruncToInt(displY);
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
    ChannelSelectorType xsel = (ChannelSelectorType)buffer.readInt();
    ChannelSelectorType ysel = (ChannelSelectorType)buffer.readInt();
    SkScalar scale = buffer.readScalar();
    return Make(xsel, ysel, scale,
                common.getInput(0), common.getInput(1),
                &common.cropRect());
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
    static sk_sp<GrFragmentProcessor> Make(
                SkDisplacementMapEffect::ChannelSelectorType xChannelSelector,
                SkDisplacementMapEffect::ChannelSelectorType yChannelSelector, SkVector scale,
                sk_sp<GrTextureProxy> displacement, const SkMatrix& offsetMatrix,
                sk_sp<GrTextureProxy> color,
                sk_sp<GrColorSpaceXform> colorSpaceXform, const SkISize& colorDimensions) {
        return sk_sp<GrFragmentProcessor>(
            new GrDisplacementMapEffect(xChannelSelector, yChannelSelector, scale,
                                        std::move(displacement),
                                        offsetMatrix, std::move(color), std::move(colorSpaceXform),
                                        colorDimensions));
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
    GrColorSpaceXform* colorSpaceXform() const { return fColorSpaceXform.get(); }

private:
    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override;

    void onGetGLSLProcessorKey(const GrShaderCaps& caps, GrProcessorKeyBuilder* b) const override;

    bool onIsEqual(const GrFragmentProcessor&) const override;

    GrDisplacementMapEffect(SkDisplacementMapEffect::ChannelSelectorType xChannelSelector,
                            SkDisplacementMapEffect::ChannelSelectorType yChannelSelector,
                            const SkVector& scale,
                            sk_sp<GrTextureProxy> displacement, const SkMatrix& offsetMatrix,
                            sk_sp<GrTextureProxy> color, sk_sp<GrColorSpaceXform> colorSpaceXform,
                            const SkISize& colorDimensions);

    GR_DECLARE_FRAGMENT_PROCESSOR_TEST

    GrCoordTransform            fDisplacementTransform;
    TextureSampler              fDisplacementSampler;
    GrCoordTransform            fColorTransform;
    GrTextureDomain             fDomain;
    TextureSampler              fColorSampler;
    sk_sp<GrColorSpaceXform>    fColorSpaceXform;
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
    Context displContext(ctx.ctm(), ctx.clipBounds(), ctx.cache(), OutputProperties(nullptr));
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
    displ = this->applyCropRect(ctx, displ.get(), &displOffset, &displBounds);
    if (!displ) {
        return nullptr;
    }

    if (!bounds.intersect(displBounds)) {
        return nullptr;
    }

    const SkIRect colorBounds = bounds.makeOffset(-colorOffset.x(), -colorOffset.y());

    SkVector scale = SkVector::Make(fScale, fScale);
    ctx.ctm().mapVectors(&scale, 1);

#if SK_SUPPORT_GPU
    if (source->isTextureBacked()) {
        GrContext* context = source->getContext();

        sk_sp<GrTextureProxy> colorProxy(color->asTextureProxyRef(context));
        sk_sp<GrTextureProxy> displProxy(displ->asTextureProxyRef(context));
        if (!colorProxy || !displProxy) {
            return nullptr;
        }

        SkMatrix offsetMatrix = SkMatrix::MakeTrans(SkIntToScalar(colorOffset.fX - displOffset.fX),
                                                    SkIntToScalar(colorOffset.fY - displOffset.fY));
        SkColorSpace* colorSpace = ctx.outputProperties().colorSpace();
        sk_sp<GrColorSpaceXform> colorSpaceXform = GrColorSpaceXform::Make(color->getColorSpace(),
                                                                           colorSpace);
        GrPaint paint;
        paint.addColorFragmentProcessor(
            GrDisplacementMapEffect::Make(fXChannelSelector,
                                          fYChannelSelector,
                                          scale,
                                          std::move(displProxy),
                                          offsetMatrix,
                                          std::move(colorProxy),
                                          std::move(colorSpaceXform),
                                          SkISize::Make(color->width(), color->height())));
        paint.setPorterDuffXPFactory(SkBlendMode::kSrc);
        SkMatrix matrix;
        matrix.setTranslate(-SkIntToScalar(colorBounds.x()), -SkIntToScalar(colorBounds.y()));

        sk_sp<GrRenderTargetContext> renderTargetContext(
            context->makeDeferredRenderTargetContext(SkBackingFit::kApprox,
                                                     bounds.width(), bounds.height(),
                                                     GrRenderableConfigForColorSpace(colorSpace),
                                                     sk_ref_sp(colorSpace)));
        if (!renderTargetContext) {
            return nullptr;
        }
        paint.setGammaCorrect(renderTargetContext->isGammaCorrect());

        renderTargetContext->drawRect(GrNoClip(), std::move(paint), GrAA::kNo, matrix,
                                      SkRect::Make(colorBounds));

        offset->fX = bounds.left();
        offset->fY = bounds.top();
        return SkSpecialImage::MakeDeferredFromGpu(
                                            context,
                                            SkIRect::MakeWH(bounds.width(), bounds.height()),
                                            kNeedNewImageUniqueID_SpecialImage,
                                            renderTargetContext->asTextureProxyRef(),
                                            renderTargetContext->refColorSpace());
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

sk_sp<SkImageFilter> SkDisplacementMapEffect::onMakeColorSpace(SkColorSpaceXformer* xformer) const {
    SkASSERT(2 == this->countInputs());
    // Intentionally avoid xforming the displacement filter.  The values will be used as
    // offsets, not as colors.
    sk_sp<SkImageFilter> displacement = sk_ref_sp(const_cast<SkImageFilter*>(this->getInput(0)));
    sk_sp<SkImageFilter> color = xformer->apply(this->getInput(1));

    if (color.get() != this->getInput(1)) {
        return SkDisplacementMapEffect::Make(fXChannelSelector, fYChannelSelector, fScale,
                                             std::move(displacement), std::move(color),
                                             this->getCropRectIfSet());
    }
    return this->refMe();
}

SkRect SkDisplacementMapEffect::computeFastBounds(const SkRect& src) const {
    SkRect bounds = this->getColorInput() ? this->getColorInput()->computeFastBounds(src) : src;
    bounds.outset(SkScalarAbs(fScale) * SK_ScalarHalf, SkScalarAbs(fScale) * SK_ScalarHalf);
    return bounds;
}

SkIRect SkDisplacementMapEffect::onFilterNodeBounds(const SkIRect& src, const SkMatrix& ctm,
                                                    MapDirection) const {
    SkVector scale = SkVector::Make(fScale, fScale);
    ctm.mapVectors(&scale, 1);
    return src.makeOutset(SkScalarCeilToInt(SkScalarAbs(scale.fX) * SK_ScalarHalf),
                          SkScalarCeilToInt(SkScalarAbs(scale.fY) * SK_ScalarHalf));
}

SkIRect SkDisplacementMapEffect::onFilterBounds(const SkIRect& src, const SkMatrix& ctm,
                                                MapDirection direction) const {
    // Recurse only into color input.
    if (this->getColorInput()) {
        return this->getColorInput()->filterBounds(src, ctm, direction);
    }
    return src;
}

#ifndef SK_IGNORE_TO_STRING
void SkDisplacementMapEffect::toString(SkString* str) const {
    str->appendf("SkDisplacementMapEffect: (");
    str->appendf("scale: %f ", fScale);
    str->appendf("displacement: (");
    if (this->getDisplacementInput()) {
        this->getDisplacementInput()->toString(str);
    }
    str->appendf(") color: (");
    if (this->getColorInput()) {
        this->getColorInput()->toString(str);
    }
    str->appendf("))");
}
#endif

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
    GrGLSLColorSpaceXformHelper fColorSpaceHelper;
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
        const SkMatrix& offsetMatrix,
        sk_sp<GrTextureProxy> color,
        sk_sp<GrColorSpaceXform> colorSpaceXform,
        const SkISize& colorDimensions)
        : INHERITED(GrPixelConfigIsOpaque(color->config()) ? kPreservesOpaqueInput_OptimizationFlag
                                                           : kNone_OptimizationFlags)
        , fDisplacementTransform(offsetMatrix, displacement.get())
        , fDisplacementSampler(displacement)
        , fColorTransform(color.get())
        , fDomain(color.get(), GrTextureDomain::MakeTexelDomain(SkIRect::MakeSize(colorDimensions)),
                  GrTextureDomain::kDecal_Mode)
        , fColorSampler(color)
        , fColorSpaceXform(std::move(colorSpaceXform))
        , fXChannelSelector(xChannelSelector)
        , fYChannelSelector(yChannelSelector)
        , fScale(scale) {
    this->initClassID<GrDisplacementMapEffect>();
    this->addCoordTransform(&fDisplacementTransform);
    this->addTextureSampler(&fDisplacementSampler);
    this->addCoordTransform(&fColorTransform);
    this->addTextureSampler(&fColorSampler);
}

GrDisplacementMapEffect::~GrDisplacementMapEffect() {
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
sk_sp<GrFragmentProcessor> GrDisplacementMapEffect::TestCreate(GrProcessorTestData* d) {
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
    auto colorSpaceXform = GrTest::TestColorXform(d->fRandom);
    return GrDisplacementMapEffect::Make(xChannelSelector, yChannelSelector, scale,
                                         std::move(dispProxy), SkMatrix::I(),
                                         std::move(colorProxy), colorSpaceXform,
                                         colorDimensions);
}
#endif

///////////////////////////////////////////////////////////////////////////////

void GrGLDisplacementMapEffect::emitCode(EmitArgs& args) {
    const GrDisplacementMapEffect& displacementMap = args.fFp.cast<GrDisplacementMapEffect>();
    const GrTextureDomain& domain = displacementMap.domain();

    fScaleUni = args.fUniformHandler->addUniform(kFragment_GrShaderFlag,
                                                 kVec2f_GrSLType, kDefault_GrSLPrecision, "Scale");
    const char* scaleUni = args.fUniformHandler->getUniformCStr(fScaleUni);
    const char* dColor = "dColor";
    const char* cCoords = "cCoords";
    const char* nearZero = "1e-6"; // Since 6.10352e-5 is the smallest half float, use
                                   // a number smaller than that to approximate 0, but
                                   // leave room for 32-bit float GPU rounding errors.

    fColorSpaceHelper.emitCode(args.fUniformHandler, displacementMap.colorSpaceXform());

    GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;
    fragBuilder->codeAppendf("\t\tvec4 %s = ", dColor);
    fragBuilder->appendTextureLookup(args.fTexSamplers[0], args.fTransformedCoords[0].c_str(),
                                   args.fTransformedCoords[0].getType());
    fragBuilder->codeAppend(";\n");

    // Unpremultiply the displacement
    fragBuilder->codeAppendf(
        "\t\t%s.rgb = (%s.a < %s) ? vec3(0.0) : clamp(%s.rgb / %s.a, 0.0, 1.0);",
        dColor, dColor, nearZero, dColor, dColor);
    SkString coords2D = fragBuilder->ensureCoords2D(args.fTransformedCoords[1]);
    fragBuilder->codeAppendf("\t\tvec2 %s = %s + %s*(%s.",
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
    fragBuilder->codeAppend("-vec2(0.5));\t\t");

    fGLDomain.sampleTexture(fragBuilder,
                            args.fUniformHandler,
                            args.fShaderCaps,
                            domain,
                            args.fOutputColor,
                            SkString(cCoords),
                            args.fTexSamplers[1],
                            nullptr,
                            &fColorSpaceHelper);
    fragBuilder->codeAppend(";\n");
}

void GrGLDisplacementMapEffect::onSetData(const GrGLSLProgramDataManager& pdman,
                                          const GrFragmentProcessor& proc) {
    const GrDisplacementMapEffect& displacementMap = proc.cast<GrDisplacementMapEffect>();
    GrTexture* colorTex = displacementMap.textureSampler(1).peekTexture();

    SkScalar scaleX = displacementMap.scale().fX / colorTex->width();
    SkScalar scaleY = displacementMap.scale().fY / colorTex->height();
    pdman.set2f(fScaleUni, SkScalarToFloat(scaleX),
                colorTex->origin() == kTopLeft_GrSurfaceOrigin ?
                SkScalarToFloat(scaleY) : SkScalarToFloat(-scaleY));
    fGLDomain.setData(pdman, displacementMap.domain(), colorTex);
    if (SkToBool(displacementMap.colorSpaceXform())) {
        fColorSpaceHelper.setData(pdman, displacementMap.colorSpaceXform());
    }
}

void GrGLDisplacementMapEffect::GenKey(const GrProcessor& proc,
                                       const GrShaderCaps&, GrProcessorKeyBuilder* b) {
    const GrDisplacementMapEffect& displacementMap = proc.cast<GrDisplacementMapEffect>();

    uint32_t xKey = displacementMap.xChannelSelector();
    uint32_t yKey = displacementMap.yChannelSelector() << kChannelSelectorKeyBits;

    b->add32(xKey | yKey);
    b->add32(GrColorSpaceXform::XformKey(displacementMap.colorSpaceXform()));
}
#endif
