
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrAAHairLinePathRenderer_DEFINED
#define GrAAHairLinePathRenderer_DEFINED

#include "GrPathRenderer.h"

class GrAAHairLinePathRenderer : public GrPathRenderer {
public:
    static GrPathRenderer* Create()  { return SkNEW(GrAAHairLinePathRenderer); }

    bool canDrawPath(const GrDrawTarget*,
                     const GrPipelineBuilder*,
                     const SkMatrix& viewMatrix,
                     const SkPath&,
                     const GrStrokeInfo&,
                     bool antiAlias) const override;

    typedef SkTArray<SkPoint, true> PtArray;
    typedef SkTArray<int, true> IntArray;
    typedef SkTArray<float, true> FloatArray;

protected:
    bool onDrawPath(GrDrawTarget*,
                    GrPipelineBuilder*,
                    GrColor,
                    const SkMatrix& viewMatrix,
                    const SkPath&,
                    const GrStrokeInfo&,
                    bool antiAlias) override;

private:
    GrAAHairLinePathRenderer() {}

    typedef GrPathRenderer INHERITED;
};


#endif
