/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkDisplacementMapEffect.h"
#include "SkDevice.h"
#include "SkReadBuffer.h"
#include "SkWriteBuffer.h"
#include "SkUnPreMultiply.h"
#include "SkColorPriv.h"
#if SK_SUPPORT_GPU
#include "GrContext.h"
#include "GrDrawContext.h"
#include "GrCoordTransform.h"
#include "GrInvariantOutput.h"
#include "SkGr.h"
#include "effects/GrTextureDomain.h"
#include "glsl/GrGLSLFragmentProcessor.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"
#include "glsl/GrGLSLProgramDataManager.h"
#include "glsl/GrGLSLUniformHandler.h"
#endif

namespace {

#define kChannelSelectorKeyBits 3; // Max value is 4, so 3 bits are required at most

template<SkDisplacementMapEffect::ChannelSelectorType type>
uint32_t getValue(SkColor, const SkUnPreMultiply::Scale*) {
    SkDEBUGFAIL("Unknown channel selector");
    return 0;
}

template<> uint32_t getValue<SkDisplacementMapEffect::kR_ChannelSelectorType>(
    SkColor l, const SkUnPreMultiply::Scale* table) {
    return SkUnPreMultiply::ApplyScale(table[SkGetPackedA32(l)], SkGetPackedR32(l));
}

template<> uint32_t getValue<SkDisplacementMapEffect::kG_ChannelSelectorType>(
    SkColor l, const SkUnPreMultiply::Scale* table) {
    return SkUnPreMultiply::ApplyScale(table[SkGetPackedA32(l)], SkGetPackedG32(l));
}

template<> uint32_t getValue<SkDisplacementMapEffect::kB_ChannelSelectorType>(
    SkColor l, const SkUnPreMultiply::Scale* table) {
    return SkUnPreMultiply::ApplyScale(table[SkGetPackedA32(l)], SkGetPackedB32(l));
}

template<> uint32_t getValue<SkDisplacementMapEffect::kA_ChannelSelectorType>(
    SkColor l, const SkUnPreMultiply::Scale*) {
    return SkGetPackedA32(l);
}

template<SkDisplacementMapEffect::ChannelSelectorType typeX,
         SkDisplacementMapEffect::ChannelSelectorType typeY>
void computeDisplacement(const SkVector& scale, SkBitmap* dst,
                         SkBitmap* displ, const SkIPoint& offset,
                         SkBitmap* src,
                         const SkIRect& bounds)
{
    static const SkScalar Inv8bit = SkScalarInvert(255);
    const int srcW = src->width();
    const int srcH = src->height();
    const SkVector scaleForColor = SkVector::Make(SkScalarMul(scale.fX, Inv8bit),
                                                  SkScalarMul(scale.fY, Inv8bit));
    const SkVector scaleAdj = SkVector::Make(SK_ScalarHalf - SkScalarMul(scale.fX, SK_ScalarHalf),
                                             SK_ScalarHalf - SkScalarMul(scale.fY, SK_ScalarHalf));
    const SkUnPreMultiply::Scale* table = SkUnPreMultiply::GetScaleTable();
    SkPMColor* dstPtr = dst->getAddr32(0, 0);
    for (int y = bounds.top(); y < bounds.bottom(); ++y) {
        const SkPMColor* displPtr = displ->getAddr32(bounds.left() + offset.fX,
                                                     y + offset.fY);
        for (int x = bounds.left(); x < bounds.right(); ++x, ++displPtr) {
            const SkScalar displX = SkScalarMul(scaleForColor.fX,
                SkIntToScalar(getValue<typeX>(*displPtr, table))) + scaleAdj.fX;
            const SkScalar displY = SkScalarMul(scaleForColor.fY,
                SkIntToScalar(getValue<typeY>(*displPtr, table))) + scaleAdj.fY;
            // Truncate the displacement values
            const int srcX = x + SkScalarTruncToInt(displX);
            const int srcY = y + SkScalarTruncToInt(displY);
            *dstPtr++ = ((srcX < 0) || (srcX >= srcW) || (srcY < 0) || (srcY >= srcH)) ?
                      0 : *(src->getAddr32(srcX, srcY));
        }
    }
}

template<SkDisplacementMapEffect::ChannelSelectorType typeX>
void computeDisplacement(SkDisplacementMapEffect::ChannelSelectorType yChannelSelector,
                         const SkVector& scale, SkBitmap* dst,
                         SkBitmap* displ, const SkIPoint& offset,
                         SkBitmap* src,
                         const SkIRect& bounds)
{
    switch (yChannelSelector) {
      case SkDisplacementMapEffect::kR_ChannelSelectorType:
        computeDisplacement<typeX, SkDisplacementMapEffect::kR_ChannelSelectorType>(
            scale, dst, displ, offset, src, bounds);
        break;
      case SkDisplacementMapEffect::kG_ChannelSelectorType:
        computeDisplacement<typeX, SkDisplacementMapEffect::kG_ChannelSelectorType>(
            scale, dst, displ, offset, src, bounds);
        break;
      case SkDisplacementMapEffect::kB_ChannelSelectorType:
        computeDisplacement<typeX, SkDisplacementMapEffect::kB_ChannelSelectorType>(
            scale, dst, displ, offset, src, bounds);
        break;
      case SkDisplacementMapEffect::kA_ChannelSelectorType:
        computeDisplacement<typeX, SkDisplacementMapEffect::kA_ChannelSelectorType>(
            scale, dst, displ, offset, src, bounds);
        break;
      case SkDisplacementMapEffect::kUnknown_ChannelSelectorType:
      default:
        SkDEBUGFAIL("Unknown Y channel selector");
    }
}

void computeDisplacement(SkDisplacementMapEffect::ChannelSelectorType xChannelSelector,
                         SkDisplacementMapEffect::ChannelSelectorType yChannelSelector,
                         const SkVector& scale, SkBitmap* dst,
                         SkBitmap* displ, const SkIPoint& offset,
                         SkBitmap* src,
                         const SkIRect& bounds)
{
    switch (xChannelSelector) {
      case SkDisplacementMapEffect::kR_ChannelSelectorType:
        computeDisplacement<SkDisplacementMapEffect::kR_ChannelSelectorType>(
            yChannelSelector, scale, dst, displ, offset, src, bounds);
        break;
      case SkDisplacementMapEffect::kG_ChannelSelectorType:
        computeDisplacement<SkDisplacementMapEffect::kG_ChannelSelectorType>(
            yChannelSelector, scale, dst, displ, offset, src, bounds);
        break;
      case SkDisplacementMapEffect::kB_ChannelSelectorType:
        computeDisplacement<SkDisplacementMapEffect::kB_ChannelSelectorType>(
            yChannelSelector, scale, dst, displ, offset, src, bounds);
        break;
      case SkDisplacementMapEffect::kA_ChannelSelectorType:
        computeDisplacement<SkDisplacementMapEffect::kA_ChannelSelectorType>(
            yChannelSelector, scale, dst, displ, offset, src, bounds);
        break;
      case SkDisplacementMapEffect::kUnknown_ChannelSelectorType:
      default:
        SkDEBUGFAIL("Unknown X channel selector");
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

SkImageFilter* SkDisplacementMapEffect::Create(ChannelSelectorType xChannelSelector,
                                               ChannelSelectorType yChannelSelector,
                                               SkScalar scale,
                                               SkImageFilter* displacement,
                                               SkImageFilter* color,
                                               const CropRect* cropRect) {
    if (!channel_selector_type_is_valid(xChannelSelector) ||
        !channel_selector_type_is_valid(yChannelSelector)) {
        return nullptr;
    }

    SkImageFilter* inputs[2] = { displacement, color };
    return new SkDisplacementMapEffect(xChannelSelector, yChannelSelector, scale, inputs, cropRect);
}

SkDisplacementMapEffect::SkDisplacementMapEffect(ChannelSelectorType xChannelSelector,
                                                 ChannelSelectorType yChannelSelector,
                                                 SkScalar scale,
                                                 SkImageFilter* inputs[2],
                                                 const CropRect* cropRect)
  : INHERITED(2, inputs, cropRect)
  , fXChannelSelector(xChannelSelector)
  , fYChannelSelector(yChannelSelector)
  , fScale(scale)
{
}

SkDisplacementMapEffect::~SkDisplacementMapEffect() {
}

sk_sp<SkFlattenable> SkDisplacementMapEffect::CreateProc(SkReadBuffer& buffer) {
    SK_IMAGEFILTER_UNFLATTEN_COMMON(common, 2);
    ChannelSelectorType xsel = (ChannelSelectorType)buffer.readInt();
    ChannelSelectorType ysel = (ChannelSelectorType)buffer.readInt();
    SkScalar scale = buffer.readScalar();
    return sk_sp<SkFlattenable>(Create(xsel, ysel, scale, common.getInput(0).get(),
                                       common.getInput(1).get(), &common.cropRect()));
}

void SkDisplacementMapEffect::flatten(SkWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    buffer.writeInt((int) fXChannelSelector);
    buffer.writeInt((int) fYChannelSelector);
    buffer.writeScalar(fScale);
}

bool SkDisplacementMapEffect::onFilterImageDeprecated(Proxy* proxy,
                                                      const SkBitmap& src,
                                                      const Context& ctx,
                                                      SkBitmap* dst,
                                                      SkIPoint* offset) const {
    SkBitmap displ = src, color = src;
    SkIPoint colorOffset = SkIPoint::Make(0, 0), displOffset = SkIPoint::Make(0, 0);
    if (!this->filterInputDeprecated(1, proxy, src, ctx, &color, &colorOffset) ||
        !this->filterInputDeprecated(0, proxy, src, ctx, &displ, &displOffset)) {
        return false;
    }
    if ((displ.colorType() != kN32_SkColorType) ||
        (color.colorType() != kN32_SkColorType)) {
        return false;
    }
    SkIRect bounds;
    // Since computeDisplacement does bounds checking on color pixel access, we don't need to pad
    // the color bitmap to bounds here.
    SkIRect srcBounds = color.bounds();
    srcBounds.offset(colorOffset);
    if (!this->applyCropRect(ctx, srcBounds, &bounds)) {
        return false;
    }
    SkIRect displBounds;
    if (!this->applyCropRectDeprecated(ctx, proxy, displ, &displOffset, &displBounds, &displ)) {
        return false;
    }
    if (!bounds.intersect(displBounds)) {
        return false;
    }
    SkAutoLockPixels alp_displacement(displ), alp_color(color);
    if (!displ.getPixels() || !color.getPixels()) {
        return false;
    }

    SkAutoTUnref<SkBaseDevice> device(proxy->createDevice(bounds.width(), bounds.height()));
    if (!device) {
        return false;
    }
    *dst = device->accessBitmap(false);
    SkAutoLockPixels alp_dst(*dst);

    SkVector scale = SkVector::Make(fScale, fScale);
    ctx.ctm().mapVectors(&scale, 1);
    SkIRect colorBounds = bounds;
    colorBounds.offset(-colorOffset);

    computeDisplacement(fXChannelSelector, fYChannelSelector, scale, dst,
                        &displ, colorOffset - displOffset, &color, colorBounds);

    offset->fX = bounds.left();
    offset->fY = bounds.top();
    return true;
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

    static inline void GenKey(const GrProcessor&, const GrGLSLCaps&, GrProcessorKeyBuilder*);

protected:
    void onSetData(const GrGLSLProgramDataManager&, const GrProcessor&) override;

private:
    GrGLSLProgramDataManager::UniformHandle fScaleUni;
    GrTextureDomain::GLDomain fGLDomain;

    typedef GrGLSLFragmentProcessor INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

class GrDisplacementMapEffect : public GrFragmentProcessor {
public:
    static GrFragmentProcessor* Create(
            SkDisplacementMapEffect::ChannelSelectorType xChannelSelector,
            SkDisplacementMapEffect::ChannelSelectorType yChannelSelector, SkVector scale,
            GrTexture* displacement, const SkMatrix& offsetMatrix, GrTexture* color,
            const SkISize& colorDimensions) {
        return new GrDisplacementMapEffect(xChannelSelector, yChannelSelector, scale, displacement,
                                           offsetMatrix, color, colorDimensions);
    }

    virtual ~GrDisplacementMapEffect();

    SkDisplacementMapEffect::ChannelSelectorType xChannelSelector() const
        { return fXChannelSelector; }
    SkDisplacementMapEffect::ChannelSelectorType yChannelSelector() const
        { return fYChannelSelector; }
    const SkVector& scale() const { return fScale; }

    const char* name() const override { return "DisplacementMap"; }
    const GrTextureDomain& domain() const { return fDomain; }

private:
    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override {
        return new GrGLDisplacementMapEffect;
    }

    void onGetGLSLProcessorKey(const GrGLSLCaps& caps, GrProcessorKeyBuilder* b) const override {
        GrGLDisplacementMapEffect::GenKey(*this, caps, b);
    }

    bool onIsEqual(const GrFragmentProcessor&) const override;

    void onComputeInvariantOutput(GrInvariantOutput* inout) const override;

    GrDisplacementMapEffect(SkDisplacementMapEffect::ChannelSelectorType xChannelSelector,
                            SkDisplacementMapEffect::ChannelSelectorType yChannelSelector,
                            const SkVector& scale,
                            GrTexture* displacement, const SkMatrix& offsetMatrix,
                            GrTexture* color,
                            const SkISize& colorDimensions);

    GR_DECLARE_FRAGMENT_PROCESSOR_TEST;

    GrCoordTransform            fDisplacementTransform;
    GrTextureAccess             fDisplacementAccess;
    GrCoordTransform            fColorTransform;
    GrTextureDomain             fDomain;
    GrTextureAccess             fColorAccess;
    SkDisplacementMapEffect::ChannelSelectorType fXChannelSelector;
    SkDisplacementMapEffect::ChannelSelectorType fYChannelSelector;
    SkVector fScale;

    typedef GrFragmentProcessor INHERITED;
};

bool SkDisplacementMapEffect::filterImageGPUDeprecated(Proxy* proxy, const SkBitmap& src,
                                                       const Context& ctx,
                                                       SkBitmap* result, SkIPoint* offset) const {
    SkBitmap colorBM = src;
    SkIPoint colorOffset = SkIPoint::Make(0, 0);
    if (!this->filterInputGPUDeprecated(1, proxy, src, ctx, &colorBM, &colorOffset)) {
        return false;
    }
    SkBitmap displacementBM = src;
    SkIPoint displacementOffset = SkIPoint::Make(0, 0);
    if (!this->filterInputGPUDeprecated(0, proxy, src, ctx, &displacementBM, &displacementOffset)) {
        return false;
    }
    SkIRect srcBounds = colorBM.bounds();
    srcBounds.offset(colorOffset);
    SkIRect bounds;
    // Since GrDisplacementMapEffect does bounds checking on color pixel access, we don't need to
    // pad the color bitmap to bounds here.
    if (!this->applyCropRect(ctx, srcBounds, &bounds)) {
        return false;
    }
    SkIRect displBounds;
    if (!this->applyCropRectDeprecated(ctx, proxy, displacementBM,
                                       &displacementOffset, &displBounds, &displacementBM)) {
        return false;
    }
    if (!bounds.intersect(displBounds)) {
        return false;
    }
    GrTexture* color = colorBM.getTexture();
    GrTexture* displacement = displacementBM.getTexture();
    GrContext* context = color->getContext();

    GrSurfaceDesc desc;
    desc.fFlags = kRenderTarget_GrSurfaceFlag;
    desc.fWidth = bounds.width();
    desc.fHeight = bounds.height();
    desc.fConfig = kSkia8888_GrPixelConfig;

    SkAutoTUnref<GrTexture> dst(context->textureProvider()->createApproxTexture(desc));

    if (!dst) {
        return false;
    }

    SkVector scale = SkVector::Make(fScale, fScale);
    ctx.ctm().mapVectors(&scale, 1);

    GrPaint paint;
    // SRGBTODO: AllowSRGBInputs?
    SkMatrix offsetMatrix = GrCoordTransform::MakeDivByTextureWHMatrix(displacement);
    offsetMatrix.preTranslate(SkIntToScalar(colorOffset.fX - displacementOffset.fX),
                              SkIntToScalar(colorOffset.fY - displacementOffset.fY));

    paint.addColorFragmentProcessor(
        GrDisplacementMapEffect::Create(fXChannelSelector,
                                        fYChannelSelector,
                                        scale,
                                        displacement,
                                        offsetMatrix,
                                        color,
                                        colorBM.dimensions()))->unref();
    paint.setPorterDuffXPFactory(SkXfermode::kSrc_Mode);
    SkIRect colorBounds = bounds;
    colorBounds.offset(-colorOffset);
    SkMatrix matrix;
    matrix.setTranslate(-SkIntToScalar(colorBounds.x()),
                        -SkIntToScalar(colorBounds.y()));

    SkAutoTUnref<GrDrawContext> drawContext(context->drawContext(dst->asRenderTarget()));
    if (!drawContext) {
        return false;
    }

    drawContext->drawRect(GrClip::WideOpen(), paint, matrix, SkRect::Make(colorBounds));
    offset->fX = bounds.left();
    offset->fY = bounds.top();
    GrWrapTextureInBitmap(dst, bounds.width(), bounds.height(), false, result);
    return true;
}

///////////////////////////////////////////////////////////////////////////////

GrDisplacementMapEffect::GrDisplacementMapEffect(
                             SkDisplacementMapEffect::ChannelSelectorType xChannelSelector,
                             SkDisplacementMapEffect::ChannelSelectorType yChannelSelector,
                             const SkVector& scale,
                             GrTexture* displacement,
                             const SkMatrix& offsetMatrix,
                             GrTexture* color,
                             const SkISize& colorDimensions)
    : fDisplacementTransform(kLocal_GrCoordSet, offsetMatrix, displacement,
                             GrTextureParams::kNone_FilterMode)
    , fDisplacementAccess(displacement)
    , fColorTransform(kLocal_GrCoordSet, color, GrTextureParams::kNone_FilterMode)
    , fDomain(GrTextureDomain::MakeTexelDomain(color, SkIRect::MakeSize(colorDimensions)),
              GrTextureDomain::kDecal_Mode)
    , fColorAccess(color)
    , fXChannelSelector(xChannelSelector)
    , fYChannelSelector(yChannelSelector)
    , fScale(scale) {
    this->initClassID<GrDisplacementMapEffect>();
    this->addCoordTransform(&fDisplacementTransform);
    this->addTextureAccess(&fDisplacementAccess);
    this->addCoordTransform(&fColorTransform);
    this->addTextureAccess(&fColorAccess);
}

GrDisplacementMapEffect::~GrDisplacementMapEffect() {
}

bool GrDisplacementMapEffect::onIsEqual(const GrFragmentProcessor& sBase) const {
    const GrDisplacementMapEffect& s = sBase.cast<GrDisplacementMapEffect>();
    return fXChannelSelector == s.fXChannelSelector &&
           fYChannelSelector == s.fYChannelSelector &&
           fScale == s.fScale;
}

void GrDisplacementMapEffect::onComputeInvariantOutput(GrInvariantOutput* inout) const {
    // Any displacement offset bringing a pixel out of bounds will output a color of (0,0,0,0),
    // so the only way we'd get a constant alpha is if the input color image has a constant alpha
    // and no displacement offset push any texture coordinates out of bounds OR if the constant
    // alpha is 0. Since this isn't trivial to compute at this point, let's assume the output is
    // not of constant color when a displacement effect is applied.
    inout->setToUnknown(GrInvariantOutput::kWillNot_ReadInput);
}

///////////////////////////////////////////////////////////////////////////////

GR_DEFINE_FRAGMENT_PROCESSOR_TEST(GrDisplacementMapEffect);

const GrFragmentProcessor* GrDisplacementMapEffect::TestCreate(GrProcessorTestData* d) {
    int texIdxDispl = d->fRandom->nextBool() ? GrProcessorUnitTest::kSkiaPMTextureIdx :
                                               GrProcessorUnitTest::kAlphaTextureIdx;
    int texIdxColor = d->fRandom->nextBool() ? GrProcessorUnitTest::kSkiaPMTextureIdx :
                                               GrProcessorUnitTest::kAlphaTextureIdx;
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
    colorDimensions.fWidth = d->fRandom->nextRangeU(0, d->fTextures[texIdxColor]->width());
    colorDimensions.fHeight = d->fRandom->nextRangeU(0, d->fTextures[texIdxColor]->height());
    return GrDisplacementMapEffect::Create(xChannelSelector, yChannelSelector, scale,
                                           d->fTextures[texIdxDispl], SkMatrix::I(),
                                           d->fTextures[texIdxColor], colorDimensions);
}

///////////////////////////////////////////////////////////////////////////////

void GrGLDisplacementMapEffect::emitCode(EmitArgs& args) {
    const GrDisplacementMapEffect& displacementMap = args.fFp.cast<GrDisplacementMapEffect>();
    const GrTextureDomain& domain = displacementMap.domain();

    fScaleUni = args.fUniformHandler->addUniform(kFragment_GrShaderFlag,
                                                 kVec2f_GrSLType, kDefault_GrSLPrecision, "Scale");
    const char* scaleUni = args.fUniformHandler->getUniformCStr(fScaleUni);
    const char* dColor = "dColor";
    const char* cCoords = "cCoords";
    const char* nearZero = "1e-6"; // Since 6.10352e−5 is the smallest half float, use
                                   // a number smaller than that to approximate 0, but
                                   // leave room for 32-bit float GPU rounding errors.

    GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;
    fragBuilder->codeAppendf("\t\tvec4 %s = ", dColor);
    fragBuilder->appendTextureLookup(args.fSamplers[0], args.fCoords[0].c_str(),
                                   args.fCoords[0].getType());
    fragBuilder->codeAppend(";\n");

    // Unpremultiply the displacement
    fragBuilder->codeAppendf(
        "\t\t%s.rgb = (%s.a < %s) ? vec3(0.0) : clamp(%s.rgb / %s.a, 0.0, 1.0);",
        dColor, dColor, nearZero, dColor, dColor);
    SkString coords2D = fragBuilder->ensureFSCoords2D(args.fCoords, 1);
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
                            args.fGLSLCaps,
                            domain,
                            args.fOutputColor,
                            SkString(cCoords),
                            args.fSamplers[1]);
    fragBuilder->codeAppend(";\n");
}

void GrGLDisplacementMapEffect::onSetData(const GrGLSLProgramDataManager& pdman,
                                          const GrProcessor& proc) {
    const GrDisplacementMapEffect& displacementMap = proc.cast<GrDisplacementMapEffect>();
    GrTexture* colorTex = displacementMap.texture(1);
    SkScalar scaleX = displacementMap.scale().fX / colorTex->width();
    SkScalar scaleY = displacementMap.scale().fY / colorTex->height();
    pdman.set2f(fScaleUni, SkScalarToFloat(scaleX),
                colorTex->origin() == kTopLeft_GrSurfaceOrigin ?
                SkScalarToFloat(scaleY) : SkScalarToFloat(-scaleY));
    fGLDomain.setData(pdman, displacementMap.domain(), colorTex->origin());
}

void GrGLDisplacementMapEffect::GenKey(const GrProcessor& proc,
                                       const GrGLSLCaps&, GrProcessorKeyBuilder* b) {
    const GrDisplacementMapEffect& displacementMap = proc.cast<GrDisplacementMapEffect>();

    uint32_t xKey = displacementMap.xChannelSelector();
    uint32_t yKey = displacementMap.yChannelSelector() << kChannelSelectorKeyBits;

    b->add32(xKey | yKey);
}
#endif
