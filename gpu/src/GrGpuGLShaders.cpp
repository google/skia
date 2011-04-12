/*
    Copyright 2011 Google Inc.

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

#include "GrBinHashKey.h"
#include "GrGLEffect.h"
#include "GrGLProgram.h"
#include "GrGpuGLShaders.h"
#include "GrGpuVertex.h"
#include "GrMemory.h"
#include "GrNoncopyable.h"
#include "GrStringBuilder.h"

#define ATTRIBUTE_MATRIX        0
#define PRINT_SHADERS           0
#define SKIP_CACHE_CHECK    true
#define GR_UINT32_MAX   static_cast<uint32_t>(-1)

#if ATTRIBUTE_MATRIX
#define VIEWMAT_ATTR_LOCATION (3 + GrDrawTarget::kMaxTexCoords)
#define TEXMAT_ATTR_LOCATION(X) (6 + GrDrawTarget::kMaxTexCoords + 3 * (X))
#define BOGUS_MATRIX_UNI_LOCATION 1000
#endif

#include "GrTHashCache.h"

class GrGpuGLShaders::ProgramCache : public ::GrNoncopyable {
private:
    class Entry;

#if GR_DEBUG
    typedef GrBinHashKey<Entry, 4> ProgramHashKey; // Flex the dynamic allocation muscle in debug
#else
    typedef GrBinHashKey<Entry, 32> ProgramHashKey;
#endif

    class Entry : public ::GrNoncopyable {
    public:
        Entry() {}
    private:
        void copyAndTakeOwnership(Entry& entry) {
            fProgramData.copyAndTakeOwnership(entry.fProgramData);
            fKey.copyAndTakeOwnership(entry.fKey); // ownership transfer
            fLRUStamp = entry.fLRUStamp;        
        }

    public:
        int compare(const ProgramHashKey& key) const { return fKey.compare(key); }

    public:
        GrGLProgram::CachedData fProgramData;
        ProgramHashKey          fKey;
        unsigned int            fLRUStamp;
    };

    GrTHashTable<Entry, ProgramHashKey, 8> fHashCache;

    enum {
        kMaxEntries = 32
    };
    Entry        fEntries[kMaxEntries];
    int          fCount;
    unsigned int fCurrLRUStamp;

public:
    ProgramCache() 
        : fCount(0)
        , fCurrLRUStamp(0) {
    }

    ~ProgramCache() {
        for (int i = 0; i < fCount; ++i) {
            GrGpuGLShaders::DeleteProgram(&fEntries[i].fProgramData);
        }
    }

    void abandon() {
        fCount = 0;
    }

    void invalidateViewMatrices() {
        for (int i = 0; i < fCount; ++i) {
            // set to illegal matrix
            fEntries[i].fProgramData.fViewMatrix = GrMatrix::InvalidMatrix();
        }
    }

    GrGLProgram::CachedData* getProgramData(const GrGLProgram& desc, 
                                            const GrDrawTarget* target) {
        ProgramHashKey key;
        while (key.doPass()) {
            desc.buildKey(key);
        }
        Entry* entry = fHashCache.find(key);
        if (NULL == entry) {
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
                GrGpuGLShaders::DeleteProgram(&entry->fProgramData);
            }
            entry->fKey.copyAndTakeOwnership(key);
            desc.genProgram(&entry->fProgramData, target);
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
        return &entry->fProgramData;
    }
};

void GrGpuGLShaders::DeleteProgram(GrGLProgram::CachedData* programData) {
    GR_GL(DeleteShader(programData->fVShaderID));
    GR_GL(DeleteShader(programData->fFShaderID));
    GR_GL(DeleteProgram(programData->fProgramID));
    GR_DEBUGCODE(memset(programData, 0, sizeof(*programData));)
}


GrGpuGLShaders::GrGpuGLShaders() {

    resetContext();

    fProgramData = NULL;
    fProgramCache = new ProgramCache();
}

GrGpuGLShaders::~GrGpuGLShaders() {
    delete fProgramCache;
}

const GrMatrix& GrGpuGLShaders::getHWSamplerMatrix(int stage) {
#if ATTRIBUTE_MATRIX
    return fHWDrawState.fSamplerStates[stage].getMatrix();
#else
    GrAssert(fProgramData);
    return fProgramData->fTextureMatrices[stage];
#endif
}

void GrGpuGLShaders::recordHWSamplerMatrix(int stage, const GrMatrix& matrix) {
#if ATTRIBUTE_MATRIX
    fHWDrawState.fSamplerStates[stage].setMatrix(matrix);
#else
    GrAssert(fProgramData);
    fProgramData->fTextureMatrices[stage] = matrix;
#endif
}

void GrGpuGLShaders::resetContext() {
    INHERITED::resetContext();

    fHWGeometryState.fVertexLayout = 0;
    fHWGeometryState.fVertexOffset = ~0;
    GR_GL(DisableVertexAttribArray(COL_ATTR_LOCATION));
    for (int t = 0; t < kMaxTexCoords; ++t) {
        GR_GL(DisableVertexAttribArray(TEX_ATTR_LOCATION(t)));
    }
    GR_GL(EnableVertexAttribArray(POS_ATTR_LOCATION));

    fHWProgramID = 0;
}

void GrGpuGLShaders::flushViewMatrix() {
    GrAssert(NULL != fCurrDrawState.fRenderTarget);
    GrMatrix m (
        GrIntToScalar(2) / fCurrDrawState.fRenderTarget->width(), 0, -GR_Scalar1,
        0,-GrIntToScalar(2) / fCurrDrawState.fRenderTarget->height(), GR_Scalar1,
        0, 0, GrMatrix::I()[8]);
    m.setConcat(m, fCurrDrawState.fViewMatrix);

    // ES doesn't allow you to pass true to the transpose param,
    // so do our own transpose
    GrScalar mt[]  = {
        m[GrMatrix::kScaleX],
        m[GrMatrix::kSkewY],
        m[GrMatrix::kPersp0],
        m[GrMatrix::kSkewX],
        m[GrMatrix::kScaleY],
        m[GrMatrix::kPersp1],
        m[GrMatrix::kTransX],
        m[GrMatrix::kTransY],
        m[GrMatrix::kPersp2]
    };
#if ATTRIBUTE_MATRIX
    GR_GL(VertexAttrib4fv(VIEWMAT_ATTR_LOCATION+0, mt+0));
    GR_GL(VertexAttrib4fv(VIEWMAT_ATTR_LOCATION+1, mt+3));
    GR_GL(VertexAttrib4fv(VIEWMAT_ATTR_LOCATION+2, mt+6));
#else
    GR_GL(UniformMatrix3fv(fProgramData->fUniLocations.fViewMatrixUni,1,false,mt));
#endif
}

void GrGpuGLShaders::flushTextureMatrix(int stage) {
    GrAssert(NULL != fCurrDrawState.fTextures[stage]);

    GrGLTexture* texture = (GrGLTexture*) fCurrDrawState.fTextures[stage];

    GrMatrix m = getSamplerMatrix(stage);
    GrSamplerState::SampleMode mode = 
        fCurrDrawState.fSamplerStates[0].getSampleMode();
    AdjustTextureMatrix(texture, mode, &m);

    // ES doesn't allow you to pass true to the transpose param,
    // so do our own transpose
    GrScalar mt[]  = {
        m[GrMatrix::kScaleX],
        m[GrMatrix::kSkewY],
        m[GrMatrix::kPersp0],
        m[GrMatrix::kSkewX],
        m[GrMatrix::kScaleY],
        m[GrMatrix::kPersp1],
        m[GrMatrix::kTransX],
        m[GrMatrix::kTransY],
        m[GrMatrix::kPersp2]
    };
#if ATTRIBUTE_MATRIX
    GR_GL(VertexAttrib4fv(TEXMAT_ATTR_LOCATION(0)+0, mt+0));
    GR_GL(VertexAttrib4fv(TEXMAT_ATTR_LOCATION(0)+1, mt+3));
    GR_GL(VertexAttrib4fv(TEXMAT_ATTR_LOCATION(0)+2, mt+6));
#else
    GR_GL(UniformMatrix3fv(fProgramData->fUniLocations.fStages[stage].fTextureMatrixUni,
                           1, false, mt));
#endif
}

void GrGpuGLShaders::flushRadial2(int stage) {

    const GrSamplerState& sampler = fCurrDrawState.fSamplerStates[stage];

    GrScalar centerX1 = sampler.getRadial2CenterX1();
    GrScalar radius0 = sampler.getRadial2Radius0();

    GrScalar a = GrMul(centerX1, centerX1) - GR_Scalar1;

    float unis[6] = {
        GrScalarToFloat(a),
        1 / (2.f * unis[0]),
        GrScalarToFloat(centerX1),
        GrScalarToFloat(radius0),
        GrScalarToFloat(GrMul(radius0, radius0)),
        sampler.isRadial2PosRoot() ? 1.f : -1.f
    };
    GR_GL(Uniform1fv(fProgramData->fUniLocations.fStages[stage].fRadial2Uni,
                     6,
                     unis));
}

bool GrGpuGLShaders::flushGraphicsState(GrPrimitiveType type) {
    if (!flushGLStateCommon(type)) {
        return false;
    }

    if (fDirtyFlags.fRenderTargetChanged) {
        // our coords are in pixel space and the GL matrices map to NDC
        // so if the viewport changed, our matrix is now wrong.
#if ATTRIBUTE_MATRIX
        fHWDrawState.fViewMatrix = GrMatrix::InvalidMatrix();
#else
        // we assume all shader matrices may be wrong after viewport changes
        fProgramCache->invalidateViewMatrices();
#endif
    }

    if (fGeometrySrc.fVertexLayout & kColor_VertexLayoutBit) {
        // invalidate the immediate mode color
        fHWDrawState.fColor = GrColor_ILLEGAL;
    } else {
        if (fHWDrawState.fColor != fCurrDrawState.fColor) {
            // OpenGL ES only supports the float varities of glVertexAttrib
            float c[] = {
                GrColorUnpackR(fCurrDrawState.fColor) / 255.f,
                GrColorUnpackG(fCurrDrawState.fColor) / 255.f,
                GrColorUnpackB(fCurrDrawState.fColor) / 255.f,
                GrColorUnpackA(fCurrDrawState.fColor) / 255.f
            };
            GR_GL(VertexAttrib4fv(COL_ATTR_LOCATION, c));
            fHWDrawState.fColor = fCurrDrawState.fColor;
        }
    }

    buildProgram(type);
    fProgramData = fProgramCache->getProgramData(fCurrentProgram, this);

    if (fHWProgramID != fProgramData->fProgramID) {
        GR_GL(UseProgram(fProgramData->fProgramID));
        fHWProgramID = fProgramData->fProgramID;
    }

    if (!fCurrentProgram.doGLSetup(type, fProgramData)) {
        return false;
    }

#if ATTRIBUTE_MATRIX
    GrMatrix& currViewMatrix = fHWDrawState.fViewMatrix;
#else
    GrMatrix& currViewMatrix = fProgramData->fViewMatrix;
#endif

    if (currViewMatrix != fCurrDrawState.fViewMatrix) {
        flushViewMatrix();
        currViewMatrix = fCurrDrawState.fViewMatrix;
    }

    for (int s = 0; s < kNumStages; ++s) {
        GrGLTexture* texture = (GrGLTexture*) fCurrDrawState.fTextures[s];
        if (NULL != texture) {
            if (-1 != fProgramData->fUniLocations.fStages[s].fTextureMatrixUni &&
                (((1 << s) & fDirtyFlags.fTextureChangedMask) ||
                getHWSamplerMatrix(s) != getSamplerMatrix(s))) {
                flushTextureMatrix(s);
                recordHWSamplerMatrix(s, getSamplerMatrix(s));
            }
        }

        const GrSamplerState& sampler = fCurrDrawState.fSamplerStates[s];
        if (-1 != fProgramData->fUniLocations.fStages[s].fRadial2Uni &&
            (fProgramData->fRadial2CenterX1[s] != sampler.getRadial2CenterX1() ||
             fProgramData->fRadial2Radius0[s]  != sampler.getRadial2Radius0()  ||
             fProgramData->fRadial2PosRoot[s]  != sampler.isRadial2PosRoot())) {

            flushRadial2(s);

            fProgramData->fRadial2CenterX1[s] = sampler.getRadial2CenterX1();
            fProgramData->fRadial2Radius0[s]  = sampler.getRadial2Radius0();
            fProgramData->fRadial2PosRoot[s]  = sampler.isRadial2PosRoot();
        }
    }
    resetDirtyFlags();
    return true;
}

void GrGpuGLShaders::postDraw() {
    fCurrentProgram.doGLPost();
}

void GrGpuGLShaders::setupGeometry(int* startVertex,
                                    int* startIndex,
                                    int vertexCount,
                                    int indexCount) {

    int newColorOffset;
    int newTexCoordOffsets[kMaxTexCoords];

    GrGLsizei newStride = VertexSizeAndOffsetsByIdx(fGeometrySrc.fVertexLayout,
                                                  newTexCoordOffsets,
                                                  &newColorOffset);
    int oldColorOffset;
    int oldTexCoordOffsets[kMaxTexCoords];
    GrGLsizei oldStride = VertexSizeAndOffsetsByIdx(fHWGeometryState.fVertexLayout,
                                                  oldTexCoordOffsets,
                                                  &oldColorOffset);
    bool indexed = NULL != startIndex;

    int extraVertexOffset;
    int extraIndexOffset;
    setBuffers(indexed, &extraVertexOffset, &extraIndexOffset);

    GrGLenum scalarType;
    bool texCoordNorm;
    if (fGeometrySrc.fVertexLayout & kTextFormat_VertexLayoutBit) {
        scalarType = GrGLTextType;
        texCoordNorm = GR_GL_TEXT_TEXTURE_NORMALIZED;
    } else {
        scalarType = GrGLType;
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
                           (((GrGLTextType != GrGLType) || GR_GL_TEXT_TEXTURE_NORMALIZED) &&
                                (kTextFormat_VertexLayoutBit &
                                  (fHWGeometryState.fVertexLayout ^
                                   fGeometrySrc.fVertexLayout)));

    if (posAndTexChange) {
        GR_GL(VertexAttribPointer(POS_ATTR_LOCATION, 2, scalarType,
                                  false, newStride, (GrGLvoid*)vertexOffset));
        fHWGeometryState.fVertexOffset = vertexOffset;
    }

    for (int t = 0; t < kMaxTexCoords; ++t) {
        if (newTexCoordOffsets[t] > 0) {
            GrGLvoid* texCoordOffset = (GrGLvoid*)(vertexOffset + newTexCoordOffsets[t]);
            if (oldTexCoordOffsets[t] <= 0) {
                GR_GL(EnableVertexAttribArray(TEX_ATTR_LOCATION(t)));
                GR_GL(VertexAttribPointer(TEX_ATTR_LOCATION(t), 2, scalarType,
                                          texCoordNorm, newStride, texCoordOffset));
            } else if (posAndTexChange ||
                       newTexCoordOffsets[t] != oldTexCoordOffsets[t]) {
                GR_GL(VertexAttribPointer(TEX_ATTR_LOCATION(t), 2, scalarType,
                                          texCoordNorm, newStride, texCoordOffset));
            }
        } else if (oldTexCoordOffsets[t] > 0) {
            GR_GL(DisableVertexAttribArray(TEX_ATTR_LOCATION(t)));
        }
    }

    if (newColorOffset > 0) {
        GrGLvoid* colorOffset = (int8_t*)(vertexOffset + newColorOffset);
        if (oldColorOffset <= 0) {
            GR_GL(EnableVertexAttribArray(COL_ATTR_LOCATION));
            GR_GL(VertexAttribPointer(COL_ATTR_LOCATION, 4,
                                      GR_GL_UNSIGNED_BYTE,
                                      true, newStride, colorOffset));
        } else if (allOffsetsChange || newColorOffset != oldColorOffset) {
            GR_GL(VertexAttribPointer(COL_ATTR_LOCATION, 4,
                                      GR_GL_UNSIGNED_BYTE,
                                      true, newStride, colorOffset));
        }
    } else if (oldColorOffset > 0) {
        GR_GL(DisableVertexAttribArray(COL_ATTR_LOCATION));
    }

    fHWGeometryState.fVertexLayout = fGeometrySrc.fVertexLayout;
    fHWGeometryState.fArrayPtrsDirty = false;
}

void GrGpuGLShaders::buildProgram(GrPrimitiveType type) {
    // Must initialize all fields or cache will have false negatives!
    fCurrentProgram.fProgramDesc.fVertexLayout = fGeometrySrc.fVertexLayout;

    fCurrentProgram.fProgramDesc.fOptFlags = 0;
    if (kPoints_PrimitiveType != type) {
        fCurrentProgram.fProgramDesc.fOptFlags |= GrGLProgram::ProgramDesc::kNotPoints_OptFlagBit;
    }
#if GR_AGGRESSIVE_SHADER_OPTS
    if (!(fCurrentProgram.fProgramDesc.fVertexLayout & kColor_VertexLayoutBit) &&
        (0xffffffff == fCurrDrawState.fColor)) {
        fCurrentProgram.fProgramDesc.fOptFlags |= GrGLProgram::ProgramDesc::kVertexColorAllOnes_OptFlagBit;
    }
#endif

    for (int s = 0; s < kNumStages; ++s) {
        GrGLProgram::ProgramDesc::StageDesc& stage = fCurrentProgram.fProgramDesc.fStages[s];

        stage.fEnabled = VertexUsesStage(s, fGeometrySrc.fVertexLayout);

        if (stage.fEnabled) {
            GrGLTexture* texture = (GrGLTexture*) fCurrDrawState.fTextures[s];
            GrAssert(NULL != texture);
            // we matrix to invert when orientation is TopDown, so make sure
            // we aren't in that case before flagging as identity.
            if (TextureMatrixIsIdentity(texture, fCurrDrawState.fSamplerStates[s])) {
                stage.fOptFlags = GrGLProgram::ProgramDesc::StageDesc::kIdentityMatrix_OptFlagBit;
            } else if (!getSamplerMatrix(s).hasPerspective()) {
                stage.fOptFlags = GrGLProgram::ProgramDesc::StageDesc::kNoPerspective_OptFlagBit;
            } else {
                stage.fOptFlags = 0;
            }
            switch (fCurrDrawState.fSamplerStates[s].getSampleMode()) {
            case GrSamplerState::kNormal_SampleMode:
                stage.fCoordMapping = GrGLProgram::ProgramDesc::StageDesc::kIdentity_CoordMapping;
                break;
            case GrSamplerState::kRadial_SampleMode:
                stage.fCoordMapping = GrGLProgram::ProgramDesc::StageDesc::kRadialGradient_CoordMapping;
                break;
            case GrSamplerState::kRadial2_SampleMode:
                stage.fCoordMapping = GrGLProgram::ProgramDesc::StageDesc::kRadial2Gradient_CoordMapping;
                break;
            case GrSamplerState::kSweep_SampleMode:
                stage.fCoordMapping = GrGLProgram::ProgramDesc::StageDesc::kSweepGradient_CoordMapping;
                break;
            default:
                GrAssert(!"Unexpected sample mode!");
                break;
            }

            if (GrPixelConfigIsAlphaOnly(texture->config())) {
                stage.fModulation = GrGLProgram::ProgramDesc::StageDesc::kAlpha_Modulation;
            } else {
                stage.fModulation = GrGLProgram::ProgramDesc::StageDesc::kColor_Modulation;
            }

            if (fCurrDrawState.fEffects[s]) {
                fCurrentProgram.fStageEffects[s] = GrGLEffect::Create(fCurrDrawState.fEffects[s]);
            } else {
                delete fCurrentProgram.fStageEffects[s];
                fCurrentProgram.fStageEffects[s] = NULL;
            }
        } else {
            stage.fOptFlags     = 0;
            stage.fCoordMapping = (GrGLProgram::ProgramDesc::StageDesc::CoordMapping)0;
            stage.fModulation   = (GrGLProgram::ProgramDesc::StageDesc::Modulation)0;
            fCurrentProgram.fStageEffects[s] = NULL;
        }
    }
}



