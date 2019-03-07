/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkMixerPriv_DEFINED
#define SkMixerPriv_DEFINED

#include "SkFlattenable.h"
#include "SkMixer.h"
#include "SkRefCnt.h"

class GrFragmentProcessor;
class GrRecordingContext;
class SkArenaAlloc;
class SkRasterPipeline;
class SkString;

/**
 *  ColorFilters are optional objects in the drawing pipeline. When present in
 *  a paint, they are called with the "src" colors, and return new colors, which
 *  are then passed onto the next stage (either ImageFilter or Xfermode).
 *
 *  All subclasses are required to be reentrant-safe : it must be legal to share
 *  the same instance between several threads.
 */
class SkMixerBase : public SkMixer {
public:
    virtual void appendStages(SkRasterPipeline*, SkColorSpace*, SkArenaAlloc*) const = 0;

#if SK_SUPPORT_GPU
    /**
     *  A subclass may implement this factory function to work with the GPU backend. It returns
     *  a GrFragmentProcessor that implemets the color filter in GPU shader code.
     *
     *  The fragment processor receives a premultiplied input color and produces a premultiplied
     *  output color.
     *
     *  A null return indicates that the color filter isn't implemented for the GPU backend.
     */
    virtual std::unique_ptr<GrFragmentProcessor> asFragmentProcessor(
            GrRecordingContext*, const GrColorSpaceInfo& dstColorSpaceInfo) const;
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
        return sk_sp<SkMixer>(static_cast<SkMixer*>(
                                  SkFlattenable::Deserialize(
                                  kSkMixer_Type, data, size, procs).release()));
    }

private:
    typedef SkFlattenable SkMixer;
};

#endif
