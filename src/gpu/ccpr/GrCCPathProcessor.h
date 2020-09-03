/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrCCPathProcessor_DEFINED
#define GrCCPathProcessor_DEFINED

#include <array>
#include "include/core/SkPath.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrGeometryProcessor.h"
#include "src/gpu/GrPipeline.h"
#include "src/gpu/ccpr/GrOctoBounds.h"

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
    struct Instance {
        SkRect fDevBounds;  // "right < left" indicates even-odd fill type.
        SkRect fDevBounds45;  // Bounding box in "| 1  -1 | * devCoords" space. See GrOctoBounds.
                              //                  | 1   1 |
        SkIVector fDevToAtlasOffset;  // Translation from device space to location in atlas.
        SkPMColor4f fColor;  // Color always stored as 4 x fp32

        void set(const GrOctoBounds&, const SkIVector& devToAtlasOffset, const SkPMColor4f&,
                 GrFillRule);
        void set(const GrCCPathCacheEntry&, const SkIVector& shift, const SkPMColor4f&, GrFillRule);
    };

    static_assert(4 * 14 == sizeof(Instance));

    static sk_sp<const GrGpuBuffer> FindVertexBuffer(GrOnFlushResourceProvider*);
    static sk_sp<const GrGpuBuffer> FindIndexBuffer(GrOnFlushResourceProvider*);

    enum class CoverageMode : bool {
        kCoverageCount,
        kLiteral
    };

    static GrColorType GetColorTypeFromCoverageMode(CoverageMode mode) {
        return mode == CoverageMode::kCoverageCount ? GrColorType::kAlpha_F16
            : GrColorType::kAlpha_8;
    }

    GrCCPathProcessor(CoverageMode, const GrTexture* atlasTexture, const GrSwizzle&,
                      GrSurfaceOrigin atlasOrigin,
                      const SkMatrix& viewMatrixIfUsingLocalCoords = SkMatrix::I());

    const char* name() const override { return "GrCCPathProcessor"; }
    void getGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder* b) const override;

    GrGLSLPrimitiveProcessor* createGLSLInstance(const GrShaderCaps&) const override;

    void drawPaths(GrOpFlushState*, const GrPipeline&, const GrSurfaceProxy& atlasProxy,
                   const GrCCPerFlushResources&, int baseInstance, int endInstance,
                   const SkRect& bounds) const;

private:
    const TextureSampler& onTextureSampler(int) const override { return fAtlasAccess; }

    const CoverageMode fCoverageMode;
    const TextureSampler fAtlasAccess;
    SkISize fAtlasDimensions;
    GrSurfaceOrigin fAtlasOrigin;

    SkMatrix fLocalMatrix;
    static constexpr Attribute kInstanceAttribs[] = {
            {"devbounds", kFloat4_GrVertexAttribType, kFloat4_GrSLType},
            {"devbounds45", kFloat4_GrVertexAttribType, kFloat4_GrSLType},
            {"dev_to_atlas_offset", kInt2_GrVertexAttribType, kInt2_GrSLType},
            {"color", kFloat4_GrVertexAttribType, kHalf4_GrSLType}
    };
    static constexpr int kColorAttribIdx = 3;
    static constexpr Attribute kCornersAttrib =
            {"corners", kFloat4_GrVertexAttribType, kFloat4_GrSLType};

    class Impl;

    using INHERITED = GrGeometryProcessor;
};

inline void GrCCPathProcessor::Instance::set(
        const GrOctoBounds& octoBounds, const SkIVector& devToAtlasOffset, const SkPMColor4f& color,
        GrFillRule fillRule) {
    if (GrFillRule::kNonzero == fillRule) {
        // We cover "nonzero" paths with clockwise triangles, which is the default result from
        // normal octo bounds.
        fDevBounds = octoBounds.bounds();
        fDevBounds45 = octoBounds.bounds45();
    } else {
        // We cover "even/odd" paths with counterclockwise triangles. Here we reorder the bounding
        // box vertices so the output is flipped horizontally.
        fDevBounds.setLTRB(
                octoBounds.right(), octoBounds.top(), octoBounds.left(), octoBounds.bottom());
        fDevBounds45.setLTRB(
                octoBounds.bottom45(), octoBounds.right45(), octoBounds.top45(),
                octoBounds.left45());
    }
    fDevToAtlasOffset = devToAtlasOffset;
    fColor = color;
}

#endif
