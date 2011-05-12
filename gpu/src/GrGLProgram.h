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

#ifndef GrGLProgram_DEFINED
#define GrGLProgram_DEFINED

#include "GrGLInterface.h"
#include "GrStringBuilder.h"
#include "GrDrawTarget.h"

#include "SkXfermode.h"

class GrBinHashKeyBuilder;
class GrGLEffect;
struct ShaderCodeSegments;

/**
 * This class manages a GPU program and records per-program information.
 * We can specify the attribute locations so that they are constant
 * across our shaders. But the driver determines the uniform locations
 * at link time. We don't need to remember the sampler uniform location
 * because we will bind a texture slot to it and never change it
 * Uniforms are program-local so we can't rely on fHWState to hold the
 * previous uniform state after a program change.
 */
class GrGLProgram {
public:
    class CachedData;

    GrGLProgram();
    ~GrGLProgram();

    /**
     *  Streams data that can uniquely identifies the generated
     *  gpu program into a key, for cache indexing purposes.
     *
     *  @param key The key object to receive the key data
     */
    void buildKey(GrBinHashKeyBuilder& key) const;

    /**
     *  This is the heavy initilization routine for building a GLProgram.
     *  The result of heavy init is not stored in datamembers of GrGLProgam,
     *  but in a separate cacheable container.
     */
    bool genProgram(CachedData* programData) const;

    /**
     *  Routine that is called before rendering. Sets-up all the state and
     *  other initializations required for the Gpu Program to run.
     */
    bool doGLSetup(GrPrimitiveType type, CachedData* programData) const;

    /**
     *  Routine that is called after rendering. Performs state restoration.
     *  May perform secondary render passes.
     */
    void doGLPost() const;

    /**
     *  Configures the GrGLProgram based on the state of a GrDrawTarget
     *  object.  This is the fast and light initialization. Retrieves all the
     *  state that is required for performing the heavy init (i.e. genProgram),
     *  or for retrieving heavy init results from cache.
     */
    void buildFromTarget(const GrDrawTarget* target);

    static int PositionAttributeIdx() { return 0; }
    static int TexCoordAttributeIdx(int tcIdx) { return 1 + tcIdx; }
    static int ColorAttributeIdx() { return 1 + GrDrawTarget::kMaxTexCoords; }
    static int ViewMatrixAttributeIdx() { 
        return 2 + GrDrawTarget::kMaxTexCoords; 
    }
    static int TextureMatrixAttributeIdx(int stage) { 
        return 5 + GrDrawTarget::kMaxTexCoords + 3 * stage; 
    }

private:

    //Parameters that affect code generation
    struct ProgramDesc {
        ProgramDesc() {
            // since we use this as part of a key we can't have any unitialized
            // padding
            memset(this, 0, sizeof(ProgramDesc));
        }

        // stripped of bits that don't affect prog generation
        GrVertexLayout fVertexLayout;

        enum {
            kNone_ColorType         = 0,
            kAttribute_ColorType    = 1,
            kUniform_ColorType      = 2,
        } fColorType;

        bool fEmitsPointSize;
        bool fUsesEdgeAA;

        SkXfermode::Mode fColorFilterXfermode;

        struct StageDesc {
            enum OptFlagBits {
                kNoPerspective_OptFlagBit  = 0x1,
                kIdentityMatrix_OptFlagBit = 0x2
            };

            unsigned fOptFlags;
            bool fEnabled;

            enum Modulation {
                kColor_Modulation,
                kAlpha_Modulation
            } fModulation;

            enum FetchMode {
                kSingle_FetchMode,
                k2x2_FetchMode
            } fFetchMode;

            enum CoordMapping {
                kIdentity_CoordMapping,
                kRadialGradient_CoordMapping,
                kSweepGradient_CoordMapping,
                kRadial2Gradient_CoordMapping
            } fCoordMapping;
        } fStages[GrDrawTarget::kNumStages];
    } fProgramDesc;

    const ProgramDesc& getDesc() { return fProgramDesc; }

public:
    enum {
        kUnusedUniform = -1,
        kSetAsAttribute = 1000,
    };

    struct StageUniLocations {
        GrGLint fTextureMatrixUni;
        GrGLint fNormalizedTexelSizeUni;
        GrGLint fSamplerUni;
        GrGLint fRadial2Uni;
        void reset() {
            fTextureMatrixUni = kUnusedUniform;
            fNormalizedTexelSizeUni = kUnusedUniform;
            fSamplerUni = kUnusedUniform;
            fRadial2Uni = kUnusedUniform;
        }
    };

    struct UniLocations {
        GrGLint fViewMatrixUni;
        GrGLint fColorUni;
        GrGLint fEdgesUni;
        GrGLint fColorFilterUni;
        StageUniLocations fStages[GrDrawTarget::kNumStages];
        void reset() {
            fViewMatrixUni = kUnusedUniform;
            fColorUni = kUnusedUniform;
            fEdgesUni = kUnusedUniform;
            fColorFilterUni = kUnusedUniform;
            for (int s = 0; s < GrDrawTarget::kNumStages; ++s) {
                fStages[s].reset();
            }
        }
    };

    class CachedData : public ::GrNoncopyable {
    public:
        CachedData() {
            GR_DEBUGCODE(fEffectUniCount = 0;)
            fEffectUniLocationsExtended = NULL;
        }

        ~CachedData() {
            GrFree(fEffectUniLocationsExtended);
        }

        void copyAndTakeOwnership(CachedData& other) {
            memcpy(this, &other, sizeof(*this));
            other.fEffectUniLocationsExtended = NULL; // ownership transfer
            GR_DEBUGCODE(other.fEffectUniCount = 0;)
        }

        void setEffectUniformCount(size_t effectUniforms) {
            GR_DEBUGCODE(fEffectUniCount = effectUniforms;)
            GrFree(fEffectUniLocationsExtended);
            if (effectUniforms > kUniLocationPreAllocSize) {
                fEffectUniLocationsExtended = (GrGLint*)GrMalloc(sizeof(GrGLint)*(effectUniforms-kUniLocationPreAllocSize));
            } else {
                fEffectUniLocationsExtended = NULL;
            }
        }

        GrGLint&  effectUniLocation(size_t index) {
            GrAssert(index < fEffectUniCount);
            return (index < kUniLocationPreAllocSize) ? 
                fEffectUniLocations[index] :
                fEffectUniLocationsExtended[index - kUniLocationPreAllocSize];
        }

    public:

        // IDs
        GrGLuint    fVShaderID;
        GrGLuint    fFShaderID;
        GrGLuint    fProgramID;
        // shader uniform locations (-1 if shader doesn't use them)
        UniLocations fUniLocations;

        GrMatrix  fViewMatrix;

        // these reflect the current values of uniforms
        // (GL uniform values travel with program)
        GrColor                     fColor;
        GrColor                     fColorFilterColor;
        GrMatrix                    fTextureMatrices[GrDrawTarget::kNumStages];
        // width and height used for normalized texel size
        int                         fTextureWidth[GrDrawTarget::kNumStages];
        int                         fTextureHeight[GrDrawTarget::kNumStages]; 
        GrScalar                    fRadial2CenterX1[GrDrawTarget::kNumStages];
        GrScalar                    fRadial2Radius0[GrDrawTarget::kNumStages];
        bool                        fRadial2PosRoot[GrDrawTarget::kNumStages];

    private:
        enum Constants {
            kUniLocationPreAllocSize = 8
        };

        GrGLint     fEffectUniLocations[kUniLocationPreAllocSize];
        GrGLint*    fEffectUniLocationsExtended;
        GR_DEBUGCODE(size_t fEffectUniCount;)
    }; // CachedData

    GrGLEffect* fStageEffects[GrDrawTarget::kNumStages];

private:
    enum {
        kUseUniform = 2000
    };

    // should set all fields in locations var to kUseUniform if the
    // corresponding uniform is required for the program.
    void genStageCode(int stageNum,
                      const ProgramDesc::StageDesc& desc,
                      const char* fsInColor, // NULL means no incoming color
                      const char* fsOutColor,
                      const char* vsInCoord,
                      ShaderCodeSegments* segments,
                      StageUniLocations* locations) const;

    static bool CompileFSAndVS(const ShaderCodeSegments& segments, 
                               CachedData* programData);

    // Compiles a GL shader, returns shader ID or 0 if failed
    // params have same meaning as glShaderSource
    static GrGLuint CompileShader(GrGLenum type, int stringCnt,
                                  const char** strings,
                                  int* stringLengths);

    // Creates a GL program ID, binds shader attributes to GL vertex attrs, and
    // links the program
    bool bindAttribsAndLinkProgram(GrStringBuilder texCoordAttrNames[GrDrawTarget::kMaxTexCoords],
                                   CachedData* programData) const;

    // Gets locations for all uniforms set to kUseUniform and initializes cache
    // to invalid values.
    void getUniformLocationsAndInitCache(CachedData* programData) const;

    friend class GrGpuGLShaders;
};

#endif
