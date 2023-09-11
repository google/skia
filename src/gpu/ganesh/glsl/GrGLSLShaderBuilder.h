/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLSLShaderBuilder_DEFINED
#define GrGLSLShaderBuilder_DEFINED

#include "include/core/SkSpan.h"
#include "include/private/base/SkTDArray.h"
#include "src/base/SkTBlockList.h"
#include "src/gpu/ganesh/GrShaderVar.h"
#include "src/gpu/ganesh/glsl/GrGLSLUniformHandler.h"
#include "src/sksl/SkSLString.h"
#include "src/sksl/ir/SkSLStatement.h"

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

    /** Appends a 2D texture sample with projection if necessary. The vec length and swizzle
        order of the result depends on the GrProcessor::TextureSampler associated with the
        SamplerHandle.
        */
    void appendTextureLookup(SkString* out, SamplerHandle, const char* coordName) const;

    /** Version of above that appends the result to the shader code instead.*/
    void appendTextureLookup(SamplerHandle,
                             const char* coordName,
                             GrGLSLColorSpaceXformHelper* colorXformHelper = nullptr);

    /** Does the work of appendTextureLookup and blends the result by dst, treating the texture
        lookup as the src input to the blend. The dst is assumed to be half4 and the result is
        always a half4. If dst is nullptr we use half4(1) as the blend dst. */
    void appendTextureLookupAndBlend(const char* dst,
                                     SkBlendMode,
                                     SamplerHandle,
                                     const char* coordName,
                                     GrGLSLColorSpaceXformHelper* colorXformHelper = nullptr);

    /** Appends a load of an input attachment into the shader code. */
    void appendInputLoad(SamplerHandle);

    /** Adds a helper function to facilitate color gamut transformation, and produces code that
        returns the srcColor transformed into a new gamut (via multiplication by the xform from
        colorXformHelper). Premultiplied sources are also handled correctly (colorXformHelper
        determines if the source is premultipled or not). */
    void appendColorGamutXform(SkString* out, const char* srcColor,
                               GrGLSLColorSpaceXformHelper* colorXformHelper);

    /** Version of above that appends the result to the shader code instead. */
    void appendColorGamutXform(const char* srcColor, GrGLSLColorSpaceXformHelper* colorXformHelper);

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

    void defineConstantf(const char* type, const char* name, const char* fmt, ...)
            SK_PRINTF_LIKE(4, 5) {
        this->definitions().appendf("const %s %s = ", type, name);
        va_list args;
        va_start(args, fmt);
        this->definitions().appendVAList(fmt, args);
        va_end(args);
        this->definitions().append(";\n");
    }

    void definitionAppend(const char* str) { this->definitions().append(str); }

    void declareGlobal(const GrShaderVar&);

    // Generates a unique variable name for holding the result of a temporary expression when it's
    // not reasonable to just add a new block for scoping. Does not declare anything.
    SkString newTmpVarName(const char* suffix) {
        int tmpIdx = fTmpVariableCounter++;
        return SkStringPrintf("_tmp_%d_%s", tmpIdx, suffix);
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

    void codeAppend(const char* str, size_t length) { this->code().append(str, length); }

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

    /**
     * Generates a mangled name for a helper function in the fragment shader. Will give consistent
     * results if called more than once.
     */
    SkString getMangledFunctionName(const char* baseName);

    /** Emits a prototype for a helper function outside of main() in the fragment shader. */
    void emitFunctionPrototype(SkSLType returnType,
                               const char* mangledName,
                               SkSpan<const GrShaderVar> args);

    void emitFunctionPrototype(const char* declaration);

    /** Emits a helper function outside of main() in the fragment shader. */
    void emitFunction(SkSLType returnType,
                      const char* mangledName,
                      SkSpan<const GrShaderVar> args,
                      const char* body);

    void emitFunction(const char* declaration, const char* body);

    /**
     * Combines the various parts of the shader to create a single finalized shader string.
     */
    void finalize(uint32_t visibility);

    /**
     * Get parent builder for adding uniforms.
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
    typedef SkTBlockList<GrShaderVar> VarArray;
    void appendDecls(const VarArray& vars, SkString* out) const;

    void appendFunctionDecl(SkSLType returnType,
                            const char* mangledName,
                            SkSpan<const GrShaderVar> args);

    /**
     * Features that should only be enabled internally by the builders.
     */
    enum GLSLPrivateFeature {
        kFragCoordConventions_GLSLPrivateFeature,
        kBlendEquationAdvanced_GLSLPrivateFeature,
        kBlendFuncExtended_GLSLPrivateFeature,
        kFramebufferFetch_GLSLPrivateFeature,
        kNoPerspectiveInterpolation_GLSLPrivateFeature,
        kSampleVariables_GLSLPrivateFeature,
        kLastGLSLPrivateFeature = kSampleVariables_GLSLPrivateFeature
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
        fCodeIndex++;
    }

    void deleteStage() {
        fShaderStrings.pop_back();
        fCodeIndex--;
    }

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

        kPrealloc = kCode + 6,  // 6 == Reasonable upper bound on number of processor stages
    };

    GrGLSLProgramBuilder* fProgramBuilder;
    std::string fCompilerString;
    skia_private::STArray<kPrealloc, SkString> fShaderStrings;
    SkString fCode;
    SkString fFunctions;
    SkString fExtensions;
    // Hangs onto Declarations so we don't destroy them prior to the variables that refer to them.
    SkSL::StatementArray fDeclarations;

    VarArray fInputs;
    VarArray fOutputs;
    uint32_t fFeaturesAddedMask;
    skia_private::STArray<1, SkString> fLayoutParams[kLastInterfaceQualifier + 1];
    int fCodeIndex;
    bool fFinalized;

    // Counter for generating unique scratch variable names in a shader.
    int fTmpVariableCounter;

    friend class GrGLSLProgramBuilder;
    friend class GrGLProgramBuilder;
    friend class GrD3DPipelineStateBuilder;
    friend class GrDawnProgramBuilder;
    friend class GrGLSLVaryingHandler; // to access noperspective interpolation feature.
    friend class GrGLPathProgramBuilder; // to access fInputs.
    friend class GrVkPipelineStateBuilder;
    friend class GrMtlPipelineStateBuilder;
};
#endif
