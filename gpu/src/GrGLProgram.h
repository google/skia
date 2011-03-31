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

#define POS_ATTR_LOCATION 0
#define TEX_ATTR_LOCATION(X) (1 + (X))
#define COL_ATTR_LOCATION (2 + GrDrawTarget::kMaxTexCoords)

#include "GrDrawTarget.h"

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
    void genProgram(CachedData* programData, const GrDrawTarget* target) const;

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

private:

    //Parameters that affect code generation
    struct ProgramDesc {
        GrVertexLayout fVertexLayout;

        enum {
            kNotPoints_OptFlagBit = 0x1,
            kVertexColorAllOnes_OptFlagBit = 0x2,
        };
        // we're assuming optflags and layout pack into 32 bits
        // VS 2010 seems to require short rather than just unsigned
        // for this to pack
        unsigned short fOptFlags : 16;

        struct StageDesc {
            enum OptFlagBits {
                kNoPerspective_OptFlagBit  = 0x1,
                kIdentityMatrix_OptFlagBit = 0x2
            };

            unsigned fOptFlags : 8;
            unsigned fEnabled : 8;

            enum Modulation {
                kColor_Modulation,
                kAlpha_Modulation
            } fModulation : 8;

            enum CoordMapping {
                kIdentity_CoordMapping,
                kRadialGradient_CoordMapping,
                kSweepGradient_CoordMapping,
                kRadial2Gradient_CoordMapping
            } fCoordMapping : 8;
        } fStages[GrDrawTarget::kNumStages];
    } fProgramDesc;

public:
    struct StageUniLocations {
        GrGLint fTextureMatrixUni;
        GrGLint fSamplerUni;
        GrGLint fRadial2Uni;
    };

    struct UniLocations {
        GrGLint fViewMatrixUni;
        StageUniLocations fStages[GrDrawTarget::kNumStages];
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
            memcpy(this, &other, sizeof(this));
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
        GrMatrix                    fTextureMatrices[GrDrawTarget::kNumStages];
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
    void genStageCode(int stageNum,
                      const ProgramDesc::StageDesc& desc,
                      const char* fsInColor, // NULL means no incoming color
                      const char* fsOutColor,
                      const char* vsInCoord,
                      ShaderCodeSegments* segments,
                      StageUniLocations* locations) const;

    // Compiles a GL shader, returns shader ID or 0 if failed
    // params have same meaning as glShaderSource
    static GrGLuint CompileShader(GrGLenum type, int stringCnt,
                                  const char** strings,
                                  int* stringLengths);

    friend class GrGpuGLShaders;
};

#endif
