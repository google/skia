/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bench/Benchmark.h"

#include "include/core/SkDeferredDisplayListRecorder.h"
#include "include/core/SkSurfaceCharacterization.h"

static SkSurfaceCharacterization create_characterization(GrContext* context) {
    size_t maxResourceBytes = context->getResourceCacheLimit();

    if (!context->colorTypeSupportedAsSurface(kRGBA_8888_SkColorType)) {
        return SkSurfaceCharacterization();
    }

    SkImageInfo ii = SkImageInfo::Make(32, 32, kRGBA_8888_SkColorType,
                                       kPremul_SkAlphaType, nullptr);

    GrBackendFormat backendFormat = context->defaultBackendFormat(kRGBA_8888_SkColorType,
                                                                  GrRenderable::kYes);
    if (!backendFormat.isValid()) {
        return SkSurfaceCharacterization();
    }

    SkSurfaceProps props(0x0, kUnknown_SkPixelGeometry);

    SkSurfaceCharacterization c = context->threadSafeProxy()->createCharacterization(
                                                        maxResourceBytes, ii, backendFormat, 1,
                                                        kTopLeft_GrSurfaceOrigin, props, false);
    return c;
}

// This benchmark tries to simulate how Viz is using SkDDLRecorders.
// For each config it will create a single DDLRecorder which it reuses for all the runs
// For each run it creates a DDL and stores it for later deletion.
class DDLRecorderBench : public Benchmark {
public:
    DDLRecorderBench() { }

protected:
    const char* onGetName() override { return "DDLRecorder"; }

    void onDraw(int loops, SkCanvas* origCanvas) override {
        if (!fRecorder) {
            return;
        }

        SkASSERT(!fDDLs.size());
        fDDLs.reserve(loops);

        for (int i = 0; i < loops; ++i) {
            SkCanvas* recordingCanvas = fRecorder->getCanvas();

            recordingCanvas->drawRect(SkRect::MakeWH(32, 32), SkPaint());

            fDDLs.emplace_back(fRecorder->detach());
        }
    }

private:
    // We create one DDLRecorder for all the timing runs and just keep reusing it
    void onPerCanvasPreDraw(SkCanvas* origCanvas) override {
        GrContext* context = origCanvas->getGrContext();
        if (!context) {
            return;
        }

        SkSurfaceCharacterization c = create_characterization(context);

        fRecorder.reset(new SkDeferredDisplayListRecorder(c));
    }

    // We defer the clean up of the DDLs so it is done outside of the timing loop
    void onPostDraw(SkCanvas*) override {
        fDDLs.clear();
    }

    std::unique_ptr<SkDeferredDisplayListRecorder>      fRecorder = nullptr;
    std::vector<std::unique_ptr<SkDeferredDisplayList>> fDDLs;

    typedef Benchmark INHERITED;
};

DEF_BENCH(return new DDLRecorderBench();)
