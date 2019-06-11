/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/SkHalf.h"
#include "include/private/SkTo.h"
#include "src/core/SkRasterPipeline.h"
#include "src/gpu/GrSwizzle.h"
#include "tests/Test.h"

DEF_TEST(SkRasterPipeline, r) {
    // Build and run a simple pipeline to exercise SkRasterPipeline,
    // drawing 50% transparent blue over opaque red in half-floats.
    uint64_t red  = 0x3c00000000003c00ull,
             blue = 0x3800380000000000ull,
             result;

    SkRasterPipeline_MemoryCtx load_s_ctx = { &blue, 0 },
                               load_d_ctx = { &red, 0 },
                               store_ctx  = { &result, 0 };

    SkRasterPipeline_<256> p;
    p.append(SkRasterPipeline::load_f16,     &load_s_ctx);
    p.append(SkRasterPipeline::load_f16_dst, &load_d_ctx);
    p.append(SkRasterPipeline::srcover);
    p.append(SkRasterPipeline::store_f16, &store_ctx);
    p.run(0,0,1,1);

    // We should see half-intensity magenta.
    REPORTER_ASSERT(r, ((result >>  0) & 0xffff) == 0x3800);
    REPORTER_ASSERT(r, ((result >> 16) & 0xffff) == 0x0000);
    REPORTER_ASSERT(r, ((result >> 32) & 0xffff) == 0x3800);
    REPORTER_ASSERT(r, ((result >> 48) & 0xffff) == 0x3c00);
}

DEF_TEST(SkRasterPipeline_empty, r) {
    // No asserts... just a test that this is safe to run.
    SkRasterPipeline_<256> p;
    p.run(0,0,20,1);
}

DEF_TEST(SkRasterPipeline_nonsense, r) {
    // No asserts... just a test that this is safe to run and terminates.
    // srcover() calls st->next(); this makes sure we've always got something there to call.
    SkRasterPipeline_<256> p;
    p.append(SkRasterPipeline::srcover);
    p.run(0,0,20,1);
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

    SkRasterPipeline_MemoryCtx src = { buf +  0, 0 },
                       dst = { buf + 36, 0 };

    // Copy buf[x] to buf[x+36] for x in [15,35).
    SkRasterPipeline_<256> p;
    p.append(SkRasterPipeline:: load_8888, &src);
    p.append(SkRasterPipeline::store_8888, &dst);
    p.run(15,0, 20,1);

    for (int i = 0; i < 36; i++) {
        if (i < 15 || i == 35) {
            REPORTER_ASSERT(r, buf[i+36] == 0);
        } else {
            REPORTER_ASSERT(r, buf[i+36] == (uint32_t)(i - 11));
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

DEF_TEST(SkRasterPipeline_tail, r) {
    {
        float data[][4] = {
            {00, 01, 02, 03},
            {10, 11, 12, 13},
            {20, 21, 22, 23},
            {30, 31, 32, 33},
        };

        float buffer[4][4];

        SkRasterPipeline_MemoryCtx src = { &data[0][0], 0 },
                           dst = { &buffer[0][0], 0 };

        for (unsigned i = 1; i <= 4; i++) {
            memset(buffer, 0xff, sizeof(buffer));
            SkRasterPipeline_<256> p;
            p.append(SkRasterPipeline::load_f32, &src);
            p.append(SkRasterPipeline::store_f32, &dst);
            p.run(0,0, i,1);
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
        float data[][2] = {
            {00, 01},
            {10, 11},
            {20, 21},
            {30, 31},
        };

        float buffer[4][4];

        SkRasterPipeline_MemoryCtx src = { &data[0][0], 0 },
                dst = { &buffer[0][0], 0 };

        for (unsigned i = 1; i <= 4; i++) {
            memset(buffer, 0xff, sizeof(buffer));
            SkRasterPipeline_<256> p;
            p.append(SkRasterPipeline::load_rgf32, &src);
            p.append(SkRasterPipeline::store_f32, &dst);
            p.run(0,0, i,1);
            for (unsigned j = 0; j < i; j++) {
                for (unsigned k = 0; k < 2; k++) {
                    if (buffer[j][k] != data[j][k]) {
                        ERRORF(r, "(%u, %u) - a: %g r: %g\n", j, k, data[j][k], buffer[j][k]);
                    }
                }
                if (buffer[j][2] != 0) {
                    ERRORF(r, "(%u, 2) - a: 0 r: %g\n", j, buffer[j][2]);
                }
                if (buffer[j][3] != 1) {
                    ERRORF(r, "(%u, 3) - a: 1 r: %g\n", j, buffer[j][3]);
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
        float data[][4] = {
            {00, 01, 02, 03},
            {10, 11, 12, 13},
            {20, 21, 22, 23},
            {30, 31, 32, 33},
        };

        float buffer[4][2];

        SkRasterPipeline_MemoryCtx src = { &data[0][0], 0 },
                dst = { &buffer[0][0], 0 };

        for (unsigned i = 1; i <= 4; i++) {
            memset(buffer, 0xff, sizeof(buffer));
            SkRasterPipeline_<256> p;
            p.append(SkRasterPipeline::load_f32, &src);
            p.append(SkRasterPipeline::store_rgf32, &dst);
            p.run(0,0, i,1);
            for (unsigned j = 0; j < i; j++) {
                for (unsigned k = 0; k < 2; k++) {
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
        alignas(8) uint16_t data[][4] = {
            {h(00), h(01), h(02), h(03)},
            {h(10), h(11), h(12), h(13)},
            {h(20), h(21), h(22), h(23)},
            {h(30), h(31), h(32), h(33)},
        };
        alignas(8) uint16_t buffer[4][4];
        SkRasterPipeline_MemoryCtx src = { &data[0][0], 0 },
                           dst = { &buffer[0][0], 0 };

        for (unsigned i = 1; i <= 4; i++) {
            memset(buffer, 0xff, sizeof(buffer));
            SkRasterPipeline_<256> p;
            p.append(SkRasterPipeline::load_f16, &src);
            p.append(SkRasterPipeline::store_f16, &dst);
            p.run(0,0, i,1);
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
        alignas(8) uint16_t data[]= {
            h(00),
            h(10),
            h(20),
            h(30),
        };
        alignas(8) uint16_t buffer[4][4];
        SkRasterPipeline_MemoryCtx src = { &data[0], 0 },
                dst = { &buffer[0][0], 0 };

        for (unsigned i = 1; i <= 4; i++) {
            memset(buffer, 0xff, sizeof(buffer));
            SkRasterPipeline_<256> p;
            p.append(SkRasterPipeline::load_af16, &src);
            p.append(SkRasterPipeline::store_f16, &dst);
            p.run(0,0, i,1);
            for (unsigned j = 0; j < i; j++) {
                uint16_t expected[] = {0, 0, 0, data[j]};
                REPORTER_ASSERT(r, !memcmp(expected, &buffer[j][0], sizeof(buffer[j])));
            }
            for (int j = i; j < 4; j++) {
                for (auto f : buffer[j]) {
                    REPORTER_ASSERT(r, f == 0xffff);
                }
            }
        }
    }

    {
        alignas(8) uint16_t data[][4] = {
            {h(00), h(01), h(02), h(03)},
            {h(10), h(11), h(12), h(13)},
            {h(20), h(21), h(22), h(23)},
            {h(30), h(31), h(32), h(33)},
        };
        alignas(8) uint16_t buffer[4];
        SkRasterPipeline_MemoryCtx src = { &data[0][0], 0 },
                dst = { &buffer[0], 0 };

        for (unsigned i = 1; i <= 4; i++) {
            memset(buffer, 0xff, sizeof(buffer));
            SkRasterPipeline_<256> p;
            p.append(SkRasterPipeline::load_f16, &src);
            p.append(SkRasterPipeline::store_af16, &dst);
            p.run(0,0, i,1);
            for (unsigned j = 0; j < i; j++) {
                REPORTER_ASSERT(r, !memcmp(&data[j][3], &buffer[j], sizeof(buffer[j])));
            }
            for (int j = i; j < 4; j++) {
                REPORTER_ASSERT(r, buffer[j] == 0xffff);
            }
        }
    }

    {
        alignas(8) uint16_t data[][4] = {
            {h(00), h(01), h(02), h(03)},
            {h(10), h(11), h(12), h(13)},
            {h(20), h(21), h(22), h(23)},
            {h(30), h(31), h(32), h(33)},
        };
        alignas(8) uint16_t buffer[4][2];
        SkRasterPipeline_MemoryCtx src = { &data[0][0], 0 },
                dst = { &buffer[0][0], 0 };

        for (unsigned i = 1; i <= 4; i++) {
            memset(buffer, 0xff, sizeof(buffer));
            SkRasterPipeline_<256> p;
            p.append(SkRasterPipeline::load_f16, &src);
            p.append(SkRasterPipeline::store_rgf16, &dst);
            p.run(0,0, i,1);
            for (unsigned j = 0; j < i; j++) {
                REPORTER_ASSERT(r, !memcmp(&buffer[j], &data[j], 2 * sizeof(uint16_t)));
            }
            for (int j = i; j < 4; j++) {
                for (auto h : buffer[j]) {
                    REPORTER_ASSERT(r, h == 0xffff);
                }
            }
        }
    }

    {
        alignas(8) uint16_t data[][2] = {
            {h(00), h(01)},
            {h(10), h(11)},
            {h(20), h(21)},
            {h(30), h(31)},
        };
        alignas(8) uint16_t buffer[4][4];
        SkRasterPipeline_MemoryCtx src = { &data[0][0], 0 },
                dst = { &buffer[0][0], 0 };

        for (unsigned i = 1; i <= 4; i++) {
            memset(buffer, 0xff, sizeof(buffer));
            SkRasterPipeline_<256> p;
            p.append(SkRasterPipeline::load_rgf16, &src);
            p.append(SkRasterPipeline::store_f16, &dst);
            p.run(0,0, i,1);
            for (unsigned j = 0; j < i; j++) {
                uint16_t expected[] = {data[j][0], data[j][1], h(0), h(1)};
                REPORTER_ASSERT(r, !memcmp(&buffer[j], expected, sizeof(expected)));
            }
            for (int j = i; j < 4; j++) {
                for (auto h : buffer[j]) {
                    REPORTER_ASSERT(r, h == 0xffff);
                }
            }
        }
    }
}

DEF_TEST(SkRasterPipeline_u16, r) {
    {
        alignas(8) uint16_t data[][2] = {
            {0x0000, 0x0111},
            {0x1010, 0x1111},
            {0x2020, 0x2121},
            {0x3030, 0x3131},
        };
        uint8_t buffer[4][4];
        SkRasterPipeline_MemoryCtx src = { &data[0][0], 0 },
                dst = { &buffer[0][0], 0 };

        for (unsigned i = 1; i <= 4; i++) {
            memset(buffer, 0xab, sizeof(buffer));
            SkRasterPipeline_<256> p;
            p.append(SkRasterPipeline::load_rg1616, &src);
            p.append(SkRasterPipeline::store_8888, &dst);
            p.run(0,0, i,1);
            for (unsigned j = 0; j < i; j++) {
                uint8_t expected[] = {
                    SkToU8(data[j][0] >> 8),
                    SkToU8(data[j][1] >> 8),
                    000,
                    0xff
                };
                REPORTER_ASSERT(r, !memcmp(&buffer[j], expected, sizeof(expected)));
            }
            for (int j = i; j < 4; j++) {
                for (auto b : buffer[j]) {
                    REPORTER_ASSERT(r, b == 0xab);
                }
            }
        }
    }

    alignas(8) uint16_t data[] = {
            0x0000,
            0x1010,
            0x2020,
            0x3030,
    };
    uint8_t buffer[4][4];
    SkRasterPipeline_MemoryCtx src = { &data[0], 0 },
            dst = { &buffer[0][0], 0 };

    for (unsigned i = 1; i <= 4; i++) {
        memset(buffer, 0xff, sizeof(buffer));
        SkRasterPipeline_<256> p;
        p.append(SkRasterPipeline::load_a16, &src);
        p.append(SkRasterPipeline::store_8888, &dst);
        p.run(0,0, i,1);
        for (unsigned j = 0; j < i; j++) {
            uint8_t expected[] = {0x00, 0x00, 0x00, SkToU8(data[j] >> 8)};
            REPORTER_ASSERT(r, !memcmp(&buffer[j], expected, sizeof(expected)));
        }
        for (int j = i; j < 4; j++) {
            for (auto b : buffer[j]) {
                REPORTER_ASSERT(r, b == 0xff);
            }
        }
    }

    {
        uint8_t data[][4] = {
            {0x00, 0x01, 0x02, 0x03},
            {0x10, 0x11, 0x12, 0x13},
            {0x20, 0x21, 0x22, 0x23},
            {0x30, 0x31, 0x32, 0x33},
        };
        alignas(8) uint16_t buffer[4];
        SkRasterPipeline_MemoryCtx src = { &data[0][0], 0 },
                dst = { &buffer[0], 0 };

        for (unsigned i = 1; i <= 4; i++) {
            memset(buffer, 0xff, sizeof(buffer));
            SkRasterPipeline_<256> p;
            p.append(SkRasterPipeline::load_8888, &src);
            p.append(SkRasterPipeline::store_a16, &dst);
            p.run(0,0, i,1);
            for (unsigned j = 0; j < i; j++) {
                uint16_t expected = (data[j][3] << 8) | data[j][3];
                REPORTER_ASSERT(r, buffer[j] == expected);
            }
            for (int j = i; j < 4; j++) {
                REPORTER_ASSERT(r, buffer[j] == 0xffff);
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

    SkRasterPipeline_MemoryCtx ptr = { rgba, 0 };

    SkRasterPipeline_<256> p;
    p.append(SkRasterPipeline::load_8888,  &ptr);
    p.append(SkRasterPipeline::swap_rb);
    p.append(SkRasterPipeline::store_8888, &ptr);
    p.run(0,0,64,1);

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

DEF_TEST(SkRasterPipeline_swizzle, r) {
    // This takes the lowp code path
    {
        uint16_t rg[64];
        for (int i = 0; i < 64; i++) {
            rg[i] = (4*i+0) << 0
                  | (4*i+1) << 8;
        }

        GrSwizzle swizzle("g1b1");

        SkRasterPipeline_MemoryCtx ptr = { rg, 0 };
        SkRasterPipeline_<256> p;
        p.append(SkRasterPipeline::load_rg88,  &ptr);
        swizzle.apply(&p);
        p.append(SkRasterPipeline::store_rg88, &ptr);
        p.run(0,0,64,1);

        for (int i = 0; i < 64; i++) {
            uint32_t want = 0xff    << 8
                          | (4*i+1) << 0;
            if (rg[i] != want) {
                ERRORF(r, "got %08x, want %08x\n", rg[i], want);
            }
        }
    }
    // This takes the highp code path
    {
        float rg[64][2];
        for (int i = 0; i < 64; i++) {
            rg[i][0] = i + 1;
            rg[i][1] = 2 * i + 1;
        }

        GrSwizzle swizzle("1gra");

        uint16_t buffer[64][4];
        SkRasterPipeline_MemoryCtx src = { rg,     0 },
                                   dst = { buffer, 0};
        SkRasterPipeline_<256> p;
        p.append(SkRasterPipeline::load_rgf32,  &src);
        swizzle.apply(&p);
        p.append(SkRasterPipeline::store_f16, &dst);
        p.run(0,0,64,1);

        for (int i = 0; i < 64; i++) {
            uint16_t want[4] {
                h(1),
                h(2 * i + 1),
                h(i + 1),
                h(1),
            };
            REPORTER_ASSERT(r, !memcmp(want, buffer[i], sizeof(buffer[i])));
        }
    }
}

DEF_TEST(SkRasterPipeline_lowp_clamp01, r) {
    // This may seem like a funny pipeline to create,
    // but it certainly shouldn't crash when you run it.

    uint32_t rgba = 0xff00ff00;

    SkRasterPipeline_MemoryCtx ptr = { &rgba, 0 };

    SkRasterPipeline_<256> p;
    p.append(SkRasterPipeline::load_8888,  &ptr);
    p.append(SkRasterPipeline::swap_rb);
    p.append(SkRasterPipeline::clamp_0);
    p.append(SkRasterPipeline::clamp_1);
    p.append(SkRasterPipeline::store_8888, &ptr);
    p.run(0,0,1,1);
}
