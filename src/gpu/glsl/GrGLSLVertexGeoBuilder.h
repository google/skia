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
public:
    // Copies the given text verbatim to the function definitions section. Does not mangle the name.
    // 'functionDefinition' should be a fully valid SkSL function, complete with return type, name,
    // arguments, braces, and a body.
    void insertFunction(const char* functionDefinition) {
        this->functions().append(functionDefinition);
    }
    using GrGLSLShaderBuilder::functions;

protected:
    GrGLSLVertexGeoBuilder(GrGLSLProgramBuilder* program) : INHERITED(program) {}

    void emitNormalizedSkPosition(const char* devPos,
                                  GrSLType devPosType = GrSLType::kFloat2_GrSLType) {
        this->emitNormalizedSkPosition(&this->code(), devPos, devPosType);
    }

    void emitNormalizedSkPosition(SkString* out, const char* devPos,
                                  GrSLType devPosType = GrSLType::kFloat2_GrSLType);

    friend class GrGLSLGeometryProcessor;

    using INHERITED = GrGLSLShaderBuilder;
};


class GrGLSLVertexBuilder : public GrGLSLVertexGeoBuilder {
public:
    GrGLSLVertexBuilder(GrGLSLProgramBuilder* program) : INHERITED(program) {}

private:
    void onFinalize() override;

    friend class GrGLProgramBuilder;

    using INHERITED = GrGLSLVertexGeoBuilder;
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

    void emitVertex(const char* devPos, GrSLType devPosType = GrSLType::kFloat2_GrSLType) {
        this->emitVertex(&this->code(), devPos, devPosType);
    }
    void emitVertex(SkString* out, const char* devPos,
                    GrSLType devPosType = GrSLType::kFloat2_GrSLType);

    void endPrimitive();

private:
    void onFinalize() override;

    int fNumInvocations = 0;

    friend class GrGLProgramBuilder;

    using INHERITED = GrGLSLVertexGeoBuilder;
};

#endif
