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

#ifdef SK_SUPPORT_LEGACY_SHADER_FACTORIES
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
#endif

///////////////////////////////////////////////////////////////////////////////

sk_sp<SkFlattenable> SkShader_Mixer::CreateProc(SkReadBuffer& buffer) {
    sk_sp<SkShader> s0(buffer.readShader());
    sk_sp<SkShader> s1(buffer.readShader());
    sk_sp<SkMixer>  mx(buffer.readMixer());

#ifdef SK_SUPPORT_LEGACY_SHADER_FACTORIES
    return MakeMixer(std::move(s0), std::move(s1), std::move(mx));
#else
    return nullptr;
#endif
}

void SkShader_Mixer::flatten(SkWriteBuffer& buffer) const {
    buffer.writeFlattenable(fShader0.get());
    buffer.writeFlattenable(fShader1.get());
    buffer.writeFlattenable(fMixer.get());
}

bool SkShader_Mixer::onAppendStages(const SkStageRec& rec) const {
    struct Storage {
        float   fRGBA[4 * SkRasterPipeline_kMaxStride];
    };
    auto storage = rec.fAlloc->make<Storage>();

    if (!as_SB(fShader0)->appendStages(rec)) {
        return false;
    }
    rec.fPipeline->append(SkRasterPipeline::store_src, storage->fRGBA);

    if (!as_SB(fShader1)->appendStages(rec)) {
        return false;
    }
    // r,g,b,a are good, as output by fShader1
    // need to restore our previously computed dr,dg,db,da
    rec.fPipeline->append(SkRasterPipeline::load_dst, storage->fRGBA);

    // 1st color in dr,dg,db,da
    // 2nd color in  r, g, b, a
    return as_MB(fMixer)->appendStages(rec);
}

#if SK_SUPPORT_GPU

#include "effects/generated/GrConstColorProcessor.h"
#include "effects/GrXfermodeFragmentProcessor.h"

/////////////////////////////////////////////////////////////////////

std::unique_ptr<GrFragmentProcessor>
SkShader_Mixer::asFragmentProcessor(const GrFPArgs& args) const {
    return as_MB(fMixer)->asFragmentProcessor(args, fShader0, fShader1);
}
#endif
