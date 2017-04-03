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

class GrDrawAtlasOp final : public GrLegacyMeshDrawOp {
public:
    DEFINE_OP_CLASS_ID

    static std::unique_ptr<GrLegacyMeshDrawOp> Make(GrColor color, const SkMatrix& viewMatrix,
                                                    int spriteCount, const SkRSXform* xforms,
                                                    const SkRect* rects, const SkColor* colors) {
        return std::unique_ptr<GrLegacyMeshDrawOp>(
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

private:
    GrDrawAtlasOp(GrColor color, const SkMatrix& viewMatrix, int spriteCount,
                  const SkRSXform* xforms, const SkRect* rects, const SkColor* colors);

    void getProcessorAnalysisInputs(GrProcessorAnalysisColor* color,
                                    GrProcessorAnalysisCoverage* coverage) const override {
        if (this->hasColors()) {
            color->setToUnknown();
        } else {
            color->setToConstant(fGeoData[0].fColor);
        }
        *coverage = GrProcessorAnalysisCoverage::kNone;
    }

    void onPrepareDraws(Target*) const override;

    void applyPipelineOptimizations(const PipelineOptimizations&) override;

    GrColor color() const { return fColor; }
    const SkMatrix& viewMatrix() const { return fViewMatrix; }
    bool hasColors() const { return fHasColors; }
    int quadCount() const { return fQuadCount; }

    bool onCombineIfPossible(GrOp* t, const GrCaps&) override;

    struct Geometry {
        GrColor fColor;
        SkTArray<uint8_t, true> fVerts;
    };

    SkSTArray<1, Geometry, true> fGeoData;

    SkMatrix fViewMatrix;
    GrColor fColor;
    int fQuadCount;
    bool fHasColors;

    typedef GrLegacyMeshDrawOp INHERITED;
};

#endif
