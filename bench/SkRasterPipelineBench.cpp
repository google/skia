/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Benchmark.h"
#include "SkRasterPipeline.h"
#include "SkSRGB.h"

static const int N = 1023;

static uint32_t dst[N],
                src[N];
static uint8_t mask[N];

// We'll build up a somewhat realistic useful pipeline:
//   - load srgb src
//   - scale src by 8-bit mask
//   - load srgb dst
//   - src = srcover(dst, src)
//   - store src back as srgb
// Every stage except for srcover interacts with memory, and so will need _tail variants.

SK_RASTER_STAGE(load_s_srgb) {
    auto ptr = (const uint32_t*)ctx + x;

    r = Sk4f{ sk_linear_from_srgb[(ptr[0] >>  0) & 0xff],
              sk_linear_from_srgb[(ptr[1] >>  0) & 0xff],
              sk_linear_from_srgb[(ptr[2] >>  0) & 0xff],
              sk_linear_from_srgb[(ptr[3] >>  0) & 0xff] };

    g = Sk4f{ sk_linear_from_srgb[(ptr[0] >>  8) & 0xff],
              sk_linear_from_srgb[(ptr[1] >>  8) & 0xff],
              sk_linear_from_srgb[(ptr[2] >>  8) & 0xff],
              sk_linear_from_srgb[(ptr[3] >>  8) & 0xff] };

    b = Sk4f{ sk_linear_from_srgb[(ptr[0] >> 16) & 0xff],
              sk_linear_from_srgb[(ptr[1] >> 16) & 0xff],
              sk_linear_from_srgb[(ptr[2] >> 16) & 0xff],
              sk_linear_from_srgb[(ptr[3] >> 16) & 0xff] };

    a = SkNx_cast<float>((Sk4i::Load(ptr) >> 24) & 0xff) * (1/255.0f);
}

SK_RASTER_STAGE(load_s_srgb_tail) {
    auto ptr = (const uint32_t*)ctx + x;

    r = Sk4f{ sk_linear_from_srgb[(*ptr >>  0) & 0xff], 0,0,0 };
    g = Sk4f{ sk_linear_from_srgb[(*ptr >>  8) & 0xff], 0,0,0 };
    b = Sk4f{ sk_linear_from_srgb[(*ptr >> 16) & 0xff], 0,0,0 };
    a = Sk4f{                (*ptr >> 24) * (1/255.0f), 0,0,0 };
}

SK_RASTER_STAGE(load_d_srgb) {
    auto ptr = (const uint32_t*)ctx + x;

    dr = Sk4f{ sk_linear_from_srgb[(ptr[0] >>  0) & 0xff],
               sk_linear_from_srgb[(ptr[1] >>  0) & 0xff],
               sk_linear_from_srgb[(ptr[2] >>  0) & 0xff],
               sk_linear_from_srgb[(ptr[3] >>  0) & 0xff] };

    dg = Sk4f{ sk_linear_from_srgb[(ptr[0] >>  8) & 0xff],
               sk_linear_from_srgb[(ptr[1] >>  8) & 0xff],
               sk_linear_from_srgb[(ptr[2] >>  8) & 0xff],
               sk_linear_from_srgb[(ptr[3] >>  8) & 0xff] };

    db = Sk4f{ sk_linear_from_srgb[(ptr[0] >> 16) & 0xff],
               sk_linear_from_srgb[(ptr[1] >> 16) & 0xff],
               sk_linear_from_srgb[(ptr[2] >> 16) & 0xff],
               sk_linear_from_srgb[(ptr[3] >> 16) & 0xff] };

    da = SkNx_cast<float>((Sk4i::Load(ptr) >> 24) & 0xff) * (1/255.0f);
}

SK_RASTER_STAGE(load_d_srgb_tail) {
    auto ptr = (const uint32_t*)ctx + x;

    dr = Sk4f{ sk_linear_from_srgb[(*ptr >>  0) & 0xff], 0,0,0 };
    dg = Sk4f{ sk_linear_from_srgb[(*ptr >>  8) & 0xff], 0,0,0 };
    db = Sk4f{ sk_linear_from_srgb[(*ptr >> 16) & 0xff], 0,0,0 };
    da = Sk4f{                (*ptr >> 24) * (1/255.0f), 0,0,0 };
}

SK_RASTER_STAGE(scale_u8) {
    auto ptr = (const uint8_t*)ctx + x;

    auto c = SkNx_cast<float>(Sk4b::Load(ptr)) * (1/255.0f);
    r *= c;
    g *= c;
    b *= c;
    a *= c;
}

SK_RASTER_STAGE(scale_u8_tail) {
    auto ptr = (const uint8_t*)ctx + x;

    auto c = *ptr * (1/255.0f);
    r *= c;
    g *= c;
    b *= c;
    a *= c;
}

SK_RASTER_STAGE(srcover) {
    auto A = 1.0f - a;
    r += dr * A;
    g += dg * A;
    b += db * A;
    a += da * A;
}

SK_RASTER_STAGE(store_srgb) {
    auto ptr = (uint32_t*)ctx + x;

    ( sk_linear_to_srgb(r)
    | sk_linear_to_srgb(g) << 8
    | sk_linear_to_srgb(b) << 16
    | Sk4f_round(255.0f*a) << 24).store(ptr);
}

SK_RASTER_STAGE(store_srgb_tail) {
    auto ptr = (uint32_t*)ctx + x;

    Sk4i rgba = sk_linear_to_srgb({r[0], g[0], b[0], 0});
    rgba = {rgba[0], rgba[1], rgba[2], (int)(255.0f * a[0] + 0.5f)};

    SkNx_cast<uint8_t>(rgba).store(ptr);
}

class SkRasterPipelineBench : public Benchmark {
public:
    SkRasterPipelineBench(bool fused) : fFused(fused) {}

    bool isSuitableFor(Backend backend) override { return backend == kNonRendering_Backend; }
    const char* onGetName() override { return fFused ? "SkRasterPipelineBench_fused"
                                                     : "SkRasterPipelineBench_pipeline"; }

    void onDraw(int loops, SkCanvas*) override {
        while (loops --> 0) {
            fFused ? this->runFused() : this->runPipeline();
        }
    }

    void runFused() {
        Sk4f r,g,b,a, dr,dg,db,da;
        size_t x = 0, n = N;
        while (n >= 4) {
            load_s_srgb(src    , x, r,g,b,a, dr,dg,db,da);
            scale_u8   (mask   , x, r,g,b,a, dr,dg,da,da);
            load_d_srgb(dst    , x, r,g,b,a, dr,dg,da,da);
            srcover    (nullptr, x, r,g,b,a, dr,dg,da,da);
            store_srgb (dst    , x, r,g,b,a, dr,dg,da,da);

            x += 4;
            n -= 4;
        }
        while (n > 0) {
            load_s_srgb_tail(src    , x, r,g,b,a, dr,dg,db,da);
            scale_u8_tail   (mask   , x, r,g,b,a, dr,dg,da,da);
            load_d_srgb_tail(dst    , x, r,g,b,a, dr,dg,da,da);
            srcover         (nullptr, x, r,g,b,a, dr,dg,da,da);
            store_srgb_tail (dst    , x, r,g,b,a, dr,dg,da,da);

            x += 1;
            n -= 1;
        }
    }

    void runPipeline() {
        SkRasterPipeline p;
        p.append<load_s_srgb, load_s_srgb_tail>( src);
        p.append<   scale_u8,    scale_u8_tail>(mask);
        p.append<load_d_srgb, load_d_srgb_tail>( dst);
        p.append<srcover>();
        p.last< store_srgb,  store_srgb_tail>( dst);

        p.run(N);
    }

    bool fFused;
};

DEF_BENCH( return new SkRasterPipelineBench(true); )
DEF_BENCH( return new SkRasterPipelineBench(false); )
