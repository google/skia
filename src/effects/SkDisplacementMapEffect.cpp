/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkDisplacementMapEffect.h"
#include "SkReadBuffer.h"
#include "SkWriteBuffer.h"
#include "SkUnPreMultiply.h"
#include "SkColorPriv.h"
#if SK_SUPPORT_GPU
#include "GrContext.h"
#include "GrCoordTransform.h"
#include "gl/GrGLEffect.h"
#include "GrTBackendEffectFactory.h"
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
    static const SkScalar Inv8bit = SkScalarDiv(SK_Scalar1, 255.0f);
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

SkDisplacementMapEffect::SkDisplacementMapEffect(SkReadBuffer& buffer)
  : INHERITED(2, buffer)
{
    fXChannelSelector = (SkDisplacementMapEffect::ChannelSelectorType) buffer.readInt();
    fYChannelSelector = (SkDisplacementMapEffect::ChannelSelectorType) buffer.readInt();
    fScale            = buffer.readScalar();
    buffer.validate(channel_selector_type_is_valid(fXChannelSelector) &&
                    channel_selector_type_is_valid(fYChannelSelector) &&
                    SkScalarIsFinite(fScale));
}

void SkDisplacementMapEffect::flatten(SkWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    buffer.writeInt((int) fXChannelSelector);
    buffer.writeInt((int) fYChannelSelector);
    buffer.writeScalar(fScale);
}

bool SkDisplacementMapEffect::onFilterImage(Proxy* proxy,
                                            const SkBitmap& src,
                                            const Context& ctx,
                                            SkBitmap* dst,
                                            SkIPoint* offset) const {
    SkBitmap displ = src, color = src;
    const SkImageFilter* colorInput = getColorInput();
    const SkImageFilter* displInput = getDisplacementInput();
    SkIPoint colorOffset = SkIPoint::Make(0, 0), displOffset = SkIPoint::Make(0, 0);
    if ((colorInput && !colorInput->filterImage(proxy, src, ctx, &color, &colorOffset)) ||
        (displInput && !displInput->filterImage(proxy, src, ctx, &displ, &displOffset))) {
        return false;
    }
    if ((displ.colorType() != kPMColor_SkColorType) ||
        (color.colorType() != kPMColor_SkColorType)) {
        return false;
    }
    SkIRect bounds;
    // Since computeDisplacement does bounds checking on color pixel access, we don't need to pad
    // the color bitmap to bounds here.
    if (!this->applyCropRect(ctx, color, colorOffset, &bounds)) {
        return false;
    }
    SkIRect displBounds;
    if (!this->applyCropRect(ctx, proxy, displ, &displOffset, &displBounds, &displ)) {
        return false;
    }
    if (!bounds.intersect(displBounds)) {
        return false;
    }
    SkAutoLockPixels alp_displacement(displ), alp_color(color);
    if (!displ.getPixels() || !color.getPixels()) {
        return false;
    }

    dst->setConfig(color.config(), bounds.width(), bounds.height());
    if (!dst->allocPixels()) {
        return false;
    }

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

void SkDisplacementMapEffect::computeFastBounds(const SkRect& src, SkRect* dst) const {
    if (getColorInput()) {
        getColorInput()->computeFastBounds(src, dst);
    } else {
        *dst = src;
    }
    dst->outset(fScale * SK_ScalarHalf, fScale * SK_ScalarHalf);
}

bool SkDisplacementMapEffect::onFilterBounds(const SkIRect& src, const SkMatrix& ctm,
                                   SkIRect* dst) const {
    SkIRect bounds = src;
    if (getColorInput() && !getColorInput()->filterBounds(src, ctm, &bounds)) {
        return false;
    }
    bounds.outset(SkScalarCeilToInt(fScale * SK_ScalarHalf),
                  SkScalarCeilToInt(fScale * SK_ScalarHalf));
    *dst = bounds;
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
                               SkVector scale,
                               GrTexture* displacement, const SkMatrix& offsetMatrix,
                               GrTexture* color) {
        AutoEffectUnref effect(SkNEW_ARGS(GrDisplacementMapEffect, (xChannelSelector,
                                                                    yChannelSelector,
                                                                    scale,
                                                                    displacement,
                                                                    offsetMatrix,
                                                                    color)));
        return CreateEffectRef(effect);
    }

    virtual ~GrDisplacementMapEffect();

    virtual const GrBackendEffectFactory& getFactory() const SK_OVERRIDE;
    SkDisplacementMapEffect::ChannelSelectorType xChannelSelector() const
        { return fXChannelSelector; }
    SkDisplacementMapEffect::ChannelSelectorType yChannelSelector() const
        { return fYChannelSelector; }
    const SkVector& scale() const { return fScale; }

    typedef GrGLDisplacementMapEffect GLEffect;
    static const char* Name() { return "DisplacementMap"; }

    virtual void getConstantColorComponents(GrColor* color, uint32_t* validFlags) const SK_OVERRIDE;

private:
    virtual bool onIsEqual(const GrEffect&) const SK_OVERRIDE;

    GrDisplacementMapEffect(SkDisplacementMapEffect::ChannelSelectorType xChannelSelector,
                            SkDisplacementMapEffect::ChannelSelectorType yChannelSelector,
                            const SkVector& scale,
                            GrTexture* displacement, const SkMatrix& offsetMatrix,
                            GrTexture* color);

    GR_DECLARE_EFFECT_TEST;

    GrCoordTransform            fDisplacementTransform;
    GrTextureAccess             fDisplacementAccess;
    GrCoordTransform            fColorTransform;
    GrTextureAccess             fColorAccess;
    SkDisplacementMapEffect::ChannelSelectorType fXChannelSelector;
    SkDisplacementMapEffect::ChannelSelectorType fYChannelSelector;
    SkVector fScale;

    typedef GrEffect INHERITED;
};

bool SkDisplacementMapEffect::filterImageGPU(Proxy* proxy, const SkBitmap& src, const Context& ctx,
                                             SkBitmap* result, SkIPoint* offset) const {
    SkBitmap colorBM = src;
    SkIPoint colorOffset = SkIPoint::Make(0, 0);
    if (getColorInput() && !getColorInput()->getInputResultGPU(proxy, src, ctx, &colorBM,
                                                               &colorOffset)) {
        return false;
    }
    SkBitmap displacementBM = src;
    SkIPoint displacementOffset = SkIPoint::Make(0, 0);
    if (getDisplacementInput() &&
        !getDisplacementInput()->getInputResultGPU(proxy, src, ctx, &displacementBM,
                                                   &displacementOffset)) {
        return false;
    }
    SkIRect bounds;
    // Since GrDisplacementMapEffect does bounds checking on color pixel access, we don't need to
    // pad the color bitmap to bounds here.
    if (!this->applyCropRect(ctx, colorBM, colorOffset, &bounds)) {
        return false;
    }
    SkIRect displBounds;
    if (!this->applyCropRect(ctx, proxy, displacementBM,
                             &displacementOffset, &displBounds, &displacementBM)) {
        return false;
    }
    if (!bounds.intersect(displBounds)) {
        return false;
    }
    GrTexture* color = colorBM.getTexture();
    GrTexture* displacement = displacementBM.getTexture();
    GrContext* context = color->getContext();

    GrTextureDesc desc;
    desc.fFlags = kRenderTarget_GrTextureFlagBit | kNoStencil_GrTextureFlagBit;
    desc.fWidth = colorBM.width();
    desc.fHeight = colorBM.height();
    desc.fConfig = kSkia8888_GrPixelConfig;

    GrAutoScratchTexture ast(context, desc);
    SkAutoTUnref<GrTexture> dst(ast.detach());

    GrContext::AutoRenderTarget art(context, dst->asRenderTarget());

    SkVector scale = SkVector::Make(fScale, fScale);
    ctx.ctm().mapVectors(&scale, 1);

    GrPaint paint;
    SkMatrix offsetMatrix = GrEffect::MakeDivByTextureWHMatrix(displacement);
    offsetMatrix.preTranslate(SkIntToScalar(colorOffset.fX - displacementOffset.fX),
                              SkIntToScalar(colorOffset.fY - displacementOffset.fY));

    paint.addColorEffect(
        GrDisplacementMapEffect::Create(fXChannelSelector,
                                        fYChannelSelector,
                                        scale,
                                        displacement,
                                        offsetMatrix,
                                        color))->unref();
    SkIRect colorBounds = bounds;
    colorBounds.offset(-colorOffset);
    SkMatrix matrix;
    matrix.setTranslate(-SkIntToScalar(colorBounds.x()),
                        -SkIntToScalar(colorBounds.y()));
    context->concatMatrix(matrix);
    context->drawRect(paint, SkRect::Make(colorBounds));
    offset->fX = bounds.left();
    offset->fY = bounds.top();
    WrapTexture(dst, bounds.width(), bounds.height(), result);
    return true;
}

///////////////////////////////////////////////////////////////////////////////

GrDisplacementMapEffect::GrDisplacementMapEffect(
                             SkDisplacementMapEffect::ChannelSelectorType xChannelSelector,
                             SkDisplacementMapEffect::ChannelSelectorType yChannelSelector,
                             const SkVector& scale,
                             GrTexture* displacement,
                             const SkMatrix& offsetMatrix,
                             GrTexture* color)
    : fDisplacementTransform(kLocal_GrCoordSet, offsetMatrix, displacement)
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
    SkVector scale = SkVector::Make(random->nextRangeScalar(0, 100.0f),
                                    random->nextRangeScalar(0, 100.0f));

    return GrDisplacementMapEffect::Create(xChannelSelector, yChannelSelector, scale,
                                           textures[texIdxDispl], SkMatrix::I(),
                                           textures[texIdxColor]);
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
    SkScalar scaleX = SkScalarDiv(displacementMap.scale().fX, SkIntToScalar(colorTex->width()));
    SkScalar scaleY = SkScalarDiv(displacementMap.scale().fY, SkIntToScalar(colorTex->height()));
    uman.set2f(fScaleUni, SkScalarToFloat(scaleX),
               colorTex->origin() == kTopLeft_GrSurfaceOrigin ?
               SkScalarToFloat(scaleY) : SkScalarToFloat(-scaleY));
}

GrGLEffect::EffectKey GrGLDisplacementMapEffect::GenKey(const GrDrawEffect& drawEffect,
                                                        const GrGLCaps&) {
    const GrDisplacementMapEffect& displacementMap =
        drawEffect.castEffect<GrDisplacementMapEffect>();

    EffectKey xKey = displacementMap.xChannelSelector();
    EffectKey yKey = displacementMap.yChannelSelector() << kChannelSelectorKeyBits;

    return xKey | yKey;
}
#endif
