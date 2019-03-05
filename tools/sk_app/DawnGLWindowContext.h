/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "DawnWindowContext.h"

namespace sk_app {

class DawnGLWindowContext : public DawnWindowContext {
public:
    DawnGLWindowContext(const DisplayParams& params);
    ~DawnGLWindowContext() override;
    dawnSwapChainImplementation createSwapChainImplementation(int width, int height, const DisplayParams& params) override;
    GrSurfaceOrigin getRTOrigin() const override { return kBottomLeft_GrSurfaceOrigin; }
};

}   //namespace sk_app
