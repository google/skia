/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkArenaAlloc.h"
#include "SkMixerShader.h"
#include "SkRasterPipeline.h"
#include "SkReadBuffer.h"
#include "SkWriteBuffer.h"
#include "SkString.h"

sk_sp<SkShader> SkShader::MakeMixer(sk_sp<SkShader> s0, sk_sp<SkShader> s1, sk_sp<SkMixer> mixer) {
    if (!mixer) {
        return nullptr;
    }
    if (!s0) {
        return s1;
    }
    if (!s1) {
        return s0;
    }
    return sk_sp<SkShader>(new SkShader_Mixer(std::move(s0), std::move(s1), std::move(mixer)));
}

///////////////////////////////////////////////////////////////////////////////

sk_sp<SkFlattenable> SkShader_Mixer::CreateProc(SkReadBuffer& buffer) {
    sk_sp<SkShader> s0(buffer.readShader());
    sk_sp<SkShader> s1(buffer.readShader());
    sk_sp<SkMixer>  mx(buffer.readMixer());

    return MakeMixer(std::move(s0), std::move(s1), std::move(mx));
}

void SkShader_Mixer::flatten(SkWriteBuffer& buffer) const {
    buffer.writeFlattenable(fShader0.get());
    buffer.writeFlattenable(fShader1.get());
    buffer.writeFlattenable(fMixer.get());
}

bool SkShader_Mixer::onAppendStages(const StageRec& rec) const {
    struct Storage {
        float   fRGBA[4 * SkRasterPipeline_kMaxStride];
        float   fAlpha;
    };
    auto storage = rec.fAlloc->make<Storage>();

    if (!as_SB(fShader0)->appendStages(rec)) {
        return false;
    }
    // This outputs r,g,b,a, which we'll need later when we apply the mixer, but we save it off now
    // since fShader1 will overwrite them.
    rec.fPipeline->append(SkRasterPipeline::store_rgba, storage->fRGBA);

    if (!as_SB(fShader1)->appendStages(rec)) {
        return false;
    }
    // We now have our 2nd input in r,g,b,a, but we need it in dr,dg,db,da for the mixer
    // so we have to shuttle them. If we had a stage the would load_into_dst, then we could
    // reverse the two shader invocations, and avoid this move...
    rec.fPipeline->append(SkRasterPipeline::move_src_dst);
    rec.fPipeline->append(SkRasterPipeline::load_rgba, storage->fRGBA);

    // 1st color in  r, g, b, a
    // 2nd color in dr,dg,db,da
    // The mixer's output will be in r,g,b,a
    return as_MB(fMixer)->appendStages(rec.fPipeline, rec.fDstCS, rec.fAlloc);
}

#if SK_SUPPORT_GPU

#include "effects/GrConstColorProcessor.h"
#include "effects/GrXfermodeFragmentProcessor.h"

/////////////////////////////////////////////////////////////////////

std::unique_ptr<GrFragmentProcessor>
SkShader_Mixer::asFragmentProcessor(const GrFPArgs& args) const {
    std::unique_ptr<GrFragmentProcessor> fpA(as_SB(fShader0)->asFragmentProcessor(args));
    if (!fpA) {
        return nullptr;
    }
    std::unique_ptr<GrFragmentProcessor> fpB(as_SB(fShader1)->asFragmentProcessor(args));
    if (!fpB) {
        return nullptr;
    }

    // TODO: need to make a mixer-processor...
    return nullptr;
    //return GrXfermodeFragmentProcessor::MakeFromTwoProcessors(std::move(fpB), std::move(fpA), fMode);
}
#endif
