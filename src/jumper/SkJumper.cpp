/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkJumper.h"
#include "SkJumper_generated.h"
#include "SkRasterPipeline.h"
#include "SkTemplates.h"

// Stages expect these constants to be set to these values.
// It's fine to rearrange and add new ones if you update SkJumper_constants.
static const SkJumper_constants kConstants = {
    1.0f, 0.5f, 255.0f, 1/255.0f, 0x000000ff,
    {0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f},
    0.0025f, 0.6975f, 0.3000f, 1/12.92f, 0.055f,       // from_srgb
    12.46f, 0.411192f, 0.689206f, -0.0988f, 0.0043f,   //   to_srgb
    0x77800000, 0x07800000,                            // fp16 <-> fp32
};

using JumperStage = void(size_t, void**, const SkJumper_constants*);

#define STAGES(M)     \
    M(seed_shader)    \
    M(constant_color) \
    M(clear)          \
    M(plus_)          \
    M(srcover)        \
    M(dstover)        \
    M(clamp_0)        \
    M(clamp_1)        \
    M(clamp_a)        \
    M(swap)           \
    M(move_src_dst)   \
    M(move_dst_src)   \
    M(premul)         \
    M(unpremul)       \
    M(from_srgb)      \
    M(to_srgb)        \
    M(scale_u8)       \
    M(load_tables)    \
    M(load_8888)      \
    M(store_8888)     \
    M(load_f16)       \
    M(store_f16)      \
    M(matrix_2x3)     \
    M(matrix_3x4)     \
    M(clamp_x)        \
    M(clamp_y)        \
    M(linear_gradient_2stops)

extern "C" {
    JumperStage sk_just_return;
#define M(st) JumperStage sk_##st;
        STAGES(M)
#undef M
}

static void* lookup_jumper(SkRasterPipeline::StockStage st) {
    switch (st) {
        default: return nullptr;
    #define M(st) case SkRasterPipeline::st: return (void*)sse2_sk_##st;
        STAGES(M)
    #undef M
    }
}

static void* lookup_tail(SkRasterPipeline::StockStage st) {
    switch (st) {
        default: return nullptr;
    #define M(st) case SkRasterPipeline::st: return (void*)sk_##st;
        STAGES(M)
    #undef M
    }
}

bool SkRasterPipeline::run_with_jumper(size_t x, size_t n) const {
    SkAutoSTMalloc<64, void*> program(2*fStages.size() + 1);

    void** ip = program.get();
    for (auto&& st : fStages) {
        auto fn = lookup_jumper(st.stage);
        if (!fn) {
            return false;
        }
        *ip++ = fn;
        *ip++ = st.ctx;
    }
    *ip = (void*)sse2_sk_just_return;

    ip = program.get();
    auto start = (JumperStage*)*ip++;

    size_t limit = x+n;
    while (x + 4 < limit) {
        start(x, ip, &kConstants);
        x += 4;
    }

    if (x < limit) {
        void** ip = program.get();
        for (auto&& st : fStages) {
            *ip++ = lookup_tail(st.stage);
            *ip++ = st.ctx;
        }
        *ip = (void*)sk_just_return;

        ip = program.get();
        auto start = (JumperStage*)*ip++;
        while (x < limit) {
            start(x, ip, &kConstants);
            x++;
        }
    }

    return true;
}
