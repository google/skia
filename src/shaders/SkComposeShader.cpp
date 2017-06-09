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

sk_sp<SkShader> SkShader::MakeCompose(sk_sp<SkShader> dst, sk_sp<SkShader> src, SkBlendMode mode,
                                      float lerpT) {
    if (!src || !dst || SkScalarIsNaN(lerpT)) {
        return nullptr;
    }
    lerpT = SkScalarPin(lerpT, 0, 1);

    if (lerpT == 0) {
        return dst;
    } else if (lerpT == 1) {
        if (mode == SkBlendMode::kSrc) {
            return src;
        }
        if (mode == SkBlendMode::kDst) {
            return dst;
        }
    }
    return sk_sp<SkShader>(new SkComposeShader(std::move(dst), std::move(src), mode, lerpT));
}

///////////////////////////////////////////////////////////////////////////////

sk_sp<SkFlattenable> SkComposeShader::CreateProc(SkReadBuffer& buffer) {
    sk_sp<SkShader> dst(buffer.readShader());
    sk_sp<SkShader> src(buffer.readShader());
    unsigned        mode = buffer.read32();

    float lerp = 1;
    if (!buffer.isVersionLT(SkReadBuffer::kComposeShaderCanLerp_Version)) {
        lerp = buffer.readScalar();
    }

    // check for valid mode before we cast to the enum type
    if (mode > (unsigned)SkBlendMode::kLastMode) {
        return nullptr;
    }

    return MakeCompose(std::move(dst), std::move(src), static_cast<SkBlendMode>(mode), lerp);
}

void SkComposeShader::flatten(SkWriteBuffer& buffer) const {
    buffer.writeFlattenable(fDst.get());
    buffer.writeFlattenable(fSrc.get());
    buffer.write32((int)fMode);
    buffer.writeScalar(fLerpT);
}

sk_sp<SkShader> SkComposeShader::onMakeColorSpace(SkColorSpaceXformer* xformer) const {
    return MakeCompose(xformer->apply(fDst.get()), xformer->apply(fSrc.get()),
                       fMode, fLerpT);
}

#ifdef SK_BUILD_FOR_ANDROID_FRAMEWORK
bool SkComposeShader::asACompose(ComposeRec* rec) const {
    if (!this->isJustMode()) {
        return false;
    }

    if (rec) {
        rec->fShaderA   = fDst.get();
        rec->fShaderB   = fSrc.get();
        rec->fBlendMode = fMode;
    }
    return true;
}
#endif

bool SkComposeShader::onAppendStages(SkRasterPipeline* pipeline, SkColorSpace* dstCS,
                                     SkArenaAlloc* alloc, const SkMatrix& ctm,
                                     const SkPaint& paint, const SkMatrix* localM) const {
    struct Storage {
        float   fRGBA[4 * SkJumper_kMaxStride];
        float   fAlpha;
    };
    auto storage = alloc->make<Storage>();

    if (!as_SB(fSrc)->appendStages(pipeline, dstCS, alloc, ctm, paint, localM)) {
        return false;
    }
    // This outputs r,g,b,a, which we'll need later when we apply the mode, but we save it off now
    // since fShaderB will overwrite them.
    pipeline->append(SkRasterPipeline::store_rgba, storage->fRGBA);

    if (!as_SB(fDst)->appendStages(pipeline, dstCS, alloc, ctm, paint, localM)) {
        return false;
    }
    // We now have our logical 'dst' in r,g,b,a, but we need it in dr,dg,db,da for the mode/lerp
    // so we have to shuttle them. If we had a stage the would load_into_dst, then we could
    // reverse the two shader invocations, and avoid this move...
    pipeline->append(SkRasterPipeline::move_src_dst);
    pipeline->append(SkRasterPipeline::load_rgba, storage->fRGBA);

    if (!this->isJustLerp()) {
        SkBlendMode_AppendStages(fMode, pipeline);
    }
    if (!this->isJustMode()) {
        pipeline->append(SkRasterPipeline::lerp_1_float, &fLerpT);
    }
    return true;
}

#if SK_SUPPORT_GPU

#include "effects/GrConstColorProcessor.h"
#include "effects/GrXfermodeFragmentProcessor.h"

/////////////////////////////////////////////////////////////////////

sk_sp<GrFragmentProcessor> SkComposeShader::asFragmentProcessor(const AsFPArgs& args) const {
    if (this->isJustMode()) {
        SkASSERT(fMode != SkBlendMode::kSrc && fMode != SkBlendMode::kDst); // caught in factory
        if (fMode == SkBlendMode::kClear) {
            return GrConstColorProcessor::Make(GrColor4f::TransparentBlack(),
                                               GrConstColorProcessor::kIgnore_InputMode);
        }
    }

    sk_sp<GrFragmentProcessor> fpA(as_SB(fDst)->asFragmentProcessor(args));
    if (!fpA) {
        return nullptr;
    }
    sk_sp<GrFragmentProcessor> fpB(as_SB(fSrc)->asFragmentProcessor(args));
    if (!fpB) {
        return nullptr;
    }
    // TODO: account for fLerpT when it is < 1
    return GrXfermodeFragmentProcessor::MakeFromTwoProcessors(std::move(fpB),
                                                              std::move(fpA), fMode);
}
#endif

#ifndef SK_IGNORE_TO_STRING
void SkComposeShader::toString(SkString* str) const {
    str->append("SkComposeShader: (");

    str->append("dst: ");
    as_SB(fDst)->toString(str);
    str->append(" src: ");
    as_SB(fSrc)->toString(str);
    str->appendf(" mode: %s", SkBlendMode_Name(fMode));
    str->appendf(" lerpT: %g", fLerpT);

    this->INHERITED::toString(str);

    str->append(")");
}
#endif
