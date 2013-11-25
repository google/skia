/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkDisplacementMapEffect.h"
#include "SkFlattenableBuffers.h"
#include "SkUnPreMultiply.h"
#include "SkColorPriv.h"
#if SK_SUPPORT_GPU
#include "GrContext.h"
#include "GrCoordTransform.h"
#include "gl/GrGLEffect.h"
#include "GrTBackendEffectFactory.h"
#include "SkImageFilterUtils.h"
#endif

namespace {

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
void computeDisplacement(SkScalar scale, SkBitmap* dst, SkBitmap* displ, SkBitmap* src, const SkIRect& bounds)
{
    static const SkScalar Inv8bit = SkScalarDiv(SK_Scalar1, 255.0f);
    const int srcW = src->width();
    const int srcH = src->height();
    const SkScalar scaleForColor = SkScalarMul(scale, Inv8bit);
    const SkScalar scaleAdj = SK_ScalarHalf - SkScalarMul(scale, SK_ScalarHalf);
    const SkUnPreMultiply::Scale* table = SkUnPreMultiply::GetScaleTable();
    SkPMColor* dstPtr = dst->getAddr32(0, 0);
    for (int y = bounds.top(); y < bounds.bottom(); ++y) {
        const SkPMColor* displPtr = displ->getAddr32(bounds.left(), y);
        for (int x = bounds.left(); x < bounds.right(); ++x, ++displPtr) {
            const SkScalar displX = SkScalarMul(scaleForColor,
                SkIntToScalar(getValue<typeX>(*displPtr, table))) + scaleAdj;
            const SkScalar displY = SkScalarMul(scaleForColor,
                SkIntToScalar(getValue<typeY>(*displPtr, table))) + scaleAdj;
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
                         SkScalar scale, SkBitmap* dst, SkBitmap* displ, SkBitmap* src, const SkIRect& bounds)
{
    switch (yChannelSelector) {
      case SkDisplacementMapEffect::kR_ChannelSelectorType:
        computeDisplacement<typeX, SkDisplacementMapEffect::kR_ChannelSelectorType>(
            scale, dst, displ, src, bounds);
        break;
      case SkDisplacementMapEffect::kG_ChannelSelectorType:
        computeDisplacement<typeX, SkDisplacementMapEffect::kG_ChannelSelectorType>(
            scale, dst, displ, src, bounds);
        break;
      case SkDisplacementMapEffect::kB_ChannelSelectorType:
        computeDisplacement<typeX, SkDisplacementMapEffect::kB_ChannelSelectorType>(
            scale, dst, displ, src, bounds);
        break;
      case SkDisplacementMapEffect::kA_ChannelSelectorType:
        computeDisplacement<typeX, SkDisplacementMapEffect::kA_ChannelSelectorType>(
            scale, dst, displ, src, bounds);
        break;
      case SkDisplacementMapEffect::kUnknown_ChannelSelectorType:
      default:
        SkDEBUGFAIL("Unknown Y channel selector");
    }
}

void computeDisplacement(SkDisplacementMapEffect::ChannelSelectorType xChannelSelector,
                         SkDisplacementMapEffect::ChannelSelectorType yChannelSelector,
                         SkScalar scale, SkBitmap* dst, SkBitmap* displ, SkBitmap* src, const SkIRect& bounds)
{
    switch (xChannelSelector) {
      case SkDisplacementMapEffect::kR_ChannelSelectorType:
        computeDisplacement<SkDisplacementMapEffect::kR_ChannelSelectorType>(
            yChannelSelector, scale, dst, displ, src, bounds);
        break;
      case SkDisplacementMapEffect::kG_ChannelSelectorType:
        computeDisplacement<SkDisplacementMapEffect::kG_ChannelSelectorType>(
            yChannelSelector, scale, dst, displ, src, bounds);
        break;
      case SkDisplacementMapEffect::kB_ChannelSelectorType:
        computeDisplacement<SkDisplacementMapEffect::kB_ChannelSelectorType>(
            yChannelSelector, scale, dst, displ, src, bounds);
        break;
      case SkDisplacementMapEffect::kA_ChannelSelectorType:
        computeDisplacement<SkDisplacementMapEffect::kA_ChannelSelectorType>(
            yChannelSelector, scale, dst, displ, src, bounds);
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

SkDisplacementMapEffect::SkDisplacementMapEffect(ChannelSelectorType xChannelSelector,
                                                 ChannelSelectorType yChannelSelector,
                                                 SkScalar scale,
                                                 SkImageFilter* displacement,
                                                 SkImageFilter* color,
                                                 const CropRect* cropRect)
  : INHERITED(displacement, color, cropRect)
  , fXChannelSelector(xChannelSelector)
  , fYChannelSelector(yChannelSelector)
  , fScale(scale)
{
}

SkDisplacementMapEffect::~SkDisplacementMapEffect() {
}

SkDisplacementMapEffect::SkDisplacementMapEffect(SkFlattenableReadBuffer& buffer)
  : INHERITED(2, buffer)
{
    fXChannelSelector = (SkDisplacementMapEffect::ChannelSelectorType) buffer.readInt();
    fYChannelSelector = (SkDisplacementMapEffect::ChannelSelectorType) buffer.readInt();
    fScale            = buffer.readScalar();
    buffer.validate(channel_selector_type_is_valid(fXChannelSelector) &&
                    channel_selector_type_is_valid(fYChannelSelector) &&
                    SkScalarIsFinite(fScale));
}

void SkDisplacementMapEffect::flatten(SkFlattenableWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    buffer.writeInt((int) fXChannelSelector);
    buffer.writeInt((int) fYChannelSelector);
    buffer.writeScalar(fScale);
}

bool SkDisplacementMapEffect::onFilterImage(Proxy* proxy,
                                            const SkBitmap& src,
                                            const SkMatrix& ctm,
                                            SkBitmap* dst,
                                            SkIPoint* offset) {
    SkBitmap displ, color = src;
    SkImageFilter* colorInput = getColorInput();
    SkImageFilter* displacementInput = getDisplacementInput();
    SkASSERT(NULL != displacementInput);
    if ((colorInput && !colorInput->filterImage(proxy, src, ctm, &color, offset)) ||
        !displacementInput->filterImage(proxy, src, ctm, &displ, offset)) {
        return false;
    }
    if ((displ.config() != SkBitmap::kARGB_8888_Config) ||
        (color.config() != SkBitmap::kARGB_8888_Config)) {
        return false;
    }

    SkAutoLockPixels alp_displacement(displ), alp_color(color);
    if (!displ.getPixels() || !color.getPixels()) {
        return false;
    }
    SkIRect bounds;
    color.getBounds(&bounds);
    if (!this->applyCropRect(&bounds, ctm)) {
        return false;
    }

    dst->setConfig(color.config(), bounds.width(), bounds.height());
    dst->allocPixels();
    if (!dst->getPixels()) {
        return false;
    }

    computeDisplacement(fXChannelSelector, fYChannelSelector, fScale, dst, &displ, &color, bounds);

    offset->fX += bounds.left();
    offset->fY += bounds.top();
    return true;
}

///////////////////////////////////////////////////////////////////////////////

#if SK_SUPPORT_GPU
class GrGLDisplacementMapEffect : public GrGLEffect {
public:
    GrGLDisplacementMapEffect(const GrBackendEffectFactory& factory,
                              const GrDrawEffect& drawEffect);
    virtual ~GrGLDisplacementMapEffect();

    virtual void emitCode(GrGLShaderBuilder*,
                          const GrDrawEffect&,
                          EffectKey,
                          const char* outputColor,
                          const char* inputColor,
                          const TransformedCoordsArray&,
                          const TextureSamplerArray&) SK_OVERRIDE;

    static inline EffectKey GenKey(const GrDrawEffect&, const GrGLCaps&);

    virtual void setData(const GrGLUniformManager&, const GrDrawEffect&) SK_OVERRIDE;

private:
    SkDisplacementMapEffect::ChannelSelectorType fXChannelSelector;
    SkDisplacementMapEffect::ChannelSelectorType fYChannelSelector;
    GrGLUniformManager::UniformHandle fScaleUni;

    typedef GrGLEffect INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

class GrDisplacementMapEffect : public GrEffect {
public:
    static GrEffectRef* Create(SkDisplacementMapEffect::ChannelSelectorType xChannelSelector,
                               SkDisplacementMapEffect::ChannelSelectorType yChannelSelector,
                               SkScalar scale, GrTexture* displacement, GrTexture* color) {
        AutoEffectUnref effect(SkNEW_ARGS(GrDisplacementMapEffect, (xChannelSelector,
                                                                    yChannelSelector,
                                                                    scale,
                                                                    displacement,
                                                                    color)));
        return CreateEffectRef(effect);
    }

    virtual ~GrDisplacementMapEffect();

    virtual const GrBackendEffectFactory& getFactory() const SK_OVERRIDE;
    SkDisplacementMapEffect::ChannelSelectorType xChannelSelector() const
        { return fXChannelSelector; }
    SkDisplacementMapEffect::ChannelSelectorType yChannelSelector() const
        { return fYChannelSelector; }
    SkScalar scale() const { return fScale; }

    typedef GrGLDisplacementMapEffect GLEffect;
    static const char* Name() { return "DisplacementMap"; }

    virtual void getConstantColorComponents(GrColor* color, uint32_t* validFlags) const SK_OVERRIDE;

private:
    virtual bool onIsEqual(const GrEffect&) const SK_OVERRIDE;

    GrDisplacementMapEffect(SkDisplacementMapEffect::ChannelSelectorType xChannelSelector,
                            SkDisplacementMapEffect::ChannelSelectorType yChannelSelector,
                            SkScalar scale, GrTexture* displacement, GrTexture* color);

    GR_DECLARE_EFFECT_TEST;

    GrCoordTransform            fDisplacementTransform;
    GrTextureAccess             fDisplacementAccess;
    GrCoordTransform            fColorTransform;
    GrTextureAccess             fColorAccess;
    SkDisplacementMapEffect::ChannelSelectorType fXChannelSelector;
    SkDisplacementMapEffect::ChannelSelectorType fYChannelSelector;
    SkScalar fScale;

    typedef GrEffect INHERITED;
};

bool SkDisplacementMapEffect::filterImageGPU(Proxy* proxy, const SkBitmap& src, const SkMatrix& ctm,
                                             SkBitmap* result, SkIPoint* offset) {
    SkBitmap colorBM;
    SkIPoint colorOffset = SkIPoint::Make(0, 0);
    if (!SkImageFilterUtils::GetInputResultGPU(getColorInput(), proxy, src, ctm, &colorBM,
                                               &colorOffset)) {
        return false;
    }
    GrTexture* color = colorBM.getTexture();
    SkBitmap displacementBM;
    SkIPoint displacementOffset = SkIPoint::Make(0, 0);
    if (!SkImageFilterUtils::GetInputResultGPU(getDisplacementInput(), proxy, src, ctm,
                                               &displacementBM, &displacementOffset)) {
        return false;
    }
    GrTexture* displacement = displacementBM.getTexture();
    GrContext* context = color->getContext();

    GrTextureDesc desc;
    desc.fFlags = kRenderTarget_GrTextureFlagBit | kNoStencil_GrTextureFlagBit;
    desc.fWidth = src.width();
    desc.fHeight = src.height();
    desc.fConfig = kSkia8888_GrPixelConfig;

    GrAutoScratchTexture ast(context, desc);
    SkAutoTUnref<GrTexture> dst(ast.detach());

    GrContext::AutoRenderTarget art(context, dst->asRenderTarget());

    GrPaint paint;
    paint.addColorEffect(
        GrDisplacementMapEffect::Create(fXChannelSelector,
                                        fYChannelSelector,
                                        fScale,
                                        displacement,
                                        color))->unref();
    SkIRect bounds;
    src.getBounds(&bounds);
    if (!this->applyCropRect(&bounds, ctm)) {
        return false;
    }
    SkRect srcRect = SkRect::Make(bounds);
    SkRect dstRect = SkRect::MakeWH(srcRect.width(), srcRect.height());
    context->drawRectToRect(paint, dstRect, srcRect);
    offset->fX += bounds.left();
    offset->fY += bounds.top();
    return SkImageFilterUtils::WrapTexture(dst, bounds.width(), bounds.height(), result);
}

///////////////////////////////////////////////////////////////////////////////

GrDisplacementMapEffect::GrDisplacementMapEffect(
                             SkDisplacementMapEffect::ChannelSelectorType xChannelSelector,
                             SkDisplacementMapEffect::ChannelSelectorType yChannelSelector,
                             SkScalar scale,
                             GrTexture* displacement,
                             GrTexture* color)
    : fDisplacementTransform(kLocal_GrCoordSet, displacement)
    , fDisplacementAccess(displacement)
    , fColorTransform(kLocal_GrCoordSet, color)
    , fColorAccess(color)
    , fXChannelSelector(xChannelSelector)
    , fYChannelSelector(yChannelSelector)
    , fScale(scale) {
    this->addCoordTransform(&fDisplacementTransform);
    this->addTextureAccess(&fDisplacementAccess);
    this->addCoordTransform(&fColorTransform);
    this->addTextureAccess(&fColorAccess);
    this->setWillNotUseInputColor();
}

GrDisplacementMapEffect::~GrDisplacementMapEffect() {
}

bool GrDisplacementMapEffect::onIsEqual(const GrEffect& sBase) const {
    const GrDisplacementMapEffect& s = CastEffect<GrDisplacementMapEffect>(sBase);
    return fDisplacementAccess.getTexture() == s.fDisplacementAccess.getTexture() &&
           fColorAccess.getTexture() == s.fColorAccess.getTexture() &&
           fXChannelSelector == s.fXChannelSelector &&
           fYChannelSelector == s.fYChannelSelector &&
           fScale == s.fScale;
}

const GrBackendEffectFactory& GrDisplacementMapEffect::getFactory() const {
    return GrTBackendEffectFactory<GrDisplacementMapEffect>::getInstance();
}

void GrDisplacementMapEffect::getConstantColorComponents(GrColor*,
                                                         uint32_t* validFlags) const {
    // Any displacement offset bringing a pixel out of bounds will output a color of (0,0,0,0),
    // so the only way we'd get a constant alpha is if the input color image has a constant alpha
    // and no displacement offset push any texture coordinates out of bounds OR if the constant
    // alpha is 0. Since this isn't trivial to compute at this point, let's assume the output is
    // not of constant color when a displacement effect is applied.
    *validFlags = 0;
}

///////////////////////////////////////////////////////////////////////////////

GR_DEFINE_EFFECT_TEST(GrDisplacementMapEffect);

GrEffectRef* GrDisplacementMapEffect::TestCreate(SkRandom* random,
                                                 GrContext*,
                                                 const GrDrawTargetCaps&,
                                                 GrTexture* textures[]) {
    int texIdxDispl = random->nextBool() ? GrEffectUnitTest::kSkiaPMTextureIdx :
                                           GrEffectUnitTest::kAlphaTextureIdx;
    int texIdxColor = random->nextBool() ? GrEffectUnitTest::kSkiaPMTextureIdx :
                                           GrEffectUnitTest::kAlphaTextureIdx;
    static const int kMaxComponent = 4;
    SkDisplacementMapEffect::ChannelSelectorType xChannelSelector =
        static_cast<SkDisplacementMapEffect::ChannelSelectorType>(
        random->nextRangeU(1, kMaxComponent));
    SkDisplacementMapEffect::ChannelSelectorType yChannelSelector =
        static_cast<SkDisplacementMapEffect::ChannelSelectorType>(
        random->nextRangeU(1, kMaxComponent));
    SkScalar scale = random->nextRangeScalar(0, 100.0f);

    return GrDisplacementMapEffect::Create(xChannelSelector, yChannelSelector, scale,
                                           textures[texIdxDispl], textures[texIdxColor]);
}

///////////////////////////////////////////////////////////////////////////////

GrGLDisplacementMapEffect::GrGLDisplacementMapEffect(const GrBackendEffectFactory& factory,
                                                     const GrDrawEffect& drawEffect)
    : INHERITED(factory)
    , fXChannelSelector(drawEffect.castEffect<GrDisplacementMapEffect>().xChannelSelector())
    , fYChannelSelector(drawEffect.castEffect<GrDisplacementMapEffect>().yChannelSelector()) {
}

GrGLDisplacementMapEffect::~GrGLDisplacementMapEffect() {
}

void GrGLDisplacementMapEffect::emitCode(GrGLShaderBuilder* builder,
                                         const GrDrawEffect&,
                                         EffectKey key,
                                         const char* outputColor,
                                         const char* inputColor,
                                         const TransformedCoordsArray& coords,
                                         const TextureSamplerArray& samplers) {
    sk_ignore_unused_variable(inputColor);

    fScaleUni = builder->addUniform(GrGLShaderBuilder::kFragment_Visibility,
                                    kVec2f_GrSLType, "Scale");
    const char* scaleUni = builder->getUniformCStr(fScaleUni);
    const char* dColor = "dColor";
    const char* cCoords = "cCoords";
    const char* outOfBounds = "outOfBounds";
    const char* nearZero = "1e-6"; // Since 6.10352e−5 is the smallest half float, use
                                   // a number smaller than that to approximate 0, but
                                   // leave room for 32-bit float GPU rounding errors.

    builder->fsCodeAppendf("\t\tvec4 %s = ", dColor);
    builder->fsAppendTextureLookup(samplers[0], coords[0].c_str(), coords[0].type());
    builder->fsCodeAppend(";\n");

    // Unpremultiply the displacement
    builder->fsCodeAppendf("\t\t%s.rgb = (%s.a < %s) ? vec3(0.0) : clamp(%s.rgb / %s.a, 0.0, 1.0);",
                           dColor, dColor, nearZero, dColor, dColor);

    builder->fsCodeAppendf("\t\tvec2 %s = %s + %s*(%s.",
                           cCoords, coords[1].c_str(), scaleUni, dColor);

    switch (fXChannelSelector) {
      case SkDisplacementMapEffect::kR_ChannelSelectorType:
        builder->fsCodeAppend("r");
        break;
      case SkDisplacementMapEffect::kG_ChannelSelectorType:
        builder->fsCodeAppend("g");
        break;
      case SkDisplacementMapEffect::kB_ChannelSelectorType:
        builder->fsCodeAppend("b");
        break;
      case SkDisplacementMapEffect::kA_ChannelSelectorType:
        builder->fsCodeAppend("a");
        break;
      case SkDisplacementMapEffect::kUnknown_ChannelSelectorType:
      default:
        SkDEBUGFAIL("Unknown X channel selector");
    }

    switch (fYChannelSelector) {
      case SkDisplacementMapEffect::kR_ChannelSelectorType:
        builder->fsCodeAppend("r");
        break;
      case SkDisplacementMapEffect::kG_ChannelSelectorType:
        builder->fsCodeAppend("g");
        break;
      case SkDisplacementMapEffect::kB_ChannelSelectorType:
        builder->fsCodeAppend("b");
        break;
      case SkDisplacementMapEffect::kA_ChannelSelectorType:
        builder->fsCodeAppend("a");
        break;
      case SkDisplacementMapEffect::kUnknown_ChannelSelectorType:
      default:
        SkDEBUGFAIL("Unknown Y channel selector");
    }
    builder->fsCodeAppend("-vec2(0.5));\t\t");

    // FIXME : This can be achieved with a "clamp to border" texture repeat mode and
    //         a 0 border color instead of computing if cCoords is out of bounds here.
    builder->fsCodeAppendf(
        "bool %s = (%s.x < 0.0) || (%s.y < 0.0) || (%s.x > 1.0) || (%s.y > 1.0);\t\t",
        outOfBounds, cCoords, cCoords, cCoords, cCoords);
    builder->fsCodeAppendf("%s = %s ? vec4(0.0) : ", outputColor, outOfBounds);
    builder->fsAppendTextureLookup(samplers[1], cCoords, coords[1].type());
    builder->fsCodeAppend(";\n");
}

void GrGLDisplacementMapEffect::setData(const GrGLUniformManager& uman,
                                        const GrDrawEffect& drawEffect) {
    const GrDisplacementMapEffect& displacementMap =
        drawEffect.castEffect<GrDisplacementMapEffect>();
    GrTexture* colorTex = displacementMap.texture(1);
    SkScalar scaleX = SkScalarDiv(displacementMap.scale(), SkIntToScalar(colorTex->width()));
    SkScalar scaleY = SkScalarDiv(displacementMap.scale(), SkIntToScalar(colorTex->height()));
    uman.set2f(fScaleUni, SkScalarToFloat(scaleX),
               colorTex->origin() == kTopLeft_GrSurfaceOrigin ?
               SkScalarToFloat(scaleY) : SkScalarToFloat(-scaleY));
}

GrGLEffect::EffectKey GrGLDisplacementMapEffect::GenKey(const GrDrawEffect& drawEffect,
                                                        const GrGLCaps&) {
    const GrDisplacementMapEffect& displacementMap =
        drawEffect.castEffect<GrDisplacementMapEffect>();

    EffectKey xKey = displacementMap.xChannelSelector();
    EffectKey yKey = displacementMap.yChannelSelector() << SkDisplacementMapEffect::kKeyBits;

    return xKey | yKey;
}
#endif
