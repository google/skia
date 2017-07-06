/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkArithmeticImageFilter.h"
#include "SkCanvas.h"
#include "SkColorSpaceXformer.h"
#include "SkNx.h"
#include "SkReadBuffer.h"
#include "SkSpecialImage.h"
#include "SkSpecialSurface.h"
#include "SkWriteBuffer.h"
#include "SkXfermodeImageFilter.h"
#if SK_SUPPORT_GPU
#include "GrClip.h"
#include "GrContext.h"
#include "GrRenderTargetContext.h"
#include "GrTextureProxy.h"
#include "SkGr.h"
#include "effects/GrConstColorProcessor.h"
#include "effects/GrTextureDomain.h"
#include "glsl/GrGLSLFragmentProcessor.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"
#include "glsl/GrGLSLProgramDataManager.h"
#include "glsl/GrGLSLUniformHandler.h"
#endif

class ArithmeticImageFilterImpl : public SkImageFilter {
public:
    ArithmeticImageFilterImpl(float k1, float k2, float k3, float k4, bool enforcePMColor,
                              sk_sp<SkImageFilter> inputs[2], const CropRect* cropRect)
            : INHERITED(inputs, 2, cropRect), fK{k1, k2, k3, k4}, fEnforcePMColor(enforcePMColor) {}

    SK_TO_STRING_OVERRIDE()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(ArithmeticImageFilterImpl)

protected:
    sk_sp<SkSpecialImage> onFilterImage(SkSpecialImage* source, const Context&,
                                        SkIPoint* offset) const override;

#if SK_SUPPORT_GPU
    sk_sp<SkSpecialImage> filterImageGPU(SkSpecialImage* source,
                                         sk_sp<SkSpecialImage> background,
                                         const SkIPoint& backgroundOffset,
                                         sk_sp<SkSpecialImage> foreground,
                                         const SkIPoint& foregroundOffset,
                                         const SkIRect& bounds,
                                         const OutputProperties& outputProperties) const;
#endif

    void flatten(SkWriteBuffer& buffer) const override {
        this->INHERITED::flatten(buffer);
        for (int i = 0; i < 4; ++i) {
            buffer.writeScalar(fK[i]);
        }
        buffer.writeBool(fEnforcePMColor);
    }

    void drawForeground(SkCanvas* canvas, SkSpecialImage*, const SkIRect&) const;

    sk_sp<SkImageFilter> onMakeColorSpace(SkColorSpaceXformer*) const override;

private:
    const float fK[4];
    const bool fEnforcePMColor;

    friend class ::SkArithmeticImageFilter;

    typedef SkImageFilter INHERITED;
};

sk_sp<SkFlattenable> ArithmeticImageFilterImpl::CreateProc(SkReadBuffer& buffer) {
    SK_IMAGEFILTER_UNFLATTEN_COMMON(common, 2);
    float k[4];
    for (int i = 0; i < 4; ++i) {
        k[i] = buffer.readScalar();
    }
    const bool enforcePMColor = buffer.readBool();
    return SkArithmeticImageFilter::Make(k[0], k[1], k[2], k[3], enforcePMColor, common.getInput(0),
                                         common.getInput(1), &common.cropRect());
}

static Sk4f pin(float min, const Sk4f& val, float max) {
    return Sk4f::Max(min, Sk4f::Min(val, max));
}

template <bool EnforcePMColor>
void arith_span(const float k[], SkPMColor dst[], const SkPMColor src[], int count) {
    const Sk4f k1 = k[0] * (1/255.0f),
               k2 = k[1],
               k3 = k[2],
               k4 = k[3] * 255.0f + 0.5f;

    for (int i = 0; i < count; i++) {
        Sk4f s = SkNx_cast<float>(Sk4b::Load(src+i)),
             d = SkNx_cast<float>(Sk4b::Load(dst+i)),
             r = pin(0, k1*s*d + k2*s + k3*d + k4, 255);
        if (EnforcePMColor) {
            Sk4f a = SkNx_shuffle<3,3,3,3>(r);
            r = Sk4f::Min(a, r);
        }
        SkNx_cast<uint8_t>(r).store(dst+i);
    }
}

// apply mode to src==transparent (0)
template<bool EnforcePMColor> void arith_transparent(const float k[], SkPMColor dst[], int count) {
    const Sk4f k3 = k[2],
               k4 = k[3] * 255.0f + 0.5f;

    for (int i = 0; i < count; i++) {
        Sk4f d = SkNx_cast<float>(Sk4b::Load(dst+i)),
             r = pin(0, k3*d + k4, 255);
        if (EnforcePMColor) {
            Sk4f a = SkNx_shuffle<3,3,3,3>(r);
            r = Sk4f::Min(a, r);
        }
        SkNx_cast<uint8_t>(r).store(dst+i);
    }
}

static bool intersect(SkPixmap* dst, SkPixmap* src, int srcDx, int srcDy) {
    SkIRect dstR = SkIRect::MakeWH(dst->width(), dst->height());
    SkIRect srcR = SkIRect::MakeXYWH(srcDx, srcDy, src->width(), src->height());
    SkIRect sect;
    if (!sect.intersect(dstR, srcR)) {
        return false;
    }
    *dst = SkPixmap(dst->info().makeWH(sect.width(), sect.height()),
                    dst->addr(sect.fLeft, sect.fTop),
                    dst->rowBytes());
    *src = SkPixmap(src->info().makeWH(sect.width(), sect.height()),
                    src->addr(SkTMax(0, -srcDx), SkTMax(0, -srcDy)),
                    src->rowBytes());
    return true;
}

sk_sp<SkSpecialImage> ArithmeticImageFilterImpl::onFilterImage(SkSpecialImage* source,
                                                               const Context& ctx,
                                                               SkIPoint* offset) const {
    SkIPoint backgroundOffset = SkIPoint::Make(0, 0);
    sk_sp<SkSpecialImage> background(this->filterInput(0, source, ctx, &backgroundOffset));

    SkIPoint foregroundOffset = SkIPoint::Make(0, 0);
    sk_sp<SkSpecialImage> foreground(this->filterInput(1, source, ctx, &foregroundOffset));

    SkIRect foregroundBounds = SkIRect::EmptyIRect();
    if (foreground) {
        foregroundBounds = SkIRect::MakeXYWH(foregroundOffset.x(), foregroundOffset.y(),
                                             foreground->width(), foreground->height());
    }

    SkIRect srcBounds = SkIRect::EmptyIRect();
    if (background) {
        srcBounds = SkIRect::MakeXYWH(backgroundOffset.x(), backgroundOffset.y(),
                                      background->width(), background->height());
    }

    srcBounds.join(foregroundBounds);
    if (srcBounds.isEmpty()) {
        return nullptr;
    }

    SkIRect bounds;
    if (!this->applyCropRect(ctx, srcBounds, &bounds)) {
        return nullptr;
    }

    offset->fX = bounds.left();
    offset->fY = bounds.top();

#if SK_SUPPORT_GPU
    if (source->isTextureBacked()) {
        return this->filterImageGPU(source, background, backgroundOffset, foreground,
                                    foregroundOffset, bounds, ctx.outputProperties());
    }
#endif

    sk_sp<SkSpecialSurface> surf(source->makeSurface(ctx.outputProperties(), bounds.size()));
    if (!surf) {
        return nullptr;
    }

    SkCanvas* canvas = surf->getCanvas();
    SkASSERT(canvas);

    canvas->clear(0x0);  // can't count on background to fully clear the background
    canvas->translate(SkIntToScalar(-bounds.left()), SkIntToScalar(-bounds.top()));

    if (background) {
        SkPaint paint;
        paint.setBlendMode(SkBlendMode::kSrc);
        background->draw(canvas, SkIntToScalar(backgroundOffset.fX),
                         SkIntToScalar(backgroundOffset.fY), &paint);
    }

    this->drawForeground(canvas, foreground.get(), foregroundBounds);

    return surf->makeImageSnapshot();
}

#if SK_SUPPORT_GPU

namespace {
class ArithmeticFP : public GrFragmentProcessor {
public:
    static sk_sp<GrFragmentProcessor> Make(float k1, float k2, float k3, float k4,
                                           bool enforcePMColor, sk_sp<GrFragmentProcessor> dst) {
        return sk_sp<GrFragmentProcessor>(
                new ArithmeticFP(k1, k2, k3, k4, enforcePMColor, std::move(dst)));
    }

    ~ArithmeticFP() override {}

    const char* name() const override { return "Arithmetic"; }

    SkString dumpInfo() const override {
        SkString str;
        str.appendf("K1: %.2f K2: %.2f K3: %.2f K4: %.2f", fK1, fK2, fK3, fK4);
        return str;
    }

    float k1() const { return fK1; }
    float k2() const { return fK2; }
    float k3() const { return fK3; }
    float k4() const { return fK4; }
    bool enforcePMColor() const { return fEnforcePMColor; }

private:
    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override {
        class GLSLFP : public GrGLSLFragmentProcessor {
        public:
            void emitCode(EmitArgs& args) override {
                const ArithmeticFP& arith = args.fFp.cast<ArithmeticFP>();

                GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;
                SkString dstColor("dstColor");
                this->emitChild(0, &dstColor, args);

                fKUni = args.fUniformHandler->addUniform(kFragment_GrShaderFlag, kVec4f_GrSLType,
                                                         kDefault_GrSLPrecision, "k");
                const char* kUni = args.fUniformHandler->getUniformCStr(fKUni);

                // We don't try to optimize for this case at all
                if (!args.fInputColor) {
                    fragBuilder->codeAppend("const vec4 src = vec4(1);");
                } else {
                    fragBuilder->codeAppendf("vec4 src = %s;", args.fInputColor);
                }

                fragBuilder->codeAppendf("vec4 dst = %s;", dstColor.c_str());
                fragBuilder->codeAppendf("%s = %s.x * src * dst + %s.y * src + %s.z * dst + %s.w;",
                                         args.fOutputColor, kUni, kUni, kUni, kUni);
                fragBuilder->codeAppendf("%s = clamp(%s, 0.0, 1.0);\n", args.fOutputColor,
                                         args.fOutputColor);
                if (arith.fEnforcePMColor) {
                    fragBuilder->codeAppendf("%s.rgb = min(%s.rgb, %s.a);", args.fOutputColor,
                                             args.fOutputColor, args.fOutputColor);
                }
            }

        protected:
            void onSetData(const GrGLSLProgramDataManager& pdman,
                           const GrFragmentProcessor& proc) override {
                const ArithmeticFP& arith = proc.cast<ArithmeticFP>();
                pdman.set4f(fKUni, arith.k1(), arith.k2(), arith.k3(), arith.k4());
            }

        private:
            GrGLSLProgramDataManager::UniformHandle fKUni;
        };
        return new GLSLFP;
    }

    void onGetGLSLProcessorKey(const GrShaderCaps& caps, GrProcessorKeyBuilder* b) const override {
        b->add32(fEnforcePMColor ? 1 : 0);
    }

    bool onIsEqual(const GrFragmentProcessor& fpBase) const override {
        const ArithmeticFP& fp = fpBase.cast<ArithmeticFP>();
        return fK1 == fp.fK1 && fK2 == fp.fK2 && fK3 == fp.fK3 && fK4 == fp.fK4 &&
               fEnforcePMColor == fp.fEnforcePMColor;
    }

    // This could implement the const input -> const output optimization but it's unlikely to help.
    ArithmeticFP(float k1, float k2, float k3, float k4, bool enforcePMColor,
                 sk_sp<GrFragmentProcessor> dst)
            : INHERITED(kNone_OptimizationFlags)
            , fK1(k1)
            , fK2(k2)
            , fK3(k3)
            , fK4(k4)
            , fEnforcePMColor(enforcePMColor) {
        this->initClassID<ArithmeticFP>();
        SkASSERT(dst);
        SkDEBUGCODE(int dstIndex =) this->registerChildProcessor(std::move(dst));
        SkASSERT(0 == dstIndex);
    }

    float fK1, fK2, fK3, fK4;
    bool fEnforcePMColor;

    GR_DECLARE_FRAGMENT_PROCESSOR_TEST
    typedef GrFragmentProcessor INHERITED;
};
}

#if GR_TEST_UTILS
sk_sp<GrFragmentProcessor> ArithmeticFP::TestCreate(GrProcessorTestData* d) {
    float k1 = d->fRandom->nextF();
    float k2 = d->fRandom->nextF();
    float k3 = d->fRandom->nextF();
    float k4 = d->fRandom->nextF();
    bool enforcePMColor = d->fRandom->nextBool();

    sk_sp<GrFragmentProcessor> dst(GrProcessorUnitTest::MakeChildFP(d));
    return ArithmeticFP::Make(k1, k2, k3, k4, enforcePMColor, std::move(dst));
}
#endif

GR_DEFINE_FRAGMENT_PROCESSOR_TEST(ArithmeticFP);

sk_sp<SkSpecialImage> ArithmeticImageFilterImpl::filterImageGPU(
        SkSpecialImage* source,
        sk_sp<SkSpecialImage> background,
        const SkIPoint& backgroundOffset,
        sk_sp<SkSpecialImage> foreground,
        const SkIPoint& foregroundOffset,
        const SkIRect& bounds,
        const OutputProperties& outputProperties) const {
    SkASSERT(source->isTextureBacked());

    GrContext* context = source->getContext();

    sk_sp<GrTextureProxy> backgroundProxy, foregroundProxy;

    if (background) {
        backgroundProxy = background->asTextureProxyRef(context);
    }

    if (foreground) {
        foregroundProxy = foreground->asTextureProxyRef(context);
    }

    GrPaint paint;
    sk_sp<GrFragmentProcessor> bgFP;

    if (backgroundProxy) {
        SkMatrix backgroundMatrix = SkMatrix::MakeTrans(-SkIntToScalar(backgroundOffset.fX),
                                                        -SkIntToScalar(backgroundOffset.fY));
        sk_sp<GrColorSpaceXform> bgXform =
                GrColorSpaceXform::Make(background->getColorSpace(), outputProperties.colorSpace());
        bgFP = GrTextureDomainEffect::Make(
                std::move(backgroundProxy), std::move(bgXform),
                backgroundMatrix, GrTextureDomain::MakeTexelDomain(background->subset()),
                GrTextureDomain::kDecal_Mode, GrSamplerParams::kNone_FilterMode);
    } else {
        bgFP = GrConstColorProcessor::Make(GrColor4f::TransparentBlack(),
                                           GrConstColorProcessor::kIgnore_InputMode);
    }

    if (foregroundProxy) {
        SkMatrix foregroundMatrix = SkMatrix::MakeTrans(-SkIntToScalar(foregroundOffset.fX),
                                                        -SkIntToScalar(foregroundOffset.fY));
        sk_sp<GrColorSpaceXform> fgXform =
                GrColorSpaceXform::Make(foreground->getColorSpace(), outputProperties.colorSpace());
        sk_sp<GrFragmentProcessor> foregroundFP(GrTextureDomainEffect::Make(
                                std::move(foregroundProxy), std::move(fgXform),
                                foregroundMatrix,
                                GrTextureDomain::MakeTexelDomain(foreground->subset()),
                                GrTextureDomain::kDecal_Mode, GrSamplerParams::kNone_FilterMode));
        paint.addColorFragmentProcessor(std::move(foregroundFP));

        sk_sp<GrFragmentProcessor> xferFP =
                ArithmeticFP::Make(fK[0], fK[1], fK[2], fK[3], fEnforcePMColor, std::move(bgFP));

        // A null 'xferFP' here means kSrc_Mode was used in which case we can just proceed
        if (xferFP) {
            paint.addColorFragmentProcessor(std::move(xferFP));
        }
    } else {
        paint.addColorFragmentProcessor(std::move(bgFP));
    }

    paint.setPorterDuffXPFactory(SkBlendMode::kSrc);

    sk_sp<GrRenderTargetContext> renderTargetContext(context->makeDeferredRenderTargetContext(
            SkBackingFit::kApprox, bounds.width(), bounds.height(),
            GrRenderableConfigForColorSpace(outputProperties.colorSpace()),
            sk_ref_sp(outputProperties.colorSpace())));
    if (!renderTargetContext) {
        return nullptr;
    }
    paint.setGammaCorrect(renderTargetContext->isGammaCorrect());

    SkMatrix matrix;
    matrix.setTranslate(SkIntToScalar(-bounds.left()), SkIntToScalar(-bounds.top()));
    renderTargetContext->drawRect(GrNoClip(), std::move(paint), GrAA::kNo, matrix,
                                  SkRect::Make(bounds));

    return SkSpecialImage::MakeDeferredFromGpu(context,
                                               SkIRect::MakeWH(bounds.width(), bounds.height()),
                                               kNeedNewImageUniqueID_SpecialImage,
                                               renderTargetContext->asTextureProxyRef(),
                                               renderTargetContext->refColorSpace());
}
#endif

void ArithmeticImageFilterImpl::drawForeground(SkCanvas* canvas, SkSpecialImage* img,
                                               const SkIRect& fgBounds) const {
    SkPixmap dst;
    if (!canvas->peekPixels(&dst)) {
        return;
    }

    const SkMatrix& ctm = canvas->getTotalMatrix();
    SkASSERT(ctm.getType() <= SkMatrix::kTranslate_Mask);
    const int dx = SkScalarRoundToInt(ctm.getTranslateX());
    const int dy = SkScalarRoundToInt(ctm.getTranslateY());

    if (img) {
        SkBitmap srcBM;
        SkPixmap src;
        if (!img->getROPixels(&srcBM)) {
            return;
        }
        if (!srcBM.peekPixels(&src)) {
            return;
        }

        auto proc = fEnforcePMColor ? arith_span<true> : arith_span<false>;
        SkPixmap tmpDst = dst;
        if (intersect(&tmpDst, &src, fgBounds.fLeft + dx, fgBounds.fTop + dy)) {
            for (int y = 0; y < tmpDst.height(); ++y) {
                proc(fK, tmpDst.writable_addr32(0, y), src.addr32(0, y), tmpDst.width());
            }
        }
    }

    // Now apply the mode with transparent-color to the outside of the fg image
    SkRegion outside(SkIRect::MakeWH(dst.width(), dst.height()));
    outside.op(fgBounds.makeOffset(dx, dy), SkRegion::kDifference_Op);
    auto proc = fEnforcePMColor ? arith_transparent<true> : arith_transparent<false>;
    for (SkRegion::Iterator iter(outside); !iter.done(); iter.next()) {
        const SkIRect r = iter.rect();
        for (int y = r.fTop; y < r.fBottom; ++y) {
            proc(fK, dst.writable_addr32(r.fLeft, y), r.width());
        }
    }
}

sk_sp<SkImageFilter> ArithmeticImageFilterImpl::onMakeColorSpace(SkColorSpaceXformer* xformer)
const {
    SkASSERT(2 == this->countInputs());
    auto background = xformer->apply(this->getInput(0));
    auto foreground = xformer->apply(this->getInput(1));
    if (background.get() != this->getInput(0) || foreground.get() != this->getInput(1)) {
        return SkArithmeticImageFilter::Make(fK[0], fK[1], fK[2], fK[3], fEnforcePMColor,
                                             std::move(background), std::move(foreground),
                                             getCropRectIfSet());
    }
    return this->refMe();
}

#ifndef SK_IGNORE_TO_STRING
void ArithmeticImageFilterImpl::toString(SkString* str) const {
    str->appendf("SkArithmeticImageFilter: (");
    str->appendf("K[]: (%f %f %f %f)", fK[0], fK[1], fK[2], fK[3]);
    if (this->getInput(0)) {
        str->appendf("foreground: (");
        this->getInput(0)->toString(str);
        str->appendf(")");
    }
    if (this->getInput(1)) {
        str->appendf("background: (");
        this->getInput(1)->toString(str);
        str->appendf(")");
    }
    str->append(")");
}
#endif

sk_sp<SkImageFilter> SkArithmeticImageFilter::Make(float k1, float k2, float k3, float k4,
                                                   bool enforcePMColor,
                                                   sk_sp<SkImageFilter> background,
                                                   sk_sp<SkImageFilter> foreground,
                                                   const SkImageFilter::CropRect* crop) {
    if (!SkScalarIsFinite(k1) || !SkScalarIsFinite(k2) || !SkScalarIsFinite(k3) ||
        !SkScalarIsFinite(k4)) {
        return nullptr;
    }

    // are we nearly some other "std" mode?
    int mode = -1;  // illegal mode
    if (SkScalarNearlyZero(k1) && SkScalarNearlyEqual(k2, SK_Scalar1) && SkScalarNearlyZero(k3) &&
        SkScalarNearlyZero(k4)) {
        mode = (int)SkBlendMode::kSrc;
    } else if (SkScalarNearlyZero(k1) && SkScalarNearlyZero(k2) &&
               SkScalarNearlyEqual(k3, SK_Scalar1) && SkScalarNearlyZero(k4)) {
        mode = (int)SkBlendMode::kDst;
    } else if (SkScalarNearlyZero(k1) && SkScalarNearlyZero(k2) && SkScalarNearlyZero(k3) &&
               SkScalarNearlyZero(k4)) {
        mode = (int)SkBlendMode::kClear;
    }
    if (mode >= 0) {
        return SkXfermodeImageFilter::Make((SkBlendMode)mode, std::move(background),
                                           std::move(foreground), crop);
    }

    sk_sp<SkImageFilter> inputs[2] = {std::move(background), std::move(foreground)};
    return sk_sp<SkImageFilter>(
            new ArithmeticImageFilterImpl(k1, k2, k3, k4, enforcePMColor, inputs, crop));
}

///////////////////////////////////////////////////////////////////////////////////////////////////

SK_DEFINE_FLATTENABLE_REGISTRAR_GROUP_START(SkArithmeticImageFilter)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(ArithmeticImageFilterImpl)
SK_DEFINE_FLATTENABLE_REGISTRAR_GROUP_END
