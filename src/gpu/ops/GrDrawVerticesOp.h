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

class GrOpFlushState;
class SkVertices;
struct GrInitInvariantOutput;

class GrDrawVerticesOp final : public GrLegacyMeshDrawOp {
public:
    DEFINE_OP_CLASS_ID

    enum {
        kIgnoreTexCoords_VerticesFlag   = 1 << 0,
        kIgnoreColors_VerticesFlag      = 1 << 1,
    };

    /**
     * The 'color' param is used if the 'colors' array is null. 'bounds' is the bounds of the
     * 'positions' array (in local space prior to application of 'viewMatrix'). If 'indices' is null
     * then 'indexCnt' must be zero and vice versa. In this case the vertices are indexed as 0, 1,
     * ..., 'vertexCount' - 1. 'localCoords' are optional and if null the vertex positions are used
     * as local coords. 'colorArrayType' specifies whether the colors are premul GrColors or
     * unpremul SkColors.
     */
    static std::unique_ptr<GrLegacyMeshDrawOp> Make(
            GrColor color, GrPrimitiveType primitiveType, const SkMatrix& viewMatrix,
            const SkPoint* positions, int vertexCount, const uint16_t* indices, int indexCount,
            const uint32_t* colors, const SkPoint* localCoords, const SkRect& bounds,
            GrRenderTargetContext::ColorArrayType colorArrayType);

    /**
     * Draw a SkVertices. The GrColor param is used if the vertices lack per-vertex color or 'flags'
     * indicates that the per-vertex color should be ignored.  The 'flags' options are those
     * specified by SkCanvas::VerticesFlags. If the vertices lack local coords or 'flags' indicates
     * that they should be ignored then the vertex positions are used as local coords.
     */
    static std::unique_ptr<GrLegacyMeshDrawOp> Make(GrColor color, sk_sp<SkVertices>,
                                                    const SkMatrix& viewMatrix);

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
    GrDrawVerticesOp(sk_sp<SkVertices>, GrPrimitiveType, GrColor,
                     GrRenderTargetContext::ColorArrayType, const SkMatrix& viewMatrix,
                     uint32_t flags = 0);

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
        uint32_t fFlags;

        bool hasExplicitLocalCoords() const {
            return fVertices->hasTexCoords() && !(kIgnoreTexCoords_VerticesFlag & fFlags);
        }

        bool hasPerVertexColors() const {
            return fVertices->hasColors() && !(kIgnoreColors_VerticesFlag & fFlags);
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
    GrRenderTargetContext::ColorArrayType fColorArrayType;
    SkSTArray<1, Mesh, true> fMeshes;

    typedef GrLegacyMeshDrawOp INHERITED;
};

#endif
