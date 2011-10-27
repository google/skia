/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrDrawState_DEFINED
#define GrDrawState_DEFINED

#include "GrColor.h"
#include "GrMatrix.h"
#include "GrSamplerState.h"
#include "GrStencil.h"

#include "SkXfermode.h"

class GrRenderTarget;
class GrTexture;

struct GrDrawState {

    /**
     * Number of texture stages. Each stage takes as input a color and
     * 2D texture coordinates. The color input to the first enabled stage is the
     * per-vertex color or the constant color (setColor/setAlpha) if there are
     * no per-vertex colors. For subsequent stages the input color is the output
     * color from the previous enabled stage. The output color of each stage is
     * the input color modulated with the result of a texture lookup. Texture
     * lookups are specified by a texture a sampler (setSamplerState). Texture
     * coordinates for each stage come from the vertices based on a
     * GrVertexLayout bitfield. The output fragment color is the output color of
     * the last enabled stage. The presence or absence of texture coordinates
     * for each stage in the vertex layout indicates whether a stage is enabled
     * or not.
     */
    enum {
        kNumStages = 3,
        kMaxTexCoords = kNumStages
    };

    enum DrawFace {
        kBoth_DrawFace,
        kCCW_DrawFace,
        kCW_DrawFace,
    };

     /**
     * When specifying edges as vertex data this enum specifies what type of
     * edges are in use. The edges are always 4 GrScalars in memory, even when
     * the edge type requires fewer than 4.
     */
    enum VertexEdgeType {
        /* 1-pixel wide line
           2D implicit line eq (a*x + b*y +c = 0). 4th component unused */
        kHairLine_EdgeType,
        /* 1-pixel wide quadratic
           u^2-v canonical coords (only 2 components used) */
        kHairQuad_EdgeType
    };

    /**
     * The absolute maximum number of edges that may be specified for
     * a single draw call when performing edge antialiasing.  This is used for
     * the size of several static buffers, so implementations of getMaxEdges()
     * (below) should clamp to this value.
     */
    enum {
        kMaxEdges = 32
    };

    class Edge {
      public:
        Edge() {}
        Edge(float x, float y, float z) : fX(x), fY(y), fZ(z) {}
        GrPoint intersect(const Edge& other) {
            return GrPoint::Make(
                (fY * other.fZ - other.fY * fZ) /
                  (fX * other.fY - other.fX * fY),
                (fX * other.fZ - other.fX * fZ) /
                  (other.fX * fY - fX * other.fY));
        }
        float fX, fY, fZ;
    };

    GrDrawState() {
        // make sure any pad is zero for memcmp
        // all GrDrawState members should default to something
        // valid by the memset
        memset(this, 0, sizeof(GrDrawState));
            
        // memset exceptions
        fColorFilterXfermode = SkXfermode::kDstIn_Mode;
        fFirstCoverageStage = kNumStages;

        // pedantic assertion that our ptrs will
        // be NULL (0 ptr is mem addr 0)
        GrAssert((intptr_t)(void*)NULL == 0LL);

        // default stencil setting should be disabled
        GrAssert(fStencilSettings.isDisabled());
        fFirstCoverageStage = kNumStages;
    }

    uint32_t                fFlagBits;
    GrBlendCoeff            fSrcBlend;
    GrBlendCoeff            fDstBlend;
    GrColor                 fBlendConstant;
    GrTexture*              fTextures[kNumStages];
    GrSamplerState          fSamplerStates[kNumStages];
    int                     fFirstCoverageStage;
    GrRenderTarget*         fRenderTarget;
    GrColor                 fColor;
    DrawFace                fDrawFace;
    GrColor                 fColorFilterColor;
    SkXfermode::Mode        fColorFilterXfermode;

    GrStencilSettings       fStencilSettings;
    GrMatrix                fViewMatrix;
    VertexEdgeType          fVertexEdgeType;
    Edge                    fEdgeAAEdges[kMaxEdges];
    int                     fEdgeAANumEdges;
    bool operator ==(const GrDrawState& s) const {
        return 0 == memcmp(this, &s, sizeof(GrDrawState));
    }
    bool operator !=(const GrDrawState& s) const { return !(*this == s); }
};

#endif
