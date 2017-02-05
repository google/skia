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

class GrDrawVerticesOp final : public GrMeshDrawOp {
public:
    DEFINE_OP_CLASS_ID

    /**
     * The 'color' param is only used if the 'colors' array is null. Bounds is the bounds of the
     * 'positions' array before application of 'viewMatrix'. If indices is null then indexCnt must
     * be zero and vice versa. In this case the vertices are indexed as 0, 1, ..., vertexCount - 1.
     * 'localCoords' are optional and if null the positions are used as local coords.
     */
    static std::unique_ptr<GrDrawOp> Make(GrColor color, GrPrimitiveType primitiveType,
                                          const SkMatrix& viewMatrix, const SkPoint* positions,
                                          int vertexCount, const uint16_t* indices, int indexCount,
                                          const uint32_t* colors, const SkPoint* localCoords,
                                          const SkRect& bounds,
                                          GrRenderTargetContext::ColorArrayType colorArrayType);

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
                     GrRenderTargetContext::ColorArrayType, const SkMatrix& viewMatrix);

    void getPipelineAnalysisInput(GrPipelineAnalysisDrawOpInput* input) const override;
    void applyPipelineOptimizations(const GrPipelineOptimizations&) override;
    void onPrepareDraws(Target*) const override;

    GrPrimitiveType primitiveType() const { return fPrimitiveType; }
    bool combinablePrimitive() const {
        return kTriangles_GrPrimitiveType == fPrimitiveType ||
               kLines_GrPrimitiveType == fPrimitiveType ||
               kPoints_GrPrimitiveType == fPrimitiveType;
    }

    bool hasExplicitLocalCoords() const {
        // We currently enforce consistency of this in onCombineIfPossible, though that could be
        // relaxed.
        return fMeshes[0].fVertices->hasTexCoords() && !(kIgnoreVerticesLocalCoords_Flag & fFlags);
    }
    bool hasExplicitColors() const {
        // We currently enforce consistency of this in onCombineIfPossible, though that could be
        // relaxed.
        return fMeshes[0].fVertices->hasColors() && !(kIgnoreVerticesColors_Flag & fFlags);
    }
    bool isIndexed() const {
        // Consistency enforced in onCombineIfPossible.
        return fMeshes[0].fVertices->isIndexed();
    }
    bool onCombineIfPossible(GrOp* t, const GrCaps&) override;

    struct Mesh {
        GrColor fColor;  // Only used if fVertics lacks colors.
        sk_sp<SkVertices> fVertices;
        SkMatrix fViewMatrix;
    };

    // GrPrimitiveType is more expressive than fVertices.mode() so it is used instead and we ignore
    // the SkVertices mode (though fPrimitiveType may have been inferred from it).
    GrPrimitiveType fPrimitiveType;
    enum Flags {
        kIgnoreVerticesColors_Flag = 0x1,
        kIgnoreVerticesLocalCoords_Flag = 0x2,
    };
    uint32_t fFlags = 0;
    int fVertexCount;
    int fIndexCount;
    bool fMultipleViewMatrices = false;
    bool fPipelineNeedsLocalCoords;
    GrRenderTargetContext::ColorArrayType fColorArrayType;
    SkSTArray<1, Mesh, true> fMeshes;

    typedef GrMeshDrawOp INHERITED;
};

#endif
