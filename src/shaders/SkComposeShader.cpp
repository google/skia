/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkArenaAlloc.h"
#include "SkBlendModePriv.h"
#include "SkComposeShader.h"
#include "SkColorFilter.h"
#include "SkColorPriv.h"
#include "SkColorShader.h"
#include "SkRasterPipeline.h"
#include "SkReadBuffer.h"
#include "SkWriteBuffer.h"
#include "SkString.h"
#include "../jumper/SkJumper.h"

sk_sp<SkShader> SkShader::MakeComposeShader(sk_sp<SkShader> dst, sk_sp<SkShader> src,
                                            SkBlendMode mode) {
    if (!src || !dst) {
        return nullptr;
    }
    if (SkBlendMode::kSrc == mode) {
        return src;
    }
    if (SkBlendMode::kDst == mode) {
        return dst;
    }
    return sk_sp<SkShader>(new SkComposeShader(std::move(dst), std::move(src), mode));
}

///////////////////////////////////////////////////////////////////////////////

sk_sp<SkFlattenable> SkComposeShader::CreateProc(SkReadBuffer& buffer) {
    sk_sp<SkShader> shaderA(buffer.readShader());
    sk_sp<SkShader> shaderB(buffer.readShader());
    SkBlendMode mode;
    if (buffer.isVersionLT(SkReadBuffer::kXfermodeToBlendMode2_Version)) {
        sk_sp<SkXfermode> xfer = buffer.readXfermode();
        mode = xfer ? xfer->blend() : SkBlendMode::kSrcOver;
    } else {
        mode = (SkBlendMode)buffer.read32();
    }
    if (!shaderA || !shaderB) {
        return nullptr;
    }
    return sk_make_sp<SkComposeShader>(std::move(shaderA), std::move(shaderB), mode);
}

void SkComposeShader::flatten(SkWriteBuffer& buffer) const {
    buffer.writeFlattenable(fShaderA.get());
    buffer.writeFlattenable(fShaderB.get());
    buffer.write32((int)fMode);
}

sk_sp<SkShader> SkComposeShader::onMakeColorSpace(SkColorSpaceXformer* xformer) const {
    return SkShader::MakeComposeShader(xformer->apply(fShaderA.get()),
                                       xformer->apply(fShaderB.get()), fMode);
}

bool SkComposeShader::asACompose(ComposeRec* rec) const {
    if (rec) {
        rec->fShaderA   = fShaderA.get();
        rec->fShaderB   = fShaderB.get();
        rec->fBlendMode = fMode;
    }
    return true;
}

bool SkComposeShader::isRasterPipelineOnly() const {
    return true;
}

bool SkComposeShader::onAppendStages(SkRasterPipeline* pipeline, SkColorSpace* dstCS,
                                     SkArenaAlloc* alloc, const SkMatrix& ctm,
                                     const SkPaint& paint, const SkMatrix* localM) const {
    struct Storage {
        float   fRGBA[4 * SkJumper_kMaxStride];
        float   fAlpha;
    };
    auto storage = alloc->make<Storage>();

    if (!as_SB(fShaderB)->appendStages(pipeline, dstCS, alloc, ctm, paint, localM)) { // SRC
        return false;
    }
    // This outputs r,g,b,a, which we'll need later when we apply the mode, but we save it off now
    // since fShaderB will overwrite them.
    pipeline->append(SkRasterPipeline::store_rgba, storage->fRGBA);

    if (!as_SB(fShaderA)->appendStages(pipeline, dstCS, alloc, ctm, paint, localM)) {  // DST
        return false;
    }
    // We now have our logical 'dst' in r,g,b,a, but we need it in dr,dg,db,da for the mode
    // so we have to shuttle them. If we had a stage the would load_into_dst, then we could
    // reverse the two shader invocations, and avoid this move...
    pipeline->append(SkRasterPipeline::move_src_dst);
    pipeline->append(SkRasterPipeline::load_rgba, storage->fRGBA);

    // Idea: should time this, and see if it helps to have custom versions of the overflow modes
    //       that do their own clamping, avoiding the overhead of an extra stage.
    SkBlendMode_AppendStages(fMode, pipeline);
    if (SkBlendMode_CanOverflow(fMode)) {
        pipeline->append(SkRasterPipeline::clamp_a);
    }
    return true;
}

#if SK_SUPPORT_GPU

#include "effects/GrConstColorProcessor.h"
#include "effects/GrXfermodeFragmentProcessor.h"

/////////////////////////////////////////////////////////////////////

sk_sp<GrFragmentProcessor> SkComposeShader::asFragmentProcessor(const AsFPArgs& args) const {
    switch (fMode) {
        case SkBlendMode::kClear:
            return GrConstColorProcessor::Make(GrColor4f::TransparentBlack(),
                                               GrConstColorProcessor::kIgnore_InputMode);
            break;
        case SkBlendMode::kSrc:
            return as_SB(fShaderB)->asFragmentProcessor(args);
            break;
        case SkBlendMode::kDst:
            return as_SB(fShaderA)->asFragmentProcessor(args);
            break;
        default:
            sk_sp<GrFragmentProcessor> fpA(as_SB(fShaderA)->asFragmentProcessor(args));
            if (!fpA) {
                return nullptr;
            }
            sk_sp<GrFragmentProcessor> fpB(as_SB(fShaderB)->asFragmentProcessor(args));
            if (!fpB) {
                return nullptr;
            }
            return GrXfermodeFragmentProcessor::MakeFromTwoProcessors(std::move(fpB),
                                                                      std::move(fpA), fMode);
    }
}
#endif

#ifndef SK_IGNORE_TO_STRING
void SkComposeShader::toString(SkString* str) const {
    str->append("SkComposeShader: (");

    str->append("ShaderA: ");
    as_SB(fShaderA)->toString(str);
    str->append(" ShaderB: ");
    as_SB(fShaderB)->toString(str);
    if (SkBlendMode::kSrcOver != fMode) {
        str->appendf(" Xfermode: %s", SkBlendMode_Name(fMode));
    }

    this->INHERITED::toString(str);

    str->append(")");
}
#endif
