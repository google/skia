/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef gr_instanced_InstanceProcessor_DEFINED
#define gr_instanced_InstanceProcessor_DEFINED

#include "GrCaps.h"
#include "GrGeometryProcessor.h"
#include "instanced/InstancedRenderingTypes.h"

namespace gr_instanced {

/**
 * This class provides a GP implementation that uses instanced rendering. Is sends geometry in as
 * basic, pre-baked canonical shapes, and uses instanced vertex attribs to control how these shapes
 * are transformed and drawn. MSAA is accomplished with the sample mask rather than finely
 * tesselated geometry.
 */
class InstanceProcessor : public GrGeometryProcessor {
public:
    InstanceProcessor(OpInfo, GrBuffer* paramsBuffer);

    const char* name() const override { return "Instance Processor"; }
    OpInfo opInfo() const { return fOpInfo; }

    void getGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder* b) const override {
        b->add32(fOpInfo.fData);
    }
    GrGLSLPrimitiveProcessor* createGLSLInstance(const GrShaderCaps&) const override;

    /**
     * Returns a buffer of ShapeVertex that defines the canonical instanced geometry.
     */
    static const GrBuffer* SK_WARN_UNUSED_RESULT FindOrCreateVertexBuffer(GrGpu*);

    /**
     * Returns a buffer of 8-bit indices for the canonical instanced geometry. The client can call
     * GetIndexRangeForXXX to know which indices to use for a specific shape.
     */
    static const GrBuffer* SK_WARN_UNUSED_RESULT FindOrCreateIndex8Buffer(GrGpu*);

    static IndexRange GetIndexRangeForRect(GrAAType);
    static IndexRange GetIndexRangeForOval(GrAAType, const SkRect& devBounds);
    static IndexRange GetIndexRangeForRRect(GrAAType);

    static const char* GetNameOfIndexRange(IndexRange);

private:
    /**
     * Called by the platform-specific instanced rendering implementation to determine the level of
     * support this class can offer on the given GLSL platform.
     */
    static GrCaps::InstancedSupport CheckSupport(const GrShaderCaps&, const GrCaps&);

    OpInfo fOpInfo;
    BufferAccess fParamsAccess;

    friend class GLInstancedRendering; // For CheckSupport.

    typedef GrGeometryProcessor INHERITED;
};

}

#endif
