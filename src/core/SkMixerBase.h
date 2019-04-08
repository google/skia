/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkMixerBase_DEFINED
#define SkMixerBase_DEFINED

#include "SkMixer.h"
#include "SkColorData.h"

class GrColorSpaceInfo;
struct GrFPArgs;
class GrFragmentProcessor;
class GrRecordingContext;
struct SkStageRec;
class SkString;

class SkMixerBase : public SkMixer {
public:
    virtual bool appendStages(const SkStageRec&) const = 0;

#if SK_SUPPORT_GPU
    /**
     *  A subclass may implement this factory function to work with the GPU backend. It returns
     *  a GrFragmentProcessor that implements the color filter in GPU shader code.
     *
     *  The fragment processor receives a premultiplied input color and produces a premultiplied
     *  output color.
     *
     *  A null return indicates that the color filter isn't implemented for the GPU backend.
     */
    virtual std::unique_ptr<GrFragmentProcessor>
    asFragmentProcessor(const GrFPArgs& args, const sk_sp<SkShader> shader1,
                        const sk_sp<SkShader> shader2) const = 0;
#endif

    static void RegisterFlattenables();

    static SkFlattenable::Type GetFlattenableType() {
        return kSkMixer_Type;
    }

    SkFlattenable::Type getFlattenableType() const override {
        return kSkMixer_Type;
    }

    static sk_sp<SkMixer> Deserialize(const void* data, size_t size,
                                      const SkDeserialProcs* procs = nullptr) {
        SkFlattenable* ret = SkFlattenable::Deserialize(kSkMixer_Type, data, size, procs).release();
        SkMixer* mx = static_cast<SkMixer*>(ret);
        return sk_sp<SkMixer>(mx);
    }

    SkPMColor4f test_mix(const SkPMColor4f& a, const SkPMColor4f& b) const;

private:
    typedef SkMixer INHERITED;
};


inline SkMixerBase* as_MB(SkMixer* shader) {
    return static_cast<SkMixerBase*>(shader);
}

inline const SkMixerBase* as_MB(const SkMixer* shader) {
    return static_cast<const SkMixerBase*>(shader);
}

inline const SkMixerBase* as_MB(const sk_sp<SkMixer>& shader) {
    return static_cast<SkMixerBase*>(shader.get());
}


#endif
