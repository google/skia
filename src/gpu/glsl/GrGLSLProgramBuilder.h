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
#include "gl/builders/GrGLFragmentShaderBuilder.h"
#include "gl/builders/GrGLGeometryShaderBuilder.h"
#include "gl/builders/GrGLVertexShaderBuilder.h"
#include "glsl/GrGLSLProgramDataManager.h"

class GrGLSLCaps;
class GrGLSLShaderVar;

class GrGLSLUniformBuilder {
public:
    enum ShaderVisibility {
        kVertex_Visibility   = 1 << kVertex_GrShaderType,
        kGeometry_Visibility = 1 << kGeometry_GrShaderType,
        kFragment_Visibility = 1 << kFragment_GrShaderType,
    };

    virtual ~GrGLSLUniformBuilder() {}

    typedef GrGLSLProgramDataManager::UniformHandle UniformHandle;
    typedef GrGLSLProgramDataManager::SeparableVaryingHandle SeparableVaryingHandle;

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

    virtual const GrGLSLCaps* glslCaps() const = 0;

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

// TODO move this into GrGLSLGPBuilder and move them both out of this file
class GrGLSLVarying {
public:
    bool vsVarying() const { return kVertToFrag_Varying == fVarying ||
                                    kVertToGeo_Varying == fVarying; }
    bool fsVarying() const { return kVertToFrag_Varying == fVarying ||
                                    kGeoToFrag_Varying == fVarying; }
    const char* vsOut() const { return fVsOut; }
    const char* gsIn() const { return fGsIn; }
    const char* gsOut() const { return fGsOut; }
    const char* fsIn() const { return fFsIn; }
    GrSLType type() const { return fType; }

protected:
    enum Varying {
        kVertToFrag_Varying,
        kVertToGeo_Varying,
        kGeoToFrag_Varying,
    };

    GrGLSLVarying(GrSLType type, Varying varying)
        : fVarying(varying), fType(type), fVsOut(nullptr), fGsIn(nullptr), fGsOut(nullptr),
          fFsIn(nullptr) {}

    Varying fVarying;

private:
    GrSLType fType;
    const char* fVsOut;
    const char* fGsIn;
    const char* fGsOut;
    const char* fFsIn;

    friend class GrGLVertexBuilder;
    friend class GrGLGeometryBuilder;
    friend class GrGLXferBuilder;
    friend class GrGLFragmentShaderBuilder;
};

struct GrGLSLVertToFrag : public GrGLSLVarying {
    GrGLSLVertToFrag(GrSLType type)
        : GrGLSLVarying(type, kVertToFrag_Varying) {}
};

struct GrGLSLVertToGeo : public GrGLSLVarying {
    GrGLSLVertToGeo(GrSLType type)
        : GrGLSLVarying(type, kVertToGeo_Varying) {}
};

struct GrGLSLGeoToFrag : public GrGLSLVarying {
    GrGLSLGeoToFrag(GrSLType type)
        : GrGLSLVarying(type, kGeoToFrag_Varying) {}
};

/* a specialization of the above for GPs.  Lets the user add uniforms, varyings, and VS / FS code */
class GrGLSLGPBuilder : public virtual GrGLSLUniformBuilder {
public:
    /*
     * addVarying allows fine grained control for setting up varyings between stages.  If you just
     * need to take an attribute and pass it through to an output value in a fragment shader, use
     * addPassThroughAttribute.
     * TODO convert most uses of addVarying to addPassThroughAttribute
     */
    virtual void addVarying(const char* name,
                            GrGLSLVarying*,
                            GrSLPrecision precision = kDefault_GrSLPrecision) = 0;

    /*
     * This call can be used by GP to pass an attribute through all shaders directly to 'output' in
     * the fragment shader.  Though this call effects both the vertex shader and fragment shader,
     * it expects 'output' to be defined in the fragment shader before this call is made.
     * TODO it might be nicer behavior to have a flag to declare output inside this call
     */
    virtual void addPassThroughAttribute(const GrGeometryProcessor::Attribute*,
                                         const char* output) = 0;

    /*
     * Creates a fragment shader varying that can be referred to.
     * Comparable to GrGLSLUniformBuilder::addUniform().
     */
    virtual SeparableVaryingHandle addSeparableVarying(
        const char* name, GrGLSLVertToFrag*,
        GrSLPrecision fsPrecision = kDefault_GrSLPrecision) = 0;

    // TODO rename getFragmentBuilder
    virtual GrGLFragmentBuilder* getFragmentShaderBuilder() = 0;
    virtual GrGLVertexBuilder* getVertexShaderBuilder() = 0;

    /*
     * *NOTE* NO MEMBERS ALLOWED, MULTIPLE INHERITANCE
     */
};


/* a specializations for FPs. Lets the user add uniforms and FS code */
class GrGLSLFPBuilder : public virtual GrGLSLUniformBuilder {
public:
    virtual GrGLFragmentBuilder* getFragmentShaderBuilder() = 0;

    /*
     * *NOTE* NO MEMBERS ALLOWED, MULTIPLE INHERITANCE
     */
};

/* a specializations for XPs. Lets the user add uniforms and FS code */
class GrGLSLXPBuilder : public virtual GrGLSLUniformBuilder {
public:
    virtual GrGLXPFragmentBuilder* getFragmentShaderBuilder() = 0;

    /*
     * *NOTE* NO MEMBERS ALLOWED, MULTIPLE INHERITANCE
     */
};

class GrGLSLProgramBuilder : public GrGLSLGPBuilder,
                             public GrGLSLFPBuilder,
                             public GrGLSLXPBuilder {
public:
    typedef GrGpu::DrawArgs DrawArgs;

    GrGLXPFragmentBuilder* getFragmentShaderBuilder() override { return &fFS; }
    GrGLVertexBuilder* getVertexShaderBuilder() override { return &fVS; }

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

    // number of each input/output type in a single allocation block, used by many builders
    static const int kVarsPerBlock;

    GrGLVertexBuilder fVS;
    GrGLGeometryBuilder fGS;
    GrGLFragmentShaderBuilder fFS;
    int fStageIndex;

    BuiltinUniformHandles fUniformHandles;

    const DrawArgs& fArgs;

private:
    virtual void onAppendUniformDecls(ShaderVisibility visibility, SkString* out) const = 0;

    friend class GrGLShaderBuilder;
    friend class GrGLVertexBuilder;
    friend class GrGLFragmentShaderBuilder;
    friend class GrGLGeometryBuilder;
};

#endif
