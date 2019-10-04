/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLSLVertexGeoBuilder_DEFINED
#define GrGLSLVertexGeoBuilder_DEFINED

#include "src/gpu/glsl/GrGLSLShaderBuilder.h"

/**
 * Base class for vertex and geometry shader builders. This is the stage that computes input
 * geometry for the rasterizer.
 */
class GrGLSLVertexGeoBuilder : public GrGLSLShaderBuilder {
protected:
    GrGLSLVertexGeoBuilder(GrGLSLProgramBuilder* program) : INHERITED(program) {}

    void emitNormalizedSkPosition(const char* devPos, const char* rtAdjustName,
                                  GrSLType devPosType = GrSLType::kFloat2_GrSLType) {
        this->emitNormalizedSkPosition(&this->code(), devPos, rtAdjustName, devPosType);
    }

    void emitNormalizedSkPosition(SkString* out, const char* devPos, const char* rtAdjustName,
                                  GrSLType devPosType = GrSLType::kFloat2_GrSLType);

    friend class GrGLSLGeometryProcessor;

    typedef GrGLSLShaderBuilder INHERITED;
};


class GrGLSLVertexBuilder : public GrGLSLVertexGeoBuilder {
public:
    GrGLSLVertexBuilder(GrGLSLProgramBuilder* program) : INHERITED(program) {}

private:
    void onFinalize() override;

    friend class GrGLProgramBuilder;

    typedef GrGLSLVertexGeoBuilder INHERITED;
};


class GrGLSLGeometryBuilder : public GrGLSLVertexGeoBuilder {
public:
    GrGLSLGeometryBuilder(GrGLSLProgramBuilder* program) : INHERITED(program) {}

    enum class InputType {
        kPoints,
        kLines,
        kTriangles,
    };

    enum class OutputType {
        kPoints,
        kLineStrip,
        kTriangleStrip
    };

    void configure(InputType, OutputType, int maxVertices, int numInvocations = 1);
    bool isConfigured() const { return fNumInvocations; }

    void emitVertex(const char* devPos, const char* rtAdjustName,
                    GrSLType devPosType = GrSLType::kFloat2_GrSLType) {
        this->emitVertex(&this->code(), devPos, rtAdjustName, devPosType);
    }
    void emitVertex(SkString* out, const char* devPos, const char* rtAdjustName,
                    GrSLType devPosType = GrSLType::kFloat2_GrSLType);

    void endPrimitive();

private:
    void onFinalize() override;

    int fNumInvocations = 0;

    friend class GrGLProgramBuilder;

    typedef GrGLSLVertexGeoBuilder INHERITED;
};

#endif
