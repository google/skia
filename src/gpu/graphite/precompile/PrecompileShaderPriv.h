/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_PrecompileShaderPriv_DEFINED
#define skgpu_graphite_PrecompileShaderPriv_DEFINED

#include "include/gpu/graphite/precompile/PrecompileShader.h"

namespace skgpu::graphite {

/** Class that exposes methods in PrecompileShader that are only intended for use internal to Skia.
    This class is purely a privileged window into PrecompileShader. It should never have additional
    data members or virtual methods. */
class PrecompileShaderPriv {
public:
    bool isConstant(int desiredCombination) const {
        return fPrecompileShader->isConstant(desiredCombination);
    }

    bool isALocalMatrixShader() const {
        return fPrecompileShader->isALocalMatrixShader();
    }

    // The remaining methods make this a viable standin for PrecompileBasePriv
    int numChildCombinations() const { return fPrecompileShader->numChildCombinations(); }

    int numCombinations() const { return fPrecompileShader->numCombinations(); }

    void addToKey(const KeyContext& keyContext,
                  PaintParamsKeyBuilder* builder,
                  PipelineDataGatherer* gatherer,
                  int desiredCombination) const {
        fPrecompileShader->addToKey(keyContext, builder, gatherer, desiredCombination);
    }

private:
    friend class PrecompileShader; // to construct/copy this type.

    explicit PrecompileShaderPriv(PrecompileShader* precompileShader)
            : fPrecompileShader(precompileShader) {}

    PrecompileShaderPriv& operator=(const PrecompileShaderPriv&) = delete;

    // No taking addresses of this type.
    const PrecompileShaderPriv* operator&() const;
    PrecompileShaderPriv *operator&();

    PrecompileShader* fPrecompileShader;
};

inline PrecompileShaderPriv PrecompileShader::priv() { return PrecompileShaderPriv(this); }

// NOLINTNEXTLINE(readability-const-return-type)
inline const PrecompileShaderPriv PrecompileShader::priv() const {
    return PrecompileShaderPriv(const_cast<PrecompileShader *>(this));
}

} // namespace skgpu::graphite

#endif // skgpu_graphite_PrecompileShaderPriv_DEFINED
