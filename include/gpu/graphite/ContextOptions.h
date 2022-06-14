/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_ContextOptions_DEFINED
#define skgpu_graphite_ContextOptions_DEFINED

namespace skgpu { class ShaderErrorHandler; }

namespace skgpu::graphite {

struct SK_API ContextOptions {
    ContextOptions() {}

    /**
     * If present, use this object to report shader compilation failures. If not, report failures
     * via SkDebugf and assert.
     */
    skgpu::ShaderErrorHandler* fShaderErrorHandler = nullptr;
};

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_ContextOptions
