/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLShaderBuilder_DEFINED
#define GrGLShaderBuilder_DEFINED

#include "gl/GrGLProgramDesc.h"
#include "gl/GrGLProgramEffects.h"
#include "gl/GrGLSL.h"
#include "gl/GrGLProgramDataManager.h"
#include "GrBackendProcessorFactory.h"
#include "GrColor.h"
#include "GrProcessor.h"
#include "SkTypes.h"

#include <stdarg.h>

class GrGLContextInfo;
class GrProcessorStage;
class GrGLProgramDesc;
class GrGLProgramBuilder;
class GrGLFullProgramBuilder;

/**
  base class for all shaders builders
*/
class GrGLShaderBuilder {
public:
    typedef GrGLProcessor::TransformedCoordsArray TransformedCoordsArray;
    typedef GrGLProcessor::TextureSampler TextureSampler;
    GrGLShaderBuilder(GrGLProgramBuilder* program);

    void addInput(GrGLShaderVar i) { fInputs.push_back(i); }
    void addOutput(GrGLShaderVar i) { fOutputs.push_back(i); }

    /*
     * We put texture lookups in the base class because it is TECHNICALLY possible to do texture
     * lookups in any kind of shader.  However, for the time being using these calls on non-fragment
     * shaders will result in a shader compilation error as texture sampler uniforms are only
     * visible to the fragment shader.  It would not be hard to change this behavior, if someone
     * actually wants to do texture lookups in a non-fragment shader
     *
     * TODO if append texture lookup is used on a non-fragment shader, sampler uniforms should be
     * made visible to that shaders
     */
    /** Appends a 2D texture sample with projection if necessary. coordType must either be Vec2f or
        Vec3f. The latter is interpreted as projective texture coords. The vec length and swizzle
        order of the result depends on the GrTextureAccess associated with the TextureSampler. */
    void appendTextureLookup(SkString* out,
                             const TextureSampler&,
                             const char* coordName,
                             GrSLType coordType = kVec2f_GrSLType) const;

    /** Version of above that appends the result to the fragment shader code instead.*/
    void appendTextureLookup(const TextureSampler&,
                             const char* coordName,
                             GrSLType coordType = kVec2f_GrSLType);


    /** Does the work of appendTextureLookup and modulates the result by modulation. The result is
        always a vec4. modulation and the swizzle specified by TextureSampler must both be vec4 or
        float. If modulation is "" or NULL it this function acts as though appendTextureLookup were
        called. */
    void appendTextureLookupAndModulate(const char* modulation,
                                        const TextureSampler&,
                                        const char* coordName,
                                        GrSLType coordType = kVec2f_GrSLType);

    /** If texture swizzling is available using tex parameters then it is preferred over mangling
        the generated shader code. This potentially allows greater reuse of cached shaders. */
    static const GrGLenum* GetTexParamSwizzle(GrPixelConfig config, const GrGLCaps& caps);

    /**
    * Called by GrGLProcessors to add code to one of the shaders.
    */
    void codeAppendf(const char format[], ...) SK_PRINTF_LIKE(2, 3) {
       va_list args;
       va_start(args, format);
       fCode.appendVAList(format, args);
       va_end(args);
    }

    void codeAppend(const char* str) { fCode.append(str); }

    void codePrependf(const char format[], ...) SK_PRINTF_LIKE(2, 3) {
       va_list args;
       va_start(args, format);
       fCode.prependVAList(format, args);
       va_end(args);
    }

    /**
     * Appends a variable declaration to one of the shaders
     */
    void declAppend(const GrGLShaderVar& var);

    /** Emits a helper function outside of main() in the fragment shader. */
    void emitFunction(GrSLType returnType,
                      const char* name,
                      int argCnt,
                      const GrGLShaderVar* args,
                      const char* body,
                      SkString* outName);

    /*
     * Get parent builder for adding uniforms
     */
    GrGLProgramBuilder* getProgramBuilder() { return fProgramBuilder; }

    /**
     * Helper for begining and ending a block in the fragment code.
     */
    class ShaderBlock {
    public:
        ShaderBlock(GrGLShaderBuilder* builder) : fBuilder(builder) {
            SkASSERT(builder);
            fBuilder->codeAppend("{");
        }

        ~ShaderBlock() {
            fBuilder->codeAppend("}");
        }
    private:
        GrGLShaderBuilder* fBuilder;
    };
protected:

    /*
     * this super low level function is just for use internally to builders
     */
    void appendTextureLookup(const char* samplerName,
                             const char* coordName,
                             uint32_t configComponentMask,
                             const char* swizzle);

    /*
     * A general function which enables an extension in a shader if the feature bit is not present
     */
    void addFeature(uint32_t featureBit, const char* extensionName);

    typedef GrTAllocator<GrGLShaderVar> VarArray;

    GrGLProgramBuilder* fProgramBuilder;

    SkString fCode;
    SkString fFunctions;
    SkString fExtensions;

    VarArray fInputs;
    VarArray fOutputs;
    uint32_t fFeaturesAddedMask;
};


/*
 * Full Shader builder is the base class for shaders which are only accessible through full program
 * builder, ie vertex, geometry, and later TCU / TES.  Using this base class, they can access the
 * full program builder functionality through the full program pointer
 */
class GrGLFullShaderBuilder : public GrGLShaderBuilder {
public:
    GrGLFullShaderBuilder(GrGLFullProgramBuilder* program);

    GrGLFullProgramBuilder* fullProgramBuilder() { return fFullProgramBuilder; }
protected:
    GrGLFullProgramBuilder* fFullProgramBuilder;
private:
    typedef GrGLShaderBuilder INHERITED;
};
#endif
