/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkMixerShader_DEFINED
#define SkMixerShader_DEFINED

#include "SkShaderBase.h"
#include "SkMixer.h"

class SkShader_Mixer final : public SkShaderBase {
public:
    SkShader_Mixer(sk_sp<SkShader> s0, sk_sp<SkShader> s1, sk_sp<SkMixer> mixer)
        : fShader0(std::move(s0))
        , fShader1(std::move(s1))
        , fMixer(std::move(mixer))
    {
        SkASSERT(fShader0);
        SkASSERT(fShader1);
        SkASSERT(fMixer);
    }

#if SK_SUPPORT_GPU
    std::unique_ptr<GrFragmentProcessor> asFragmentProcessor(const GrFPArgs&) const override;
#endif

protected:
    SkShader_Mixer(SkReadBuffer&);
    void flatten(SkWriteBuffer&) const override;
    bool onAppendStages(const SkStageRec&) const override;

private:
    SK_FLATTENABLE_HOOKS(SkShader_Mixer)

    sk_sp<SkShader> fShader0;
    sk_sp<SkShader> fShader1;
    sk_sp<SkMixer>  fMixer;

    typedef SkShaderBase INHERITED;
};

#endif
