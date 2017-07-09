/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrCCPRPathProcessor_DEFINED
#define GrCCPRPathProcessor_DEFINED

#include "GrGeometryProcessor.h"
#include "SkPath.h"
#include <array>

class GrShaderCaps;

/**
 * This class draws AA paths using the coverage count masks produced by GrCCPRCoverageProcessor.
 *
 * Paths are drawn as bloated octagons, and coverage is derived from the coverage count mask and
 * fill rule.
 *
 * The caller must set up an instance buffer as detailed below, then draw instanced, 8-vertex
 * triangle strips. There are no vertex attribs.
 */
class GrCCPRPathProcessor : public GrGeometryProcessor {
public:
    static constexpr GrPrimitiveType kInstanceGrPrimitiveType = GrPrimitiveType::kTriangleStrip;
    static constexpr int kInstanceVertexCount = 8;

    enum class InstanceAttribs {
        kDevBounds,
        kDevBounds45,
        kViewMatrix, // FIXME: This causes a lot of duplication. It could move to a texel buffer.
        kViewTranslate,
        kAtlasOffset,
        kColor
    };
    static constexpr int kNumInstanceAttribs = 1 + (int)InstanceAttribs::kColor;

    struct Instance {
        SkRect                   fDevBounds;
        SkRect                   fDevBounds45; // Bounding box in "| 1  -1 | * devCoords" space.
                                               //                  | 1   1 |
        std::array<float, 4>     fViewMatrix;  // {kScaleX, kSkewy, kSkewX, kScaleY}
        std::array<float, 2>     fViewTranslate;
        std::array<int32_t, 2>   fAtlasOffset;
        uint32_t                 fColor;

        GR_STATIC_ASSERT(SK_SCALAR_IS_FLOAT);
    };

    GR_STATIC_ASSERT(4 * 17 == sizeof(Instance));

    GrCCPRPathProcessor(GrResourceProvider*, sk_sp<GrTextureProxy> atlas, SkPath::FillType,
                       const GrShaderCaps&);

    const char* name() const override { return "GrCCPRPathProcessor"; }
    const GrTexture* atlas() const { return fAtlasAccess.peekTexture(); }
    SkPath::FillType fillType() const { return fFillType; }
    const Attribute& getAttrib(InstanceAttribs attrib) const {
        return this->INHERITED::getAttrib((int)attrib);
    }

    void getGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder*) const override;
    GrGLSLPrimitiveProcessor* createGLSLInstance(const GrShaderCaps&) const override;

private:
    TextureSampler     fAtlasAccess;
    SkPath::FillType   fFillType;

    typedef GrGeometryProcessor INHERITED;
};

#endif
