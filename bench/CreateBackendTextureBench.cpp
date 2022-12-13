/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bench/Benchmark.h"
#include "include/core/SkCanvas.h"
#include "include/gpu/GrDirectContext.h"

class CreateBackendTextureBench : public Benchmark {
private:
    SkString fName;
    SkTArray<GrBackendTexture> fBackendTextures;
    GrMipmapped fMipmapped;

public:
    CreateBackendTextureBench(GrMipmapped mipmapped) : fMipmapped(mipmapped) {
        fName.printf("create_backend_texture%s", mipmapped == GrMipmapped::kYes ? "_mipped" : "");
    }

private:
    bool isSuitableFor(Backend backend) override { return kGPU_Backend == backend; }

    const char* onGetName() override { return fName.c_str(); }

    void onDraw(int loops, SkCanvas* canvas) override {
        auto context = canvas->recordingContext()->asDirectContext();

        fBackendTextures.reserve_back(loops);

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
        context->submit(true);

        for (int i = 0; i < fBackendTextures.size(); ++i) {
            if (fBackendTextures[i].isValid()) {
                context->deleteBackendTexture(fBackendTextures[i]);
            }
        }
        fBackendTextures.clear();
    }
};

DEF_BENCH(return new CreateBackendTextureBench(GrMipmapped::kNo);)
DEF_BENCH(return new CreateBackendTextureBench(GrMipmapped::kYes);)
