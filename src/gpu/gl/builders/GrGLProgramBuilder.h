/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLProgramBuilder_DEFINED
#define GrGLProgramBuilder_DEFINED

#include "GrPipeline.h"
#include "gl/GrGLProgramDataManager.h"
#include "gl/GrGLVaryingHandler.h"
#include "glsl/GrGLSLPrimitiveProcessor.h"
#include "glsl/GrGLSLProgramBuilder.h"
#include "glsl/GrGLSLProgramDataManager.h"
#include "glsl/GrGLSLTextureSampler.h"
#include "glsl/GrGLSLXferProcessor.h"

class GrFragmentProcessor;
class GrGLContextInfo;
class GrGLSLShaderBuilder;
class GrGLSLCaps;

/**
 * The below struct represent processors installed in programs.
 */
template <class Proc>
struct GrGLInstalledProc {
    SkDEBUGCODE(int fSamplersIdx;)
    SkAutoTDelete<Proc> fGLProc;
};

typedef GrGLInstalledProc<GrGLSLPrimitiveProcessor> GrGLInstalledGeoProc;
typedef GrGLInstalledProc<GrGLSLXferProcessor> GrGLInstalledXferProc;
typedef GrGLInstalledProc<GrGLSLFragmentProcessor> GrGLInstalledFragProc;

struct GrGLInstalledFragProcs : public SkRefCnt {
    virtual ~GrGLInstalledFragProcs();
    SkSTArray<8, GrGLInstalledFragProc*, true> fProcs;
};

/*
 * Please note - no diamond problems because of virtual inheritance.  Also, both base classes
 * are pure virtual with no data members.  This is the base class for program building.
 * Subclasses are nearly identical but each has their own way of emitting transforms.  State for
 * each of the elements of the shader pipeline, ie vertex, fragment, geometry, etc, lives in those
 * respective builders
*/
class GrGLProgramBuilder : public GrGLSLProgramBuilder {
public:
    /** Generates a shader program.
     *
     * The program implements what is specified in the stages given as input.
     * After successful generation, the builder result objects are available
     * to be used.
     * @return true if generation was successful.
     */
    static GrGLProgram* CreateProgram(const DrawArgs&, GrGLGpu*);

    const GrGLSLShaderVar& getUniformVariable(UniformHandle u) const override {
        return fUniforms[u.toIndex()].fVariable;
    }

    const char* getUniformCStr(UniformHandle u) const override {
        return this->getUniformVariable(u).c_str();
    }

    const GrGLSLCaps* glslCaps() const override;

    GrGLGpu* gpu() const { return fGpu; }

private:
    typedef GrGLProgramDataManager::UniformInfo UniformInfo;
    typedef GrGLProgramDataManager::UniformInfoArray UniformInfoArray;

    GrGLProgramBuilder(GrGLGpu*, const DrawArgs&);

    UniformHandle internalAddUniformArray(uint32_t visibility,
                                          GrSLType type,
                                          GrSLPrecision precision,
                                          const char* name,
                                          bool mangleName,
                                          int arrayCount,
                                          const char** outName) override;

    // Generates a possibly mangled name for a stage variable and writes it to the fragment shader.
    // If GrGLSLExpr4 has a valid name then it will use that instead
    void nameExpression(GrGLSLExpr4*, const char* baseName);
    bool emitAndInstallProcs(GrGLSLExpr4* inputColor, GrGLSLExpr4* inputCoverage);
    void emitAndInstallFragProcs(int procOffset, int numProcs, GrGLSLExpr4* inOut);
    void emitAndInstallProc(const GrFragmentProcessor&,
                            int index,
                            const GrGLSLExpr4& input,
                            GrGLSLExpr4* output);

    void emitAndInstallProc(const GrPrimitiveProcessor&,
                            GrGLSLExpr4* outputColor,
                            GrGLSLExpr4* outputCoverage);

    // these emit functions help to keep the createAndEmitProcessors template general
    void emitAndInstallProc(const GrFragmentProcessor&,
                            int index,
                            const char* outColor,
                            const char* inColor);
    void emitAndInstallProc(const GrPrimitiveProcessor&,
                            const char* outColor,
                            const char* outCoverage);
    void emitAndInstallXferProc(const GrXferProcessor&,
                                const GrGLSLExpr4& colorIn,
                                const GrGLSLExpr4& coverageIn,
                                bool ignoresCoverage);

    void verify(const GrPrimitiveProcessor&);
    void verify(const GrXferProcessor&);
    void verify(const GrFragmentProcessor&);
    template <class Proc>
    void emitSamplers(const GrProcessor&,
                      GrGLSLTextureSampler::TextureSamplerArray* outSamplers,
                      GrGLInstalledProc<Proc>*);

    bool compileAndAttachShaders(GrGLSLShaderBuilder& shader,
                                 GrGLuint programId,
                                 GrGLenum type,
                                 SkTDArray<GrGLuint>* shaderIds); 
    GrGLProgram* finalize();
    void bindProgramResourceLocations(GrGLuint programID);
    bool checkLinkStatus(GrGLuint programID);
    void resolveProgramResourceLocations(GrGLuint programID);
    void cleanupProgram(GrGLuint programID, const SkTDArray<GrGLuint>& shaderIDs);
    void cleanupShaders(const SkTDArray<GrGLuint>& shaderIDs);

    // Subclasses create different programs
    GrGLProgram* createProgram(GrGLuint programID);

    void onAppendUniformDecls(ShaderVisibility visibility, SkString* out) const override;

    GrGLSLVaryingHandler* varyingHandler() override { return &fVaryingHandler; }

    // reset is called by program creator between each processor's emit code.  It increments the
    // stage offset for variable name mangling, and also ensures verfication variables in the
    // fragment shader are cleared.
    void reset() {
        this->addStage();
        fFS.reset();
    }
    void addStage() { fStageIndex++; }

    class AutoStageAdvance {
    public:
        AutoStageAdvance(GrGLProgramBuilder* pb)
            : fPB(pb) {
            fPB->reset();
            // Each output to the fragment processor gets its own code section
            fPB->fFS.nextStage();
        }
        ~AutoStageAdvance() {}
    private:
        GrGLProgramBuilder* fPB;
    };

    GrGLInstalledGeoProc* fGeometryProcessor;
    GrGLInstalledXferProc* fXferProcessor;
    SkAutoTUnref<GrGLInstalledFragProcs> fFragmentProcessors;

    GrGLGpu* fGpu;
    UniformInfoArray fUniforms;
    GrGLSLPrimitiveProcessor::TransformsIn fCoordTransforms;
    GrGLSLPrimitiveProcessor::TransformsOut fOutCoords;
    SkTArray<UniformHandle> fSamplerUniforms;

    GrGLVaryingHandler        fVaryingHandler;

    friend class GrGLVaryingHandler; 

    typedef GrGLSLProgramBuilder INHERITED; 
};
#endif
