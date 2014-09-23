/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLGeometryProcessor_DEFINED
#define GrGLGeometryProcessor_DEFINED

#include "GrGLProcessor.h"

/**
 * If a GL effect needs a GrGLFullShaderBuilder* object to emit vertex code, then it must inherit
 * from this class. Since paths don't have vertices, this class is only meant to be used internally
 * by skia, for special cases.
 */
class GrGLGeometryProcessor : public GrGLProcessor {
public:
    GrGLGeometryProcessor(const GrBackendProcessorFactory& factory)
        : INHERITED(factory) {}

    /**
     * This is similar to emitCode() in the base class, except it takes a full shader builder.
     * This allows the effect subclass to emit vertex code.
     */
    virtual void emitCode(GrGLFullProgramBuilder* builder,
                          const GrGeometryProcessor& geometryProcessor,
                          const GrProcessorKey& key,
                          const char* outputColor,
                          const char* inputColor,
                          const TransformedCoordsArray& coords,
                          const TextureSamplerArray& samplers) = 0;

private:
    typedef GrGLProcessor INHERITED;
};

#endif
