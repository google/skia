/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrCCPathProcessor_DEFINED
#define GrCCPathProcessor_DEFINED

#include <array>
#include "GrCaps.h"
#include "GrGeometryProcessor.h"
#include "GrPipeline.h"
#include "SkPath.h"

class GrCCPathCacheEntry;
class GrCCPerFlushResources;
class GrOnFlushResourceProvider;
class GrOpFlushState;

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
        kDevToAtlasOffset,
        kColor
    };
    static constexpr int kNumInstanceAttribs = 1 + (int)InstanceAttribs::kColor;

    // Helper to offset the 45-degree bounding box returned by GrCCPathParser::parsePath().
    static SkRect MakeOffset45(const SkRect& devBounds45, float dx, float dy) {
        // devBounds45 is in "| 1  -1 | * devCoords" space.
        //                    | 1   1 |
        return devBounds45.makeOffset(dx - dy, dx + dy);
    }

    enum class DoEvenOddFill : bool {
        kNo = false,
        kYes = true
    };

    struct Instance {
        SkRect fDevBounds;  // "right < left" indicates even-odd fill type.
        SkRect fDevBounds45;  // Bounding box in "| 1  -1 | * devCoords" space.
                              //                  | 1   1 |
        SkIVector fDevToAtlasOffset;  // Translation from device space to location in atlas.
        uint64_t fColor;  // Color always stored as 4 x fp16

        void set(const SkRect& devBounds, const SkRect& devBounds45,
                 const SkIVector& devToAtlasOffset, uint64_t, DoEvenOddFill = DoEvenOddFill::kNo);
        void set(const GrCCPathCacheEntry&, const SkIVector& shift, uint64_t,
                 DoEvenOddFill = DoEvenOddFill::kNo);
    };

    GR_STATIC_ASSERT(4 * 12 == sizeof(Instance));

    static sk_sp<const GrGpuBuffer> FindVertexBuffer(GrOnFlushResourceProvider*);
    static sk_sp<const GrGpuBuffer> FindIndexBuffer(GrOnFlushResourceProvider*);

    GrCCPathProcessor(const GrTextureProxy* atlas,
                      const SkMatrix& viewMatrixIfUsingLocalCoords = SkMatrix::I());

    const char* name() const override { return "GrCCPathProcessor"; }
    const SkISize& atlasSize() const { return fAtlasSize; }
    GrSurfaceOrigin atlasOrigin() const { return fAtlasOrigin; }
    const SkMatrix& localMatrix() const { return fLocalMatrix; }
    const Attribute& getInstanceAttrib(InstanceAttribs attribID) const {
        int idx = static_cast<int>(attribID);
        SkASSERT(idx >= 0 && idx < static_cast<int>(SK_ARRAY_COUNT(kInstanceAttribs)));
        return kInstanceAttribs[idx];
    }
    const Attribute& getEdgeNormsAttrib() const { return kEdgeNormsAttrib; }

    void getGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder*) const override {}
    GrGLSLPrimitiveProcessor* createGLSLInstance(const GrShaderCaps&) const override;

    void drawPaths(GrOpFlushState*, const GrPipeline&, const GrPipeline::FixedDynamicState*,
                   const GrCCPerFlushResources&, int baseInstance, int endInstance,
                   const SkRect& bounds) const;

private:
    const TextureSampler& onTextureSampler(int) const override { return fAtlasAccess; }

    const TextureSampler fAtlasAccess;
    SkISize fAtlasSize;
    GrSurfaceOrigin fAtlasOrigin;

    SkMatrix fLocalMatrix;
    static constexpr Attribute kInstanceAttribs[kNumInstanceAttribs] = {
            {"devbounds", kFloat4_GrVertexAttribType, kFloat4_GrSLType},
            {"devbounds45", kFloat4_GrVertexAttribType, kFloat4_GrSLType},
            {"dev_to_atlas_offset", kInt2_GrVertexAttribType, kInt2_GrSLType},
            {"color", kHalf4_GrVertexAttribType, kHalf4_GrSLType}
    };
    static constexpr Attribute kEdgeNormsAttrib = {"edge_norms", kFloat4_GrVertexAttribType,
                                                                 kFloat4_GrSLType};

    typedef GrGeometryProcessor INHERITED;
};

inline void GrCCPathProcessor::Instance::set(const SkRect& devBounds, const SkRect& devBounds45,
                                             const SkIVector& devToAtlasOffset, uint64_t color,
                                             DoEvenOddFill doEvenOddFill) {
    if (DoEvenOddFill::kYes == doEvenOddFill) {
        // "right < left" indicates even-odd fill type.
        fDevBounds.setLTRB(devBounds.fRight, devBounds.fTop, devBounds.fLeft, devBounds.fBottom);
    } else {
        fDevBounds = devBounds;
    }
    fDevBounds45 = devBounds45;
    fDevToAtlasOffset = devToAtlasOffset;
    fColor = color;
}

#endif
