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
    static const SkScalar Half8bit = SkFloatToScalar(255.0f * 0.5f);
    const int dstW = displ->width();
    const int dstH = displ->height();
    const int srcW = src->width();
    const int srcH = src->height();
    const SkScalar scaleX = SkScalarMul(SkScalarMul(scale, SkIntToScalar(dstW)), Inv8bit);
    const SkScalar scaleY = SkScalarMul(SkScalarMul(scale, SkIntToScalar(dstH)), Inv8bit);
    const SkUnPreMultiply::Scale* table = SkUnPreMultiply::GetScaleTable();
    for (int y = 0; y < dstH; ++y) {
        const SkPMColor* displPtr = displ->getAddr32(0, y);
        SkPMColor* dstPtr = dst->getAddr32(0, y);
        for (int x = 0; x < dstW; ++x, ++displPtr, ++dstPtr) {
            const SkScalar displX =
                SkScalarMul(scaleX, SkIntToScalar(getValue<typeX>(*displPtr, table))-Half8bit);
            const SkScalar displY =
                SkScalarMul(scaleY, SkIntToScalar(getValue<typeY>(*displPtr, table))-Half8bit);
            const int coordX = x + SkScalarRoundToInt(displX);
            const int coordY = y + SkScalarRoundToInt(displY);
            *dstPtr = ((coordX < 0) || (coordX >= srcW) || (coordY < 0) || (coordY >= srcH)) ?
                      0 : *(src->getAddr32(coordX, coordY));
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
                              const GrEffectRef& effect);
    virtual ~GrGLDisplacementMapEffect();

    virtual void emitCode(GrGLShaderBuilder*,
                          const GrEffectStage&,
                          EffectKey,
                          const char* vertexCoords,
                          const char* outputColor,
                          const char* inputColor,
                          const TextureSamplerArray&) SK_OVERRIDE;

    static inline EffectKey GenKey(const GrEffectStage&, const GrGLCaps&);

    virtual void setData(const GrGLUniformManager&, const GrEffectStage&);

private:
    SkDisplacementMapEffect::ChannelSelectorType fXChannelSelector;
    SkDisplacementMapEffect::ChannelSelectorType fYChannelSelector;
    GrGLEffectMatrix fDisplacementEffectMatrix;
    GrGLEffectMatrix fColorEffectMatrix;
    GrGLUniformManager::UniformHandle fScaleUni;
    GrGLUniformManager::UniformHandle fYSignColor;
    GrGLUniformManager::UniformHandle fYSignDispl;

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

    const GrBackendEffectFactory& getFactory() const;
    SkDisplacementMapEffect::ChannelSelectorType xChannelSelector() const
        { return fXChannelSelector; }
    SkDisplacementMapEffect::ChannelSelectorType yChannelSelector() const
        { return fYChannelSelector; }
    SkScalar scale() const { return fScale; }

    typedef GrGLDisplacementMapEffect GLEffect;
    static const char* Name() { return "DisplacementMap"; }

    void getConstantColorComponents(GrColor* color, uint32_t* validFlags) const SK_OVERRIDE;

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

bool SkDisplacementMapEffect::filterImageGPU(Proxy* proxy, const SkBitmap& src, SkBitmap* result) {
    SkBitmap colorBM;
    if (!SkImageFilterUtils::GetInputResultGPU(getColorInput(), proxy, src, &colorBM)) {
        return false;
    }
    GrTexture* color = (GrTexture*) colorBM.getTexture();
    SkBitmap displacementBM;
    if (!SkImageFilterUtils::GetInputResultGPU(getDisplacementInput(), proxy, src, &displacementBM)) {
        return false;
    }
    GrTexture* displacement = (GrTexture*) displacementBM.getTexture();
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
    paint.colorStage(0)->setEffect(
        GrDisplacementMapEffect::Create(fXChannelSelector,
                                        fYChannelSelector,
                                        fScale,
                                        displacement,
                                        color))->unref();
    SkRect srcRect;
    src.getBounds(&srcRect);
    context->drawRect(paint, srcRect);
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

void GrDisplacementMapEffect::getConstantColorComponents(GrColor* color,
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
                                                 GrContext* context,
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
    SkScalar scale = random->nextUScalar1();

    return GrDisplacementMapEffect::Create(xChannelSelector, yChannelSelector, scale,
                                           textures[texIdxDispl], textures[texIdxColor]);
}

///////////////////////////////////////////////////////////////////////////////

GrGLDisplacementMapEffect::GrGLDisplacementMapEffect(const GrBackendEffectFactory& factory,
                                                     const GrEffectRef& effect)
    : INHERITED(factory)
    , fXChannelSelector(CastEffect<GrDisplacementMapEffect>(effect).xChannelSelector())
    , fYChannelSelector(CastEffect<GrDisplacementMapEffect>(effect).yChannelSelector()) {
}

GrGLDisplacementMapEffect::~GrGLDisplacementMapEffect() {
}

void GrGLDisplacementMapEffect::emitCode(GrGLShaderBuilder* builder,
                               const GrEffectStage&,
                               EffectKey key,
                               const char* vertexCoords,
                               const char* outputColor,
                               const char* inputColor,
                               const TextureSamplerArray& samplers) {
    fScaleUni = builder->addUniform(GrGLShaderBuilder::kFragment_ShaderType,
                                    kVec2f_GrSLType, "Scale");
    const char* scaleUni = builder->getUniformCStr(fScaleUni);

    const char* dCoordsIn;
    GrSLType dCoordsType = fDisplacementEffectMatrix.emitCode(
                                builder, key, vertexCoords, &dCoordsIn, NULL, "DISPL");
    const char* cCoordsIn;
    GrSLType cCoordsType = fColorEffectMatrix.emitCode(
                                builder, key, vertexCoords, &cCoordsIn, NULL, "COLOR");

    SkString* code = &builder->fFSCode;
    const char* dColor = "dColor";
    const char* cCoords = "cCoords";
    const char* nearZero = "1e-6"; // Since 6.10352e−5 is the smallest half float, use
                                   // a number smaller than that to approximate 0, but
                                   // leave room for 32-bit float GPU rounding errors.

    code->appendf("\t\tvec4 %s = ", dColor);
    builder->appendTextureLookup(code, samplers[0], dCoordsIn, dCoordsType);
    code->append(";\n");

    // Unpremultiply the displacement
    code->appendf("\t\t%s.rgb = (%s.a < %s) ? vec3(0.0) : clamp(%s.rgb / %s.a, 0.0, 1.0);",
                  dColor, dColor, nearZero, dColor, dColor);

    code->appendf("\t\tvec2 %s = %s + %s*(%s.",
                  cCoords, cCoordsIn, scaleUni, dColor);

    switch (fXChannelSelector) {
      case SkDisplacementMapEffect::kR_ChannelSelectorType:
        code->append("r");
        break;
      case SkDisplacementMapEffect::kG_ChannelSelectorType:
        code->append("g");
        break;
      case SkDisplacementMapEffect::kB_ChannelSelectorType:
        code->append("b");
        break;
      case SkDisplacementMapEffect::kA_ChannelSelectorType:
        code->append("a");
        break;
      case SkDisplacementMapEffect::kUnknown_ChannelSelectorType:
      default:
        SkASSERT(!"Unknown X channel selector");
    }

    switch (fYChannelSelector) {
      case SkDisplacementMapEffect::kR_ChannelSelectorType:
        code->append("r");
        break;
      case SkDisplacementMapEffect::kG_ChannelSelectorType:
        code->append("g");
        break;
      case SkDisplacementMapEffect::kB_ChannelSelectorType:
        code->append("b");
        break;
      case SkDisplacementMapEffect::kA_ChannelSelectorType:
        code->append("a");
        break;
      case SkDisplacementMapEffect::kUnknown_ChannelSelectorType:
      default:
        SkASSERT(!"Unknown Y channel selector");
    }
    code->append("-vec2(0.5));\t\t");

    // FIXME : This can be achieved with a "clamp to border" texture repeat mode and
    //         a 0 border color instead of computing if cCoords is out of bounds here.
    code->appendf(
        "%s = any(greaterThan(vec4(vec2(0.0), %s), vec4(%s, vec2(1.0)))) ? vec4(0.0) : ",
        outputColor, cCoords, cCoords);
    builder->appendTextureLookup(code, samplers[1], cCoords, cCoordsType);
    code->append(";\n");
}

void GrGLDisplacementMapEffect::setData(const GrGLUniformManager& uman, const GrEffectStage& stage) {
    const GrDisplacementMapEffect& displacementMap = GetEffectFromStage<GrDisplacementMapEffect>(stage);
    GrTexture* displTex = displacementMap.texture(0);
    GrTexture* colorTex = displacementMap.texture(1);
    fDisplacementEffectMatrix.setData(uman,
                                     GrEffect::MakeDivByTextureWHMatrix(displTex),
                                     stage.getCoordChangeMatrix(),
                                     displTex);
    fColorEffectMatrix.setData(uman,
                               GrEffect::MakeDivByTextureWHMatrix(colorTex),
                               stage.getCoordChangeMatrix(),
                               colorTex);

    uman.set2f(fScaleUni, SkScalarToFloat(displacementMap.scale()),
                colorTex->origin() == kTopLeft_GrSurfaceOrigin ?
                SkScalarToFloat(displacementMap.scale()) :
                SkScalarToFloat(-displacementMap.scale()));
}

GrGLEffect::EffectKey GrGLDisplacementMapEffect::GenKey(const GrEffectStage& stage,
                                                        const GrGLCaps&) {
    const GrDisplacementMapEffect& displacementMap =
        GetEffectFromStage<GrDisplacementMapEffect>(stage);

    GrTexture* displTex = displacementMap.texture(0);
    GrTexture* colorTex = displacementMap.texture(1);

    EffectKey displKey = GrGLEffectMatrix::GenKey(GrEffect::MakeDivByTextureWHMatrix(displTex),
                                                  stage.getCoordChangeMatrix(),
                                                  displTex);

    EffectKey colorKey = GrGLEffectMatrix::GenKey(GrEffect::MakeDivByTextureWHMatrix(colorTex),
                                                  stage.getCoordChangeMatrix(),
                                                  colorTex);

    colorKey <<= GrGLEffectMatrix::kKeyBits;
    EffectKey xKey = displacementMap.xChannelSelector() << (2 * GrGLEffectMatrix::kKeyBits);
    EffectKey yKey = displacementMap.yChannelSelector() << (2 * GrGLEffectMatrix::kKeyBits +
                                                            SkDisplacementMapEffect::kKeyBits);

    return xKey | yKey | displKey | colorKey;
}
#endif
