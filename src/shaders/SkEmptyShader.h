/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkEmptyShader_DEFINED
#define SkEmptyShader_DEFINED

#include "SkShaderBase.h"

// TODO: move this to private, as there is a public factory on SkShader

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

private:
    SK_FLATTENABLE_HOOKS(SkEmptyShader)

    typedef SkShaderBase INHERITED;
};

#endif
