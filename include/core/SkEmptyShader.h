
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */



#ifndef SkEmptyShader_DEFINED
#define SkEmptyShader_DEFINED

#include "SkShader.h"

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

    virtual bool validContext(const SkBitmap&, const SkPaint&,
                              const SkMatrix&, SkMatrix* totalInverse = NULL) const SK_OVERRIDE {
        return false;
    }

    virtual SkShader::Context* createContext(const SkBitmap&, const SkPaint&,
                                             const SkMatrix&, void*) const SK_OVERRIDE {
        // validContext returns false.
        return NULL;
    }

    SK_TO_STRING_OVERRIDE()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkEmptyShader)

protected:
    SkEmptyShader(SkReadBuffer& buffer) : INHERITED(buffer) {}

private:
    typedef SkShader INHERITED;
};

#endif
