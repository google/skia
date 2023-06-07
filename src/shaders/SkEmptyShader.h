/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/shaders/SkShaderBase.h"

#include "include/core/SkFlattenable.h"

#if defined(SK_ENABLE_SKVM)
#include "src/core/SkVM.h"
#endif

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

#if defined(SK_ENABLE_SKVM)
    skvm::Color program(skvm::Builder*,
                        skvm::Coord,
                        skvm::Coord,
                        skvm::Color,
                        const SkShaders::MatrixRec&,
                        const SkColorInfo&,
                        skvm::Uniforms*,
                        SkArenaAlloc*) const override;
#endif

private:
    friend void ::SkRegisterEmptyShaderFlattenable();
    SK_FLATTENABLE_HOOKS(SkEmptyShader)
};
