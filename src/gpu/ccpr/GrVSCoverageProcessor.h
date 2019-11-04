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
    GrVSCoverageProcessor(const GrCaps& caps)
        : GrCCCoverageProcessor(kGrVSCoverageProcessor_ClassID) {
        fTriangleType1 = caps.usePrimitiveRestart() ? GrPrimitiveType::kTriangleStrip
                                                    : GrPrimitiveType::kTriangles;
    }

private:
    void reset1(PrimitiveType, GrResourceProvider*) override;

    // Always either kTriangleStrip or kTriangles - consistent
    void appendMesh(sk_sp<const GrGpuBuffer> instanceBuffer, int instanceCount, int baseInstance,
                    SkTArray<GrMesh>* out, GrPrimitiveType*) const override;

    GrPrimitiveType primType(const GrCaps& caps) override { return fTriangleType1; }

    GrGLSLPrimitiveProcessor* onCreateGLSLInstance(std::unique_ptr<Shader>) const override;

    Attribute fPerVertexData;
    Attribute fInputXAndYValues[2];
    sk_sp<const GrGpuBuffer> fVertexBuffer;
    sk_sp<const GrGpuBuffer> fIndexBuffer;
    int fNumIndicesPerInstance;
    GrPrimitiveType fTriangleType1;

    class Impl;
};

#endif
