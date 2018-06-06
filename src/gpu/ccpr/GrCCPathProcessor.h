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
        SkRect fDevBounds; // "right < left" indicates even-odd fill type.
        SkRect fDevBounds45; // Bounding box in "| 1  -1 | * devCoords" space.
                             //                  | 1   1 |
        std::array<int16_t, 2> fAtlasOffset;
        uint32_t fColor;

        void set(SkPath::FillType, const SkRect& devBounds, const SkRect& devBounds45,
                 int16_t atlasOffsetX, int16_t atlasOffsetY, uint32_t color);
    };

    GR_STATIC_ASSERT(4 * 10 == sizeof(Instance));

    static sk_sp<const GrBuffer> FindVertexBuffer(GrOnFlushResourceProvider*);
    static sk_sp<const GrBuffer> FindIndexBuffer(GrOnFlushResourceProvider*);

    GrCCPathProcessor(GrResourceProvider*, sk_sp<GrTextureProxy> atlas,
                      const SkMatrix& viewMatrixIfUsingLocalCoords = SkMatrix::I());

    const char* name() const override { return "GrCCPathProcessor"; }
    const GrSurfaceProxy* atlasProxy() const { return fAtlasAccess.proxy(); }
    const GrTexture* atlas() const { return fAtlasAccess.peekTexture(); }
    const SkMatrix& localMatrix() const { return fLocalMatrix; }
    const Attribute& getInstanceAttrib(InstanceAttribs attribID) const {
        int idx = static_cast<int>(attribID);
        SkASSERT(idx >= 0 && idx < static_cast<int>(SK_ARRAY_COUNT(kInstancedAttribs)));
        const Attribute& attrib = this->onInstanceAttribute(idx);
        return attrib;
    }
    const Attribute& getEdgeNormsAttrib() const {
        return kEdgeNormsAttrib;
    }

    void getGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder*) const override {}
    GrGLSLPrimitiveProcessor* createGLSLInstance(const GrShaderCaps&) const override;

    void drawPaths(GrOpFlushState*, const GrPipeline&, const GrBuffer* indexBuffer,
                   const GrBuffer* vertexBuffer, GrBuffer* instanceBuffer, int baseInstance,
                   int endInstance, const SkRect& bounds) const;

private:
    const Attribute& onVertexAttribute(int i) const override { return kEdgeNormsAttrib; }
    const Attribute& onInstanceAttribute(int i) const override { return kInstancedAttribs[i]; }

    const TextureSampler fAtlasAccess;
    SkMatrix fLocalMatrix;
    static constexpr Attribute kInstancedAttribs[kNumInstanceAttribs] = {
            {"devbounds", kFloat4_GrVertexAttribType, offsetof(Instance, fDevBounds)},
            {"devbounds45", kFloat4_GrVertexAttribType, offsetof(Instance, fDevBounds45)},
            {"atlas_offset", kShort2_GrVertexAttribType, offsetof(Instance, fAtlasOffset)},
            {"color", kUByte4_norm_GrVertexAttribType, offsetof(Instance, fColor)},
    };
    static constexpr Attribute kEdgeNormsAttrib = {"edge_norms", kFloat4_GrVertexAttribType};

    typedef GrGeometryProcessor INHERITED;
};

inline void GrCCPathProcessor::Instance::set(SkPath::FillType fillType, const SkRect& devBounds,
                                             const SkRect& devBounds45, int16_t atlasOffsetX,
                                             int16_t atlasOffsetY, uint32_t color) {
    if (SkPath::kEvenOdd_FillType == fillType) {
        // "right < left" indicates even-odd fill type.
        fDevBounds.setLTRB(devBounds.fRight, devBounds.fTop, devBounds.fLeft, devBounds.fBottom);
    } else {
        SkASSERT(SkPath::kWinding_FillType == fillType);
        fDevBounds = devBounds;
    }
    fDevBounds45 = devBounds45;
    fAtlasOffset = {{atlasOffsetX, atlasOffsetY}};
    fColor = color;
}

#endif
