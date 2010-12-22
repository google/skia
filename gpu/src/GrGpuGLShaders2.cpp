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

#if GR_SUPPORT_GLES2 || GR_SUPPORT_GLDESKTOP

#include "GrGpuGLShaders2.h"
#include "GrGpuVertex.h"
#include "GrMemory.h"
#include "GrStringBuilder.h"


#define ATTRIBUTE_MATRIX        0

#define SKIP_COLOR_MODULATE_OPT 0

#define PRINT_SHADERS           0

#define SKIP_CACHE_CHECK    true

#if GR_SUPPORT_GLES2
    #define GR_PRECISION                "mediump"
    const char GR_SHADER_PRECISION[] =  "precision mediump float;\n";
#else
    #define GR_PRECISION                ""
    const char GR_SHADER_PRECISION[] =  "";
#endif

#define POS_ATTR_LOCATION 0
#define TEX_ATTR_LOCATION 1
#define COL_ATTR_LOCATION 2
#if ATTRIBUTE_MATRIX
#define VIEWMAT_ATTR_LOCATION 3
#define TEXMAT_ATTR_LOCATION(X) (6 + 3 * (X))
#define BOGUS_MATRIX_UNI_LOCATION 1000
#endif

const int GrGpuGLShaders2::NUM_STAGES = 1;

struct GrGpuGLShaders2::StageUniLocations {
    GLint fTextureMatrixUni;
    GLint fSamplerUni;
    GLint fRadial2Uni;
};

struct GrGpuGLShaders2::UniLocations {
    GLint fViewMatrixUni;
    StageUniLocations fStages[NUM_STAGES];
};

// Records per-program information
// we can specify the attribute locations so that they are constant
// across our shaders. But the driver determines the uniform locations
// at link time. We don't need to remember the sampler uniform location
// because we will bind a texture slot to it and never change it
// Uniforms are program-local so we can't rely on fHWState to hold the
// previous uniform state after a program change.
struct  GrGpuGLShaders2::Program {
    // IDs
    GLuint    fVShaderID;
    GLuint    fFShaderID;
    GLuint    fProgramID;

    // shader uniform locations (-1 if shader doesn't use them)
    UniLocations fUniLocations;

    // these reflect the current values of uniforms
    // (GL uniform values travel with program)
    GrMatrix                    fViewMatrix;
    GrMatrix                    fTextureMatrix[NUM_STAGES];
    GrGLTexture::Orientation    fTextureOrientation[NUM_STAGES];
    GrScalar                    fRadial2CenterX1[NUM_STAGES];
    GrScalar                    fRadial2Radius0[NUM_STAGES];
    bool                        fRadial2PosRoot[NUM_STAGES];

};

// must be tightly packed
struct GrGpuGLShaders2::StageDesc {
    enum OptFlagBits {
        kNoPerspective_OptFlagBit  = 0x1,
        kIdentityMatrix_OptFlagBit = 0x2,
    };
    int fOptFlags : 8;
    bool fEnabled : 8;
    enum Modulation {
        kColor_Modulation,
        kAlpha_Modulation,
    } fModulation : 8;
    enum CoordMapping {
        kIdentity_CoordMapping,
        kRadialGradient_CoordMapping,
        kSweepGradient_CoordMapping,
        kRadial2Gradient_CoordMapping,
    } fCoordMapping : 8;
};

// must be tightly packed
struct GrGpuGLShaders2::ProgramDesc {
    GrVertexLayout fVertexLayout;
    enum {
        kNotPoints_OptFlagBit = 0x1,
        kVertexColorAllOnes_OptFlagBit = 0x2,
    };
    // we're assuming optflags and layout pack into 32 bits
    GR_STATIC_ASSERT(2 == sizeof(GrVertexLayout));
    int fOptFlags : 16;

    StageDesc fStages[NUM_STAGES];

    bool operator == (const ProgramDesc& desc) const {
        // keep 4-byte aligned and tightly packed
        GR_STATIC_ASSERT(4 == sizeof(StageDesc));
        GR_STATIC_ASSERT(2 + 2 + 4 * NUM_STAGES == sizeof(ProgramDesc));
        return 0 == memcmp(this, &desc, sizeof(ProgramDesc));
    }
};

#include "GrTHashCache.h"

class GrGpuGLShaders2::ProgramCache : public ::GrNoncopyable {
private:
    struct Entry;
    class HashKey {
    public:
        HashKey();
        HashKey(const ProgramDesc& desc);
        static const HashKey& GetKey(const Entry&);
        static bool EQ(const Entry&, const HashKey&);
        static bool LT(const Entry&, const HashKey&);
        bool operator <(const HashKey& key) const;
        bool operator ==(const HashKey& key) const;
        uint32_t getHash() const;
    private:
        ProgramDesc fDesc;
        uint32_t fHash;
    };

    struct Entry {
        Program     fProgram;
        HashKey     fKey;
        uint32_t    fLRUStamp;
    };

    // if hash bits is changed, need to change hash function
    GrTHashTable<Entry, HashKey, 8> fHashCache;

    static const int MAX_ENTRIES = 16;
    Entry        fEntries[MAX_ENTRIES];
    int          fCount;
    uint32_t     fCurrLRUStamp;

public:
    ProgramCache() {
        fCount        = 0;
        fCurrLRUStamp = 0;
    }

    ~ProgramCache() {
        for (int i = 0; i < fCount; ++i) {
            GrGpuGLShaders2::DeleteProgram(&fEntries[i].fProgram);
        }
    }

    void abandon() {
        fCount = 0;
    }

    void invalidateViewMatrices() {
        for (int i = 0; i < fCount; ++i) {
            // set to illegal matrix
            fEntries[i].fProgram.fViewMatrix.setScale(GR_ScalarMax,
                                                      GR_ScalarMax);
        }
    }

    Program* getProgram(const ProgramDesc& desc) {
        HashKey key(desc);
        Entry* entry = fHashCache.find(key);
        if (NULL == entry) {
            if (fCount < MAX_ENTRIES) {
                entry = fEntries + fCount;
                ++fCount;
            } else {
                GrAssert(MAX_ENTRIES == fCount);
                entry = fEntries;
                for (int i = 1; i < MAX_ENTRIES; ++i) {
                    if (fEntries[i].fLRUStamp < entry->fLRUStamp) {
                        entry = fEntries + i;
                    }
                }
                fHashCache.remove(entry->fKey, entry);
                GrGpuGLShaders2::DeleteProgram(&entry->fProgram);
            }
            entry->fKey = key;
            GrGpuGLShaders2::GenProgram(desc, &entry->fProgram);
            fHashCache.insert(entry->fKey, entry);
        }

        entry->fLRUStamp = fCurrLRUStamp;
        if (UINT32_MAX == fCurrLRUStamp) {
            // wrap around! just trash our LRU, one time hit.
            for (int i = 0; i < fCount; ++i) {
                fEntries[i].fLRUStamp = 0;
            }
        }
        ++fCurrLRUStamp;
        return &entry->fProgram;
    }
};

GrGpuGLShaders2::ProgramCache::HashKey::HashKey() {
}

static uint32_t ror(uint32_t x) {
    return (x >> 8) | (x << 24);
}


GrGpuGLShaders2::ProgramCache::HashKey::HashKey(const ProgramDesc& desc) {
    fDesc = desc;
    // if you change the size of the desc, need to update the hash function
    GR_STATIC_ASSERT(8 == sizeof(ProgramDesc));

    uint32_t* d = (uint32_t*) &fDesc;
    fHash = d[0] ^ ror(d[1]);
}

bool GrGpuGLShaders2::ProgramCache::HashKey::EQ(const Entry& entry,
                                                const HashKey& key) {
    return entry.fKey == key;
}

bool GrGpuGLShaders2::ProgramCache::HashKey::LT(const Entry& entry,
                                                const HashKey& key) {
    return entry.fKey < key;
}

bool GrGpuGLShaders2::ProgramCache::HashKey::operator ==(const HashKey& key) const {
    return fDesc == key.fDesc;
}

bool GrGpuGLShaders2::ProgramCache::HashKey::operator <(const HashKey& key) const {
    return memcmp(&fDesc, &key.fDesc, sizeof(HashKey)) < 0;
}

uint32_t GrGpuGLShaders2::ProgramCache::HashKey::getHash() const {
    return fHash;
}


struct GrGpuGLShaders2::ShaderCodeSegments {
    GrSStringBuilder<256> fVSUnis;
    GrSStringBuilder<256> fVSAttrs;
    GrSStringBuilder<256> fVaryings;
    GrSStringBuilder<256> fFSUnis;
    GrSStringBuilder<512> fVSCode;
    GrSStringBuilder<512> fFSCode;
};
// for variable names etc
typedef GrSStringBuilder<16> GrTokenString;

#if ATTRIBUTE_MATRIX
    #define VIEW_MATRIX_NAME "aViewM"
#else
    #define VIEW_MATRIX_NAME "uViewM"
#endif

#define POS_ATTR_NAME "aPosition"
#define COL_ATTR_NAME "aColor"
#define TEX_ATTR_NAME "aTexture"

static inline const char* float_vector_type(int count) {
    static const char* FLOAT_VECS[] = {"ERROR", "float", "vec2", "vec3", "vec4"};
    GrAssert(count >= 1 && count < GR_ARRAY_COUNT(FLOAT_VECS));
    return FLOAT_VECS[count];
}

static inline const char* vector_homog_coord(int count) {
    static const char* HOMOGS[] = {"ERROR", "", ".y", ".z", ".w"};
    GrAssert(count >= 1 && count < GR_ARRAY_COUNT(HOMOGS));
    return HOMOGS[count];
}

static inline const char* vector_nonhomog_coords(int count) {
    static const char* NONHOMOGS[] = {"ERROR", "", ".x", ".xy", ".xyz"};
    GrAssert(count >= 1 && count < GR_ARRAY_COUNT(NONHOMOGS));
    return NONHOMOGS[count];
}

static inline const char* vector_all_coords(int count) {
    static const char* ALL[] = {"ERROR", "", ".xy", ".xyz", ".xyzw"};
    GrAssert(count >= 1 && count < GR_ARRAY_COUNT(ALL));
    return ALL[count];
}

static void tex_matrix_name(int stage, GrStringBuilder* s) {
#if ATTRIBUTE_MATRIX
    *s = "aTexM";
#else
    *s = "uTexM";
#endif
    s->appendInt(stage);
}

static void sampler_name(int stage, GrStringBuilder* s) {
    *s = "uSampler";
    s->appendInt(stage);
}

static void stage_varying_name(int stage, GrStringBuilder* s) {
    *s = "vStage";
    s->appendInt(stage);
}

static void radial2_param_name(int stage, GrStringBuilder* s) {
    *s = "uRadial2Params";
    s->appendInt(stage);
}

static void radial2_varying_name(int stage, GrStringBuilder* s) {
    *s = "vB";
    s->appendInt(stage);
}

#include "GrRandom.h"

void GrGpuGLShaders2::ProgramUnitTest() {
    static const uint16_t VFORMATS[] = {
        0,
        kSeparateTexCoord_VertexLayoutBit,
        kPositionAsTexCoord_VertexLayoutBit,
        kSeparateTexCoord_VertexLayoutBit | kColor_VertexLayoutBit,
        kPositionAsTexCoord_VertexLayoutBit | kColor_VertexLayoutBit,
        kTextFormat_VertexLayoutBit
    };
    static const int PROG_OPTS[] = {
        0,
        ProgramDesc::kNotPoints_OptFlagBit,
        ProgramDesc::kVertexColorAllOnes_OptFlagBit,
        ProgramDesc::kNotPoints_OptFlagBit | ProgramDesc::kVertexColorAllOnes_OptFlagBit
    };
    static const int STAGE_OPTS[] = {
        0,
        StageDesc::kNoPerspective_OptFlagBit,
        StageDesc::kIdentity_CoordMapping
    };
    static const int STAGE_MODULATES[] = {
        StageDesc::kColor_Modulation,
        StageDesc::kAlpha_Modulation
    };
    static const int STAGE_COORD_MAPPINGS[] = {
        StageDesc::kIdentity_CoordMapping,
        StageDesc::kRadialGradient_CoordMapping,
        StageDesc::kSweepGradient_CoordMapping,
        StageDesc::kRadial2Gradient_CoordMapping
    };
    ProgramDesc pdesc;
    memset(&pdesc, 0, sizeof(pdesc));

    static const int NUM_TESTS = 1024;

    // GrRandoms nextU() values have patterns in the low bits
    // So using nextU() % array_count might never take some values.
    GrRandom random;
    for (int t = 0; t < NUM_TESTS; ++t) {
        int x = (int)(random.nextF() * GR_ARRAY_COUNT(VFORMATS));
        pdesc.fVertexLayout = VFORMATS[x];
        x = (int)(random.nextF() * GR_ARRAY_COUNT(PROG_OPTS));
        pdesc.fOptFlags = PROG_OPTS[x];
        for (int s = 0; s < NUM_STAGES; ++s) {
            x = (int)(random.nextF() * 2.f);
            pdesc.fStages[s].fEnabled = x;
            x = (int)(random.nextF() * GR_ARRAY_COUNT(STAGE_OPTS));
            pdesc.fStages[s].fOptFlags = STAGE_OPTS[x];
            x = (int)(random.nextF() * GR_ARRAY_COUNT(STAGE_MODULATES));
            pdesc.fStages[s].fModulation = STAGE_MODULATES[x];
            x = (int)(random.nextF() * GR_ARRAY_COUNT(STAGE_COORD_MAPPINGS));
            pdesc.fStages[s].fCoordMapping = STAGE_COORD_MAPPINGS[x];
        }
        Program program;
        GenProgram(pdesc, &program);
        DeleteProgram(&program);
    }
}

void GrGpuGLShaders2::GenStageCode(int stageNum,
                                   const StageDesc& desc,
                                   const char* fsInColor, // NULL means no incoming color
                                   const char* fsOutColor,
                                   const char* vsInCoord,
                                   ShaderCodeSegments* segments,
                                   StageUniLocations* locations) {

    GrAssert(stageNum >= 0 && stageNum <= 9);

    GrTokenString varyingName;
    stage_varying_name(stageNum, &varyingName);

    // First decide how many coords are needed to access the texture
    // Right now it's always 2 but we could start using 1D textures for
    // gradients.
    static const int coordDims = 2;
    int varyingDims;
    /// Vertex Shader Stuff

    // decide whether we need a matrix to transform texture coords
    // and whether the varying needs a perspective coord.
    GrTokenString texMName;
    tex_matrix_name(stageNum, &texMName);
    if (desc.fOptFlags & StageDesc::kIdentityMatrix_OptFlagBit) {
        varyingDims = coordDims;
    } else {
    #if ATTRIBUTE_MATRIX
        segments->fVSAttrs += "attribute mat3 ";
        segments->fVSAttrs += texMName;
        segments->fVSAttrs += ";\n";
    #else
        segments->fVSUnis += "uniform mat3 ";
        segments->fVSUnis += texMName;
        segments->fVSUnis += ";\n";
        locations->fTextureMatrixUni = 1;
    #endif
        if (desc.fOptFlags & StageDesc::kNoPerspective_OptFlagBit) {
            varyingDims = coordDims;
        } else {
            varyingDims = coordDims + 1;
        }
    }

    GrTokenString samplerName;
    sampler_name(stageNum, &samplerName);
    segments->fFSUnis += "uniform sampler2D ";
    segments->fFSUnis += samplerName;
    segments->fFSUnis += ";\n";
    locations->fSamplerUni = 1;

    segments->fVaryings += "varying ";
    segments->fVaryings += float_vector_type(varyingDims);
    segments->fVaryings += " ";
    segments->fVaryings += varyingName;
    segments->fVaryings += ";\n";

    if (desc.fOptFlags & StageDesc::kIdentityMatrix_OptFlagBit) {
        GrAssert(varyingDims == coordDims);
        segments->fVSCode += "\t";
        segments->fVSCode += varyingName;
        segments->fVSCode += " = ";
        segments->fVSCode += vsInCoord;
        segments->fVSCode += ";\n";
    } else {
        segments->fVSCode += "\t";
        segments->fVSCode += varyingName;
        segments->fVSCode += " = (";
        segments->fVSCode += texMName;
        segments->fVSCode += " * vec3(";
        segments->fVSCode += vsInCoord;
        segments->fVSCode += ", 1))";
        segments->fVSCode += vector_all_coords(varyingDims);
        segments->fVSCode += ";\n";
    }

    GrTokenString radial2ParamsName;
    radial2_param_name(stageNum, &radial2ParamsName);
    // for radial grads without perspective we can pass the linear
    // part of the quadratic as a varying.
    GrTokenString radial2VaryingName;
    radial2_varying_name(stageNum, &radial2VaryingName);

    if (StageDesc::kRadial2Gradient_CoordMapping == desc.fCoordMapping) {

        segments->fVSUnis += "uniform " GR_PRECISION " float ";
        segments->fVSUnis += radial2ParamsName;
        segments->fVSUnis += "[6];\n";

        segments->fFSUnis += "uniform " GR_PRECISION " float ";
        segments->fFSUnis += radial2ParamsName;
        segments->fFSUnis += "[6];\n";
        locations->fRadial2Uni = 1;

        // if there is perspective we don't interpolate this
        if (varyingDims == coordDims) {
            GrAssert(2 == coordDims);
            segments->fVaryings += "varying float ";
            segments->fVaryings += radial2VaryingName;
            segments->fVaryings += ";\n";

            segments->fVSCode += "\t";
            segments->fVSCode += radial2VaryingName;
            segments->fVSCode += " = 2.0 * (";
            segments->fVSCode += radial2ParamsName;
            segments->fVSCode += "[2] * ";
            segments->fVSCode += varyingName;
            segments->fVSCode += ".x ";
            segments->fVSCode += " - ";
            segments->fVSCode += radial2ParamsName;
            segments->fVSCode += "[3]);\n";
        }
    }

    /// Fragment Shader Stuff
    GrTokenString fsCoordName;
    // function used to access the shader, may be made projective
    GrTokenString texFunc("texture2D");
    if (desc.fOptFlags & (StageDesc::kIdentityMatrix_OptFlagBit |
                          StageDesc::kNoPerspective_OptFlagBit)) {
        GrAssert(varyingDims == coordDims);
        fsCoordName = varyingName;
    } else {
        // if we have to do some non-matrix op on the varyings to get
        // our final tex coords then when in perspective we have to
        // do an explicit divide
        if  (StageDesc::kIdentity_CoordMapping == desc.fCoordMapping) {
            texFunc += "Proj";
            fsCoordName = varyingName;
        } else {
            fsCoordName = "tCoord";
            fsCoordName.appendInt(stageNum);

            segments->fFSCode += "\t";
            segments->fFSCode += float_vector_type(coordDims);
            segments->fFSCode += " ";
            segments->fFSCode += fsCoordName;
            segments->fFSCode += " = ";
            segments->fFSCode += varyingName;
            segments->fFSCode += vector_nonhomog_coords(varyingDims);
            segments->fFSCode += " / ";
            segments->fFSCode += varyingName;
            segments->fFSCode += vector_homog_coord(varyingDims);
            segments->fFSCode += ";\n";
        }
    }

    GrSStringBuilder<96> sampleCoords;
    switch (desc.fCoordMapping) {
    case StageDesc::kIdentity_CoordMapping:
        sampleCoords = fsCoordName;
        break;
    case StageDesc::kSweepGradient_CoordMapping:
        sampleCoords = "vec2(atan(-";
        sampleCoords += fsCoordName;
        sampleCoords += ".y, -";
        sampleCoords += fsCoordName;
        sampleCoords += ".x)*0.1591549430918 + 0.5, 0.5)";
        break;
    case StageDesc::kRadialGradient_CoordMapping:
        sampleCoords = "vec2(length(";
        sampleCoords += fsCoordName;
        sampleCoords += ".xy), 0.5)";
        break;
    case StageDesc::kRadial2Gradient_CoordMapping: {
        GrTokenString cName    = "c";
        GrTokenString ac4Name  = "ac4";
        GrTokenString rootName = "root";

        cName.appendInt(stageNum);
        ac4Name.appendInt(stageNum);
        rootName.appendInt(stageNum);

        GrTokenString bVar;
        if (coordDims == varyingDims) {
            bVar = radial2VaryingName;
            GrAssert(2 == varyingDims);
        } else {
            GrAssert(3 == varyingDims);
            bVar = "b";
            bVar.appendInt(stageNum);
            segments->fFSCode += "\tfloat ";
            segments->fFSCode += bVar;
            segments->fFSCode += " = 2.0 * (";
            segments->fFSCode += radial2ParamsName;
            segments->fFSCode += "[2] * ";
            segments->fFSCode += fsCoordName;
            segments->fFSCode += ".x ";
            segments->fFSCode += " - ";
            segments->fFSCode += radial2ParamsName;
            segments->fFSCode += "[3]);\n";
        }

        segments->fFSCode += "\tfloat ";
        segments->fFSCode += cName;
        segments->fFSCode += " = dot(";
        segments->fFSCode += fsCoordName;
        segments->fFSCode += ", ";
        segments->fFSCode += fsCoordName;
        segments->fFSCode += ") + ";
        segments->fFSCode += " - ";
        segments->fFSCode += radial2ParamsName;
        segments->fFSCode += "[4];\n";

        segments->fFSCode += "\tfloat ";
        segments->fFSCode += ac4Name;
        segments->fFSCode += " = ";
        segments->fFSCode += radial2ParamsName;
        segments->fFSCode += "[0] * 4.0 * ";
        segments->fFSCode += cName;
        segments->fFSCode += ";\n";

        segments->fFSCode += "\tfloat ";
        segments->fFSCode += rootName;
        segments->fFSCode += " = sqrt(abs(";
        segments->fFSCode += bVar;
        segments->fFSCode += " * ";
        segments->fFSCode += bVar;
        segments->fFSCode += " - ";
        segments->fFSCode += ac4Name;
        segments->fFSCode += "));\n";

        sampleCoords = "vec2((-";
        sampleCoords += bVar;
        sampleCoords += " + ";
        sampleCoords += radial2ParamsName;
        sampleCoords += "[5] * ";
        sampleCoords += rootName;
        sampleCoords += ") * ";
        sampleCoords += radial2ParamsName;
        sampleCoords += "[1], 0.5)\n";
        break;}
    };

    segments->fFSCode += "\t";
    segments->fFSCode += fsOutColor;
    segments->fFSCode += " = ";
    if (NULL != fsInColor) {
        segments->fFSCode += fsInColor;
        segments->fFSCode += " * ";
    }
    segments->fFSCode += texFunc;
    segments->fFSCode += "(";
    segments->fFSCode += samplerName;
    segments->fFSCode += ", ";
    segments->fFSCode += sampleCoords;
    segments->fFSCode += ")";
    if (desc.fModulation == StageDesc::kAlpha_Modulation) {
        segments->fFSCode += ".aaaa";
    }
    segments->fFSCode += ";\n";

}

void GrGpuGLShaders2::GenProgram(const ProgramDesc& desc,
                                 Program* program) {

    ShaderCodeSegments segments;
    const uint32_t& layout = desc.fVertexLayout;

    memset(&program->fUniLocations, 0, sizeof(UniLocations));

    bool haveColor = !(ProgramDesc::kVertexColorAllOnes_OptFlagBit &
                       desc.fOptFlags);

#if ATTRIBUTE_MATRIX
    segments.fVSAttrs = "attribute mat3 " VIEW_MATRIX_NAME ";\n"
#else
    segments.fVSUnis  = "uniform mat3 " VIEW_MATRIX_NAME ";\n";
    segments.fVSAttrs = "";
#endif
    segments.fVSAttrs += "attribute vec2 " POS_ATTR_NAME ";\n";
    if (haveColor) {
        segments.fVSAttrs += "attribute vec4 " COL_ATTR_NAME ";\n";
        segments.fVaryings = "varying vec4 vColor;\n";
    } else {
        segments.fVaryings = "";
    }

    segments.fVSCode   = "void main() {\n"
                         "\tvec3 pos3 = " VIEW_MATRIX_NAME " * vec3(" POS_ATTR_NAME ", 1);\n"
                         "\tgl_Position = vec4(pos3.xy, 0, pos3.z);\n";
    if (haveColor) {
        segments.fVSCode += "\tvColor = " COL_ATTR_NAME ";\n";
    }

    if (!(desc.fOptFlags & ProgramDesc::kNotPoints_OptFlagBit)){
        segments.fVSCode += "\tgl_PointSize = 1.0;\n";
    }
    segments.fFSCode   = "void main() {\n";

    bool textureCoordAttr = false;
    static const char* IN_COORDS[] = {POS_ATTR_NAME, TEX_ATTR_NAME};
    const char* inCoords = NULL;
    if ((kSeparateTexCoord_VertexLayoutBit | kTextFormat_VertexLayoutBit) &
        layout) {
        segments.fVSAttrs += "attribute vec2 " TEX_ATTR_NAME ";\n";
        inCoords = IN_COORDS[1];
        textureCoordAttr = true;
    } else if (kPositionAsTexCoord_VertexLayoutBit & layout) {
        inCoords = IN_COORDS[0];
    }

    GrTokenString inColor = "vColor";
    GR_STATIC_ASSERT(NUM_STAGES <= 9);
    int numActiveStages = 0;
    for (int i = 0; i < NUM_STAGES; ++i) {
        if (desc.fStages[i].fEnabled) {
            ++numActiveStages;
        }
    }
    if (NULL != inCoords && numActiveStages) {
        int currActiveStage = 0;
        for (int i = 0; i < NUM_STAGES; ++i) {
            if (desc.fStages[i].fEnabled) {
                GrTokenString outColor;
                if (currActiveStage < (numActiveStages - 1)) {
                    outColor = "color";
                    outColor.appendInt(currActiveStage);
                    segments.fFSCode += "\tvec4 ";
                    segments.fFSCode += outColor;
                    segments.fFSCode += ";\n";
                } else {
                    outColor = "gl_FragColor";
                }
                GenStageCode(i,
                             desc.fStages[i],
                             haveColor ? inColor.cstr() : NULL,
                             outColor.cstr(),
                             inCoords,
                             &segments,
                             &program->fUniLocations.fStages[i]);
                ++currActiveStage;
                inColor = outColor;
                haveColor = true;
            }
        }
    } else {
        segments.fFSCode += "\tgl_FragColor = ";
        if (haveColor) {
            segments.fFSCode += inColor;
        } else {
            segments.fFSCode += "vec4(1,1,1,1)";
        }
        segments.fFSCode += ";\n";
    }
    segments.fFSCode += "}\n";
    segments.fVSCode += "}\n";


    const char* strings[4];
    int lengths[4];
    int stringCnt = 0;

    if (segments.fVSUnis.length()) {
        strings[stringCnt] = segments.fVSUnis.cstr();
        lengths[stringCnt] = segments.fVSUnis.length();
        ++stringCnt;
    }
    if (segments.fVSAttrs.length()) {
        strings[stringCnt] = segments.fVSAttrs.cstr();
        lengths[stringCnt] = segments.fVSAttrs.length();
        ++stringCnt;
    }
    if (segments.fVaryings.length()) {
        strings[stringCnt] = segments.fVaryings.cstr();
        lengths[stringCnt] = segments.fVaryings.length();
        ++stringCnt;
    }

    GrAssert(segments.fVSCode.length());
    strings[stringCnt] = segments.fVSCode.cstr();
    lengths[stringCnt] = segments.fVSCode.length();
    ++stringCnt;

#if PRINT_SHADERS
    GrPrintf("%s%s%s%s\n",
             segments.fVSUnis.cstr(),
             segments.fVSAttrs.cstr(),
             segments.fVaryings.cstr(),
             segments.fVSCode.cstr());
#endif
    program->fVShaderID = CompileShader(GL_VERTEX_SHADER,
                                        stringCnt,
                                        strings,
                                        lengths);

    stringCnt = 0;

    if (GR_ARRAY_COUNT(GR_SHADER_PRECISION) > 1) {
        strings[stringCnt] = GR_SHADER_PRECISION;
        lengths[stringCnt] = GR_ARRAY_COUNT(GR_SHADER_PRECISION) - 1;
        ++stringCnt;
    }
    if (segments.fFSUnis.length()) {
        strings[stringCnt] = segments.fFSUnis.cstr();
        lengths[stringCnt] = segments.fFSUnis.length();
        ++stringCnt;
    }
    if (segments.fVaryings.length()) {
        strings[stringCnt] = segments.fVaryings.cstr();
        lengths[stringCnt] = segments.fVaryings.length();
        ++stringCnt;
    }

    GrAssert(segments.fFSCode.length());
    strings[stringCnt] = segments.fFSCode.cstr();
    lengths[stringCnt] = segments.fFSCode.length();
    ++stringCnt;

#if PRINT_SHADERS
    GrPrintf("%s%s%s%s\n",
             GR_SHADER_PRECISION,
             segments.fFSUnis.cstr(),
             segments.fVaryings.cstr(),
             segments.fFSCode.cstr());
#endif
    program->fFShaderID = CompileShader(GL_FRAGMENT_SHADER,
                                        stringCnt,
                                        strings,
                                        lengths);

    program->fProgramID = GR_GL(CreateProgram());
    const GLint& progID = program->fProgramID;

    GR_GL(AttachShader(progID, program->fVShaderID));
    GR_GL(AttachShader(progID, program->fFShaderID));

    // Bind the attrib locations to same values for all shaders
    GR_GL(BindAttribLocation(progID, POS_ATTR_LOCATION, POS_ATTR_NAME));
    if (textureCoordAttr) {
        GR_GL(BindAttribLocation(progID, TEX_ATTR_LOCATION, TEX_ATTR_NAME));
    }

#if ATTRIBUTE_MATRIX
    // set unis to a bogus value so that checks against -1 before
    // flushing will pass.
    GR_GL(BindAttribLocation(progID,
                             VIEWMAT_ATTR_LOCATION,
                             VIEW_MATRIX_NAME));

    program->fUniLocations.fViewMatrixUni = BOGUS_MATRIX_UNI_LOCATION;

    for (int i = 0; i < NUM_STAGES; ++i) {
        if (desc.fStages[i].fEnabled) {
            GR_GL(BindAttribLocation(progID,
                                     TEXMAT_ATTR_LOCATION(i),
                                     tex_matrix_name(i).cstr()));
            program->fUniLocations.fStages[i].fTextureMatrixUni =
                                                    BOGUS_MATRIX_UNI_LOCATION;
        }
    }
#endif

    GR_GL(BindAttribLocation(progID, COL_ATTR_LOCATION, COL_ATTR_NAME));

    GR_GL(LinkProgram(progID));

    GLint linked;
    GR_GL(GetProgramiv(progID, GL_LINK_STATUS, &linked));
    if (!linked) {
        GLint infoLen;
        GR_GL(GetProgramiv(progID, GL_INFO_LOG_LENGTH, &infoLen));
        GrAutoMalloc log(sizeof(char)*(infoLen+1));  // outside if for debugger
        if (infoLen > 0) {
            GR_GL(GetProgramInfoLog(progID,
                                    infoLen+1,
                                    NULL,
                                    (char*)log.get()));
            GrPrintf((char*)log.get());
        }
        GrAssert(!"Error linking program");
        GR_GL(DeleteProgram(progID));
        program->fProgramID = 0;
        return;
    }

    // Get uniform locations
#if !ATTRIBUTE_MATRIX
    program->fUniLocations.fViewMatrixUni =
                    GR_GL(GetUniformLocation(progID, VIEW_MATRIX_NAME));
    GrAssert(-1 != program->fUniLocations.fViewMatrixUni);
#endif
    for (int i = 0; i < NUM_STAGES; ++i) {
        StageUniLocations& locations = program->fUniLocations.fStages[i];
        if (desc.fStages[i].fEnabled) {
#if !ATTRIBUTE_MATRIX
            if (locations.fTextureMatrixUni) {
                GrTokenString texMName;
                tex_matrix_name(i, &texMName);
                locations.fTextureMatrixUni = GR_GL(GetUniformLocation(
                                                progID,
                                                texMName.cstr()));
                GrAssert(-1 != locations.fTextureMatrixUni);
            } else {
                locations.fTextureMatrixUni = -1;

            }
#endif

            if (locations.fSamplerUni) {
                GrTokenString samplerName;
                sampler_name(i, &samplerName);
                locations.fSamplerUni = GR_GL(GetUniformLocation(
                                                     progID,
                                                     samplerName.cstr()));
                GrAssert(-1 != locations.fSamplerUni);
            } else {
                locations.fSamplerUni = -1;
            }

            if (locations.fRadial2Uni) {
                GrTokenString radial2ParamName;
                radial2_param_name(i, &radial2ParamName);
                locations.fRadial2Uni = GR_GL(GetUniformLocation(
                                             progID,
                                             radial2ParamName.cstr()));
                GrAssert(-1 != locations.fRadial2Uni);
            } else {
                locations.fRadial2Uni = -1;
            }
        } else {
            locations.fSamplerUni = -1;
            locations.fRadial2Uni = -1;
            locations.fTextureMatrixUni = -1;
        }
    }
    GR_GL(UseProgram(progID));

    // init sampler unis and set bogus values for state tracking
    for (int i = 0; i < NUM_STAGES; ++i) {
        if (-1 != program->fUniLocations.fStages[i].fSamplerUni) {
            GR_GL(Uniform1i(program->fUniLocations.fStages[i].fSamplerUni, i));
        }
        program->fTextureMatrix[i].setScale(GR_ScalarMax, GR_ScalarMax);
        program->fRadial2CenterX1[i] = GR_ScalarMax;
        program->fRadial2Radius0[i] = -GR_ScalarMax;
    }
    program->fViewMatrix.setScale(GR_ScalarMax, GR_ScalarMax);
}

void GrGpuGLShaders2::getProgramDesc(PrimitiveType primType, ProgramDesc* desc) {

    // Must initialize all fields or cache will have false negatives!
    desc->fVertexLayout = fGeometrySrc.fVertexLayout;
    desc->fStages[0].fEnabled = VertexHasTexCoords(fGeometrySrc.fVertexLayout);
    for (int i = 1; i < NUM_STAGES; ++i) {
        desc->fStages[i].fEnabled       = false;
        desc->fStages[i].fOptFlags      = 0;
        desc->fStages[i].fCoordMapping  = 0;
        desc->fStages[i].fModulation    = 0;
    }

    if (primType != kPoints_PrimitiveType) {
        desc->fOptFlags = ProgramDesc::kNotPoints_OptFlagBit;
    } else {
        desc->fOptFlags = 0;
    }
#if SKIP_COLOR_MODULATE_OPT
    if (!(desc->fVertexLayout & kColor_VertexLayoutBit) &&
        (0xffffffff == fCurrDrawState.fColor)) {
        desc->fOptFlags |= ProgramDesc::kVertexColorAllOnes_OptFlagBit;
    }
#endif

    StageDesc& stage = desc->fStages[0];

    if (stage.fEnabled) {
        GrGLTexture* texture = (GrGLTexture*) fCurrDrawState.fTexture;
        GrAssert(NULL != texture);
        // we matrix to invert when orientation is TopDown, so make sure
        // we aren't in that case before flagging as identity.
        if (fCurrDrawState.fMatrixModeCache[kTexture_MatrixMode].isIdentity() &&
            GrGLTexture::kTopDown_Orientation == texture->orientation()) {
            stage.fOptFlags = StageDesc::kIdentityMatrix_OptFlagBit;
        } else if (!fCurrDrawState.fMatrixModeCache[kTexture_MatrixMode].hasPerspective()) {
            stage.fOptFlags = StageDesc::kNoPerspective_OptFlagBit;
        } else {
            stage.fOptFlags = 0;
        }
        switch (fCurrDrawState.fSamplerState.getSampleMode()) {
        case GrSamplerState::kNormal_SampleMode:
            stage.fCoordMapping = StageDesc::kIdentity_CoordMapping;
            stage.fModulation = StageDesc::kColor_Modulation;
            break;
        case GrSamplerState::kAlphaMod_SampleMode:
            stage.fCoordMapping = StageDesc::kIdentity_CoordMapping;
            stage.fModulation = StageDesc::kAlpha_Modulation;
            break;
        case GrSamplerState::kRadial_SampleMode:
            stage.fCoordMapping = StageDesc::kRadialGradient_CoordMapping;
            stage.fModulation = StageDesc::kColor_Modulation;
            break;
        case GrSamplerState::kRadial2_SampleMode:
            stage.fCoordMapping = StageDesc::kRadial2Gradient_CoordMapping;
            stage.fModulation = StageDesc::kColor_Modulation;
            break;
        case GrSamplerState::kSweep_SampleMode:
            stage.fCoordMapping = StageDesc::StageDesc::kSweepGradient_CoordMapping;
            stage.fModulation = StageDesc::kColor_Modulation;
            break;
        default:
            GrAssert(!"Unexpected sample mode!");
            break;
        }
    } else {
        stage.fOptFlags     = 0;
        stage.fCoordMapping = 0;
        stage.fModulation   = 0;
    }
}

GLuint GrGpuGLShaders2::CompileShader(GLenum type,
                                      int stringCnt,
                                      const char** strings,
                                      int* stringLengths) {
    GLuint shader = GR_GL(CreateShader(type));
    if (0 == shader) {
        return 0;
    }

    GLint compiled;
    GR_GL(ShaderSource(shader, stringCnt, strings, stringLengths));
    GR_GL(CompileShader(shader));
    GR_GL(GetShaderiv(shader, GL_COMPILE_STATUS, &compiled));

    if (!compiled) {
        GLint infoLen;
        GR_GL(GetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen));
        GrAutoMalloc log(sizeof(char)*(infoLen+1)); // outside if for debugger
        if (infoLen > 0) {
            GR_GL(GetShaderInfoLog(shader, infoLen+1, NULL, (char*)log.get()));
            for (int i = 0; i < stringCnt; ++i) {
                if (NULL == stringLengths || stringLengths[i] < 0) {
                    GrPrintf(strings[i]);
                } else {
                    GrPrintf("%.*s", stringLengths[i], strings[i]);
                }
            }
            GrPrintf("\n%s", log.get());
        }
        GrAssert(!"Shader compilation failed!");
        GR_GL(DeleteShader(shader));
        return 0;
    }
    return shader;
}

void GrGpuGLShaders2::DeleteProgram(Program* program) {
    GR_GL(DeleteShader(program->fVShaderID));
    GR_GL(DeleteShader(program->fFShaderID));
    GR_GL(DeleteProgram(program->fProgramID));
    GR_DEBUGCODE(memset(program, 0, sizeof(Program)));
}


GrGpuGLShaders2::GrGpuGLShaders2() {

    resetContextHelper();

    fProgram = NULL;
    fProgramCache = new ProgramCache();

#if GR_DEBUG
    ProgramUnitTest();
#endif
}

GrGpuGLShaders2::~GrGpuGLShaders2() {
    delete fProgramCache;
}

void GrGpuGLShaders2::resetContext() {
    INHERITED::resetContext();
    resetContextHelper();
}

void GrGpuGLShaders2::resetContextHelper() {
    fTextureOrientation = (GrGLTexture::Orientation)-1; // illegal

    fHWGeometryState.fVertexLayout = 0;
    fHWGeometryState.fPositionPtr  = (void*) ~0;
    GR_GL(DisableVertexAttribArray(COL_ATTR_LOCATION));
    GR_GL(DisableVertexAttribArray(TEX_ATTR_LOCATION));
    GR_GL(EnableVertexAttribArray(POS_ATTR_LOCATION));

    fHWProgramID = 0;
}

void GrGpuGLShaders2::flushViewMatrix() {
    GrAssert(NULL != fCurrDrawState.fRenderTarget);
    GrMatrix m (
        GrIntToScalar(2) / fCurrDrawState.fRenderTarget->width(), 0, -GR_Scalar1,
        0,-GrIntToScalar(2) / fCurrDrawState.fRenderTarget->height(), GR_Scalar1,
        0, 0, GrMatrix::I()[8]);
    m.setConcat(m, fCurrDrawState.fMatrixModeCache[kModelView_MatrixMode]);

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
    glVertexAttrib4fv(VIEWMAT_ATTR_LOCATION+0, mt+0);
    glVertexAttrib4fv(VIEWMAT_ATTR_LOCATION+1, mt+3);
    glVertexAttrib4fv(VIEWMAT_ATTR_LOCATION+2, mt+6);
#else
    GR_GL(UniformMatrix3fv(fProgram->fUniLocations.fViewMatrixUni,1,false,mt));
#endif
}

void GrGpuGLShaders2::flushTextureMatrix() {

    GrAssert(NULL != fCurrDrawState.fTexture);
    GrGLTexture::Orientation orientation =
         ((GrGLTexture*)fCurrDrawState.fTexture)->orientation();

    GrMatrix* m;
    GrMatrix temp;
    if (GrGLTexture::kBottomUp_Orientation == orientation) {
        temp.setAll(
            GR_Scalar1, 0, 0,
            0, -GR_Scalar1, GR_Scalar1,
            0, 0, GrMatrix::I()[8]
        );
        temp.preConcat(fCurrDrawState.fMatrixModeCache[kTexture_MatrixMode]);
        m = &temp;
    } else {
        GrAssert(GrGLTexture::kTopDown_Orientation == orientation);
        m = &fCurrDrawState.fMatrixModeCache[kTexture_MatrixMode];
    }

    // ES doesn't allow you to pass true to the transpose param,
    // so do our own transpose
    GrScalar mt[]  = {
        (*m)[GrMatrix::kScaleX],
        (*m)[GrMatrix::kSkewY],
        (*m)[GrMatrix::kPersp0],
        (*m)[GrMatrix::kSkewX],
        (*m)[GrMatrix::kScaleY],
        (*m)[GrMatrix::kPersp1],
        (*m)[GrMatrix::kTransX],
        (*m)[GrMatrix::kTransY],
        (*m)[GrMatrix::kPersp2]
    };
#if ATTRIBUTE_MATRIX
    glVertexAttrib4fv(TEXMAT_ATTR_LOCATION(0)+0, mt+0);
    glVertexAttrib4fv(TEXMAT_ATTR_LOCATION(0)+1, mt+3);
    glVertexAttrib4fv(TEXMAT_ATTR_LOCATION(0)+2, mt+6);
#else
    GR_GL(UniformMatrix3fv(fProgram->fUniLocations.fStages[0].fTextureMatrixUni,
                           1,
                           false,
                           mt));
#endif
}

void GrGpuGLShaders2::flushRadial2() {

    const GrSamplerState& sampler = fCurrDrawState.fSamplerState;

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
    GR_GL(Uniform1fv(fProgram->fUniLocations.fStages[0].fRadial2Uni, 6, unis));
}

void GrGpuGLShaders2::flushProgram(PrimitiveType type) {
    ProgramDesc desc;
    getProgramDesc(type, &desc);
    fProgram = fProgramCache->getProgram(desc);

    if (fHWProgramID != fProgram->fProgramID) {
        GR_GL(UseProgram(fProgram->fProgramID));
        fHWProgramID = fProgram->fProgramID;
#if GR_COLLECT_STATS
        ++fStats.fProgChngCnt;
#endif
    }
}

bool GrGpuGLShaders2::flushGraphicsState(PrimitiveType type) {

    flushGLStateCommon(type);

    if (fRenderTargetChanged) {
        // our coords are in pixel space and the GL matrices map to NDC
        // so if the viewport changed, our matrix is now wrong.
#if ATTRIBUTE_MATRIX
        fHWDrawState.fMatrixModeCache[kModelView_MatrixMode].setScale(GR_ScalarMax,
                                                                      GR_ScalarMax);
#else
        // we assume all shader matrices may be wrong after viewport changes
        fProgramCache->invalidateViewMatrices();
#endif
        fRenderTargetChanged = false;
    }

    flushProgram(type);

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

#if ATTRIBUTE_MATRIX
    GrMatrix& currViewMatrix = fHWDrawState.fMatrixModeCache[kModelView_MatrixMode];
    GrMatrix& currTextureMatrix = fHWDrawState.fMatrixModeCache[kTexture_MatrixMode];
    GrGLTexture::Orientation& orientation = fTextureOrientation;
#else
    GrMatrix& currViewMatrix = fProgram->fViewMatrix;
    GrMatrix& currTextureMatrix = fProgram->fTextureMatrix[0];
    GrGLTexture::Orientation& orientation =  fProgram->fTextureOrientation[0];
#endif

    if (currViewMatrix != fCurrDrawState.fMatrixModeCache[kModelView_MatrixMode]) {
        flushViewMatrix();
        currViewMatrix = fCurrDrawState.fMatrixModeCache[kModelView_MatrixMode];
    }

    GrGLTexture* texture = (GrGLTexture*) fCurrDrawState.fTexture;
    if (NULL != texture) {
        if (-1 != fProgram->fUniLocations.fStages[0].fTextureMatrixUni &&
            (currTextureMatrix !=
                        fCurrDrawState.fMatrixModeCache[kTexture_MatrixMode] ||
             orientation != texture->orientation())) {
            flushTextureMatrix();
            currTextureMatrix = fCurrDrawState.fMatrixModeCache[kTexture_MatrixMode];
            orientation = texture->orientation();
        }
    }

    const GrSamplerState& sampler = fCurrDrawState.fSamplerState;
    if (-1 != fProgram->fUniLocations.fStages[0].fRadial2Uni &&
        (fProgram->fRadial2CenterX1[0] != sampler.getRadial2CenterX1() ||
         fProgram->fRadial2Radius0[0]  != sampler.getRadial2Radius0()  ||
         fProgram->fRadial2PosRoot[0]  != sampler.isRadial2PosRoot())) {

        flushRadial2();

        fProgram->fRadial2CenterX1[0] = sampler.getRadial2CenterX1();
        fProgram->fRadial2Radius0[0]  = sampler.getRadial2Radius0();
        fProgram->fRadial2PosRoot[0]  = sampler.isRadial2PosRoot();
    }

    return true;
}

void GrGpuGLShaders2::setupGeometry(uint32_t startVertex,
                                   uint32_t startIndex,
                                   uint32_t vertexCount,
                                   uint32_t indexCount) {

    int newColorOffset, newTexCoordOffset;

    GLsizei newStride = VertexSizeAndOffsets(fGeometrySrc.fVertexLayout,
                                             &newTexCoordOffset,
                                             &newColorOffset);
    int oldColorOffset, oldTexCoordOffset;
    GLsizei oldStride = VertexSizeAndOffsets(fHWGeometryState.fVertexLayout,
                                             &oldTexCoordOffset,
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
    bool texCoordNorm;
    if (fGeometrySrc.fVertexLayout & kTextFormat_VertexLayoutBit) {
        scalarType = GrGLTextType;
        texCoordNorm = GR_GL_TEXT_TEXTURE_NORMALIZED;
    } else {
        scalarType = GrGLType;
        texCoordNorm = false;
    }

    bool baseChange = posPtr != fHWGeometryState.fPositionPtr;
    bool scalarChange = (GrGLTextType != GrGLType) &&
                        (kTextFormat_VertexLayoutBit &
                         (fHWGeometryState.fVertexLayout ^
                          fGeometrySrc.fVertexLayout));
    bool strideChange = newStride != oldStride;
    bool posChange = baseChange || scalarChange || strideChange;

    if (posChange) {
        GR_GL(VertexAttribPointer(POS_ATTR_LOCATION, 2, scalarType,
                                  false, newStride, posPtr));
        fHWGeometryState.fPositionPtr = posPtr;
    }

    if (newTexCoordOffset > 0) {
        GLvoid* texCoordPtr = (int8_t*)posPtr + newTexCoordOffset;
        if (oldTexCoordOffset <= 0) {
            GR_GL(EnableVertexAttribArray(TEX_ATTR_LOCATION));
        }
        if (posChange || newTexCoordOffset != oldTexCoordOffset) {
            GR_GL(VertexAttribPointer(TEX_ATTR_LOCATION, 2, scalarType,
                                      texCoordNorm, newStride, texCoordPtr));
        }
    } else if (oldTexCoordOffset > 0) {
        GR_GL(DisableVertexAttribArray(TEX_ATTR_LOCATION));
    }

    if (newColorOffset > 0) {
        GLvoid* colorPtr = (int8_t*)posPtr + newColorOffset;
        if (oldColorOffset <= 0) {
            GR_GL(EnableVertexAttribArray(COL_ATTR_LOCATION));
        }
        if (posChange || newColorOffset != oldColorOffset) {
            GR_GL(VertexAttribPointer(COL_ATTR_LOCATION, 4,
                                      GL_UNSIGNED_BYTE,
                                      true, newStride, colorPtr));
        }
    } else if (oldColorOffset > 0) {
        GR_GL(DisableVertexAttribArray(COL_ATTR_LOCATION));
    }

    fHWGeometryState.fVertexLayout = fGeometrySrc.fVertexLayout;
}
#endif

