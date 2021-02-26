/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBitmap.h"
#include "include/core/SkString.h"
#include "include/effects/SkTableColorFilter.h"
#include "include/private/SkColorData.h"
#include "include/private/SkTo.h"
#include "src/core/SkArenaAlloc.h"
#include "src/core/SkColorFilterBase.h"
#include "src/core/SkEffectPriv.h"
#include "src/core/SkRasterPipeline.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkVM.h"
#include "src/core/SkWriteBuffer.h"

class SkTable_ColorFilter : public SkColorFilterBase {
public:
    SkTable_ColorFilter(const uint8_t tableA[], const uint8_t tableR[],
                        const uint8_t tableG[], const uint8_t tableB[]) {
        fBitmap.allocPixels(SkImageInfo::MakeA8(256, 4));
        uint8_t *a = fBitmap.getAddr8(0,0),
                *r = fBitmap.getAddr8(0,1),
                *g = fBitmap.getAddr8(0,2),
                *b = fBitmap.getAddr8(0,3);
        for (int i = 0; i < 256; i++) {
            a[i] = tableA ? tableA[i] : i;
            r[i] = tableR ? tableR[i] : i;
            g[i] = tableG ? tableG[i] : i;
            b[i] = tableB ? tableB[i] : i;
        }
        fBitmap.setImmutable();
    }

#if SK_SUPPORT_GPU
    GrFPResult asFragmentProcessor(std::unique_ptr<GrFragmentProcessor> inputFP,
                                   GrRecordingContext*, const GrColorInfo&) const override;
#endif

    bool onAppendStages(const SkStageRec& rec, bool shaderIsOpaque) const override {
        SkRasterPipeline* p = rec.fPipeline;
        if (!shaderIsOpaque) {
            p->append(SkRasterPipeline::unpremul);
        }

        // Notice fBitmap is in a,r,g,b order, but the byte_tables stage takes r,g,b,a pointers.
        const uint8_t *a = fBitmap.getAddr8(0,0),
                      *r = fBitmap.getAddr8(0,1),
                      *g = fBitmap.getAddr8(0,2),
                      *b = fBitmap.getAddr8(0,3);
        struct Tables { const uint8_t *r, *g, *b, *a; };
        p->append(SkRasterPipeline::byte_tables, rec.fAlloc->make<Tables>(Tables{r,g,b,a}));

        bool definitelyOpaque = shaderIsOpaque && a[0xff] == 0xff;
        if (!definitelyOpaque) {
            p->append(SkRasterPipeline::premul);
        }
        return true;
    }

    skvm::Color onProgram(skvm::Builder* p, skvm::Color c,
                          SkColorSpace* dstCS,
                          skvm::Uniforms* uniforms, SkArenaAlloc*) const override {

        auto apply_table_to_component = [&](skvm::F32 c, const uint8_t* bytePtr) -> skvm::F32 {
            skvm::I32     index = to_unorm(8, clamp01(c));
            skvm::Uniform table = uniforms->pushPtr(bytePtr);
            return from_unorm(8, gather8(table, index));
        };

        c = unpremul(c);
        c.a = apply_table_to_component(c.a, fBitmap.getAddr8(0,0));
        c.r = apply_table_to_component(c.r, fBitmap.getAddr8(0,1));
        c.g = apply_table_to_component(c.g, fBitmap.getAddr8(0,2));
        c.b = apply_table_to_component(c.b, fBitmap.getAddr8(0,3));
        return premul(c);
    }

    void flatten(SkWriteBuffer& buffer) const override {
        buffer.writeByteArray(fBitmap.getAddr8(0,0), 4*256);
    }
    SK_FLATTENABLE_HOOKS(SkTable_ColorFilter)

private:
    SkBitmap fBitmap;
};

sk_sp<SkFlattenable> SkTable_ColorFilter::CreateProc(SkReadBuffer& buffer) {
    uint8_t argb[4*256];
    if (buffer.readByteArray(argb, sizeof(argb))) {
        return SkTableColorFilter::MakeARGB(argb+0*256,
                                            argb+1*256,
                                            argb+2*256,
                                            argb+3*256);
    }
    return nullptr;
}

#if SK_SUPPORT_GPU

#include "include/gpu/GrRecordingContext.h"
#include "src/gpu/GrColorInfo.h"
#include "src/gpu/GrFragmentProcessor.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/SkGr.h"
#include "src/gpu/effects/GrTextureEffect.h"
#include "src/gpu/glsl/GrGLSLFragmentProcessor.h"
#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"

class ColorTableEffect : public GrFragmentProcessor {
public:
    static std::unique_ptr<GrFragmentProcessor> Make(std::unique_ptr<GrFragmentProcessor> inputFP,
                                                     GrRecordingContext* context,
                                                     const SkBitmap& bitmap);

    ~ColorTableEffect() override {}

    const char* name() const override { return "ColorTableEffect"; }

    std::unique_ptr<GrFragmentProcessor> clone() const override {
        return std::unique_ptr<GrFragmentProcessor>(new ColorTableEffect(*this));
    }

    static constexpr int kTexEffectFPIndex = 0;
    static constexpr int kInputFPIndex = 1;

private:
    std::unique_ptr<GrGLSLFragmentProcessor> onMakeProgramImpl() const override;

    void onGetGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder*) const override {}

    bool onIsEqual(const GrFragmentProcessor&) const override { return true; }

    ColorTableEffect(std::unique_ptr<GrFragmentProcessor> inputFP, GrSurfaceProxyView view);

    explicit ColorTableEffect(const ColorTableEffect& that);

    GR_DECLARE_FRAGMENT_PROCESSOR_TEST

    using INHERITED = GrFragmentProcessor;
};

ColorTableEffect::ColorTableEffect(std::unique_ptr<GrFragmentProcessor> inputFP,
                                   GrSurfaceProxyView view)
        // Not bothering with table-specific optimizations.
        : INHERITED(kColorTableEffect_ClassID, kNone_OptimizationFlags) {
    this->registerChild(GrTextureEffect::Make(std::move(view), kUnknown_SkAlphaType),
                        SkSL::SampleUsage::Explicit());
    this->registerChild(std::move(inputFP));
}

ColorTableEffect::ColorTableEffect(const ColorTableEffect& that)
        : INHERITED(kColorTableEffect_ClassID, that.optimizationFlags()) {
    this->cloneAndRegisterAllChildProcessors(that);
}

std::unique_ptr<GrGLSLFragmentProcessor> ColorTableEffect::onMakeProgramImpl() const {
    class Impl : public GrGLSLFragmentProcessor {
    public:
        void emitCode(EmitArgs& args) override {
            GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;
            SkString inputColor = this->invokeChild(kInputFPIndex, args);
            SkString a = this->invokeChild(kTexEffectFPIndex, args, "half2(coord.a, 0.5)");
            SkString r = this->invokeChild(kTexEffectFPIndex, args, "half2(coord.r, 1.5)");
            SkString g = this->invokeChild(kTexEffectFPIndex, args, "half2(coord.g, 2.5)");
            SkString b = this->invokeChild(kTexEffectFPIndex, args, "half2(coord.b, 3.5)");
            fragBuilder->codeAppendf(
                    "half4 coord = 255 * unpremul(%s) + 0.5;\n"
                    "half4 color = half4(%s.a, %s.a, %s.a, 1);\n"
                    "return color * %s.a;\n",
                    inputColor.c_str(), r.c_str(), g.c_str(), b.c_str(), a.c_str());
        }
    };
    return std::make_unique<Impl>();
}

std::unique_ptr<GrFragmentProcessor> ColorTableEffect::Make(
        std::unique_ptr<GrFragmentProcessor> inputFP,
        GrRecordingContext* context, const SkBitmap& bitmap) {
    SkASSERT(kPremul_SkAlphaType == bitmap.alphaType());
    SkASSERT(bitmap.isImmutable());

    auto view = GrMakeCachedBitmapProxyView(context, bitmap);
    if (!view) {
        return nullptr;
    }

    return std::unique_ptr<GrFragmentProcessor>(
               new ColorTableEffect(std::move(inputFP), std::move(view)));
}

///////////////////////////////////////////////////////////////////////////////

GR_DEFINE_FRAGMENT_PROCESSOR_TEST(ColorTableEffect);

#if GR_TEST_UTILS


std::unique_ptr<GrFragmentProcessor> ColorTableEffect::TestCreate(GrProcessorTestData* d) {
    int flags = 0;
    uint8_t luts[256][4];
    do {
        for (int i = 0; i < 4; ++i) {
            flags |= d->fRandom->nextBool() ? (1  << i): 0;
        }
    } while (!flags);
    for (int i = 0; i < 4; ++i) {
        if (flags & (1 << i)) {
            for (int j = 0; j < 256; ++j) {
                luts[j][i] = SkToU8(d->fRandom->nextBits(8));
            }
        }
    }
    auto filter(SkTableColorFilter::MakeARGB(
        (flags & (1 << 0)) ? luts[0] : nullptr,
        (flags & (1 << 1)) ? luts[1] : nullptr,
        (flags & (1 << 2)) ? luts[2] : nullptr,
        (flags & (1 << 3)) ? luts[3] : nullptr
    ));
    sk_sp<SkColorSpace> colorSpace = GrTest::TestColorSpace(d->fRandom);
    auto [success, fp] = as_CFB(filter)->asFragmentProcessor(
            d->inputFP(), d->context(),
            GrColorInfo(GrColorType::kRGBA_8888, kUnknown_SkAlphaType, std::move(colorSpace)));
    SkASSERT(success);
    return std::move(fp);
}
#endif

GrFPResult SkTable_ColorFilter::asFragmentProcessor(std::unique_ptr<GrFragmentProcessor> inputFP,
                                                    GrRecordingContext* context,
                                                    const GrColorInfo&) const {
    return GrFPSuccess(ColorTableEffect::Make(std::move(inputFP), context, fBitmap));
}

#endif // SK_SUPPORT_GPU

///////////////////////////////////////////////////////////////////////////////

sk_sp<SkColorFilter> SkTableColorFilter::Make(const uint8_t table[256]) {
    return sk_make_sp<SkTable_ColorFilter>(table, table, table, table);
}

sk_sp<SkColorFilter> SkTableColorFilter::MakeARGB(const uint8_t tableA[256],
                                                  const uint8_t tableR[256],
                                                  const uint8_t tableG[256],
                                                  const uint8_t tableB[256]) {
    if (!tableA && !tableR && !tableG && !tableB) {
        return nullptr;
    }

    return sk_make_sp<SkTable_ColorFilter>(tableA, tableR, tableG, tableB);
}

void SkTableColorFilter::RegisterFlattenables() { SK_REGISTER_FLATTENABLE(SkTable_ColorFilter); }
