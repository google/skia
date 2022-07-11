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
#ifdef SK_ENABLE_LEGACY_SHADERCONTEXT
    Context* onMakeContext(const ContextRec&, SkArenaAlloc*) const override {
        return nullptr;
    }
#endif

    void flatten(SkWriteBuffer& buffer) const override {
        // Do nothing.
        // We just don't want to fall through to SkShader::flatten(),
        // which will write data we don't care to serialize or decode.
    }

    bool onAppendStages(const SkStageRec&) const override {
        return false;
    }

    skvm::Color onProgram(skvm::Builder*, skvm::Coord, skvm::Coord, skvm::Color,
                          const SkMatrixProvider&, const SkMatrix*, const SkColorInfo&,
                          skvm::Uniforms*, SkArenaAlloc*) const override;

private:
    friend void ::SkRegisterEmptyShaderFlattenable();
    SK_FLATTENABLE_HOOKS(SkEmptyShader)

    using INHERITED = SkShaderBase;
};

skvm::Color SkEmptyShader::onProgram(skvm::Builder*, skvm::Coord, skvm::Coord, skvm::Color,
                                     const SkMatrixProvider&, const SkMatrix*, const SkColorInfo&,
                                     skvm::Uniforms*, SkArenaAlloc*) const {
    return {};  // signal failure
}

sk_sp<SkFlattenable> SkEmptyShader::CreateProc(SkReadBuffer&) {
    return SkShaders::Empty();
}

sk_sp<SkShader> SkShaders::Empty() { return sk_make_sp<SkEmptyShader>(); }

void SkRegisterEmptyShaderFlattenable() {
    SK_REGISTER_FLATTENABLE(SkEmptyShader);
}
