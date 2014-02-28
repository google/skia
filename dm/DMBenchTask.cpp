#include "DMBenchTask.h"
#include "DMUtil.h"
#include "SkSurface.h"

namespace DM {

static SkString bench_name(const char* name, const char* config) {
    SkString result("bench ");
    result.appendf("%s_%s", name, config);
    return result;
}

NonRenderingBenchTask::NonRenderingBenchTask(const char* config,
                                             Reporter* reporter,
                                             TaskRunner* tasks,
                                             BenchRegistry::Factory factory)
    : CpuTask(reporter, tasks)
    , fBench(factory(NULL))
    , fName(bench_name(fBench->getName(), config)) {}

CpuBenchTask::CpuBenchTask(const char* config,
                           Reporter* reporter,
                           TaskRunner* tasks,
                           BenchRegistry::Factory factory,
                           SkColorType colorType)
    : CpuTask(reporter, tasks)
    , fBench(factory(NULL))
    , fName(bench_name(fBench->getName(), config))
    , fColorType(colorType) {}

GpuBenchTask::GpuBenchTask(const char* config,
                           Reporter* reporter,
                           TaskRunner* tasks,
                           BenchRegistry::Factory factory,
                           GrContextFactory::GLContextType contextType,
                           int sampleCount)
    : GpuTask(reporter, tasks)
    , fBench(factory(NULL))
    , fName(bench_name(fBench->getName(), config))
    , fContextType(contextType)
    , fSampleCount(sampleCount) {}

bool NonRenderingBenchTask::shouldSkip() const {
    return !fBench->isSuitableFor(SkBenchmark::kNonRendering_Backend);
}

bool CpuBenchTask::shouldSkip() const {
    return !fBench->isSuitableFor(SkBenchmark::kRaster_Backend);
}

bool GpuBenchTask::shouldSkip() const {
    return !fBench->isSuitableFor(SkBenchmark::kGPU_Backend);
}

static void draw_raster(SkBenchmark* bench, SkColorType colorType) {
    SkBitmap bitmap;
    SetupBitmap(colorType, bench, &bitmap);
    SkCanvas canvas(bitmap);

    bench->preDraw();
    bench->draw(1, &canvas);
    bench->postDraw();
}

void NonRenderingBenchTask::draw() {
    draw_raster(fBench.get(), kPMColor_SkColorType);
}

void CpuBenchTask::draw() {
    draw_raster(fBench.get(), fColorType);
}

void GpuBenchTask::draw(GrContextFactory* grFactory) {
    SkImageInfo info = SkImageInfo::Make(fBench->getSize().x(),
                                         fBench->getSize().y(),
                                         kPMColor_SkColorType,
                                         kPremul_SkAlphaType);
    SkAutoTUnref<SkSurface> surface(SkSurface::NewRenderTarget(
            grFactory->get(fContextType), info, fSampleCount));

    fBench->preDraw();
    fBench->draw(1, surface->getCanvas());
    fBench->postDraw();
}

}  // namespace DM
