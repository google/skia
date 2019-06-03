/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bench/Benchmark.h"
#include "src/core/SkOpts.h"
#include "src/core/SkVM.h"
#include "tools/SkVMBuilders.h"

namespace {

    enum Mode {Opts, RP, F32, I32, I32_SWAR};
    static const char* kMode_name[] = { "Opts", "RP","F32", "I32", "I32_SWAR" };

}

class SkVMBench : public Benchmark {
public:
    SkVMBench(int pixels, Mode mode)
        : fPixels(pixels)
        , fMode(mode)
        , fName(SkStringPrintf("SkVM_%d_%s", pixels, kMode_name[mode]))
    {}

private:
    const char* onGetName() override { return fName.c_str(); }
    bool isSuitableFor(Backend backend) override { return backend == kNonRendering_Backend; }

    void onDelayedSetup() override {
        this->setUnits(fPixels);
        fSrc.resize(fPixels, 0x7f123456);  // Arbitrary non-opaque non-transparent value.
        fDst.resize(fPixels, 0xff987654);  // Arbitrary value.

        if (fMode == F32     ) { fProgram = SrcoverBuilder_F32     {}.done(); }
        if (fMode == I32     ) { fProgram = SrcoverBuilder_I32     {}.done(); }
        if (fMode == I32_SWAR) { fProgram = SrcoverBuilder_I32_SWAR{}.done(); }

        if (fMode == RP) {
            fSrcCtx = { fSrc.data(), 0 };
            fDstCtx = { fDst.data(), 0 };
            fPipeline.append(SkRasterPipeline::load_8888    , &fSrcCtx);
            fPipeline.append(SkRasterPipeline::load_8888_dst, &fDstCtx);
            fPipeline.append(SkRasterPipeline::srcover);
            fPipeline.append(SkRasterPipeline::store_8888, &fDstCtx);
        }

        // Trigger one run now so we can do a quick correctness check.
        this->draw(1,nullptr);
        for (int i = 0; i < fPixels; i++) {
            SkASSERT(fDst[i] == 0xff5e6f80);
        }
    }

    void onDraw(int loops, SkCanvas*) override {
        while (loops --> 0) {
            if (fMode == Opts) {
                SkOpts::blit_row_s32a_opaque(fDst.data(), fSrc.data(), fPixels, 0xff);
            } else if (fMode == RP) {
                fPipeline.run(0,0,fPixels,1);
            } else {
                fProgram.eval(fPixels, fSrc.data(), fDst.data());
            }
        }
    }

    int                   fPixels;
    Mode                  fMode;
    SkString              fName;
    std::vector<uint32_t> fSrc,
                          fDst;
    skvm::Program         fProgram;

    SkRasterPipeline_MemoryCtx fSrcCtx,
                               fDstCtx;
    SkRasterPipeline_<256>     fPipeline;
};

DEF_BENCH(return (new SkVMBench{   1, Opts});)
DEF_BENCH(return (new SkVMBench{   4, Opts});)
DEF_BENCH(return (new SkVMBench{  16, Opts});)
DEF_BENCH(return (new SkVMBench{  64, Opts});)
DEF_BENCH(return (new SkVMBench{ 256, Opts});)
DEF_BENCH(return (new SkVMBench{1024, Opts});)
DEF_BENCH(return (new SkVMBench{4096, Opts});)

DEF_BENCH(return (new SkVMBench{   1, RP});)
DEF_BENCH(return (new SkVMBench{   4, RP});)
DEF_BENCH(return (new SkVMBench{  16, RP});)
DEF_BENCH(return (new SkVMBench{  64, RP});)
DEF_BENCH(return (new SkVMBench{ 256, RP});)
DEF_BENCH(return (new SkVMBench{1024, RP});)
DEF_BENCH(return (new SkVMBench{4096, RP});)

DEF_BENCH(return (new SkVMBench{   1, F32});)
DEF_BENCH(return (new SkVMBench{   4, F32});)
DEF_BENCH(return (new SkVMBench{  16, F32});)
DEF_BENCH(return (new SkVMBench{  64, F32});)
DEF_BENCH(return (new SkVMBench{ 256, F32});)
DEF_BENCH(return (new SkVMBench{1024, F32});)
DEF_BENCH(return (new SkVMBench{4096, F32});)

DEF_BENCH(return (new SkVMBench{   1, I32});)
DEF_BENCH(return (new SkVMBench{   4, I32});)
DEF_BENCH(return (new SkVMBench{  16, I32});)
DEF_BENCH(return (new SkVMBench{  64, I32});)
DEF_BENCH(return (new SkVMBench{ 256, I32});)
DEF_BENCH(return (new SkVMBench{1024, I32});)
DEF_BENCH(return (new SkVMBench{4096, I32});)

DEF_BENCH(return (new SkVMBench{   1, I32_SWAR});)
DEF_BENCH(return (new SkVMBench{   4, I32_SWAR});)
DEF_BENCH(return (new SkVMBench{  16, I32_SWAR});)
DEF_BENCH(return (new SkVMBench{  64, I32_SWAR});)
DEF_BENCH(return (new SkVMBench{ 256, I32_SWAR});)
DEF_BENCH(return (new SkVMBench{1024, I32_SWAR});)
DEF_BENCH(return (new SkVMBench{4096, I32_SWAR});)
