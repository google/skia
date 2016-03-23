/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLSLShaderBuilder_DEFINED
#define GrGLSLShaderBuilder_DEFINED

#include "GrAllocator.h"
#include "glsl/GrGLSLShaderVar.h"
#include "SkTDArray.h"

#include <stdarg.h>

class GrGLSLProgramBuilder;
class GrGLSLTextureSampler;

/**
  base class for all shaders builders
*/
class GrGLSLShaderBuilder {
public:
    GrGLSLShaderBuilder(GrGLSLProgramBuilder* program);
    virtual ~GrGLSLShaderBuilder() {}

    /** Appends a 2D texture sample with projection if necessary. coordType must either be Vec2f or
        Vec3f. The latter is interpreted as projective texture coords. The vec length and swizzle
        order of the result depends on the GrTextureAccess associated with the GrGLSLTextureSampler.
        */
    void appendTextureLookup(SkString* out,
                             const GrGLSLTextureSampler&,
                             const char* coordName,
                             GrSLType coordType = kVec2f_GrSLType) const;

    /** Version of above that appends the result to the fragment shader code instead.*/
    void appendTextureLookup(const GrGLSLTextureSampler&,
                             const char* coordName,
                             GrSLType coordType = kVec2f_GrSLType);


    /** Does the work of appendTextureLookup and modulates the result by modulation. The result is
        always a vec4. modulation and the swizzle specified by GrGLSLTextureSampler must both be
        vec4 or float. If modulation is "" or nullptr it this function acts as though
        appendTextureLookup were called. */
    void appendTextureLookupAndModulate(const char* modulation,
                                        const GrGLSLTextureSampler&,
                                        const char* coordName,
                                        GrSLType coordType = kVec2f_GrSLType);

    /**
    * Adds a #define directive to the top of the shader.
    */
    void define(const char* macro, const char* replacement) {
        this->definitions().appendf("#define %s %s\n", macro, replacement);
    }

    void define(const char* macro, int replacement) {
        this->definitions().appendf("#define %s %i\n", macro, replacement);
    }

    void definef(const char* macro, const char* replacement, ...) {
       this->definitions().appendf("#define %s ", macro);
       va_list args;
       va_start(args, replacement);
       this->definitions().appendVAList(replacement, args);
       va_end(args);
       this->definitions().append("\n");
    }

    /**
    * Called by GrGLSLProcessors to add code to one of the shaders.
    */
    void codeAppendf(const char format[], ...) SK_PRINTF_LIKE(2, 3) {
       va_list args;
       va_start(args, format);
       this->code().appendVAList(format, args);
       va_end(args);
    }

    void codeAppend(const char* str) { this->code().append(str); }

    void codePrependf(const char format[], ...) SK_PRINTF_LIKE(2, 3) {
       va_list args;
       va_start(args, format);
       this->code().prependVAList(format, args);
       va_end(args);
    }

    /**
     * Appends a variable declaration to one of the shaders
     */
    void declAppend(const GrGLSLShaderVar& var);

    /** Emits a helper function outside of main() in the fragment shader. */
    void emitFunction(GrSLType returnType,
                      const char* name,
                      int argCnt,
                      const GrGLSLShaderVar* args,
                      const char* body,
                      SkString* outName);

    /*
     * Combines the various parts of the shader to create a single finalized shader string.
     */
    void finalize(uint32_t visibility);

    /*
     * Get parent builder for adding uniforms
     */
    GrGLSLProgramBuilder* getProgramBuilder() { return fProgramBuilder; }

    /**
     * Helper for begining and ending a block in the shader code.
     */
    class ShaderBlock {
    public:
        ShaderBlock(GrGLSLShaderBuilder* builder) : fBuilder(builder) {
            SkASSERT(builder);
            fBuilder->codeAppend("{");
        }

        ~ShaderBlock() {
            fBuilder->codeAppend("}");
        }
    private:
        GrGLSLShaderBuilder* fBuilder;
    };

protected:
    typedef GrTAllocator<GrGLSLShaderVar> VarArray;
    void appendDecls(const VarArray& vars, SkString* out) const;

    /**
     * Features that should only be enabled internally by the builders.
     */
    enum GLSLPrivateFeature {
        kFragCoordConventions_GLSLPrivateFeature,
        kBlendEquationAdvanced_GLSLPrivateFeature,
        kBlendFuncExtended_GLSLPrivateFeature,
        kExternalTexture_GLSLPrivateFeature,
        kFramebufferFetch_GLSLPrivateFeature,
        kNoPerspectiveInterpolation_GLSLPrivateFeature,
        kSampleVariables_GLSLPrivateFeature,
        kSampleMaskOverrideCoverage_GLSLPrivateFeature,
        kLastGLSLPrivateFeature = kSampleMaskOverrideCoverage_GLSLPrivateFeature
    };

    /*
     * A general function which enables an extension in a shader if the feature bit is not present
     *
     * @return true if the feature bit was not yet present, false otherwise.
     */
    bool addFeature(uint32_t featureBit, const char* extensionName);

    enum InterfaceQualifier {
        kOut_InterfaceQualifier,
        kLastInterfaceQualifier = kOut_InterfaceQualifier
    };

    /*
     * A low level function to build default layout qualifiers.
     *
     *   e.g. layout(param1, param2, ...) out;
     *
     * GLSL allows default layout qualifiers for in, out, and uniform.
     */
    void addLayoutQualifier(const char* param, InterfaceQualifier);

    void compileAndAppendLayoutQualifiers();

    void nextStage() {
        fShaderStrings.push_back();
        fCompilerStrings.push_back(this->code().c_str());
        fCompilerStringLengths.push_back((int)this->code().size());
        fCodeIndex++;
    }

    SkString& versionDecl() { return fShaderStrings[kVersionDecl]; }
    SkString& extensions() { return fShaderStrings[kExtensions]; }
    SkString& definitions() { return fShaderStrings[kDefinitions]; }
    SkString& precisionQualifier() { return fShaderStrings[kPrecisionQualifier]; }
    SkString& layoutQualifiers() { return fShaderStrings[kLayoutQualifiers]; }
    SkString& uniforms() { return fShaderStrings[kUniforms]; }
    SkString& inputs() { return fShaderStrings[kInputs]; }
    SkString& outputs() { return fShaderStrings[kOutputs]; }
    SkString& functions() { return fShaderStrings[kFunctions]; }
    SkString& main() { return fShaderStrings[kMain]; }
    SkString& code() { return fShaderStrings[fCodeIndex]; }

    virtual void onFinalize() = 0;

    enum {
        kVersionDecl,
        kExtensions,
        kDefinitions,
        kPrecisionQualifier,
        kLayoutQualifiers,
        kUniforms,
        kInputs,
        kOutputs,
        kFunctions,
        kMain,
        kCode,
    };

    GrGLSLProgramBuilder* fProgramBuilder;
    SkSTArray<kCode, const char*, true> fCompilerStrings;
    SkSTArray<kCode, int, true> fCompilerStringLengths;
    SkSTArray<kCode, SkString> fShaderStrings;
    SkString fCode;
    SkString fFunctions;
    SkString fExtensions;

    VarArray fInputs;
    VarArray fOutputs;
    uint32_t fFeaturesAddedMask;
    SkSTArray<1, SkString> fLayoutParams[kLastInterfaceQualifier + 1];
    int fCodeIndex;
    bool fFinalized;

    friend class GrGLSLProgramBuilder;
    friend class GrGLProgramBuilder;
    friend class GrGLSLVaryingHandler; // to access noperspective interpolation feature.
    friend class GrGLPathProgramBuilder; // to access fInputs.
    friend class GrVkPipelineStateBuilder;
};
#endif
