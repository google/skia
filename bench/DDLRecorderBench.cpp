/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <memory>

#include "bench/Benchmark.h"

#include "include/core/SkCanvas.h"
#include "include/gpu/GrDirectContext.h"
#include "include/private/chromium/GrDeferredDisplayListRecorder.h"
#include "include/private/chromium/GrSurfaceCharacterization.h"

static GrSurfaceCharacterization create_characterization(GrDirectContext* direct) {
    size_t maxResourceBytes = direct->getResourceCacheLimit();

    if (!direct->colorTypeSupportedAsSurface(kRGBA_8888_SkColorType)) {
        return GrSurfaceCharacterization();
    }

    SkImageInfo ii = SkImageInfo::Make(32, 32, kRGBA_8888_SkColorType,
                                       kPremul_SkAlphaType, nullptr);

    GrBackendFormat backendFormat = direct->defaultBackendFormat(kRGBA_8888_SkColorType,
                                                                 GrRenderable::kYes);
    if (!backendFormat.isValid()) {
        return GrSurfaceCharacterization();
    }

    SkSurfaceProps props(0x0, kUnknown_SkPixelGeometry);

    GrSurfaceCharacterization c =
            direct->threadSafeProxy()->createCharacterization(maxResourceBytes,
                                                              ii,
                                                              backendFormat,
                                                              1,
                                                              kTopLeft_GrSurfaceOrigin,
                                                              props,
                                                              skgpu::Mipmapped::kNo);
    return c;
}

// This benchmark tries to simulate how Viz is using SkDDLRecorders.
// For each config it will create a single DDLRecorder which it reuses for all the runs
// For each run it creates a DDL and stores it for later deletion.
class DDLRecorderBench : public Benchmark {
public:
    DDLRecorderBench() { }

protected:
    bool isSuitableFor(Backend backend) override { return Backend::kGanesh == backend; }

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
        auto context = origCanvas->recordingContext()->asDirectContext();
        if (!context) {
            return;
        }

        GrSurfaceCharacterization c = create_characterization(context);

        fRecorder = std::make_unique<GrDeferredDisplayListRecorder>(c);
    }

    // We defer the clean up of the DDLs so it is done outside of the timing loop
    void onPostDraw(SkCanvas*) override {
        fDDLs.clear();
    }

    std::unique_ptr<GrDeferredDisplayListRecorder>      fRecorder = nullptr;
    std::vector<sk_sp<GrDeferredDisplayList>>           fDDLs;

    using INHERITED = Benchmark;
};

DEF_BENCH(return new DDLRecorderBench();)
