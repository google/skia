/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bench/Benchmark.h"
#include "include/core/SkCanvas.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrDirectContext.h"

using namespace skia_private;

class CreateBackendTextureBench : public Benchmark {
private:
    SkString fName;
    TArray<GrBackendTexture> fBackendTextures;
    skgpu::Mipmapped fMipmapped;

public:
    CreateBackendTextureBench(skgpu::Mipmapped mipmapped) : fMipmapped(mipmapped) {
        fName.printf("create_backend_texture%s",
                     mipmapped == skgpu::Mipmapped::kYes ? "_mipped" : "");
    }

private:
    bool isSuitableFor(Backend backend) override { return kGPU_Backend == backend; }

    const char* onGetName() override { return fName.c_str(); }

    void onDraw(int loops, SkCanvas* canvas) override {
        auto context = canvas->recordingContext()->asDirectContext();

        fBackendTextures.reserve_exact(fBackendTextures.size() + loops);

        static const int kSize = 16;
        for (int i = 0; i < loops; ++i) {
            fBackendTextures.push_back(
                    context->createBackendTexture(kSize,
                                                  kSize,
                                                  kRGBA_8888_SkColorType,
                                                  SkColors::kRed,
                                                  fMipmapped,
                                                  GrRenderable::kNo,
                                                  GrProtected::kNo,
                                                  nullptr,
                                                  nullptr,
                                                  /*label=*/"DrawBackendTextureBench"));
        }
    }

    void onPerCanvasPostDraw(SkCanvas* canvas) override {
        auto context = canvas->recordingContext()->asDirectContext();

        context->flush();
        context->submit(GrSyncCpu::kYes);

        for (int i = 0; i < fBackendTextures.size(); ++i) {
            if (fBackendTextures[i].isValid()) {
                context->deleteBackendTexture(fBackendTextures[i]);
            }
        }
        fBackendTextures.clear();
    }
};

DEF_BENCH(return new CreateBackendTextureBench(skgpu::Mipmapped::kNo);)
DEF_BENCH(return new CreateBackendTextureBench(skgpu::Mipmapped::kYes);)
