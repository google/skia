/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkAlphaType.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkFlattenable.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkString.h"
#include "include/core/SkTypes.h"
#include "include/private/SkSLSampleUsage.h"
#include "src/base/SkArenaAlloc.h"
#include "src/core/SkColorFilterBase.h"
#include "src/core/SkEffectPriv.h"
#include "src/core/SkRasterPipeline.h"
#include "src/core/SkRasterPipelineOpContexts.h"
#include "src/core/SkRasterPipelineOpList.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkWriteBuffer.h"

#include <cstdint>
#include <memory>
#include <tuple>
#include <utility>

#ifdef SK_GRAPHITE_ENABLED
#include "src/gpu/graphite/Image_Graphite.h"
#include "src/gpu/graphite/KeyContext.h"
#include "src/gpu/graphite/KeyHelpers.h"
#include "src/gpu/graphite/PaintParamsKey.h"

namespace skgpu::graphite {
class PipelineDataGatherer;
}
#endif

#if SK_SUPPORT_GPU
#include "include/gpu/GpuTypes.h"
#include "include/gpu/GrTypes.h"
#include "src/gpu/ganesh/GrColorInfo.h"
#include "src/gpu/ganesh/GrFragmentProcessor.h"
#include "src/gpu/ganesh/GrProcessorUnitTest.h"
#include "src/gpu/ganesh/GrSurfaceProxyView.h"
#include "src/gpu/ganesh/SkGr.h"
#include "src/gpu/ganesh/effects/GrTextureEffect.h"
#include "src/gpu/ganesh/glsl/GrGLSLFragmentShaderBuilder.h"

class GrRecordingContext;
struct GrShaderCaps;
namespace skgpu { class KeyBuilder; }
#endif

#if GR_TEST_UTILS
#include "include/core/SkColorSpace.h"
#include "include/core/SkSurfaceProps.h"
#include "include/private/base/SkTo.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/base/SkRandom.h"
#include "src/gpu/ganesh/GrTestUtils.h"
#else
class SkSurfaceProps;
#endif

#if defined(SK_ENABLE_SKSL)
#include "src/core/SkVM.h"
#endif

class SkTable_ColorFilter final : public SkColorFilterBase {
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
                                   GrRecordingContext*, const GrColorInfo&,
                                   const SkSurfaceProps&) const override;
#endif

#ifdef SK_GRAPHITE_ENABLED
    void addToKey(const skgpu::graphite::KeyContext&,
                  skgpu::graphite::PaintParamsKeyBuilder*,
                  skgpu::graphite::PipelineDataGatherer*) const override;
#endif

    bool appendStages(const SkStageRec& rec, bool shaderIsOpaque) const override {
        SkRasterPipeline* p = rec.fPipeline;
        if (!shaderIsOpaque) {
            p->append(SkRasterPipelineOp::unpremul);
        }

        SkRasterPipeline_TablesCtx* tables = rec.fAlloc->make<SkRasterPipeline_TablesCtx>();
        tables->a = fBitmap.getAddr8(0, 0);
        tables->r = fBitmap.getAddr8(0, 1);
        tables->g = fBitmap.getAddr8(0, 2);
        tables->b = fBitmap.getAddr8(0, 3);
        p->append(SkRasterPipelineOp::byte_tables, tables);

        bool definitelyOpaque = shaderIsOpaque && tables->a[0xff] == 0xff;
        if (!definitelyOpaque) {
            p->append(SkRasterPipelineOp::premul);
        }
        return true;
    }

    skvm::Color onProgram(skvm::Builder* p, skvm::Color c,
                          const SkColorInfo& dst,
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

private:
    friend void ::SkRegisterTableColorFilterFlattenable();
    SK_FLATTENABLE_HOOKS(SkTable_ColorFilter)

    SkBitmap fBitmap;
};

sk_sp<SkFlattenable> SkTable_ColorFilter::CreateProc(SkReadBuffer& buffer) {
    uint8_t argb[4*256];
    if (buffer.readByteArray(argb, sizeof(argb))) {
        return SkColorFilters::TableARGB(argb+0*256, argb+1*256, argb+2*256, argb+3*256);
    }
    return nullptr;
}

#if SK_SUPPORT_GPU

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

    inline static constexpr int kTexEffectFPIndex = 0;
    inline static constexpr int kInputFPIndex = 1;

private:
    std::unique_ptr<ProgramImpl> onMakeProgramImpl() const override;

    void onAddToKey(const GrShaderCaps&, skgpu::KeyBuilder*) const override {}

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
        : INHERITED(that) {}

std::unique_ptr<GrFragmentProcessor::ProgramImpl> ColorTableEffect::onMakeProgramImpl() const {
    class Impl : public ProgramImpl {
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

    auto view = std::get<0>(GrMakeCachedBitmapProxyView(context,
                                                        bitmap,
                                                        /*label=*/"MakeColorTableEffect",
                                                        GrMipmapped::kNo));
    if (!view) {
        return nullptr;
    }

    return std::unique_ptr<GrFragmentProcessor>(new ColorTableEffect(std::move(inputFP),
                                                                     std::move(view)));
}

///////////////////////////////////////////////////////////////////////////////

GR_DEFINE_FRAGMENT_PROCESSOR_TEST(ColorTableEffect)

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
    auto filter(SkColorFilters::TableARGB(
        (flags & (1 << 0)) ? luts[0] : nullptr,
        (flags & (1 << 1)) ? luts[1] : nullptr,
        (flags & (1 << 2)) ? luts[2] : nullptr,
        (flags & (1 << 3)) ? luts[3] : nullptr
    ));
    sk_sp<SkColorSpace> colorSpace = GrTest::TestColorSpace(d->fRandom);
    SkSurfaceProps props; // default props for testing
    auto [success, fp] = as_CFB(filter)->asFragmentProcessor(
            d->inputFP(), d->context(),
            GrColorInfo(GrColorType::kRGBA_8888, kUnknown_SkAlphaType, std::move(colorSpace)),
            props);
    SkASSERT(success);
    return std::move(fp);
}
#endif

GrFPResult SkTable_ColorFilter::asFragmentProcessor(std::unique_ptr<GrFragmentProcessor> inputFP,
                                                    GrRecordingContext* context,
                                                    const GrColorInfo&,
                                                    const SkSurfaceProps&) const {
    auto cte = ColorTableEffect::Make(std::move(inputFP), context, fBitmap);
    return cte ? GrFPSuccess(std::move(cte)) : GrFPFailure(nullptr);
}

#endif // SK_SUPPORT_GPU

#ifdef SK_GRAPHITE_ENABLED

void SkTable_ColorFilter::addToKey(const skgpu::graphite::KeyContext& keyContext,
                                   skgpu::graphite::PaintParamsKeyBuilder* builder,
                                   skgpu::graphite::PipelineDataGatherer* gatherer) const {
    using namespace skgpu::graphite;

    TableColorFilterBlock::TableColorFilterData data;

    // TODO(b/239604347): remove this hack. This is just here until we determine what Graphite's
    // Recorder-level caching story is going to be.
    sk_sp<SkImage> image = SkImage::MakeFromBitmap(fBitmap);
    image = image->makeTextureImage(keyContext.recorder(), { skgpu::Mipmapped::kNo });

    if (as_IB(image)->isGraphiteBacked()) {
        skgpu::graphite::Image* grImage = static_cast<skgpu::graphite::Image*>(image.get());

        auto [view, _] = grImage->asView(keyContext.recorder(), skgpu::Mipmapped::kNo);
        data.fTextureProxy = view.refProxy();
    }

    TableColorFilterBlock::BeginBlock(keyContext, builder, gatherer, data);
    builder->endBlock();
}

#endif

///////////////////////////////////////////////////////////////////////////////

sk_sp<SkColorFilter> SkColorFilters::Table(const uint8_t table[256]) {
    return sk_make_sp<SkTable_ColorFilter>(table, table, table, table);
}

sk_sp<SkColorFilter> SkColorFilters::TableARGB(const uint8_t tableA[256],
                                               const uint8_t tableR[256],
                                               const uint8_t tableG[256],
                                               const uint8_t tableB[256]) {
    if (!tableA && !tableR && !tableG && !tableB) {
        return nullptr;
    }

    return sk_make_sp<SkTable_ColorFilter>(tableA, tableR, tableG, tableB);
}

void SkRegisterTableColorFilterFlattenable() {
    SK_REGISTER_FLATTENABLE(SkTable_ColorFilter);
}
