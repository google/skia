/*
 * Copyright 2019 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrSampleMaskProcessor_DEFINED
#define GrSampleMaskProcessor_DEFINED

#include "src/gpu/ccpr/GrCCCoverageProcessor.h"

/**
 * This class implements GrCCCoverageProcessor with MSAA using the sample mask.
 */
class GrSampleMaskProcessor : public GrCCCoverageProcessor {
public:
    GrSampleMaskProcessor() : GrCCCoverageProcessor(kGrSampleMaskProcessor_ClassID) {}

private:
    GrPrimitiveType primType() const final;
    int numSubpasses() const override { return 1; }
    void reset(PrimitiveType, int subpassIdx, GrResourceProvider*) override;
    void bindBuffers(GrOpsRenderPass*, const GrBuffer* instanceBuffer) const override;
    void drawInstances(GrOpsRenderPass*, int instanceCount, int baseInstance) const override;

    GrGLSLPrimitiveProcessor* onCreateGLSLInstance(std::unique_ptr<Shader>) const override;

    SkSTArray<2, Attribute> fInputAttribs;

    class Impl;
};

#endif
