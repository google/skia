/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_Precompile_DEFINED
#define skgpu_graphite_Precompile_DEFINED

#include "include/core/SkTypes.h"

#ifdef SK_ENABLE_PRECOMPILE

#include "include/core/SkBlendMode.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSpan.h"

#include <optional>
#include <vector>

class SkRuntimeEffect;

namespace skgpu::graphite {

class PrecompileBase : public SkRefCnt {
public:
    enum class Type {
        kBlender,
        kColorFilter,
        kImageFilter,
        kMaskFilter,
        kShader,
        // TODO: add others: kDrawable, kPathEffect (?!)
    };

    PrecompileBase(Type type) : fType(type) {}

    Type type() const { return fType; }

private:
    Type fType;
};

//--------------------------------------------------------------------------------------------------
class PrecompileShader : public PrecompileBase {
public:
    PrecompileShader() : PrecompileBase(Type::kShader) {}
};

class PrecompileMaskFilter : public PrecompileBase {
public:
    PrecompileMaskFilter() : PrecompileBase(Type::kMaskFilter) {}
};

class PrecompileColorFilter : public PrecompileBase {
public:
    PrecompileColorFilter() : PrecompileBase(Type::kColorFilter) {}
};

class PrecompileImageFilter : public PrecompileBase {
public:
    PrecompileImageFilter() : PrecompileBase(Type::kImageFilter) {}
};

class PrecompileBlender : public PrecompileBase {
public:
    PrecompileBlender() : PrecompileBase(Type::kBlender) {}

    virtual std::optional<SkBlendMode> asBlendMode() const { return {}; }

    static sk_sp<PrecompileBlender> Mode(SkBlendMode blendMode);
};

//--------------------------------------------------------------------------------------------------
class PaintOptions {
public:
    void setShaders(SkSpan<const sk_sp<PrecompileShader>> shaders) {
        fShaders.assign(shaders.begin(), shaders.end());
    }

    void setMaskFilters(SkSpan<const sk_sp<PrecompileMaskFilter>> maskFilters) {
        fMaskFilters.assign(maskFilters.begin(), maskFilters.end());
    }

    void setColorFilters(SkSpan<const sk_sp<PrecompileColorFilter>> colorFilters) {
        fColorFilters.assign(colorFilters.begin(), colorFilters.end());
    }

    void setImageFilters(SkSpan<const sk_sp<PrecompileImageFilter>> imageFilters) {
        fImageFilters.assign(imageFilters.begin(), imageFilters.end());
    }

    void setBlendModes(SkSpan<SkBlendMode> blendModes) {
        fBlenders.reserve(blendModes.size());
        for (SkBlendMode bm : blendModes) {
            fBlenders.emplace_back(PrecompileBlender::Mode(bm));
        }
    }
    void setBlenders(SkSpan<const sk_sp<PrecompileBlender>> blenders) {
        fBlenders.assign(blenders.begin(), blenders.end());
    }

private:
    std::vector<sk_sp<PrecompileShader>> fShaders;
    std::vector<sk_sp<PrecompileMaskFilter>> fMaskFilters;
    std::vector<sk_sp<PrecompileColorFilter>> fColorFilters;
    std::vector<sk_sp<PrecompileImageFilter>> fImageFilters;
    std::vector<sk_sp<PrecompileBlender>> fBlenders;
};

} // namespace skgpu::graphite

#endif // SK_ENABLE_PRECOMPILE

#endif // skgpu_graphite_Precompile_DEFINED
