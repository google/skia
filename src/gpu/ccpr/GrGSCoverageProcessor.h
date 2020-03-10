/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGSCoverageProcessor_DEFINED
#define GrGSCoverageProcessor_DEFINED

#include "src/gpu/ccpr/GrCCCoverageProcessor.h"

/**
 * This class implements GrCCCoverageProcessor with analytic coverage using geometry shaders.
 */
class GrGSCoverageProcessor : public GrCCCoverageProcessor {
public:
    GrGSCoverageProcessor() : GrCCCoverageProcessor(kGrGSCoverageProcessor_ClassID) {
        this->setWillUseGeoShader();
    }

private:
    void getGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder* b) const override {
        SkDEBUGCODE(this->getDebugBloatKey(b));
        b->add32(((int)fPrimitiveType << 16) | (int)fSubpass);
    }

    GrPrimitiveType primType() const final { return GrPrimitiveType::kLines; }
    int numSubpasses() const override { return 2; }
    void reset(PrimitiveType, int subpassIdx, GrResourceProvider*) override;
    void bindBuffers(GrOpsRenderPass*, const GrBuffer* instanceBuffer) const override;
    void drawInstances(GrOpsRenderPass*, int instanceCount, int baseInstance) const override;

    GrGLSLPrimitiveProcessor* onCreateGLSLInstance(std::unique_ptr<Shader>) const override;

    // The geometry shader impl draws primitives in two subpasses. The first pass fills the interior
    // and does edge AA. The second pass does touch up on corner pixels.
    enum class Subpass : bool {
        kHulls,
        kCorners
    };

    Attribute fInputXOrYValues;
    mutable Subpass fSubpass = Subpass::kHulls;

    class Impl;
    class TriangleHullImpl;
    class CurveHullImpl;
    class CornerImpl;

    typedef GrCCCoverageProcessor INHERITED;
};

#endif
