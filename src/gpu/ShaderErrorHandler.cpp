/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/ShaderErrorHandler.h"

#include "src/utils/SkShaderUtils.h"

namespace skgpu {

ShaderErrorHandler* DefaultShaderErrorHandler() {
    class DefaultShaderErrorHandler : public ShaderErrorHandler {
    public:
        void compileError(const char* shader, const char* errors) override {
            std::string message = SkShaderUtils::BuildShaderErrorMessage(shader, errors);
            SkShaderUtils::VisitLineByLine(message, [](int, const char* lineText) {
                SkDebugf("%s\n", lineText);
            });
            SkDEBUGFAILF("Shader compilation failed!\n\n%s", message.c_str());
        }
    };

    static DefaultShaderErrorHandler gHandler;
    return &gHandler;
}

}  // namespace SkShaderUtils
