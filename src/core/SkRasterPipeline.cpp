/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkRasterPipeline.h"

#include "include/core/SkColorType.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkMatrix.h"
#include "include/private/base/SkTemplates.h"
#include "modules/skcms/skcms.h"
#include "src/base/SkVx.h"
#include "src/core/SkImageInfoPriv.h"
#include "src/core/SkOpts.h"

#include <algorithm>
#include <cstring>
#include <vector>

using namespace skia_private;
using Op = SkRasterPipelineOp;

bool gForceHighPrecisionRasterPipeline;

SkRasterPipeline::SkRasterPipeline(SkArenaAlloc* alloc) : fAlloc(alloc) {
    this->reset();
}
void SkRasterPipeline::reset() {
    fRewindCtx = nullptr;
    fStages    = nullptr;
    fNumStages = 0;
}

void SkRasterPipeline::append(SkRasterPipelineOp op, void* ctx) {
    SkASSERT(op != Op::uniform_color);            // Please use append_constant_color().
    SkASSERT(op != Op::unbounded_uniform_color);  // Please use append_constant_color().
    SkASSERT(op != Op::set_rgb);                  // Please use append_set_rgb().
    SkASSERT(op != Op::unbounded_set_rgb);        // Please use append_set_rgb().
    SkASSERT(op != Op::parametric);               // Please use append_transfer_function().
    SkASSERT(op != Op::gamma_);                   // Please use append_transfer_function().
    SkASSERT(op != Op::PQish);                    // Please use append_transfer_function().
    SkASSERT(op != Op::HLGish);                   // Please use append_transfer_function().
    SkASSERT(op != Op::HLGinvish);                // Please use append_transfer_function().
    SkASSERT(op != Op::stack_checkpoint);         // Please use append_stack_rewind().
    SkASSERT(op != Op::stack_rewind);             // Please use append_stack_rewind().
    this->unchecked_append(op, ctx);
}
void SkRasterPipeline::unchecked_append(SkRasterPipelineOp op, void* ctx) {
    fStages = fAlloc->make<StageList>(StageList{fStages, op, ctx});
    fNumStages += 1;
}
void SkRasterPipeline::append(SkRasterPipelineOp op, uintptr_t ctx) {
    void* ptrCtx;
    memcpy(&ptrCtx, &ctx, sizeof(ctx));
    this->append(op, ptrCtx);
}

void SkRasterPipeline::extend(const SkRasterPipeline& src) {
    if (src.empty()) {
        return;
    }
    // Create a rewind context if `src` has one already, but we don't. If we _do_ already have one,
    // we need to keep it, since we already have rewind ops that reference it. Either way, we need
    // to rewrite all the rewind ops to point to _our_ rewind context; we only get that checkpoint.
    if (src.fRewindCtx && !fRewindCtx) {
        fRewindCtx = fAlloc->make<SkRasterPipeline_RewindCtx>();
    }
    auto stages = fAlloc->makeArrayDefault<StageList>(src.fNumStages);

    int n = src.fNumStages;
    const StageList* st = src.fStages;
    while (n --> 1) {
        stages[n]      = *st;
        stages[n].prev = &stages[n-1];

        if (stages[n].stage == Op::stack_rewind) {
            // We make sure that all stack rewinds use _our_ stack context.
            stages[n].ctx = fRewindCtx;
        }

        st = st->prev;
    }
    stages[0]      = *st;
    stages[0].prev = fStages;

    fStages = &stages[src.fNumStages - 1];
    fNumStages += src.fNumStages;
}

const char* SkRasterPipeline::GetOpName(SkRasterPipelineOp op) {
    const char* name = "";
    switch (op) {
    #define M(x) case Op::x: name = #x; break;
        SK_RASTER_PIPELINE_OPS_ALL(M)
    #undef M
    }
    return name;
}

void SkRasterPipeline::dump() const {
    SkDebugf("SkRasterPipeline, %d stages\n", fNumStages);
    std::vector<const char*> stages;
    for (auto st = fStages; st; st = st->prev) {
        stages.push_back(GetOpName(st->stage));
    }
    std::reverse(stages.begin(), stages.end());
    for (const char* name : stages) {
        SkDebugf("\t%s\n", name);
    }
    SkDebugf("\n");
}

void SkRasterPipeline::append_set_rgb(SkArenaAlloc* alloc, const float rgb[3]) {
    auto arg = alloc->makeArrayDefault<float>(3);
    arg[0] = rgb[0];
    arg[1] = rgb[1];
    arg[2] = rgb[2];

    auto op = Op::unbounded_set_rgb;
    if (0 <= rgb[0] && rgb[0] <= 1 &&
        0 <= rgb[1] && rgb[1] <= 1 &&
        0 <= rgb[2] && rgb[2] <= 1)
    {
        op = Op::set_rgb;
    }

    this->unchecked_append(op, arg);
}

void SkRasterPipeline::append_constant_color(SkArenaAlloc* alloc, const float rgba[4]) {
    // r,g,b might be outside [0,1], but alpha should probably always be in [0,1].
    SkASSERT(0 <= rgba[3] && rgba[3] <= 1);

    if (rgba[0] == 0 && rgba[1] == 0 && rgba[2] == 0 && rgba[3] == 1) {
        this->append(Op::black_color);
    } else if (rgba[0] == 1 && rgba[1] == 1 && rgba[2] == 1 && rgba[3] == 1) {
        this->append(Op::white_color);
    } else {
        auto ctx = alloc->make<SkRasterPipeline_UniformColorCtx>();
        skvx::float4 color = skvx::float4::Load(rgba);
        color.store(&ctx->r);

        // uniform_color requires colors in range and can go lowp,
        // while unbounded_uniform_color supports out-of-range colors too but not lowp.
        if (0 <= rgba[0] && rgba[0] <= rgba[3] &&
            0 <= rgba[1] && rgba[1] <= rgba[3] &&
            0 <= rgba[2] && rgba[2] <= rgba[3]) {
            // To make loads more direct, we store 8-bit values in 16-bit slots.
            color = color * 255.0f + 0.5f;
            ctx->rgba[0] = (uint16_t)color[0];
            ctx->rgba[1] = (uint16_t)color[1];
            ctx->rgba[2] = (uint16_t)color[2];
            ctx->rgba[3] = (uint16_t)color[3];
            this->unchecked_append(Op::uniform_color, ctx);
        } else {
            this->unchecked_append(Op::unbounded_uniform_color, ctx);
        }
    }
}

void SkRasterPipeline::append_matrix(SkArenaAlloc* alloc, const SkMatrix& matrix) {
    SkMatrix::TypeMask mt = matrix.getType();

    if (mt == SkMatrix::kIdentity_Mask) {
        return;
    }
    if (mt == SkMatrix::kTranslate_Mask) {
        float* trans = alloc->makeArrayDefault<float>(2);
        trans[0] = matrix.getTranslateX();
        trans[1] = matrix.getTranslateY();
        this->append(Op::matrix_translate, trans);
    } else if ((mt | (SkMatrix::kScale_Mask | SkMatrix::kTranslate_Mask)) ==
                     (SkMatrix::kScale_Mask | SkMatrix::kTranslate_Mask)) {
        float* scaleTrans = alloc->makeArrayDefault<float>(4);
        scaleTrans[0] = matrix.getScaleX();
        scaleTrans[1] = matrix.getScaleY();
        scaleTrans[2] = matrix.getTranslateX();
        scaleTrans[3] = matrix.getTranslateY();
        this->append(Op::matrix_scale_translate, scaleTrans);
    } else {
        float* storage = alloc->makeArrayDefault<float>(9);
        matrix.get9(storage);
        if (!matrix.hasPerspective()) {
            // note: asAffine and the 2x3 stage really only need 6 entries
            this->append(Op::matrix_2x3, storage);
        } else {
            this->append(Op::matrix_perspective, storage);
        }
    }
}

void SkRasterPipeline::append_load(SkColorType ct, const SkRasterPipeline_MemoryCtx* ctx) {
    switch (ct) {
        case kUnknown_SkColorType: SkASSERT(false); break;

        case kAlpha_8_SkColorType:           this->append(Op::load_a8,      ctx); break;
        case kA16_unorm_SkColorType:         this->append(Op::load_a16,     ctx); break;
        case kA16_float_SkColorType:         this->append(Op::load_af16,    ctx); break;
        case kRGB_565_SkColorType:           this->append(Op::load_565,     ctx); break;
        case kARGB_4444_SkColorType:         this->append(Op::load_4444,    ctx); break;
        case kR8G8_unorm_SkColorType:        this->append(Op::load_rg88,    ctx); break;
        case kR16G16_unorm_SkColorType:      this->append(Op::load_rg1616,  ctx); break;
        case kR16G16_float_SkColorType:      this->append(Op::load_rgf16,   ctx); break;
        case kRGBA_8888_SkColorType:         this->append(Op::load_8888,    ctx); break;
        case kRGBA_1010102_SkColorType:      this->append(Op::load_1010102, ctx); break;
        case kR16G16B16A16_unorm_SkColorType:this->append(Op::load_16161616,ctx); break;
        case kRGBA_F16Norm_SkColorType:
        case kRGBA_F16_SkColorType:          this->append(Op::load_f16,     ctx); break;
        case kRGBA_F32_SkColorType:          this->append(Op::load_f32,     ctx); break;
        case kRGBA_10x6_SkColorType:         this->append(Op::load_10x6,    ctx); break;

        case kGray_8_SkColorType:            this->append(Op::load_a8, ctx);
                                             this->append(Op::alpha_to_gray);
                                             break;

        case kR8_unorm_SkColorType:          this->append(Op::load_a8, ctx);
                                             this->append(Op::alpha_to_red);
                                             break;

        case kRGB_888x_SkColorType:          this->append(Op::load_8888, ctx);
                                             this->append(Op::force_opaque);
                                             break;

        case kBGRA_1010102_SkColorType:      this->append(Op::load_1010102, ctx);
                                             this->append(Op::swap_rb);
                                             break;

        case kRGB_101010x_SkColorType:       this->append(Op::load_1010102, ctx);
                                             this->append(Op::force_opaque);
                                             break;

        case kBGR_101010x_SkColorType:       this->append(Op::load_1010102, ctx);
                                             this->append(Op::force_opaque);
                                             this->append(Op::swap_rb);
                                             break;

        case kBGR_101010x_XR_SkColorType:    this->append(Op::load_1010102_xr, ctx);
                                             this->append(Op::force_opaque);
                                             this->append(Op::swap_rb);
                                             break;

        case kBGRA_8888_SkColorType:         this->append(Op::load_8888, ctx);
                                             this->append(Op::swap_rb);
                                             break;

        case kSRGBA_8888_SkColorType:
            this->append(Op::load_8888, ctx);
            this->append_transfer_function(*skcms_sRGB_TransferFunction());
            break;
    }
}

void SkRasterPipeline::append_load_dst(SkColorType ct, const SkRasterPipeline_MemoryCtx* ctx) {
    switch (ct) {
        case kUnknown_SkColorType: SkASSERT(false); break;

        case kAlpha_8_SkColorType:            this->append(Op::load_a8_dst,      ctx); break;
        case kA16_unorm_SkColorType:          this->append(Op::load_a16_dst,     ctx); break;
        case kA16_float_SkColorType:          this->append(Op::load_af16_dst,    ctx); break;
        case kRGB_565_SkColorType:            this->append(Op::load_565_dst,     ctx); break;
        case kARGB_4444_SkColorType:          this->append(Op::load_4444_dst,    ctx); break;
        case kR8G8_unorm_SkColorType:         this->append(Op::load_rg88_dst,    ctx); break;
        case kR16G16_unorm_SkColorType:       this->append(Op::load_rg1616_dst,  ctx); break;
        case kR16G16_float_SkColorType:       this->append(Op::load_rgf16_dst,   ctx); break;
        case kRGBA_8888_SkColorType:          this->append(Op::load_8888_dst,    ctx); break;
        case kRGBA_1010102_SkColorType:       this->append(Op::load_1010102_dst, ctx); break;
        case kR16G16B16A16_unorm_SkColorType: this->append(Op::load_16161616_dst,ctx); break;
        case kRGBA_F16Norm_SkColorType:
        case kRGBA_F16_SkColorType:           this->append(Op::load_f16_dst,     ctx); break;
        case kRGBA_F32_SkColorType:           this->append(Op::load_f32_dst,     ctx); break;
        case kRGBA_10x6_SkColorType:          this->append(Op::load_10x6_dst,    ctx); break;

        case kGray_8_SkColorType:             this->append(Op::load_a8_dst, ctx);
                                              this->append(Op::alpha_to_gray_dst);
                                              break;

        case kR8_unorm_SkColorType:           this->append(Op::load_a8_dst, ctx);
                                              this->append(Op::alpha_to_red_dst);
                                              break;

        case kRGB_888x_SkColorType:           this->append(Op::load_8888_dst, ctx);
                                              this->append(Op::force_opaque_dst);
                                              break;

        case kBGRA_1010102_SkColorType:       this->append(Op::load_1010102_dst, ctx);
                                              this->append(Op::swap_rb_dst);
                                              break;

        case kRGB_101010x_SkColorType:        this->append(Op::load_1010102_dst, ctx);
                                              this->append(Op::force_opaque_dst);
                                              break;

        case kBGR_101010x_SkColorType:        this->append(Op::load_1010102_dst, ctx);
                                              this->append(Op::force_opaque_dst);
                                              this->append(Op::swap_rb_dst);
                                              break;

        case kBGR_101010x_XR_SkColorType:     this->append(Op::load_1010102_xr_dst, ctx);
                                              this->append(Op::force_opaque_dst);
                                              this->append(Op::swap_rb_dst);
                                              break;

        case kBGRA_8888_SkColorType:          this->append(Op::load_8888_dst, ctx);
                                              this->append(Op::swap_rb_dst);
                                              break;

        case kSRGBA_8888_SkColorType:
            // TODO: We could remove the double-swap if we had _dst versions of all the TF stages
            this->append(Op::load_8888_dst, ctx);
            this->append(Op::swap_src_dst);
            this->append_transfer_function(*skcms_sRGB_TransferFunction());
            this->append(Op::swap_src_dst);
            break;
    }
}

void SkRasterPipeline::append_store(SkColorType ct, const SkRasterPipeline_MemoryCtx* ctx) {
    switch (ct) {
        case kUnknown_SkColorType: SkASSERT(false); break;

        case kAlpha_8_SkColorType:            this->append(Op::store_a8,      ctx); break;
        case kR8_unorm_SkColorType:           this->append(Op::store_r8,      ctx); break;
        case kA16_unorm_SkColorType:          this->append(Op::store_a16,     ctx); break;
        case kA16_float_SkColorType:          this->append(Op::store_af16,    ctx); break;
        case kRGB_565_SkColorType:            this->append(Op::store_565,     ctx); break;
        case kARGB_4444_SkColorType:          this->append(Op::store_4444,    ctx); break;
        case kR8G8_unorm_SkColorType:         this->append(Op::store_rg88,    ctx); break;
        case kR16G16_unorm_SkColorType:       this->append(Op::store_rg1616,  ctx); break;
        case kR16G16_float_SkColorType:       this->append(Op::store_rgf16,   ctx); break;
        case kRGBA_8888_SkColorType:          this->append(Op::store_8888,    ctx); break;
        case kRGBA_1010102_SkColorType:       this->append(Op::store_1010102, ctx); break;
        case kR16G16B16A16_unorm_SkColorType: this->append(Op::store_16161616,ctx); break;
        case kRGBA_F16Norm_SkColorType:
        case kRGBA_F16_SkColorType:           this->append(Op::store_f16,     ctx); break;
        case kRGBA_F32_SkColorType:           this->append(Op::store_f32,     ctx); break;
        case kRGBA_10x6_SkColorType:          this->append(Op::store_10x6,    ctx); break;

        case kRGB_888x_SkColorType:           this->append(Op::force_opaque);
                                              this->append(Op::store_8888, ctx);
                                              break;

        case kBGRA_1010102_SkColorType:       this->append(Op::swap_rb);
                                              this->append(Op::store_1010102, ctx);
                                              break;

        case kRGB_101010x_SkColorType:        this->append(Op::force_opaque);
                                              this->append(Op::store_1010102, ctx);
                                              break;

        case kBGR_101010x_SkColorType:        this->append(Op::force_opaque);
                                              this->append(Op::swap_rb);
                                              this->append(Op::store_1010102, ctx);
                                              break;

        case kBGR_101010x_XR_SkColorType:     this->append(Op::force_opaque);
                                              this->append(Op::swap_rb);
                                              this->append(Op::store_1010102_xr, ctx);
                                              break;

        case kGray_8_SkColorType:             this->append(Op::bt709_luminance_or_luma_to_alpha);
                                              this->append(Op::store_a8, ctx);
                                              break;

        case kBGRA_8888_SkColorType:          this->append(Op::swap_rb);
                                              this->append(Op::store_8888, ctx);
                                              break;

        case kSRGBA_8888_SkColorType:
            this->append_transfer_function(*skcms_sRGB_Inverse_TransferFunction());
            this->append(Op::store_8888, ctx);
            break;
    }
}

void SkRasterPipeline::append_transfer_function(const skcms_TransferFunction& tf) {
    void* ctx = const_cast<void*>(static_cast<const void*>(&tf));
    switch (skcms_TransferFunction_getType(&tf)) {
        case skcms_TFType_Invalid: SkASSERT(false); break;

        case skcms_TFType_sRGBish:
            if (tf.a == 1 && tf.b == 0 && tf.c == 0 && tf.d == 0 && tf.e == 0 && tf.f == 0) {
                this->unchecked_append(Op::gamma_, ctx);
            } else {
                this->unchecked_append(Op::parametric, ctx);
            }
            break;
        case skcms_TFType_PQish:     this->unchecked_append(Op::PQish,     ctx); break;
        case skcms_TFType_HLGish:    this->unchecked_append(Op::HLGish,    ctx); break;
        case skcms_TFType_HLGinvish: this->unchecked_append(Op::HLGinvish, ctx); break;
    }
}

// GPUs clamp all color channels to the limits of the format just before the blend step. To match
// that auto-clamp, the RP blitter uses this helper immediately before appending blending stages.
void SkRasterPipeline::append_clamp_if_normalized(const SkImageInfo& info) {
    if (SkColorTypeIsNormalized(info.colorType())) {
        this->unchecked_append(Op::clamp_01, nullptr);
    }
}

void SkRasterPipeline::append_stack_rewind() {
    if (!fRewindCtx) {
        fRewindCtx = fAlloc->make<SkRasterPipeline_RewindCtx>();
    }
    this->unchecked_append(Op::stack_rewind, fRewindCtx);
}

static void prepend_to_pipeline(SkRasterPipelineStage*& ip, SkOpts::StageFn stageFn, void* ctx) {
    --ip;
    ip->fn = stageFn;
    ip->ctx = ctx;
}

bool SkRasterPipeline::build_lowp_pipeline(SkRasterPipelineStage* ip) const {
    if (gForceHighPrecisionRasterPipeline || fRewindCtx) {
        return false;
    }
    // Stages are stored backwards in fStages; to compensate, we assemble the pipeline in reverse
    // here, back to front.
    prepend_to_pipeline(ip, SkOpts::just_return_lowp, /*ctx=*/nullptr);
    for (const StageList* st = fStages; st; st = st->prev) {
        int opIndex = (int)st->stage;
        if (opIndex >= kNumRasterPipelineLowpOps || !SkOpts::ops_lowp[opIndex]) {
            // This program contains a stage that doesn't exist in lowp.
            return false;
        }
        prepend_to_pipeline(ip, SkOpts::ops_lowp[opIndex], st->ctx);
    }
    return true;
}

void SkRasterPipeline::build_highp_pipeline(SkRasterPipelineStage* ip) const {
    // We assemble the pipeline in reverse, since the stage list is stored backwards.
    prepend_to_pipeline(ip, SkOpts::just_return_highp, /*ctx=*/nullptr);
    for (const StageList* st = fStages; st; st = st->prev) {
        int opIndex = (int)st->stage;
        prepend_to_pipeline(ip, SkOpts::ops_highp[opIndex], st->ctx);
    }

    // stack_checkpoint and stack_rewind are only implemented in highp. We only need these stages
    // when generating long (or looping) pipelines from SkSL. The other stages used by the SkSL
    // Raster Pipeline generator will only have highp implementations, because we can't execute SkSL
    // code without floating point.
    if (fRewindCtx) {
        const int rewindIndex = (int)Op::stack_checkpoint;
        prepend_to_pipeline(ip, SkOpts::ops_highp[rewindIndex], fRewindCtx);
    }
}

SkRasterPipeline::StartPipelineFn SkRasterPipeline::build_pipeline(
        SkRasterPipelineStage* ip) const {
    // We try to build a lowp pipeline first; if that fails, we fall back to a highp float pipeline.
    if (this->build_lowp_pipeline(ip)) {
        return SkOpts::start_pipeline_lowp;
    }

    this->build_highp_pipeline(ip);
    return SkOpts::start_pipeline_highp;
}

int SkRasterPipeline::stages_needed() const {
    // Add 1 to budget for a `just_return` stage at the end.
    int stages = fNumStages + 1;

    // If we have any stack_rewind stages, we will need to inject a stack_checkpoint stage.
    if (fRewindCtx) {
        stages += 1;
    }
    return stages;
}

void SkRasterPipeline::run(size_t x, size_t y, size_t w, size_t h) const {
    if (this->empty()) {
        return;
    }

    int stagesNeeded = this->stages_needed();

    // Best to not use fAlloc here... we can't bound how often run() will be called.
    AutoSTMalloc<32, SkRasterPipelineStage> program(stagesNeeded);

    auto start_pipeline = this->build_pipeline(program.get() + stagesNeeded);
    start_pipeline(x,y,x+w,y+h, program.get());
}

std::function<void(size_t, size_t, size_t, size_t)> SkRasterPipeline::compile() const {
    if (this->empty()) {
        return [](size_t, size_t, size_t, size_t) {};
    }

    int stagesNeeded = this->stages_needed();

    SkRasterPipelineStage* program = fAlloc->makeArray<SkRasterPipelineStage>(stagesNeeded);

    auto start_pipeline = this->build_pipeline(program + stagesNeeded);
    return [=](size_t x, size_t y, size_t w, size_t h) {
        start_pipeline(x,y,x+w,y+h, program);
    };
}
