
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef GrTesselatedPathRenderer_DEFINED
#define GrTesselatedPathRenderer_DEFINED

#include "GrPathRenderer.h"

class GrTesselatedPathRenderer : public GrPathRenderer {
public:
    GrTesselatedPathRenderer();

    virtual void drawPath(GrDrawTarget::StageBitfield stages);
    virtual bool canDrawPath(const GrDrawTarget::Caps& targetCaps,
                             const GrPath& path,
                             GrPathFill fill,
                             bool antiAlias) const SK_OVERRIDE;
    virtual void drawPathToStencil() SK_OVERRIDE;
};

#endif
