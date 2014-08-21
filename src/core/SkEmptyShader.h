/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkEmptyShader_DEFINED
#define SkEmptyShader_DEFINED

#include "SkShader.h"

// TODO: move this to private, as there is a public factory on SkShader

/**
 *  \class SkEmptyShader
 *  A Shader that always draws nothing. Its createContext always returns NULL.
 */
class SK_API SkEmptyShader : public SkShader {
public:
    SkEmptyShader() {}

    virtual size_t contextSize() const SK_OVERRIDE {
        // Even though createContext returns NULL we have to return a value of at least
        // sizeof(SkShader::Context) to satisfy SkSmallAllocator.
        return sizeof(SkShader::Context);
    }

    SK_TO_STRING_OVERRIDE()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkEmptyShader)

protected:
#ifdef SK_SUPPORT_LEGACY_DEEPFLATTENING
    SkEmptyShader(SkReadBuffer& buffer) : INHERITED(buffer) {}
#endif

    virtual SkShader::Context* onCreateContext(const ContextRec&, void*) const SK_OVERRIDE {
        return NULL;
    }

private:
    typedef SkShader INHERITED;
};

#endif
