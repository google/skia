/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrDashLinePathRenderer_DEFINED
#define GrDashLinePathRenderer_DEFINED

#include "GrPathRenderer.h"

class GrGpu;

class GrDashLinePathRenderer : public GrPathRenderer {
private:
    CanDrawPath onCanDrawPath(const CanDrawPathArgs&, DrawType) const override;

    StencilSupport onGetStencilSupport(const GrShape&) const override {
        return StencilSupport::kNoSupport;
    }

    bool onDrawPath(const DrawPathArgs&) override;

    sk_sp<GrGpu> fGpu;
    typedef GrPathRenderer INHERITED;
};


#endif
