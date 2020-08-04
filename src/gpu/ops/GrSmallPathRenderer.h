/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrSmallPathRenderer_DEFINED
#define GrSmallPathRenderer_DEFINED

#include "src/gpu/GrPathRenderer.h"
#include "src/gpu/geometry/GrRect.h"
#include "src/gpu/geometry/GrStyledShape.h"

#include "src/core/SkOpts.h"
#include "src/core/SkTDynamicHash.h"
#include "src/core/SkTInternalLList.h"

class GrDrawOp;
class GrRecordingContext;

class ShapeData;
class ShapeDataKey;

class GrSmallPathRenderer : public GrPathRenderer {
public:
    GrSmallPathRenderer();
    ~GrSmallPathRenderer() override;

    const char* name() const final { return "Small"; }


    using ShapeCache = SkTDynamicHash<ShapeData, ShapeDataKey>;
    typedef SkTInternalLList<ShapeData> ShapeDataList;

    static std::unique_ptr<GrDrawOp> createOp_TestingOnly(GrRecordingContext*,
                                                          GrPaint&&,
                                                          const GrStyledShape&,
                                                          const SkMatrix& viewMatrix,
                                                          ShapeCache*,
                                                          ShapeDataList*,
                                                          bool gammaCorrect,
                                                          const GrUserStencilSettings*);
    struct PathTestStruct;

private:
    class SmallPathOp;

    StencilSupport onGetStencilSupport(const GrStyledShape&) const override {
        return GrPathRenderer::kNoSupport_StencilSupport;
    }

    CanDrawPath onCanDrawPath(const CanDrawPathArgs&) const override;

    bool onDrawPath(const DrawPathArgs&) override;

    ShapeCache fShapeCache;
    ShapeDataList fShapeList;

    typedef GrPathRenderer INHERITED;
};

#endif
