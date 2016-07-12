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

static void SK_VECTORCALL load_s_srgb(SkRasterPipeline::Stage* st, size_t x,
                                      Sk4f  r, Sk4f  g, Sk4f  b, Sk4f  a,
                                      Sk4f dr, Sk4f dg, Sk4f db, Sk4f da) {
    auto ptr = st->ctx<const uint32_t*>() + x;

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

    st->next(x, r,g,b,a, dr,dg,db,da);
}

static void SK_VECTORCALL load_s_srgb_tail(SkRasterPipeline::Stage* st, size_t x,
                                           Sk4f  r, Sk4f  g, Sk4f  b, Sk4f  a,
                                           Sk4f dr, Sk4f dg, Sk4f db, Sk4f da) {
    auto ptr = st->ctx<const uint32_t*>() + x;

    r = Sk4f{ sk_linear_from_srgb[(*ptr >>  0) & 0xff], 0,0,0 };
    g = Sk4f{ sk_linear_from_srgb[(*ptr >>  8) & 0xff], 0,0,0 };
    b = Sk4f{ sk_linear_from_srgb[(*ptr >> 16) & 0xff], 0,0,0 };
    a = Sk4f{                (*ptr >> 24) * (1/255.0f), 0,0,0 };

    st->next(x, r,g,b,a, dr,dg,db,da);
}

static void SK_VECTORCALL load_d_srgb(SkRasterPipeline::Stage* st, size_t x,
                                      Sk4f  r, Sk4f  g, Sk4f  b, Sk4f  a,
                                      Sk4f dr, Sk4f dg, Sk4f db, Sk4f da) {
    auto ptr = st->ctx<const uint32_t*>() + x;

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

    st->next(x, r,g,b,a, dr,dg,db,da);
}

static void SK_VECTORCALL load_d_srgb_tail(SkRasterPipeline::Stage* st, size_t x,
                                           Sk4f  r, Sk4f  g, Sk4f  b, Sk4f  a,
                                           Sk4f dr, Sk4f dg, Sk4f db, Sk4f da) {
    auto ptr = st->ctx<const uint32_t*>() + x;

    dr = Sk4f{ sk_linear_from_srgb[(*ptr >>  0) & 0xff], 0,0,0 };
    dg = Sk4f{ sk_linear_from_srgb[(*ptr >>  8) & 0xff], 0,0,0 };
    db = Sk4f{ sk_linear_from_srgb[(*ptr >> 16) & 0xff], 0,0,0 };
    da = Sk4f{                (*ptr >> 24) * (1/255.0f), 0,0,0 };

    st->next(x, r,g,b,a, dr,dg,db,da);
}

static void SK_VECTORCALL scale_u8(SkRasterPipeline::Stage* st, size_t x,
                                   Sk4f  r, Sk4f  g, Sk4f  b, Sk4f  a,
                                   Sk4f dr, Sk4f dg, Sk4f db, Sk4f da) {
    auto ptr = st->ctx<const uint8_t*>() + x;

    auto c = SkNx_cast<float>(Sk4b::Load(ptr)) * (1/255.0f);
    r *= c;
    g *= c;
    b *= c;
    a *= c;

    st->next(x, r,g,b,a, dr,dg,db,da);
}

static void SK_VECTORCALL scale_u8_tail(SkRasterPipeline::Stage* st, size_t x,
                                        Sk4f  r, Sk4f  g, Sk4f  b, Sk4f  a,
                                        Sk4f dr, Sk4f dg, Sk4f db, Sk4f da) {
    auto ptr = st->ctx<const uint8_t*>() + x;

    auto c = *ptr * (1/255.0f);
    r *= c;
    g *= c;
    b *= c;
    a *= c;

    st->next(x, r,g,b,a, dr,dg,db,da);
}

static void SK_VECTORCALL srcover(SkRasterPipeline::Stage* st, size_t x,
                                  Sk4f  r, Sk4f  g, Sk4f  b, Sk4f  a,
                                  Sk4f dr, Sk4f dg, Sk4f db, Sk4f da) {
    auto A = 1.0f - a;
    r += dr * A;
    g += dg * A;
    b += db * A;
    a += da * A;

    st->next(x, r,g,b,a, dr,dg,db,da);
}

static Sk4f clamp(const Sk4f& x) {
    return Sk4f::Min(Sk4f::Max(x, 0.0f), 255.0f);
}

static void SK_VECTORCALL store_srgb(SkRasterPipeline::Stage* st, size_t x,
                                     Sk4f  r, Sk4f  g, Sk4f  b, Sk4f  a,
                                     Sk4f dr, Sk4f dg, Sk4f db, Sk4f da) {
    auto ptr = st->ctx<uint32_t*>() + x;

    r = clamp(sk_linear_to_srgb(r));
    g = clamp(sk_linear_to_srgb(g));
    b = clamp(sk_linear_to_srgb(b));
    a = clamp(         255.0f * a );

    ( SkNx_cast<int>(r)
    | SkNx_cast<int>(g) << 8
    | SkNx_cast<int>(b) << 16
    | SkNx_cast<int>(a) << 24 ).store(ptr);
}

static void SK_VECTORCALL store_srgb_tail(SkRasterPipeline::Stage* st, size_t x,
                                          Sk4f  r, Sk4f  g, Sk4f  b, Sk4f  a,
                                          Sk4f dr, Sk4f dg, Sk4f db, Sk4f da) {
    auto ptr = st->ctx<uint32_t*>() + x;

    auto rgba = sk_linear_to_srgb({r[0], g[0], b[0], 0});
    rgba = {rgba[0], rgba[1], rgba[2], 255.0f*a[0]};
    rgba = clamp(rgba);

    SkNx_cast<uint8_t>(rgba).store(ptr);
}

class SkRasterPipelineBench : public Benchmark {
public:
    SkRasterPipelineBench() {}

    bool isSuitableFor(Backend backend) override { return backend == kNonRendering_Backend; }
    const char* onGetName() override { return "SkRasterPipelineBench"; }

    void onDraw(int loops, SkCanvas*) override {
        SkRasterPipeline p;
        p.append(load_s_srgb, load_s_srgb_tail,  src);
        p.append(   scale_u8,    scale_u8_tail, mask);
        p.append(load_d_srgb, load_d_srgb_tail,  dst);
        p.append(srcover);
        p.append( store_srgb,  store_srgb_tail,  dst);

        while (loops --> 0) {
            p.run(N);
        }
    }
};

DEF_BENCH( return new SkRasterPipelineBench; )
