/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/shaders/SkShaderBase.h"

#include "include/core/SkFlattenable.h"
#include "src/core/SkVM.h"

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

    bool appendStages(const SkStageRec&, const MatrixRec&) const override { return false; }

    skvm::Color program(skvm::Builder*,
                        skvm::Coord,
                        skvm::Coord,
                        skvm::Color,
                        const MatrixRec&,
                        const SkColorInfo&,
                        skvm::Uniforms*,
                        SkArenaAlloc*) const override;

private:
    friend void ::SkRegisterEmptyShaderFlattenable();
    SK_FLATTENABLE_HOOKS(SkEmptyShader)

    using INHERITED = SkShaderBase;
};

skvm::Color SkEmptyShader::program(skvm::Builder*,
                                   skvm::Coord,
                                   skvm::Coord,
                                   skvm::Color,
                                   const MatrixRec&,
                                   const SkColorInfo&,
                                   skvm::Uniforms*,
                                   SkArenaAlloc*) const {
    return {};  // signal failure
}

sk_sp<SkFlattenable> SkEmptyShader::CreateProc(SkReadBuffer&) {
    return SkShaders::Empty();
}

sk_sp<SkShader> SkShaders::Empty() { return sk_make_sp<SkEmptyShader>(); }

void SkRegisterEmptyShaderFlattenable() {
    SK_REGISTER_FLATTENABLE(SkEmptyShader);
}
