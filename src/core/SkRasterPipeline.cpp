/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/SkImageInfoPriv.h"
#include "src/core/SkColorSpacePriv.h"
#include "src/core/SkOpts.h"
#include "src/core/SkRasterPipeline.h"
#include <algorithm>

SkRasterPipeline::SkRasterPipeline(SkArenaAlloc* alloc) : fAlloc(alloc) {
    this->reset();
}
void SkRasterPipeline::reset() {
    fStages      = nullptr;
    fNumStages   = 0;
    fSlotsNeeded = 1;  // We always need one extra slot for just_return().
}

void SkRasterPipeline::append(StockStage stage, void* ctx) {
    SkASSERT(stage !=           uniform_color);  // Please use append_constant_color().
    SkASSERT(stage != unbounded_uniform_color);  // Please use append_constant_color().
    SkASSERT(stage !=                 set_rgb);  // Please use append_set_rgb().
    SkASSERT(stage !=       unbounded_set_rgb);  // Please use append_set_rgb().
    SkASSERT(stage !=             clamp_gamut);  // Please use append_gamut_clamp_if_normalized().
    SkASSERT(stage !=              parametric);  // Please use append_transfer_function().
    SkASSERT(stage !=                  gamma_);  // Please use append_transfer_function().
    SkASSERT(stage !=                   PQish);  // Please use append_transfer_function().
    SkASSERT(stage !=                  HLGish);  // Please use append_transfer_function().
    SkASSERT(stage !=               HLGinvish);  // Please use append_transfer_function().
    this->unchecked_append(stage, ctx);
}
void SkRasterPipeline::unchecked_append(StockStage stage, void* ctx) {
    fStages = fAlloc->make<StageList>( StageList{fStages, stage, ctx} );
    fNumStages   += 1;
    fSlotsNeeded += ctx ? 2 : 1;
}
void SkRasterPipeline::append(StockStage stage, uintptr_t ctx) {
    void* ptrCtx;
    memcpy(&ptrCtx, &ctx, sizeof(ctx));
    this->append(stage, ptrCtx);
}

void SkRasterPipeline::extend(const SkRasterPipeline& src) {
    if (src.empty()) {
        return;
    }
    auto stages = fAlloc->makeArrayDefault<StageList>(src.fNumStages);

    int n = src.fNumStages;
    const StageList* st = src.fStages;
    while (n --> 1) {
        stages[n]      = *st;
        stages[n].prev = &stages[n-1];
        st = st->prev;
    }
    stages[0]      = *st;
    stages[0].prev = fStages;

    fStages = &stages[src.fNumStages - 1];
    fNumStages   += src.fNumStages;
    fSlotsNeeded += src.fSlotsNeeded - 1;  // Don't double count just_returns().
}

void SkRasterPipeline::dump() const {
    SkDebugf("SkRasterPipeline, %d stages\n", fNumStages);
    std::vector<const char*> stages;
    for (auto st = fStages; st; st = st->prev) {
        const char* name = "";
        switch (st->stage) {
        #define M(x) case x: name = #x; break;
            SK_RASTER_PIPELINE_STAGES(M)
        #undef M
        }
        stages.push_back(name);
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

    auto stage = unbounded_set_rgb;
    if (0 <= rgb[0] && rgb[0] <= 1 &&
        0 <= rgb[1] && rgb[1] <= 1 &&
        0 <= rgb[2] && rgb[2] <= 1)
    {
        stage = set_rgb;
    }

    this->unchecked_append(stage, arg);
}

void SkRasterPipeline::append_constant_color(SkArenaAlloc* alloc, const float rgba[4]) {
    // r,g,b might be outside [0,1], but alpha should probably always be in [0,1].
    SkASSERT(0 <= rgba[3] && rgba[3] <= 1);

    if (rgba[0] == 0 && rgba[1] == 0 && rgba[2] == 0 && rgba[3] == 1) {
        this->append(black_color);
    } else if (rgba[0] == 1 && rgba[1] == 1 && rgba[2] == 1 && rgba[3] == 1) {
        this->append(white_color);
    } else {
        auto ctx = alloc->make<SkRasterPipeline_UniformColorCtx>();
        Sk4f color = Sk4f::Load(rgba);
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
            this->unchecked_append(uniform_color, ctx);
        } else {
            this->unchecked_append(unbounded_uniform_color, ctx);
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
        this->append(SkRasterPipeline::matrix_translate, trans);
    } else if ((mt | (SkMatrix::kScale_Mask | SkMatrix::kTranslate_Mask)) ==
                     (SkMatrix::kScale_Mask | SkMatrix::kTranslate_Mask)) {
        float* scaleTrans = alloc->makeArrayDefault<float>(4);
        scaleTrans[0] = matrix.getScaleX();
        scaleTrans[1] = matrix.getScaleY();
        scaleTrans[2] = matrix.getTranslateX();
        scaleTrans[3] = matrix.getTranslateY();
        this->append(SkRasterPipeline::matrix_scale_translate, scaleTrans);
    } else {
        float* storage = alloc->makeArrayDefault<float>(9);
        if (matrix.asAffine(storage)) {
            // note: asAffine and the 2x3 stage really only need 6 entries
            this->append(SkRasterPipeline::matrix_2x3, storage);
        } else {
            matrix.get9(storage);
            this->append(SkRasterPipeline::matrix_perspective, storage);
        }
    }
}

void SkRasterPipeline::append_load(SkColorType ct, const SkRasterPipeline_MemoryCtx* ctx) {
    switch (ct) {
        case kUnknown_SkColorType: SkASSERT(false); break;

        case kAlpha_8_SkColorType:           this->append(load_a8,      ctx); break;
        case kA16_unorm_SkColorType:         this->append(load_a16,     ctx); break;
        case kA16_float_SkColorType:         this->append(load_af16,    ctx); break;
        case kRGB_565_SkColorType:           this->append(load_565,     ctx); break;
        case kARGB_4444_SkColorType:         this->append(load_4444,    ctx); break;
        case kR8G8_unorm_SkColorType:        this->append(load_rg88,    ctx); break;
        case kR16G16_unorm_SkColorType:      this->append(load_rg1616,  ctx); break;
        case kR16G16_float_SkColorType:      this->append(load_rgf16,   ctx); break;
        case kRGBA_8888_SkColorType:         this->append(load_8888,    ctx); break;
        case kRGBA_1010102_SkColorType:      this->append(load_1010102, ctx); break;
        case kR16G16B16A16_unorm_SkColorType:this->append(load_16161616,ctx); break;
        case kRGBA_F16Norm_SkColorType:
        case kRGBA_F16_SkColorType:          this->append(load_f16,     ctx); break;
        case kRGBA_F32_SkColorType:          this->append(load_f32,     ctx); break;

        case kGray_8_SkColorType:            this->append(load_a8, ctx);
                                             this->append(alpha_to_gray);
                                             break;

        case kRGB_888x_SkColorType:          this->append(load_8888, ctx);
                                             this->append(force_opaque);
                                             break;

        case kBGRA_1010102_SkColorType:      this->append(load_1010102, ctx);
                                             this->append(swap_rb);
                                             break;

        case kRGB_101010x_SkColorType:       this->append(load_1010102, ctx);
                                             this->append(force_opaque);
                                             break;

        case kBGR_101010x_SkColorType:       this->append(load_1010102, ctx);
                                             this->append(force_opaque);
                                             this->append(swap_rb);
                                             break;

        case kBGRA_8888_SkColorType:         this->append(load_8888, ctx);
                                             this->append(swap_rb);
                                             break;
    }
}

void SkRasterPipeline::append_load_dst(SkColorType ct, const SkRasterPipeline_MemoryCtx* ctx) {
    switch (ct) {
        case kUnknown_SkColorType: SkASSERT(false); break;

        case kAlpha_8_SkColorType:            this->append(load_a8_dst,      ctx); break;
        case kA16_unorm_SkColorType:          this->append(load_a16_dst,     ctx); break;
        case kA16_float_SkColorType:          this->append(load_af16_dst,    ctx); break;
        case kRGB_565_SkColorType:            this->append(load_565_dst,     ctx); break;
        case kARGB_4444_SkColorType:          this->append(load_4444_dst,    ctx); break;
        case kR8G8_unorm_SkColorType:         this->append(load_rg88_dst,    ctx); break;
        case kR16G16_unorm_SkColorType:       this->append(load_rg1616_dst,  ctx); break;
        case kR16G16_float_SkColorType:       this->append(load_rgf16_dst,   ctx); break;
        case kRGBA_8888_SkColorType:          this->append(load_8888_dst,    ctx); break;
        case kRGBA_1010102_SkColorType:       this->append(load_1010102_dst, ctx); break;
        case kR16G16B16A16_unorm_SkColorType: this->append(load_16161616_dst,ctx); break;
        case kRGBA_F16Norm_SkColorType:
        case kRGBA_F16_SkColorType:           this->append(load_f16_dst,     ctx); break;
        case kRGBA_F32_SkColorType:           this->append(load_f32_dst,     ctx); break;

        case kGray_8_SkColorType:             this->append(load_a8_dst, ctx);
                                              this->append(alpha_to_gray_dst);
                                              break;

        case kRGB_888x_SkColorType:           this->append(load_8888_dst, ctx);
                                              this->append(force_opaque_dst);
                                              break;

        case kBGRA_1010102_SkColorType:       this->append(load_1010102_dst, ctx);
                                              this->append(swap_rb_dst);
                                              break;

        case kRGB_101010x_SkColorType:        this->append(load_1010102_dst, ctx);
                                              this->append(force_opaque_dst);
                                              break;

        case kBGR_101010x_SkColorType:        this->append(load_1010102_dst, ctx);
                                              this->append(force_opaque_dst);
                                              this->append(swap_rb_dst);
                                              break;

        case kBGRA_8888_SkColorType:          this->append(load_8888_dst, ctx);
                                              this->append(swap_rb_dst);
                                              break;
    }
}

void SkRasterPipeline::append_store(SkColorType ct, const SkRasterPipeline_MemoryCtx* ctx) {
    switch (ct) {
        case kUnknown_SkColorType: SkASSERT(false); break;

        case kAlpha_8_SkColorType:            this->append(store_a8,      ctx); break;
        case kA16_unorm_SkColorType:          this->append(store_a16,     ctx); break;
        case kA16_float_SkColorType:          this->append(store_af16,    ctx); break;
        case kRGB_565_SkColorType:            this->append(store_565,     ctx); break;
        case kARGB_4444_SkColorType:          this->append(store_4444,    ctx); break;
        case kR8G8_unorm_SkColorType:         this->append(store_rg88,    ctx); break;
        case kR16G16_unorm_SkColorType:       this->append(store_rg1616,  ctx); break;
        case kR16G16_float_SkColorType:       this->append(store_rgf16,   ctx); break;
        case kRGBA_8888_SkColorType:          this->append(store_8888,    ctx); break;
        case kRGBA_1010102_SkColorType:       this->append(store_1010102, ctx); break;
        case kR16G16B16A16_unorm_SkColorType: this->append(store_16161616,ctx); break;
        case kRGBA_F16Norm_SkColorType:
        case kRGBA_F16_SkColorType:           this->append(store_f16,     ctx); break;
        case kRGBA_F32_SkColorType:           this->append(store_f32,     ctx); break;

        case kRGB_888x_SkColorType:           this->append(force_opaque);
                                              this->append(store_8888, ctx);
                                              break;

        case kBGRA_1010102_SkColorType:       this->append(swap_rb);
                                              this->append(store_1010102, ctx);
                                              break;

        case kRGB_101010x_SkColorType:        this->append(force_opaque);
                                              this->append(store_1010102, ctx);
                                              break;

        case kBGR_101010x_SkColorType:        this->append(force_opaque);
                                              this->append(swap_rb);
                                              this->append(store_1010102, ctx);
                                              break;

        case kGray_8_SkColorType:             this->append(bt709_luminance_or_luma_to_alpha);
                                              this->append(store_a8, ctx);
                                              break;

        case kBGRA_8888_SkColorType:          this->append(swap_rb);
                                              this->append(store_8888, ctx);
                                              break;
    }
}

void SkRasterPipeline::append_transfer_function(const skcms_TransferFunction& tf) {
    void* ctx = const_cast<void*>(static_cast<const void*>(&tf));
    switch (classify_transfer_fn(tf)) {
        case Bad_TF: SkASSERT(false); break;

        case TFKind::sRGBish_TF:
            if (tf.a == 1 && tf.b == 0 && tf.c == 0 && tf.d == 0 && tf.e == 0 && tf.f == 0) {
                this->unchecked_append(gamma_, ctx);
            } else {
                this->unchecked_append(parametric, ctx);
            }
            break;
        case PQish_TF:     this->unchecked_append(PQish,     ctx); break;
        case HLGish_TF:    this->unchecked_append(HLGish,    ctx); break;
        case HLGinvish_TF: this->unchecked_append(HLGinvish, ctx); break;
    }
}

// Clamp premul values to [0,alpha] (logical [0,1]) to avoid the confusing
// scenario of being able to store a logical color channel > 1.0 when alpha < 1.0.
// Most software that works with normalized premul values expect r,g,b channels all <= a.
//
// In addition, GL clamps all its color channels to limits of the format just
// before the blend step (~here).  To match that auto-clamp, we clamp alpha to
// [0,1] too, just in case someone gave us a crazy alpha.
void SkRasterPipeline::append_gamut_clamp_if_normalized(const SkImageInfo& info) {
    if (info.alphaType() == kPremul_SkAlphaType && SkColorTypeIsNormalized(info.colorType())) {
        this->unchecked_append(SkRasterPipeline::clamp_gamut, nullptr);
    }
}

SkRasterPipeline::StartPipelineFn SkRasterPipeline::build_pipeline(void** ip) const {
    // We'll try to build a lowp pipeline, but if that fails fallback to a highp float pipeline.
    void** reset_point = ip;

    // Stages are stored backwards in fStages, so we reverse here, back to front.
    *--ip = (void*)SkOpts::just_return_lowp;
    for (const StageList* st = fStages; st; st = st->prev) {
        if (auto fn = SkOpts::stages_lowp[st->stage]) {
            if (st->ctx) {
                *--ip = st->ctx;
            }
            *--ip = (void*)fn;
        } else {
            ip = reset_point;
            break;
        }
    }
    if (ip != reset_point) {
        return SkOpts::start_pipeline_lowp;
    }

    *--ip = (void*)SkOpts::just_return_highp;
    for (const StageList* st = fStages; st; st = st->prev) {
        if (st->ctx) {
            *--ip = st->ctx;
        }
        *--ip = (void*)SkOpts::stages_highp[st->stage];
    }
    return SkOpts::start_pipeline_highp;
}

void SkRasterPipeline::run(size_t x, size_t y, size_t w, size_t h) const {
    if (this->empty()) {
        return;
    }

    // Best to not use fAlloc here... we can't bound how often run() will be called.
    SkAutoSTMalloc<64, void*> program(fSlotsNeeded);

    auto start_pipeline = this->build_pipeline(program.get() + fSlotsNeeded);
    start_pipeline(x,y,x+w,y+h, program.get());
}

std::function<void(size_t, size_t, size_t, size_t)> SkRasterPipeline::compile() const {
    if (this->empty()) {
        return [](size_t, size_t, size_t, size_t) {};
    }

    void** program = fAlloc->makeArray<void*>(fSlotsNeeded);

    auto start_pipeline = this->build_pipeline(program + fSlotsNeeded);
    return [=](size_t x, size_t y, size_t w, size_t h) {
        start_pipeline(x,y,x+w,y+h, program);
    };
}
