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

    SK_TO_STRING_OVERRIDE()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkEmptyShader)

protected:
    Context* onMakeContext(const ContextRec&, SkArenaAlloc*) const override {
        return nullptr;
    }

    void flatten(SkWriteBuffer& buffer) const override {
        // Do nothing.
        // We just don't want to fall through to SkShader::flatten(),
        // which will write data we don't care to serialize or decode.
    }

    bool onAppendStages(SkRasterPipeline*, SkColorSpace*, SkArenaAlloc*, const SkMatrix&,
                        const SkPaint&, const SkMatrix*) const override {
        return false;
    }

private:
    typedef SkShaderBase INHERITED;
};

#endif
