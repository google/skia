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
#include "gl/GrGLEffect.h"
#include "gl/GrGLEffectMatrix.h"
#include "GrTBackendEffectFactory.h"
#include "SkImageFilterUtils.h"
#endif

namespace {

template<SkDisplacementMapEffect::ChannelSelectorType type>
uint32_t getValue(SkColor, const SkUnPreMultiply::Scale*) {
    SkASSERT(!"Unknown channel selector");
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
void computeDisplacement(SkScalar scale, SkBitmap* dst, SkBitmap* displ, SkBitmap* src)
{
    static const SkScalar Inv8bit = SkScalarDiv(SK_Scalar1, SkFloatToScalar(255.0f));
    const int dstW = displ->width();
    const int dstH = displ->height();
    const int srcW = src->width();
    const int srcH = src->height();
    const SkScalar scaleForColor = SkScalarMul(scale, Inv8bit);
    const SkScalar scaleAdj = SK_ScalarHalf - SkScalarMul(scale, SK_ScalarHalf);
    const SkUnPreMultiply::Scale* table = SkUnPreMultiply::GetScaleTable();
    for (int y = 0; y < dstH; ++y) {
        const SkPMColor* displPtr = displ->getAddr32(0, y);
        SkPMColor* dstPtr = dst->getAddr32(0, y);
        for (int x = 0; x < dstW; ++x, ++displPtr, ++dstPtr) {
            const SkScalar displX = SkScalarMul(scaleForColor,
                SkIntToScalar(getValue<typeX>(*displPtr, table))) + scaleAdj;
            const SkScalar displY = SkScalarMul(scaleForColor,
                SkIntToScalar(getValue<typeY>(*displPtr, table))) + scaleAdj;
            // Truncate the displacement values
            const int srcX = x + SkScalarTruncToInt(displX);
            const int srcY = y + SkScalarTruncToInt(displY);
            *dstPtr = ((srcX < 0) || (srcX >= srcW) || (srcY < 0) || (srcY >= srcH)) ?
                      0 : *(src->getAddr32(srcX, srcY));
        }
    }
}

template<SkDisplacementMapEffect::ChannelSelectorType typeX>
void computeDisplacement(SkDisplacementMapEffect::ChannelSelectorType yChannelSelector,
                         SkScalar scale, SkBitmap* dst, SkBitmap* displ, SkBitmap* src)
{
    switch (yChannelSelector) {
      case SkDisplacementMapEffect::kR_ChannelSelectorType:
        computeDisplacement<typeX, SkDisplacementMapEffect::kR_ChannelSelectorType>(
            scale, dst, displ, src);
        break;
      case SkDisplacementMapEffect::kG_ChannelSelectorType:
        computeDisplacement<typeX, SkDisplacementMapEffect::kG_ChannelSelectorType>(
            scale, dst, displ, src);
        break;
      case SkDisplacementMapEffect::kB_ChannelSelectorType:
        computeDisplacement<typeX, SkDisplacementMapEffect::kB_ChannelSelectorType>(
            scale, dst, displ, src);
        break;
      case SkDisplacementMapEffect::kA_ChannelSelectorType:
        computeDisplacement<typeX, SkDisplacementMapEffect::kA_ChannelSelectorType>(
            scale, dst, displ, src);
        break;
      case SkDisplacementMapEffect::kUnknown_ChannelSelectorType:
      default:
        SkASSERT(!"Unknown Y channel selector");
    }
}

void computeDisplacement(SkDisplacementMapEffect::ChannelSelectorType xChannelSelector,
                         SkDisplacementMapEffect::ChannelSelectorType yChannelSelector,
                         SkScalar scale, SkBitmap* dst, SkBitmap* displ, SkBitmap* src)
{
    switch (xChannelSelector) {
      case SkDisplacementMapEffect::kR_ChannelSelectorType:
        computeDisplacement<SkDisplacementMapEffect::kR_ChannelSelectorType>(
            yChannelSelector, scale, dst, displ, src);
        break;
      case SkDisplacementMapEffect::kG_ChannelSelectorType:
        computeDisplacement<SkDisplacementMapEffect::kG_ChannelSelectorType>(
            yChannelSelector, scale, dst, displ, src);
        break;
      case SkDisplacementMapEffect::kB_ChannelSelectorType:
        computeDisplacement<SkDisplacementMapEffect::kB_ChannelSelectorType>(
            yChannelSelector, scale, dst, displ, src);
        break;
      case SkDisplacementMapEffect::kA_ChannelSelectorType:
        computeDisplacement<SkDisplacementMapEffect::kA_ChannelSelectorType>(
            yChannelSelector, scale, dst, displ, src);
        break;
      case SkDisplacementMapEffect::kUnknown_ChannelSelectorType:
      default:
        SkASSERT(!"Unknown X channel selector");
    }
}

} // end namespace

///////////////////////////////////////////////////////////////////////////////

SkDisplacementMapEffect::SkDisplacementMapEffect(ChannelSelectorType xChannelSelector,
                                                 ChannelSelectorType yChannelSelector,
                                                 SkScalar scale,
                                                 SkImageFilter* displacement,
                                                 SkImageFilter* color)
  : INHERITED(displacement, color)
  , fXChannelSelector(xChannelSelector)
  , fYChannelSelector(yChannelSelector)
  , fScale(scale)
{
}

SkDisplacementMapEffect::~SkDisplacementMapEffect() {
}

SkDisplacementMapEffect::SkDisplacementMapEffect(SkFlattenableReadBuffer& buffer)
  : INHERITED(buffer)
{
    fXChannelSelector = (SkDisplacementMapEffect::ChannelSelectorType) buffer.readInt();
    fYChannelSelector = (SkDisplacementMapEffect::ChannelSelectorType) buffer.readInt();
    fScale            = buffer.readScalar();
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
    dst->setConfig(displ.config(), displ.width(), displ.height());
    dst->allocPixels();
    if (!dst->getPixels()) {
        return false;
    }

    computeDisplacement(fXChannelSelector, fYChannelSelector, fScale, dst, &displ, &color);

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
                          const TextureSamplerArray&) SK_OVERRIDE;

    static inline EffectKey GenKey(const GrDrawEffect&, const GrGLCaps&);

    virtual void setData(const GrGLUniformManager&, const GrDrawEffect&) SK_OVERRIDE;

private:
    static const GrEffect::CoordsType kCoordsType = GrEffect::kLocal_CoordsType;

    SkDisplacementMapEffect::ChannelSelectorType fXChannelSelector;
    SkDisplacementMapEffect::ChannelSelectorType fYChannelSelector;
    GrGLEffectMatrix fDisplacementEffectMatrix;
    GrGLEffectMatrix fColorEffectMatrix;
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

    GrTextureAccess             fDisplacementAccess;
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
    SkRect srcRect;
    src.getBounds(&srcRect);
    SkRect dstRect = srcRect;
    dstRect.offset(SkIntToScalar(colorOffset.fX), SkIntToScalar(colorOffset.fY));
    context->drawRectToRect(paint, srcRect, dstRect);
    return SkImageFilterUtils::WrapTexture(dst, src.width(), src.height(), result);
}

///////////////////////////////////////////////////////////////////////////////

GrDisplacementMapEffect::GrDisplacementMapEffect(
                             SkDisplacementMapEffect::ChannelSelectorType xChannelSelector,
                             SkDisplacementMapEffect::ChannelSelectorType yChannelSelector,
                             SkScalar scale,
                             GrTexture* displacement,
                             GrTexture* color)
    : fDisplacementAccess(displacement)
    , fColorAccess(color)
    , fXChannelSelector(xChannelSelector)
    , fYChannelSelector(yChannelSelector)
    , fScale(scale) {
    this->addTextureAccess(&fDisplacementAccess);
    this->addTextureAccess(&fColorAccess);
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

GrEffectRef* GrDisplacementMapEffect::TestCreate(SkMWCRandom* random,
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
    SkScalar scale = random->nextRangeScalar(0, SkFloatToScalar(100.0f));

    return GrDisplacementMapEffect::Create(xChannelSelector, yChannelSelector, scale,
                                           textures[texIdxDispl], textures[texIdxColor]);
}

///////////////////////////////////////////////////////////////////////////////

GrGLDisplacementMapEffect::GrGLDisplacementMapEffect(const GrBackendEffectFactory& factory,
                                                     const GrDrawEffect& drawEffect)
    : INHERITED(factory)
    , fXChannelSelector(drawEffect.castEffect<GrDisplacementMapEffect>().xChannelSelector())
    , fYChannelSelector(drawEffect.castEffect<GrDisplacementMapEffect>().yChannelSelector())
    , fDisplacementEffectMatrix(kCoordsType)
    , fColorEffectMatrix(kCoordsType) {
}

GrGLDisplacementMapEffect::~GrGLDisplacementMapEffect() {
}

void GrGLDisplacementMapEffect::emitCode(GrGLShaderBuilder* builder,
                                         const GrDrawEffect&,
                                         EffectKey key,
                                         const char* outputColor,
                                         const char* inputColor,
                                         const TextureSamplerArray& samplers) {
    sk_ignore_unused_variable(inputColor);

    fScaleUni = builder->addUniform(GrGLShaderBuilder::kFragment_ShaderType,
                                    kVec2f_GrSLType, "Scale");
    const char* scaleUni = builder->getUniformCStr(fScaleUni);

    const char* dCoordsIn;
    GrSLType dCoordsType = fDisplacementEffectMatrix.emitCode(
                                builder, key, &dCoordsIn, NULL, "DISPL");
    const char* cCoordsIn;
    GrSLType cCoordsType = fColorEffectMatrix.emitCode(
                                builder, key, &cCoordsIn, NULL, "COLOR");

    const char* dColor = "dColor";
    const char* cCoords = "cCoords";
    const char* outOfBounds = "outOfBounds";
    const char* nearZero = "1e-6"; // Since 6.10352e−5 is the smallest half float, use
                                   // a number smaller than that to approximate 0, but
                                   // leave room for 32-bit float GPU rounding errors.

    builder->fsCodeAppendf("\t\tvec4 %s = ", dColor);
    builder->appendTextureLookup(GrGLShaderBuilder::kFragment_ShaderType,
                                 samplers[0],
                                 dCoordsIn,
                                 dCoordsType);
    builder->fsCodeAppend(";\n");

    // Unpremultiply the displacement
    builder->fsCodeAppendf("\t\t%s.rgb = (%s.a < %s) ? vec3(0.0) : clamp(%s.rgb / %s.a, 0.0, 1.0);",
                           dColor, dColor, nearZero, dColor, dColor);

    builder->fsCodeAppendf("\t\tvec2 %s = %s + %s*(%s.",
                           cCoords, cCoordsIn, scaleUni, dColor);

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
        SkASSERT(!"Unknown X channel selector");
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
        SkASSERT(!"Unknown Y channel selector");
    }
    builder->fsCodeAppend("-vec2(0.5));\t\t");

    // FIXME : This can be achieved with a "clamp to border" texture repeat mode and
    //         a 0 border color instead of computing if cCoords is out of bounds here.
    builder->fsCodeAppendf(
        "bool %s = (%s.x < 0.0) || (%s.y < 0.0) || (%s.x > 1.0) || (%s.y > 1.0);\t\t",
        outOfBounds, cCoords, cCoords, cCoords, cCoords);
    builder->fsCodeAppendf("%s = %s ? vec4(0.0) : ", outputColor, outOfBounds);
    builder->appendTextureLookup(GrGLShaderBuilder::kFragment_ShaderType,
                                 samplers[1],
                                 cCoords,
                                 cCoordsType);
    builder->fsCodeAppend(";\n");
}

void GrGLDisplacementMapEffect::setData(const GrGLUniformManager& uman,
                                        const GrDrawEffect& drawEffect) {
    const GrDisplacementMapEffect& displacementMap =
        drawEffect.castEffect<GrDisplacementMapEffect>();
    GrTexture* displTex = displacementMap.texture(0);
    GrTexture* colorTex = displacementMap.texture(1);
    fDisplacementEffectMatrix.setData(uman,
                                     GrEffect::MakeDivByTextureWHMatrix(displTex),
                                     drawEffect,
                                     displTex);
    fColorEffectMatrix.setData(uman,
                               GrEffect::MakeDivByTextureWHMatrix(colorTex),
                               drawEffect,
                               colorTex);

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

    GrTexture* displTex = displacementMap.texture(0);
    GrTexture* colorTex = displacementMap.texture(1);

    EffectKey displKey = GrGLEffectMatrix::GenKey(GrEffect::MakeDivByTextureWHMatrix(displTex),
                                                  drawEffect,
                                                  kCoordsType,
                                                  displTex);

    EffectKey colorKey = GrGLEffectMatrix::GenKey(GrEffect::MakeDivByTextureWHMatrix(colorTex),
                                                  drawEffect,
                                                  kCoordsType,
                                                  colorTex);

    colorKey <<= GrGLEffectMatrix::kKeyBits;
    EffectKey xKey = displacementMap.xChannelSelector() << (2 * GrGLEffectMatrix::kKeyBits);
    EffectKey yKey = displacementMap.yChannelSelector() << (2 * GrGLEffectMatrix::kKeyBits +
                                                            SkDisplacementMapEffect::kKeyBits);

    return xKey | yKey | displKey | colorKey;
}
#endif
