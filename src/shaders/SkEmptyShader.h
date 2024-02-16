/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkEmptyShader_DEFINED
#define SkEmptyShader_DEFINED

#include "src/shaders/SkShaderBase.h"

#include "include/core/SkFlattenable.h"

class SkReadBuffer;
class SkWriteBuffer;
struct SkStageRec;

/**
 *  \class SkEmptyShader
 *  A Shader that always draws nothing. Its createContext always returns nullptr.
 */
class SkEmptyShader : public SkShaderBase {
public:
    SkEmptyShader() {}

protected:
    void flatten(SkWriteBuffer& buffer) const override {
        // Do nothing.
        // We just don't want to fall through to SkShader::flatten(),
        // which will write data we don't care to serialize or decode.
    }

    bool appendStages(const SkStageRec&, const SkShaders::MatrixRec&) const override {
        return false;
    }

    ShaderType type() const override { return ShaderType::kEmpty; }

private:
    friend void ::SkRegisterEmptyShaderFlattenable();
    SK_FLATTENABLE_HOOKS(SkEmptyShader)
};

#endif  // SkEmptyShader_DEFINED
