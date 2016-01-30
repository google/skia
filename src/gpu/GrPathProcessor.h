/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrPathProcessor_DEFINED
#define GrPathProcessor_DEFINED

#include "GrPrimitiveProcessor.h"

/*
 * The path equivalent of the GP.  For now this just manages color. In the long term we plan on
 * extending this class to handle all nvpr uniform / varying / program work.
 */
class GrPathProcessor : public GrPrimitiveProcessor {
public:
    static GrPathProcessor* Create(GrColor color,
                                   const GrXPOverridesForBatch& overrides,
                                   const SkMatrix& viewMatrix = SkMatrix::I(),
                                   const SkMatrix& localMatrix = SkMatrix::I()) {
        return new GrPathProcessor(color, overrides, viewMatrix, localMatrix);
    }

    const char* name() const override { return "PathProcessor"; }

    GrColor color() const { return fColor; }
    const SkMatrix& viewMatrix() const { return fViewMatrix; }
    const SkMatrix& localMatrix() const { return fLocalMatrix; }

    bool willUseGeoShader() const override { return false; }

    virtual void getGLSLProcessorKey(const GrGLSLCaps& caps,
                                     GrProcessorKeyBuilder* b) const override;

    virtual GrGLSLPrimitiveProcessor* createGLSLInstance(const GrGLSLCaps& caps) const override;

    bool hasTransformedLocalCoords() const override { return false; }

    const GrXPOverridesForBatch& overrides() const { return fOverrides; }

    virtual bool isPathRendering() const override { return true; }

private:
    GrPathProcessor(GrColor color, const GrXPOverridesForBatch& overrides,
                    const SkMatrix& viewMatrix, const SkMatrix& localMatrix);

    bool hasExplicitLocalCoords() const override { return false; }

    GrColor fColor;
    const SkMatrix fViewMatrix;
    const SkMatrix fLocalMatrix;
    GrXPOverridesForBatch fOverrides;

    typedef GrPrimitiveProcessor INHERITED;
};

#endif
