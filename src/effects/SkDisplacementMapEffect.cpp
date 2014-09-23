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
#include "gl/GrGLProcessor.h"
#include "gl/builders/GrGLProgramBuilder.h"
#include "GrTBackendProcessorFactory.h"
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

SkDisplacementMapEffect* SkDisplacementMapEffect::Create(ChannelSelectorType xChannelSelector,
                                                         ChannelSelectorType yChannelSelector,
                                                         SkScalar scale,
                                                         SkImageFilter* displacement,
                                                         SkImageFilter* color,
                                                         const CropRect* cropRect, uint32_t uniqueID) {
    if (!channel_selector_type_is_valid(xChannelSelector) ||
        !channel_selector_type_is_valid(yChannelSelector)) {
        return NULL;
    }

    SkImageFilter* inputs[2] = { displacement, color };
    return SkNEW_ARGS(SkDisplacementMapEffect, (xChannelSelector, yChannelSelector, scale,
                                                inputs, cropRect, uniqueID));
}

SkDisplacementMapEffect::SkDisplacementMapEffect(ChannelSelectorType xChannelSelector,
                                                 ChannelSelectorType yChannelSelector,
                                                 SkScalar scale,
                                                 SkImageFilter* inputs[2],
                                                 const CropRect* cropRect,
                                                 uint32_t uniqueID)
  : INHERITED(2, inputs, cropRect, uniqueID)
  , fXChannelSelector(xChannelSelector)
  , fYChannelSelector(yChannelSelector)
  , fScale(scale)
{
}

SkDisplacementMapEffect::~SkDisplacementMapEffect() {
}

#ifdef SK_SUPPORT_LEGACY_DEEPFLATTENING
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
#endif

SkFlattenable* SkDisplacementMapEffect::CreateProc(SkReadBuffer& buffer) {
    SK_IMAGEFILTER_UNFLATTEN_COMMON(common, 2);
    ChannelSelectorType xsel = (ChannelSelectorType)buffer.readInt();
    ChannelSelectorType ysel = (ChannelSelectorType)buffer.readInt();
    SkScalar scale = buffer.readScalar();
    return Create(xsel, ysel, scale, common.getInput(0), common.getInput(1), &common.cropRect(), common.uniqueID());
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
    if ((displ.colorType() != kN32_SkColorType) ||
        (color.colorType() != kN32_SkColorType)) {
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

    if (!dst->tryAllocPixels(color.info().makeWH(bounds.width(), bounds.height()))) {
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
    SkVector scale = SkVector::Make(fScale, fScale);
    ctm.mapVectors(&scale, 1);
    bounds.outset(SkScalarCeilToInt(scale.fX * SK_ScalarHalf),
                  SkScalarCeilToInt(scale.fY * SK_ScalarHalf));
    if (getColorInput()) {
        return getColorInput()->filterBounds(bounds, ctm, dst);
    }
    *dst = bounds;
    return true;
}

///////////////////////////////////////////////////////////////////////////////

#if SK_SUPPORT_GPU
class GrGLDisplacementMapEffect : public GrGLFragmentProcessor {
public:
    GrGLDisplacementMapEffect(const GrBackendProcessorFactory&,
                              const GrProcessor&);
    virtual ~GrGLDisplacementMapEffect();

    virtual void emitCode(GrGLProgramBuilder*,
                          const GrFragmentProcessor&,
                          const GrProcessorKey&,
                          const char* outputColor,
                          const char* inputColor,
                          const TransformedCoordsArray&,
                          const TextureSamplerArray&) SK_OVERRIDE;

    static inline void GenKey(const GrProcessor&, const GrGLCaps&, GrProcessorKeyBuilder*);

    virtual void setData(const GrGLProgramDataManager&, const GrProcessor&) SK_OVERRIDE;

private:
    SkDisplacementMapEffect::ChannelSelectorType fXChannelSelector;
    SkDisplacementMapEffect::ChannelSelectorType fYChannelSelector;
    GrGLProgramDataManager::UniformHandle fScaleUni;

    typedef GrGLFragmentProcessor INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

class GrDisplacementMapEffect : public GrFragmentProcessor {
public:
    static GrFragmentProcessor* Create(
            SkDisplacementMapEffect::ChannelSelectorType xChannelSelector,
            SkDisplacementMapEffect::ChannelSelectorType yChannelSelector, SkVector scale,
            GrTexture* displacement, const SkMatrix& offsetMatrix, GrTexture* color) {
        return SkNEW_ARGS(GrDisplacementMapEffect, (xChannelSelector,
                                                    yChannelSelector,
                                                    scale,
                                                    displacement,
                                                    offsetMatrix,
                                                    color));
    }

    virtual ~GrDisplacementMapEffect();

    virtual const GrBackendFragmentProcessorFactory& getFactory() const SK_OVERRIDE;
    SkDisplacementMapEffect::ChannelSelectorType xChannelSelector() const
        { return fXChannelSelector; }
    SkDisplacementMapEffect::ChannelSelectorType yChannelSelector() const
        { return fYChannelSelector; }
    const SkVector& scale() const { return fScale; }

    typedef GrGLDisplacementMapEffect GLProcessor;
    static const char* Name() { return "DisplacementMap"; }

    virtual void getConstantColorComponents(GrColor* color, uint32_t* validFlags) const SK_OVERRIDE;

private:
    virtual bool onIsEqual(const GrProcessor&) const SK_OVERRIDE;

    GrDisplacementMapEffect(SkDisplacementMapEffect::ChannelSelectorType xChannelSelector,
                            SkDisplacementMapEffect::ChannelSelectorType yChannelSelector,
                            const SkVector& scale,
                            GrTexture* displacement, const SkMatrix& offsetMatrix,
                            GrTexture* color);

    GR_DECLARE_FRAGMENT_PROCESSOR_TEST;

    GrCoordTransform            fDisplacementTransform;
    GrTextureAccess             fDisplacementAccess;
    GrCoordTransform            fColorTransform;
    GrTextureAccess             fColorAccess;
    SkDisplacementMapEffect::ChannelSelectorType fXChannelSelector;
    SkDisplacementMapEffect::ChannelSelectorType fYChannelSelector;
    SkVector fScale;

    typedef GrFragmentProcessor INHERITED;
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
    desc.fWidth = bounds.width();
    desc.fHeight = bounds.height();
    desc.fConfig = kSkia8888_GrPixelConfig;

    GrAutoScratchTexture ast(context, desc);
    if (NULL == ast.texture()) {
        return false;
    }
    SkAutoTUnref<GrTexture> dst(ast.detach());

    GrContext::AutoRenderTarget art(context, dst->asRenderTarget());

    SkVector scale = SkVector::Make(fScale, fScale);
    ctx.ctm().mapVectors(&scale, 1);

    GrPaint paint;
    SkMatrix offsetMatrix = GrCoordTransform::MakeDivByTextureWHMatrix(displacement);
    offsetMatrix.preTranslate(SkIntToScalar(colorOffset.fX - displacementOffset.fX),
                              SkIntToScalar(colorOffset.fY - displacementOffset.fY));

    paint.addColorProcessor(
        GrDisplacementMapEffect::Create(fXChannelSelector,
                                        fYChannelSelector,
                                        scale,
                                        displacement,
                                        offsetMatrix,
                                        color))->unref();
    SkIRect colorBounds = bounds;
    colorBounds.offset(-colorOffset);
    GrContext::AutoMatrix am;
    am.setIdentity(context);
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

bool GrDisplacementMapEffect::onIsEqual(const GrProcessor& sBase) const {
    const GrDisplacementMapEffect& s = sBase.cast<GrDisplacementMapEffect>();
    return fDisplacementAccess.getTexture() == s.fDisplacementAccess.getTexture() &&
           fColorAccess.getTexture() == s.fColorAccess.getTexture() &&
           fXChannelSelector == s.fXChannelSelector &&
           fYChannelSelector == s.fYChannelSelector &&
           fScale == s.fScale;
}

const GrBackendFragmentProcessorFactory& GrDisplacementMapEffect::getFactory() const {
    return GrTBackendFragmentProcessorFactory<GrDisplacementMapEffect>::getInstance();
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

GR_DEFINE_FRAGMENT_PROCESSOR_TEST(GrDisplacementMapEffect);

GrFragmentProcessor* GrDisplacementMapEffect::TestCreate(SkRandom* random,
                                              GrContext*,
                                              const GrDrawTargetCaps&,
                                              GrTexture* textures[]) {
    int texIdxDispl = random->nextBool() ? GrProcessorUnitTest::kSkiaPMTextureIdx :
                                           GrProcessorUnitTest::kAlphaTextureIdx;
    int texIdxColor = random->nextBool() ? GrProcessorUnitTest::kSkiaPMTextureIdx :
                                           GrProcessorUnitTest::kAlphaTextureIdx;
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

GrGLDisplacementMapEffect::GrGLDisplacementMapEffect(const GrBackendProcessorFactory& factory,
                                                     const GrProcessor& proc)
    : INHERITED(factory)
    , fXChannelSelector(proc.cast<GrDisplacementMapEffect>().xChannelSelector())
    , fYChannelSelector(proc.cast<GrDisplacementMapEffect>().yChannelSelector()) {
}

GrGLDisplacementMapEffect::~GrGLDisplacementMapEffect() {
}

void GrGLDisplacementMapEffect::emitCode(GrGLProgramBuilder* builder,
                                         const GrFragmentProcessor&,
                                         const GrProcessorKey& key,
                                         const char* outputColor,
                                         const char* inputColor,
                                         const TransformedCoordsArray& coords,
                                         const TextureSamplerArray& samplers) {
    sk_ignore_unused_variable(inputColor);

    fScaleUni = builder->addUniform(GrGLProgramBuilder::kFragment_Visibility,
                                    kVec2f_GrSLType, "Scale");
    const char* scaleUni = builder->getUniformCStr(fScaleUni);
    const char* dColor = "dColor";
    const char* cCoords = "cCoords";
    const char* outOfBounds = "outOfBounds";
    const char* nearZero = "1e-6"; // Since 6.10352e−5 is the smallest half float, use
                                   // a number smaller than that to approximate 0, but
                                   // leave room for 32-bit float GPU rounding errors.

    GrGLFragmentShaderBuilder* fsBuilder = builder->getFragmentShaderBuilder();
    fsBuilder->codeAppendf("\t\tvec4 %s = ", dColor);
    fsBuilder->appendTextureLookup(samplers[0], coords[0].c_str(), coords[0].getType());
    fsBuilder->codeAppend(";\n");

    // Unpremultiply the displacement
    fsBuilder->codeAppendf("\t\t%s.rgb = (%s.a < %s) ? vec3(0.0) : clamp(%s.rgb / %s.a, 0.0, 1.0);",
                           dColor, dColor, nearZero, dColor, dColor);

    fsBuilder->codeAppendf("\t\tvec2 %s = %s + %s*(%s.",
                           cCoords, coords[1].c_str(), scaleUni, dColor);

    switch (fXChannelSelector) {
      case SkDisplacementMapEffect::kR_ChannelSelectorType:
        fsBuilder->codeAppend("r");
        break;
      case SkDisplacementMapEffect::kG_ChannelSelectorType:
        fsBuilder->codeAppend("g");
        break;
      case SkDisplacementMapEffect::kB_ChannelSelectorType:
        fsBuilder->codeAppend("b");
        break;
      case SkDisplacementMapEffect::kA_ChannelSelectorType:
        fsBuilder->codeAppend("a");
        break;
      case SkDisplacementMapEffect::kUnknown_ChannelSelectorType:
      default:
        SkDEBUGFAIL("Unknown X channel selector");
    }

    switch (fYChannelSelector) {
      case SkDisplacementMapEffect::kR_ChannelSelectorType:
        fsBuilder->codeAppend("r");
        break;
      case SkDisplacementMapEffect::kG_ChannelSelectorType:
        fsBuilder->codeAppend("g");
        break;
      case SkDisplacementMapEffect::kB_ChannelSelectorType:
        fsBuilder->codeAppend("b");
        break;
      case SkDisplacementMapEffect::kA_ChannelSelectorType:
        fsBuilder->codeAppend("a");
        break;
      case SkDisplacementMapEffect::kUnknown_ChannelSelectorType:
      default:
        SkDEBUGFAIL("Unknown Y channel selector");
    }
    fsBuilder->codeAppend("-vec2(0.5));\t\t");

    // FIXME : This can be achieved with a "clamp to border" texture repeat mode and
    //         a 0 border color instead of computing if cCoords is out of bounds here.
    fsBuilder->codeAppendf(
        "bool %s = (%s.x < 0.0) || (%s.y < 0.0) || (%s.x > 1.0) || (%s.y > 1.0);\t\t",
        outOfBounds, cCoords, cCoords, cCoords, cCoords);
    fsBuilder->codeAppendf("%s = %s ? vec4(0.0) : ", outputColor, outOfBounds);
    fsBuilder->appendTextureLookup(samplers[1], cCoords, coords[1].getType());
    fsBuilder->codeAppend(";\n");
}

void GrGLDisplacementMapEffect::setData(const GrGLProgramDataManager& pdman,
                                        const GrProcessor& proc) {
    const GrDisplacementMapEffect& displacementMap = proc.cast<GrDisplacementMapEffect>();
    GrTexture* colorTex = displacementMap.texture(1);
    SkScalar scaleX = SkScalarDiv(displacementMap.scale().fX, SkIntToScalar(colorTex->width()));
    SkScalar scaleY = SkScalarDiv(displacementMap.scale().fY, SkIntToScalar(colorTex->height()));
    pdman.set2f(fScaleUni, SkScalarToFloat(scaleX),
                colorTex->origin() == kTopLeft_GrSurfaceOrigin ?
                SkScalarToFloat(scaleY) : SkScalarToFloat(-scaleY));
}

void GrGLDisplacementMapEffect::GenKey(const GrProcessor& proc,
                                       const GrGLCaps&, GrProcessorKeyBuilder* b) {
    const GrDisplacementMapEffect& displacementMap = proc.cast<GrDisplacementMapEffect>();

    uint32_t xKey = displacementMap.xChannelSelector();
    uint32_t yKey = displacementMap.yChannelSelector() << kChannelSelectorKeyBits;

    b->add32(xKey | yKey);
}
#endif
