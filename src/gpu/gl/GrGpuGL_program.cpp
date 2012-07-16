/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrGpuGL.h"

#include "GrCustomStage.h"
#include "GrGLProgramStage.h"
#include "GrGpuVertex.h"

#define SKIP_CACHE_CHECK    true
#define GR_UINT32_MAX   static_cast<uint32_t>(-1)

GrGpuGL::ProgramCache::ProgramCache(const GrGLContextInfo& gl)
    : fCount(0)
    , fCurrLRUStamp(0)
    , fGL(gl) {
}

GrGpuGL::ProgramCache::~ProgramCache() {
    for (int i = 0; i < fCount; ++i) {
        GrGpuGL::DeleteProgram(fGL.interface(), fEntries[i].fProgram);
    }
}

void GrGpuGL::ProgramCache::abandon() {
    fCount = 0;
}

GrGLProgram* GrGpuGL::ProgramCache::getProgram(const ProgramDesc& desc, GrCustomStage** stages) {
    Entry newEntry;
    newEntry.fKey.setKeyData(desc.asKey());

    Entry* entry = fHashCache.find(newEntry.fKey);
    if (NULL == entry) {
        newEntry.fProgram.reset(SkNEW(GrGLProgram));
        if (!newEntry.fProgram->genProgram(fGL, desc, stages)) {
            return NULL;
        }
        if (fCount < kMaxEntries) {
            entry = fEntries + fCount;
            ++fCount;
        } else {
            GrAssert(kMaxEntries == fCount);
            entry = fEntries;
            for (int i = 1; i < kMaxEntries; ++i) {
                if (fEntries[i].fLRUStamp < entry->fLRUStamp) {
                    entry = fEntries + i;
                }
            }
            fHashCache.remove(entry->fKey, entry);
            GrGpuGL::DeleteProgram(fGL.interface(), entry->fProgram);
        }
        *entry = newEntry;
        fHashCache.insert(entry->fKey, entry);
    }

    entry->fLRUStamp = fCurrLRUStamp;
    if (GR_UINT32_MAX == fCurrLRUStamp) {
        // wrap around! just trash our LRU, one time hit.
        for (int i = 0; i < fCount; ++i) {
            fEntries[i].fLRUStamp = 0;
        }
    }
    ++fCurrLRUStamp;
    return entry->fProgram;
}

void GrGpuGL::DeleteProgram(const GrGLInterface* gl, GrGLProgram* program) {
    GR_GL_CALL(gl, DeleteShader(program->fVShaderID));
    if (program->fGShaderID) {
        GR_GL_CALL(gl, DeleteShader(program->fGShaderID));
    }
    GR_GL_CALL(gl, DeleteShader(program->fFShaderID));
    GR_GL_CALL(gl, DeleteProgram(program->fProgramID));
    GR_DEBUGCODE(program->fVShaderID = 0);
    GR_DEBUGCODE(program->fGShaderID = 0);
    GR_DEBUGCODE(program->fFShaderID = 0);
    GR_DEBUGCODE(program->fProgramID = 0);
}

////////////////////////////////////////////////////////////////////////////////

void GrGpuGL::abandonResources(){
    INHERITED::abandonResources();
    fProgramCache->abandon();
    fHWProgramID = 0;
}

////////////////////////////////////////////////////////////////////////////////

#define GL_CALL(X) GR_GL_CALL(this->glInterface(), X)

void GrGpuGL::flushViewMatrix(DrawType type) {
    const GrGLRenderTarget* rt = static_cast<const GrGLRenderTarget*>(this->getDrawState().getRenderTarget());
    SkISize viewportSize;
    const GrGLIRect& viewport = rt->getViewport();
    viewportSize.set(viewport.fWidth, viewport.fHeight);

    const GrMatrix& vm = this->getDrawState().getViewMatrix();

    if (kStencilPath_DrawType == type) {
        if (fHWPathMatrixState.fViewMatrix != vm ||
            fHWPathMatrixState.fRTSize != viewportSize) {
            // rescale the coords from skia's "device" coords to GL's normalized coords,
            // and perform a y-flip.
            GrMatrix m;
            m.setScale(GrIntToScalar(2) / rt->width(), GrIntToScalar(-2) / rt->height());
            m.postTranslate(-GR_Scalar1, GR_Scalar1);
            m.preConcat(vm);

            // GL wants a column-major 4x4.
            GrGLfloat mv[]  = {
                // col 0
                GrScalarToFloat(m[GrMatrix::kMScaleX]),
                GrScalarToFloat(m[GrMatrix::kMSkewY]),
                0,
                GrScalarToFloat(m[GrMatrix::kMPersp0]),

                // col 1
                GrScalarToFloat(m[GrMatrix::kMSkewX]),
                GrScalarToFloat(m[GrMatrix::kMScaleY]),
                0,
                GrScalarToFloat(m[GrMatrix::kMPersp1]),

                // col 2
                0, 0, 0, 0,

                // col3
                GrScalarToFloat(m[GrMatrix::kMTransX]),
                GrScalarToFloat(m[GrMatrix::kMTransY]),
                0.0f,
                GrScalarToFloat(m[GrMatrix::kMPersp2])
            };
            GL_CALL(MatrixMode(GR_GL_PROJECTION));
            GL_CALL(LoadMatrixf(mv));
            fHWPathMatrixState.fViewMatrix = vm;
            fHWPathMatrixState.fRTSize = viewportSize;
        }
    } else if (!fCurrentProgram->fViewMatrix.cheapEqualTo(vm) ||
               fCurrentProgram->fViewportSize != viewportSize) {
        GrMatrix m;
        m.setAll(
            GrIntToScalar(2) / viewportSize.fWidth, 0, -GR_Scalar1,
            0,-GrIntToScalar(2) / viewportSize.fHeight, GR_Scalar1,
            0, 0, GrMatrix::I()[8]);
        m.setConcat(m, vm);

        // ES doesn't allow you to pass true to the transpose param,
        // so do our own transpose
        GrGLfloat mt[]  = {
            GrScalarToFloat(m[GrMatrix::kMScaleX]),
            GrScalarToFloat(m[GrMatrix::kMSkewY]),
            GrScalarToFloat(m[GrMatrix::kMPersp0]),
            GrScalarToFloat(m[GrMatrix::kMSkewX]),
            GrScalarToFloat(m[GrMatrix::kMScaleY]),
            GrScalarToFloat(m[GrMatrix::kMPersp1]),
            GrScalarToFloat(m[GrMatrix::kMTransX]),
            GrScalarToFloat(m[GrMatrix::kMTransY]),
            GrScalarToFloat(m[GrMatrix::kMPersp2])
        };

        GrAssert(GrGLProgram::kUnusedUniform != 
                 fCurrentProgram->fUniLocations.fViewMatrixUni);
        GL_CALL(UniformMatrix3fv(fCurrentProgram->fUniLocations.fViewMatrixUni,
                                 1, false, mt));
        fCurrentProgram->fViewMatrix = vm;
        fCurrentProgram->fViewportSize = viewportSize;
    }
}

///////////////////////////////////////////////////////////////////////////////

// helpers for texture matrices

void GrGpuGL::AdjustTextureMatrix(const GrGLTexture* texture,
                                  GrMatrix* matrix) {
    GrAssert(NULL != texture);
    GrAssert(NULL != matrix);
    GrGLTexture::Orientation orientation = texture->orientation();
    if (GrGLTexture::kBottomUp_Orientation == orientation) {
        GrMatrix invY;
        invY.setAll(GR_Scalar1, 0,           0,
                    0,          -GR_Scalar1, GR_Scalar1,
                    0,          0,           GrMatrix::I()[8]);
        matrix->postConcat(invY);
    } else {
        GrAssert(GrGLTexture::kTopDown_Orientation == orientation);
    }
}

bool GrGpuGL::TextureMatrixIsIdentity(const GrGLTexture* texture,
                                      const GrSamplerState& sampler) {
    GrAssert(NULL != texture);
    if (!sampler.getMatrix().isIdentity()) {
        return false;
    }
    GrGLTexture::Orientation orientation = texture->orientation();
    if (GrGLTexture::kBottomUp_Orientation == orientation) {
        return false;
    } else {
        GrAssert(GrGLTexture::kTopDown_Orientation == orientation);
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////

void GrGpuGL::flushTextureMatrixAndDomain(int s) {
    const GrDrawState& drawState = this->getDrawState();
    const GrGLTexture* texture =
        static_cast<const GrGLTexture*>(drawState.getTexture(s));
    if (NULL != texture) {

        bool orientationChange = fCurrentProgram->fTextureOrientation[s] !=
                                 texture->orientation();

        const GrGLint& matrixUni =
            fCurrentProgram->fUniLocations.fStages[s].fTextureMatrixUni;

        const GrMatrix& hwMatrix = fCurrentProgram->fTextureMatrices[s];
        const GrMatrix& samplerMatrix = drawState.getSampler(s).getMatrix();

        if (GrGLProgram::kUnusedUniform != matrixUni &&
            (orientationChange || !hwMatrix.cheapEqualTo(samplerMatrix))) {

            GrMatrix m = samplerMatrix;
            AdjustTextureMatrix(texture, &m);

            // ES doesn't allow you to pass true to the transpose param,
            // so do our own transpose
            GrGLfloat mt[]  = {
                GrScalarToFloat(m[GrMatrix::kMScaleX]),
                GrScalarToFloat(m[GrMatrix::kMSkewY]),
                GrScalarToFloat(m[GrMatrix::kMPersp0]),
                GrScalarToFloat(m[GrMatrix::kMSkewX]),
                GrScalarToFloat(m[GrMatrix::kMScaleY]),
                GrScalarToFloat(m[GrMatrix::kMPersp1]),
                GrScalarToFloat(m[GrMatrix::kMTransX]),
                GrScalarToFloat(m[GrMatrix::kMTransY]),
                GrScalarToFloat(m[GrMatrix::kMPersp2])
            };

            GL_CALL(UniformMatrix3fv(matrixUni, 1, false, mt));
            fCurrentProgram->fTextureMatrices[s] = samplerMatrix;
        }

        const GrGLint& domUni =  fCurrentProgram->fUniLocations.fStages[s].fTexDomUni;
        const GrRect &texDom = drawState.getSampler(s).getTextureDomain();
        if (GrGLProgram::kUnusedUniform != domUni &&
            (orientationChange ||fCurrentProgram->fTextureDomain[s] != texDom)) {

            fCurrentProgram->fTextureDomain[s] = texDom;

            float values[4] = {
                GrScalarToFloat(texDom.left()),
                GrScalarToFloat(texDom.top()),
                GrScalarToFloat(texDom.right()),
                GrScalarToFloat(texDom.bottom())
            };

            // vertical flip if necessary
            if (GrGLTexture::kBottomUp_Orientation == texture->orientation()) {
                values[1] = 1.0f - values[1];
                values[3] = 1.0f - values[3];
                // The top and bottom were just flipped, so correct the ordering
                // of elements so that values = (l, t, r, b).
                SkTSwap(values[1], values[3]);
            }
            GL_CALL(Uniform4fv(domUni, 1, values));
        }
        fCurrentProgram->fTextureOrientation[s] = texture->orientation();
    }
}


void GrGpuGL::flushColorMatrix() {
    int matrixUni = fCurrentProgram->fUniLocations.fColorMatrixUni;
    int vecUni = fCurrentProgram->fUniLocations.fColorMatrixVecUni;
    if (GrGLProgram::kUnusedUniform != matrixUni
     && GrGLProgram::kUnusedUniform != vecUni) {
        const float* m = this->getDrawState().getColorMatrix();
        GrGLfloat mt[]  = {
            m[0], m[5], m[10], m[15],
            m[1], m[6], m[11], m[16],
            m[2], m[7], m[12], m[17],
            m[3], m[8], m[13], m[18],
        };
        static float scale = 1.0f / 255.0f;
        GrGLfloat vec[] = {
            m[4] * scale, m[9] * scale, m[14] * scale, m[19] * scale,
        };
        GL_CALL(UniformMatrix4fv(matrixUni, 1, false, mt));
        GL_CALL(Uniform4fv(vecUni, 1, vec));
    }
}

static const float ONE_OVER_255 = 1.f / 255.f;

#define GR_COLOR_TO_VEC4(color) {\
    GrColorUnpackR(color) * ONE_OVER_255,\
    GrColorUnpackG(color) * ONE_OVER_255,\
    GrColorUnpackB(color) * ONE_OVER_255,\
    GrColorUnpackA(color) * ONE_OVER_255 \
}

void GrGpuGL::flushColor(GrColor color) {
    const ProgramDesc& desc = fCurrentProgram->getDesc();
    const GrDrawState& drawState = this->getDrawState();

    if (this->getVertexLayout() & kColor_VertexLayoutBit) {
        // color will be specified per-vertex as an attribute
        // invalidate the const vertex attrib color
        fHWConstAttribColor = GrColor_ILLEGAL;
    } else {
        switch (desc.fColorInput) {
            case ProgramDesc::kAttribute_ColorInput:
                if (fHWConstAttribColor != color) {
                    // OpenGL ES only supports the float varieties of
                    // glVertexAttrib
                    float c[] = GR_COLOR_TO_VEC4(color);
                    GL_CALL(VertexAttrib4fv(GrGLProgram::ColorAttributeIdx(), 
                                            c));
                    fHWConstAttribColor = color;
                }
                break;
            case ProgramDesc::kUniform_ColorInput:
                if (fCurrentProgram->fColor != color) {
                    // OpenGL ES doesn't support unsigned byte varieties of
                    // glUniform
                    float c[] = GR_COLOR_TO_VEC4(color);
                    GrAssert(GrGLProgram::kUnusedUniform != 
                             fCurrentProgram->fUniLocations.fColorUni);
                    GL_CALL(Uniform4fv(fCurrentProgram->fUniLocations.fColorUni, 1, c));
                    fCurrentProgram->fColor = color;
                }
                break;
            case ProgramDesc::kSolidWhite_ColorInput:
            case ProgramDesc::kTransBlack_ColorInput:
                break;
            default:
                GrCrash("Unknown color type.");
        }
    }
    if (fCurrentProgram->fUniLocations.fColorFilterUni
                != GrGLProgram::kUnusedUniform
            && fCurrentProgram->fColorFilterColor
                != drawState.getColorFilterColor()) {
        float c[] = GR_COLOR_TO_VEC4(drawState.getColorFilterColor());
        GL_CALL(Uniform4fv(fCurrentProgram->fUniLocations.fColorFilterUni, 1, c));
        fCurrentProgram->fColorFilterColor = drawState.getColorFilterColor();
    }
}

void GrGpuGL::flushCoverage(GrColor coverage) {
    const ProgramDesc& desc = fCurrentProgram->getDesc();
    // const GrDrawState& drawState = this->getDrawState();


    if (this->getVertexLayout() & kCoverage_VertexLayoutBit) {
        // coverage will be specified per-vertex as an attribute
        // invalidate the const vertex attrib coverage
        fHWConstAttribCoverage = GrColor_ILLEGAL;
    } else {
        switch (desc.fCoverageInput) {
            case ProgramDesc::kAttribute_ColorInput:
                if (fHWConstAttribCoverage != coverage) {
                    // OpenGL ES only supports the float varieties of
                    // glVertexAttrib
                    float c[] = GR_COLOR_TO_VEC4(coverage);
                    GL_CALL(VertexAttrib4fv(GrGLProgram::CoverageAttributeIdx(), 
                                            c));
                    fHWConstAttribCoverage = coverage;
                }
                break;
            case ProgramDesc::kUniform_ColorInput:
                if (fCurrentProgram->fCoverage != coverage) {
                    // OpenGL ES doesn't support unsigned byte varieties of
                    // glUniform
                    float c[] = GR_COLOR_TO_VEC4(coverage);
                    GrAssert(GrGLProgram::kUnusedUniform != 
                             fCurrentProgram->fUniLocations.fCoverageUni);
                    GL_CALL(Uniform4fv(fCurrentProgram->fUniLocations.fCoverageUni, 1, c));
                    fCurrentProgram->fCoverage = coverage;
                }
                break;
            case ProgramDesc::kSolidWhite_ColorInput:
            case ProgramDesc::kTransBlack_ColorInput:
                break;
            default:
                GrCrash("Unknown coverage type.");
        }
    }
}

bool GrGpuGL::flushGraphicsState(DrawType type) {
    const GrDrawState& drawState = this->getDrawState();

    // GrGpu::setupClipAndFlushState should have already checked this
    // and bailed if not true.
    GrAssert(NULL != drawState.getRenderTarget());

    if (kStencilPath_DrawType != type) {
        this->flushMiscFixedFunctionState();

        GrBlendCoeff srcCoeff;
        GrBlendCoeff dstCoeff;
        BlendOptFlags blendOpts = this->getBlendOpts(false, &srcCoeff, &dstCoeff);
        if (kSkipDraw_BlendOptFlag & blendOpts) {
            return false;
        }

        GrCustomStage* customStages [GrDrawState::kNumStages];
        GrGLProgram::Desc desc;
        this->buildProgram(kDrawPoints_DrawType == type, blendOpts, dstCoeff, customStages, &desc);

        fCurrentProgram.reset(fProgramCache->getProgram(desc, customStages));
        if (NULL == fCurrentProgram.get()) {
            GrAssert(!"Failed to create program!");
            return false;
        }
        fCurrentProgram.get()->ref();

        if (fHWProgramID != fCurrentProgram->fProgramID) {
            GL_CALL(UseProgram(fCurrentProgram->fProgramID));
            fHWProgramID = fCurrentProgram->fProgramID;
        }
        fCurrentProgram->overrideBlend(&srcCoeff, &dstCoeff);
        this->flushBlend(kDrawLines_DrawType == type, srcCoeff, dstCoeff);

        GrColor color;
        GrColor coverage;
        if (blendOpts & kEmitTransBlack_BlendOptFlag) {
            color = 0;
            coverage = 0;
        } else if (blendOpts & kEmitCoverage_BlendOptFlag) {
            color = 0xffffffff;
            coverage = drawState.getCoverage();
        } else {
            color = drawState.getColor();
            coverage = drawState.getCoverage();
        }
        this->flushColor(color);
        this->flushCoverage(coverage);

        for (int s = 0; s < GrDrawState::kNumStages; ++s) {
            if (this->isStageEnabled(s)) {
#if GR_DEBUG
                // check for circular rendering
                GrAssert(NULL == drawState.getRenderTarget() ||
                         NULL == drawState.getTexture(s) ||
                         drawState.getTexture(s)->asRenderTarget() !=
                            drawState.getRenderTarget());
#endif
                this->flushBoundTextureAndParams(s);

                this->flushTextureMatrixAndDomain(s);

                if (NULL != fCurrentProgram->fProgramStage[s]) {
                    const GrSamplerState& sampler = this->getDrawState().getSampler(s);
                    const GrGLTexture* texture = static_cast<const GrGLTexture*>(
                                                    this->getDrawState().getTexture(s));
                    fCurrentProgram->fProgramStage[s]->setData(this->glInterface(),
                                                               *sampler.getCustomStage(),
                                                               drawState.getRenderTarget(), s);
                }
            }
        }
        this->flushColorMatrix();
    }
    this->flushStencil(type);
    this->flushViewMatrix(type);
    this->flushScissor();
    this->flushAAState(type);

    GrIRect* rect = NULL;
    GrIRect clipBounds;
    if (drawState.isClipState()) {
        fClip.getConservativeBounds().roundOut(&clipBounds);
        rect = &clipBounds;
    }
    // This must come after textures are flushed because a texture may need
    // to be msaa-resolved (which will modify bound FBO state).
    this->flushRenderTarget(rect);

    return true;
}

#if GR_TEXT_SCALAR_IS_USHORT
    #define TEXT_COORDS_GL_TYPE          GR_GL_UNSIGNED_SHORT
    #define TEXT_COORDS_ARE_NORMALIZED   1
#elif GR_TEXT_SCALAR_IS_FLOAT
    #define TEXT_COORDS_GL_TYPE          GR_GL_FLOAT
    #define TEXT_COORDS_ARE_NORMALIZED   0
#elif GR_TEXT_SCALAR_IS_FIXED
    #define TEXT_COORDS_GL_TYPE          GR_GL_FIXED
    #define TEXT_COORDS_ARE_NORMALIZED   0
#else
    #error "unknown GR_TEXT_SCALAR type"
#endif

void GrGpuGL::setupGeometry(int* startVertex,
                            int* startIndex,
                            int vertexCount,
                            int indexCount) {

    int newColorOffset;
    int newCoverageOffset;
    int newTexCoordOffsets[GrDrawState::kMaxTexCoords];
    int newEdgeOffset;

    GrVertexLayout currLayout = this->getVertexLayout();

    GrGLsizei newStride = VertexSizeAndOffsetsByIdx(
                                            currLayout,
                                            newTexCoordOffsets,
                                            &newColorOffset,
                                            &newCoverageOffset,
                                            &newEdgeOffset);
    int oldColorOffset;
    int oldCoverageOffset;
    int oldTexCoordOffsets[GrDrawState::kMaxTexCoords];
    int oldEdgeOffset;

    GrGLsizei oldStride = VertexSizeAndOffsetsByIdx(
                                            fHWGeometryState.fVertexLayout,
                                            oldTexCoordOffsets,
                                            &oldColorOffset,
                                            &oldCoverageOffset,
                                            &oldEdgeOffset);
    bool indexed = NULL != startIndex;

    int extraVertexOffset;
    int extraIndexOffset;
    this->setBuffers(indexed, &extraVertexOffset, &extraIndexOffset);

    GrGLenum scalarType;
    bool texCoordNorm;
    if (currLayout & kTextFormat_VertexLayoutBit) {
        scalarType = TEXT_COORDS_GL_TYPE;
        texCoordNorm = SkToBool(TEXT_COORDS_ARE_NORMALIZED);
    } else {
        GR_STATIC_ASSERT(GR_SCALAR_IS_FLOAT);
        scalarType = GR_GL_FLOAT;
        texCoordNorm = false;
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
    // or the type/normalization changed based on text vs nontext type coords.
    bool posAndTexChange = allOffsetsChange ||
                           (((TEXT_COORDS_GL_TYPE != GR_GL_FLOAT) || TEXT_COORDS_ARE_NORMALIZED) &&
                                (kTextFormat_VertexLayoutBit &
                                  (fHWGeometryState.fVertexLayout ^ currLayout)));

    if (posAndTexChange) {
        int idx = GrGLProgram::PositionAttributeIdx();
        GL_CALL(VertexAttribPointer(idx, 2, scalarType, false, newStride, 
                                  (GrGLvoid*)vertexOffset));
        fHWGeometryState.fVertexOffset = vertexOffset;
    }

    for (int t = 0; t < GrDrawState::kMaxTexCoords; ++t) {
        if (newTexCoordOffsets[t] > 0) {
            GrGLvoid* texCoordOffset = (GrGLvoid*)(vertexOffset + newTexCoordOffsets[t]);
            int idx = GrGLProgram::TexCoordAttributeIdx(t);
            if (oldTexCoordOffsets[t] <= 0) {
                GL_CALL(EnableVertexAttribArray(idx));
                GL_CALL(VertexAttribPointer(idx, 2, scalarType, texCoordNorm, 
                                          newStride, texCoordOffset));
            } else if (posAndTexChange ||
                       newTexCoordOffsets[t] != oldTexCoordOffsets[t]) {
                GL_CALL(VertexAttribPointer(idx, 2, scalarType, texCoordNorm, 
                                          newStride, texCoordOffset));
            }
        } else if (oldTexCoordOffsets[t] > 0) {
            GL_CALL(DisableVertexAttribArray(GrGLProgram::TexCoordAttributeIdx(t)));
        }
    }

    if (newColorOffset > 0) {
        GrGLvoid* colorOffset = (int8_t*)(vertexOffset + newColorOffset);
        int idx = GrGLProgram::ColorAttributeIdx();
        if (oldColorOffset <= 0) {
            GL_CALL(EnableVertexAttribArray(idx));
            GL_CALL(VertexAttribPointer(idx, 4, GR_GL_UNSIGNED_BYTE,
                                      true, newStride, colorOffset));
        } else if (allOffsetsChange || newColorOffset != oldColorOffset) {
            GL_CALL(VertexAttribPointer(idx, 4, GR_GL_UNSIGNED_BYTE,
                                      true, newStride, colorOffset));
        }
    } else if (oldColorOffset > 0) {
        GL_CALL(DisableVertexAttribArray(GrGLProgram::ColorAttributeIdx()));
    }

    if (newCoverageOffset > 0) {
        GrGLvoid* coverageOffset = (int8_t*)(vertexOffset + newCoverageOffset);
        int idx = GrGLProgram::CoverageAttributeIdx();
        if (oldCoverageOffset <= 0) {
            GL_CALL(EnableVertexAttribArray(idx));
            GL_CALL(VertexAttribPointer(idx, 4, GR_GL_UNSIGNED_BYTE,
                                        true, newStride, coverageOffset));
        } else if (allOffsetsChange || newCoverageOffset != oldCoverageOffset) {
            GL_CALL(VertexAttribPointer(idx, 4, GR_GL_UNSIGNED_BYTE,
                                        true, newStride, coverageOffset));
        }
    } else if (oldCoverageOffset > 0) {
        GL_CALL(DisableVertexAttribArray(GrGLProgram::CoverageAttributeIdx()));
    }

    if (newEdgeOffset > 0) {
        GrGLvoid* edgeOffset = (int8_t*)(vertexOffset + newEdgeOffset);
        int idx = GrGLProgram::EdgeAttributeIdx();
        if (oldEdgeOffset <= 0) {
            GL_CALL(EnableVertexAttribArray(idx));
            GL_CALL(VertexAttribPointer(idx, 4, scalarType,
                                        false, newStride, edgeOffset));
        } else if (allOffsetsChange || newEdgeOffset != oldEdgeOffset) {
            GL_CALL(VertexAttribPointer(idx, 4, scalarType,
                                        false, newStride, edgeOffset));
        }
    } else if (oldEdgeOffset > 0) {
        GL_CALL(DisableVertexAttribArray(GrGLProgram::EdgeAttributeIdx()));
    }

    fHWGeometryState.fVertexLayout = currLayout;
    fHWGeometryState.fArrayPtrsDirty = false;
}

namespace {

void setup_custom_stage(GrGLProgram::Desc::StageDesc* stage,
                        const GrSamplerState& sampler,
                        GrCustomStage** customStages,
                        GrGLProgram* program, int index) {
    GrCustomStage* customStage = sampler.getCustomStage();
    if (customStage) {
        const GrProgramStageFactory& factory = customStage->getFactory();
        stage->fCustomStageKey = factory.glStageKey(*customStage);
        customStages[index] = customStage;
    } else {
        stage->fCustomStageKey = 0;
        customStages[index] = NULL;
    }
}

}

void GrGpuGL::buildProgram(bool isPoints,
                           BlendOptFlags blendOpts,
                           GrBlendCoeff dstCoeff,
                           GrCustomStage** customStages,
                           ProgramDesc* desc) {
    const GrDrawState& drawState = this->getDrawState();

    // This should already have been caught
    GrAssert(!(kSkipDraw_BlendOptFlag & blendOpts));

    bool skipCoverage = SkToBool(blendOpts & kEmitTransBlack_BlendOptFlag);

    bool skipColor = SkToBool(blendOpts & (kEmitTransBlack_BlendOptFlag |
                                           kEmitCoverage_BlendOptFlag));

    // The descriptor is used as a cache key. Thus when a field of the
    // descriptor will not affect program generation (because of the vertex
    // layout in use or other descriptor field settings) it should be set
    // to a canonical value to avoid duplicate programs with different keys.

    // Must initialize all fields or cache will have false negatives!
    desc->fVertexLayout = this->getVertexLayout();

    desc->fEmitsPointSize = isPoints;

    bool requiresAttributeColors = 
        !skipColor && SkToBool(desc->fVertexLayout & kColor_VertexLayoutBit);
    bool requiresAttributeCoverage = 
        !skipCoverage && SkToBool(desc->fVertexLayout &
                                  kCoverage_VertexLayoutBit);

    // fColorInput/fCoverageInput records how colors are specified for the.
    // program. So we strip the bits from the layout to avoid false negatives
    // when searching for an existing program in the cache.
    desc->fVertexLayout &= ~(kColor_VertexLayoutBit | kCoverage_VertexLayoutBit);

    desc->fColorFilterXfermode = skipColor ?
                                SkXfermode::kDst_Mode :
                                drawState.getColorFilterMode();

    desc->fColorMatrixEnabled = drawState.isStateFlagEnabled(GrDrawState::kColorMatrix_StateBit);

    // no reason to do edge aa or look at per-vertex coverage if coverage is
    // ignored
    if (skipCoverage) {
        desc->fVertexLayout &= ~(kEdge_VertexLayoutBit |
                                kCoverage_VertexLayoutBit);
    }

    bool colorIsTransBlack = SkToBool(blendOpts & kEmitTransBlack_BlendOptFlag);
    bool colorIsSolidWhite = (blendOpts & kEmitCoverage_BlendOptFlag) ||
                             (!requiresAttributeColors &&
                              0xffffffff == drawState.getColor());
    if (GR_AGGRESSIVE_SHADER_OPTS && colorIsTransBlack) {
        desc->fColorInput = ProgramDesc::kTransBlack_ColorInput;
    } else if (GR_AGGRESSIVE_SHADER_OPTS && colorIsSolidWhite) {
        desc->fColorInput = ProgramDesc::kSolidWhite_ColorInput;
    } else if (GR_GL_NO_CONSTANT_ATTRIBUTES && !requiresAttributeColors) {
        desc->fColorInput = ProgramDesc::kUniform_ColorInput;
    } else {
        desc->fColorInput = ProgramDesc::kAttribute_ColorInput;
    }
    
    bool covIsSolidWhite = !requiresAttributeCoverage &&
                           0xffffffff == drawState.getCoverage();
    
    if (skipCoverage) {
        desc->fCoverageInput = ProgramDesc::kTransBlack_ColorInput;
    } else if (covIsSolidWhite) {
        desc->fCoverageInput = ProgramDesc::kSolidWhite_ColorInput;
    } else if (GR_GL_NO_CONSTANT_ATTRIBUTES && !requiresAttributeCoverage) {
        desc->fCoverageInput = ProgramDesc::kUniform_ColorInput;
    } else {
        desc->fCoverageInput = ProgramDesc::kAttribute_ColorInput;
    }

    int lastEnabledStage = -1;

    if (!skipCoverage && (desc->fVertexLayout &
                          GrDrawTarget::kEdge_VertexLayoutBit)) {
        desc->fVertexEdgeType = drawState.getVertexEdgeType();
    } else {
        // use canonical value when not set to avoid cache misses
        desc->fVertexEdgeType = GrDrawState::kHairLine_EdgeType;
    }

    for (int s = 0; s < GrDrawState::kNumStages; ++s) {
        StageDesc& stage = desc->fStages[s];

        stage.fOptFlags = 0;
        stage.setEnabled(this->isStageEnabled(s));

        bool skip = s < drawState.getFirstCoverageStage() ? skipColor :
                                                             skipCoverage;

        if (!skip && stage.isEnabled()) {
            lastEnabledStage = s;
            const GrGLTexture* texture =
                static_cast<const GrGLTexture*>(drawState.getTexture(s));
            GrAssert(NULL != texture);
            const GrSamplerState& sampler = drawState.getSampler(s);
            // we matrix to invert when orientation is TopDown, so make sure
            // we aren't in that case before flagging as identity.
            if (TextureMatrixIsIdentity(texture, sampler)) {
                stage.fOptFlags |= StageDesc::kIdentityMatrix_OptFlagBit;
            } else if (!sampler.getMatrix().hasPerspective()) {
                stage.fOptFlags |= StageDesc::kNoPerspective_OptFlagBit;
            }

            if (sampler.hasTextureDomain()) {
                GrAssert(GrSamplerState::kClamp_WrapMode ==
                            sampler.getWrapX() &&
                         GrSamplerState::kClamp_WrapMode ==
                            sampler.getWrapY());
                stage.fOptFlags |= StageDesc::kCustomTextureDomain_OptFlagBit;
            }

            stage.fInConfigFlags = 0;
            if (!this->glCaps().textureSwizzleSupport()) {
                if (GrPixelConfigIsAlphaOnly(texture->config())) {
                    // if we don't have texture swizzle support then
                    // the shader must smear the single channel after
                    // reading the texture
                    if (this->glCaps().textureRedSupport()) {
                        // we can use R8 textures so use kSmearRed
                        stage.fInConfigFlags |= 
                                        StageDesc::kSmearRed_InConfigFlag;
                    } else {
                        // we can use A8 textures so use kSmearAlpha
                        stage.fInConfigFlags |= 
                                        StageDesc::kSmearAlpha_InConfigFlag;
                    }
                } else if (sampler.swapsRAndB()) {
                    stage.fInConfigFlags |= StageDesc::kSwapRAndB_InConfigFlag;
                }
            }
            if (GrPixelConfigIsUnpremultiplied(texture->config())) {
                // The shader generator assumes that color channels are bytes
                // when rounding.
                GrAssert(4 == GrBytesPerPixel(texture->config()));
                if (kUpOnWrite_DownOnRead_UnpremulConversion ==
                    fUnpremulConversion) {
                    stage.fInConfigFlags |=
                        StageDesc::kMulRGBByAlpha_RoundDown_InConfigFlag;
                } else {
                    stage.fInConfigFlags |=
                        StageDesc::kMulRGBByAlpha_RoundUp_InConfigFlag;
                }
            }

            setup_custom_stage(&stage, sampler, customStages, fCurrentProgram.get(), s);

        } else {
            stage.fOptFlags         = 0;
            stage.fInConfigFlags    = 0;
            stage.fCustomStageKey   = 0;
            customStages[s] = NULL;
        }
    }

    if (GrPixelConfigIsUnpremultiplied(drawState.getRenderTarget()->config())) {
        // The shader generator assumes that color channels are bytes
        // when rounding.
        GrAssert(4 == GrBytesPerPixel(drawState.getRenderTarget()->config()));
        if (kUpOnWrite_DownOnRead_UnpremulConversion == fUnpremulConversion) {
            desc->fOutputConfig =
                ProgramDesc::kUnpremultiplied_RoundUp_OutputConfig;
        } else {
            desc->fOutputConfig =
                ProgramDesc::kUnpremultiplied_RoundDown_OutputConfig;
        }
    } else {
        desc->fOutputConfig = ProgramDesc::kPremultiplied_OutputConfig;
    }

    desc->fDualSrcOutput = ProgramDesc::kNone_DualSrcOutput;

    // currently the experimental GS will only work with triangle prims
    // (and it doesn't do anything other than pass through values from
    // the VS to the FS anyway).
#if 0 && GR_GL_EXPERIMENTAL_GS
    desc->fExperimentalGS = this->getCaps().fGeometryShaderSupport;
#endif

    // we want to avoid generating programs with different "first cov stage"
    // values when they would compute the same result.
    // We set field in the desc to kNumStages when either there are no 
    // coverage stages or the distinction between coverage and color is
    // immaterial.
    int firstCoverageStage = GrDrawState::kNumStages;
    desc->fFirstCoverageStage = GrDrawState::kNumStages;
    bool hasCoverage = drawState.getFirstCoverageStage() <= lastEnabledStage;
    if (hasCoverage) {
        firstCoverageStage = drawState.getFirstCoverageStage();
    }

    // other coverage inputs
    if (!hasCoverage) {
        hasCoverage =
               requiresAttributeCoverage ||
               (desc->fVertexLayout & GrDrawTarget::kEdge_VertexLayoutBit);
    }

    if (hasCoverage) {
        // color filter is applied between color/coverage computation
        if (SkXfermode::kDst_Mode != desc->fColorFilterXfermode) {
            desc->fFirstCoverageStage = firstCoverageStage;
        }

        if (this->getCaps().fDualSourceBlendingSupport &&
            !(blendOpts & (kEmitCoverage_BlendOptFlag |
                           kCoverageAsAlpha_BlendOptFlag))) {
            if (kZero_GrBlendCoeff == dstCoeff) {
                // write the coverage value to second color
                desc->fDualSrcOutput =  ProgramDesc::kCoverage_DualSrcOutput;
                desc->fFirstCoverageStage = firstCoverageStage;
            } else if (kSA_GrBlendCoeff == dstCoeff) {
                // SA dst coeff becomes 1-(1-SA)*coverage when dst is partially 
                // cover
                desc->fDualSrcOutput = ProgramDesc::kCoverageISA_DualSrcOutput;
                desc->fFirstCoverageStage = firstCoverageStage;
            } else if (kSC_GrBlendCoeff == dstCoeff) {
                // SA dst coeff becomes 1-(1-SA)*coverage when dst is partially
                // cover
                desc->fDualSrcOutput = ProgramDesc::kCoverageISC_DualSrcOutput;
                desc->fFirstCoverageStage = firstCoverageStage;
            }
        }
    }
}
