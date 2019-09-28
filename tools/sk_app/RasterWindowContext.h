
/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef RasterWindowContext_DEFINED
#define RasterWindowContext_DEFINED

#include "tools/sk_app/WindowContext.h"

namespace sk_app {

class RasterWindowContext : public WindowContext {
public:
    RasterWindowContext(const DisplayParams& params) : WindowContext(params) {}

protected:
    bool isGpuContext() override { return false; }
};

}   // namespace sk_app

#endif
