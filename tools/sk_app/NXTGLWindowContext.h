/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "NXTWindowContext.h"

namespace sk_app {

class NXTGLWindowContext : public NXTWindowContext {
public:
    NXTGLWindowContext(const DisplayParams& params);
    ~NXTGLWindowContext() override;
    dawnSwapChainImplementation createSwapChainImplementation(int width, int height, const DisplayParams& params) override;
};

}   //namespace sk_app
