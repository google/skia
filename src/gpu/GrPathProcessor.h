/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrPathProcessor_DEFINED
#define GrPathProcessor_DEFINED

#include "GrPrimitiveProcessor.h"

struct PathBatchTracker {
    GrGPInput fInputColorType;
    GrGPInput fInputCoverageType;
    GrColor fColor;
    bool fUsesLocalCoords;
};

/*
 * The path equivalent of the GP.  For now this just manages color. In the long term we plan on
 * extending this class to handle all nvpr uniform / varying / program work.
 */
class GrPathProcessor : public GrPrimitiveProcessor {
public:
    static GrPathProcessor* Create(GrColor color,
                                   const GrPipelineOptimizations& opts,
                                   const SkMatrix& viewMatrix = SkMatrix::I(),
                                   const SkMatrix& localMatrix = SkMatrix::I()) {
        return new GrPathProcessor(color, opts, viewMatrix, localMatrix);
    }

    void initBatchTracker(GrBatchTracker*, const GrPipelineOptimizations&) const override;

    bool isEqual(const GrBatchTracker& mine,
                 const GrPrimitiveProcessor& that,
                 const GrBatchTracker& theirs) const;

    const char* name() const override { return "PathProcessor"; }

    GrColor color() const { return fColor; }
    const SkMatrix& viewMatrix() const { return fViewMatrix; }
    const SkMatrix& localMatrix() const { return fLocalMatrix; }

    void getInvariantOutputColor(GrInitInvariantOutput* out) const override;
    void getInvariantOutputCoverage(GrInitInvariantOutput* out) const override;

    bool willUseGeoShader() const override { return false; }

    virtual void getGLProcessorKey(const GrBatchTracker& bt,
                                   const GrGLSLCaps& caps,
                                   GrProcessorKeyBuilder* b) const override;

    virtual GrGLPrimitiveProcessor* createGLInstance(const GrBatchTracker& bt,
                                                     const GrGLSLCaps& caps) const override;

    bool hasTransformedLocalCoords() const override { return false; }

    const GrPipelineOptimizations& opts() const { return fOpts; }

private:
    GrPathProcessor(GrColor color, const GrPipelineOptimizations& opts,
                    const SkMatrix& viewMatrix, const SkMatrix& localMatrix);

    bool hasExplicitLocalCoords() const override { return false; }

    GrColor fColor;
    const SkMatrix fViewMatrix;
    const SkMatrix fLocalMatrix;
    GrPipelineOptimizations fOpts;

    typedef GrPrimitiveProcessor INHERITED;
};

#endif
