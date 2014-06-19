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
    return !fBench->isSuitableFor(Benchmark::kNonRendering_Backend);
}

bool CpuBenchTask::shouldSkip() const {
    return !fBench->isSuitableFor(Benchmark::kRaster_Backend);
}

bool GpuBenchTask::shouldSkip() const {
    return kGPUDisabled || !fBench->isSuitableFor(Benchmark::kGPU_Backend);
}

static void draw_raster(Benchmark* bench, SkColorType colorType) {
    SkBitmap bitmap;
    AllocatePixels(colorType, bench->getSize().x(), bench->getSize().y(), &bitmap);
    SkCanvas canvas(bitmap);

    bench->preDraw();
    bench->draw(1, &canvas);
}

void NonRenderingBenchTask::draw() {
    draw_raster(fBench.get(), kN32_SkColorType);
}

void CpuBenchTask::draw() {
    draw_raster(fBench.get(), fColorType);
}

void GpuBenchTask::draw(GrContextFactory* grFactory) {
    SkImageInfo info = SkImageInfo::Make(fBench->getSize().x(),
                                         fBench->getSize().y(),
                                         kN32_SkColorType,
                                         kPremul_SkAlphaType);
    SkAutoTUnref<SkSurface> surface(NewGpuSurface(grFactory, fContextType, info, fSampleCount));

    fBench->preDraw();
    fBench->draw(1, surface->getCanvas());
}

}  // namespace DM
