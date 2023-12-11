/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_ShaderErrorHandler_DEFINED
#define skgpu_ShaderErrorHandler_DEFINED

#include "include/core/SkTypes.h"

namespace skgpu {
/**
 * Abstract class to report errors when compiling shaders.
 */
class SK_API ShaderErrorHandler {
public:
    virtual ~ShaderErrorHandler() = default;

    /**
     * compileError(shader, errors) is kept for backward compatibility with older clients.
     */
    virtual void compileError([[maybe_unused]] const char* shader,
                              [[maybe_unused]] const char* errors) {}
    virtual void compileError(const char* shader,
                              const char* errors,
                              [[maybe_unused]] bool shaderWasCached) {
        // Default implementation. Ignore shaderWasCached.
        this->compileError(shader, errors);
    }

protected:
    ShaderErrorHandler() = default;
    ShaderErrorHandler(const ShaderErrorHandler&) = delete;
    ShaderErrorHandler& operator=(const ShaderErrorHandler&) = delete;
};

/**
 * Used when no error handler is set. Will report failures via SkDebugf and asserts.
 */
ShaderErrorHandler* DefaultShaderErrorHandler();

}  // namespace skgpu

#endif // skgpu_ShaderErrorHandler_DEFINED
