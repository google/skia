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
#include "gl/GrGLUniformHandler.h"
#include "gl/GrGLVaryingHandler.h"
#include "glsl/GrGLSLProgramBuilder.h"
#include "glsl/GrGLSLProgramDataManager.h"
#include "ir/SkSLProgram.h"

class GrFragmentProcessor;
class GrGLContextInfo;
class GrProgramDesc;
class GrGLSLShaderBuilder;
class GrShaderCaps;

class GrGLProgramBuilder : public GrGLSLProgramBuilder {
public:
    /**
     * Abstract class which stores GLSL shader strings in a cache that persists between sessions.
     */
    class PersistentCache {
    public:
        struct Shader {
            SkSL::String fText;
            SkSL::Program::Inputs fInputs;
        };

        virtual ~PersistentCache() {}

        /**
         * If the shader for this key exists, returns true and stores the saved strings into
         * outVS (vertex), outGS (geometry), and outFS (fragment). If the shader was not cached,
         * returns false and leaves the strings unmodified.
         */
        virtual bool load(const GrProgramDesc& key, Shader* outVS, Shader* outGS,
                          Shader* outFS) = 0;

        virtual void store(const GrProgramDesc& key, const Shader& vs, const Shader& gs,
                           const Shader& fs) = 0;
    };

    /** Generates a shader program.
     *
     * The program implements what is specified in the stages given as input.
     * After successful generation, the builder result objects are available
     * to be used.
     * This function may modify the GrProgramDesc by setting the surface origin
     * key to 0 (unspecified) if it turns out the program does not care about
     * the surface origin.
     * @return true if generation was successful.
     */
    static GrGLProgram* CreateProgram(const GrPipeline&,
                                      const GrPrimitiveProcessor&,
                                      GrProgramDesc*,
                                      GrGLGpu*,
                                      PersistentCache*);

    const GrCaps* caps() const override;

    GrGLGpu* gpu() const { return fGpu; }

private:
    GrGLProgramBuilder(GrGLGpu*, const GrPipeline&, const GrPrimitiveProcessor&,
                       GrProgramDesc*, PersistentCache* cache);

    bool compileAndAttachShaders(const char* glsl,
                                 int length,
                                 GrGLuint programId,
                                 GrGLenum type,
                                 SkTDArray<GrGLuint>* shaderIds,
                                 const SkSL::Program::Settings& settings,
                                 const SkSL::Program::Inputs& inputs);

    bool compileAndAttachShaders(GrGLSLShaderBuilder& shader,
                                 GrGLuint programId,
                                 GrGLenum type,
                                 SkTDArray<GrGLuint>* shaderIds,
                                 const SkSL::Program::Settings& settings,
                                 SkSL::Program::Inputs* outInputs);
    GrGLProgram* finalize();
    void bindProgramResourceLocations(GrGLuint programID);
    bool checkLinkStatus(GrGLuint programID);
    void resolveProgramResourceLocations(GrGLuint programID);
    void cleanupProgram(GrGLuint programID, const SkTDArray<GrGLuint>& shaderIDs);
    void cleanupShaders(const SkTDArray<GrGLuint>& shaderIDs);

    // Subclasses create different programs
    GrGLProgram* createProgram(GrGLuint programID);

    GrGLSLUniformHandler* uniformHandler() override { return &fUniformHandler; }
    const GrGLSLUniformHandler* uniformHandler() const override { return &fUniformHandler; }
    GrGLSLVaryingHandler* varyingHandler() override { return &fVaryingHandler; }

    SkString getString(GrGLSLShaderBuilder& shader);

    GrGLGpu*              fGpu;
    GrGLVaryingHandler    fVaryingHandler;
    GrGLUniformHandler    fUniformHandler;

    PersistentCache* fCache;

    PersistentCache::Shader fCachedVS;
    PersistentCache::Shader fCachedGS;
    PersistentCache::Shader fCachedFS;

    typedef GrGLSLProgramBuilder INHERITED;
};
#endif
