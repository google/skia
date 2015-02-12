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
                                   const SkMatrix& viewMatrix = SkMatrix::I(),
                                   const SkMatrix& localMatrix = SkMatrix::I()) {
        return SkNEW_ARGS(GrPathProcessor, (color, viewMatrix, localMatrix));
    }

    void initBatchTracker(GrBatchTracker*, const GrPipelineInfo&) const SK_OVERRIDE;

    bool canMakeEqual(const GrBatchTracker& mine,
                      const GrPrimitiveProcessor& that,
                      const GrBatchTracker& theirs) const SK_OVERRIDE;

    const char* name() const SK_OVERRIDE { return "PathProcessor"; }

    GrColor color() const { return fColor; }

    void getInvariantOutputColor(GrInitInvariantOutput* out) const SK_OVERRIDE;
    void getInvariantOutputCoverage(GrInitInvariantOutput* out) const SK_OVERRIDE;

    bool willUseGeoShader() const SK_OVERRIDE { return false; }

    virtual void getGLProcessorKey(const GrBatchTracker& bt,
                                   const GrGLCaps& caps,
                                   GrProcessorKeyBuilder* b) const SK_OVERRIDE;

    virtual GrGLPrimitiveProcessor* createGLInstance(const GrBatchTracker& bt,
                                                     const GrGLCaps& caps) const SK_OVERRIDE;

protected:
    GrPathProcessor(GrColor color, const SkMatrix& viewMatrix, const SkMatrix& localMatrix);

private:
    bool hasExplicitLocalCoords() const SK_OVERRIDE { return false; }

    GrColor fColor;

    typedef GrPrimitiveProcessor INHERITED;
};

#endif
