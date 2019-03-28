/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkArenaAlloc.h"
#include "SkColorFilter_Mixer.h"
#include "SkRasterPipeline.h"
#include "SkReadBuffer.h"
#include "SkWriteBuffer.h"
#include "SkString.h"

sk_sp<SkColorFilter> SkColorFilter::MakeMixer(sk_sp<SkColorFilter> f0, sk_sp<SkColorFilter> f1, sk_sp<SkMixer> mixer) {
    if (!mixer) {
        return nullptr;
    }
    return sk_sp<SkColorFilter>(new SkColorFilter_Mixer(std::move(f0), std::move(f1),
                                                        std::move(mixer)));
}

///////////////////////////////////////////////////////////////////////////////

sk_sp<SkFlattenable> SkColorFilter_Mixer::CreateProc(SkReadBuffer& buffer) {
    sk_sp<SkColorFilter> s0(buffer.readColorFilter());
    sk_sp<SkColorFilter> s1(buffer.readColorFilter());
    sk_sp<SkMixer>  mx(buffer.readMixer());

    return MakeMixer(std::move(s0), std::move(s1), std::move(mx));
}

void SkColorFilter_Mixer::flatten(SkWriteBuffer& buffer) const {
    buffer.writeFlattenable(fFilter0.get());
    buffer.writeFlattenable(fFilter1.get());
    buffer.writeFlattenable(fMixer.get());
}

void SkColorFilter_Mixer::onAppendStages(const SkStageRec& rec,
                                         bool shaderIsOpaque) const {
    struct Storage {
        float   fOrig[4 * SkRasterPipeline_kMaxStride];
        float   fRes0[4 * SkRasterPipeline_kMaxStride];
    };
    Storage* storage = nullptr;

    if (fFilter0 || fFilter1) {
        storage = rec.fAlloc->make<Storage>();
        rec.fPipeline->append(SkRasterPipeline::store_src, storage->fOrig);
    }

    if (fFilter0 && fFilter1) {
        fFilter0->appendStages(rec, shaderIsOpaque);
        rec.fPipeline->append(SkRasterPipeline::store_src, storage->fRes0);

        rec.fPipeline->append(SkRasterPipeline::load_src, storage->fOrig);
        fFilter1->appendStages(rec, shaderIsOpaque);
        rec.fPipeline->append(SkRasterPipeline::load_dst, storage->fRes0);
    } else if (fFilter0 && !fFilter1) {
        fFilter0->appendStages(rec, shaderIsOpaque);
        rec.fPipeline->append(SkRasterPipeline::move_src_dst);
        rec.fPipeline->append(SkRasterPipeline::load_src, storage->fOrig);
    } else if (!fFilter0 && fFilter1) {
        fFilter1->appendStages(rec, shaderIsOpaque);
        rec.fPipeline->append(SkRasterPipeline::load_dst, storage->fOrig);
    } else {
        SkASSERT(!fFilter0);
        SkASSERT(!fFilter1);
        rec.fPipeline->append(SkRasterPipeline::move_src_dst);
    }

    // 1st color in dr,dg,db,da
    // 2nd color in  r, g, b, a
    (void) as_MB(fMixer)->appendStages(rec);
}

#if SK_SUPPORT_GPU

#include "effects/GrConstColorProcessor.h"
#include "effects/GrXfermodeFragmentProcessor.h"

/////////////////////////////////////////////////////////////////////

std::unique_ptr<GrFragmentProcessor>
SkColorFilter_Mixer::asFragmentProcessor(GrRecordingContext*,
                                          const GrColorSpaceInfo& dstColorSpaceInfo) const {
    // TODO: need to make a mixer-processor...
    return nullptr;
}
#endif
