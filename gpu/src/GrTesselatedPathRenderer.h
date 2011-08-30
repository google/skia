
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
    virtual bool canDrawPath(const GrDrawTarget* target,
                             const GrPath& path,
                             GrPathFill fill) const;

    virtual bool requiresStencilPass(const GrDrawTarget* target,
                                     const GrPath& path,
                                     GrPathFill fill) const { return false; }
    virtual void drawPathToStencil();
    virtual bool supportsAA(const GrDrawTarget* target,
                            const GrPath& path,
                            GrPathFill fill);
};

#endif
