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

    enum Mode {Opts, RP, F32, I32_Naive, I32, I32_SWAR};
    static const char* kMode_name[] = { "Opts", "RP","F32", "I32_Naive", "I32", "I32_SWAR" };

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

        if (fMode == F32      ) { fProgram = SrcoverBuilder_F32      {}.done(); }
        if (fMode == I32_Naive) { fProgram = SrcoverBuilder_I32_Naive{}.done(); }
        if (fMode == I32      ) { fProgram = SrcoverBuilder_I32      {}.done(); }
        if (fMode == I32_SWAR ) { fProgram = SrcoverBuilder_I32_SWAR {}.done(); }

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
            SkASSERTF(fDst[i] == 0xff5e6f80, "Want 0xff5e6f80, got %08x", fDst[i]);
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
DEF_BENCH(return (new SkVMBench{  15, Opts});)
DEF_BENCH(return (new SkVMBench{  63, Opts});)
DEF_BENCH(return (new SkVMBench{ 256, Opts});)
DEF_BENCH(return (new SkVMBench{1024, Opts});)
DEF_BENCH(return (new SkVMBench{4096, Opts});)

DEF_BENCH(return (new SkVMBench{   1, RP});)
DEF_BENCH(return (new SkVMBench{   4, RP});)
DEF_BENCH(return (new SkVMBench{  15, RP});)
DEF_BENCH(return (new SkVMBench{  63, RP});)
DEF_BENCH(return (new SkVMBench{ 256, RP});)
DEF_BENCH(return (new SkVMBench{1024, RP});)
DEF_BENCH(return (new SkVMBench{4096, RP});)

DEF_BENCH(return (new SkVMBench{   1, F32});)
DEF_BENCH(return (new SkVMBench{   4, F32});)
DEF_BENCH(return (new SkVMBench{  15, F32});)
DEF_BENCH(return (new SkVMBench{  63, F32});)
DEF_BENCH(return (new SkVMBench{ 256, F32});)
DEF_BENCH(return (new SkVMBench{1024, F32});)
DEF_BENCH(return (new SkVMBench{4096, F32});)

DEF_BENCH(return (new SkVMBench{   1, I32_Naive});)
DEF_BENCH(return (new SkVMBench{   4, I32_Naive});)
DEF_BENCH(return (new SkVMBench{  15, I32_Naive});)
DEF_BENCH(return (new SkVMBench{  63, I32_Naive});)
DEF_BENCH(return (new SkVMBench{ 256, I32_Naive});)
DEF_BENCH(return (new SkVMBench{1024, I32_Naive});)
DEF_BENCH(return (new SkVMBench{4096, I32_Naive});)

DEF_BENCH(return (new SkVMBench{   1, I32});)
DEF_BENCH(return (new SkVMBench{   4, I32});)
DEF_BENCH(return (new SkVMBench{  15, I32});)
DEF_BENCH(return (new SkVMBench{  63, I32});)
DEF_BENCH(return (new SkVMBench{ 256, I32});)
DEF_BENCH(return (new SkVMBench{1024, I32});)
DEF_BENCH(return (new SkVMBench{4096, I32});)

DEF_BENCH(return (new SkVMBench{   1, I32_SWAR});)
DEF_BENCH(return (new SkVMBench{   4, I32_SWAR});)
DEF_BENCH(return (new SkVMBench{  15, I32_SWAR});)
DEF_BENCH(return (new SkVMBench{  63, I32_SWAR});)
DEF_BENCH(return (new SkVMBench{ 256, I32_SWAR});)
DEF_BENCH(return (new SkVMBench{1024, I32_SWAR});)
DEF_BENCH(return (new SkVMBench{4096, I32_SWAR});)

class SkVM_Overhead : public Benchmark {
public:
    explicit SkVM_Overhead(bool rp) : fRP(rp) {}

private:
    const char* onGetName() override { return fRP ? "SkVM_Overhead_RP" : "SkVM_Overhead_VM"; }
    bool isSuitableFor(Backend backend) override { return backend == kNonRendering_Backend; }

    void onDraw(int loops, SkCanvas*) override {
        float dummy;
        if (fRP) {
            while (loops --> 0) {
                SkRasterPipeline_<256> rp;
                SkRasterPipeline_MemoryCtx src = { &dummy, 0},
                                           dst = { &dummy, 0};
                rp.append_load    (SkColorType::kRGBA_F32_SkColorType, &src);
                rp.append_load_dst(SkColorType::kRGBA_F32_SkColorType, &dst);
                rp.append         (SkRasterPipeline::srcover);
                rp.append_store   (SkColorType::kRGBA_F32_SkColorType, &dst);

                (void)rp.compile();
            }
        } else {
            while (loops --> 0) {
                skvm::Program program = SrcoverBuilder_F32{}.done();
                program.eval(0, &dummy, &dummy);
            }
        }
    }

    bool fRP;
};
DEF_BENCH(return new SkVM_Overhead{ true};)
DEF_BENCH(return new SkVM_Overhead{false};)
