/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkKeyHelpers_DEFINED
#define SkKeyHelpers_DEFINED

#ifdef SK_GRAPHITE_ENABLED
#include "experimental/graphite/include/Context.h"
#endif

#include "include/core/SkBlendMode.h"
#include "include/core/SkShader.h"
#include "include/core/SkTileMode.h"

enum class SkBackend : uint8_t;
class SkPaintParamsKey;

// The KeyHelpers can be used to manually construct an SkPaintParamsKey

namespace DepthStencilOnlyBlock {

    void AddToKey(SkBackend, SkPaintParamsKey*);
#ifdef SK_DEBUG
    void Dump(const SkPaintParamsKey&, int headerOffset);
#endif

} // namespace DepthStencilOnlyBlock

namespace SolidColorShaderBlock {

    void AddToKey(SkBackend, SkPaintParamsKey*);
#ifdef SK_DEBUG
    void Dump(const SkPaintParamsKey&, int headerOffset);
#endif

} // namespace SolidColorShaderBlock

// TODO: move this functionality to the SkLinearGradient, SkRadialGradient, etc classes
namespace GradientShaderBlocks {

    void AddToKey(SkBackend, SkPaintParamsKey*, SkShader::GradientType, SkTileMode);
#ifdef SK_DEBUG
    void Dump(const SkPaintParamsKey&, int headerOffset);
#endif

} // namespace GradientShaderBlocks

namespace BlendModeBlock {

    void AddToKey(SkBackend, SkPaintParamsKey*, SkBlendMode);
#ifdef SK_DEBUG
    void Dump(const SkPaintParamsKey&, int headerOffset);
#endif

} // namespace BlendModeBlock

#ifdef SK_GRAPHITE_ENABLED
// Bridge between the combinations system and the SkPaintParamsKey
SkPaintParamsKey CreateKey(SkBackend, skgpu::ShaderCombo::ShaderType, SkTileMode, SkBlendMode);
#endif

#endif // SkKeyHelpers_DEFINED
