/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bench/Benchmark.h"
#include "src/core/SkVM.h"

// N.B. I have not tested that the math performed by these benchmarks is correct.
// They're really more meant to be representative load.  (Wouldn't hurt to be correct though.)

namespace {

    enum Mode {F32, I32, I32_SWAR};
    static const char* kMode_name[] = { "F32", "I32", "I32_SWAR" };

    struct SrcoverBuilder_F32 : public skvm::Builder {
        SrcoverBuilder_F32() {

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
                *a = byte_to_f32(        shr(rgba, 24)              );
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

    struct SrcoverBuilder_I32 : public skvm::Builder {
        SrcoverBuilder_I32() {
            skvm::Arg src = arg(0),
                      dst = arg(1);

            auto load = [&](skvm::Arg ptr,
                            skvm::Val* r, skvm::Val* g, skvm::Val* b, skvm::Val* a) {
                skvm::Val rgba = load32(ptr);
                *r = bit_and(    rgba     , splat(0xff));
                *g = bit_and(shr(rgba,  8), splat(0xff));
                *b = bit_and(shr(rgba, 16), splat(0xff));
                *a =         shr(rgba, 24)              ;
            };

            auto mul_unorm8 = [&](skvm::Val x, skvm::Val y) {
                // (x*y + 127)/255 ~= (x*y+255)/256
                return shr(add_i32(mul_i32(x, y), splat(0xff)), 8);
            };

            skvm::Val r,g,b,a;
            load(src, &r,&g,&b,&a);

            skvm::Val dr,dg,db,da;
            load(dst, &dr,&dg,&db,&da);

            skvm::Val invA = sub_i32(splat(0xff), a);
            r = add_i32(r, mul_unorm8(dr, invA));
            g = add_i32(g, mul_unorm8(dr, invA));
            b = add_i32(b, mul_unorm8(dr, invA));
            a = add_i32(a, mul_unorm8(dr, invA));

            store32(dst, bit_or(    r     ,
                         bit_or(shl(g,  8),
                         bit_or(shl(b, 16),
                                shl(a, 24)))));
        }
    };

    struct SrcoverBuilder_I32_SWAR : public skvm::Builder {
        SrcoverBuilder_I32_SWAR() {
            skvm::Arg src = arg(0),
                      dst = arg(1);

            auto load = [&](skvm::Arg ptr,
                            skvm::Val* rb, skvm::Val* ga) {
                skvm::Val rgba = load32(ptr);
                *rb = bit_and(    rgba,     splat(0x00ff00ff));
                *ga = bit_and(shr(rgba, 8), splat(0x00ff00ff));
            };

            auto mul_unorm8 = [&](skvm::Val x, skvm::Val y) {
                // As above, assuming x is two SWAR bytes in lanes 0 and 2, and y is a byte.
                return shr(add_i32(mul_i32(x, y), splat(0x00ff00ff)), 8);
            };

            skvm::Val rb, ga;
            load(src, &rb, &ga);

            skvm::Val drb, dga;
            load(dst, &drb, &dga);

            skvm::Val invA = sub_i32(splat(0xff), shr(ga, 16));
            rb = add_i32(rb, mul_unorm8(drb, invA));
            ga = add_i32(ga, mul_unorm8(dga, invA));

            store32(dst, bit_or(rb, shl(ga, 8)));
        }
    };
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
        fSrc.resize(fPixels);
        fDst.resize(fPixels);

        if (fMode == F32     ) { fSrcover = SrcoverBuilder_F32     {}.done(); }
        if (fMode == I32     ) { fSrcover = SrcoverBuilder_I32     {}.done(); }
        if (fMode == I32_SWAR) { fSrcover = SrcoverBuilder_I32_SWAR{}.done(); }
    }

    void onDraw(int loops, SkCanvas*) override {
        while (loops --> 0) {
            fSrcover.eval(fPixels, fSrc.data(), fDst.data());
        }
    }

    int                   fPixels;
    Mode                  fMode;
    SkString              fName;
    std::vector<uint32_t> fSrc,
                          fDst;
    skvm::Program         fSrcover;
};

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
