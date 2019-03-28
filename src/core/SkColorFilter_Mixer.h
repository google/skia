/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkMixerColorFilter_DEFINED
#define SkMixerColorFilter_DEFINED

#include "SkColorFilter.h"
#include "SkMixer.h"

#if SK_SUPPORT_GPU
#include "GrFPArgs.h"
#endif

class SkColorFilter_Mixer final : public SkColorFilter {
public:
    SkColorFilter_Mixer(sk_sp<SkColorFilter> f0, sk_sp<SkColorFilter> f1, sk_sp<SkMixer> mixer)
        : fFilter0(std::move(f0))
        , fFilter1(std::move(f1))
        , fMixer(std::move(mixer))
    {
        SkASSERT(fMixer);
    }

#if SK_SUPPORT_GPU
    std::unique_ptr<GrFragmentProcessor> asFragmentProcessor(
                                                                 GrRecordingContext*, const GrColorSpaceInfo& dstColorSpaceInfo) const override;
#endif

protected:
    SkColorFilter_Mixer(SkReadBuffer&);
    void flatten(SkWriteBuffer&) const override;
    void onAppendStages(const SkStageRec&, bool shaderIsOpaque) const override;

private:
    SK_FLATTENABLE_HOOKS(SkColorFilter_Mixer)

    sk_sp<SkColorFilter> fFilter0;
    sk_sp<SkColorFilter> fFilter1;
    sk_sp<SkMixer>       fMixer;

    friend class SkColorFilter;

    typedef SkColorFilter INHERITED;
};

#endif
