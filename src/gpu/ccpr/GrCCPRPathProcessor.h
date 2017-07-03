/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrCCPRPathProcessor_DEFINED
#define GrCCPRPathProcessor_DEFINED

#include "GrGeometryProcessor.h"
#include <array>

class GrShaderCaps;

class GrCCPRPathProcessor : public GrGeometryProcessor {
public:
    GrCCPRPathProcessor(GrResourceProvider*, sk_sp<GrTextureProxy> atlas, SkPath::FillType,
                       const GrShaderCaps&);

    enum class Attrib {
        kDevBounds,
        kDevBounds45,
        kViewMatrix,
        kViewTranslate,
        kAtlasOffset,
        kColor
    };
    static constexpr int kNumAttribs = 1 + (int)Attrib::kColor;

    struct Instance {
        SkRect                 fDevBounds;
        SkRect                 fDevBounds45;
        std::array<float, 4>   fViewMatrix;
        std::array<float, 2>   fViewTranslate;
        std::array<int, 2>     fAtlasOffset;
        uint32_t               fColor;

        GR_STATIC_ASSERT(SK_SCALAR_IS_FLOAT);
    };

    GR_STATIC_ASSERT(sizeof(Instance) == 4 * 17);

    const char* name() const override { return "Cover Processor"; }

    const Attribute& getAttrib(Attrib attrib) const {
        return this->INHERITED::getAttrib(int(attrib));
    }

    const GrTexture* atlas() const { return fAtlasAccess.peekTexture(); }

    SkPath::FillType fillType() const { return fFillType; }

    void getGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder*) const override;
    GrGLSLPrimitiveProcessor* createGLSLInstance(const GrShaderCaps&) const override;

private:
    TextureSampler     fAtlasAccess;
    SkPath::FillType   fFillType;

    typedef GrGeometryProcessor INHERITED;
};

#endif
