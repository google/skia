/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrDrawAtlasOp_DEFINED
#define GrDrawAtlasOp_DEFINED

#include "GrColor.h"
#include "GrDefaultGeoProcFactory.h"
#include "GrMeshDrawOp.h"

class GrDrawAtlasOp final : public GrMeshDrawOp {
public:
    DEFINE_OP_CLASS_ID

    static sk_sp<GrDrawOp> Make(GrColor color, const SkMatrix& viewMatrix, int spriteCount,
                                const SkRSXform* xforms, const SkRect* rects,
                                const SkColor* colors) {
        return sk_sp<GrDrawOp>(
                new GrDrawAtlasOp(color, viewMatrix, spriteCount, xforms, rects, colors));
    }

    const char* name() const override { return "DrawAtlasOp"; }

    SkString dumpInfo() const override {
        SkString string;
        for (const auto& geo : fGeoData) {
            string.appendf("Color: 0x%08x, Quads: %d\n", geo.fColor, geo.fVerts.count() / 4);
        }
        string.append(DumpPipelineInfo(*this->pipeline()));
        string.append(INHERITED::dumpInfo());
        return string;
    }

    void computePipelineOptimizations(GrInitInvariantOutput* color,
                                      GrInitInvariantOutput* coverage,
                                      GrBatchToXPOverrides* overrides) const override {
        // When this is called there is only one atlas draw.
        if (this->hasColors()) {
            color->setUnknownFourComponents();
        } else {
            color->setKnownFourComponents(fGeoData[0].fColor);
        }
        coverage->setKnownSingleComponent(0xff);
    }

private:
    GrDrawAtlasOp(GrColor color, const SkMatrix& viewMatrix, int spriteCount,
                  const SkRSXform* xforms, const SkRect* rects, const SkColor* colors);

    void onPrepareDraws(Target*) const override;

    void initBatchTracker(const GrXPOverridesForBatch&) override;

    GrColor color() const { return fColor; }
    bool colorIgnored() const { return fColorIgnored; }
    const SkMatrix& viewMatrix() const { return fViewMatrix; }
    bool hasColors() const { return fHasColors; }
    int quadCount() const { return fQuadCount; }
    bool coverageIgnored() const { return fCoverageIgnored; }

    bool onCombineIfPossible(GrOp* t, const GrCaps&) override;

    struct Geometry {
        GrColor fColor;
        SkTArray<uint8_t, true> fVerts;
    };

    SkSTArray<1, Geometry, true> fGeoData;

    SkMatrix fViewMatrix;
    GrColor fColor;
    int fQuadCount;
    bool fColorIgnored;
    bool fCoverageIgnored;
    bool fHasColors;

    typedef GrMeshDrawOp INHERITED;
};

#endif
