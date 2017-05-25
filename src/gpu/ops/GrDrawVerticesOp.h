/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrDrawVerticesOp_DEFINED
#define GrDrawVerticesOp_DEFINED

#include "GrColor.h"
#include "GrMeshDrawOp.h"
#include "GrRenderTargetContext.h"
#include "GrTypes.h"
#include "SkMatrix.h"
#include "SkRect.h"
#include "SkTDArray.h"
#include "SkVertices.h"

#if GR_TEST_UTILS
#include "GrDrawOpTest.h"
#endif

class GrOpFlushState;
class SkVertices;
struct GrInitInvariantOutput;

class GrDrawVerticesOp final : public GrLegacyMeshDrawOp {
public:
    DEFINE_OP_CLASS_ID

    /**
     * Draw a SkVertices. The GrColor param is used if the vertices lack per-vertex color. If the
     * vertices lack local coords then the vertex positions are used as local coords. The primitive
     * type drawn is derived from the SkVertices object, unless overridePrimType is specified.
     */
    static std::unique_ptr<GrLegacyMeshDrawOp> Make(GrColor color, sk_sp<SkVertices>,
                                                    const SkMatrix& viewMatrix,
                                                    GrPrimitiveType* overridePrimType = nullptr);

    const char* name() const override { return "DrawVerticesOp"; }

    SkString dumpInfo() const override {
        SkString string;
        string.appendf("PrimType: %d, MeshCount %d, VCount: %d, ICount: %d\n", fPrimitiveType,
                       fMeshes.count(), fVertexCount, fIndexCount);
        string.append(DumpPipelineInfo(*this->pipeline()));
        string.append(INHERITED::dumpInfo());
        return string;
    }

private:
    enum class ColorArrayType {
        kPremulGrColor,
        kSkColor,
    };

    GrDrawVerticesOp(sk_sp<SkVertices>, GrPrimitiveType, GrColor, const SkMatrix& viewMatrix);

    void getProcessorAnalysisInputs(GrProcessorAnalysisColor* color,
                                    GrProcessorAnalysisCoverage* coverage) const override;
    void applyPipelineOptimizations(const PipelineOptimizations&) override;
    void onPrepareDraws(Target*) const override;

    sk_sp<GrGeometryProcessor> makeGP(bool* hasColorAttribute, bool* hasLocalCoordAttribute) const;

    GrPrimitiveType primitiveType() const { return fPrimitiveType; }
    bool combinablePrimitive() const {
        return kTriangles_GrPrimitiveType == fPrimitiveType ||
               kLines_GrPrimitiveType == fPrimitiveType ||
               kPoints_GrPrimitiveType == fPrimitiveType;
    }

    bool onCombineIfPossible(GrOp* t, const GrCaps&) override;

    struct Mesh {
        GrColor fColor;  // Used if this->hasPerVertexColors() is false.
        sk_sp<SkVertices> fVertices;
        SkMatrix fViewMatrix;
        bool fIgnoreTexCoords;
        bool fIgnoreColors;

        bool hasExplicitLocalCoords() const {
            return fVertices->hasTexCoords() && !fIgnoreTexCoords;
        }

        bool hasPerVertexColors() const {
            return fVertices->hasColors() && !fIgnoreColors;
        }
    };

    bool isIndexed() const {
        // Consistency enforced in onCombineIfPossible.
        return fMeshes[0].fVertices->hasIndices();
    }

    bool requiresPerVertexColors() const {
        return SkToBool(kRequiresPerVertexColors_Flag & fFlags);
    }

    bool anyMeshHasExplicitLocalCoords() const {
        return SkToBool(kAnyMeshHasExplicitLocalCoords & fFlags);
    }

    bool pipelineRequiresLocalCoords() const {
        return SkToBool(kPipelineRequiresLocalCoords_Flag & fFlags);
    }

    bool hasMultipleViewMatrices() const {
        return SkToBool(kHasMultipleViewMatrices_Flag & fFlags);
    }

    enum Flags {
        kRequiresPerVertexColors_Flag = 0x1,
        kAnyMeshHasExplicitLocalCoords = 0x2,
        kPipelineRequiresLocalCoords_Flag = 0x4,
        kHasMultipleViewMatrices_Flag = 0x8

    };

    // GrPrimitiveType is more expressive than fVertices.mode() so it is used instead and we ignore
    // the SkVertices mode (though fPrimitiveType may have been inferred from it).
    GrPrimitiveType fPrimitiveType;
    uint32_t fFlags;
    int fVertexCount;
    int fIndexCount;
    ColorArrayType fColorArrayType;
    SkSTArray<1, Mesh, true> fMeshes;

    typedef GrLegacyMeshDrawOp INHERITED;

#if GR_TEST_UTILS
    GR_LEGACY_MESH_DRAW_OP_TEST_FRIEND(VerticesOp);
#endif
};

#endif
