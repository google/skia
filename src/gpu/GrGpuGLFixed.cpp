
/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
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

#define GL_CALL(X) GR_GL_CALL(this->glInterface(), X)
///////////////////////////////////////////////////////////////////////////////

namespace {
GrGLBinding get_binding_in_use(const GrGLInterface* gl) {
    if (gl->supportsDesktop()) {
        return kDesktop_GrGLBinding;
    } else {
        GrAssert(gl->supportsES1());
        return kES1_GrGLBinding;
    }
}
}

GrGpuGLFixed::GrGpuGLFixed(const GrGLInterface* gl)
    : GrGpuGL(gl, get_binding_in_use(gl)) {
}

GrGpuGLFixed::~GrGpuGLFixed() {
}

void GrGpuGLFixed::resetContext() {
    INHERITED::resetContext();

    GL_CALL(Disable(GR_GL_TEXTURE_2D));

    for (int s = 0; s < GrDrawState::kNumStages; ++s) {
        setTextureUnit(s);
        GL_CALL(EnableClientState(GR_GL_VERTEX_ARRAY));
        GL_CALL(TexEnvi(GR_GL_TEXTURE_ENV,
                        GR_GL_TEXTURE_ENV_MODE,
                        GR_GL_COMBINE));
        GL_CALL(TexEnvi(GR_GL_TEXTURE_ENV,
                        GR_GL_COMBINE_RGB,
                        GR_GL_MODULATE));
        GL_CALL(TexEnvi(GR_GL_TEXTURE_ENV,
                        GR_GL_SRC0_RGB,
                        GR_GL_TEXTURE0+s));
        GL_CALL(TexEnvi(GR_GL_TEXTURE_ENV,
                        GR_GL_SRC1_RGB,
                        GR_GL_PREVIOUS));
        GL_CALL(TexEnvi(GR_GL_TEXTURE_ENV,
                        GR_GL_OPERAND1_RGB,
                        GR_GL_SRC_COLOR));

        GL_CALL(TexEnvi(GR_GL_TEXTURE_ENV,
                        GR_GL_COMBINE_ALPHA,
                        GR_GL_MODULATE));
        GL_CALL(TexEnvi(GR_GL_TEXTURE_ENV,
                        GR_GL_SRC0_ALPHA,
                        GR_GL_TEXTURE0+s));
        GL_CALL(TexEnvi(GR_GL_TEXTURE_ENV,
                        GR_GL_OPERAND0_ALPHA,
                        GR_GL_SRC_ALPHA));
        GL_CALL(TexEnvi(GR_GL_TEXTURE_ENV,
                        GR_GL_SRC1_ALPHA,
                        GR_GL_PREVIOUS));
        GL_CALL(TexEnvi(GR_GL_TEXTURE_ENV,
                        GR_GL_OPERAND1_ALPHA,
                        GR_GL_SRC_ALPHA));

        // color oprand0 changes between GL_SRC_COLR and GL_SRC_ALPHA depending
        // upon whether we have a (premultiplied) RGBA texture or just an ALPHA
        // texture, e.g.:
        //glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB,  GL_SRC_COLOR);
        fHWRGBOperand0[s] = (TextureEnvRGBOperands) -1;
    }

    fHWGeometryState.fVertexLayout = 0;
    fHWGeometryState.fVertexOffset  = ~0;
    GL_CALL(EnableClientState(GR_GL_VERTEX_ARRAY));
    GL_CALL(DisableClientState(GR_GL_TEXTURE_COORD_ARRAY));
    GL_CALL(ShadeModel(GR_GL_FLAT));
    GL_CALL(DisableClientState(GR_GL_COLOR_ARRAY));

    GL_CALL(PointSize(1.f));

    GrGLClearErr(this->glInterface());
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

    GL_CALL(MatrixMode(GR_GL_PROJECTION));
    GL_CALL(LoadMatrixf(mat));
}

bool GrGpuGLFixed::flushGraphicsState(GrPrimitiveType type) {

    bool usingTextures[GrDrawState::kNumStages];

    for (int s = 0; s < GrDrawState::kNumStages; ++s) {
        usingTextures[s] = this->isStageEnabled(s);
        if (usingTextures[s] && fCurrDrawState.fSamplerStates[s].isGradient()) {
            unimpl("Fixed pipe doesn't support radial/sweep gradients");
            return false;
        }
    }

    if (kES1_GrGLBinding == this->glBinding()) {
        if (BlendCoeffReferencesConstant(fCurrDrawState.fSrcBlend) ||
            BlendCoeffReferencesConstant(fCurrDrawState.fDstBlend)) {
            unimpl("ES1 doesn't support blend constant");
            return false;
        }
    }

    if (!flushGLStateCommon(type)) {
        return false;
    }

    GrBlendCoeff srcCoeff, dstCoeff;
    if (kSkipDraw_BlendOptFlag & 
        this->getBlendOpts(false, &srcCoeff, &dstCoeff)) {
        return false;
    }

    this->flushBlend(type, srcCoeff, dstCoeff);

    if (fDirtyFlags.fRenderTargetChanged) {
        flushProjectionMatrix();
    }

    for (int s = 0; s < GrDrawState::kNumStages; ++s) {
        bool wasUsingTexture = StageWillBeUsed(s, fHWGeometryState.fVertexLayout, fHWDrawState);
        if (usingTextures[s] != wasUsingTexture) {
            setTextureUnit(s);
            if (usingTextures[s]) {
                GL_CALL(Enable(GR_GL_TEXTURE_2D));
            } else {
                GL_CALL(Disable(GR_GL_TEXTURE_2D));
            }
        }
    }

    uint32_t vertColor = (this->getGeomSrc().fVertexLayout & kColor_VertexLayoutBit);
    uint32_t prevVertColor = (fHWGeometryState.fVertexLayout &
                              kColor_VertexLayoutBit);

    if (vertColor != prevVertColor) {
        if (vertColor) {
            GL_CALL(ShadeModel(GR_GL_SMOOTH));
            // invalidate the immediate mode color
            fHWDrawState.fColor = GrColor_ILLEGAL;
        } else {
            GL_CALL(ShadeModel(GR_GL_FLAT));
        }
    }


    if (!vertColor && fHWDrawState.fColor != fCurrDrawState.fColor) {
        GL_CALL(Color4ub(GrColorUnpackR(fCurrDrawState.fColor),
                       GrColorUnpackG(fCurrDrawState.fColor),
                       GrColorUnpackB(fCurrDrawState.fColor),
                       GrColorUnpackA(fCurrDrawState.fColor)));
        fHWDrawState.fColor = fCurrDrawState.fColor;
    }

    // set texture environment, decide whether we are modulating by RGB or A.
    for (int s = 0; s < GrDrawState::kNumStages; ++s) {
        if (usingTextures[s]) {
            GrGLTexture* texture = (GrGLTexture*)fCurrDrawState.fTextures[s];
            if (NULL != texture) {
                TextureEnvRGBOperands nextRGBOperand0 =
                    (GrPixelConfigIsAlphaOnly(texture->config())) ?
                        kAlpha_TextureEnvRGBOperand :
                        kColor_TextureEnvRGBOperand;
                if (fHWRGBOperand0[s] != nextRGBOperand0) {
                    setTextureUnit(s);
                    GL_CALL(TexEnvi(GR_GL_TEXTURE_ENV,
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
                    GL_CALL(MatrixMode(GR_GL_TEXTURE));
                    GL_CALL(LoadMatrixf(glm.fMat));
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
        GL_CALL(MatrixMode(GR_GL_MODELVIEW));
        GL_CALL(LoadMatrixf(glm.fMat));
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
    int newCoverageOffset;
    int newTexCoordOffsets[GrDrawState::kNumStages];
    int newEdgeOffset;

    GrGLsizei newStride = VertexSizeAndOffsetsByStage(this->getGeomSrc().fVertexLayout,
                                                      newTexCoordOffsets,
                                                      &newColorOffset,
                                                      &newCoverageOffset,
                                                      &newEdgeOffset);
    GrAssert(-1 == newEdgeOffset); // not supported by fixed pipe
    GrAssert(-1 == newCoverageOffset); // not supported by fixed pipe

    int oldColorOffset;
    int oldCoverageOffset;
    int oldTexCoordOffsets[GrDrawState::kNumStages];
    int oldEdgeOffset;
    GrGLsizei oldStride = VertexSizeAndOffsetsByStage(fHWGeometryState.fVertexLayout,
                                                      oldTexCoordOffsets,
                                                      &oldColorOffset,
                                                      &oldCoverageOffset,
                                                      &oldEdgeOffset);
    GrAssert(-1 == oldEdgeOffset);
    GrAssert(-1 == oldCoverageOffset);

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
        GL_CALL(VertexPointer(2, scalarType,
                              newStride, (GrGLvoid*)vertexOffset));
        fHWGeometryState.fVertexOffset = vertexOffset;
    }

    for (int s = 0; s < GrDrawState::kNumStages; ++s) {
        // need to enable array if tex coord offset is 0
        // (using positions as coords)
        if (newTexCoordOffsets[s] >= 0) {
            GrGLvoid* texCoordOffset = (GrGLvoid*)(vertexOffset +
                                                   newTexCoordOffsets[s]);
            if (oldTexCoordOffsets[s] < 0) {
                GL_CALL(ClientActiveTexture(GR_GL_TEXTURE0+s));
                GL_CALL(EnableClientState(GR_GL_TEXTURE_COORD_ARRAY));
                GL_CALL(TexCoordPointer(2, scalarType,
                                        newStride, texCoordOffset));
            } else if (posAndTexChange ||
                       newTexCoordOffsets[s] != oldTexCoordOffsets[s]) {
                GL_CALL(ClientActiveTexture(GR_GL_TEXTURE0+s));
                GL_CALL(TexCoordPointer(2, scalarType,
                                        newStride, texCoordOffset));
            }
        } else if (oldTexCoordOffsets[s] >= 0) {
            GL_CALL(ClientActiveTexture(GR_GL_TEXTURE0+s));
            GL_CALL(DisableClientState(GR_GL_TEXTURE_COORD_ARRAY));
        }
    }

    if (newColorOffset > 0) {
        GrGLvoid* colorOffset = (GrGLvoid*)(vertexOffset + newColorOffset);
        if (oldColorOffset <= 0) {
            GL_CALL(EnableClientState(GR_GL_COLOR_ARRAY));
            GL_CALL(ColorPointer(4, GR_GL_UNSIGNED_BYTE,
                                newStride, colorOffset));
        } else if (allOffsetsChange || newColorOffset != oldColorOffset) {
            GL_CALL(ColorPointer(4, GR_GL_UNSIGNED_BYTE,
                                 newStride, colorOffset));
        }
    } else if (oldColorOffset > 0) {
        GL_CALL(DisableClientState(GR_GL_COLOR_ARRAY));
    }

    fHWGeometryState.fVertexLayout = this->getGeomSrc().fVertexLayout;
    fHWGeometryState.fArrayPtrsDirty = false;
}
