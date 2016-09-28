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

SK_RASTER_STAGE(load_s_srgb) {
    auto ptr = (const uint32_t*)ctx + x;

    if (tail) {
        float rs[] = {0,0,0,0},
              gs[] = {0,0,0,0},
              bs[] = {0,0,0,0},
              as[] = {0,0,0,0};
        for (size_t i = 0; i < (tail&3); i++) {
            rs[i] = sk_linear_from_srgb[(ptr[i] >>  0) & 0xff];
            gs[i] = sk_linear_from_srgb[(ptr[i] >>  8) & 0xff];
            bs[i] = sk_linear_from_srgb[(ptr[i] >> 16) & 0xff];
            as[i] = (ptr[i] >> 24) * (1/255.0f);
        }
        r = Sk4f::Load(rs);
        g = Sk4f::Load(gs);
        b = Sk4f::Load(bs);
        a = Sk4f::Load(as);
        return;
    }

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

SK_RASTER_STAGE(load_d_srgb) {
    auto ptr = (const uint32_t*)ctx + x;

    if (tail) {
        float rs[] = {0,0,0,0},
              gs[] = {0,0,0,0},
              bs[] = {0,0,0,0},
              as[] = {0,0,0,0};
        for (size_t i = 0; i < (tail&3); i++) {
            rs[i] = sk_linear_from_srgb[(ptr[i] >>  0) & 0xff];
            gs[i] = sk_linear_from_srgb[(ptr[i] >>  8) & 0xff];
            bs[i] = sk_linear_from_srgb[(ptr[i] >> 16) & 0xff];
            as[i] = (ptr[i] >> 24) * (1/255.0f);
        }
        dr = Sk4f::Load(rs);
        dg = Sk4f::Load(gs);
        db = Sk4f::Load(bs);
        da = Sk4f::Load(as);
        return;
    }

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

SK_RASTER_STAGE(scale_u8) {
    auto ptr = (const uint8_t*)ctx + x;

    Sk4b cov;

    if (tail) {
        uint8_t cs[] = {0,0,0,0};
        switch (tail&3) {
            case 3: cs[2] = ptr[2];
            case 2: cs[1] = ptr[1];
            case 1: cs[0] = ptr[0];
        }
        cov = Sk4b::Load(cs);
    } else {
        cov = Sk4b::Load(ptr);
    }

    auto c = SkNx_cast<float>(cov) * (1/255.0f);
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

    uint32_t* dst = nullptr;
    uint32_t stack[4];

    if (tail) {
        dst = ptr;
        ptr = stack;
    }

    ( sk_linear_to_srgb(r)
    | sk_linear_to_srgb(g) << 8
    | sk_linear_to_srgb(b) << 16
    | Sk4f_round(255.0f*a) << 24).store(ptr);

    switch (tail&3) {
        case 3: dst[2] = ptr[2];
        case 2: dst[1] = ptr[1];
        case 1: dst[0] = ptr[0];
    }
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
            load_s_srgb(src    , x,0, r,g,b,a, dr,dg,db,da);
            scale_u8   (mask   , x,0, r,g,b,a, dr,dg,da,da);
            load_d_srgb(dst    , x,0, r,g,b,a, dr,dg,da,da);
            srcover    (nullptr, x,0, r,g,b,a, dr,dg,da,da);
            store_srgb (dst    , x,0, r,g,b,a, dr,dg,da,da);

            x += 4;
            n -= 4;
        }
        if (n > 0) {
            load_s_srgb(src    , x,n, r,g,b,a, dr,dg,db,da);
            scale_u8   (mask   , x,n, r,g,b,a, dr,dg,da,da);
            load_d_srgb(dst    , x,n, r,g,b,a, dr,dg,da,da);
            srcover    (nullptr, x,n, r,g,b,a, dr,dg,da,da);
            store_srgb (dst    , x,n, r,g,b,a, dr,dg,da,da);
        }
    }

    void runPipeline() {
        SkRasterPipeline p;
        p.append<load_s_srgb>(src);
        p.append<   scale_u8>(mask);
        p.append<load_d_srgb>(dst);
        p.append<    srcover>();
        p.last  < store_srgb>(dst);
        p.run(N);
    }

    bool fFused;
};

DEF_BENCH( return new SkRasterPipelineBench(true); )
DEF_BENCH( return new SkRasterPipelineBench(false); )
