/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPrecompile_DEFINED
#define SkPrecompile_DEFINED

#include "include/core/SkTypes.h"

#ifdef SK_ENABLE_PRECOMPILE

#include "include/core/SkBlendMode.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSpan.h"

#include <optional>
#include <vector>

class SkRuntimeEffect;

//--------------------------------------------------------------------------------------------------
class SkPrecompileShader : public SkRefCnt {
public:
    // TODO: add a type enum to each of these base classes
};

class SkPrecompileMaskFilter : public SkRefCnt {
public:

};

class SkPrecompileColorFilter : public SkRefCnt {
public:

};

class SkPrecompileImageFilter : public SkRefCnt {
public:

};

class SkPrecompileBlender : public SkRefCnt {
public:
    virtual std::optional<SkBlendMode> asBlendMode() const { return {}; }

    static sk_sp<SkPrecompileBlender> Mode(SkBlendMode blendMode);

};

//--------------------------------------------------------------------------------------------------
class SkPaintOptions {
public:
    void setShaders(SkSpan<const sk_sp<SkPrecompileShader>> shaders) {
        fShaders.assign(shaders.begin(), shaders.end());
    }

    void setMaskFilters(SkSpan<const sk_sp<SkPrecompileMaskFilter>> maskFilters) {
        fMaskFilters.assign(maskFilters.begin(), maskFilters.end());
    }

    void setColorFilters(SkSpan<const sk_sp<SkPrecompileColorFilter>> colorFilters) {
        fColorFilters.assign(colorFilters.begin(), colorFilters.end());
    }

    void setImageFilters(SkSpan<const sk_sp<SkPrecompileImageFilter>> imageFilters) {
        fImageFilters.assign(imageFilters.begin(), imageFilters.end());
    }

    void setBlendModes(SkSpan<SkBlendMode> blendModes) {
        fBlenders.reserve(blendModes.size());
        for (SkBlendMode bm : blendModes) {
            fBlenders.emplace_back(SkPrecompileBlender::Mode(bm));
        }
    }
    void setBlenders(SkSpan<const sk_sp<SkPrecompileBlender>> blenders) {
        fBlenders.assign(blenders.begin(), blenders.end());
    }

private:
    std::vector<sk_sp<SkPrecompileShader>> fShaders;
    std::vector<sk_sp<SkPrecompileMaskFilter>> fMaskFilters;
    std::vector<sk_sp<SkPrecompileColorFilter>> fColorFilters;
    std::vector<sk_sp<SkPrecompileImageFilter>> fImageFilters;
    std::vector<sk_sp<SkPrecompileBlender>> fBlenders;
};

#endif // SK_ENABLE_PRECOMPILE

#endif // SkPrecompile_DEFINED
