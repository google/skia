/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkArenaAlloc.h"
#include "SkColorSpaceXformer.h"
#include "SkRasterPipeline.h"
#include "SkReadBuffer.h"
#include "SkWriteBuffer.h"
#include "SkString.h"
#include "../jumper/SkJumper.h"

class SkMixerShader : public SkShaderBase {
public:
    SkMixerShader(sk_sp<SkShader> sA, sk_sp<SkShader> sB, float t)
        : fShaderA(std::move(sA))
        , fShaderB(std::move(sB))
        , fT(t)
    {}

#if SK_SUPPORT_GPUx
    sk_sp<GrFragmentProcessor> asFragmentProcessor(const AsFPArgs&) const override;
#endif

    SK_TO_STRING_OVERRIDE()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkMixerShader)

protected:
    void flatten(SkWriteBuffer& buffer) const override {
        buffer.writeFlattenable(fShaderA.get());
        buffer.writeFlattenable(fShaderB.get());
        buffer.writeScalar(fT);
    }

    sk_sp<SkShader> onMakeColorSpace(SkColorSpaceXformer* xformer) const override {
        return SkShader::MakeMixer(xformer->apply(fShaderA.get()),
                                   xformer->apply(fShaderA.get()), fT);
    }

    bool onAppendStages(SkRasterPipeline*, SkColorSpace* dstCS, SkArenaAlloc*,
                        const SkMatrix&, const SkPaint&, const SkMatrix* localM) const override;

    bool isRasterPipelineOnly() const final { return true; }

private:
    sk_sp<SkShader> fShaderA;
    sk_sp<SkShader> fShaderB;
    const float     fT;

    typedef SkShaderBase INHERITED;
};

sk_sp<SkFlattenable> SkMixerShader::CreateProc(SkReadBuffer& buffer) {
    sk_sp<SkShader> shaderA(buffer.readShader());
    sk_sp<SkShader> shaderB(buffer.readShader());
    const float     t = buffer.readScalar();

    return SkShader::MakeMixer(shaderA, shaderB, t);
}

bool SkMixerShader::onAppendStages(SkRasterPipeline* pipeline, SkColorSpace* dstCS,
                                   SkArenaAlloc* alloc, const SkMatrix& ctm,
                                   const SkPaint& paint, const SkMatrix* localM) const {
    struct Storage {
        float   fRGBA[4 * SkJumper_kMaxStride];
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

    // Here we end the similarity with SkComposeShader, as we must apply fT instead of a mode

    pipeline->append(SkRasterPipeline::lerp_1_float, &fT);
    return true;
}

#if SK_SUPPORT_GPUx

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
void SkMixerShader::toString(SkString* str) const {
    str->append("SkComposeShader: (");

    str->append("ShaderA: ");
    as_SB(fShaderA)->toString(str);
    str->append(" ShaderB: ");
    as_SB(fShaderB)->toString(str);
    str->appendf(" t: %g", fT);

    this->INHERITED::toString(str);

    str->append(")");
}
#endif

sk_sp<SkShader> SkShader::MakeMixer(sk_sp<SkShader> a, sk_sp<SkShader> b, float t) {
    if (!a || !b || !SkScalarIsFinite(t)) {
        return nullptr;
    }
    if (t <= 0) {
        return a;
    } else if (t >= 1) {
        return b;
    } else {
        return sk_sp<SkShader>(new SkMixerShader(std::move(a), std::move(b), t));
    }
}
