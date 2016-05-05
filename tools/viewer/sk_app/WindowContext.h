/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef WindowContext_DEFINED
#define WindowContext_DEFINED

#include "GrTypes.h"

class SkSurface;

namespace sk_app {

// TODO: fill this out with an interface
class WindowContext {
public:
    virtual ~WindowContext() {}

    virtual SkSurface* getBackbufferSurface() = 0;

    virtual void swapBuffers() = 0;

    virtual bool makeCurrent() = 0;

    virtual bool isValid() = 0;

    virtual void resize(uint32_t w, uint32_t h) = 0;

    virtual GrBackendContext getBackendContext() = 0;
};

}   // namespace sk_app

#endif
