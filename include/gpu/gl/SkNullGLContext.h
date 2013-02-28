
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkNullGLContext_DEFINED
#define SkNullGLContext_DEFINED

#include "SkGLContextHelper.h"

class SkNullGLContext : public SkGLContextHelper {

public:
    SkNullGLContext() {};

    virtual void makeCurrent() const SK_OVERRIDE {};

protected:
    virtual const GrGLInterface* createGLContext() SK_OVERRIDE;

    virtual void destroyGLContext() SK_OVERRIDE {};
};

#endif
