/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Benchmark.h"
#include "SkHalf.h"
#include "SkNx.h"
#include "SkRasterPipeline.h"

static void hand_written(uint64_t* buf, int n) {
    while (n --> 0) {
        Sk4f rgba = SkHalfToFloat_finite_ftz(*buf);

        float a     = rgba[3],
              scale = a == 0 ? 1 : 1.0f/a;
        rgba *= Sk4f{scale,scale,scale,1};

        SkFloatToHalf_finite_ftz(rgba).store(buf++);
    }
}

class SkXbyakBench : public Benchmark {
public:
    enum Mode { kHandWritten, kInterpreted, kJITCompiled };

    SkXbyakBench(Mode mode) : fMode(mode) {
        if (mode == kInterpreted || mode == kJITCompiled) {
            fPtr = &fBuf;
            fP.append(SkRasterPipeline::load_f16, &fPtr);
            fP.append(SkRasterPipeline::unpremul);
            fP.append(SkRasterPipeline::store_f16, &fPtr);
        }

        if (mode == kJITCompiled) {
            fFn = fP.compile();
        }
    }
    bool isSuitableFor(Backend backend) override { return backend == kNonRendering_Backend; }

    const char* onGetName() override {
        switch(fMode) {
            case kHandWritten: return "SkXbyak_HandWritten";
            case kInterpreted: return "SkXbyak_Interpreted";
            case kJITCompiled: return "SkXbyak_JITCompiled";
        }
        return "";
    }

    void onDraw(int loops, SkCanvas*) override {
        switch (fMode) {
            case kHandWritten:
                while (loops --> 0) { hand_written(fBuf, N); }
                break;
            case kInterpreted:
                while (loops --> 0) { fP.run(0,0,N); }
                break;
            case kJITCompiled:
                while (loops --> 0) { fFn(0,0,N); }
                break;
        }
    }

private:
    static const int N = 1024;  // TODO: 1023, making the tail jagged

    SkRasterPipeline                            fP;
    Mode                                        fMode;
    uint64_t                                    fBuf[N];
    void*                                       fPtr;
    std::function<void(size_t, size_t, size_t)> fFn;
};

DEF_BENCH( return new SkXbyakBench(SkXbyakBench::kHandWritten); )
DEF_BENCH( return new SkXbyakBench(SkXbyakBench::kInterpreted); )
DEF_BENCH( return new SkXbyakBench(SkXbyakBench::kJITCompiled); )
