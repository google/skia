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
#include "src/base/SkAutoMalloc.h"
#include "src/base/SkVx.h"
#include "src/core/SkImageInfoPriv.h"
#include "src/core/SkOpts.h"
#include "src/core/SkRasterPipelineOpContexts.h"

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
    // We intentionally leave the alloc alone here; we don't own it.
    fRewindCtx   = nullptr;
    fStages      = nullptr;
    fTailPointer = nullptr;
    fNumStages   = 0;
    fMemoryCtxInfos.clear();
}

void SkRasterPipeline::append(SkRasterPipelineOp op, void* ctx) {
    SkASSERT(op != Op::uniform_color);            // Please use appendConstantColor().
    SkASSERT(op != Op::unbounded_uniform_color);  // Please use appendConstantColor().
    SkASSERT(op != Op::set_rgb);                  // Please use appendSetRGB().
    SkASSERT(op != Op::unbounded_set_rgb);        // Please use appendSetRGB().
    SkASSERT(op != Op::parametric);               // Please use appendTransferFunction().
    SkASSERT(op != Op::gamma_);                   // Please use appendTransferFunction().
    SkASSERT(op != Op::PQish);                    // Please use appendTransferFunction().
    SkASSERT(op != Op::HLGish);                   // Please use appendTransferFunction().
    SkASSERT(op != Op::HLGinvish);                // Please use appendTransferFunction().
    SkASSERT(op != Op::stack_checkpoint);         // Please use appendStackRewind().
    SkASSERT(op != Op::stack_rewind);             // Please use appendStackRewind().
    this->uncheckedAppend(op, ctx);
}

uint8_t* SkRasterPipeline::tailPointer() {
    if (!fTailPointer) {
        // All ops in the pipeline that use the tail value share the same value.
        fTailPointer = fAlloc->make<uint8_t>(0xFF);
    }
    return fTailPointer;
}

void SkRasterPipeline::uncheckedAppend(SkRasterPipelineOp op, void* ctx) {
    bool isLoad = false, isStore = false;
    SkColorType ct = kUnknown_SkColorType;

#define COLOR_TYPE_CASE(stage_ct, sk_ct) \
    case Op::load_##stage_ct:            \
    case Op::load_##stage_ct##_dst:      \
        ct = sk_ct;                      \
        isLoad = true;                   \
        break;                           \
    case Op::store_##stage_ct:           \
        ct = sk_ct;                      \
        isStore = true;                  \
        break;

    switch (op) {
        COLOR_TYPE_CASE(a8, kAlpha_8_SkColorType)
        COLOR_TYPE_CASE(565, kRGB_565_SkColorType)
        COLOR_TYPE_CASE(4444, kARGB_4444_SkColorType)
        COLOR_TYPE_CASE(8888, kRGBA_8888_SkColorType)
        COLOR_TYPE_CASE(rg88, kR8G8_unorm_SkColorType)
        COLOR_TYPE_CASE(16161616, kR16G16B16A16_unorm_SkColorType)
        COLOR_TYPE_CASE(a16, kA16_unorm_SkColorType)
        COLOR_TYPE_CASE(rg1616, kR16G16_unorm_SkColorType)
        COLOR_TYPE_CASE(f16, kRGBA_F16_SkColorType)
        COLOR_TYPE_CASE(af16, kA16_float_SkColorType)
        COLOR_TYPE_CASE(rgf16, kR16G16_float_SkColorType)
        COLOR_TYPE_CASE(f32, kRGBA_F32_SkColorType)
        COLOR_TYPE_CASE(1010102, kRGBA_1010102_SkColorType)
        COLOR_TYPE_CASE(1010102_xr, kBGR_101010x_XR_SkColorType)
        COLOR_TYPE_CASE(10x6, kRGBA_10x6_SkColorType)

#undef COLOR_TYPE_CASE

        // Odd stage that doesn't have a load variant (appendLoad uses load_a8 + alpha_to_red)
        case Op::store_r8: {
            ct = kR8_unorm_SkColorType;
            isStore = true;
            break;
        }
        case Op::srcover_rgba_8888: {
            ct = kRGBA_8888_SkColorType;
            isLoad = true;
            isStore = true;
            break;
        }
        case Op::scale_u8:
        case Op::lerp_u8: {
            ct = kAlpha_8_SkColorType;
            isLoad = true;
            break;
        }
        case Op::scale_565:
        case Op::lerp_565: {
            ct = kRGB_565_SkColorType;
            isLoad = true;
            break;
        }
        case Op::emboss: {
            // Special-case, this op uses a context that holds *two* MemoryCtxs
            SkRasterPipeline_EmbossCtx* embossCtx = (SkRasterPipeline_EmbossCtx*)ctx;
            this->addMemoryContext(&embossCtx->add,
                                   SkColorTypeBytesPerPixel(kAlpha_8_SkColorType),
                                   /*load=*/true, /*store=*/false);
            this->addMemoryContext(&embossCtx->mul,
                                   SkColorTypeBytesPerPixel(kAlpha_8_SkColorType),
                                   /*load=*/true, /*store=*/false);
            break;
        }
        case Op::init_lane_masks: {
            auto* initCtx = (SkRasterPipeline_InitLaneMasksCtx*)ctx;
            initCtx->tail = this->tailPointer();
            break;
        }
        case Op::branch_if_all_lanes_active: {
            auto* branchCtx = (SkRasterPipeline_BranchIfAllLanesActiveCtx*)ctx;
            branchCtx->tail = this->tailPointer();
            break;
        }
        default:
            break;
    }

    fStages = fAlloc->make<StageList>(StageList{fStages, op, ctx});
    fNumStages += 1;

    if (isLoad || isStore) {
        SkASSERT(ct != kUnknown_SkColorType);
        this->addMemoryContext(
                (SkRasterPipeline_MemoryCtx*)ctx, SkColorTypeBytesPerPixel(ct), isLoad, isStore);
    }
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

        // We make sure that all ops use _our_ stack context and tail pointer.
        switch (stages[n].stage) {
            case Op::stack_rewind: {
                stages[n].ctx = fRewindCtx;
                break;
            }
            case Op::init_lane_masks: {
                auto* ctx = (SkRasterPipeline_InitLaneMasksCtx*)stages[n].ctx;
                ctx->tail = this->tailPointer();
                break;
            }
            case Op::branch_if_all_lanes_active: {
                auto* ctx = (SkRasterPipeline_BranchIfAllLanesActiveCtx*)stages[n].ctx;
                ctx->tail = this->tailPointer();
                break;
            }
            default:
                break;
        }

        st = st->prev;
    }
    stages[0]      = *st;
    stages[0].prev = fStages;

    fStages = &stages[src.fNumStages - 1];
    fNumStages += src.fNumStages;
    for (const SkRasterPipeline_MemoryCtxInfo& info : src.fMemoryCtxInfos) {
        this->addMemoryContext(info.context, info.bytesPerPixel, info.load, info.store);
    }
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

void SkRasterPipeline::appendSetRGB(SkArenaAlloc* alloc, const float rgb[3]) {
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

    this->uncheckedAppend(op, arg);
}

void SkRasterPipeline::appendConstantColor(SkArenaAlloc* alloc, const float rgba[4]) {
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
            this->uncheckedAppend(Op::uniform_color, ctx);
        } else {
            this->uncheckedAppend(Op::unbounded_uniform_color, ctx);
        }
    }
}

void SkRasterPipeline::appendMatrix(SkArenaAlloc* alloc, const SkMatrix& matrix) {
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

void SkRasterPipeline::appendLoad(SkColorType ct, const SkRasterPipeline_MemoryCtx* ctx) {
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
            this->appendTransferFunction(*skcms_sRGB_TransferFunction());
            break;
    }
}

void SkRasterPipeline::appendLoadDst(SkColorType ct, const SkRasterPipeline_MemoryCtx* ctx) {
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
            this->appendTransferFunction(*skcms_sRGB_TransferFunction());
            this->append(Op::swap_src_dst);
            break;
    }
}

void SkRasterPipeline::appendStore(SkColorType ct, const SkRasterPipeline_MemoryCtx* ctx) {
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
            this->appendTransferFunction(*skcms_sRGB_Inverse_TransferFunction());
            this->append(Op::store_8888, ctx);
            break;
    }
}

void SkRasterPipeline::appendTransferFunction(const skcms_TransferFunction& tf) {
    void* ctx = const_cast<void*>(static_cast<const void*>(&tf));
    switch (skcms_TransferFunction_getType(&tf)) {
        case skcms_TFType_Invalid: SkASSERT(false); break;

        case skcms_TFType_sRGBish:
            if (tf.a == 1 && tf.b == 0 && tf.c == 0 && tf.d == 0 && tf.e == 0 && tf.f == 0) {
                this->uncheckedAppend(Op::gamma_, ctx);
            } else {
                this->uncheckedAppend(Op::parametric, ctx);
            }
            break;
        case skcms_TFType_PQish:     this->uncheckedAppend(Op::PQish,     ctx); break;
        case skcms_TFType_HLGish:    this->uncheckedAppend(Op::HLGish,    ctx); break;
        case skcms_TFType_HLGinvish: this->uncheckedAppend(Op::HLGinvish, ctx); break;
    }
}

// GPUs clamp all color channels to the limits of the format just before the blend step. To match
// that auto-clamp, the RP blitter uses this helper immediately before appending blending stages.
void SkRasterPipeline::appendClampIfNormalized(const SkImageInfo& info) {
    if (SkColorTypeIsNormalized(info.colorType())) {
        this->uncheckedAppend(Op::clamp_01, nullptr);
    }
}

void SkRasterPipeline::appendStackRewind() {
    if (!fRewindCtx) {
        fRewindCtx = fAlloc->make<SkRasterPipeline_RewindCtx>();
    }
    this->uncheckedAppend(Op::stack_rewind, fRewindCtx);
}

static void prepend_to_pipeline(SkRasterPipelineStage*& ip, SkOpts::StageFn stageFn, void* ctx) {
    --ip;
    ip->fn = stageFn;
    ip->ctx = ctx;
}

bool SkRasterPipeline::buildLowpPipeline(SkRasterPipelineStage* ip) const {
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

void SkRasterPipeline::buildHighpPipeline(SkRasterPipelineStage* ip) const {
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

SkRasterPipeline::StartPipelineFn SkRasterPipeline::buildPipeline(SkRasterPipelineStage* ip) const {
    // We try to build a lowp pipeline first; if that fails, we fall back to a highp float pipeline.
    if (this->buildLowpPipeline(ip)) {
        return SkOpts::start_pipeline_lowp;
    }

    this->buildHighpPipeline(ip);
    return SkOpts::start_pipeline_highp;
}

int SkRasterPipeline::stagesNeeded() const {
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

    int stagesNeeded = this->stagesNeeded();

    // Best to not use fAlloc here... we can't bound how often run() will be called.
    AutoSTMalloc<32, SkRasterPipelineStage> program(stagesNeeded);

    int numMemoryCtxs = fMemoryCtxInfos.size();
    AutoSTMalloc<2, SkRasterPipeline_MemoryCtxPatch> patches(numMemoryCtxs);
    for (int i = 0; i < numMemoryCtxs; ++i) {
        patches[i].info = fMemoryCtxInfos[i];
        patches[i].backup = nullptr;
        memset(patches[i].scratch, 0, sizeof(patches[i].scratch));
    }

    auto start_pipeline = this->buildPipeline(program.get() + stagesNeeded);
    start_pipeline(x, y, x + w, y + h, program.get(),
                   SkSpan{patches.data(), numMemoryCtxs},
                   fTailPointer);
}

std::function<void(size_t, size_t, size_t, size_t)> SkRasterPipeline::compile() const {
    if (this->empty()) {
        return [](size_t, size_t, size_t, size_t) {};
    }

    int stagesNeeded = this->stagesNeeded();

    SkRasterPipelineStage* program = fAlloc->makeArray<SkRasterPipelineStage>(stagesNeeded);

    int numMemoryCtxs = fMemoryCtxInfos.size();
    SkRasterPipeline_MemoryCtxPatch* patches =
            fAlloc->makeArray<SkRasterPipeline_MemoryCtxPatch>(numMemoryCtxs);
    for (int i = 0; i < numMemoryCtxs; ++i) {
        patches[i].info = fMemoryCtxInfos[i];
        patches[i].backup = nullptr;
        memset(patches[i].scratch, 0, sizeof(patches[i].scratch));
    }
    uint8_t* tailPointer = fTailPointer;

    auto start_pipeline = this->buildPipeline(program + stagesNeeded);
    return [=](size_t x, size_t y, size_t w, size_t h) {
        start_pipeline(x, y, x + w, y + h, program,
                       SkSpan{patches, numMemoryCtxs},
                       tailPointer);
    };
}

void SkRasterPipeline::addMemoryContext(SkRasterPipeline_MemoryCtx* ctx,
                                        int bytesPerPixel,
                                        bool load,
                                        bool store) {
    SkRasterPipeline_MemoryCtxInfo* info =
            std::find_if(fMemoryCtxInfos.begin(), fMemoryCtxInfos.end(),
                         [=](const SkRasterPipeline_MemoryCtxInfo& i) { return i.context == ctx; });
    if (info != fMemoryCtxInfos.end()) {
        SkASSERT(bytesPerPixel == info->bytesPerPixel);
        info->load = info->load || load;
        info->store = info->store || store;
    } else {
        fMemoryCtxInfos.push_back(SkRasterPipeline_MemoryCtxInfo{ctx, bytesPerPixel, load, store});
    }
}
