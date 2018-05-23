/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrCCPathProcessor_DEFINED
#define GrCCPathProcessor_DEFINED

#include "GrCaps.h"
#include "GrGeometryProcessor.h"
#include "SkPath.h"
#include <array>

class GrOnFlushResourceProvider;
class GrOpFlushState;
class GrPipeline;

/**
 * This class draws AA paths using the coverage count masks produced by GrCCCoverageProcessor.
 *
 * Paths are drawn as bloated octagons, and coverage is derived from the coverage count mask and
 * fill rule.
 *
 * To draw paths, the caller must set up an instance buffer as detailed below, then call drawPaths()
 * providing its own instance buffer alongside the buffers found by calling FindIndexBuffer/
 * FindVertexBuffer.
 */
class GrCCPathProcessor : public GrGeometryProcessor {
public:
    enum class InstanceAttribs {
        kDevBounds,
        kDevBounds45,
        kAtlasOffset,
        kColor
    };
    static constexpr int kNumInstanceAttribs = 1 + (int)InstanceAttribs::kColor;

    struct Instance {
        SkRect fDevBounds;
        SkRect fDevBounds45; // Bounding box in "| 1  -1 | * devCoords" space.
                             //                  | 1   1 |
        std::array<int16_t, 2> fAtlasOffset;
        uint32_t fColor;

        GR_STATIC_ASSERT(SK_SCALAR_IS_FLOAT);
    };

    GR_STATIC_ASSERT(4 * 10 == sizeof(Instance));

    static sk_sp<const GrBuffer> FindVertexBuffer(GrOnFlushResourceProvider*);
    static sk_sp<const GrBuffer> FindIndexBuffer(GrOnFlushResourceProvider*);

    GrCCPathProcessor(GrResourceProvider*, sk_sp<GrTextureProxy> atlas, SkPath::FillType,
                      const SkMatrix& viewMatrixIfUsingLocalCoords = SkMatrix::I());

    const char* name() const override { return "GrCCPathProcessor"; }
    const GrSurfaceProxy* atlasProxy() const { return fAtlasAccess.proxy(); }
    const GrTexture* atlas() const { return fAtlasAccess.peekTexture(); }
    const SkMatrix& localMatrix() const { return fLocalMatrix; }
    SkPath::FillType fillType() const { return fFillType; }
    const Attribute& getInstanceAttrib(InstanceAttribs attribID) const {
        const Attribute& attrib = this->getAttrib((int)attribID);
        SkASSERT(Attribute::InputRate::kPerInstance == attrib.fInputRate);
        return attrib;
    }
    const Attribute& getEdgeNormsAttrib() const {
        SkASSERT(1 + kNumInstanceAttribs == this->numAttribs());
        const Attribute& attrib = this->getAttrib(kNumInstanceAttribs);
        SkASSERT(Attribute::InputRate::kPerVertex == attrib.fInputRate);
        return attrib;
    }

    void getGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder*) const override;
    GrGLSLPrimitiveProcessor* createGLSLInstance(const GrShaderCaps&) const override;

    void drawPaths(GrOpFlushState*, const GrPipeline&, const GrBuffer* indexBuffer,
                   const GrBuffer* vertexBuffer, GrBuffer* instanceBuffer, int baseInstance,
                   int endInstance, const SkRect& bounds) const;

private:
    const SkPath::FillType fFillType;
    const TextureSampler fAtlasAccess;
    SkMatrix fLocalMatrix;

    typedef GrGeometryProcessor INHERITED;
};

#endif
