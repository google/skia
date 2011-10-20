
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkMesaGLContext_DEFINED
#define SkMesaGLContext_DEFINED

#include "SkGLContext.h"

#if SK_MESA

class SkMesaGLContext : public SkGLContext {
public:
    SkMesaGLContext();

    virtual ~SkMesaGLContext();

    virtual void makeCurrent() const SK_OVERRIDE;

protected:
    virtual const GrGLInterface* createGLContext() SK_OVERRIDE;
    void destroyGLContext() SK_OVERRIDE;

private:
    typedef intptr_t Context;
    Context fContext;
    GrGLubyte *fImage;
};

#endif

#endif