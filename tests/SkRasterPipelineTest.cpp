/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/base/SkTo.h"
#include "src/base/SkHalf.h"
#include "src/base/SkUtils.h"
#include "src/core/SkOpts.h"
#include "src/core/SkRasterPipeline.h"
#include "src/gpu/Swizzle.h"
#include "tests/Test.h"

#include <cmath>
#include <numeric>

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
    p.append(SkRasterPipelineOp::load_f16,     &load_s_ctx);
    p.append(SkRasterPipelineOp::load_f16_dst, &load_d_ctx);
    p.append(SkRasterPipelineOp::srcover);
    p.append(SkRasterPipelineOp::store_f16, &store_ctx);
    p.run(0,0,1,1);

    // We should see half-intensity magenta.
    REPORTER_ASSERT(r, ((result >>  0) & 0xffff) == 0x3800);
    REPORTER_ASSERT(r, ((result >> 16) & 0xffff) == 0x0000);
    REPORTER_ASSERT(r, ((result >> 32) & 0xffff) == 0x3800);
    REPORTER_ASSERT(r, ((result >> 48) & 0xffff) == 0x3c00);
}

DEF_TEST(SkRasterPipeline_LoadStoreConditionMask, r) {
    alignas(64) int32_t mask[]  = {~0, 0, ~0,  0, ~0, ~0, ~0,  0};
    alignas(64) int32_t maskCopy[SkRasterPipeline_kMaxStride_highp] = {};
    alignas(64) int32_t dst[4 * SkRasterPipeline_kMaxStride_highp] = {};

    static_assert(std::size(mask) == SkRasterPipeline_kMaxStride_highp);

    SkRasterPipeline_<256> p;
    p.append(SkRasterPipelineOp::init_lane_masks);
    p.append(SkRasterPipelineOp::load_condition_mask, mask);
    p.append(SkRasterPipelineOp::store_condition_mask, maskCopy);
    p.append(SkRasterPipelineOp::store_dst, dst);
    p.run(0,0,SkOpts::raster_pipeline_highp_stride,1);

    {
        // `maskCopy` should be populated with `mask` in the frontmost positions
        // (depending on the architecture that SkRasterPipeline is targeting).
        size_t index = 0;
        for (; index < SkOpts::raster_pipeline_highp_stride; ++index) {
            REPORTER_ASSERT(r, maskCopy[index] == mask[index]);
        }

        // The remaining slots should have been left alone.
        for (; index < std::size(maskCopy); ++index) {
            REPORTER_ASSERT(r, maskCopy[index] == 0);
        }
    }
    {
        // `dr` and `da` should be populated with `mask`.
        // `dg` and `db` should remain initialized to true.
        const int dr = 0 * SkOpts::raster_pipeline_highp_stride;
        const int dg = 1 * SkOpts::raster_pipeline_highp_stride;
        const int db = 2 * SkOpts::raster_pipeline_highp_stride;
        const int da = 3 * SkOpts::raster_pipeline_highp_stride;
        for (size_t index = 0; index < SkOpts::raster_pipeline_highp_stride; ++index) {
            REPORTER_ASSERT(r, dst[dr + index] == mask[index]);
            REPORTER_ASSERT(r, dst[dg + index] == ~0);
            REPORTER_ASSERT(r, dst[db + index] == ~0);
            REPORTER_ASSERT(r, dst[da + index] == mask[index]);
        }
    }
}

DEF_TEST(SkRasterPipeline_LoadStoreLoopMask, r) {
    alignas(64) int32_t mask[]  = {~0, 0, ~0,  0, ~0, ~0, ~0,  0};
    alignas(64) int32_t maskCopy[SkRasterPipeline_kMaxStride_highp] = {};
    alignas(64) int32_t dst[4 * SkRasterPipeline_kMaxStride_highp] = {};

    static_assert(std::size(mask) == SkRasterPipeline_kMaxStride_highp);

    SkRasterPipeline_<256> p;
    p.append(SkRasterPipelineOp::init_lane_masks);
    p.append(SkRasterPipelineOp::load_loop_mask, mask);
    p.append(SkRasterPipelineOp::store_loop_mask, maskCopy);
    p.append(SkRasterPipelineOp::store_dst, dst);
    p.run(0,0,SkOpts::raster_pipeline_highp_stride,1);

    {
        // `maskCopy` should be populated with `mask` in the frontmost positions
        // (depending on the architecture that SkRasterPipeline is targeting).
        size_t index = 0;
        for (; index < SkOpts::raster_pipeline_highp_stride; ++index) {
            REPORTER_ASSERT(r, maskCopy[index] == mask[index]);
        }

        // The remaining slots should have been left alone.
        for (; index < std::size(maskCopy); ++index) {
            REPORTER_ASSERT(r, maskCopy[index] == 0);
        }
    }
    {
        // `dg` and `da` should be populated with `mask`.
        // `dr` and `db` should remain initialized to true.
        const int dr = 0 * SkOpts::raster_pipeline_highp_stride;
        const int dg = 1 * SkOpts::raster_pipeline_highp_stride;
        const int db = 2 * SkOpts::raster_pipeline_highp_stride;
        const int da = 3 * SkOpts::raster_pipeline_highp_stride;
        for (size_t index = 0; index < SkOpts::raster_pipeline_highp_stride; ++index) {
            REPORTER_ASSERT(r, dst[dr + index] == ~0);
            REPORTER_ASSERT(r, dst[dg + index] == mask[index]);
            REPORTER_ASSERT(r, dst[db + index] == ~0);
            REPORTER_ASSERT(r, dst[da + index] == mask[index]);
        }
    }
}

DEF_TEST(SkRasterPipeline_LoadStoreReturnMask, r) {
    alignas(64) int32_t mask[]  = {~0, 0, ~0,  0, ~0, ~0, ~0,  0};
    alignas(64) int32_t maskCopy[SkRasterPipeline_kMaxStride_highp] = {};
    alignas(64) int32_t dst[4 * SkRasterPipeline_kMaxStride_highp] = {};

    static_assert(std::size(mask) == SkRasterPipeline_kMaxStride_highp);

    SkRasterPipeline_<256> p;
    p.append(SkRasterPipelineOp::init_lane_masks);
    p.append(SkRasterPipelineOp::load_return_mask, mask);
    p.append(SkRasterPipelineOp::store_return_mask, maskCopy);
    p.append(SkRasterPipelineOp::store_dst, dst);
    p.run(0,0,SkOpts::raster_pipeline_highp_stride,1);

    {
        // `maskCopy` should be populated with `mask` in the frontmost positions
        // (depending on the architecture that SkRasterPipeline is targeting).
        size_t index = 0;
        for (; index < SkOpts::raster_pipeline_highp_stride; ++index) {
            REPORTER_ASSERT(r, maskCopy[index] == mask[index]);
        }

        // The remaining slots should have been left alone.
        for (; index < std::size(maskCopy); ++index) {
            REPORTER_ASSERT(r, maskCopy[index] == 0);
        }
    }
    {
        // `db` and `da` should be populated with `mask`.
        // `dr` and `dg` should remain initialized to true.
        const int dr = 0 * SkOpts::raster_pipeline_highp_stride;
        const int dg = 1 * SkOpts::raster_pipeline_highp_stride;
        const int db = 2 * SkOpts::raster_pipeline_highp_stride;
        const int da = 3 * SkOpts::raster_pipeline_highp_stride;
        for (size_t index = 0; index < SkOpts::raster_pipeline_highp_stride; ++index) {
            REPORTER_ASSERT(r, dst[dr + index] == ~0);
            REPORTER_ASSERT(r, dst[dg + index] == ~0);
            REPORTER_ASSERT(r, dst[db + index] == mask[index]);
            REPORTER_ASSERT(r, dst[da + index] == mask[index]);
        }
    }
}

DEF_TEST(SkRasterPipeline_MergeConditionMask, r) {
    alignas(64) int32_t mask[]  = { 0,  0, ~0, ~0, 0, ~0, 0, ~0,
                                   ~0, ~0, ~0, ~0, 0,  0, 0,  0};
    alignas(64) int32_t dst[4 * SkRasterPipeline_kMaxStride_highp] = {};
    static_assert(std::size(mask) == (2 * SkRasterPipeline_kMaxStride_highp));

    SkRasterPipeline_<256> p;
    p.append(SkRasterPipelineOp::init_lane_masks);
    p.append(SkRasterPipelineOp::merge_condition_mask, mask);
    p.append(SkRasterPipelineOp::store_dst, dst);
    p.run(0,0,SkOpts::raster_pipeline_highp_stride,1);

    // `dr` and `da` should be populated with `mask[x] & mask[y]` in the frontmost positions.
    // `dg` and `db` should remain initialized to true.
    const int dr = 0 * SkOpts::raster_pipeline_highp_stride;
    const int dg = 1 * SkOpts::raster_pipeline_highp_stride;
    const int db = 2 * SkOpts::raster_pipeline_highp_stride;
    const int da = 3 * SkOpts::raster_pipeline_highp_stride;
    for (size_t index = 0; index < SkOpts::raster_pipeline_highp_stride; ++index) {
        int32_t expected = mask[index] & mask[index + SkOpts::raster_pipeline_highp_stride];
        REPORTER_ASSERT(r, dst[dr + index] == expected);
        REPORTER_ASSERT(r, dst[dg + index] == ~0);
        REPORTER_ASSERT(r, dst[db + index] == ~0);
        REPORTER_ASSERT(r, dst[da + index] == expected);
    }
}

DEF_TEST(SkRasterPipeline_MergeLoopMask, r) {
    alignas(64) int32_t initial[]  = {~0, ~0, ~0, ~0, ~0,  0, ~0, ~0,  // dr (condition)
                                      ~0,  0, ~0,  0, ~0, ~0, ~0, ~0,  // dg (loop)
                                      ~0, ~0, ~0, ~0, ~0, ~0,  0, ~0,  // db (return)
                                      ~0, ~0, ~0, ~0, ~0, ~0, ~0, ~0}; // da (combined)
    alignas(64) int32_t mask[]     = { 0, ~0, ~0,  0, ~0, ~0, ~0, ~0};
    alignas(64) int32_t dst[4 * SkRasterPipeline_kMaxStride_highp] = {};
    static_assert(std::size(initial) == (4 * SkRasterPipeline_kMaxStride_highp));

    SkRasterPipeline_<256> p;
    p.append(SkRasterPipelineOp::load_dst, initial);
    p.append(SkRasterPipelineOp::merge_loop_mask, mask);
    p.append(SkRasterPipelineOp::store_dst, dst);
    p.run(0,0,SkOpts::raster_pipeline_highp_stride,1);

    const int dr = 0 * SkOpts::raster_pipeline_highp_stride;
    const int dg = 1 * SkOpts::raster_pipeline_highp_stride;
    const int db = 2 * SkOpts::raster_pipeline_highp_stride;
    const int da = 3 * SkOpts::raster_pipeline_highp_stride;
    for (size_t index = 0; index < SkOpts::raster_pipeline_highp_stride; ++index) {
        // `dg` should contain `dg & mask` in each lane.
        REPORTER_ASSERT(r, dst[dg + index] == (initial[dg + index] & mask[index]));

        // `dr` and `db` should be unchanged.
        REPORTER_ASSERT(r, dst[dr + index] == initial[dr + index]);
        REPORTER_ASSERT(r, dst[db + index] == initial[db + index]);

        // `da` should contain `dr & dg & gb`.
        REPORTER_ASSERT(r, dst[da + index] == (dst[dr+index] & dst[dg+index] & dst[db+index]));
    }
}

DEF_TEST(SkRasterPipeline_ReenableLoopMask, r) {
    alignas(64) int32_t initial[]  = {~0, ~0, ~0, ~0, ~0,  0, ~0, ~0,  // dr (condition)
                                      ~0,  0, ~0,  0, ~0, ~0,  0, ~0,  // dg (loop)
                                       0, ~0, ~0, ~0,  0,  0,  0, ~0,  // db (return)
                                       0,  0, ~0,  0,  0,  0,  0, ~0}; // da (combined)
    alignas(64) int32_t mask[]     = { 0, ~0,  0,  0,  0,  0, ~0,  0};
    alignas(64) int32_t dst[4 * SkRasterPipeline_kMaxStride_highp] = {};
    static_assert(std::size(initial) == (4 * SkRasterPipeline_kMaxStride_highp));

    SkRasterPipeline_<256> p;
    p.append(SkRasterPipelineOp::load_dst, initial);
    p.append(SkRasterPipelineOp::reenable_loop_mask, mask);
    p.append(SkRasterPipelineOp::store_dst, dst);
    p.run(0,0,SkOpts::raster_pipeline_highp_stride,1);

    const int dr = 0 * SkOpts::raster_pipeline_highp_stride;
    const int dg = 1 * SkOpts::raster_pipeline_highp_stride;
    const int db = 2 * SkOpts::raster_pipeline_highp_stride;
    const int da = 3 * SkOpts::raster_pipeline_highp_stride;
    for (size_t index = 0; index < SkOpts::raster_pipeline_highp_stride; ++index) {
        // `dg` should contain `dg | mask` in each lane.
        REPORTER_ASSERT(r, dst[dg + index] == (initial[dg + index] | mask[index]));

        // `dr` and `db` should be unchanged.
        REPORTER_ASSERT(r, dst[dr + index] == initial[dr + index]);
        REPORTER_ASSERT(r, dst[db + index] == initial[db + index]);

        // `da` should contain `dr & dg & gb`.
        REPORTER_ASSERT(r, dst[da + index] == (dst[dr+index] & dst[dg+index] & dst[db+index]));
    }
}

DEF_TEST(SkRasterPipeline_CaseOp, r) {
    alignas(64) int32_t initial[]        = {~0, ~0, ~0, ~0, ~0,  0, ~0, ~0,  // dr (condition)
                                             0, ~0, ~0,  0, ~0, ~0,  0, ~0,  // dg (loop)
                                            ~0,  0, ~0, ~0,  0,  0,  0, ~0,  // db (return)
                                             0,  0, ~0,  0,  0,  0,  0, ~0}; // da (combined)
    alignas(64) int32_t dst[4 * SkRasterPipeline_kMaxStride_highp] = {};
    static_assert(std::size(initial) == (4 * SkRasterPipeline_kMaxStride_highp));

    constexpr int32_t actualValues[] = { 2,  1,  2,  4,  5,  2,  2,  8};
    static_assert(std::size(actualValues) == SkRasterPipeline_kMaxStride_highp);

    alignas(64) int32_t caseOpData[2 * SkRasterPipeline_kMaxStride_highp];
    for (size_t index = 0; index < SkOpts::raster_pipeline_highp_stride; ++index) {
        caseOpData[0 * SkOpts::raster_pipeline_highp_stride + index] = actualValues[index];
        caseOpData[1 * SkOpts::raster_pipeline_highp_stride + index] = ~0;
    }

    SkRasterPipeline_CaseOpCtx ctx;
    ctx.ptr = caseOpData;
    ctx.expectedValue = 2;

    SkRasterPipeline_<256> p;
    p.append(SkRasterPipelineOp::load_dst, initial);
    p.append(SkRasterPipelineOp::case_op, &ctx);
    p.append(SkRasterPipelineOp::store_dst, dst);
    p.run(0,0,SkOpts::raster_pipeline_highp_stride,1);

    const int dr = 0 * SkOpts::raster_pipeline_highp_stride;
    const int dg = 1 * SkOpts::raster_pipeline_highp_stride;
    const int db = 2 * SkOpts::raster_pipeline_highp_stride;
    const int da = 3 * SkOpts::raster_pipeline_highp_stride;
    const int actualValueIdx = 0 * SkOpts::raster_pipeline_highp_stride;
    const int defaultMaskIdx = 1 * SkOpts::raster_pipeline_highp_stride;

    for (size_t index = 0; index < SkOpts::raster_pipeline_highp_stride; ++index) {
        // `dg` should have been set to true for each lane containing 2.
        int32_t expected = (actualValues[index] == 2) ? ~0 : initial[dg + index];
        REPORTER_ASSERT(r, dst[dg + index] == expected);

        // `dr` and `db` should be unchanged.
        REPORTER_ASSERT(r, dst[dr + index] == initial[dr + index]);
        REPORTER_ASSERT(r, dst[db + index] == initial[db + index]);

        // `da` should contain `dr & dg & gb`.
        REPORTER_ASSERT(r, dst[da + index] == (dst[dr+index] & dst[dg+index] & dst[db+index]));

        // The actual-value part of `caseOpData` should be unchanged from the inputs.
        REPORTER_ASSERT(r, caseOpData[actualValueIdx + index] == actualValues[index]);

        // The default-mask part of `caseOpData` should have been zeroed where the values matched.
        expected = (actualValues[index] == 2) ? 0 : ~0;
        REPORTER_ASSERT(r, caseOpData[defaultMaskIdx + index] == expected);
    }
}

DEF_TEST(SkRasterPipeline_MaskOffLoopMask, r) {
    alignas(64) int32_t initial[]  = {~0, ~0, ~0, ~0, ~0,  0, ~0, ~0,  // dr (condition)
                                      ~0,  0, ~0, ~0,  0,  0,  0, ~0,  // dg (loop)
                                      ~0, ~0,  0, ~0,  0,  0, ~0, ~0,  // db (return)
                                      ~0,  0,  0, ~0,  0,  0,  0, ~0}; // da (combined)
    alignas(64) int32_t dst[4 * SkRasterPipeline_kMaxStride_highp] = {};
    static_assert(std::size(initial) == (4 * SkRasterPipeline_kMaxStride_highp));

    SkRasterPipeline_<256> p;
    p.append(SkRasterPipelineOp::load_dst, initial);
    p.append(SkRasterPipelineOp::mask_off_loop_mask);
    p.append(SkRasterPipelineOp::store_dst, dst);
    p.run(0,0,SkOpts::raster_pipeline_highp_stride,1);

    const int dr = 0 * SkOpts::raster_pipeline_highp_stride;
    const int dg = 1 * SkOpts::raster_pipeline_highp_stride;
    const int db = 2 * SkOpts::raster_pipeline_highp_stride;
    const int da = 3 * SkOpts::raster_pipeline_highp_stride;
    for (size_t index = 0; index < SkOpts::raster_pipeline_highp_stride; ++index) {
        // `dg` should have masked off any lanes that are currently executing.
        int32_t expected = initial[dg + index] & ~initial[da + index];
        REPORTER_ASSERT(r, dst[dg + index] == expected);

        // `da` should contain `dr & dg & gb`.
        expected = dst[dr + index] & dst[dg + index] & dst[db + index];
        REPORTER_ASSERT(r, dst[da + index] == expected);
    }
}

DEF_TEST(SkRasterPipeline_MaskOffReturnMask, r) {
    alignas(64) int32_t initial[]  = {~0, ~0, ~0, ~0, ~0,  0, ~0, ~0,  // dr (condition)
                                      ~0,  0, ~0, ~0,  0,  0,  0, ~0,  // dg (loop)
                                      ~0, ~0,  0, ~0,  0,  0, ~0, ~0,  // db (return)
                                      ~0,  0,  0, ~0,  0,  0,  0, ~0}; // da (combined)
    alignas(64) int32_t dst[4 * SkRasterPipeline_kMaxStride_highp] = {};
    static_assert(std::size(initial) == (4 * SkRasterPipeline_kMaxStride_highp));

    SkRasterPipeline_<256> p;
    p.append(SkRasterPipelineOp::load_dst, initial);
    p.append(SkRasterPipelineOp::mask_off_return_mask);
    p.append(SkRasterPipelineOp::store_dst, dst);
    p.run(0,0,SkOpts::raster_pipeline_highp_stride,1);

    const int dr = 0 * SkOpts::raster_pipeline_highp_stride;
    const int dg = 1 * SkOpts::raster_pipeline_highp_stride;
    const int db = 2 * SkOpts::raster_pipeline_highp_stride;
    const int da = 3 * SkOpts::raster_pipeline_highp_stride;
    for (size_t index = 0; index < SkOpts::raster_pipeline_highp_stride; ++index) {
        // `db` should have masked off any lanes that are currently executing.
        int32_t expected = initial[db + index] & ~initial[da + index];
        REPORTER_ASSERT(r, dst[db + index] == expected);

        // `da` should contain `dr & dg & gb`.
        expected = dst[dr + index] & dst[dg + index] & dst[db + index];
        REPORTER_ASSERT(r, dst[da + index] == expected);
    }
}

DEF_TEST(SkRasterPipeline_InitLaneMasks, r) {
    for (size_t width = 1; width <= SkOpts::raster_pipeline_highp_stride; ++width) {
        SkRasterPipeline_<256> p;

        // Initialize dRGBA to unrelated values.
        SkRasterPipeline_UniformColorCtx uniformCtx;
        uniformCtx.a = 0.0f;
        uniformCtx.r = 0.25f;
        uniformCtx.g = 0.50f;
        uniformCtx.b = 0.75f;
        p.append(SkRasterPipelineOp::uniform_color_dst, &uniformCtx);

        // Overwrite dRGB with lane masks up to the tail width.
        p.append(SkRasterPipelineOp::init_lane_masks);

        // Use the store_dst command to write out dRGBA for inspection.
        alignas(64) int32_t dRGBA[4 * SkRasterPipeline_kMaxStride_highp] = {};
        p.append(SkRasterPipelineOp::store_dst, dRGBA);

        // Execute our program.
        p.run(0,0,width,1);

        // Initialized data should look like on/on/on/on (RGBA are all set) and is
        // striped by the raster pipeline stride because we wrote it using store_dst.
        size_t index = 0;
        int32_t* channelR = dRGBA;
        int32_t* channelG = channelR + SkOpts::raster_pipeline_highp_stride;
        int32_t* channelB = channelG + SkOpts::raster_pipeline_highp_stride;
        int32_t* channelA = channelB + SkOpts::raster_pipeline_highp_stride;
        for (; index < width; ++index) {
            REPORTER_ASSERT(r, *channelR++ == ~0);
            REPORTER_ASSERT(r, *channelG++ == ~0);
            REPORTER_ASSERT(r, *channelB++ == ~0);
            REPORTER_ASSERT(r, *channelA++ == ~0);
        }

        // The rest of the output array should be untouched (all zero).
        for (; index < SkOpts::raster_pipeline_highp_stride; ++index) {
            REPORTER_ASSERT(r, *channelR++ == 0);
            REPORTER_ASSERT(r, *channelG++ == 0);
            REPORTER_ASSERT(r, *channelB++ == 0);
            REPORTER_ASSERT(r, *channelA++ == 0);
        }
    }
}

DEF_TEST(SkRasterPipeline_CopyFromIndirectUnmasked, r) {
    // Allocate space for 5 source slots, and 5 dest slots.
    alignas(64) float src[5 * SkRasterPipeline_kMaxStride_highp];
    alignas(64) float dst[5 * SkRasterPipeline_kMaxStride_highp];

    // Test with various mixes of indirect offsets.
    static_assert(SkRasterPipeline_kMaxStride_highp == 8);
    alignas(64) const uint32_t kOffsets1[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    alignas(64) const uint32_t kOffsets2[8] = {2, 2, 2, 2, 2, 2, 2, 2};
    alignas(64) const uint32_t kOffsets3[8] = {0, 2, 0, 2, 0, 2, 0, 2};
    alignas(64) const uint32_t kOffsets4[8] = {99, 99, 0, 0, 99, 99, 0, 0};

    const int N = SkOpts::raster_pipeline_highp_stride;

    for (const uint32_t* offsets : {kOffsets1, kOffsets2, kOffsets3, kOffsets4}) {
        for (int copySize = 1; copySize <= 5; ++copySize) {
            // Initialize the destination slots to 0,1,2.. and the source slots to 1000,1001,1002...
            std::iota(&dst[0], &dst[5 * N], 0.0f);
            std::iota(&src[0], &src[5 * N], 1000.0f);

            // Run `copy_from_indirect_unmasked` over our data.
            SkArenaAlloc alloc(/*firstHeapAllocation=*/256);
            SkRasterPipeline p(&alloc);
            auto* ctx = alloc.make<SkRasterPipeline_CopyIndirectCtx>();
            ctx->dst = &dst[0];
            ctx->src = &src[0];
            ctx->indirectOffset = offsets;
            ctx->indirectLimit = 5 - copySize;
            ctx->slots = copySize;

            p.append(SkRasterPipelineOp::copy_from_indirect_unmasked, ctx);
            p.run(0,0,N,1);

            // If the offset plus copy-size would overflow the source data, the results don't
            // matter; indexing off the end of the buffer is UB, and we don't make any promises
            // about the values you get. If we didn't crash, that's success. (In practice, we
            // will have clamped the source pointer so that we don't read past the end.)
            int maxOffset = *std::max_element(offsets, offsets + N);
            if (copySize + maxOffset > 5) {
                continue;
            }

            // Verify that the destination has been overwritten in the mask-on fields, and has
            // not been overwritten in the mask-off fields, for each destination slot.
            float expectedUnchanged = 0.0f;
            float expectedFromZero = src[0 * N], expectedFromTwo = src[2 * N];
            float* destPtr = dst;
            for (int checkSlot = 0; checkSlot < 5; ++checkSlot) {
                for (int checkLane = 0; checkLane < N; ++checkLane) {
                    if (checkSlot < copySize) {
                        if (offsets[checkLane] == 0) {
                            REPORTER_ASSERT(r, *destPtr == expectedFromZero);
                        } else if (offsets[checkLane] == 2) {
                            REPORTER_ASSERT(r, *destPtr == expectedFromTwo);
                        } else {
                            ERRORF(r, "unexpected offset value");
                        }
                    } else {
                        REPORTER_ASSERT(r, *destPtr == expectedUnchanged);
                    }

                    ++destPtr;
                    expectedUnchanged += 1.0f;
                    expectedFromZero += 1.0f;
                    expectedFromTwo += 1.0f;
                }
            }
        }
    }
}

DEF_TEST(SkRasterPipeline_CopyFromIndirectUniformUnmasked, r) {
    // Allocate space for 5 source uniform values, and 5 dest slots.
    // (Note that unlike slots, uniforms don't use multiple lanes per value.)
    alignas(64) float src[5];
    alignas(64) float dst[5 * SkRasterPipeline_kMaxStride_highp];

    // Test with various mixes of indirect offsets.
    static_assert(SkRasterPipeline_kMaxStride_highp == 8);
    alignas(64) const uint32_t kOffsets1[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    alignas(64) const uint32_t kOffsets2[8] = {2, 2, 2, 2, 2, 2, 2, 2};
    alignas(64) const uint32_t kOffsets3[8] = {0, 2, 0, 2, 0, 2, 0, 2};
    alignas(64) const uint32_t kOffsets4[8] = {99, ~99u, 0, 0, ~99u, 99, 0, 0};

    const int N = SkOpts::raster_pipeline_highp_stride;

    for (const uint32_t* offsets : {kOffsets1, kOffsets2, kOffsets3, kOffsets4}) {
        for (int copySize = 1; copySize <= 5; ++copySize) {
            // Initialize the destination slots to 0,1,2.. and the source uniforms to
            // 1000,1001,1002...
            std::iota(&dst[0], &dst[5 * N], 0.0f);
            std::iota(&src[0], &src[5], 1000.0f);

            // Run `copy_from_indirect_unmasked` over our data.
            SkArenaAlloc alloc(/*firstHeapAllocation=*/256);
            SkRasterPipeline p(&alloc);
            auto* ctx = alloc.make<SkRasterPipeline_CopyIndirectCtx>();
            ctx->dst = &dst[0];
            ctx->src = &src[0];
            ctx->indirectOffset = offsets;
            ctx->indirectLimit = 5 - copySize;
            ctx->slots = copySize;

            p.append(SkRasterPipelineOp::copy_from_indirect_uniform_unmasked, ctx);
            p.run(0,0,N,1);

            // If the offset plus copy-size would overflow the source data, the results don't
            // matter; indexing off the end of the buffer is UB, and we don't make any promises
            // about the values you get. If we didn't crash, that's success. (In practice, we
            // will have clamped the source pointer so that we don't read past the end.)
            uint32_t maxOffset = *std::max_element(offsets, offsets + N);
            if (copySize + maxOffset > 5) {
                continue;
            }

            // Verify that the destination has been overwritten in each slot.
            float expectedUnchanged = 0.0f;
            float expectedFromZero = src[0], expectedFromTwo = src[2];
            float* destPtr = dst;
            for (int checkSlot = 0; checkSlot < 5; ++checkSlot) {
                for (int checkLane = 0; checkLane < N; ++checkLane) {
                    if (checkSlot < copySize) {
                        if (offsets[checkLane] == 0) {
                            REPORTER_ASSERT(r, *destPtr == expectedFromZero);
                        } else if (offsets[checkLane] == 2) {
                            REPORTER_ASSERT(r, *destPtr == expectedFromTwo);
                        } else {
                            ERRORF(r, "unexpected offset value");
                        }
                    } else {
                        REPORTER_ASSERT(r, *destPtr == expectedUnchanged);
                    }

                    ++destPtr;
                    expectedUnchanged += 1.0f;
                }
                expectedFromZero += 1.0f;
                expectedFromTwo += 1.0f;
            }
        }
    }
}

DEF_TEST(SkRasterPipeline_CopyToIndirectMasked, r) {
    // Allocate space for 5 source slots, and 5 dest slots.
    alignas(64) float src[5 * SkRasterPipeline_kMaxStride_highp];
    alignas(64) float dst[5 * SkRasterPipeline_kMaxStride_highp];

    // Test with various mixes of indirect offsets.
    static_assert(SkRasterPipeline_kMaxStride_highp == 8);
    alignas(64) const uint32_t kOffsets1[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    alignas(64) const uint32_t kOffsets2[8] = {2, 2, 2, 2, 2, 2, 2, 2};
    alignas(64) const uint32_t kOffsets3[8] = {0, 2, 0, 2, 0, 2, 0, 2};
    alignas(64) const uint32_t kOffsets4[8] = {99, ~99u, 0, 0, ~99u, 99, 0, 0};

    // Test with various masks.
    alignas(64) const int32_t kMask1[8]  = {~0, ~0, ~0, ~0, ~0,  0, ~0, ~0};
    alignas(64) const int32_t kMask2[8]  = {~0,  0, ~0, ~0,  0,  0,  0, ~0};
    alignas(64) const int32_t kMask3[8]  = {~0, ~0,  0, ~0,  0,  0, ~0, ~0};
    alignas(64) const int32_t kMask4[8]  = { 0,  0,  0,  0,  0,  0,  0,  0};

    const int N = SkOpts::raster_pipeline_highp_stride;

    for (const int32_t* mask : {kMask1, kMask2, kMask3, kMask4}) {
        for (const uint32_t* offsets : {kOffsets1, kOffsets2, kOffsets3, kOffsets4}) {
            for (int copySize = 1; copySize <= 5; ++copySize) {
                // Initialize the destination slots to 0,1,2.. and the source slots to
                // 1000,1001,1002...
                std::iota(&dst[0], &dst[5 * N], 0.0f);
                std::iota(&src[0], &src[5 * N], 1000.0f);

                // Run `copy_to_indirect_masked` over our data.
                SkArenaAlloc alloc(/*firstHeapAllocation=*/256);
                SkRasterPipeline p(&alloc);
                auto* ctx = alloc.make<SkRasterPipeline_CopyIndirectCtx>();
                ctx->dst = &dst[0];
                ctx->src = &src[0];
                ctx->indirectOffset = offsets;
                ctx->indirectLimit = 5 - copySize;
                ctx->slots = copySize;

                p.append(SkRasterPipelineOp::init_lane_masks);
                p.append(SkRasterPipelineOp::load_condition_mask, mask);
                p.append(SkRasterPipelineOp::copy_to_indirect_masked, ctx);
                p.run(0,0,N,1);

                // If the offset plus copy-size would overflow the destination, the results don't
                // matter; indexing off the end of the buffer is UB, and we don't make any promises
                // about the values you get. If we didn't crash, that's success. (In practice, we
                // will have clamped the destination pointer so that we don't read past the end.)
                uint32_t maxOffset = *std::max_element(offsets, offsets + N);
                if (copySize + maxOffset > 5) {
                    continue;
                }

                // Verify that the destination has been overwritten in the mask-on fields, and has
                // not been overwritten in the mask-off fields, for each destination slot.
                float expectedUnchanged = 0.0f;
                float expectedFromZero = src[0], expectedFromTwo = src[0] - (2 * N);
                float* destPtr = dst;
                int pos = 0;
                for (int checkSlot = 0; checkSlot < 5; ++checkSlot) {
                    for (int checkLane = 0; checkLane < N; ++checkLane) {
                        int rangeStart = offsets[checkLane] * N;
                        int rangeEnd   = (offsets[checkLane] + copySize) * N;
                        if (mask[checkLane] && pos >= rangeStart && pos < rangeEnd) {
                            if (offsets[checkLane] == 0) {
                                REPORTER_ASSERT(r, *destPtr == expectedFromZero);
                            } else if (offsets[checkLane] == 2) {
                                REPORTER_ASSERT(r, *destPtr == expectedFromTwo);
                            } else {
                                ERRORF(r, "unexpected offset value");
                            }
                        } else {
                            REPORTER_ASSERT(r, *destPtr == expectedUnchanged);
                        }

                        ++pos;
                        ++destPtr;
                        expectedUnchanged += 1.0f;
                        expectedFromZero += 1.0f;
                        expectedFromTwo += 1.0f;
                    }
                }
            }
        }
    }
}

DEF_TEST(SkRasterPipeline_SwizzleCopyToIndirectMasked, r) {
    // Allocate space for 5 source slots, and 5 dest slots.
    alignas(64) float src[5 * SkRasterPipeline_kMaxStride_highp];
    alignas(64) float dst[5 * SkRasterPipeline_kMaxStride_highp];

    // Test with various mixes of indirect offsets.
    static_assert(SkRasterPipeline_kMaxStride_highp == 8);
    alignas(64) const uint32_t kOffsets1[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    alignas(64) const uint32_t kOffsets2[8] = {2, 2, 2, 2, 2, 2, 2, 2};
    alignas(64) const uint32_t kOffsets3[8] = {0, 2, 0, 2, 0, 2, 0, 2};
    alignas(64) const uint32_t kOffsets4[8] = {99, ~99u, 0, 0, ~99u, 99, 0, 0};

    // Test with various masks.
    alignas(64) const int32_t kMask1[8]  = {~0, ~0, ~0, ~0, ~0,  0, ~0, ~0};
    alignas(64) const int32_t kMask2[8]  = {~0,  0, ~0, ~0,  0,  0,  0, ~0};
    alignas(64) const int32_t kMask3[8]  = {~0, ~0,  0, ~0,  0,  0, ~0, ~0};
    alignas(64) const int32_t kMask4[8]  = { 0,  0,  0,  0,  0,  0,  0,  0};

    // Test with various swizzle permutations.
    struct TestPattern {
        int swizzleSize;
        int swizzleUpperBound;
        uint16_t swizzle[4];
    };

    static const TestPattern kPatterns[] = {
        {1, 4, {3}},          // v.w    = (1)
        {2, 2, {1, 0}},       // v.yx   = (1,2)
        {3, 3, {2, 1, 0}},    // v.zyx  = (1,2,3)
        {4, 4, {3, 0, 1, 2}}, // v.wxyz = (1,2,3,4)
    };

    enum Result {
        kOutOfBounds = 0,
        kUnchanged = 1,
        S0 = 2,
        S1 = 3,
        S2 = 4,
        S3 = 5,
        S4 = 6,
    };

#define __ kUnchanged
#define XX kOutOfBounds
    static const Result kExpectationsAtZero[4][5] = {
    //  d[0].w = 1        d[0].yx = (1,2)   d[0].zyx = (1,2,3) d[0].wxyz = (1,2,3,4)
        {__,__,__,S0,__}, {S1,S0,__,__,__}, {S2,S1,S0,__,__},  {S1,S2,S3,S0,__},
    };
    static const Result kExpectationsAtTwo[4][5] = {
    //  d[2].w = 1        d[2].yx = (1,2)   d[2].zyx = (1,2,3) d[2].wxyz = (1,2,3,4)
        {XX,XX,XX,XX,XX}, {__,__,S1,S0,__}, {__,__,S2,S1,S0},  {XX,XX,XX,XX,XX},
    };
#undef __
#undef XX

    const int N = SkOpts::raster_pipeline_highp_stride;

    for (const int32_t* mask : {kMask1, kMask2, kMask3, kMask4}) {
        for (const uint32_t* offsets : {kOffsets1, kOffsets2, kOffsets3, kOffsets4}) {
            for (size_t patternIndex = 0; patternIndex < std::size(kPatterns); ++patternIndex) {
                const TestPattern& pattern = kPatterns[patternIndex];

                // Initialize the destination slots to 0,1,2.. and the source slots to
                // 1000,1001,1002...
                std::iota(&dst[0], &dst[5 * N], 0.0f);
                std::iota(&src[0], &src[5 * N], 1000.0f);

                // Run `swizzle_copy_to_indirect_masked` over our data.
                SkArenaAlloc alloc(/*firstHeapAllocation=*/256);
                SkRasterPipeline p(&alloc);
                auto* ctx = alloc.make<SkRasterPipeline_SwizzleCopyIndirectCtx>();
                ctx->dst = &dst[0];
                ctx->src = &src[0];
                ctx->indirectOffset = offsets;
                ctx->indirectLimit = 5 - pattern.swizzleUpperBound;
                ctx->slots = pattern.swizzleSize;
                ctx->offsets[0] = pattern.swizzle[0] * N * sizeof(float);
                ctx->offsets[1] = pattern.swizzle[1] * N * sizeof(float);
                ctx->offsets[2] = pattern.swizzle[2] * N * sizeof(float);
                ctx->offsets[3] = pattern.swizzle[3] * N * sizeof(float);

                p.append(SkRasterPipelineOp::init_lane_masks);
                p.append(SkRasterPipelineOp::load_condition_mask, mask);
                p.append(SkRasterPipelineOp::swizzle_copy_to_indirect_masked, ctx);
                p.run(0,0,N,1);

                // If the offset plus copy-size would overflow the destination, the results don't
                // matter; indexing off the end of the buffer is UB, and we don't make any promises
                // about the values you get. If we didn't crash, that's success. (In practice, we
                // will have clamped the destination pointer so that we don't read past the end.)
                uint32_t maxOffset = *std::max_element(offsets, offsets + N);
                if (pattern.swizzleUpperBound + maxOffset > 5) {
                    continue;
                }

                // Verify that the destination has been overwritten in the mask-on fields, and has
                // not been overwritten in the mask-off fields, for each destination slot.
                float expectedUnchanged = 0.0f;
                float* destPtr = dst;
                for (int checkSlot = 0; checkSlot < 5; ++checkSlot) {
                    for (int checkLane = 0; checkLane < N; ++checkLane) {
                        Result expectedType = kUnchanged;
                        if (offsets[checkLane] == 0) {
                            expectedType = kExpectationsAtZero[patternIndex][checkSlot];
                        } else if (offsets[checkLane] == 2) {
                            expectedType = kExpectationsAtTwo[patternIndex][checkSlot];
                        }
                        if (!mask[checkLane]) {
                            expectedType = kUnchanged;
                        }
                        switch (expectedType) {
                            case kOutOfBounds: // out of bounds; ignore result
                                break;
                            case kUnchanged:
                                REPORTER_ASSERT(r, *destPtr == expectedUnchanged);
                                break;
                            case S0: // destination should match source 0
                                REPORTER_ASSERT(r, *destPtr == src[0*N + checkLane]);
                                break;
                            case S1: // destination should match source 1
                                REPORTER_ASSERT(r, *destPtr == src[1*N + checkLane]);
                                break;
                            case S2: // destination should match source 2
                                REPORTER_ASSERT(r, *destPtr == src[2*N + checkLane]);
                                break;
                            case S3: // destination should match source 3
                                REPORTER_ASSERT(r, *destPtr == src[3*N + checkLane]);
                                break;
                            case S4: // destination should match source 4
                                REPORTER_ASSERT(r, *destPtr == src[4*N + checkLane]);
                                break;
                        }

                        ++destPtr;
                        expectedUnchanged += 1.0f;
                    }
                }
            }
        }
    }
}

DEF_TEST(SkRasterPipeline_CopySlotsMasked, r) {
    // Allocate space for 5 source slots and 5 dest slots.
    alignas(64) float slots[10 * SkRasterPipeline_kMaxStride_highp];
    const int srcIndex = 0, dstIndex = 5;

    struct CopySlotsOp {
        SkRasterPipelineOp stage;
        int numSlotsAffected;
    };

    static const CopySlotsOp kCopyOps[] = {
        {SkRasterPipelineOp::copy_slot_masked,    1},
        {SkRasterPipelineOp::copy_2_slots_masked, 2},
        {SkRasterPipelineOp::copy_3_slots_masked, 3},
        {SkRasterPipelineOp::copy_4_slots_masked, 4},
    };

    static_assert(SkRasterPipeline_kMaxStride_highp == 8);
    alignas(64) const int32_t kMask1[8] = {~0, ~0, ~0, ~0, ~0, ~0, ~0, ~0};
    alignas(64) const int32_t kMask2[8] = { 0,  0,  0,  0,  0,  0,  0,  0};
    alignas(64) const int32_t kMask3[8] = {~0,  0, ~0, ~0, ~0, ~0,  0, ~0};
    alignas(64) const int32_t kMask4[8] = { 0, ~0,  0,  0,  0, ~0, ~0,  0};

    const int N = SkOpts::raster_pipeline_highp_stride;

    for (const CopySlotsOp& op : kCopyOps) {
        for (const int32_t* mask : {kMask1, kMask2, kMask3, kMask4}) {
            // Initialize the destination slots to 0,1,2.. and the source slots to 1000,1001,1002...
            std::iota(&slots[N * dstIndex],  &slots[N * (dstIndex + 5)], 0.0f);
            std::iota(&slots[N * srcIndex],  &slots[N * (srcIndex + 5)], 1000.0f);

            // Run `copy_slots_masked` over our data.
            SkArenaAlloc alloc(/*firstHeapAllocation=*/256);
            SkRasterPipeline p(&alloc);
            auto* ctx = alloc.make<SkRasterPipeline_BinaryOpCtx>();
            ctx->dst = &slots[N * dstIndex];
            ctx->src = &slots[N * srcIndex];

            p.append(SkRasterPipelineOp::init_lane_masks);
            p.append(SkRasterPipelineOp::load_condition_mask, mask);
            p.append(op.stage, ctx);
            p.run(0,0,N,1);

            // Verify that the destination has been overwritten in the mask-on fields, and has not
            // been overwritten in the mask-off fields, for each destination slot.
            float expectedUnchanged = 0.0f, expectedChanged = 1000.0f;
            float* destPtr = &slots[N * dstIndex];
            for (int checkSlot = 0; checkSlot < 5; ++checkSlot) {
                for (int checkMask = 0; checkMask < N; ++checkMask) {
                    if (checkSlot < op.numSlotsAffected && mask[checkMask]) {
                        REPORTER_ASSERT(r, *destPtr == expectedChanged);
                    } else {
                        REPORTER_ASSERT(r, *destPtr == expectedUnchanged);
                    }

                    ++destPtr;
                    expectedUnchanged += 1.0f;
                    expectedChanged += 1.0f;
                }
            }
        }
    }
}

DEF_TEST(SkRasterPipeline_CopySlotsUnmasked, r) {
    // Allocate space for 5 source slots and 5 dest slots.
    alignas(64) float slots[10 * SkRasterPipeline_kMaxStride_highp];
    const int srcIndex = 0, dstIndex = 5;
    const int N = SkOpts::raster_pipeline_highp_stride;

    struct CopySlotsOp {
        SkRasterPipelineOp stage;
        int numSlotsAffected;
    };

    static const CopySlotsOp kCopyOps[] = {
        {SkRasterPipelineOp::copy_slot_unmasked,    1},
        {SkRasterPipelineOp::copy_2_slots_unmasked, 2},
        {SkRasterPipelineOp::copy_3_slots_unmasked, 3},
        {SkRasterPipelineOp::copy_4_slots_unmasked, 4},
    };

    for (const CopySlotsOp& op : kCopyOps) {
        // Initialize the destination slots to 0,1,2.. and the source slots to 1000,1001,1002...
        std::iota(&slots[N * dstIndex],  &slots[N * (dstIndex + 5)], 0.0f);
        std::iota(&slots[N * srcIndex],  &slots[N * (srcIndex + 5)], 1000.0f);

        // Run `copy_slots_unmasked` over our data.
        SkArenaAlloc alloc(/*firstHeapAllocation=*/256);
        SkRasterPipeline p(&alloc);
        auto* ctx = alloc.make<SkRasterPipeline_BinaryOpCtx>();
        ctx->dst = &slots[N * dstIndex];
        ctx->src = &slots[N * srcIndex];
        p.append(op.stage, ctx);
        p.run(0,0,1,1);

        // Verify that the destination has been overwritten in each slot.
        float expectedUnchanged = 0.0f, expectedChanged = 1000.0f;
        float* destPtr = &slots[N * dstIndex];
        for (int checkSlot = 0; checkSlot < 5; ++checkSlot) {
            for (int checkLane = 0; checkLane < N; ++checkLane) {
                if (checkSlot < op.numSlotsAffected) {
                    REPORTER_ASSERT(r, *destPtr == expectedChanged);
                } else {
                    REPORTER_ASSERT(r, *destPtr == expectedUnchanged);
                }

                ++destPtr;
                expectedUnchanged += 1.0f;
                expectedChanged += 1.0f;
            }
        }
    }
}

DEF_TEST(SkRasterPipeline_ZeroSlotsUnmasked, r) {
    // Allocate space for 5 dest slots.
    alignas(64) float slots[5 * SkRasterPipeline_kMaxStride_highp];
    const int N = SkOpts::raster_pipeline_highp_stride;

    struct ZeroSlotsOp {
        SkRasterPipelineOp stage;
        int numSlotsAffected;
    };

    static const ZeroSlotsOp kZeroOps[] = {
        {SkRasterPipelineOp::zero_slot_unmasked,    1},
        {SkRasterPipelineOp::zero_2_slots_unmasked, 2},
        {SkRasterPipelineOp::zero_3_slots_unmasked, 3},
        {SkRasterPipelineOp::zero_4_slots_unmasked, 4},
    };

    for (const ZeroSlotsOp& op : kZeroOps) {
        // Initialize the destination slots to 1,2,3...
        std::iota(&slots[0], &slots[5 * N], 1.0f);

        // Run `zero_slots_unmasked` over our data.
        SkArenaAlloc alloc(/*firstHeapAllocation=*/256);
        SkRasterPipeline p(&alloc);
        p.append(op.stage, &slots[0]);
        p.run(0,0,1,1);

        // Verify that the destination has been zeroed out in each slot.
        float expectedUnchanged = 1.0f;
        float* destPtr = &slots[0];
        for (int checkSlot = 0; checkSlot < 5; ++checkSlot) {
            for (int checkLane = 0; checkLane < N; ++checkLane) {
                if (checkSlot < op.numSlotsAffected) {
                    REPORTER_ASSERT(r, *destPtr == 0.0f);
                } else {
                    REPORTER_ASSERT(r, *destPtr == expectedUnchanged);
                }

                ++destPtr;
                expectedUnchanged += 1.0f;
            }
        }
    }
}

DEF_TEST(SkRasterPipeline_CopyConstants, r) {
    // Allocate space for 5 dest slots.
    alignas(64) float slots[5 * SkRasterPipeline_kMaxStride_highp];
    float constants[5];
    const int N = SkOpts::raster_pipeline_highp_stride;

    struct CopySlotsOp {
        SkRasterPipelineOp stage;
        int numSlotsAffected;
    };

    static const CopySlotsOp kCopyOps[] = {
        {SkRasterPipelineOp::copy_constant,    1},
        {SkRasterPipelineOp::copy_2_constants, 2},
        {SkRasterPipelineOp::copy_3_constants, 3},
        {SkRasterPipelineOp::copy_4_constants, 4},
    };

    for (const CopySlotsOp& op : kCopyOps) {
        // Initialize the destination slots to 1,2,3...
        std::iota(&slots[0], &slots[5 * N], 1.0f);
        // Initialize the constant buffer to 1000,1001,1002...
        std::iota(&constants[0], &constants[5], 1000.0f);

        // Run `copy_constants` over our data.
        SkArenaAlloc alloc(/*firstHeapAllocation=*/256);
        SkRasterPipeline p(&alloc);
        auto* ctx = alloc.make<SkRasterPipeline_BinaryOpCtx>();
        ctx->dst = slots;
        ctx->src = constants;
        p.append(op.stage, ctx);
        p.run(0,0,1,1);

        // Verify that our constants have been broadcast into each slot.
        float expectedUnchanged = 1.0f;
        float expectedChanged = 1000.0f;
        float* destPtr = &slots[0];
        for (int checkSlot = 0; checkSlot < 5; ++checkSlot) {
            for (int checkLane = 0; checkLane < N; ++checkLane) {
                if (checkSlot < op.numSlotsAffected) {
                    REPORTER_ASSERT(r, *destPtr == expectedChanged);
                } else {
                    REPORTER_ASSERT(r, *destPtr == expectedUnchanged);
                }

                ++destPtr;
                expectedUnchanged += 1.0f;
            }
            expectedChanged += 1.0f;
        }
    }
}

DEF_TEST(SkRasterPipeline_Swizzle, r) {
    // Allocate space for 4 dest slots.
    alignas(64) float slots[4 * SkRasterPipeline_kMaxStride_highp];
    const int N = SkOpts::raster_pipeline_highp_stride;

    struct TestPattern {
        SkRasterPipelineOp stage;
        uint16_t swizzle[4];
        uint16_t expectation[4];
    };
    static const TestPattern kPatterns[] = {
        {SkRasterPipelineOp::swizzle_1, {3},          {3, 1, 2, 3}}, // (1,2,3,4).w    = (4)
        {SkRasterPipelineOp::swizzle_2, {1, 0},       {1, 0, 2, 3}}, // (1,2,3,4).yx   = (2,1)
        {SkRasterPipelineOp::swizzle_3, {2, 2, 2},    {2, 2, 2, 3}}, // (1,2,3,4).zzz  = (3,3,3)
        {SkRasterPipelineOp::swizzle_4, {0, 0, 1, 2}, {0, 0, 1, 2}}, // (1,2,3,4).xxyz = (1,1,2,3)
    };
    static_assert(sizeof(TestPattern::swizzle) == sizeof(SkRasterPipeline_SwizzleCtx::offsets));

    for (const TestPattern& pattern : kPatterns) {
        // Initialize the destination slots to 0,1,2,3...
        std::iota(&slots[0], &slots[4 * N], 0.0f);

        // Apply the test-pattern swizzle.
        SkArenaAlloc alloc(/*firstHeapAllocation=*/256);
        SkRasterPipeline p(&alloc);
        SkRasterPipeline_SwizzleCtx ctx;
        ctx.ptr = slots;
        for (size_t index = 0; index < std::size(ctx.offsets); ++index) {
            ctx.offsets[index] = pattern.swizzle[index] * N * sizeof(float);
        }
        p.append(pattern.stage, &ctx);
        p.run(0,0,1,1);

        // Verify that the swizzle has been applied in each slot.
        float* destPtr = &slots[0];
        for (int checkSlot = 0; checkSlot < 4; ++checkSlot) {
            float expected = pattern.expectation[checkSlot] * N;
            for (int checkLane = 0; checkLane < N; ++checkLane) {
                REPORTER_ASSERT(r, *destPtr == expected);

                ++destPtr;
                expected += 1.0f;
            }
        }
    }
}

DEF_TEST(SkRasterPipeline_SwizzleCopy, r) {
    const int N = SkOpts::raster_pipeline_highp_stride;

    struct TestPattern {
        SkRasterPipelineOp op;
        uint16_t swizzle[4];
        uint16_t expectation[4];
    };
    constexpr uint16_t _ = ~0;
    static const TestPattern kPatterns[] = {
        {SkRasterPipelineOp::swizzle_copy_slot_masked,    {3,_,_,_}, {_,_,_,0}},//v.w    = (1)
        {SkRasterPipelineOp::swizzle_copy_2_slots_masked, {1,0,_,_}, {1,0,_,_}},//v.yx   = (1,2)
        {SkRasterPipelineOp::swizzle_copy_3_slots_masked, {2,3,0,_}, {2,_,0,1}},//v.zwy  = (1,2,3)
        {SkRasterPipelineOp::swizzle_copy_4_slots_masked, {3,0,1,2}, {1,2,3,0}},//v.wxyz = (1,2,3,4)
    };
    static_assert(sizeof(TestPattern::swizzle) == sizeof(SkRasterPipeline_SwizzleCopyCtx::offsets));

    for (const TestPattern& pattern : kPatterns) {
        // Allocate space for 4 dest slots, and initialize them to zero.
        alignas(64) float dest[4 * SkRasterPipeline_kMaxStride_highp] = {};

        // Allocate 4 source slots and initialize them to 1, 2, 3, 4...
        alignas(64) float source[4 * SkRasterPipeline_kMaxStride_highp] = {};
        std::iota(&source[0 * N], &source[4 * N], 1.0f);

        // Apply the dest-swizzle pattern.
        SkArenaAlloc alloc(/*firstHeapAllocation=*/256);
        SkRasterPipeline p(&alloc);
        SkRasterPipeline_SwizzleCopyCtx ctx = {};
        ctx.src = source;
        ctx.dst = dest;
        for (size_t index = 0; index < std::size(ctx.offsets); ++index) {
            if (pattern.swizzle[index] != _) {
                ctx.offsets[index] = pattern.swizzle[index] * N * sizeof(float);
            }
        }
        p.append(SkRasterPipelineOp::init_lane_masks);
        p.append(pattern.op, &ctx);
        p.run(0,0,N,1);

        // Verify that the swizzle has been applied in each slot.
        float* destPtr = &dest[0];
        for (int checkSlot = 0; checkSlot < 4; ++checkSlot) {
            for (int checkLane = 0; checkLane < N; ++checkLane) {
                if (pattern.expectation[checkSlot] == _) {
                    REPORTER_ASSERT(r, *destPtr == 0);
                } else {
                    int expectedIdx = pattern.expectation[checkSlot] * N + checkLane;
                    REPORTER_ASSERT(r, *destPtr == source[expectedIdx]);
                }

                ++destPtr;
            }
        }
    }
}

DEF_TEST(SkRasterPipeline_Shuffle, r) {
    // Allocate space for 16 dest slots.
    alignas(64) float slots[16 * SkRasterPipeline_kMaxStride_highp];
    const int N = SkOpts::raster_pipeline_highp_stride;

    struct TestPattern {
        int count;
        uint16_t shuffle[16];
        uint16_t expectation[16];
    };
    static const TestPattern kPatterns[] = {
        {9,  { 0,  3,  6,
               1,  4,  7,
               2,  5,  8, /* past end: */  0,  0,  0,  0,  0,  0,  0},
             { 0,  3,  6,
               1,  4,  7,
               2,  5,  8, /* unchanged: */ 9, 10, 11, 12, 13, 14, 15}},
        {16, { 0,  4,  8, 12,
               1,  5,  9, 13,
               2,  6, 10, 14,
               3,  7, 11, 15},
             { 0,  4,  8, 12,
               1,  5,  9, 13,
               2,  6, 10, 14,
               3,  7, 11, 15}},
    };
    static_assert(sizeof(TestPattern::shuffle) == sizeof(SkRasterPipeline_ShuffleCtx::offsets));

    for (const TestPattern& pattern : kPatterns) {
        // Initialize the destination slots to 1,2,3...
        std::iota(&slots[0], &slots[16 * N], 1.0f);

        // Apply the shuffle.
        SkArenaAlloc alloc(/*firstHeapAllocation=*/256);
        SkRasterPipeline p(&alloc);
        SkRasterPipeline_ShuffleCtx ctx;
        ctx.ptr = slots;
        ctx.count = pattern.count;
        for (size_t index = 0; index < std::size(ctx.offsets); ++index) {
            ctx.offsets[index] = pattern.shuffle[index] * N * sizeof(float);
        }
        p.append(SkRasterPipelineOp::shuffle, &ctx);
        p.run(0,0,1,1);

        // Verify that the shuffle has been applied in each slot.
        float* destPtr = &slots[0];
        for (int checkSlot = 0; checkSlot < 16; ++checkSlot) {
            float expected = pattern.expectation[checkSlot] * N + 1;
            for (int checkLane = 0; checkLane < N; ++checkLane) {
                REPORTER_ASSERT(r, *destPtr == expected);

                ++destPtr;
                expected += 1.0f;
            }
        }
    }
}

DEF_TEST(SkRasterPipeline_FloatArithmeticWithNSlots, r) {
    // Allocate space for 5 dest and 5 source slots.
    alignas(64) float slots[10 * SkRasterPipeline_kMaxStride_highp];
    const int N = SkOpts::raster_pipeline_highp_stride;

    struct ArithmeticOp {
        SkRasterPipelineOp stage;
        std::function<float(float, float)> verify;
    };

    static const ArithmeticOp kArithmeticOps[] = {
        {SkRasterPipelineOp::add_n_floats, [](float a, float b) { return a + b; }},
        {SkRasterPipelineOp::sub_n_floats, [](float a, float b) { return a - b; }},
        {SkRasterPipelineOp::mul_n_floats, [](float a, float b) { return a * b; }},
        {SkRasterPipelineOp::div_n_floats, [](float a, float b) { return a / b; }},
    };

    for (const ArithmeticOp& op : kArithmeticOps) {
        for (int numSlotsAffected = 1; numSlotsAffected <= 5; ++numSlotsAffected) {
            // Initialize the slot values to 1,2,3...
            std::iota(&slots[0], &slots[10 * N], 1.0f);

            // Run the arithmetic op over our data.
            SkArenaAlloc alloc(/*firstHeapAllocation=*/256);
            SkRasterPipeline p(&alloc);
            auto* ctx = alloc.make<SkRasterPipeline_BinaryOpCtx>();
            ctx->dst = &slots[0];
            ctx->src = &slots[numSlotsAffected * N];
            p.append(op.stage, ctx);
            p.run(0,0,1,1);

            // Verify that the affected slots now equal (1,2,3...) op (4,5,6...).
            float leftValue = 1.0f;
            float rightValue = float(numSlotsAffected * N) + 1.0f;
            float* destPtr = &slots[0];
            for (int checkSlot = 0; checkSlot < 10; ++checkSlot) {
                for (int checkLane = 0; checkLane < N; ++checkLane) {
                    if (checkSlot < numSlotsAffected) {
                        REPORTER_ASSERT(r, *destPtr == op.verify(leftValue, rightValue));
                    } else {
                        REPORTER_ASSERT(r, *destPtr == leftValue);
                    }

                    ++destPtr;
                    leftValue += 1.0f;
                    rightValue += 1.0f;
                }
            }
        }
    }
}

DEF_TEST(SkRasterPipeline_FloatArithmeticWithHardcodedSlots, r) {
    // Allocate space for 5 dest and 5 source slots.
    alignas(64) float slots[10 * SkRasterPipeline_kMaxStride_highp];
    const int N = SkOpts::raster_pipeline_highp_stride;

    struct ArithmeticOp {
        SkRasterPipelineOp stage;
        int numSlotsAffected;
        std::function<float(float, float)> verify;
    };

    static const ArithmeticOp kArithmeticOps[] = {
        {SkRasterPipelineOp::add_float,    1, [](float a, float b) { return a + b; }},
        {SkRasterPipelineOp::sub_float,    1, [](float a, float b) { return a - b; }},
        {SkRasterPipelineOp::mul_float,    1, [](float a, float b) { return a * b; }},
        {SkRasterPipelineOp::div_float,    1, [](float a, float b) { return a / b; }},

        {SkRasterPipelineOp::add_2_floats, 2, [](float a, float b) { return a + b; }},
        {SkRasterPipelineOp::sub_2_floats, 2, [](float a, float b) { return a - b; }},
        {SkRasterPipelineOp::mul_2_floats, 2, [](float a, float b) { return a * b; }},
        {SkRasterPipelineOp::div_2_floats, 2, [](float a, float b) { return a / b; }},

        {SkRasterPipelineOp::add_3_floats, 3, [](float a, float b) { return a + b; }},
        {SkRasterPipelineOp::sub_3_floats, 3, [](float a, float b) { return a - b; }},
        {SkRasterPipelineOp::mul_3_floats, 3, [](float a, float b) { return a * b; }},
        {SkRasterPipelineOp::div_3_floats, 3, [](float a, float b) { return a / b; }},

        {SkRasterPipelineOp::add_4_floats, 4, [](float a, float b) { return a + b; }},
        {SkRasterPipelineOp::sub_4_floats, 4, [](float a, float b) { return a - b; }},
        {SkRasterPipelineOp::mul_4_floats, 4, [](float a, float b) { return a * b; }},
        {SkRasterPipelineOp::div_4_floats, 4, [](float a, float b) { return a / b; }},
    };

    for (const ArithmeticOp& op : kArithmeticOps) {
        // Initialize the slot values to 1,2,3...
        std::iota(&slots[0], &slots[10 * N], 1.0f);

        // Run the arithmetic op over our data.
        SkArenaAlloc alloc(/*firstHeapAllocation=*/256);
        SkRasterPipeline p(&alloc);
        p.append(op.stage, &slots[0]);
        p.run(0,0,1,1);

        // Verify that the affected slots now equal (1,2,3...) op (4,5,6...).
        float leftValue = 1.0f;
        float rightValue = float(op.numSlotsAffected * N) + 1.0f;
        float* destPtr = &slots[0];
        for (int checkSlot = 0; checkSlot < 10; ++checkSlot) {
            for (int checkLane = 0; checkLane < N; ++checkLane) {
                if (checkSlot < op.numSlotsAffected) {
                    REPORTER_ASSERT(r, *destPtr == op.verify(leftValue, rightValue));
                } else {
                    REPORTER_ASSERT(r, *destPtr == leftValue);
                }

                ++destPtr;
                leftValue += 1.0f;
                rightValue += 1.0f;
            }
        }
    }
}

static int divide_unsigned(int a, int b) { return int(uint32_t(a) / uint32_t(b)); }
static int min_unsigned   (int a, int b) { return uint32_t(a) < uint32_t(b) ? a : b; }
static int max_unsigned   (int a, int b) { return uint32_t(a) > uint32_t(b) ? a : b; }

DEF_TEST(SkRasterPipeline_IntArithmeticWithNSlots, r) {
    // Allocate space for 5 dest and 5 source slots.
    alignas(64) int slots[10 * SkRasterPipeline_kMaxStride_highp];
    const int N = SkOpts::raster_pipeline_highp_stride;

    struct ArithmeticOp {
        SkRasterPipelineOp stage;
        std::function<int(int, int)> verify;
    };

    static const ArithmeticOp kArithmeticOps[] = {
        {SkRasterPipelineOp::add_n_ints,         [](int a, int b) { return a + b; }},
        {SkRasterPipelineOp::sub_n_ints,         [](int a, int b) { return a - b; }},
        {SkRasterPipelineOp::mul_n_ints,         [](int a, int b) { return a * b; }},
        {SkRasterPipelineOp::div_n_ints,         [](int a, int b) { return a / b; }},
        {SkRasterPipelineOp::div_n_uints,        divide_unsigned},
        {SkRasterPipelineOp::bitwise_and_n_ints, [](int a, int b) { return a & b; }},
        {SkRasterPipelineOp::bitwise_or_n_ints,  [](int a, int b) { return a | b; }},
        {SkRasterPipelineOp::bitwise_xor_n_ints, [](int a, int b) { return a ^ b; }},
        {SkRasterPipelineOp::min_n_ints,         [](int a, int b) { return a < b ? a : b; }},
        {SkRasterPipelineOp::min_n_uints,        min_unsigned},
        {SkRasterPipelineOp::max_n_ints,         [](int a, int b) { return a > b ? a : b; }},
        {SkRasterPipelineOp::max_n_uints,        max_unsigned},
    };

    for (const ArithmeticOp& op : kArithmeticOps) {
        for (int numSlotsAffected = 1; numSlotsAffected <= 5; ++numSlotsAffected) {
            // Initialize the slot values to 1,2,3...
            std::iota(&slots[0], &slots[10 * N], 1);
            int leftValue = slots[0];
            int rightValue = slots[numSlotsAffected * N];

            // Run the op (e.g. `add_n_ints`) over our data.
            SkArenaAlloc alloc(/*firstHeapAllocation=*/256);
            SkRasterPipeline p(&alloc);
            auto* ctx = alloc.make<SkRasterPipeline_BinaryOpCtx>();
            ctx->dst = (float*)&slots[0];
            ctx->src = (float*)&slots[numSlotsAffected * N];
            p.append(op.stage, ctx);
            p.run(0,0,1,1);

            // Verify that the affected slots now equal (1,2,3...) op (4,5,6...).
            int* destPtr = &slots[0];
            for (int checkSlot = 0; checkSlot < 10; ++checkSlot) {
                for (int checkLane = 0; checkLane < N; ++checkLane) {
                    if (checkSlot < numSlotsAffected) {
                        REPORTER_ASSERT(r, *destPtr == op.verify(leftValue, rightValue));
                    } else {
                        REPORTER_ASSERT(r, *destPtr == leftValue);
                    }

                    ++destPtr;
                    leftValue += 1;
                    rightValue += 1;
                }
            }
        }
    }
}

DEF_TEST(SkRasterPipeline_IntArithmeticWithHardcodedSlots, r) {
    // Allocate space for 5 dest and 5 source slots.
    alignas(64) int slots[10 * SkRasterPipeline_kMaxStride_highp];
    const int N = SkOpts::raster_pipeline_highp_stride;

    struct ArithmeticOp {
        SkRasterPipelineOp stage;
        int numSlotsAffected;
        std::function<int(int, int)> verify;
    };

    static const ArithmeticOp kArithmeticOps[] = {
        {SkRasterPipelineOp::add_int,            1, [](int a, int b) { return a + b; }},
        {SkRasterPipelineOp::sub_int,            1, [](int a, int b) { return a - b; }},
        {SkRasterPipelineOp::mul_int,            1, [](int a, int b) { return a * b; }},
        {SkRasterPipelineOp::div_int,            1, [](int a, int b) { return a / b; }},
        {SkRasterPipelineOp::div_uint,           1, divide_unsigned},
        {SkRasterPipelineOp::bitwise_and_int,    1, [](int a, int b) { return a & b; }},
        {SkRasterPipelineOp::bitwise_or_int,     1, [](int a, int b) { return a | b; }},
        {SkRasterPipelineOp::bitwise_xor_int,    1, [](int a, int b) { return a ^ b; }},
        {SkRasterPipelineOp::min_int,            1, [](int a, int b) { return a < b ? a: b; }},
        {SkRasterPipelineOp::min_uint,           1, min_unsigned},
        {SkRasterPipelineOp::max_int,            1, [](int a, int b) { return a > b ? a: b; }},
        {SkRasterPipelineOp::max_uint,           1, max_unsigned},

        {SkRasterPipelineOp::add_2_ints,         2, [](int a, int b) { return a + b; }},
        {SkRasterPipelineOp::sub_2_ints,         2, [](int a, int b) { return a - b; }},
        {SkRasterPipelineOp::mul_2_ints,         2, [](int a, int b) { return a * b; }},
        {SkRasterPipelineOp::div_2_ints,         2, [](int a, int b) { return a / b; }},
        {SkRasterPipelineOp::div_2_uints,        2, divide_unsigned},
        {SkRasterPipelineOp::bitwise_and_2_ints, 2, [](int a, int b) { return a & b; }},
        {SkRasterPipelineOp::bitwise_or_2_ints,  2, [](int a, int b) { return a | b; }},
        {SkRasterPipelineOp::bitwise_xor_2_ints, 2, [](int a, int b) { return a ^ b; }},
        {SkRasterPipelineOp::min_2_ints,         2, [](int a, int b) { return a < b ? a: b; }},
        {SkRasterPipelineOp::min_2_uints,        2, min_unsigned},
        {SkRasterPipelineOp::max_2_ints,         2, [](int a, int b) { return a > b ? a: b; }},
        {SkRasterPipelineOp::max_2_uints,        2, max_unsigned},

        {SkRasterPipelineOp::add_3_ints,         3, [](int a, int b) { return a + b; }},
        {SkRasterPipelineOp::sub_3_ints,         3, [](int a, int b) { return a - b; }},
        {SkRasterPipelineOp::mul_3_ints,         3, [](int a, int b) { return a * b; }},
        {SkRasterPipelineOp::div_3_ints,         3, [](int a, int b) { return a / b; }},
        {SkRasterPipelineOp::div_3_uints,        3, divide_unsigned},
        {SkRasterPipelineOp::bitwise_and_3_ints, 3, [](int a, int b) { return a & b; }},
        {SkRasterPipelineOp::bitwise_or_3_ints,  3, [](int a, int b) { return a | b; }},
        {SkRasterPipelineOp::bitwise_xor_3_ints, 3, [](int a, int b) { return a ^ b; }},
        {SkRasterPipelineOp::min_3_ints,         3, [](int a, int b) { return a < b ? a: b; }},
        {SkRasterPipelineOp::min_3_uints,        3, min_unsigned},
        {SkRasterPipelineOp::max_3_ints,         3, [](int a, int b) { return a > b ? a: b; }},
        {SkRasterPipelineOp::max_3_uints,        3, max_unsigned},

        {SkRasterPipelineOp::add_4_ints,         4, [](int a, int b) { return a + b; }},
        {SkRasterPipelineOp::sub_4_ints,         4, [](int a, int b) { return a - b; }},
        {SkRasterPipelineOp::mul_4_ints,         4, [](int a, int b) { return a * b; }},
        {SkRasterPipelineOp::div_4_ints,         4, [](int a, int b) { return a / b; }},
        {SkRasterPipelineOp::div_4_uints,        4, divide_unsigned},
        {SkRasterPipelineOp::bitwise_and_4_ints, 4, [](int a, int b) { return a & b; }},
        {SkRasterPipelineOp::bitwise_or_4_ints,  4, [](int a, int b) { return a | b; }},
        {SkRasterPipelineOp::bitwise_xor_4_ints, 4, [](int a, int b) { return a ^ b; }},
        {SkRasterPipelineOp::min_4_ints,         4, [](int a, int b) { return a < b ? a: b; }},
        {SkRasterPipelineOp::min_4_uints,        4, min_unsigned},
        {SkRasterPipelineOp::max_4_ints,         4, [](int a, int b) { return a > b ? a: b; }},
        {SkRasterPipelineOp::max_4_uints,        4, max_unsigned},
    };

    for (const ArithmeticOp& op : kArithmeticOps) {
        // Initialize the slot values to 1,2,3...
        std::iota(&slots[0], &slots[10 * N], 1);
        int leftValue = slots[0];
        int rightValue = slots[op.numSlotsAffected * N];

        // Run the op (e.g. `add_2_ints`) over our data.
        SkArenaAlloc alloc(/*firstHeapAllocation=*/256);
        SkRasterPipeline p(&alloc);
        p.append(op.stage, &slots[0]);
        p.run(0,0,1,1);

        // Verify that the affected slots now equal (1,2,3...) op (4,5,6...).
        int* destPtr = &slots[0];
        for (int checkSlot = 0; checkSlot < 10; ++checkSlot) {
            for (int checkLane = 0; checkLane < N; ++checkLane) {
                if (checkSlot < op.numSlotsAffected) {
                    REPORTER_ASSERT(r, *destPtr == op.verify(leftValue, rightValue));
                } else {
                    REPORTER_ASSERT(r, *destPtr == leftValue);
                }

                ++destPtr;
                leftValue += 1;
                rightValue += 1;
            }
        }
    }
}

DEF_TEST(SkRasterPipeline_CompareFloatsWithNSlots, r) {
    // Allocate space for 5 dest and 5 source slots.
    alignas(64) float slots[10 * SkRasterPipeline_kMaxStride_highp];
    const int N = SkOpts::raster_pipeline_highp_stride;

    struct CompareOp {
        SkRasterPipelineOp stage;
        std::function<bool(float, float)> verify;
    };

    static const CompareOp kCompareOps[] = {
        {SkRasterPipelineOp::cmpeq_n_floats, [](float a, float b) { return a == b; }},
        {SkRasterPipelineOp::cmpne_n_floats, [](float a, float b) { return a != b; }},
        {SkRasterPipelineOp::cmplt_n_floats, [](float a, float b) { return a <  b; }},
        {SkRasterPipelineOp::cmple_n_floats, [](float a, float b) { return a <= b; }},
    };

    for (const CompareOp& op : kCompareOps) {
        for (int numSlotsAffected = 1; numSlotsAffected <= 5; ++numSlotsAffected) {
            // Initialize the slot values to 0,1,2,0,1,2,0,1,2...
            for (int index = 0; index < 10 * N; ++index) {
                slots[index] = std::fmod(index, 3.0f);
            }

            float leftValue  = slots[0];
            float rightValue = slots[numSlotsAffected * N];

            // Run the comparison op over our data.
            SkArenaAlloc alloc(/*firstHeapAllocation=*/256);
            SkRasterPipeline p(&alloc);
            auto* ctx = alloc.make<SkRasterPipeline_BinaryOpCtx>();
            ctx->dst = &slots[0];
            ctx->src = &slots[numSlotsAffected * N];
            p.append(op.stage, ctx);
            p.run(0, 0, 1, 1);

            // Verify that the affected slots now contain "(0,1,2,0...) op (1,2,0,1...)".
            float* destPtr = &slots[0];
            for (int checkSlot = 0; checkSlot < 10; ++checkSlot) {
                for (int checkLane = 0; checkLane < N; ++checkLane) {
                    if (checkSlot < numSlotsAffected) {
                        bool compareIsTrue = op.verify(leftValue, rightValue);
                        REPORTER_ASSERT(r, *(int*)destPtr == (compareIsTrue ? ~0 : 0));
                    } else {
                        REPORTER_ASSERT(r, *destPtr == leftValue);
                    }

                    ++destPtr;
                    leftValue = std::fmod(leftValue + 1.0f, 3.0f);
                    rightValue = std::fmod(rightValue + 1.0f, 3.0f);
                }
            }
        }
    }
}

DEF_TEST(SkRasterPipeline_CompareFloatsWithHardcodedSlots, r) {
    // Allocate space for 5 dest and 5 source slots.
    alignas(64) float slots[10 * SkRasterPipeline_kMaxStride_highp];
    const int N = SkOpts::raster_pipeline_highp_stride;

    struct CompareOp {
        SkRasterPipelineOp stage;
        int numSlotsAffected;
        std::function<bool(float, float)> verify;
    };

    static const CompareOp kCompareOps[] = {
        {SkRasterPipelineOp::cmpeq_float,    1, [](float a, float b) { return a == b; }},
        {SkRasterPipelineOp::cmpne_float,    1, [](float a, float b) { return a != b; }},
        {SkRasterPipelineOp::cmplt_float,    1, [](float a, float b) { return a <  b; }},
        {SkRasterPipelineOp::cmple_float,    1, [](float a, float b) { return a <= b; }},

        {SkRasterPipelineOp::cmpeq_2_floats, 2, [](float a, float b) { return a == b; }},
        {SkRasterPipelineOp::cmpne_2_floats, 2, [](float a, float b) { return a != b; }},
        {SkRasterPipelineOp::cmplt_2_floats, 2, [](float a, float b) { return a <  b; }},
        {SkRasterPipelineOp::cmple_2_floats, 2, [](float a, float b) { return a <= b; }},

        {SkRasterPipelineOp::cmpeq_3_floats, 3, [](float a, float b) { return a == b; }},
        {SkRasterPipelineOp::cmpne_3_floats, 3, [](float a, float b) { return a != b; }},
        {SkRasterPipelineOp::cmplt_3_floats, 3, [](float a, float b) { return a <  b; }},
        {SkRasterPipelineOp::cmple_3_floats, 3, [](float a, float b) { return a <= b; }},

        {SkRasterPipelineOp::cmpeq_4_floats, 4, [](float a, float b) { return a == b; }},
        {SkRasterPipelineOp::cmpne_4_floats, 4, [](float a, float b) { return a != b; }},
        {SkRasterPipelineOp::cmplt_4_floats, 4, [](float a, float b) { return a <  b; }},
        {SkRasterPipelineOp::cmple_4_floats, 4, [](float a, float b) { return a <= b; }},
    };

    for (const CompareOp& op : kCompareOps) {
        // Initialize the slot values to 0,1,2,0,1,2,0,1,2...
        for (int index = 0; index < 10 * N; ++index) {
            slots[index] = std::fmod(index, 3.0f);
        }

        float leftValue  = slots[0];
        float rightValue = slots[op.numSlotsAffected * N];

        // Run the comparison op over our data.
        SkArenaAlloc alloc(/*firstHeapAllocation=*/256);
        SkRasterPipeline p(&alloc);
        p.append(op.stage, &slots[0]);
        p.run(0, 0, 1, 1);

        // Verify that the affected slots now contain "(0,1,2,0...) op (1,2,0,1...)".
        float* destPtr = &slots[0];
        for (int checkSlot = 0; checkSlot < 10; ++checkSlot) {
            for (int checkLane = 0; checkLane < N; ++checkLane) {
                if (checkSlot < op.numSlotsAffected) {
                    bool compareIsTrue = op.verify(leftValue, rightValue);
                    REPORTER_ASSERT(r, *(int*)destPtr == (compareIsTrue ? ~0 : 0));
                } else {
                    REPORTER_ASSERT(r, *destPtr == leftValue);
                }

                ++destPtr;
                leftValue = std::fmod(leftValue + 1.0f, 3.0f);
                rightValue = std::fmod(rightValue + 1.0f, 3.0f);
            }
        }
    }
}

static bool compare_lt_uint  (int a, int b) { return uint32_t(a) <  uint32_t(b); }
static bool compare_lteq_uint(int a, int b) { return uint32_t(a) <= uint32_t(b); }

DEF_TEST(SkRasterPipeline_CompareIntsWithNSlots, r) {
    // Allocate space for 5 dest and 5 source slots.
    alignas(64) int slots[10 * SkRasterPipeline_kMaxStride_highp];
    const int N = SkOpts::raster_pipeline_highp_stride;

    struct CompareOp {
        SkRasterPipelineOp stage;
        std::function<bool(int, int)> verify;
    };

    static const CompareOp kCompareOps[] = {
        {SkRasterPipelineOp::cmpeq_n_ints,  [](int a, int b) { return a == b; }},
        {SkRasterPipelineOp::cmpne_n_ints,  [](int a, int b) { return a != b; }},
        {SkRasterPipelineOp::cmplt_n_ints,  [](int a, int b) { return a <  b; }},
        {SkRasterPipelineOp::cmple_n_ints,  [](int a, int b) { return a <= b; }},
        {SkRasterPipelineOp::cmplt_n_uints, compare_lt_uint},
        {SkRasterPipelineOp::cmple_n_uints, compare_lteq_uint},
    };

    for (const CompareOp& op : kCompareOps) {
        for (int numSlotsAffected = 1; numSlotsAffected <= 5; ++numSlotsAffected) {
            // Initialize the slot values to -1,0,1,-1,0,1,-1,0,1,-1...
            for (int index = 0; index < 10 * N; ++index) {
                slots[index] = (index % 3) - 1;
            }

            int leftValue = slots[0];
            int rightValue = slots[numSlotsAffected * N];

            // Run the comparison op over our data.
            SkArenaAlloc alloc(/*firstHeapAllocation=*/256);
            SkRasterPipeline p(&alloc);
            auto* ctx = alloc.make<SkRasterPipeline_BinaryOpCtx>();
            ctx->dst = (float*)&slots[0];
            ctx->src = (float*)&slots[numSlotsAffected * N];
            p.append(op.stage, ctx);
            p.run(0, 0, 1, 1);

            // Verify that the affected slots now contain "(-1,0,1,-1...) op (0,1,-1,0...)".
            int* destPtr = &slots[0];
            for (int checkSlot = 0; checkSlot < 10; ++checkSlot) {
                for (int checkLane = 0; checkLane < N; ++checkLane) {
                    if (checkSlot < numSlotsAffected) {
                        bool compareIsTrue = op.verify(leftValue, rightValue);
                        REPORTER_ASSERT(r, *destPtr == (compareIsTrue ? ~0 : 0));
                    } else {
                        REPORTER_ASSERT(r, *destPtr == leftValue);
                    }

                    ++destPtr;
                    if (++leftValue == 2) {
                        leftValue = -1;
                    }
                    if (++rightValue == 2) {
                        rightValue = -1;
                    }
                }
            }
        }
    }
}

DEF_TEST(SkRasterPipeline_CompareIntsWithHardcodedSlots, r) {
    // Allocate space for 5 dest and 5 source slots.
    alignas(64) int slots[10 * SkRasterPipeline_kMaxStride_highp];
    const int N = SkOpts::raster_pipeline_highp_stride;

    struct CompareOp {
        SkRasterPipelineOp stage;
        int numSlotsAffected;
        std::function<bool(int, int)> verify;
    };

    static const CompareOp kCompareOps[] = {
        {SkRasterPipelineOp::cmpeq_int,     1, [](int a, int b) { return a == b; }},
        {SkRasterPipelineOp::cmpne_int,     1, [](int a, int b) { return a != b; }},
        {SkRasterPipelineOp::cmplt_int,     1, [](int a, int b) { return a <  b; }},
        {SkRasterPipelineOp::cmple_int,     1, [](int a, int b) { return a <= b; }},
        {SkRasterPipelineOp::cmplt_uint,    1, compare_lt_uint},
        {SkRasterPipelineOp::cmple_uint,    1, compare_lteq_uint},

        {SkRasterPipelineOp::cmpeq_2_ints,  2, [](int a, int b) { return a == b; }},
        {SkRasterPipelineOp::cmpne_2_ints,  2, [](int a, int b) { return a != b; }},
        {SkRasterPipelineOp::cmplt_2_ints,  2, [](int a, int b) { return a <  b; }},
        {SkRasterPipelineOp::cmple_2_ints,  2, [](int a, int b) { return a <= b; }},
        {SkRasterPipelineOp::cmplt_2_uints, 2, compare_lt_uint},
        {SkRasterPipelineOp::cmple_2_uints, 2, compare_lteq_uint},

        {SkRasterPipelineOp::cmpeq_3_ints,  3, [](int a, int b) { return a == b; }},
        {SkRasterPipelineOp::cmpne_3_ints,  3, [](int a, int b) { return a != b; }},
        {SkRasterPipelineOp::cmplt_3_ints,  3, [](int a, int b) { return a <  b; }},
        {SkRasterPipelineOp::cmple_3_ints,  3, [](int a, int b) { return a <= b; }},
        {SkRasterPipelineOp::cmplt_3_uints, 3, compare_lt_uint},
        {SkRasterPipelineOp::cmple_3_uints, 3, compare_lteq_uint},

        {SkRasterPipelineOp::cmpeq_4_ints,  4, [](int a, int b) { return a == b; }},
        {SkRasterPipelineOp::cmpne_4_ints,  4, [](int a, int b) { return a != b; }},
        {SkRasterPipelineOp::cmplt_4_ints,  4, [](int a, int b) { return a <  b; }},
        {SkRasterPipelineOp::cmple_4_ints,  4, [](int a, int b) { return a <= b; }},
        {SkRasterPipelineOp::cmplt_4_uints, 4, compare_lt_uint},
        {SkRasterPipelineOp::cmple_4_uints, 4, compare_lteq_uint},
    };

    for (const CompareOp& op : kCompareOps) {
        // Initialize the slot values to -1,0,1,-1,0,1,-1,0,1,-1...
        for (int index = 0; index < 10 * N; ++index) {
            slots[index] = (index % 3) - 1;
        }

        int leftValue = slots[0];
        int rightValue = slots[op.numSlotsAffected * N];

        // Run the comparison op over our data.
        SkArenaAlloc alloc(/*firstHeapAllocation=*/256);
        SkRasterPipeline p(&alloc);
        p.append(op.stage, &slots[0]);
        p.run(0, 0, 1, 1);

        // Verify that the affected slots now contain "(0,1,2,0...) op (1,2,0,1...)".
        int* destPtr = &slots[0];
        for (int checkSlot = 0; checkSlot < 10; ++checkSlot) {
            for (int checkLane = 0; checkLane < N; ++checkLane) {
                if (checkSlot < op.numSlotsAffected) {
                    bool compareIsTrue = op.verify(leftValue, rightValue);
                    REPORTER_ASSERT(r, *destPtr == (compareIsTrue ? ~0 : 0));
                } else {
                    REPORTER_ASSERT(r, *destPtr == leftValue);
                }

                ++destPtr;
                if (++leftValue == 2) {
                    leftValue = -1;
                }
                if (++rightValue == 2) {
                    rightValue = -1;
                }
            }
        }
    }
}

static int to_float(int a) { return sk_bit_cast<int>((float)a); }

DEF_TEST(SkRasterPipeline_UnaryIntOps, r) {
    // Allocate space for 5 slots.
    alignas(64) int slots[5 * SkRasterPipeline_kMaxStride_highp];
    const int N = SkOpts::raster_pipeline_highp_stride;

    struct UnaryOp {
        SkRasterPipelineOp stage;
        int numSlotsAffected;
        std::function<int(int)> verify;
    };

    static const UnaryOp kUnaryOps[] = {
        {SkRasterPipelineOp::bitwise_not_int,    1, [](int a) { return ~a; }},
        {SkRasterPipelineOp::bitwise_not_2_ints, 2, [](int a) { return ~a; }},
        {SkRasterPipelineOp::bitwise_not_3_ints, 3, [](int a) { return ~a; }},
        {SkRasterPipelineOp::bitwise_not_4_ints, 4, [](int a) { return ~a; }},

        {SkRasterPipelineOp::cast_to_float_from_int,    1, to_float},
        {SkRasterPipelineOp::cast_to_float_from_2_ints, 2, to_float},
        {SkRasterPipelineOp::cast_to_float_from_3_ints, 3, to_float},
        {SkRasterPipelineOp::cast_to_float_from_4_ints, 4, to_float},

        {SkRasterPipelineOp::abs_int,    1, [](int a) { return a < 0 ? -a : a; }},
        {SkRasterPipelineOp::abs_2_ints, 2, [](int a) { return a < 0 ? -a : a; }},
        {SkRasterPipelineOp::abs_3_ints, 3, [](int a) { return a < 0 ? -a : a; }},
        {SkRasterPipelineOp::abs_4_ints, 4, [](int a) { return a < 0 ? -a : a; }},
    };

    for (const UnaryOp& op : kUnaryOps) {
        // Initialize the slot values to -10,-9,-8...
        std::iota(&slots[0], &slots[5 * N], -10);
        int inputValue = slots[0];

        // Run the unary op over our data.
        SkArenaAlloc alloc(/*firstHeapAllocation=*/256);
        SkRasterPipeline p(&alloc);
        p.append(op.stage, &slots[0]);
        p.run(0, 0, 1, 1);

        // Verify that the destination slots have been updated.
        int* destPtr = &slots[0];
        for (int checkSlot = 0; checkSlot < 5; ++checkSlot) {
            for (int checkLane = 0; checkLane < N; ++checkLane) {
                if (checkSlot < op.numSlotsAffected) {
                    int expected = op.verify(inputValue);
                    REPORTER_ASSERT(r, *destPtr == expected);
                } else {
                    REPORTER_ASSERT(r, *destPtr == inputValue);
                }

                ++destPtr;
                ++inputValue;
            }
        }
    }
}

static float to_int(float a)  { return sk_bit_cast<float>((int)a); }
static float to_uint(float a) { return sk_bit_cast<float>((unsigned int)a); }

DEF_TEST(SkRasterPipeline_UnaryFloatOps, r) {
    // Allocate space for 5 slots.
    alignas(64) float slots[5 * SkRasterPipeline_kMaxStride_highp];
    const int N = SkOpts::raster_pipeline_highp_stride;

    struct UnaryOp {
        SkRasterPipelineOp stage;
        int numSlotsAffected;
        std::function<float(float)> verify;
    };

    static const UnaryOp kUnaryOps[] = {
        {SkRasterPipelineOp::cast_to_int_from_float,    1, to_int},
        {SkRasterPipelineOp::cast_to_int_from_2_floats, 2, to_int},
        {SkRasterPipelineOp::cast_to_int_from_3_floats, 3, to_int},
        {SkRasterPipelineOp::cast_to_int_from_4_floats, 4, to_int},

        {SkRasterPipelineOp::cast_to_uint_from_float,    1, to_uint},
        {SkRasterPipelineOp::cast_to_uint_from_2_floats, 2, to_uint},
        {SkRasterPipelineOp::cast_to_uint_from_3_floats, 3, to_uint},
        {SkRasterPipelineOp::cast_to_uint_from_4_floats, 4, to_uint},

        {SkRasterPipelineOp::abs_float,    1, [](float a) { return a < 0 ? -a : a; }},
        {SkRasterPipelineOp::abs_2_floats, 2, [](float a) { return a < 0 ? -a : a; }},
        {SkRasterPipelineOp::abs_3_floats, 3, [](float a) { return a < 0 ? -a : a; }},
        {SkRasterPipelineOp::abs_4_floats, 4, [](float a) { return a < 0 ? -a : a; }},

        {SkRasterPipelineOp::floor_float,    1, [](float a) { return floorf(a); }},
        {SkRasterPipelineOp::floor_2_floats, 2, [](float a) { return floorf(a); }},
        {SkRasterPipelineOp::floor_3_floats, 3, [](float a) { return floorf(a); }},
        {SkRasterPipelineOp::floor_4_floats, 4, [](float a) { return floorf(a); }},

        {SkRasterPipelineOp::ceil_float,    1, [](float a) { return ceilf(a); }},
        {SkRasterPipelineOp::ceil_2_floats, 2, [](float a) { return ceilf(a); }},
        {SkRasterPipelineOp::ceil_3_floats, 3, [](float a) { return ceilf(a); }},
        {SkRasterPipelineOp::ceil_4_floats, 4, [](float a) { return ceilf(a); }},
    };

    for (const UnaryOp& op : kUnaryOps) {
        // The result of some ops are undefined with negative inputs, so only test positive values.
        bool positiveOnly = (op.stage == SkRasterPipelineOp::cast_to_uint_from_float ||
                             op.stage == SkRasterPipelineOp::cast_to_uint_from_2_floats ||
                             op.stage == SkRasterPipelineOp::cast_to_uint_from_3_floats ||
                             op.stage == SkRasterPipelineOp::cast_to_uint_from_4_floats);

        float iotaStart = positiveOnly ? 1.0f : -9.75f;
        std::iota(&slots[0], &slots[5 * N], iotaStart);
        float inputValue = slots[0];

        // Run the unary op over our data.
        SkArenaAlloc alloc(/*firstHeapAllocation=*/256);
        SkRasterPipeline p(&alloc);
        p.append(op.stage, &slots[0]);
        p.run(0, 0, 1, 1);

        // Verify that the destination slots have been updated.
        float* destPtr = &slots[0];
        for (int checkSlot = 0; checkSlot < 5; ++checkSlot) {
            for (int checkLane = 0; checkLane < N; ++checkLane) {
                if (checkSlot < op.numSlotsAffected) {
                    float expected = op.verify(inputValue);
                    // The casting tests can generate NaN, depending on the input value, so a value
                    // match (via ==) might not succeed.
                    // The ceil tests can generate negative zeros _sometimes_, depending on the
                    // exact implementation of ceil(), so a bitwise match might not succeed.
                    // Because of this, we allow either a value match or a bitwise match.
                    bool bitwiseMatch = (0 == memcmp(destPtr, &expected, sizeof(float)));
                    bool valueMatch   = (*destPtr == expected);
                    REPORTER_ASSERT(r, valueMatch || bitwiseMatch);
                } else {
                    REPORTER_ASSERT(r, *destPtr == inputValue);
                }

                ++destPtr;
                ++inputValue;
            }
        }
    }
}

static float to_mix_weight(float value) {
    // Convert a positive value to a mix-weight (a number between 0 and 1).
    value /= 16.0f;
    return value - std::floor(value);
}

DEF_TEST(SkRasterPipeline_MixTest, r) {
    // Allocate space for 5 dest and 10 source slots.
    alignas(64) float slots[15 * SkRasterPipeline_kMaxStride_highp];
    const int N = SkOpts::raster_pipeline_highp_stride;

    struct MixOp {
        int numSlotsAffected;
        std::function<void(SkRasterPipeline*, SkArenaAlloc*)> append;
    };

    static const MixOp kMixOps[] = {
        {1, [&](SkRasterPipeline* p, SkArenaAlloc* alloc) {
                p->append(SkRasterPipelineOp::mix_float, slots);
            }},
        {2, [&](SkRasterPipeline* p, SkArenaAlloc* alloc) {
                p->append(SkRasterPipelineOp::mix_2_floats, slots);
            }},
        {3, [&](SkRasterPipeline* p, SkArenaAlloc* alloc) {
                p->append(SkRasterPipelineOp::mix_3_floats, slots);
            }},
        {4, [&](SkRasterPipeline* p, SkArenaAlloc* alloc) {
                p->append(SkRasterPipelineOp::mix_4_floats, slots);
            }},
        {5, [&](SkRasterPipeline* p, SkArenaAlloc* alloc) {
                auto* ctx = alloc->make<SkRasterPipeline_TernaryOpCtx>();
                ctx->dst = &slots[0];
                ctx->src0 = &slots[5 * N];
                ctx->src1 = &slots[10 * N];
                p->append(SkRasterPipelineOp::mix_n_floats, ctx);
            }},
    };

    for (const MixOp& op : kMixOps) {
        // Initialize the values to 1,2,3...
        std::iota(&slots[0], &slots[15 * N], 1.0f);

        float weightValue = slots[0];
        float fromValue   = slots[1 * op.numSlotsAffected * N];
        float toValue     = slots[2 * op.numSlotsAffected * N];

        // The first group of values (the weights) must be between zero and one.
        for (int idx = 0; idx < 1 * op.numSlotsAffected * N; ++idx) {
            slots[idx] = to_mix_weight(slots[idx]);
        }

        // Run the mix op over our data.
        SkArenaAlloc alloc(/*firstHeapAllocation=*/256);
        SkRasterPipeline p(&alloc);
        op.append(&p, &alloc);
        p.run(0,0,1,1);

        // Verify that the affected slots now equal mix({0.25, 0.3125...}, {3,4...}, {5,6...}, ).
        float* destPtr = &slots[0];
        for (int checkSlot = 0; checkSlot < op.numSlotsAffected; ++checkSlot) {
            for (int checkLane = 0; checkLane < N; ++checkLane) {
                float checkValue = (toValue - fromValue) * to_mix_weight(weightValue) + fromValue;
                REPORTER_ASSERT(r, *destPtr == checkValue);

                ++destPtr;
                fromValue += 1.0f;
                toValue += 1.0f;
                weightValue += 1.0f;
            }
        }
    }
}

DEF_TEST(SkRasterPipeline_Jump, r) {
    // Allocate space for 4 slots.
    alignas(64) float slots[4 * SkRasterPipeline_kMaxStride_highp] = {};
    const int N = SkOpts::raster_pipeline_highp_stride;

    alignas(64) static constexpr float kColorDarkRed[4] = {0.5f, 0.0f, 0.0f, 0.75f};
    alignas(64) static constexpr float kColorGreen[4]   = {0.0f, 1.0f, 0.0f, 1.0f};
    const int offset = 2;

    // Make a program which jumps over an append_constant_color op.
    SkArenaAlloc alloc(/*firstHeapAllocation=*/256);
    SkRasterPipeline p(&alloc);
    p.append_constant_color(&alloc, kColorGreen);      // assign green
    p.append(SkRasterPipelineOp::jump, &offset);       // jump over the dark-red color assignment
    p.append_constant_color(&alloc, kColorDarkRed);    // (not executed)
    p.append(SkRasterPipelineOp::store_src, slots);    // store the result so we can check it
    p.run(0,0,1,1);

    // Verify that the slots contain green.
    float* destPtr = &slots[0];
    for (int checkSlot = 0; checkSlot < 4; ++checkSlot) {
        for (int checkLane = 0; checkLane < N; ++checkLane) {
            REPORTER_ASSERT(r, *destPtr == kColorGreen[checkSlot]);
            ++destPtr;
        }
    }
}

DEF_TEST(SkRasterPipeline_BranchIfAllLanesActive, r) {
    // Allocate space for 4 slots.
    alignas(64) float slots[4 * SkRasterPipeline_kMaxStride_highp] = {};
    const int N = SkOpts::raster_pipeline_highp_stride;

    alignas(64) static constexpr float kTransparentHyperRed[4] = {2.0f, 0.0f, 0.0f, 0.5f};
    alignas(64) static constexpr float kColorRed[4]            = {1.0f, 0.0f, 0.0f, 1.0f};
    SkRasterPipeline_BranchCtx ctx;
    ctx.offset = 2;

    // An array of all zeros.
    alignas(64) static constexpr int32_t kNoLanesActive[4 * SkRasterPipeline_kMaxStride_highp] = {};

    // An array of all zeros, except for a single ~0 in the second dA slot.
    alignas(64) int32_t oneLaneActive[4 * SkRasterPipeline_kMaxStride_highp] = {};
    oneLaneActive[3*N + 1] = ~0;

    // Make a program which conditionally branches past two append_constant_color ops.
    for (int lanes = 1; lanes <= N; ++lanes) {
        SkArenaAlloc alloc(/*firstHeapAllocation=*/256);
        SkRasterPipeline p(&alloc);
        p.append(SkRasterPipelineOp::init_lane_masks);                 // execution mask all-on
        p.append_constant_color(&alloc, kTransparentHyperRed);         // set the color to hyper-red
        p.append(SkRasterPipelineOp::branch_if_all_lanes_active, &ctx);// skip past next line
        p.append(SkRasterPipelineOp::swap_rb);                         // (not executed)
        p.append(SkRasterPipelineOp::load_dst, oneLaneActive);         // set one lane active
        p.append(SkRasterPipelineOp::branch_if_all_lanes_active, &ctx);// do not skip past next line
        p.append(SkRasterPipelineOp::force_opaque);                    // set alpha to 1
        p.append(SkRasterPipelineOp::load_dst, kNoLanesActive);        // set no lanes active
        p.append(SkRasterPipelineOp::branch_if_all_lanes_active, &ctx);// do not skip past next line
        p.append(SkRasterPipelineOp::clamp_x_1);                       // clamp red to 1
        p.append(SkRasterPipelineOp::store_src, slots);                // store final color
        p.run(0,0,lanes,1);

        // Verify that the slots contain green.
        float* destPtr = &slots[0];
        for (int checkSlot = 0; checkSlot < 4; ++checkSlot) {
            for (int checkLane = 0; checkLane < N; ++checkLane) {
                REPORTER_ASSERT(r, *destPtr == kColorRed[checkSlot]);
                ++destPtr;
            }
        }
    }
}

DEF_TEST(SkRasterPipeline_BranchIfAnyLanesActive, r) {
    // Allocate space for 4 slots.
    alignas(64) float slots[4 * SkRasterPipeline_kMaxStride_highp] = {};
    const int N = SkOpts::raster_pipeline_highp_stride;

    alignas(64) static constexpr float kColorDarkRed[4] = {0.5f, 0.0f, 0.0f, 0.75f};
    alignas(64) static constexpr float kColorGreen[4]   = {0.0f, 1.0f, 0.0f, 1.0f};
    SkRasterPipeline_BranchCtx ctx;
    ctx.offset = 2;

    // An array of all zeros.
    alignas(64) static constexpr int32_t kNoLanesActive[4 * SkRasterPipeline_kMaxStride_highp] = {};

    // An array of all zeros, except for a single ~0 in the first dA slot.
    alignas(64) int32_t oneLaneActive[4 * SkRasterPipeline_kMaxStride_highp] = {};
    oneLaneActive[3*N] = ~0;

    // Make a program which conditionally branches past two append_constant_color ops.
    SkArenaAlloc alloc(/*firstHeapAllocation=*/256);
    SkRasterPipeline p(&alloc);
    p.append_constant_color(&alloc, kColorDarkRed);                    // set the color to dark red
    p.append(SkRasterPipelineOp::load_dst, kNoLanesActive);            // make no lanes active
    p.append(SkRasterPipelineOp::branch_if_any_lanes_active, &ctx);    // do not skip past next line
    p.append_constant_color(&alloc, kColorGreen);                      // set the color to green
    p.append(SkRasterPipelineOp::load_dst, oneLaneActive);             // set one lane active
    p.append(SkRasterPipelineOp::branch_if_any_lanes_active, &ctx);    // skip past next line
    p.append_constant_color(&alloc, kColorDarkRed);                    // (not executed)
    p.append(SkRasterPipelineOp::init_lane_masks);                     // set all lanes active
    p.append(SkRasterPipelineOp::branch_if_any_lanes_active, &ctx);    // skip past next line
    p.append_constant_color(&alloc, kColorDarkRed);                    // (not executed)
    p.append(SkRasterPipelineOp::store_src, slots);                    // store final color
    p.run(0,0,N,1);

    // Verify that the slots contain green.
    float* destPtr = &slots[0];
    for (int checkSlot = 0; checkSlot < 4; ++checkSlot) {
        for (int checkLane = 0; checkLane < N; ++checkLane) {
            REPORTER_ASSERT(r, *destPtr == kColorGreen[checkSlot]);
            ++destPtr;
        }
    }
}

DEF_TEST(SkRasterPipeline_BranchIfNoLanesActive, r) {
    // Allocate space for 4 slots.
    alignas(64) float slots[4 * SkRasterPipeline_kMaxStride_highp] = {};
    const int N = SkOpts::raster_pipeline_highp_stride;

    alignas(64) static constexpr float kColorBlack[4]   = {0.0f, 0.0f, 0.0f, 0.0f};
    alignas(64) static constexpr float kColorRed[4]     = {1.0f, 0.0f, 0.0f, 1.0f};
    alignas(64) static constexpr float kColorBlue[4]    = {0.0f, 0.0f, 1.0f, 1.0f};
    SkRasterPipeline_BranchCtx ctx;
    ctx.offset = 2;

    // An array of all zeros.
    alignas(64) static constexpr int32_t kNoLanesActive[4 * SkRasterPipeline_kMaxStride_highp] = {};

    // An array of all zeros, except for a single ~0 in the first dA slot.
    alignas(64) int32_t oneLaneActive[4 * SkRasterPipeline_kMaxStride_highp] = {};
    oneLaneActive[3*N] = ~0;

    // Make a program which conditionally branches past a append_constant_color op.
    SkArenaAlloc alloc(/*firstHeapAllocation=*/256);
    SkRasterPipeline p(&alloc);
    p.append_constant_color(&alloc, kColorBlack);                      // set the color to black
    p.append(SkRasterPipelineOp::init_lane_masks);                     // set all lanes active
    p.append(SkRasterPipelineOp::branch_if_no_lanes_active, &ctx);     // do not skip past next line
    p.append_constant_color(&alloc, kColorRed);                        // sets the color to red
    p.append(SkRasterPipelineOp::load_dst, oneLaneActive);             // set one lane active
    p.append(SkRasterPipelineOp::branch_if_no_lanes_active, &ctx);     // do not skip past next line
    p.append(SkRasterPipelineOp::swap_rb);                             // swap R and B (making blue)
    p.append(SkRasterPipelineOp::load_dst, kNoLanesActive);            // make no lanes active
    p.append(SkRasterPipelineOp::branch_if_no_lanes_active, &ctx);     // skip past next line
    p.append_constant_color(&alloc, kColorBlack);                      // (not executed)
    p.append(SkRasterPipelineOp::store_src, slots);                    // store final blue color
    p.run(0,0,N,1);

    // Verify that the slots contain blue.
    float* destPtr = &slots[0];
    for (int checkSlot = 0; checkSlot < 4; ++checkSlot) {
        for (int checkLane = 0; checkLane < N; ++checkLane) {
            REPORTER_ASSERT(r, *destPtr == kColorBlue[checkSlot]);
            ++destPtr;
        }
    }
}

DEF_TEST(SkRasterPipeline_BranchIfActiveLanesEqual, r) {
    // Allocate space for 4 slots.
    alignas(64) float slots[4 * SkRasterPipeline_kMaxStride_highp] = {};
    const int N = SkOpts::raster_pipeline_highp_stride;

    alignas(64) static constexpr float kColorBlack[4]   = {0.0f, 0.0f, 0.0f, 0.0f};
    alignas(64) static constexpr float kColorRed[4]     = {1.0f, 0.0f, 0.0f, 1.0f};

    // An array of all 6s.
    alignas(64) int allSixes[SkRasterPipeline_kMaxStride_highp] = {};
    std::fill(std::begin(allSixes), std::end(allSixes), 6);

    // An array of all 6s, except for a single 5 in one lane.
    alignas(64) int mostlySixesWithOneFive[SkRasterPipeline_kMaxStride_highp] = {};
    std::fill(std::begin(mostlySixesWithOneFive), std::end(mostlySixesWithOneFive), 6);
    mostlySixesWithOneFive[N - 1] = 5;

    // A condition mask with all lanes on except for the six-lane.
    alignas(64) int mask[SkRasterPipeline_kMaxStride_highp] = {};
    std::fill(std::begin(mask), std::end(mask), ~0);
    mask[N - 1] = 0;

    SkRasterPipeline_BranchIfEqualCtx matching; // comparing all-six vs five will match
    matching.offset = 2;
    matching.value = 5;
    matching.ptr = allSixes;

    SkRasterPipeline_BranchIfEqualCtx nonmatching;  // comparing mostly-six vs five won't match
    nonmatching.offset = 2;
    nonmatching.value = 5;
    nonmatching.ptr = mostlySixesWithOneFive;

    // Make a program which conditionally branches past a swap_rb op.
    SkArenaAlloc alloc(/*firstHeapAllocation=*/256);
    SkRasterPipeline p(&alloc);
    p.append_constant_color(&alloc, kColorBlack);                          // set the color to black
    p.append(SkRasterPipelineOp::init_lane_masks);                         // set all lanes active
    p.append(SkRasterPipelineOp::branch_if_no_active_lanes_eq, &nonmatching);// don't skip next line
    p.append_constant_color(&alloc, kColorRed);                            // set the color to red
    p.append(SkRasterPipelineOp::branch_if_no_active_lanes_eq, &matching); // do skip next line
    p.append(SkRasterPipelineOp::swap_rb);                                 // swap R and B (= blue)
    p.append(SkRasterPipelineOp::load_condition_mask, mask);               // mask off the six
    p.append(SkRasterPipelineOp::branch_if_no_active_lanes_eq, &nonmatching);// do skip next line
    p.append(SkRasterPipelineOp::white_color);                             // set the color to white
    p.append(SkRasterPipelineOp::store_src, slots);                        // store final red color
    p.run(0,0,SkOpts::raster_pipeline_highp_stride,1);

    // Verify that the slots contain red.
    float* destPtr = &slots[0];
    for (int checkSlot = 0; checkSlot < 4; ++checkSlot) {
        for (int checkLane = 0; checkLane < N; ++checkLane) {
            REPORTER_ASSERT(r, *destPtr == kColorRed[checkSlot]);
            ++destPtr;
        }
    }
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
    p.append(SkRasterPipelineOp::srcover);
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
    p.append(SkRasterPipelineOp::load_8888,  &src);
    p.append(SkRasterPipelineOp::store_8888, &dst);
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
            p.append(SkRasterPipelineOp::load_f32, &src);
            p.append(SkRasterPipelineOp::store_f32, &dst);
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
            p.append(SkRasterPipelineOp::load_rgf32, &src);
            p.append(SkRasterPipelineOp::store_f32, &dst);
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
            p.append(SkRasterPipelineOp::load_f32, &src);
            p.append(SkRasterPipelineOp::store_rgf32, &dst);
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
            p.append(SkRasterPipelineOp::load_f16, &src);
            p.append(SkRasterPipelineOp::store_f16, &dst);
            p.run(0,0, i,1);
            for (unsigned j = 0; j < i; j++) {
                for (int k = 0; k < 4; k++) {
                    REPORTER_ASSERT(r, buffer[j][k] == data[j][k]);
                }
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
            p.append(SkRasterPipelineOp::load_af16, &src);
            p.append(SkRasterPipelineOp::store_f16, &dst);
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
            p.append(SkRasterPipelineOp::load_f16, &src);
            p.append(SkRasterPipelineOp::store_af16, &dst);
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
            p.append(SkRasterPipelineOp::load_f16, &src);
            p.append(SkRasterPipelineOp::store_rgf16, &dst);
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
            p.append(SkRasterPipelineOp::load_rgf16, &src);
            p.append(SkRasterPipelineOp::store_f16, &dst);
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
            p.append(SkRasterPipelineOp::load_rg1616, &src);
            p.append(SkRasterPipelineOp::store_8888, &dst);
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

    {
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
            p.append(SkRasterPipelineOp::load_a16, &src);
            p.append(SkRasterPipelineOp::store_8888, &dst);
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
            p.append(SkRasterPipelineOp::load_8888, &src);
            p.append(SkRasterPipelineOp::store_a16, &dst);
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

    {
        alignas(8) uint16_t data[][4] = {
            {0x0000, 0x1000, 0x2000, 0x3000},
            {0x0001, 0x1001, 0x2001, 0x3001},
            {0x0002, 0x1002, 0x2002, 0x3002},
            {0x0003, 0x1003, 0x2003, 0x3003},
        };
        alignas(8) uint16_t buffer[4][4];
        SkRasterPipeline_MemoryCtx src = { &data[0][0], 0 },
                dst = { &buffer[0], 0 };

        for (unsigned i = 1; i <= 4; i++) {
            memset(buffer, 0xff, sizeof(buffer));
            SkRasterPipeline_<256> p;
            p.append(SkRasterPipelineOp::load_16161616, &src);
            p.append(SkRasterPipelineOp::swap_rb);
            p.append(SkRasterPipelineOp::store_16161616, &dst);
            p.run(0,0, i,1);
            for (unsigned j = 0; j < i; j++) {
                uint16_t expected[4] = {data[j][2], data[j][1], data[j][0], data[j][3]};
                REPORTER_ASSERT(r, !memcmp(&expected[0], &buffer[j], sizeof(expected)));
            }
            for (int j = i; j < 4; j++) {
                for (uint16_t u16 : buffer[j])
                REPORTER_ASSERT(r, u16 == 0xffff);
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
    p.append(SkRasterPipelineOp::load_8888,  &ptr);
    p.append(SkRasterPipelineOp::swap_rb);
    p.append(SkRasterPipelineOp::store_8888, &ptr);
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

        skgpu::Swizzle swizzle("g1b1");

        SkRasterPipeline_MemoryCtx ptr = { rg, 0 };
        SkRasterPipeline_<256> p;
        p.append(SkRasterPipelineOp::load_rg88,  &ptr);
        swizzle.apply(&p);
        p.append(SkRasterPipelineOp::store_rg88, &ptr);
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

        skgpu::Swizzle swizzle("0gra");

        uint16_t buffer[64][4];
        SkRasterPipeline_MemoryCtx src = { rg,     0 },
                                   dst = { buffer, 0};
        SkRasterPipeline_<256> p;
        p.append(SkRasterPipelineOp::load_rgf32,  &src);
        swizzle.apply(&p);
        p.append(SkRasterPipelineOp::store_f16, &dst);
        p.run(0,0,64,1);

        for (int i = 0; i < 64; i++) {
            uint16_t want[4] {
                h(0),
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
    p.append(SkRasterPipelineOp::load_8888,  &ptr);
    p.append(SkRasterPipelineOp::swap_rb);
    p.append(SkRasterPipelineOp::clamp_01);
    p.append(SkRasterPipelineOp::store_8888, &ptr);
    p.run(0,0,1,1);
}

// Helper struct that can be used to scrape stack addresses at different points in a pipeline
class StackCheckerCtx : SkRasterPipeline_CallbackCtx {
public:
    StackCheckerCtx() {
        this->fn = [](SkRasterPipeline_CallbackCtx* self, int active_pixels) {
            auto ctx = (StackCheckerCtx*)self;
            ctx->fStackAddrs.push_back(&active_pixels);
        };
    }

    enum class Behavior {
        kGrowth,
        kBaseline,
        kUnknown,
    };

    static Behavior GrowthBehavior() {
        // Only some stages use the musttail attribute, so we have no way of knowing what's going to
        // happen. In release builds, it's likely that the compiler will apply tail-call
        // optimization. Even in some debug builds (on Windows), we don't see stack growth.
        return Behavior::kUnknown;
    }

    // Call one of these two each time the checker callback is added:
    StackCheckerCtx* expectGrowth() {
        fExpectedBehavior.push_back(GrowthBehavior());
        return this;
    }

    StackCheckerCtx* expectBaseline() {
        fExpectedBehavior.push_back(Behavior::kBaseline);
        return this;
    }

    void validate(skiatest::Reporter* r) {
        REPORTER_ASSERT(r, fStackAddrs.size() == fExpectedBehavior.size());

        // This test is storing and comparing stack pointers (to dead stack frames) as a way of
        // measuring stack usage. Unsurprisingly, ASAN doesn't like that. HWASAN actually inserts
        // tag bytes in the pointers, causing them not to match. Newer versions of vanilla ASAN
        // also appear to salt the stack slightly, causing repeated calls to scrape different
        // addresses, even though $rsp is identical on each invocation of the lambda.
#if !defined(SK_SANITIZE_ADDRESS)
        void* baseline = fStackAddrs[0];
        for (size_t i = 1; i < fStackAddrs.size(); i++) {
            if (fExpectedBehavior[i] == Behavior::kGrowth) {
                REPORTER_ASSERT(r, fStackAddrs[i] != baseline);
            } else if (fExpectedBehavior[i] == Behavior::kBaseline) {
                REPORTER_ASSERT(r, fStackAddrs[i] == baseline);
            } else {
                // Unknown behavior, nothing we can assert here
            }
        }
#endif
    }

private:
    std::vector<void*>    fStackAddrs;
    std::vector<Behavior> fExpectedBehavior;
};

DEF_TEST(SkRasterPipeline_stack_rewind, r) {
    // This test verifies that we can control stack usage with stack_rewind

    // Without stack_rewind, we should (maybe) see stack growth
    {
        StackCheckerCtx stack;
        uint32_t rgba = 0xff0000ff;
        SkRasterPipeline_MemoryCtx ptr = { &rgba, 0 };

        SkRasterPipeline_<256> p;
        p.append(SkRasterPipelineOp::callback, stack.expectBaseline());
        p.append(SkRasterPipelineOp::load_8888,  &ptr);
        p.append(SkRasterPipelineOp::callback, stack.expectGrowth());
        p.append(SkRasterPipelineOp::swap_rb);
        p.append(SkRasterPipelineOp::callback, stack.expectGrowth());
        p.append(SkRasterPipelineOp::store_8888, &ptr);
        p.run(0,0,1,1);

        REPORTER_ASSERT(r, rgba == 0xffff0000); // Ensure the pipeline worked
        stack.validate(r);
    }

    // With stack_rewind, we should (always) be able to get back to baseline
    {
        StackCheckerCtx stack;
        uint32_t rgba = 0xff0000ff;
        SkRasterPipeline_MemoryCtx ptr = { &rgba, 0 };

        SkRasterPipeline_<256> p;
        p.append(SkRasterPipelineOp::callback, stack.expectBaseline());
        p.append(SkRasterPipelineOp::load_8888,  &ptr);
        p.append(SkRasterPipelineOp::callback, stack.expectGrowth());
        p.append_stack_rewind();
        p.append(SkRasterPipelineOp::callback, stack.expectBaseline());
        p.append(SkRasterPipelineOp::swap_rb);
        p.append(SkRasterPipelineOp::callback, stack.expectGrowth());
        p.append_stack_rewind();
        p.append(SkRasterPipelineOp::callback, stack.expectBaseline());
        p.append(SkRasterPipelineOp::store_8888, &ptr);
        p.run(0,0,1,1);

        REPORTER_ASSERT(r, rgba == 0xffff0000); // Ensure the pipeline worked
        stack.validate(r);
    }
}
