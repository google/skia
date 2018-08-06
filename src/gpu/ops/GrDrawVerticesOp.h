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
     * specified.
     */
    static std::unique_ptr<GrDrawOp> Make(GrContext* context,
                                          GrPaint&&,
                                          sk_sp<SkVertices>,
                                          const SkMatrix bones[],
                                          int boneCount,
                                          const SkMatrix& viewMatrix,
                                          GrAAType,
                                          sk_sp<GrColorSpaceXform>,
                                          GrPrimitiveType* overridePrimType = nullptr);

    GrDrawVerticesOp(const Helper::MakeArgs&, GrColor, sk_sp<SkVertices>, const SkMatrix bones[],
                     int boneCount, GrPrimitiveType, GrAAType, sk_sp<GrColorSpaceXform>,
                     const SkMatrix& viewMatrix);

    const char* name() const override { return "DrawVerticesOp"; }

    void visitProxies(const VisitProxyFunc& func) const override {
        fHelper.visitProxies(func);
    }

    SkString dumpInfo() const override;

    FixedFunctionFlags fixedFunctionFlags() const override;

    RequiresDstTexture finalize(const GrCaps& caps, const GrAppliedClip* clip) override;

private:
    enum class ColorArrayType {
        kPremulGrColor,
        kSkColor,
    };

    void onPrepareDraws(Target*) override;

    void drawVolatile(Target*);
    void drawNonVolatile(Target*);

    void fillBuffers(bool hasColorAttribute,
                     bool hasLocalCoordsAttribute,
                     bool hasBoneAttribute,
                     size_t vertexStride,
                     void* verts,
                     uint16_t* indices) const;

    void drawVertices(Target*,
                      GrGeometryProcessor*,
                      const GrBuffer* vertexBuffer,
                      int firstVertex,
                      const GrBuffer* indexBuffer,
                      int firstIndex);

    sk_sp<GrGeometryProcessor> makeGP(const GrShaderCaps* shaderCaps,
                                      bool* hasColorAttribute,
                                      bool* hasLocalCoordAttribute,
                                      bool* hasBoneAttribute) const;

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
        std::vector<float> fBones; // Transformation matrices stored in GPU format.
        SkMatrix fViewMatrix;
        bool fIgnoreTexCoords;
        bool fIgnoreColors;
        bool fIgnoreBones;

        bool hasExplicitLocalCoords() const {
            return fVertices->hasTexCoords() && !fIgnoreTexCoords;
        }

        bool hasPerVertexColors() const {
            return fVertices->hasColors() && !fIgnoreColors;
        }

        bool hasBones() const {
            return fVertices->hasBones() && fBones.size() && !fIgnoreBones;
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
        return SkToBool(kAnyMeshHasExplicitLocalCoords_Flag & fFlags);
    }

    bool hasMultipleViewMatrices() const {
        return SkToBool(kHasMultipleViewMatrices_Flag & fFlags);
    }

    bool hasBones() const {
        return SkToBool(kHasBones_Flag & fFlags);
    }

    enum Flags {
        kRequiresPerVertexColors_Flag       = 0x1,
        kAnyMeshHasExplicitLocalCoords_Flag = 0x2,
        kHasMultipleViewMatrices_Flag       = 0x4,
        kHasBones_Flag                      = 0x8,
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
    sk_sp<GrColorSpaceXform> fColorSpaceXform;

    typedef GrMeshDrawOp INHERITED;
};

#endif
