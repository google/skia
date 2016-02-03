/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 */

#include <VisualBench/VisualBenchmarkStream.h>
#include <VisualBench/WrappedBenchmark.h>
#include "GMBench.h"
#include "SkOSFile.h"
#include "SkPath.h"
#include "SkPictureRecorder.h"
#include "SkStream.h"
#include "sk_tool_utils.h"
#include "VisualFlags.h"
#include "VisualSKPBench.h"

#if SK_SUPPORT_GPU
#include "GrContext.h"
#endif

DEFINE_string2(match, m, nullptr,
               "[~][^]substring[$] [...] of bench name to run.\n"
               "Multiple matches may be separated by spaces.\n"
               "~ causes a matching bench to always be skipped\n"
               "^ requires the start of the bench to match\n"
               "$ requires the end of the bench to match\n"
               "^ and $ requires an exact match\n"
               "If a bench does not match any list entry,\n"
               "it is skipped unless some list entry starts with ~");
DEFINE_string(skps, "skps", "Directory to read skps from.");
DEFINE_bool(warmup, true, "Include a warmup bench? (Excluding the warmup may compromise results)");

// We draw a big nonAA path to warmup the gpu / cpu
#include "SkPerlinNoiseShader.h"
class WarmupBench : public Benchmark {
public:
    WarmupBench() {
        sk_tool_utils::make_big_path(fPath);
        fPerlinRect = SkRect::MakeLTRB(0., 0., 400., 400.);
    }
private:
    const char* onGetName() override { return "warmupbench"; }
    SkIPoint onGetSize() override {
        int w = SkScalarCeilToInt(SkTMax(fPath.getBounds().right(), fPerlinRect.right()));
        int h = SkScalarCeilToInt(SkTMax(fPath.getBounds().bottom(), fPerlinRect.bottom()));
        return SkIPoint::Make(w, h);
    }
    void onDraw(int loops, SkCanvas* canvas) override {
        // We draw a big path to warm up the cpu, and then use perlin noise shader to warm up the
        // gpu
        SkPaint paint;
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setStrokeWidth(2);

        SkPaint perlinPaint;
        perlinPaint.setShader(SkPerlinNoiseShader::CreateTurbulence(0.1f, 0.1f, 1, 0,
                                                                    nullptr))->unref();
        for (int i = 0; i < loops; i++) {
            canvas->drawPath(fPath, paint);
            canvas->drawRect(fPerlinRect, perlinPaint);
#if SK_SUPPORT_GPU
            // Ensure the GrContext doesn't batch across draw loops.
            if (GrContext* context = canvas->getGrContext()) {
                context->flush();
            }
#endif
        }
    }
    SkPath fPath;
    SkRect fPerlinRect;
};

VisualBenchmarkStream::VisualBenchmarkStream(const SkSurfaceProps& surfaceProps, bool justSKP)
    : fSurfaceProps(surfaceProps)
    , fBenches(BenchRegistry::Head())
    , fGMs(skiagm::GMRegistry::Head())
    , fSourceType(nullptr)
    , fBenchType(nullptr)
    , fCurrentSKP(0)
    , fIsWarmedUp(false) {
    for (int i = 0; i < FLAGS_skps.count(); i++) {
        if (SkStrEndsWith(FLAGS_skps[i], ".skp")) {
            fSKPs.push_back() = FLAGS_skps[i];
        } else {
            SkOSFile::Iter it(FLAGS_skps[i], ".skp");
            SkString path;
            while (it.next(&path)) {
                fSKPs.push_back() = SkOSPath::Join(FLAGS_skps[0], path.c_str());
            }
        }
    }

    if (justSKP) {
       fGMs = nullptr;
       fBenches = nullptr;
    }

    // seed with an initial benchmark
    // NOTE the initial benchmark will not have preTimingHooks called, but that is okay because
    // it is the warmupbench
    this->next();
}

bool VisualBenchmarkStream::ReadPicture(const char* path, SkAutoTUnref<SkPicture>* pic) {
    // Not strictly necessary, as it will be checked again later,
    // but helps to avoid a lot of pointless work if we're going to skip it.
    if (SkCommandLineFlags::ShouldSkip(FLAGS_match, path)) {
        return false;
    }

    SkAutoTDelete<SkStream> stream(SkStream::NewFromFile(path));
    if (stream.get() == nullptr) {
        SkDebugf("Could not read %s.\n", path);
        return false;
    }

    pic->reset(SkPicture::CreateFromStream(stream.get()));
    if (pic->get() == nullptr) {
        SkDebugf("Could not read %s as an SkPicture.\n", path);
        return false;
    }
    return true;
}

Benchmark* VisualBenchmarkStream::next() {
    Benchmark* bench;
    if (FLAGS_warmup && !fIsWarmedUp) {
        fIsWarmedUp = true;
        bench = new WarmupBench;
    } else {
        // skips non matching benches
        while ((bench = this->innerNext()) &&
               (SkCommandLineFlags::ShouldSkip(FLAGS_match, bench->getUniqueName()) ||
                !bench->isSuitableFor(Benchmark::kGPU_Backend))) {
            bench->unref();
        }
    }

    // TODO move this all to --config
    if (bench && FLAGS_cpu) {
        bench = new CpuWrappedBenchmark(fSurfaceProps, bench);
    } else if (bench && FLAGS_offscreen) {
        bench = new GpuWrappedBenchmark(fSurfaceProps, bench, FLAGS_msaa);
    }

    fBenchmark.reset(bench);
    return fBenchmark;
}

Benchmark* VisualBenchmarkStream::innerNext() {
    while (fBenches) {
        Benchmark* bench = fBenches->factory()(nullptr);
        fBenches = fBenches->next();
        if (bench->isVisual()) {
            fSourceType = "bench";
            fBenchType  = "micro";
            return bench;
        }
        bench->unref();
    }

    while (fGMs) {
        SkAutoTDelete<skiagm::GM> gm(fGMs->factory()(nullptr));
        fGMs = fGMs->next();
        if (gm->runAsBench()) {
            fSourceType = "gm";
            fBenchType  = "micro";
            return new GMBench(gm.detach());
        }
    }

    // Render skps
    while (fCurrentSKP < fSKPs.count()) {
        const SkString& path = fSKPs[fCurrentSKP++];
        SkAutoTUnref<SkPicture> pic;
        if (!ReadPicture(path.c_str(), &pic)) {
            continue;
        }

        SkString name = SkOSPath::Basename(path.c_str());
        fSourceType = "skp";
        fBenchType = "playback";
        return new VisualSKPBench(name.c_str(), pic.get());
    }

    return nullptr;
}
