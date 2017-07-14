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

class GrOnFlushResourceProvider;
class GrShaderCaps;

/**
 * This class draws AA paths using the coverage count masks produced by GrCCPRCoverageProcessor.
 *
 * Paths are drawn as bloated octagons, and coverage is derived from the coverage count mask and
 * fill rule.
 *
 * The caller must set up an instance buffer as detailed below, then draw indexed-instanced
 * triangles using the index and vertex buffers provided by this class.
 */
class GrCCPRPathProcessor : public GrGeometryProcessor {
public:
    static constexpr int kPerInstanceIndexCount = 6 * 3;
    static sk_sp<GrBuffer> FindOrMakeIndexBuffer(GrOnFlushResourceProvider*);
    static sk_sp<GrBuffer> FindOrMakeVertexBuffer(GrOnFlushResourceProvider*);

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

    GR_STATIC_ASSERT(4 * 17 == sizeof(Instance)); // FIXME: 4 * 16 by making fAtlasOffset int16_t's.

    GrCCPRPathProcessor(GrResourceProvider*, sk_sp<GrTextureProxy> atlas, SkPath::FillType,
                       const GrShaderCaps&);

    const char* name() const override { return "GrCCPRPathProcessor"; }
    const GrTexture* atlas() const { return fAtlasAccess.peekTexture(); }
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

private:
    const SkPath::FillType   fFillType;
    TextureSampler           fAtlasAccess;

    typedef GrGeometryProcessor INHERITED;
};

#endif
