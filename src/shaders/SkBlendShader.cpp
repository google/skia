/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/shaders/SkBlendShader.h"

#include "include/core/SkBlendMode.h"
#include "include/core/SkBlender.h"
#include "include/core/SkData.h"
#include "include/core/SkFlattenable.h"
#include "include/effects/SkRuntimeEffect.h"
#include "src/base/SkArenaAlloc.h"
#include "src/core/SkBlendModePriv.h"
#include "src/core/SkBlenderBase.h"
#include "src/core/SkEffectPriv.h"
#include "src/core/SkRasterPipeline.h"
#include "src/core/SkRasterPipelineOpContexts.h"
#include "src/core/SkRasterPipelineOpList.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkRuntimeEffectPriv.h"
#include "src/core/SkWriteBuffer.h"
#include "src/shaders/SkShaderBase.h"

#include <optional>

sk_sp<SkFlattenable> SkBlendShader::CreateProc(SkReadBuffer& buffer) {
    sk_sp<SkShader> dst(buffer.readShader());
    sk_sp<SkShader> src(buffer.readShader());
    if (!buffer.validate(dst && src)) {
        return nullptr;
    }

    unsigned mode = buffer.read32();

    if (mode == kCustom_SkBlendMode) {
        sk_sp<SkBlender> blender = buffer.readBlender();
        if (buffer.validate(blender != nullptr)) {
            return SkShaders::Blend(std::move(blender), std::move(dst), std::move(src));
        }
    } else {
        if (buffer.validate(mode <= (unsigned)SkBlendMode::kLastMode)) {
            return SkShaders::Blend(static_cast<SkBlendMode>(mode), std::move(dst), std::move(src));
        }
    }
    return nullptr;
}

void SkBlendShader::flatten(SkWriteBuffer& buffer) const {
    buffer.writeFlattenable(fDst.get());
    buffer.writeFlattenable(fSrc.get());
    buffer.write32((int)fMode);
}

// Returns the output of e0, and leaves the output of e1 in r,g,b,a
static float* append_two_shaders(const SkStageRec& rec,
                                 const SkShaders::MatrixRec& mRec,
                                 SkShader* s0,
                                 SkShader* s1) {
    struct Storage {
        float fCoords[2 * SkRasterPipeline_kMaxStride];
        float fRes0[4 * SkRasterPipeline_kMaxStride];
    };
    auto storage = rec.fAlloc->make<Storage>();

    // Note we cannot simply apply mRec here and then unconditionally store the coordinates. When
    // building for Android Framework it would interrupt the backwards local matrix concatenation if
    // mRec had a pending local matrix and either of the children also had a local matrix.
    // b/256873449
    if (mRec.rasterPipelineCoordsAreSeeded()) {
        rec.fPipeline->append(SkRasterPipelineOp::store_src_rg, storage->fCoords);
    }
    if (!as_SB(s0)->appendStages(rec, mRec)) {
        return nullptr;
    }
    rec.fPipeline->append(SkRasterPipelineOp::store_src, storage->fRes0);

    if (mRec.rasterPipelineCoordsAreSeeded()) {
        rec.fPipeline->append(SkRasterPipelineOp::load_src_rg, storage->fCoords);
    }
    if (!as_SB(s1)->appendStages(rec, mRec)) {
        return nullptr;
    }
    return storage->fRes0;
}

bool SkBlendShader::appendStages(const SkStageRec& rec, const SkShaders::MatrixRec& mRec) const {
    float* res0 = append_two_shaders(rec, mRec, fDst.get(), fSrc.get());
    if (!res0) {
        return false;
    }

    rec.fPipeline->append(SkRasterPipelineOp::load_dst, res0);
    SkBlendMode_AppendStages(fMode, rec.fPipeline);
    return true;
}

sk_sp<SkShader> SkShaders::Blend(SkBlendMode mode, sk_sp<SkShader> dst, sk_sp<SkShader> src) {
    if (!src || !dst) {
        return nullptr;
    }
    switch (mode) {
        case SkBlendMode::kClear:
            return Color(0);
        case SkBlendMode::kDst:
            return dst;
        case SkBlendMode::kSrc:
            return src;
        default:
            break;
    }
    return sk_sp<SkShader>(new SkBlendShader(mode, std::move(dst), std::move(src)));
}

sk_sp<SkShader> SkShaders::Blend(sk_sp<SkBlender> blender,
                                 sk_sp<SkShader> dst,
                                 sk_sp<SkShader> src) {
    if (!src || !dst) {
        return nullptr;
    }
    if (!blender) {
        return SkShaders::Blend(SkBlendMode::kSrcOver, std::move(dst), std::move(src));
    }
    if (std::optional<SkBlendMode> mode = as_BB(blender)->asBlendMode()) {
        return sk_make_sp<SkBlendShader>(mode.value(), std::move(dst), std::move(src));
    }

    // This isn't a built-in blend mode; we might as well use a runtime effect to evaluate it.
    static SkRuntimeEffect* sBlendEffect = SkMakeRuntimeEffect(SkRuntimeEffect::MakeForShader,
        "uniform shader s, d;"
        "uniform blender b;"
        "half4 main(float2 xy) {"
            "return b.eval(s.eval(xy), d.eval(xy));"
        "}"
    );
    SkRuntimeEffect::ChildPtr children[] = {std::move(src), std::move(dst), std::move(blender)};
    return sBlendEffect->makeShader(/*uniforms=*/{}, children);
}

void SkRegisterBlendShaderFlattenable() {
    SK_REGISTER_FLATTENABLE(SkBlendShader);
    // Previous name
    SkFlattenable::Register("SkShader_Blend", SkBlendShader::CreateProc);
}
