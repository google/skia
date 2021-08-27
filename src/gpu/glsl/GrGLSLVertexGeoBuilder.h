/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLSLVertexGeoBuilder_DEFINED
#define GrGLSLVertexGeoBuilder_DEFINED

#include "src/gpu/GrGeometryProcessor.h"
#include "src/gpu/glsl/GrGLSLShaderBuilder.h"

/**
 * Base class for vertex shader builder. This is the stage that computes input geometry for the
 * rasterizer.
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
    using GrGLSLShaderBuilder::code;

protected:
    GrGLSLVertexGeoBuilder(GrGLSLProgramBuilder* program) : INHERITED(program) {}

    void emitNormalizedSkPosition(const char* devPos,
                                  GrSLType devPosType = GrSLType::kFloat2_GrSLType) {
        this->emitNormalizedSkPosition(&this->code(), devPos, devPosType);
    }

    void emitNormalizedSkPosition(SkString* out, const char* devPos,
                                  GrSLType devPosType = GrSLType::kFloat2_GrSLType);

    friend class GrGeometryProcessor::ProgramImpl;

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

#endif
