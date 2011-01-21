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

#if GR_SUPPORT_GLES1 || GR_SUPPORT_GLDESKTOP

#include "GrGpuGLFixed.h"
#include "GrGpuVertex.h"

#define SKIP_CACHE_CHECK    true

struct GrGpuMatrix {
    GrScalar    fMat[16];

    void reset() {
        Gr_bzero(fMat, sizeof(fMat));
        fMat[0] = fMat[5] = fMat[10] = fMat[15] = GR_Scalar1;
    }

    void set(const GrMatrix& m) {
        Gr_bzero(fMat, sizeof(fMat));
        fMat[0]  = m[GrMatrix::kScaleX];
        fMat[4]  = m[GrMatrix::kSkewX];
        fMat[12] = m[GrMatrix::kTransX];

        fMat[1]  = m[GrMatrix::kSkewY];
        fMat[5]  = m[GrMatrix::kScaleY];
        fMat[13] = m[GrMatrix::kTransY];

        fMat[3]  = m[GrMatrix::kPersp0];
        fMat[7]  = m[GrMatrix::kPersp1];
        fMat[15] = m[GrMatrix::kPersp2];

        fMat[10] = GR_Scalar1;    // z-scale
    }
};

// these must match the order in the corresponding enum in GrGpu.h
static const GLenum gMatrixMode2Enum[] = {
    GL_MODELVIEW, GL_TEXTURE
};

///////////////////////////////////////////////////////////////////////////////

GrGpuGLFixed::GrGpuGLFixed() {
    resetContextHelper();
}

GrGpuGLFixed::~GrGpuGLFixed() {
}

void GrGpuGLFixed::resetContext() {
    INHERITED::resetContext();
    resetContextHelper();
}

void GrGpuGLFixed::resetContextHelper() {
    GR_GL(Disable(GL_TEXTURE_2D));

    for (int s = 0; s < kNumStages; ++s) {
        setTextureUnit(s);
        GR_GL(EnableClientState(GL_VERTEX_ARRAY));
        GR_GL(TexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE));
        GR_GL(TexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB,   GL_MODULATE));
        GR_GL(TexEnvi(GL_TEXTURE_ENV, GL_SRC0_RGB,      GL_TEXTURE0+s));
        GR_GL(TexEnvi(GL_TEXTURE_ENV, GL_SRC1_RGB,      GL_PREVIOUS));
        GR_GL(TexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB,  GL_SRC_COLOR));

        GR_GL(TexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_MODULATE));
        GR_GL(TexEnvi(GL_TEXTURE_ENV, GL_SRC0_ALPHA, GL_TEXTURE0+s));
        GR_GL(TexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, GL_SRC_ALPHA));
        GR_GL(TexEnvi(GL_TEXTURE_ENV, GL_SRC1_ALPHA, GL_PREVIOUS));
        GR_GL(TexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_ALPHA, GL_SRC_ALPHA));

        // color oprand0 changes between GL_SRC_COLR and GL_SRC_ALPHA depending
        // upon whether we have a (premultiplied) RGBA texture or just an ALPHA
        // texture, e.g.:
        //glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB,  GL_SRC_COLOR);
        fHWRGBOperand0[s] = (TextureEnvRGBOperands) -1;
    }

    fHWGeometryState.fVertexLayout = 0;
    fHWGeometryState.fPositionPtr  = (void*) ~0;
    GR_GL(EnableClientState(GL_VERTEX_ARRAY));
    GR_GL(DisableClientState(GL_TEXTURE_COORD_ARRAY));
    GR_GL(ShadeModel(GL_FLAT));
    GR_GL(DisableClientState(GL_COLOR_ARRAY));

    GR_GL(PointSize(1.f));

    GrGLClearErr();
    fTextVerts = false;

    fHWTextureOrientation = (GrGLTexture::Orientation)-1; // illegal
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

    GR_GL(MatrixMode(GL_PROJECTION));
    GR_GL(LoadMatrixf(mat));
}

bool GrGpuGLFixed::flushGraphicsState(PrimitiveType type) {

    bool usingTextures[kNumStages];

    for (int s = 0; s < kNumStages; ++s) {
        usingTextures[s] = VertexUsesStage(s, fGeometrySrc.fVertexLayout);

        if (usingTextures[s] && fCurrDrawState.fSamplerStates[s].isGradient()) {
            unimpl("Fixed pipe doesn't support radial/sweep gradients");
            return false;
        }
    }

    if (!flushGLStateCommon(type)) {
        return false;
    }

    if (fRenderTargetChanged) {
        flushProjectionMatrix();
        fRenderTargetChanged = false;
    }

    for (int s = 0; s < kNumStages; ++s) {
        bool wasUsingTexture = VertexUsesStage(s, fHWGeometryState.fVertexLayout);
        if (usingTextures[s] != wasUsingTexture) {
            setTextureUnit(s);
            if (usingTextures[s]) {
                GR_GL(Enable(GL_TEXTURE_2D));
            } else {
                GR_GL(Disable(GL_TEXTURE_2D));
            }
        }
    }

    uint32_t vertColor = (fGeometrySrc.fVertexLayout & kColor_VertexLayoutBit);
    uint32_t prevVertColor = (fHWGeometryState.fVertexLayout &
                              kColor_VertexLayoutBit);

    if (vertColor != prevVertColor) {
        if (vertColor) {
            GR_GL(ShadeModel(GL_SMOOTH));
            // invalidate the immediate mode color
            fHWDrawState.fColor = GrColor_ILLEGAL;
        } else {
            GR_GL(ShadeModel(GL_FLAT));
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
                    (texture->config() == GrTexture::kAlpha_8_PixelConfig) ?
                        kAlpha_TextureEnvRGBOperand :
                        kColor_TextureEnvRGBOperand;
                if (fHWRGBOperand0[s] != nextRGBOperand0) {
                    setTextureUnit(s);
                    GR_GL(TexEnvi(GL_TEXTURE_ENV,
                                  GL_OPERAND0_RGB,
                                  (nextRGBOperand0==kAlpha_TextureEnvRGBOperand) ?
                                    GL_SRC_ALPHA :
                                    GL_SRC_COLOR));
                    fHWRGBOperand0[s] = nextRGBOperand0;
                }

                if (fHWTextureOrientation != texture->orientation() ||
                    fHWDrawState.fTextureMatrices[s] !=
                    fCurrDrawState.fTextureMatrices[s]) {
                    GrGpuMatrix glm;
                    if (GrGLTexture::kBottomUp_Orientation ==
                        texture->orientation()) {
                        GrMatrix m(
                            GR_Scalar1, 0, 0,
                            0, -GR_Scalar1, GR_Scalar1,
                            0, 0, GrMatrix::I()[8]
                        );
                        m.preConcat(fCurrDrawState.fTextureMatrices[s]);
                        glm.set(m);
                    } else {
                        glm.set(fCurrDrawState.fTextureMatrices[s]);
                    }
                    setTextureUnit(s);
                    GR_GL(MatrixMode(GL_TEXTURE));
                    GR_GL(LoadMatrixf(glm.fMat));
                    fHWDrawState.fTextureMatrices[s] =
                                            fCurrDrawState.fTextureMatrices[s];
                    fHWTextureOrientation = texture->orientation();
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
        GR_GL(MatrixMode(GL_MODELVIEW));
        GR_GL(LoadMatrixf(glm.fMat));
        fHWDrawState.fViewMatrix =
        fCurrDrawState.fViewMatrix;
    }
    return true;
}

void GrGpuGLFixed::setupGeometry(uint32_t startVertex,
                                 uint32_t startIndex,
                                 uint32_t vertexCount,
                                 uint32_t indexCount) {

    int newColorOffset;
    int newTexCoordOffsets[kNumStages];

    GLsizei newStride = VertexSizeAndOffsetsByStage(fGeometrySrc.fVertexLayout,
                                                    newTexCoordOffsets,
                                                    &newColorOffset);
    int oldColorOffset;
    int oldTexCoordOffsets[kNumStages];
    GLsizei oldStride = VertexSizeAndOffsetsByStage(fHWGeometryState.fVertexLayout,
                                                    oldTexCoordOffsets,
                                                    &oldColorOffset);

    const GLvoid* posPtr = (GLvoid*)(newStride * startVertex);

    if (kBuffer_GeometrySrcType == fGeometrySrc.fVertexSrc) {
        GrAssert(NULL != fGeometrySrc.fVertexBuffer);
        GrAssert(!fGeometrySrc.fVertexBuffer->isLocked());
        if (fHWGeometryState.fVertexBuffer != fGeometrySrc.fVertexBuffer) {
            GrGLVertexBuffer* buf =
                            (GrGLVertexBuffer*)fGeometrySrc.fVertexBuffer;
            GR_GL(BindBuffer(GL_ARRAY_BUFFER, buf->bufferID()));
            fHWGeometryState.fVertexBuffer = fGeometrySrc.fVertexBuffer;
        }
    } else {
        if (kArray_GeometrySrcType == fGeometrySrc.fVertexSrc) {
            posPtr = (void*)((intptr_t)fGeometrySrc.fVertexArray +
                             (intptr_t)posPtr);
        } else {
            GrAssert(kReserved_GeometrySrcType == fGeometrySrc.fVertexSrc);
            posPtr = (void*)((intptr_t)fVertices.get() + (intptr_t)posPtr);
        }
        if (NULL != fHWGeometryState.fVertexBuffer) {
            GR_GL(BindBuffer(GL_ARRAY_BUFFER, 0));
            fHWGeometryState.fVertexBuffer = NULL;
        }
    }

    if (kBuffer_GeometrySrcType == fGeometrySrc.fIndexSrc) {
        GrAssert(NULL != fGeometrySrc.fIndexBuffer);
        GrAssert(!fGeometrySrc.fIndexBuffer->isLocked());
        if (fHWGeometryState.fIndexBuffer != fGeometrySrc.fIndexBuffer) {
            GrGLIndexBuffer* buf =
            (GrGLIndexBuffer*)fGeometrySrc.fIndexBuffer;
            GR_GL(BindBuffer(GL_ELEMENT_ARRAY_BUFFER, buf->bufferID()));
            fHWGeometryState.fIndexBuffer = fGeometrySrc.fIndexBuffer;
        }
    } else if (NULL != fHWGeometryState.fIndexBuffer) {
        GR_GL(BindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
        fHWGeometryState.fIndexBuffer = NULL;
    }

    GLenum scalarType;
    if (fGeometrySrc.fVertexLayout & kTextFormat_VertexLayoutBit) {
        scalarType = GrGLTextType;
    } else {
        scalarType = GrGLType;
    }

    bool baseChange = posPtr != fHWGeometryState.fPositionPtr;
    bool scalarChange =
        (GrGLTextType != GrGLType) &&
        (kTextFormat_VertexLayoutBit &
         (fHWGeometryState.fVertexLayout ^ fGeometrySrc.fVertexLayout));
    bool strideChange = newStride != oldStride;
    bool posChange = baseChange || scalarChange || strideChange;

    if (posChange) {
        GR_GL(VertexPointer(2, scalarType, newStride, posPtr));
        fHWGeometryState.fPositionPtr = posPtr;
    }

    for (int s = 0; s < kNumStages; ++s) {
        // need to enable array if tex coord offset is 0
        // (using positions as coords)
        if (newTexCoordOffsets[s] >= 0) {
            GLvoid* texCoordPtr = (int8_t*)posPtr + newTexCoordOffsets[s];
            if (oldTexCoordOffsets[s] < 0) {
                GR_GL(ClientActiveTexture(GL_TEXTURE0+s));
                GR_GL(EnableClientState(GL_TEXTURE_COORD_ARRAY));
            }
            if (posChange || newTexCoordOffsets[s] != oldTexCoordOffsets[s]) {
                GR_GL(ClientActiveTexture(GL_TEXTURE0+s));
                GR_GL(TexCoordPointer(2, scalarType, newStride, texCoordPtr));
            }
        } else if (oldTexCoordOffsets[s] >= 0) {
            GR_GL(ClientActiveTexture(GL_TEXTURE0+s));
            GR_GL(DisableClientState(GL_TEXTURE_COORD_ARRAY));
        }
    }

    if (newColorOffset > 0) {
        GLvoid* colorPtr = (int8_t*)posPtr + newColorOffset;
        if (oldColorOffset <= 0) {
            GR_GL(EnableClientState(GL_COLOR_ARRAY));
        }
        if (posChange || newColorOffset != oldColorOffset) {
            GR_GL(ColorPointer(4, GL_UNSIGNED_BYTE, newStride, colorPtr));
        }
    } else if (oldColorOffset > 0) {
        GR_GL(DisableClientState(GL_COLOR_ARRAY));
    }

    fHWGeometryState.fVertexLayout = fGeometrySrc.fVertexLayout;
}

#endif

