/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"
#include "SkHalf.h"
#include "SkRasterPipeline.h"
#include "../src/jumper/SkJumper.h"

DEF_TEST(SkRasterPipeline, r) {
    // Build and run a simple pipeline to exercise SkRasterPipeline,
    // drawing 50% transparent blue over opaque red in half-floats.
    uint64_t red  = 0x3c00000000003c00ull,
             blue = 0x3800380000000000ull,
             result;

    void* load_s_ctx = &blue;
    void* load_d_ctx = &red;
    void* store_ctx  = &result;

    SkRasterPipeline_<256> p;
    p.append(SkRasterPipeline::load_f16,     &load_s_ctx);
    p.append(SkRasterPipeline::load_f16_dst, &load_d_ctx);
    p.append(SkRasterPipeline::srcover);
    p.append(SkRasterPipeline::store_f16, &store_ctx);
    p.run(0,0,1);

    // We should see half-intensity magenta.
    REPORTER_ASSERT(r, ((result >>  0) & 0xffff) == 0x3800);
    REPORTER_ASSERT(r, ((result >> 16) & 0xffff) == 0x0000);
    REPORTER_ASSERT(r, ((result >> 32) & 0xffff) == 0x3800);
    REPORTER_ASSERT(r, ((result >> 48) & 0xffff) == 0x3c00);
}

DEF_TEST(SkRasterPipeline_empty, r) {
    // No asserts... just a test that this is safe to run.
    SkRasterPipeline_<256> p;
    p.run(0,0,20);
}

DEF_TEST(SkRasterPipeline_nonsense, r) {
    // No asserts... just a test that this is safe to run and terminates.
    // srcover() calls st->next(); this makes sure we've always got something there to call.
    SkRasterPipeline_<256> p;
    p.append(SkRasterPipeline::srcover);
    p.run(0,0,20);
}

DEF_TEST(SkRasterPipeline_JIT, r) {
    // This tests a couple odd corners that a JIT backend can stumble over.

    uint32_t buf[72] = {
         0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
         1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12,
        13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24,
         0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
         0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
         0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    };

    const uint32_t* src = buf +  0;
    uint32_t*       dst = buf + 36;

    // Copy buf[x] to buf[x+36] for x in [15,35).
    SkRasterPipeline_<256> p;
    p.append(SkRasterPipeline:: load_8888, &src);
    p.append(SkRasterPipeline::store_8888, &dst);
    p.run(15,0, 20);

    for (int i = 0; i < 36; i++) {
        if (i < 15 || i == 35) {
            REPORTER_ASSERT(r, dst[i] == 0);
        } else {
            REPORTER_ASSERT(r, dst[i] == (uint32_t)(i - 11));
        }
    }
}

static uint16_t h(float f) {
    // Remember, a float is 1-8-23 (sign-exponent-mantissa) with 127 exponent bias.
    uint32_t sem;
    memcpy(&sem, &f, sizeof(sem));
    uint32_t s  = sem & 0x80000000,
             em = sem ^ s;

    // Convert to 1-5-10 half with 15 bias, flushing denorm halfs (including zero) to zero.
    auto denorm = (int32_t)em < 0x38800000;  // I32 comparison is often quicker, and always safe
    // here.
    return denorm ? SkTo<uint16_t>(0)
                  : SkTo<uint16_t>((s>>16) + (em>>13) - ((127-15)<<10));
}

static uint16_t n(uint16_t x) {
    return (x<<8) | (x>>8);
}

static float a(uint16_t x) {
    return (1/65535.0f) * x;
}

DEF_TEST(SkRasterPipeline_tail, r) {
    {
        float data[][4] = {
            {00, 01, 02, 03},
            {10, 11, 12, 13},
            {20, 21, 22, 23},
            {30, 31, 32, 33},
        };

        float buffer[4][4];
        float* src = &data[0][0];
        float* dst = &buffer[0][0];

        for (unsigned i = 1; i <= 4; i++) {
            memset(buffer, 0xff, sizeof(buffer));
            SkRasterPipeline_<256> p;
            p.append(SkRasterPipeline::load_f32, &src);
            p.append(SkRasterPipeline::store_f32, &dst);
            p.run(0,0, i);
            for (unsigned j = 0; j < i; j++) {
                for (unsigned k = 0; k < 4; k++) {
                    if (buffer[j][k] != data[j][k]) {
                        ERRORF(r, "(%u, %u) - a: %g r: %g\n", j, k, data[j][k], buffer[j][k]);
                    }
                }
            }
            for (int j = i; j < 4; j++) {
                for (auto f : buffer[j]) {
                    REPORTER_ASSERT(r, SkScalarIsNaN(f));
                }
            }
        }
    }

    {
        uint16_t data[][4] = {
            {h(00), h(01), h(02), h(03)},
            {h(10), h(11), h(12), h(13)},
            {h(20), h(21), h(22), h(23)},
            {h(30), h(31), h(32), h(33)},
        };
        uint16_t buffer[4][4];
        uint16_t* src = &data[0][0];
        uint16_t* dst = &buffer[0][0];

        for (unsigned i = 1; i <= 4; i++) {
            memset(buffer, 0xff, sizeof(buffer));
            SkRasterPipeline_<256> p;
            p.append(SkRasterPipeline::load_f16, &src);
            p.append(SkRasterPipeline::store_f16, &dst);
            p.run(0,0, i);
            for (unsigned j = 0; j < i; j++) {
                REPORTER_ASSERT(r,
                                !memcmp(&data[j][0], &buffer[j][0], sizeof(buffer[j])));
            }
            for (int j = i; j < 4; j++) {
                for (auto f : buffer[j]) {
                    REPORTER_ASSERT(r, f == 0xffff);
                }
            }
        }
    }

    {
        uint16_t data[][3] = {
            {n(00), n(01), n(02)},
            {n(10), n(11), n(12)},
            {n(20), n(21), n(22)},
            {n(30), n(31), n(32)}
        };

        float answer[][4] = {
            {a(00), a(01), a(02), 1.0f},
            {a(10), a(11), a(12), 1.0f},
            {a(20), a(21), a(22), 1.0f},
            {a(30), a(31), a(32), 1.0f}
        };

        float buffer[4][4];
        uint16_t* src = &data[0][0];
        float* dst = &buffer[0][0];

        for (unsigned i = 1; i <= 4; i++) {
            memset(buffer, 0xff, sizeof(buffer));
            SkRasterPipeline_<256> p;
            p.append(SkRasterPipeline::load_rgb_u16_be, &src);
            p.append(SkRasterPipeline::store_f32, &dst);
            p.run(0,0, i);
            for (unsigned j = 0; j < i; j++) {
                for (unsigned k = 0; k < 4; k++) {
                    if (buffer[j][k] != answer[j][k]) {
                        ERRORF(r, "(%u, %u) - a: %g r: %g\n", j, k, answer[j][k], buffer[j][k]);
                    }
                }
            }
            for (int j = i; j < 4; j++) {
                for (auto f : buffer[j]) {
                    REPORTER_ASSERT(r, SkScalarIsNaN(f));
                }
            }
        }
    }
}

DEF_TEST(SkRasterPipeline_lowp, r) {
    uint32_t rgba[64];
    for (int i = 0; i < 64; i++) {
        rgba[i] = (4*i+0) << 0
                | (4*i+1) << 8
                | (4*i+2) << 16
                | (4*i+3) << 24;
    }

    void* ptr = rgba;

    SkRasterPipeline_<256> p;
    p.append(SkRasterPipeline::load_bgra,  &ptr);
    p.append(SkRasterPipeline::store_8888, &ptr);
    p.run(0,0,64);

    for (int i = 0; i < 64; i++) {
        uint32_t want = (4*i+0) << 16
                      | (4*i+1) << 8
                      | (4*i+2) << 0
                      | (4*i+3) << 24;
        if (rgba[i] != want) {
            ERRORF(r, "got %08x, want %08x\n", rgba[i], want);
        }
    }
}

DEF_TEST(SkRasterPipeline_2d, r) {
    uint32_t rgba[2*2] = {0,0,0,0};

    SkSTArenaAlloc<256> alloc;
    SkRasterPipeline p(&alloc);

    // Splat out the (2d) dst coordinates: (0.5,0.5), (1.5,0.5), (0.5,1.5), (1.5,1.5).
    p.append(SkRasterPipeline::seed_shader);

    // Scale down to [0,1] range to write out as bytes.
    p.append_matrix(&alloc, SkMatrix::Concat(SkMatrix::MakeScale(0.5f),
                                             SkMatrix::MakeTrans(-0.5f, -0.5f)));

    // Write out to rgba, with row stride = 2 pixels.
    SkJumper_MemoryCtx ctx = { rgba, 2 };
    p.append(SkRasterPipeline::store_8888_2d, &ctx);

    p.run_2d(0,0, 2,2);

    REPORTER_ASSERT(r, ((rgba[0] >> 0) & 0xff) ==   0);
    REPORTER_ASSERT(r, ((rgba[1] >> 0) & 0xff) == 128);
    REPORTER_ASSERT(r, ((rgba[2] >> 0) & 0xff) ==   0);
    REPORTER_ASSERT(r, ((rgba[3] >> 0) & 0xff) == 128);

    REPORTER_ASSERT(r, ((rgba[0] >> 8) & 0xff) ==   0);
    REPORTER_ASSERT(r, ((rgba[1] >> 8) & 0xff) ==   0);
    REPORTER_ASSERT(r, ((rgba[2] >> 8) & 0xff) == 128);
    REPORTER_ASSERT(r, ((rgba[3] >> 8) & 0xff) == 128);
}
