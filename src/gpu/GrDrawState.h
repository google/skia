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

    /**
     *  Bitfield used to indicate a set of stages.
     */
    typedef uint32_t StageMask;
    GR_STATIC_ASSERT(sizeof(StageMask)*8 >= GrDrawState::kNumStages);

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
        // TODO: this should be 32 when GrTesselatedPathRenderer is used
        // Visual Studio 2010 does not permit a member array of size 0.
        kMaxEdges = 1
    };

    class Edge {
      public:
        Edge() {}
        Edge(float x, float y, float z) : fX(x), fY(y), fZ(z) {}
        GrPoint intersect(const Edge& other) {
            return GrPoint::Make(
                SkFloatToScalar((fY * other.fZ - other.fY * fZ) /
                                (fX * other.fY - other.fX * fY)),
                SkFloatToScalar((fX * other.fZ - other.fX * fZ) /
                                (other.fX * fY - fX * other.fY)));
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

    uint8_t                 fFlagBits;
    GrBlendCoeff            fSrcBlend : 8;
    GrBlendCoeff            fDstBlend : 8;
    DrawFace                fDrawFace : 8;
    uint8_t                 fFirstCoverageStage;
    SkXfermode::Mode        fColorFilterXfermode : 8;
    GrColor                 fBlendConstant;
    GrTexture*              fTextures[kNumStages];
    GrRenderTarget*         fRenderTarget;
    GrColor                 fColor;
    GrColor                 fColorFilterColor;

    GrStencilSettings       fStencilSettings;
    GrMatrix                fViewMatrix;

    // @{ Data for GrTesselatedPathRenderer
    // TODO: currently ignored in copying & comparison for performance.
    // Must be considered if GrTesselatedPathRenderer is being used.

    int                     fEdgeAANumEdges;
    VertexEdgeType          fVertexEdgeType;
    Edge                    fEdgeAAEdges[kMaxEdges];

    // @}

    // This field must be last; it will not be copied or compared
    // if the corresponding fTexture[] is NULL.
    GrSamplerState          fSamplerStates[kNumStages];

    // Most stages are usually not used, so conditionals here
    // reduce the expected number of bytes touched by 50%.
    bool operator ==(const GrDrawState& s) const {
        if (memcmp(this, &s, this->leadingBytes())) return false;

        for (int i = 0; i < kNumStages; i++) {
            if (fTextures[i] &&
                memcmp(&this->fSamplerStates[i], &s.fSamplerStates[i],
                       sizeof(GrSamplerState))) {
                return false;
            }
        }

        return true;
    }
    bool operator !=(const GrDrawState& s) const { return !(*this == s); }

    // Most stages are usually not used, so conditionals here 
    // reduce the expected number of bytes touched by 50%.
    GrDrawState& operator =(const GrDrawState& s) {
        memcpy(this, &s, this->leadingBytes());

        for (int i = 0; i < kNumStages; i++) {
            if (s.fTextures[i]) {
                memcpy(&this->fSamplerStates[i], &s.fSamplerStates[i],
                       sizeof(GrSamplerState));
            }
        }

        return *this;
    }

private:
    size_t leadingBytes() const {
        // Can't use offsetof() with non-POD types, so stuck with pointer math.
        // TODO: ignores GrTesselatedPathRenderer data structures. We don't
        // have a compile-time flag that lets us know if it's being used, and
        // checking at runtime seems to cost 5% performance.
        return (size_t) ((unsigned char*)&fEdgeAANumEdges -
                         (unsigned char*)&fFlagBits);
    }

};

#endif

