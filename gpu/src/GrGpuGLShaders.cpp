
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "GrBinHashKey.h"
#include "GrGLProgram.h"
#include "GrGpuGLShaders.h"
#include "GrGpuVertex.h"
#include "GrNoncopyable.h"
#include "GrStringBuilder.h"
#include "GrRandom.h"

#define SKIP_CACHE_CHECK    true
#define GR_UINT32_MAX   static_cast<uint32_t>(-1)

#include "GrTHashCache.h"

class GrGpuGLShaders::ProgramCache : public ::GrNoncopyable {
private:
    class Entry;

    typedef GrBinHashKey<Entry, GrGLProgram::kProgramKeySize> ProgramHashKey;

    class Entry : public ::GrNoncopyable {
    public:
        Entry() {}
        void copyAndTakeOwnership(Entry& entry) {
            fProgramData.copyAndTakeOwnership(entry.fProgramData);
            fKey = entry.fKey; // ownership transfer
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
    Entry                       fEntries[kMaxEntries];
    int                         fCount;
    unsigned int                fCurrLRUStamp;
    const GrGLInterface*        fGL;
    GrGLProgram::GLSLVersion    fGLSLVersion;

public:
    ProgramCache(const GrGLInterface* gl,
                 GrGLProgram::GLSLVersion glslVersion) 
        : fCount(0)
        , fCurrLRUStamp(0)
        , fGL(gl)
        , fGLSLVersion(glslVersion) {
    }

    ~ProgramCache() {
        for (int i = 0; i < fCount; ++i) {
            GrGpuGLShaders::DeleteProgram(fGL, &fEntries[i].fProgramData);
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
        newEntry.fKey.setKeyData(desc.keyData());
        
        Entry* entry = fHashCache.find(newEntry.fKey);
        if (NULL == entry) {
            if (!desc.genProgram(fGL, fGLSLVersion, &newEntry.fProgramData)) {
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
                GrGpuGLShaders::DeleteProgram(fGL, &entry->fProgramData);
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

void GrGpuGLShaders::DeleteProgram(const GrGLInterface* gl,
                                   CachedData* programData) {
    GR_GL_CALL(gl, DeleteShader(programData->fVShaderID));
    GR_GL_CALL(gl, DeleteShader(programData->fFShaderID));
    GR_GL_CALL(gl, DeleteProgram(programData->fProgramID));
    GR_DEBUGCODE(memset(programData, 0, sizeof(*programData));)
}

////////////////////////////////////////////////////////////////////////////////

#define GL_CALL(X) GR_GL_CALL(this->glInterface(), X)

namespace {

GrGLProgram::GLSLVersion get_glsl_version(GrGLBinding binding, GrGLVersion glVersion) {
    switch (binding) {
        case kDesktop_GrGLBinding:
            // TODO: proper check of the glsl version string
            return (glVersion >= GR_GL_VER(3,0)) ? 
                                                GrGLProgram::k130_GLSLVersion :
                                                GrGLProgram::k120_GLSLVersion;
        case kES2_GrGLBinding:
            return GrGLProgram::k120_GLSLVersion;
        default:
            GrCrash("Attempting to get GLSL version in unknown or fixed-"
                     "function GL binding.");
            return GrGLProgram::k120_GLSLVersion; // suppress warning
    }
}

template <typename T>
T random_val(GrRandom* r, T count) {
    return (T)(int)(r->nextF() * count);
}

}

bool GrGpuGLShaders::programUnitTest() {

    GrGLProgram::GLSLVersion glslVersion = 
            get_glsl_version(this->glBinding(), this->glVersion());
    static const int STAGE_OPTS[] = {
        0,
        StageDesc::kNoPerspective_OptFlagBit,
        StageDesc::kIdentity_CoordMapping
    };
    GrGLProgram program;
    ProgramDesc& pdesc = program.fProgramDesc;

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
            pdesc.fColorType = ProgramDesc::kAttribute_ColorType;
        } else if (colorType < 2.f / 3.f) {
            pdesc.fColorType = ProgramDesc::kUniform_ColorType;
        } else {
            pdesc.fColorType = ProgramDesc::kNone_ColorType;
        }

        int idx = (int)(random.nextF() * (SkXfermode::kCoeffModesCnt));
        pdesc.fColorFilterXfermode = (SkXfermode::Mode)idx;

        idx = (int)(random.nextF() * (kNumStages+1));
        pdesc.fFirstCoverageStage = idx;

        bool edgeAA = random.nextF() > .5f;
        if (edgeAA) {
            bool vertexEdgeAA = random.nextF() > .5f;
            if (vertexEdgeAA) {
                pdesc.fVertexLayout |= GrDrawTarget::kEdge_VertexLayoutBit;
                if (this->getCaps().fShaderDerivativeSupport) {
                    pdesc.fVertexEdgeType = random.nextF() > 0.5f ?
                                                        kHairQuad_EdgeType :
                                                        kHairLine_EdgeType;
                } else {
                    pdesc.fVertexEdgeType = kHairLine_EdgeType;
                }
                pdesc.fEdgeAANumEdges = 0;
            } else {
                pdesc.fEdgeAANumEdges = random.nextF() * this->getMaxEdges() + 1;
                pdesc.fEdgeAAConcave = random.nextF() > .5f;
            }
        } else {
            pdesc.fEdgeAANumEdges = 0;
        }

        if (this->getCaps().fDualSourceBlendingSupport) {
            pdesc.fDualSrcOutput =
               (ProgramDesc::DualSrcOutput)
               (int)(random.nextF() * ProgramDesc::kDualSrcOutputCnt);
        } else {
            pdesc.fDualSrcOutput = ProgramDesc::kNone_DualSrcOutput;
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
            StageDesc& stage = pdesc.fStages[s];
            stage.fOptFlags = STAGE_OPTS[idx];
            stage.fModulation = random_val(&random, StageDesc::kModulationCnt);
            stage.fCoordMapping =  random_val(&random, StageDesc::kCoordMappingCnt);
            stage.fFetchMode = random_val(&random, StageDesc::kFetchModeCnt);
            // convolution shaders don't work with persp tex matrix
            if (stage.fFetchMode == StageDesc::kConvolution_FetchMode) {
                stage.fOptFlags |= StageDesc::kNoPerspective_OptFlagBit;
            }
            stage.setEnabled(VertexUsesStage(s, pdesc.fVertexLayout));
            stage.fKernelWidth = 4 * random.nextF() + 2;
        }
        CachedData cachedData;
        if (!program.genProgram(this->glInterface(),
                                glslVersion,
                                &cachedData)) {
            return false;
        }
        DeleteProgram(this->glInterface(), &cachedData);
    }
    return true;
}

namespace {
GrGLBinding get_binding_in_use(const GrGLInterface* gl) {
    if (gl->supportsDesktop()) {
        return kDesktop_GrGLBinding;
    } else {
        GrAssert(gl->supportsES2());
        return kES2_GrGLBinding;
    }
}
}

GrGpuGLShaders::GrGpuGLShaders(const GrGLInterface* gl)
    : GrGpuGL(gl, get_binding_in_use(gl)) {

    fCaps.fShaderSupport = true;
    if (kDesktop_GrGLBinding == this->glBinding()) {
        fCaps.fDualSourceBlendingSupport =
                            this->glVersion() >= GR_GL_VER(3,3) ||
                            this->hasExtension("GL_ARB_blend_func_extended");
        fCaps.fShaderDerivativeSupport = true;
    } else {
        fCaps.fDualSourceBlendingSupport = false;
        fCaps.fShaderDerivativeSupport =
                            this->hasExtension("GL_OES_standard_derivatives");
    }

    fProgramData = NULL;
    GrGLProgram::GLSLVersion glslVersion =
        get_glsl_version(this->glBinding(), this->glVersion());
    fProgramCache = new ProgramCache(gl, glslVersion);

#if 0
    this->programUnitTest();
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
    GL_CALL(DisableVertexAttribArray(GrGLProgram::ColorAttributeIdx()));
    GL_CALL(DisableVertexAttribArray(GrGLProgram::EdgeAttributeIdx()));
    for (int t = 0; t < kMaxTexCoords; ++t) {
        GL_CALL(DisableVertexAttribArray(GrGLProgram::TexCoordAttributeIdx(t)));
    }
    GL_CALL(EnableVertexAttribArray(GrGLProgram::PositionAttributeIdx()));

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
        GL_CALL(VertexAttrib4fv(baseIdx + 0, mt+0));
        GL_CALL(VertexAttrib4fv(baseIdx + 1, mt+3));
        GL_CALL(VertexAttrib4fv(baseIdx + 2, mt+6));
    } else {
        GrAssert(GrGLProgram::kUnusedUniform != 
                 fProgramData->fUniLocations.fViewMatrixUni);
        GL_CALL(UniformMatrix3fv(fProgramData->fUniLocations.fViewMatrixUni,
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

            GL_CALL(Uniform4fv(uni, 1, values));
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
                GL_CALL(VertexAttrib4fv(baseIdx + 0, mt+0));
                GL_CALL(VertexAttrib4fv(baseIdx + 1, mt+3));
                GL_CALL(VertexAttrib4fv(baseIdx + 2, mt+6));
            } else {
                GL_CALL(UniformMatrix3fv(uni, 1, false, mt));
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

        // when were in the degenerate (linear) case the second
        // value will be INF but the program doesn't read it. (We
        // use the same 6 uniforms even though we don't need them
        // all in the linear case just to keep the code complexity
        // down).
        float values[6] = {
            GrScalarToFloat(a),
            1 / (2.f * values[0]),
            GrScalarToFloat(centerX1),
            GrScalarToFloat(radius0),
            GrScalarToFloat(GrMul(radius0, radius0)),
            sampler.isRadial2PosRoot() ? 1.f : -1.f
        };
        GL_CALL(Uniform1fv(uni, 6, values));
        fProgramData->fRadial2CenterX1[s] = sampler.getRadial2CenterX1();
        fProgramData->fRadial2Radius0[s]  = sampler.getRadial2Radius0();
        fProgramData->fRadial2PosRoot[s]  = sampler.isRadial2PosRoot();
    }
}

void GrGpuGLShaders::flushConvolution(int s) {
    const GrSamplerState& sampler = fCurrDrawState.fSamplerStates[s];
    int kernelUni = fProgramData->fUniLocations.fStages[s].fKernelUni;
    if (GrGLProgram::kUnusedUniform != kernelUni) {
        GL_CALL(Uniform1fv(kernelUni, sampler.getKernelWidth(),
                           sampler.getKernel()));
    }
    int imageIncrementUni = fProgramData->fUniLocations.fStages[s].fImageIncrementUni;
    if (GrGLProgram::kUnusedUniform != imageIncrementUni) {
        GL_CALL(Uniform2fv(imageIncrementUni, 1, sampler.getImageIncrement()));
    }
}

void GrGpuGLShaders::flushTexelSize(int s) {
    const int& uni = fProgramData->fUniLocations.fStages[s].fNormalizedTexelSizeUni;
    if (GrGLProgram::kUnusedUniform != uni) {
        GrGLTexture* texture = (GrGLTexture*) fCurrDrawState.fTextures[s];
        if (texture->allocatedWidth() != fProgramData->fTextureWidth[s] ||
            texture->allocatedHeight() != fProgramData->fTextureWidth[s]) {

            float texelSize[] = {1.f / texture->allocatedWidth(),
                                 1.f / texture->allocatedHeight()};
            GL_CALL(Uniform2fv(uni, 1, texelSize));
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
        GL_CALL(Uniform3fv(uni, count, &edges[0].fX));
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
    const ProgramDesc& desc = fCurrentProgram.getDesc();
    if (this->getGeomSrc().fVertexLayout & kColor_VertexLayoutBit) {
        // color will be specified per-vertex as an attribute
        // invalidate the const vertex attrib color
        fHWDrawState.fColor = GrColor_ILLEGAL;
    } else {
        switch (desc.fColorType) {
            case ProgramDesc::kAttribute_ColorType:
                if (fHWDrawState.fColor != fCurrDrawState.fColor) {
                    // OpenGL ES only supports the float varities of glVertexAttrib
                    float c[] = GR_COLOR_TO_VEC4(fCurrDrawState.fColor);
                    GL_CALL(VertexAttrib4fv(GrGLProgram::ColorAttributeIdx(), 
                                            c));
                    fHWDrawState.fColor = fCurrDrawState.fColor;
                }
                break;
            case ProgramDesc::kUniform_ColorType:
                if (fProgramData->fColor != fCurrDrawState.fColor) {
                    // OpenGL ES only supports the float varities of glVertexAttrib
                    float c[] = GR_COLOR_TO_VEC4(fCurrDrawState.fColor);
                    GrAssert(GrGLProgram::kUnusedUniform != 
                             fProgramData->fUniLocations.fColorUni);
                    GL_CALL(Uniform4fv(fProgramData->fUniLocations.fColorUni,
                                        1, c));
                    fProgramData->fColor = fCurrDrawState.fColor;
                }
                break;
            case ProgramDesc::kNone_ColorType:
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
        GL_CALL(Uniform4fv(fProgramData->fUniLocations.fColorFilterUni, 1, c));
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
        GL_CALL(UseProgram(fProgramData->fProgramID));
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

        this->flushConvolution(s);

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
    int newEdgeOffset;

    GrGLsizei newStride = VertexSizeAndOffsetsByIdx(
                                            this->getGeomSrc().fVertexLayout,
                                            newTexCoordOffsets,
                                            &newColorOffset,
                                            &newEdgeOffset);
    int oldColorOffset;
    int oldTexCoordOffsets[kMaxTexCoords];
    int oldEdgeOffset;

    GrGLsizei oldStride = VertexSizeAndOffsetsByIdx(
                                            fHWGeometryState.fVertexLayout,
                                            oldTexCoordOffsets,
                                            &oldColorOffset,
                                            &oldEdgeOffset);
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
        GL_CALL(VertexAttribPointer(idx, 2, scalarType, false, newStride, 
                                  (GrGLvoid*)vertexOffset));
        fHWGeometryState.fVertexOffset = vertexOffset;
    }

    for (int t = 0; t < kMaxTexCoords; ++t) {
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

    fHWGeometryState.fVertexLayout = this->getGeomSrc().fVertexLayout;
    fHWGeometryState.fArrayPtrsDirty = false;
}

void GrGpuGLShaders::buildProgram(GrPrimitiveType type) {
    ProgramDesc& desc = fCurrentProgram.fProgramDesc;

    // The descriptor is used as a cache key. Thus when a field of the
    // descriptor will not affect program generation (because of the vertex
    // layout in use or other descriptor field settings) it should be set
    // to a canonical value to avoid duplicate programs with different keys.

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
        desc.fColorType = ProgramDesc::kNone_ColorType;
    } else
#endif
#if GR_GL_NO_CONSTANT_ATTRIBUTES
    if (!requiresAttributeColors) {
        desc.fColorType = ProgramDesc::kUniform_ColorType;
    } else
#endif
    {
        if (requiresAttributeColors) {} // suppress unused var warning
        desc.fColorType = ProgramDesc::kAttribute_ColorType;
    }

    desc.fEdgeAANumEdges = fCurrDrawState.fEdgeAANumEdges;
    desc.fEdgeAAConcave = desc.fEdgeAANumEdges > 0 && SkToBool(fCurrDrawState.fFlagBits & kEdgeAAConcave_StateBit);

    int lastEnabledStage = -1;

    if (desc.fVertexLayout & GrDrawTarget::kEdge_VertexLayoutBit) {
        desc.fVertexEdgeType = fCurrDrawState.fVertexEdgeType;
    } else {
        // use canonical value when not set to avoid cache misses
        desc.fVertexEdgeType = GrDrawTarget::kHairLine_EdgeType;
    }

    for (int s = 0; s < kNumStages; ++s) {
        StageDesc& stage = desc.fStages[s];

        stage.fOptFlags = 0;
        stage.setEnabled(this->isStageEnabled(s));

        if (stage.isEnabled()) {
            lastEnabledStage = s;
            GrGLTexture* texture = (GrGLTexture*) fCurrDrawState.fTextures[s];
            GrAssert(NULL != texture);
            const GrSamplerState& sampler = fCurrDrawState.fSamplerStates[s];
            // we matrix to invert when orientation is TopDown, so make sure
            // we aren't in that case before flagging as identity.
            if (TextureMatrixIsIdentity(texture, sampler)) {
                stage.fOptFlags |= StageDesc::kIdentityMatrix_OptFlagBit;
            } else if (!getSamplerMatrix(s).hasPerspective()) {
                stage.fOptFlags |= StageDesc::kNoPerspective_OptFlagBit;
            }
            switch (sampler.getSampleMode()) {
                case GrSamplerState::kNormal_SampleMode:
                    stage.fCoordMapping = StageDesc::kIdentity_CoordMapping;
                    break;
                case GrSamplerState::kRadial_SampleMode:
                    stage.fCoordMapping = StageDesc::kRadialGradient_CoordMapping;
                    break;
                case GrSamplerState::kRadial2_SampleMode:
                    if (sampler.radial2IsDegenerate()) {
                        stage.fCoordMapping =
                            StageDesc::kRadial2GradientDegenerate_CoordMapping;
                    } else {
                        stage.fCoordMapping =
                            StageDesc::kRadial2Gradient_CoordMapping;
                    }
                    break;
                case GrSamplerState::kSweep_SampleMode:
                    stage.fCoordMapping = StageDesc::kSweepGradient_CoordMapping;
                    break;
                default:
                    GrCrash("Unexpected sample mode!");
                    break;
            }

            switch (sampler.getFilter()) {
                // these both can use a regular texture2D()
                case GrSamplerState::kNearest_Filter:
                case GrSamplerState::kBilinear_Filter:
                    stage.fFetchMode = StageDesc::kSingle_FetchMode;
                    break;
                // performs 4 texture2D()s
                case GrSamplerState::k4x4Downsample_Filter:
                    stage.fFetchMode = StageDesc::k2x2_FetchMode;
                    break;
                // performs fKernelWidth texture2D()s
                case GrSamplerState::kConvolution_Filter:
                    stage.fFetchMode = StageDesc::kConvolution_FetchMode;
                    break;
                default:
                    GrCrash("Unexpected filter!");
                    break;
            }

            if (sampler.hasTextureDomain()) {
                GrAssert(GrSamplerState::kClamp_WrapMode ==
                            sampler.getWrapX() &&
                         GrSamplerState::kClamp_WrapMode ==
                            sampler.getWrapY());
                stage.fOptFlags |= StageDesc::kCustomTextureDomain_OptFlagBit;
            }

            if (GrPixelConfigIsAlphaOnly(texture->config())) {
                stage.fModulation = StageDesc::kAlpha_Modulation;
            } else {
                stage.fModulation = StageDesc::kColor_Modulation;
            }
            if (sampler.getFilter() == GrSamplerState::kConvolution_Filter) {
                stage.fKernelWidth = sampler.getKernelWidth();
            } else {
                stage.fKernelWidth = 0;
            }
        } else {
            stage.fOptFlags     = 0;
            stage.fCoordMapping = (StageDesc::CoordMapping)0;
            stage.fModulation   = (StageDesc::Modulation)0;
        }
    }

    desc.fDualSrcOutput = ProgramDesc::kNone_DualSrcOutput;
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
        if (this->getCaps().fDualSourceBlendingSupport) {
            if (kZero_BlendCoeff == fCurrDrawState.fDstBlend) {
                // write the coverage value to second color
                desc.fDualSrcOutput =  ProgramDesc::kCoverage_DualSrcOutput;
                desc.fFirstCoverageStage = fCurrDrawState.fFirstCoverageStage;
            } else if (kSA_BlendCoeff == fCurrDrawState.fDstBlend) {
                // SA dst coeff becomes 1-(1-SA)*coverage when dst is partially 
                // cover
                desc.fDualSrcOutput = ProgramDesc::kCoverageISA_DualSrcOutput;
                desc.fFirstCoverageStage = fCurrDrawState.fFirstCoverageStage;
            } else if (kSC_BlendCoeff == fCurrDrawState.fDstBlend) {
                // SA dst coeff becomes 1-(1-SA)*coverage when dst is partially
                // cover
                desc.fDualSrcOutput = ProgramDesc::kCoverageISC_DualSrcOutput;
                desc.fFirstCoverageStage = fCurrDrawState.fFirstCoverageStage;
            }
        }
    }
}
