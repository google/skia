/*
    Copyright 2010 Google Inc.

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

         http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
 */


#include "GrGLConfig.h"

#include "GrGpuGLFixed.h"
#include "GrGpuVertex.h"

#define SKIP_CACHE_CHECK    true

struct GrGpuMatrix {
    GrGLfloat    fMat[16];

    void reset() {
        Gr_bzero(fMat, sizeof(fMat));
        fMat[0] = fMat[5] = fMat[10] = fMat[15] = GR_Scalar1;
    }

    void set(const GrMatrix& m) {
        Gr_bzero(fMat, sizeof(fMat));
        fMat[0]  = GrScalarToFloat(m[GrMatrix::kMScaleX]);
        fMat[4]  = GrScalarToFloat(m[GrMatrix::kMSkewX]);
        fMat[12] = GrScalarToFloat(m[GrMatrix::kMTransX]);

        fMat[1]  = GrScalarToFloat(m[GrMatrix::kMSkewY]);
        fMat[5]  = GrScalarToFloat(m[GrMatrix::kMScaleY]);
        fMat[13] = GrScalarToFloat(m[GrMatrix::kMTransY]);

        fMat[3]  = GrScalarToFloat(m[GrMatrix::kMPersp0]);
        fMat[7]  = GrScalarToFloat(m[GrMatrix::kMPersp1]);
        fMat[15] = GrScalarToFloat(m[GrMatrix::kMPersp2]);

        fMat[10] = 1.f;    // z-scale
    }
};

// these must match the order in the corresponding enum in GrGpu.h
static const GrGLenum gMatrixMode2Enum[] = {
    GR_GL_MODELVIEW, GR_GL_TEXTURE
};

///////////////////////////////////////////////////////////////////////////////

GrGpuGLFixed::GrGpuGLFixed() {
    f4X4DownsampleFilterSupport = false;
    fDualSourceBlendingSupport = false;
}

GrGpuGLFixed::~GrGpuGLFixed() {
}

void GrGpuGLFixed::resetContext() {
    INHERITED::resetContext();

    GR_GL(Disable(GR_GL_TEXTURE_2D));

    for (int s = 0; s < kNumStages; ++s) {
        setTextureUnit(s);
        GR_GL(EnableClientState(GR_GL_VERTEX_ARRAY));
        GR_GL(TexEnvi(GR_GL_TEXTURE_ENV, GR_GL_TEXTURE_ENV_MODE, GR_GL_COMBINE));
        GR_GL(TexEnvi(GR_GL_TEXTURE_ENV, GR_GL_COMBINE_RGB,   GR_GL_MODULATE));
        GR_GL(TexEnvi(GR_GL_TEXTURE_ENV, GR_GL_SRC0_RGB,      GR_GL_TEXTURE0+s));
        GR_GL(TexEnvi(GR_GL_TEXTURE_ENV, GR_GL_SRC1_RGB,      GR_GL_PREVIOUS));
        GR_GL(TexEnvi(GR_GL_TEXTURE_ENV, GR_GL_OPERAND1_RGB,  GR_GL_SRC_COLOR));

        GR_GL(TexEnvi(GR_GL_TEXTURE_ENV, GR_GL_COMBINE_ALPHA, GR_GL_MODULATE));
        GR_GL(TexEnvi(GR_GL_TEXTURE_ENV, GR_GL_SRC0_ALPHA, GR_GL_TEXTURE0+s));
        GR_GL(TexEnvi(GR_GL_TEXTURE_ENV, GR_GL_OPERAND0_ALPHA, GR_GL_SRC_ALPHA));
        GR_GL(TexEnvi(GR_GL_TEXTURE_ENV, GR_GL_SRC1_ALPHA, GR_GL_PREVIOUS));
        GR_GL(TexEnvi(GR_GL_TEXTURE_ENV, GR_GL_OPERAND1_ALPHA, GR_GL_SRC_ALPHA));

        // color oprand0 changes between GL_SRC_COLR and GL_SRC_ALPHA depending
        // upon whether we have a (premultiplied) RGBA texture or just an ALPHA
        // texture, e.g.:
        //glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB,  GL_SRC_COLOR);
        fHWRGBOperand0[s] = (TextureEnvRGBOperands) -1;
    }

    fHWGeometryState.fVertexLayout = 0;
    fHWGeometryState.fVertexOffset  = ~0;
    GR_GL(EnableClientState(GR_GL_VERTEX_ARRAY));
    GR_GL(DisableClientState(GR_GL_TEXTURE_COORD_ARRAY));
    GR_GL(ShadeModel(GR_GL_FLAT));
    GR_GL(DisableClientState(GR_GL_COLOR_ARRAY));

    GR_GL(PointSize(1.f));

    GrGLClearErr();
    fTextVerts = false;

    fBaseVertex = 0xffffffff;
}


void GrGpuGLFixed::flushProjectionMatrix() {
    float mat[16];
    Gr_bzero(mat, sizeof(mat));

    GrAssert(NULL != fCurrDrawState.fRenderTarget);

    mat[0] = 2.f / fCurrDrawState.fRenderTarget->width();
    mat[5] = -2.f / fCurrDrawState.fRenderTarget->height();
    mat[10] = -1.f;
    mat[15] = 1;

    mat[12] = -1.f;
    mat[13] = 1.f;

    GR_GL(MatrixMode(GR_GL_PROJECTION));
    GR_GL(LoadMatrixf(mat));
}

bool GrGpuGLFixed::flushGraphicsState(GrPrimitiveType type) {

    bool usingTextures[kNumStages];

    for (int s = 0; s < kNumStages; ++s) {
        usingTextures[s] = this->isStageEnabled(s);
        if (usingTextures[s] && fCurrDrawState.fSamplerStates[s].isGradient()) {
            unimpl("Fixed pipe doesn't support radial/sweep gradients");
            return false;
        }
    }

    if (GR_GL_SUPPORT_ES1) {
        if (BlendCoeffReferencesConstant(fCurrDrawState.fSrcBlend) ||
            BlendCoeffReferencesConstant(fCurrDrawState.fDstBlend)) {
            unimpl("ES1 doesn't support blend constant");
            return false;
        }
    }

    if (!flushGLStateCommon(type)) {
        return false;
    }

    this->flushBlend(type, fCurrDrawState.fSrcBlend, fCurrDrawState.fDstBlend);

    if (fDirtyFlags.fRenderTargetChanged) {
        flushProjectionMatrix();
    }

    for (int s = 0; s < kNumStages; ++s) {
        bool wasUsingTexture = StageWillBeUsed(s, fHWGeometryState.fVertexLayout, fHWDrawState);
        if (usingTextures[s] != wasUsingTexture) {
            setTextureUnit(s);
            if (usingTextures[s]) {
                GR_GL(Enable(GR_GL_TEXTURE_2D));
            } else {
                GR_GL(Disable(GR_GL_TEXTURE_2D));
            }
        }
    }

    uint32_t vertColor = (this->getGeomSrc().fVertexLayout & kColor_VertexLayoutBit);
    uint32_t prevVertColor = (fHWGeometryState.fVertexLayout &
                              kColor_VertexLayoutBit);

    if (vertColor != prevVertColor) {
        if (vertColor) {
            GR_GL(ShadeModel(GR_GL_SMOOTH));
            // invalidate the immediate mode color
            fHWDrawState.fColor = GrColor_ILLEGAL;
        } else {
            GR_GL(ShadeModel(GR_GL_FLAT));
        }
    }


    if (!vertColor && fHWDrawState.fColor != fCurrDrawState.fColor) {
        GR_GL(Color4ub(GrColorUnpackR(fCurrDrawState.fColor),
                       GrColorUnpackG(fCurrDrawState.fColor),
                       GrColorUnpackB(fCurrDrawState.fColor),
                       GrColorUnpackA(fCurrDrawState.fColor)));
        fHWDrawState.fColor = fCurrDrawState.fColor;
    }

    // set texture environment, decide whether we are modulating by RGB or A.
    for (int s = 0; s < kNumStages; ++s) {
        if (usingTextures[s]) {
            GrGLTexture* texture = (GrGLTexture*)fCurrDrawState.fTextures[s];
            if (NULL != texture) {
                TextureEnvRGBOperands nextRGBOperand0 =
                    (GrPixelConfigIsAlphaOnly(texture->config())) ?
                        kAlpha_TextureEnvRGBOperand :
                        kColor_TextureEnvRGBOperand;
                if (fHWRGBOperand0[s] != nextRGBOperand0) {
                    setTextureUnit(s);
                    GR_GL(TexEnvi(GR_GL_TEXTURE_ENV,
                                  GR_GL_OPERAND0_RGB,
                                  (nextRGBOperand0==kAlpha_TextureEnvRGBOperand) ?
                                    GR_GL_SRC_ALPHA :
                                    GR_GL_SRC_COLOR));
                    fHWRGBOperand0[s] = nextRGBOperand0;
                }

                if (((1 << s) & fDirtyFlags.fTextureChangedMask) ||
                    (fHWDrawState.fSamplerStates[s].getMatrix() !=
                     getSamplerMatrix(s))) {

                    GrMatrix texMat = getSamplerMatrix(s);
                    AdjustTextureMatrix(texture,
                                        GrSamplerState::kNormal_SampleMode,
                                        &texMat);
                    GrGpuMatrix glm;
                    glm.set(texMat);
                    setTextureUnit(s);
                    GR_GL(MatrixMode(GR_GL_TEXTURE));
                    GR_GL(LoadMatrixf(glm.fMat));
                    recordHWSamplerMatrix(s, getSamplerMatrix(s));
                }
            } else {
                GrAssert(!"Rendering with texture vert flag set but no bound texture");
                return false;
            }
        }
    }

    if (fHWDrawState.fViewMatrix != fCurrDrawState.fViewMatrix) {
        GrGpuMatrix glm;
        glm.set(fCurrDrawState.fViewMatrix);
        GR_GL(MatrixMode(GR_GL_MODELVIEW));
        GR_GL(LoadMatrixf(glm.fMat));
        fHWDrawState.fViewMatrix =
        fCurrDrawState.fViewMatrix;
    }
    resetDirtyFlags();
    return true;
}

void GrGpuGLFixed::setupGeometry(int* startVertex,
                                 int* startIndex,
                                 int vertexCount,
                                 int indexCount) {

    int newColorOffset;
    int newTexCoordOffsets[kNumStages];

    GrGLsizei newStride = VertexSizeAndOffsetsByStage(this->getGeomSrc().fVertexLayout,
                                                      newTexCoordOffsets,
                                                      &newColorOffset);
    int oldColorOffset;
    int oldTexCoordOffsets[kNumStages];
    GrGLsizei oldStride = VertexSizeAndOffsetsByStage(fHWGeometryState.fVertexLayout,
                                                      oldTexCoordOffsets,
                                                      &oldColorOffset);

    bool indexed = NULL != startIndex;

    int extraVertexOffset;
    int extraIndexOffset;
    setBuffers(indexed, &extraVertexOffset, &extraIndexOffset);

    GrGLenum scalarType;
    if (this->getGeomSrc().fVertexLayout & kTextFormat_VertexLayoutBit) {
        scalarType = GrGLTextType;
    } else {
        scalarType = GrGLType;
    }

    size_t vertexOffset = (*startVertex + extraVertexOffset) * newStride;
    *startVertex = 0;
    if (indexed) {
        *startIndex += extraIndexOffset;
    }

    // all the Pointers must be set if any of these are true
    bool allOffsetsChange =  fHWGeometryState.fArrayPtrsDirty ||
                             vertexOffset != fHWGeometryState.fVertexOffset ||
                             newStride != oldStride;

    // position and tex coord offsets change if above conditions are true
    // or the type changed based on text vs nontext type coords.
    bool posAndTexChange = allOffsetsChange ||
                           ((GrGLTextType != GrGLType) &&
                                (kTextFormat_VertexLayoutBit &
                                  (fHWGeometryState.fVertexLayout ^
                                   this->getGeomSrc().fVertexLayout)));

    if (posAndTexChange) {
        GR_GL(VertexPointer(2, scalarType, newStride, (GrGLvoid*)vertexOffset));
        fHWGeometryState.fVertexOffset = vertexOffset;
    }

    for (int s = 0; s < kNumStages; ++s) {
        // need to enable array if tex coord offset is 0
        // (using positions as coords)
        if (newTexCoordOffsets[s] >= 0) {
            GrGLvoid* texCoordOffset = (GrGLvoid*)(vertexOffset + newTexCoordOffsets[s]);
            if (oldTexCoordOffsets[s] < 0) {
                GR_GL(ClientActiveTexture(GR_GL_TEXTURE0+s));
                GR_GL(EnableClientState(GR_GL_TEXTURE_COORD_ARRAY));
                GR_GL(TexCoordPointer(2, scalarType, newStride, texCoordOffset));
            } else if (posAndTexChange ||
                       newTexCoordOffsets[s] != oldTexCoordOffsets[s]) {
                GR_GL(ClientActiveTexture(GR_GL_TEXTURE0+s));
                GR_GL(TexCoordPointer(2, scalarType, newStride, texCoordOffset));
            }
        } else if (oldTexCoordOffsets[s] >= 0) {
            GR_GL(ClientActiveTexture(GR_GL_TEXTURE0+s));
            GR_GL(DisableClientState(GR_GL_TEXTURE_COORD_ARRAY));
        }
    }

    if (newColorOffset > 0) {
        GrGLvoid* colorOffset = (GrGLvoid*)(vertexOffset + newColorOffset);
        if (oldColorOffset <= 0) {
            GR_GL(EnableClientState(GR_GL_COLOR_ARRAY));
            GR_GL(ColorPointer(4, GR_GL_UNSIGNED_BYTE, newStride, colorOffset));
        } else if (allOffsetsChange || newColorOffset != oldColorOffset) {
            GR_GL(ColorPointer(4, GR_GL_UNSIGNED_BYTE, newStride, colorOffset));
        }
    } else if (oldColorOffset > 0) {
        GR_GL(DisableClientState(GR_GL_COLOR_ARRAY));
    }

    fHWGeometryState.fVertexLayout = this->getGeomSrc().fVertexLayout;
    fHWGeometryState.fArrayPtrsDirty = false;
}
