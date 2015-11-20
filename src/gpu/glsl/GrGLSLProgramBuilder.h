/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLSLProgramBuilder_DEFINED
#define GrGLSLProgramBuilder_DEFINED

#include "GrGeometryProcessor.h"
#include "GrGpu.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"
#include "glsl/GrGLSLGeometryShaderBuilder.h"
#include "glsl/GrGLSLProgramDataManager.h"
#include "glsl/GrGLSLVertexShaderBuilder.h"

class GrGLSLCaps;
class GrGLSLShaderVar;
class GrGLSLVaryingHandler;

// Enough precision to represent 1 / 2048 accurately in printf
#define GR_SIGNIFICANT_POW2_DECIMAL_DIG 11

class GrGLSLUniformBuilder {
public:
    enum ShaderVisibility {
        kVertex_Visibility   = 1 << kVertex_GrShaderType,
        kGeometry_Visibility = 1 << kGeometry_GrShaderType,
        kFragment_Visibility = 1 << kFragment_GrShaderType,
    };

    virtual ~GrGLSLUniformBuilder() {}

    typedef GrGLSLProgramDataManager::UniformHandle UniformHandle;

    /** Add a uniform variable to the current program, that has visibility in one or more shaders.
        visibility is a bitfield of ShaderVisibility values indicating from which shaders the
        uniform should be accessible. At least one bit must be set. Geometry shader uniforms are not
        supported at this time. The actual uniform name will be mangled. If outName is not nullptr
        then it will refer to the final uniform name after return. Use the addUniformArray variant
        to add an array of uniforms. */
    UniformHandle addUniform(uint32_t visibility,
                             GrSLType type,
                             GrSLPrecision precision,
                             const char* name,
                             const char** outName = nullptr) {
        return this->addUniformArray(visibility, type, precision, name, 0, outName);
    }

    UniformHandle addUniformArray(uint32_t visibility,
                                  GrSLType type,
                                  GrSLPrecision precision,
                                  const char* name,
                                  int arrayCount,
                                  const char** outName = nullptr) {
        return this->internalAddUniformArray(visibility, type, precision, name, true, arrayCount,
                                             outName);
    }

    virtual const GrGLSLShaderVar& getUniformVariable(UniformHandle u) const = 0;

    /**
     * Shortcut for getUniformVariable(u).c_str()
     */
    virtual const char* getUniformCStr(UniformHandle u) const = 0;

    /*
     * *NOTE* NO MEMBERS ALLOWED, MULTIPLE INHERITANCE
     */
protected:
    virtual UniformHandle internalAddUniformArray(
        uint32_t visibility,
        GrSLType type,
        GrSLPrecision precision,
        const char* name,
        bool mangleName,
        int arrayCount,
        const char** outName) = 0;
};

/* a specialization of the above for GPs.  Lets the user add uniforms, varyings, and VS / FS code */
class GrGLSLGPBuilder : public virtual GrGLSLUniformBuilder {
public:
    /*
     * *NOTE* NO MEMBERS ALLOWED, MULTIPLE INHERITANCE
     */
};


/* a specializations for FPs. Lets the user add uniforms and FS code */
class GrGLSLFPBuilder : public virtual GrGLSLUniformBuilder {
public:
    /*
     * *NOTE* NO MEMBERS ALLOWED, MULTIPLE INHERITANCE
     */
};

/* a specializations for XPs. Lets the user add uniforms and FS code */
class GrGLSLXPBuilder : public virtual GrGLSLUniformBuilder {
public:
    /*
     * *NOTE* NO MEMBERS ALLOWED, MULTIPLE INHERITANCE
     */
};

class GrGLSLProgramBuilder : public GrGLSLGPBuilder,
                             public GrGLSLFPBuilder,
                             public GrGLSLXPBuilder {
public:
    typedef GrGpu::DrawArgs DrawArgs;

    virtual const GrGLSLCaps* glslCaps() const = 0;

    // Handles for program uniforms (other than per-effect uniforms)
    struct BuiltinUniformHandles {
        UniformHandle       fRTAdjustmentUni;

        // We use the render target height to provide a y-down frag coord when specifying
        // origin_upper_left is not supported.
        UniformHandle       fRTHeightUni;
    };

protected:
    explicit GrGLSLProgramBuilder(const DrawArgs& args);

    const GrPrimitiveProcessor& primitiveProcessor() const { return *fArgs.fPrimitiveProcessor; }
    const GrPipeline& pipeline() const { return *fArgs.fPipeline; }
    const GrProgramDesc& desc() const { return *fArgs.fDesc; }
    const GrProgramDesc::KeyHeader& header() const { return fArgs.fDesc->header(); }

    void appendUniformDecls(ShaderVisibility, SkString*) const;

    // Used to add a uniform for frag position without mangling the name of the uniform inside of a
    // stage.
    UniformHandle addFragPosUniform(uint32_t visibility,
                                    GrSLType type,
                                    GrSLPrecision precision,
                                    const char* name,
                                    const char** outName) {
        return this->internalAddUniformArray(visibility, type, precision, name, false, 0, outName);
    }

    const char* rtAdjustment() const { return "rtAdjustment"; }

    // Generates a name for a variable. The generated string will be name prefixed by the prefix
    // char (unless the prefix is '\0'). It also will mangle the name to be stage-specific unless
    // explicitly asked not to.
    void nameVariable(SkString* out, char prefix, const char* name, bool mangle = true);

    virtual GrGLSLVaryingHandler* varyingHandler() = 0;

    // number of each input/output type in a single allocation block, used by many builders
    static const int kVarsPerBlock;

    GrGLSLVertexBuilder         fVS;
    GrGLSLGeometryBuilder       fGS;
    GrGLSLFragmentShaderBuilder fFS;

    int fStageIndex;

    BuiltinUniformHandles fUniformHandles;

    const DrawArgs& fArgs;

private:
    virtual void onAppendUniformDecls(ShaderVisibility visibility, SkString* out) const = 0;

    friend class GrGLSLShaderBuilder;
    friend class GrGLSLVertexBuilder;
    friend class GrGLSLFragmentShaderBuilder;
    friend class GrGLSLGeometryBuilder;
    friend class GrGLSLVaryingHandler;
};

#endif
