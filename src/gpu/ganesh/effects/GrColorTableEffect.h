/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrColorTableEffect_DEFINED
#define GrColorTableEffect_DEFINED

#include "src/gpu/ganesh/GrFragmentProcessor.h"
#include "src/gpu/ganesh/GrProcessorUnitTest.h"

#include <memory>

class GrRecordingContext;
class GrSurfaceProxyView;
class SkBitmap;
struct GrShaderCaps;

namespace skgpu {
class KeyBuilder;
}

class ColorTableEffect : public GrFragmentProcessor {
public:
    static std::unique_ptr<GrFragmentProcessor> Make(std::unique_ptr<GrFragmentProcessor> inputFP,
                                                     GrRecordingContext* context,
                                                     const SkBitmap& bitmap);

    ~ColorTableEffect() override {}

    const char* name() const override { return "ColorTableEffect"; }

    std::unique_ptr<GrFragmentProcessor> clone() const override {
        return std::unique_ptr<GrFragmentProcessor>(new ColorTableEffect(*this));
    }

    inline static constexpr int kTexEffectFPIndex = 0;
    inline static constexpr int kInputFPIndex = 1;

private:
    std::unique_ptr<ProgramImpl> onMakeProgramImpl() const override;

    void onAddToKey(const GrShaderCaps&, skgpu::KeyBuilder*) const override {}

    bool onIsEqual(const GrFragmentProcessor&) const override { return true; }

    ColorTableEffect(std::unique_ptr<GrFragmentProcessor> inputFP, GrSurfaceProxyView view);

    explicit ColorTableEffect(const ColorTableEffect& that);

    GR_DECLARE_FRAGMENT_PROCESSOR_TEST
};

#endif
