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
#include "GrSimpleMeshDrawOpHelper.h"
#include "GrTypes.h"
#include "SkMatrix.h"
#include "SkRect.h"
#include "SkTDArray.h"
#include "SkVertices.h"

class GrOpFlushState;
class SkVertices;
struct GrInitInvariantOutput;

class GrDrawVerticesOp final : public GrMeshDrawOp {
private:
    using Helper = GrSimpleMeshDrawOpHelper;

public:
    DEFINE_OP_CLASS_ID

    /**
     * Draw a SkVertices. The GrPaint param's color is used if the vertices lack per-vertex color.
     * If the vertices lack local coords then the vertex positions are used as local coords. The
     * primitive type drawn is derived from the SkVertices object, unless overridePrimType is
     * specified. If gammaCorrect is true, the vertex colors will be linearized in the shader to get
     * correct rendering.
     */
    static std::unique_ptr<GrDrawOp> Make(GrPaint&&, sk_sp<SkVertices>, const SkMatrix& viewMatrix,
                                          GrAAType, bool gammaCorrect, sk_sp<GrColorSpaceXform>,
                                          GrPrimitiveType* overridePrimType = nullptr);

    GrDrawVerticesOp(const Helper::MakeArgs& helperArgs, GrColor, sk_sp<SkVertices>,
                     GrPrimitiveType, GrAAType, bool gammaCorrect, sk_sp<GrColorSpaceXform>,
                     const SkMatrix& viewMatrix);

    const char* name() const override { return "DrawVerticesOp"; }

    SkString dumpInfo() const override;

    FixedFunctionFlags fixedFunctionFlags() const override;

    RequiresDstTexture finalize(const GrCaps& caps, const GrAppliedClip* clip) override;

private:
    enum class ColorArrayType {
        kPremulGrColor,
        kSkColor,
    };

    void onPrepareDraws(Target*) const override;

    sk_sp<GrGeometryProcessor> makeGP(bool* hasColorAttribute, bool* hasLocalCoordAttribute) const;

    GrPrimitiveType primitiveType() const { return fPrimitiveType; }
    bool combinablePrimitive() const {
        return GrPrimitiveType::kTriangles == fPrimitiveType ||
               GrPrimitiveType::kLines == fPrimitiveType ||
               GrPrimitiveType::kPoints == fPrimitiveType;
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

    bool hasMultipleViewMatrices() const {
        return SkToBool(kHasMultipleViewMatrices_Flag & fFlags);
    }

    enum Flags {
        kRequiresPerVertexColors_Flag = 0x1,
        kAnyMeshHasExplicitLocalCoords = 0x2,
        kHasMultipleViewMatrices_Flag = 0x4

    };

    Helper fHelper;
    SkSTArray<1, Mesh, true> fMeshes;
    // GrPrimitiveType is more expressive than fVertices.mode() so it is used instead and we ignore
    // the SkVertices mode (though fPrimitiveType may have been inferred from it).
    GrPrimitiveType fPrimitiveType;
    uint32_t fFlags;
    int fVertexCount;
    int fIndexCount;
    ColorArrayType fColorArrayType;
    bool fLinearizeColors;
    sk_sp<GrColorSpaceXform> fColorSpaceXform;

    typedef GrMeshDrawOp INHERITED;
};

#endif
