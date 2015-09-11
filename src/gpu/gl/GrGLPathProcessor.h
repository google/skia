/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLPathProcessor_DEFINED
#define GrGLPathProcessor_DEFINED

#include "GrGLPrimitiveProcessor.h"

class GrPathProcessor;
class GrGLPathRendering;
class GrGLGpu;

class GrGLPathProcessor : public GrGLPrimitiveProcessor {
public:
    GrGLPathProcessor();

    static void GenKey(const GrPathProcessor&,
                       const GrGLSLCaps&,
                       GrProcessorKeyBuilder* b);

    void emitCode(EmitArgs&) override;

    void emitTransforms(GrGLGPBuilder*, const TransformsIn&, TransformsOut*);

    void setData(const GrGLProgramDataManager&, const GrPrimitiveProcessor&) override;

    void setTransformData(const GrPrimitiveProcessor&,
                          const GrGLProgramDataManager&,
                          int index,
                          const SkTArray<const GrCoordTransform*, true>& transforms) override;

private:
    UniformHandle fColorUniform;
    GrColor fColor;

    typedef GrGLPrimitiveProcessor INHERITED;
};

#endif
