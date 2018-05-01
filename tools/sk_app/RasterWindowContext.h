
/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef RasterWindowContext_DEFINED
#define RasterWindowContext_DEFINED

#include "WindowContext.h"

namespace sk_app {

class RasterWindowContext : public WindowContext {
public:
    RasterWindowContext(const DisplayParams& params) : WindowContext(params) {}

    // Explicitly convert nullptr to GrBackendContext is needed for compiling
    GrBackendContext getBackendContext() override { return (GrBackendContext) nullptr; }

protected:
    bool isGpuContext() override { return false; }
};

}   // namespace sk_app

#endif
