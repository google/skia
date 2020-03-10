/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrVSCoverageProcessor_DEFINED
#define GrVSCoverageProcessor_DEFINED

#include "src/gpu/ccpr/GrCCCoverageProcessor.h"

/**
 * This class implements GrCCCoverageProcessor with analytic coverage using vertex shaders.
 */
class GrVSCoverageProcessor : public GrCCCoverageProcessor {
public:
    GrVSCoverageProcessor() : GrCCCoverageProcessor(kGrVSCoverageProcessor_ClassID) {}

private:
    GrPrimitiveType primType() const final { return fTriangleType; }
    int numSubpasses() const override { return 1; }
    void reset(PrimitiveType, int subpassIdx, GrResourceProvider*) override;
    void bindBuffers(GrOpsRenderPass*, const GrBuffer* instanceBuffer) const override;
    void drawInstances(GrOpsRenderPass*, int instanceCount, int baseInstance) const override;

    GrGLSLPrimitiveProcessor* onCreateGLSLInstance(std::unique_ptr<Shader>) const override;

    Attribute fPerVertexData;
    Attribute fInputXAndYValues[2];
    sk_sp<const GrGpuBuffer> fVertexBuffer;
    sk_sp<const GrGpuBuffer> fIndexBuffer;
    int fNumIndicesPerInstance;
    GrPrimitiveType fTriangleType = GrPrimitiveType::kPoints;

    class Impl;
};

#endif
