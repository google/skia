/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bench/Benchmark.h"
#include "src/core/SkVM.h"

namespace {

    struct SrcoverBuilder : public skvm::Builder {
        SrcoverBuilder() {

            skvm::Arg src = arg(0),
                      dst = arg(1);

            auto byte_to_f32 = [&](skvm::Val byte) {
                return mul_f32(splat(1/255.0f), to_f32(byte));
            };
            auto f32_to_byte = [&](skvm::Val f32) {
                return to_i32(mad_f32(f32, splat(255.0f), splat(0.5f)));
            };

            auto load = [&](skvm::Arg ptr,
                            skvm::Val* r, skvm::Val* g, skvm::Val* b, skvm::Val* a) {
                skvm::Val rgba = load32(ptr);
                *r = byte_to_f32(bit_and(    rgba     , splat(0xff)));
                *g = byte_to_f32(bit_and(shr(rgba,  8), splat(0xff)));
                *b = byte_to_f32(bit_and(shr(rgba, 16), splat(0xff)));
                *a = byte_to_f32(        shr(rgba, 24)       );
            };

            skvm::Val r,g,b,a;
            load(src, &r,&g,&b,&a);

            skvm::Val dr,dg,db,da;
            load(dst, &dr,&dg,&db,&da);

            skvm::Val invA = sub_f32(splat(1.0f), a);
            r = mad_f32(dr, invA, r);
            g = mad_f32(dg, invA, g);
            b = mad_f32(db, invA, b);
            a = mad_f32(da, invA, a);

            store32(dst, bit_or(    f32_to_byte(r)     ,
                         bit_or(shl(f32_to_byte(g),  8),
                         bit_or(shl(f32_to_byte(b), 16),
                                shl(f32_to_byte(a), 24)))));
        }
    };

}

class SkVMBench : public Benchmark {
public:
    SkVMBench(int pixels, bool lowp)
        : fPixels(pixels)
        , fLowp(lowp)
        , fName(SkStringPrintf("SkVM_%d%s", pixels, lowp ? "_lowp" : ""))
    {}

private:
    const char* onGetName() override { return fName.c_str(); }
    bool isSuitableFor(Backend backend) override { return backend == kNonRendering_Backend; }

    void onDelayedSetup() override {
        this->setUnits(fPixels);
        fSrc.resize(fPixels);
        fDst.resize(fPixels);
        (void)fLowp;  // TODO: int math
        fSrcover = SrcoverBuilder{}.done();
    }

    void onDraw(int loops, SkCanvas*) override {
        while (loops --> 0) {
            fSrcover.eval(fPixels, fSrc.data(), fDst.data());
        }
    }

    int                   fPixels;
    bool                  fLowp;
    SkString              fName;
    std::vector<uint32_t> fSrc,
                          fDst;
    skvm::Program         fSrcover;
};

DEF_BENCH(return (new SkVMBench{   1,false});)
DEF_BENCH(return (new SkVMBench{   4,false});)
DEF_BENCH(return (new SkVMBench{  16,false});)
DEF_BENCH(return (new SkVMBench{  64,false});)
DEF_BENCH(return (new SkVMBench{ 256,false});)
DEF_BENCH(return (new SkVMBench{1024,false});)
DEF_BENCH(return (new SkVMBench{4096,false});)
