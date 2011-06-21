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
#include "GrGLProgram.h"
#include "GrGpuGLShaders.h"
#include "GrGpuVertex.h"
#include "GrMemory.h"
#include "GrNoncopyable.h"
#include "GrStringBuilder.h"
#include "GrRandom.h"

#define SKIP_CACHE_CHECK    true
#define GR_UINT32_MAX   static_cast<uint32_t>(-1)

#include "GrTHashCache.h"

class GrGpuGLShaders::ProgramCache : public ::GrNoncopyable {
private:
    class Entry;

#if GR_DEBUG
    typedef GrBinHashKey<Entry, 4> ProgramHashKey; // Flex the dynamic allocation muscle in debug
#else
    typedef GrBinHashKey<Entry, 64> ProgramHashKey;
#endif

    class Entry : public ::GrNoncopyable {
    public:
        Entry() {}
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

    // We may have kMaxEntries+1 shaders in the GL context because
    // we create a new shader before evicting from the cache.
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

    GrGLProgram::CachedData* getProgramData(const GrGLProgram& desc) {
        Entry newEntry;
        while (newEntry.fKey.doPass()) {
            desc.buildKey(newEntry.fKey);
        }
        Entry* entry = fHashCache.find(newEntry.fKey);
        if (NULL == entry) {
            if (!desc.genProgram(&newEntry.fProgramData)) {
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
                GrGpuGLShaders::DeleteProgram(&entry->fProgramData);
            }
            entry->copyAndTakeOwnership(newEntry);
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

void GrGpuGLShaders::abandonResources(){
    INHERITED::abandonResources();
    fProgramCache->abandon();
}

void GrGpuGLShaders::DeleteProgram(GrGLProgram::CachedData* programData) {
    GR_GL(DeleteShader(programData->fVShaderID));
    GR_GL(DeleteShader(programData->fFShaderID));
    GR_GL(DeleteProgram(programData->fProgramID));
    GR_DEBUGCODE(memset(programData, 0, sizeof(*programData));)
}

void GrGpuGLShaders::ProgramUnitTest() {

    static const int STAGE_OPTS[] = {
        0,
        GrGLProgram::ProgramDesc::StageDesc::kNoPerspective_OptFlagBit,
        GrGLProgram::ProgramDesc::StageDesc::kIdentity_CoordMapping
    };
    static const GrGLProgram::ProgramDesc::StageDesc::Modulation STAGE_MODULATES[] = {
        GrGLProgram::ProgramDesc::StageDesc::kColor_Modulation,
        GrGLProgram::ProgramDesc::StageDesc::kAlpha_Modulation
    };
    static const GrGLProgram::ProgramDesc::StageDesc::CoordMapping STAGE_COORD_MAPPINGS[] = {
        GrGLProgram::ProgramDesc::StageDesc::kIdentity_CoordMapping,
        GrGLProgram::ProgramDesc::StageDesc::kRadialGradient_CoordMapping,
        GrGLProgram::ProgramDesc::StageDesc::kSweepGradient_CoordMapping,
        GrGLProgram::ProgramDesc::StageDesc::kRadial2Gradient_CoordMapping
    };
    static const GrGLProgram::ProgramDesc::StageDesc::FetchMode FETCH_MODES[] = {
        GrGLProgram::ProgramDesc::StageDesc::kSingle_FetchMode,
        GrGLProgram::ProgramDesc::StageDesc::k2x2_FetchMode,
    };
    GrGLProgram program;
    GrGLProgram::ProgramDesc& pdesc = program.fProgramDesc;

    static const int NUM_TESTS = 512;

    // GrRandoms nextU() values have patterns in the low bits
    // So using nextU() % array_count might never take some values.
    GrRandom random;
    for (int t = 0; t < NUM_TESTS; ++t) {

#if 0
        GrPrintf("\nTest Program %d\n-------------\n", t);
        static const int stop = -1;
        if (t == stop) {
            int breakpointhere = 9;
        }
#endif

        pdesc.fVertexLayout = 0;
        pdesc.fEmitsPointSize = random.nextF() > .5f;
        float colorType = random.nextF();
        if (colorType < 1.f / 3.f) {
            pdesc.fColorType = GrGLProgram::ProgramDesc::kAttribute_ColorType;
        } else if (colorType < 2.f / 3.f) {
            pdesc.fColorType = GrGLProgram::ProgramDesc::kUniform_ColorType;
        } else {
            pdesc.fColorType = GrGLProgram::ProgramDesc::kNone_ColorType;
        }

        int idx = (int)(random.nextF() * (SkXfermode::kCoeffModesCnt));
        pdesc.fColorFilterXfermode = (SkXfermode::Mode)idx;

        idx = (int)(random.nextF() * (kNumStages+1));
        pdesc.fFirstCoverageStage = idx;

        pdesc.fEdgeAANumEdges = (random.nextF() * (getMaxEdges() + 1));
        pdesc.fEdgeAAConcave = random.nextF() > .5f;

        if (fDualSourceBlendingSupport) {
            pdesc.fDualSrcOutput =
               (GrGLProgram::ProgramDesc::DualSrcOutput)
               (int)(random.nextF() * GrGLProgram::ProgramDesc::kDualSrcOutputCnt);
        } else {
            pdesc.fDualSrcOutput =
                                GrGLProgram::ProgramDesc::kNone_DualSrcOutput;
        }

        for (int s = 0; s < kNumStages; ++s) {
            // enable the stage?
            if (random.nextF() > .5f) {
                // use separate tex coords?
                if (random.nextF() > .5f) {
                    int t = (int)(random.nextF() * kMaxTexCoords);
                    pdesc.fVertexLayout |= StageTexCoordVertexLayoutBit(s, t);
                } else {
                    pdesc.fVertexLayout |= StagePosAsTexCoordVertexLayoutBit(s);
                }
            }
            // use text-formatted verts?
            if (random.nextF() > .5f) {
                pdesc.fVertexLayout |= kTextFormat_VertexLayoutBit;
            }
            idx = (int)(random.nextF() * GR_ARRAY_COUNT(STAGE_OPTS));
            pdesc.fStages[s].fOptFlags = STAGE_OPTS[idx];
            idx = (int)(random.nextF() * GR_ARRAY_COUNT(STAGE_MODULATES));
            pdesc.fStages[s].fModulation = STAGE_MODULATES[idx];
            idx = (int)(random.nextF() * GR_ARRAY_COUNT(STAGE_COORD_MAPPINGS));
            pdesc.fStages[s].fCoordMapping = STAGE_COORD_MAPPINGS[idx];
            idx = (int)(random.nextF() * GR_ARRAY_COUNT(FETCH_MODES));
            pdesc.fStages[s].fFetchMode = FETCH_MODES[idx];
            pdesc.fStages[s].setEnabled(VertexUsesStage(s, pdesc.fVertexLayout));
        }
        GrGLProgram::CachedData cachedData;
        program.genProgram(&cachedData);
        DeleteProgram(&cachedData);
        bool again = false;
        if (again) {
            program.genProgram(&cachedData);
            DeleteProgram(&cachedData);
        }
    }
}

GrGpuGLShaders::GrGpuGLShaders() {

    resetContext();
    int major, minor;
    gl_version(&major, &minor);

    f4X4DownsampleFilterSupport = true;
    if (GR_GL_SUPPORT_DESKTOP) {
        fDualSourceBlendingSupport =
            major > 3 ||(3 == major && 3 <= minor) ||
            has_gl_extension("GL_ARB_blend_func_extended");
    } else {
        fDualSourceBlendingSupport = false;
    }

    fProgramData = NULL;
    fProgramCache = new ProgramCache();

#if 0
    ProgramUnitTest();
#endif
}

GrGpuGLShaders::~GrGpuGLShaders() {
    delete fProgramCache;
}

const GrMatrix& GrGpuGLShaders::getHWSamplerMatrix(int stage) {
    GrAssert(fProgramData);

    if (GrGLProgram::kSetAsAttribute == 
        fProgramData->fUniLocations.fStages[stage].fTextureMatrixUni) {
        return fHWDrawState.fSamplerStates[stage].getMatrix();
    } else {
        return fProgramData->fTextureMatrices[stage];
    }
}

void GrGpuGLShaders::recordHWSamplerMatrix(int stage, const GrMatrix& matrix) {
    GrAssert(fProgramData);
    if (GrGLProgram::kSetAsAttribute == 
        fProgramData->fUniLocations.fStages[stage].fTextureMatrixUni) {
        fHWDrawState.fSamplerStates[stage].setMatrix(matrix);
    } else {
        fProgramData->fTextureMatrices[stage] = matrix;
    }
}

void GrGpuGLShaders::resetContext() {
    INHERITED::resetContext();

    fHWGeometryState.fVertexLayout = 0;
    fHWGeometryState.fVertexOffset = ~0;
    GR_GL(DisableVertexAttribArray(GrGLProgram::ColorAttributeIdx()));
    for (int t = 0; t < kMaxTexCoords; ++t) {
        GR_GL(DisableVertexAttribArray(GrGLProgram::TexCoordAttributeIdx(t)));
    }
    GR_GL(EnableVertexAttribArray(GrGLProgram::PositionAttributeIdx()));

    fHWProgramID = 0;
}

void GrGpuGLShaders::flushViewMatrix() {
    GrAssert(NULL != fCurrDrawState.fRenderTarget);
    GrMatrix m;
    m.setAll(
        GrIntToScalar(2) / fCurrDrawState.fRenderTarget->width(), 0, -GR_Scalar1,
        0,-GrIntToScalar(2) / fCurrDrawState.fRenderTarget->height(), GR_Scalar1,
        0, 0, GrMatrix::I()[8]);
    m.setConcat(m, fCurrDrawState.fViewMatrix);

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

    if (GrGLProgram::kSetAsAttribute ==  
        fProgramData->fUniLocations.fViewMatrixUni) {
        int baseIdx = GrGLProgram::ViewMatrixAttributeIdx();
        GR_GL(VertexAttrib4fv(baseIdx + 0, mt+0));
        GR_GL(VertexAttrib4fv(baseIdx + 1, mt+3));
        GR_GL(VertexAttrib4fv(baseIdx + 2, mt+6));
    } else {
        GrAssert(GrGLProgram::kUnusedUniform != 
                 fProgramData->fUniLocations.fViewMatrixUni);
        GR_GL(UniformMatrix3fv(fProgramData->fUniLocations.fViewMatrixUni,
                               1, false, mt));
    }
}

void GrGpuGLShaders::flushTextureDomain(int s) {
    const GrGLint& uni = fProgramData->fUniLocations.fStages[s].fTexDomUni;
    if (GrGLProgram::kUnusedUniform != uni) {
        const GrRect &texDom =
            fCurrDrawState.fSamplerStates[s].getTextureDomain();

        if (((1 << s) & fDirtyFlags.fTextureChangedMask) ||
            fProgramData->fTextureDomain[s] != texDom) {

            fProgramData->fTextureDomain[s] = texDom;

            float values[4] = {
                GrScalarToFloat(texDom.left()),
                GrScalarToFloat(texDom.top()),
                GrScalarToFloat(texDom.right()),
                GrScalarToFloat(texDom.bottom())
            };

            GrGLTexture* texture = (GrGLTexture*) fCurrDrawState.fTextures[s];
            GrGLTexture::Orientation orientation = texture->orientation();

            // vertical flip if necessary
            if (GrGLTexture::kBottomUp_Orientation == orientation) {
                values[1] = 1.0f - values[1];
                values[3] = 1.0f - values[3];
                // The top and bottom were just flipped, so correct the ordering
                // of elements so that values = (l, t, r, b).
                SkTSwap(values[1], values[3]);
            }

            values[0] *= SkScalarToFloat(texture->contentScaleX());
            values[2] *= SkScalarToFloat(texture->contentScaleX());
            values[1] *= SkScalarToFloat(texture->contentScaleY());
            values[3] *= SkScalarToFloat(texture->contentScaleY());

            GR_GL(Uniform4fv(uni, 1, values));
        }
    }
}

void GrGpuGLShaders::flushTextureMatrix(int s) {
    const GrGLint& uni = fProgramData->fUniLocations.fStages[s].fTextureMatrixUni;
    GrGLTexture* texture = (GrGLTexture*) fCurrDrawState.fTextures[s];
    if (NULL != texture) {
        if (GrGLProgram::kUnusedUniform != uni &&
            (((1 << s) & fDirtyFlags.fTextureChangedMask) ||
            getHWSamplerMatrix(s) != getSamplerMatrix(s))) {

            GrAssert(NULL != fCurrDrawState.fTextures[s]);

            GrGLTexture* texture = (GrGLTexture*) fCurrDrawState.fTextures[s];

            GrMatrix m = getSamplerMatrix(s);
            GrSamplerState::SampleMode mode = 
                fCurrDrawState.fSamplerStates[s].getSampleMode();
            AdjustTextureMatrix(texture, mode, &m);

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

            if (GrGLProgram::kSetAsAttribute ==
                fProgramData->fUniLocations.fStages[s].fTextureMatrixUni) {
                int baseIdx = GrGLProgram::TextureMatrixAttributeIdx(s);
                GR_GL(VertexAttrib4fv(baseIdx + 0, mt+0));
                GR_GL(VertexAttrib4fv(baseIdx + 1, mt+3));
                GR_GL(VertexAttrib4fv(baseIdx + 2, mt+6));
            } else {
                GR_GL(UniformMatrix3fv(uni, 1, false, mt));
            }
            recordHWSamplerMatrix(s, getSamplerMatrix(s));
        }
    }
}

void GrGpuGLShaders::flushRadial2(int s) {

    const int &uni = fProgramData->fUniLocations.fStages[s].fRadial2Uni;
    const GrSamplerState& sampler = fCurrDrawState.fSamplerStates[s];
    if (GrGLProgram::kUnusedUniform != uni &&
        (fProgramData->fRadial2CenterX1[s] != sampler.getRadial2CenterX1() ||
         fProgramData->fRadial2Radius0[s]  != sampler.getRadial2Radius0()  ||
         fProgramData->fRadial2PosRoot[s]  != sampler.isRadial2PosRoot())) {

        GrScalar centerX1 = sampler.getRadial2CenterX1();
        GrScalar radius0 = sampler.getRadial2Radius0();

        GrScalar a = GrMul(centerX1, centerX1) - GR_Scalar1;

        float values[6] = {
            GrScalarToFloat(a),
            1 / (2.f * values[0]),
            GrScalarToFloat(centerX1),
            GrScalarToFloat(radius0),
            GrScalarToFloat(GrMul(radius0, radius0)),
            sampler.isRadial2PosRoot() ? 1.f : -1.f
        };
        GR_GL(Uniform1fv(uni, 6, values));
        fProgramData->fRadial2CenterX1[s] = sampler.getRadial2CenterX1();
        fProgramData->fRadial2Radius0[s]  = sampler.getRadial2Radius0();
        fProgramData->fRadial2PosRoot[s]  = sampler.isRadial2PosRoot();
    }
}

void GrGpuGLShaders::flushTexelSize(int s) {
    const int& uni = fProgramData->fUniLocations.fStages[s].fNormalizedTexelSizeUni;
    if (GrGLProgram::kUnusedUniform != uni) {
        GrGLTexture* texture = (GrGLTexture*) fCurrDrawState.fTextures[s];
        if (texture->allocWidth() != fProgramData->fTextureWidth[s] ||
            texture->allocHeight() != fProgramData->fTextureWidth[s]) {

            float texelSize[] = {1.f / texture->allocWidth(),
                                 1.f / texture->allocHeight()};
            GR_GL(Uniform2fv(uni, 1, texelSize));
        }
    }
}

void GrGpuGLShaders::flushEdgeAAData() {
    const int& uni = fProgramData->fUniLocations.fEdgesUni;
    if (GrGLProgram::kUnusedUniform != uni) {
        int count = fCurrDrawState.fEdgeAANumEdges;
        Edge edges[kMaxEdges];
        // Flip the edges in Y
        float height = fCurrDrawState.fRenderTarget->height();
        for (int i = 0; i < count; ++i) {
            edges[i] = fCurrDrawState.fEdgeAAEdges[i];
            float b = edges[i].fY;
            edges[i].fY = -b;
            edges[i].fZ += b * height;
        }
        GR_GL(Uniform3fv(uni, count, &edges[0].fX));
    }
}

static const float ONE_OVER_255 = 1.f / 255.f;

#define GR_COLOR_TO_VEC4(color) {\
    GrColorUnpackR(color) * ONE_OVER_255,\
    GrColorUnpackG(color) * ONE_OVER_255,\
    GrColorUnpackB(color) * ONE_OVER_255,\
    GrColorUnpackA(color) * ONE_OVER_255 \
}

void GrGpuGLShaders::flushColor() {
    const GrGLProgram::ProgramDesc& desc = fCurrentProgram.getDesc();
    if (this->getGeomSrc().fVertexLayout & kColor_VertexLayoutBit) {
        // color will be specified per-vertex as an attribute
        // invalidate the const vertex attrib color
        fHWDrawState.fColor = GrColor_ILLEGAL;
    } else {
        switch (desc.fColorType) {
            case GrGLProgram::ProgramDesc::kAttribute_ColorType:
                if (fHWDrawState.fColor != fCurrDrawState.fColor) {
                    // OpenGL ES only supports the float varities of glVertexAttrib
                    float c[] = GR_COLOR_TO_VEC4(fCurrDrawState.fColor);
                    GR_GL(VertexAttrib4fv(GrGLProgram::ColorAttributeIdx(), c));
                    fHWDrawState.fColor = fCurrDrawState.fColor;
                }
                break;
            case GrGLProgram::ProgramDesc::kUniform_ColorType:
                if (fProgramData->fColor != fCurrDrawState.fColor) {
                    // OpenGL ES only supports the float varities of glVertexAttrib
                    float c[] = GR_COLOR_TO_VEC4(fCurrDrawState.fColor);
                    GrAssert(GrGLProgram::kUnusedUniform != 
                             fProgramData->fUniLocations.fColorUni);
                    GR_GL(Uniform4fv(fProgramData->fUniLocations.fColorUni, 1, c));
                    fProgramData->fColor = fCurrDrawState.fColor;
                }
                break;
            case GrGLProgram::ProgramDesc::kNone_ColorType:
                GrAssert(0xffffffff == fCurrDrawState.fColor);
                break;
            default:
                GrCrash("Unknown color type.");
        }
    }
    if (fProgramData->fUniLocations.fColorFilterUni
                != GrGLProgram::kUnusedUniform
            && fProgramData->fColorFilterColor
                != fCurrDrawState.fColorFilterColor) {
        float c[] = GR_COLOR_TO_VEC4(fCurrDrawState.fColorFilterColor);
        GR_GL(Uniform4fv(fProgramData->fUniLocations.fColorFilterUni, 1, c));
        fProgramData->fColorFilterColor = fCurrDrawState.fColorFilterColor;
    }
}


bool GrGpuGLShaders::flushGraphicsState(GrPrimitiveType type) {
    if (!flushGLStateCommon(type)) {
        return false;
    }

    if (fDirtyFlags.fRenderTargetChanged) {
        // our coords are in pixel space and the GL matrices map to NDC
        // so if the viewport changed, our matrix is now wrong.
        fHWDrawState.fViewMatrix = GrMatrix::InvalidMatrix();
        // we assume all shader matrices may be wrong after viewport changes
        fProgramCache->invalidateViewMatrices();
    }

    buildProgram(type);
    fProgramData = fProgramCache->getProgramData(fCurrentProgram);
    if (NULL == fProgramData) {
        GrAssert(!"Failed to create program!");
        return false;
    }

    if (fHWProgramID != fProgramData->fProgramID) {
        GR_GL(UseProgram(fProgramData->fProgramID));
        fHWProgramID = fProgramData->fProgramID;
    }
    GrBlendCoeff srcCoeff = fCurrDrawState.fSrcBlend;
    GrBlendCoeff dstCoeff = fCurrDrawState.fDstBlend;
    
    fCurrentProgram.overrideBlend(&srcCoeff, &dstCoeff);
    this->flushBlend(type, srcCoeff, dstCoeff);

    this->flushColor();

    GrMatrix* currViewMatrix;
    if (GrGLProgram::kSetAsAttribute == 
        fProgramData->fUniLocations.fViewMatrixUni) {
        currViewMatrix = &fHWDrawState.fViewMatrix;
    } else {
        currViewMatrix = &fProgramData->fViewMatrix;
    }

    if (*currViewMatrix != fCurrDrawState.fViewMatrix) {
        flushViewMatrix();
        *currViewMatrix = fCurrDrawState.fViewMatrix;
    }

    for (int s = 0; s < kNumStages; ++s) {
        this->flushTextureMatrix(s);

        this->flushRadial2(s);

        this->flushTexelSize(s);

        this->flushTextureDomain(s);
    }
    this->flushEdgeAAData();
    resetDirtyFlags();
    return true;
}

void GrGpuGLShaders::postDraw() {
}

void GrGpuGLShaders::setupGeometry(int* startVertex,
                                    int* startIndex,
                                    int vertexCount,
                                    int indexCount) {

    int newColorOffset;
    int newTexCoordOffsets[kMaxTexCoords];

    GrGLsizei newStride = VertexSizeAndOffsetsByIdx(
                                            this->getGeomSrc().fVertexLayout,
                                            newTexCoordOffsets,
                                            &newColorOffset);
    int oldColorOffset;
    int oldTexCoordOffsets[kMaxTexCoords];
    GrGLsizei oldStride = VertexSizeAndOffsetsByIdx(
                                            fHWGeometryState.fVertexLayout,
                                            oldTexCoordOffsets,
                                            &oldColorOffset);
    bool indexed = NULL != startIndex;

    int extraVertexOffset;
    int extraIndexOffset;
    this->setBuffers(indexed, &extraVertexOffset, &extraIndexOffset);

    GrGLenum scalarType;
    bool texCoordNorm;
    if (this->getGeomSrc().fVertexLayout & kTextFormat_VertexLayoutBit) {
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
                                   this->getGeomSrc().fVertexLayout)));

    if (posAndTexChange) {
        int idx = GrGLProgram::PositionAttributeIdx();
        GR_GL(VertexAttribPointer(idx, 2, scalarType, false, newStride, 
                                  (GrGLvoid*)vertexOffset));
        fHWGeometryState.fVertexOffset = vertexOffset;
    }

    for (int t = 0; t < kMaxTexCoords; ++t) {
        if (newTexCoordOffsets[t] > 0) {
            GrGLvoid* texCoordOffset = (GrGLvoid*)(vertexOffset + newTexCoordOffsets[t]);
            int idx = GrGLProgram::TexCoordAttributeIdx(t);
            if (oldTexCoordOffsets[t] <= 0) {
                GR_GL(EnableVertexAttribArray(idx));
                GR_GL(VertexAttribPointer(idx, 2, scalarType, texCoordNorm, 
                                          newStride, texCoordOffset));
            } else if (posAndTexChange ||
                       newTexCoordOffsets[t] != oldTexCoordOffsets[t]) {
                GR_GL(VertexAttribPointer(idx, 2, scalarType, texCoordNorm, 
                                          newStride, texCoordOffset));
            }
        } else if (oldTexCoordOffsets[t] > 0) {
            GR_GL(DisableVertexAttribArray(GrGLProgram::TexCoordAttributeIdx(t)));
        }
    }

    if (newColorOffset > 0) {
        GrGLvoid* colorOffset = (int8_t*)(vertexOffset + newColorOffset);
        int idx = GrGLProgram::ColorAttributeIdx();
        if (oldColorOffset <= 0) {
            GR_GL(EnableVertexAttribArray(idx));
            GR_GL(VertexAttribPointer(idx, 4, GR_GL_UNSIGNED_BYTE,
                                      true, newStride, colorOffset));
        } else if (allOffsetsChange || newColorOffset != oldColorOffset) {
            GR_GL(VertexAttribPointer(idx, 4, GR_GL_UNSIGNED_BYTE,
                                      true, newStride, colorOffset));
        }
    } else if (oldColorOffset > 0) {
        GR_GL(DisableVertexAttribArray(GrGLProgram::ColorAttributeIdx()));
    }

    fHWGeometryState.fVertexLayout = this->getGeomSrc().fVertexLayout;
    fHWGeometryState.fArrayPtrsDirty = false;
}

void GrGpuGLShaders::buildProgram(GrPrimitiveType type) {
    GrGLProgram::ProgramDesc& desc = fCurrentProgram.fProgramDesc;

    // Must initialize all fields or cache will have false negatives!
    desc.fVertexLayout = this->getGeomSrc().fVertexLayout;

    desc.fEmitsPointSize = kPoints_PrimitiveType == type;

    bool requiresAttributeColors = desc.fVertexLayout & kColor_VertexLayoutBit;
    // fColorType records how colors are specified for the program. Strip
    // the bit from the layout to avoid false negatives when searching for an
    // existing program in the cache.
    desc.fVertexLayout &= ~(kColor_VertexLayoutBit);

    desc.fColorFilterXfermode = fCurrDrawState.fColorFilterXfermode;

#if GR_AGGRESSIVE_SHADER_OPTS
    if (!requiresAttributeColors && (0xffffffff == fCurrDrawState.fColor)) {
        desc.fColorType = GrGLProgram::ProgramDesc::kNone_ColorType;
    } else
#endif
#if GR_GL_NO_CONSTANT_ATTRIBUTES
    if (!requiresAttributeColors) {
        desc.fColorType = GrGLProgram::ProgramDesc::kUniform_ColorType;
    } else
#endif
    {
        if (requiresAttributeColors) {} // suppress unused var warning
        desc.fColorType = GrGLProgram::ProgramDesc::kAttribute_ColorType;
    }

    desc.fEdgeAANumEdges = fCurrDrawState.fEdgeAANumEdges;
    desc.fEdgeAAConcave = desc.fEdgeAANumEdges > 0 && SkToBool(fCurrDrawState.fFlagBits & kEdgeAAConcave_StateBit);

    int lastEnabledStage = -1;

    for (int s = 0; s < kNumStages; ++s) {
        GrGLProgram::ProgramDesc::StageDesc& stage = desc.fStages[s];

        stage.fOptFlags = 0;
        stage.setEnabled(this->isStageEnabled(s));

        if (stage.isEnabled()) {
            lastEnabledStage = s;
            GrGLTexture* texture = (GrGLTexture*) fCurrDrawState.fTextures[s];
            GrAssert(NULL != texture);
            // we matrix to invert when orientation is TopDown, so make sure
            // we aren't in that case before flagging as identity.
            if (TextureMatrixIsIdentity(texture, fCurrDrawState.fSamplerStates[s])) {
                stage.fOptFlags |= GrGLProgram::ProgramDesc::StageDesc::kIdentityMatrix_OptFlagBit;
            } else if (!getSamplerMatrix(s).hasPerspective()) {
                stage.fOptFlags |= GrGLProgram::ProgramDesc::StageDesc::kNoPerspective_OptFlagBit;
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
                    GrCrash("Unexpected sample mode!");
                    break;
            }

            switch (fCurrDrawState.fSamplerStates[s].getFilter()) {
                // these both can use a regular texture2D()
                case GrSamplerState::kNearest_Filter:
                case GrSamplerState::kBilinear_Filter:
                    stage.fFetchMode = GrGLProgram::ProgramDesc::StageDesc::kSingle_FetchMode;
                    break;
                // performs 4 texture2D()s
                case GrSamplerState::k4x4Downsample_Filter:
                    stage.fFetchMode = GrGLProgram::ProgramDesc::StageDesc::k2x2_FetchMode;
                    break;
                default:
                    GrCrash("Unexpected filter!");
                    break;
            }

            if (fCurrDrawState.fSamplerStates[s].hasTextureDomain()) {
                GrAssert(GrSamplerState::kClamp_WrapMode ==
                    fCurrDrawState.fSamplerStates[s].getWrapX() &&
                    GrSamplerState::kClamp_WrapMode ==
                    fCurrDrawState.fSamplerStates[s].getWrapY());
                stage.fOptFlags |=
                    GrGLProgram::ProgramDesc::StageDesc::
                    kCustomTextureDomain_OptFlagBit;
            }

            if (GrPixelConfigIsAlphaOnly(texture->config())) {
                stage.fModulation = GrGLProgram::ProgramDesc::StageDesc::kAlpha_Modulation;
            } else {
                stage.fModulation = GrGLProgram::ProgramDesc::StageDesc::kColor_Modulation;
            }
        } else {
            stage.fOptFlags     = 0;
            stage.fCoordMapping = (GrGLProgram::ProgramDesc::StageDesc::CoordMapping)0;
            stage.fModulation   = (GrGLProgram::ProgramDesc::StageDesc::Modulation)0;
        }
    }

    desc.fDualSrcOutput = GrGLProgram::ProgramDesc::kNone_DualSrcOutput;
    // use canonical value when coverage/color distinction won't affect
    // generated code to prevent duplicate programs.
    desc.fFirstCoverageStage = kNumStages;
    if (fCurrDrawState.fFirstCoverageStage <= lastEnabledStage) {
        // color filter is applied between color/coverage computation
        if (SkXfermode::kDst_Mode != desc.fColorFilterXfermode) {
            desc.fFirstCoverageStage = fCurrDrawState.fFirstCoverageStage;
        }

        // We could consider cases where the final color is solid (0xff alpha)
        // and the dst coeff can correctly be set to a non-dualsrc gl value.
        // (e.g. solid draw, and dst coeff is kZero. It's correct to make
        // the dst coeff be kISA. Or solid draw with kSA can be tweaked to be
        // kOne).
        if (fDualSourceBlendingSupport) {
            if (kZero_BlendCoeff == fCurrDrawState.fDstBlend) {
                // write the coverage value to second color
                desc.fDualSrcOutput = 
                                GrGLProgram::ProgramDesc::kCoverage_DualSrcOutput;
                desc.fFirstCoverageStage = fCurrDrawState.fFirstCoverageStage;
            } else if (kSA_BlendCoeff == fCurrDrawState.fDstBlend) {
                // SA dst coeff becomes 1-(1-SA)*coverage when dst is partially 
                // cover
                desc.fDualSrcOutput = 
                            GrGLProgram::ProgramDesc::kCoverageISA_DualSrcOutput;
                desc.fFirstCoverageStage = fCurrDrawState.fFirstCoverageStage;
            } else if (kSC_BlendCoeff == fCurrDrawState.fDstBlend) {
                // SA dst coeff becomes 1-(1-SA)*coverage when dst is partially
                // cover
                desc.fDualSrcOutput = 
                        GrGLProgram::ProgramDesc::kCoverageISC_DualSrcOutput;
                desc.fFirstCoverageStage = fCurrDrawState.fFirstCoverageStage;
            }
        }
    }
}
