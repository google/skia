/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bench/Benchmark.h"
#include "include/core/SkCanvas.h"
#include "include/gpu/GrContext.h"

class CreateBackendTextureBench : public Benchmark {
private:
    SkString fName;
    SkTArray<GrBackendTexture> fBackendTextures;
    GrMipMapped fMipMapped;

public:
    CreateBackendTextureBench(GrMipMapped mipMapped) : fMipMapped(mipMapped) {
        fName.printf("create_backend_texture%s", mipMapped == GrMipMapped::kYes ? "_mipped" : "");
    }

private:
    bool isSuitableFor(Backend backend) override { return kGPU_Backend == backend; }

    const char* onGetName() override { return fName.c_str(); }

    void onDraw(int loops, SkCanvas* canvas) override {
        GrContext* context = canvas->getGrContext();

        fBackendTextures.reserve(loops);

        static const int kSize = 16;
        for (int i = 0; i < loops; ++i) {
            fBackendTextures.push_back(context->createBackendTexture(
                    kSize, kSize, kRGBA_8888_SkColorType, SkColors::kRed, fMipMapped,
                    GrRenderable::kNo, GrProtected::kNo));
        }
    }

    void onPerCanvasPostDraw(SkCanvas* canvas) override {
        GrContext* context = canvas->getGrContext();

        GrFlushInfo info;
        info.fFlags = kSyncCpu_GrFlushFlag;
        context->flush(info);

        for (int i = 0; i < fBackendTextures.count(); ++i) {
            if (fBackendTextures[i].isValid()) {
                context->deleteBackendTexture(fBackendTextures[i]);
            }
        }
        fBackendTextures.reset();
    }
};

DEF_BENCH(return new CreateBackendTextureBench(GrMipMapped::kNo);)
DEF_BENCH(return new CreateBackendTextureBench(GrMipMapped::kYes);)
