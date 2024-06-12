/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_precompile_PrecompileBlender_DEFINED
#define skgpu_graphite_precompile_PrecompileBlender_DEFINED

#include "include/core/SkBlendMode.h"
#include "include/gpu/graphite/precompile/PrecompileBase.h"

#include <optional>

namespace skgpu::graphite {

class PrecompileBlenderPriv;

/** \class PrecompileBlender
    This class corresponds to the SkBlender class in the main API.
*/
class SK_API PrecompileBlender : public PrecompileBase {
public:
    // Provides access to functions that aren't part of the public API.
    PrecompileBlenderPriv priv();
    const PrecompileBlenderPriv priv() const;  // NOLINT(readability-const-return-type)

protected:
    friend class PrecompileBlenderPriv;

    virtual std::optional<SkBlendMode> asBlendMode() const { return {}; }

    PrecompileBlender() : PrecompileBase(Type::kBlender) {}
    ~PrecompileBlender() override;
};

//--------------------------------------------------------------------------------------------------
// This is the Precompile correlate to the SkBlenders namespace in the main API
namespace PrecompileBlenders {

    // This factory creates a Precompilation object that represents the Arithmetic SkBlender from
    // the main API. Note that the actual Arithmetic parameters (i.e., k1, k2, k3 and k4) are
    // abstracted away since they do not impact the generated shader.
    SK_API sk_sp<PrecompileBlender> Arithmetic();

    // This is the Precompilation correlate to the SkBlender::Mode factory in the main API. It
    // creates an object that represents SkBlendMode-based blending.
    SK_API sk_sp<PrecompileBlender> Mode(SkBlendMode);

} // namespace PrecompileBlenders


} // namespace skgpu::graphite

#endif // skgpu_graphite_precompile_PrecompileBlender_DEFINED
