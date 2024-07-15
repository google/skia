/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_FactoryFunctions_DEFINED
#define skgpu_graphite_FactoryFunctions_DEFINED

#include "include/core/SkRefCnt.h"

namespace skgpu::graphite {

class PrecompileShader;

namespace PrecompileShaders {
    // ??
    SK_API sk_sp<PrecompileShader> YUVImage();

} // namespace PrecompileShaders

} // namespace skgpu::graphite

#endif // skgpu_graphite_FactoryFunctions_DEFINED
