/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLSLShaderBuilder_DEFINED
#define GrGLSLShaderBuilder_DEFINED

#include "GrAllocator.h"
#include "GrShaderVar.h"
#include "glsl/GrGLSLUniformHandler.h"
#include "SkTDArray.h"

#include <stdarg.h>

class GrGLSLColorSpaceXformHelper;

/**
  base class for all shaders builders
*/
class GrGLSLShaderBuilder {
public:
    GrGLSLShaderBuilder(GrGLSLProgramBuilder* program);
    virtual ~GrGLSLShaderBuilder() {}

    using SamplerHandle      = GrGLSLUniformHandler::SamplerHandle;
    using ImageStorageHandle = GrGLSLUniformHandler::ImageStorageHandle;

    /** Appends a 2D texture sample with projection if necessary. coordType must either be Vec2f or
        Vec3f. The latter is interpreted as projective texture coords. The vec length and swizzle
        order of the result depends on the GrProcessor::TextureSampler associated with the
        SamplerHandle.
        */
    void appendTextureLookup(SkString* out,
                             SamplerHandle,
                             const char* coordName,
                             GrSLType coordType = kVec2f_GrSLType) const;

    /** Version of above that appends the result to the shader code instead.*/
    void appendTextureLookup(SamplerHandle,
                             const char* coordName,
                             GrSLType coordType = kVec2f_GrSLType,
                             GrGLSLColorSpaceXformHelper* colorXformHelper = nullptr);


    /** Does the work of appendTextureLookup and modulates the result by modulation. The result is
        always a vec4. modulation and the swizzle specified by SamplerHandle must both be
        vec4 or float. If modulation is "" or nullptr it this function acts as though
        appendTextureLookup were called. */
    void appendTextureLookupAndModulate(const char* modulation,
                                        SamplerHandle,
                                        const char* coordName,
                                        GrSLType coordType = kVec2f_GrSLType,
                                        GrGLSLColorSpaceXformHelper* colorXformHelper = nullptr);

    /** Adds a helper function to facilitate color gamut transformation, and produces code that
        returns the srcColor transformed into a new gamut (via multiplication by the xform from
        colorXformHelper). Premultiplied sources are also handled correctly (colorXformHelper
        determines if the source is premultipled or not). */
    void appendColorGamutXform(SkString* out, const char* srcColor,
                               GrGLSLColorSpaceXformHelper* colorXformHelper);

    /** Version of above that appends the result to the shader code instead. */
    void appendColorGamutXform(const char* srcColor, GrGLSLColorSpaceXformHelper* colorXformHelper);

    /** Fetches an unfiltered texel from a sampler at integer coordinates. coordExpr must match the
        dimensionality of the sampler and must be within the sampler's range. coordExpr is emitted
        exactly once, so expressions like "idx++" are acceptable. */
    void appendTexelFetch(SkString* out, SamplerHandle, const char* coordExpr) const;

    /** Version of above that appends the result to the shader code instead.*/
    void appendTexelFetch(SamplerHandle, const char* coordExpr);

    /** Creates a string of shader code that performs an image load. */
    void appendImageStorageLoad(SkString* out, ImageStorageHandle, const char* coordExpr);
    /** Version of above that appends the result to the shader code instead. */
    void appendImageStorageLoad(ImageStorageHandle, const char* coordExpr);

    /**
    * Adds a constant declaration to the top of the shader.
    */
    void defineConstant(const char* type, const char* name, const char* value) {
        this->definitions().appendf("const %s %s = %s;\n", type, name, value);
    }

    void defineConstant(const char* name, int value) {
        this->definitions().appendf("const int %s = %i;\n", name, value);
    }

    void defineConstant(const char* name, float value) {
        this->definitions().appendf("const float %s = %f;\n", name, value);
    }

    void defineConstantf(const char* type, const char* name, const char* fmt, ...) {
       this->definitions().appendf("const %s %s = ", type, name);
       va_list args;
       va_start(args, fmt);
       this->definitions().appendVAList(fmt, args);
       va_end(args);
       this->definitions().append(";\n");
    }

    void declareGlobal(const GrShaderVar&);

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
    void declAppend(const GrShaderVar& var);

    /** Emits a helper function outside of main() in the fragment shader. */
    void emitFunction(GrSLType returnType,
                      const char* name,
                      int argCnt,
                      const GrShaderVar* args,
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
    typedef GrTAllocator<GrShaderVar> VarArray;
    void appendDecls(const VarArray& vars, SkString* out) const;

    /**
     * Features that should only be enabled internally by the builders.
     */
    enum GLSLPrivateFeature {
        kFragCoordConventions_GLSLPrivateFeature,
        kBlendEquationAdvanced_GLSLPrivateFeature,
        kBlendFuncExtended_GLSLPrivateFeature,
        kExternalTexture_GLSLPrivateFeature,
        kTexelBuffer_GLSLPrivateFeature,
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
        kIn_InterfaceQualifier,
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
