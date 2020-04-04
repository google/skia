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
    void reset(PrimitiveType, GrResourceProvider*) override;

    void appendMesh(sk_sp<const GrGpuBuffer> instanceBuffer, int instanceCount, int baseInstance,
                    SkTArray<GrMesh>* out) const override;

    GrPrimitiveType primType() const final { return fTriangleType; }

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
