/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLProgramBuilder_DEFINED
#define GrGLProgramBuilder_DEFINED

#include "GrGLFragmentShaderBuilder.h"
#include "GrGLGeometryShaderBuilder.h"
#include "GrGLVertexShaderBuilder.h"
#include "../GrGLProgramDataManager.h"
#include "../GrGLPathProgramDataManager.h"
#include "../GrGLUniformHandle.h"
#include "../GrGLPrimitiveProcessor.h"
#include "../GrGLXferProcessor.h"
#include "../../GrPendingFragmentStage.h"
#include "../../GrPipeline.h"

// Enough precision to represent 1 / 2048 accurately in printf
#define GR_SIGNIFICANT_POW2_DECIMAL_DIG 11

/*
 * This is the base class for a series of interfaces.  This base class *MUST* remain abstract with
 * NO data members because it is used in multiple interface inheritance.
 * Heirarchy:
 *                      GrGLUniformBuilder
 *                     /                  \
 *                GrGLFPBuilder       GrGLGPBuilder
 *                     \                  /
 *                     GrGLProgramBuilder(internal use only)
 */
class GrGLUniformBuilder {
public:
    enum ShaderVisibility {
        kVertex_Visibility   = 1 << kVertex_GrShaderType,
        kGeometry_Visibility = 1 << kGeometry_GrShaderType,
        kFragment_Visibility = 1 << kFragment_GrShaderType,
    };

    virtual ~GrGLUniformBuilder() {}

    typedef GrGLProgramDataManager::UniformHandle UniformHandle;
    typedef GrGLPathProgramDataManager::SeparableVaryingHandle SeparableVaryingHandle;

    /** Add a uniform variable to the current program, that has visibility in one or more shaders.
        visibility is a bitfield of ShaderVisibility values indicating from which shaders the
        uniform should be accessible. At least one bit must be set. Geometry shader uniforms are not
        supported at this time. The actual uniform name will be mangled. If outName is not NULL then
        it will refer to the final uniform name after return. Use the addUniformArray variant to add
        an array of uniforms. */
    UniformHandle addUniform(uint32_t visibility,
                             GrSLType type,
                             GrSLPrecision precision,
                             const char* name,
                             const char** outName = NULL) {
        return this->addUniformArray(visibility, type, precision, name, 0, outName);
    }

    virtual UniformHandle addUniformArray(
        uint32_t visibility,
        GrSLType type,
        GrSLPrecision precision,
        const char* name,
        int arrayCount,
        const char** outName = NULL) = 0;

    virtual const GrGLShaderVar& getUniformVariable(UniformHandle u) const = 0;

    /**
     * Shortcut for getUniformVariable(u).c_str()
     */
    virtual const char* getUniformCStr(UniformHandle u) const = 0;

    virtual const GrGLContextInfo& ctxInfo() const = 0;

    virtual GrGLGpu* gpu() const = 0;

    /*
     * *NOTE* NO MEMBERS ALLOWED, MULTIPLE INHERITANCE
     */
};

// TODO move this into GrGLGPBuilder and move them both out of this file
class GrGLVarying {
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

    GrGLVarying(GrSLType type, Varying varying)
        : fVarying(varying), fType(type), fVsOut(NULL), fGsIn(NULL), fGsOut(NULL),
          fFsIn(NULL) {}

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

struct GrGLVertToFrag : public GrGLVarying {
    GrGLVertToFrag(GrSLType type)
        : GrGLVarying(type, kVertToFrag_Varying) {}
};

struct GrGLVertToGeo : public GrGLVarying {
    GrGLVertToGeo(GrSLType type)
        : GrGLVarying(type, kVertToGeo_Varying) {}
};

struct GrGLGeoToFrag : public GrGLVarying {
    GrGLGeoToFrag(GrSLType type)
        : GrGLVarying(type, kGeoToFrag_Varying) {}
};

/* a specialization of the above for GPs.  Lets the user add uniforms, varyings, and VS / FS code */
class GrGLGPBuilder : public virtual GrGLUniformBuilder {
public:
    /*
     * addVarying allows fine grained control for setting up varyings between stages.  If you just
     * need to take an attribute and pass it through to an output value in a fragment shader, use
     * addPassThroughAttribute.
     * TODO convert most uses of addVarying to addPassThroughAttribute
     */
    virtual void addVarying(const char* name,
                            GrGLVarying*,
                            GrSLPrecision fsPrecision = kDefault_GrSLPrecision) = 0;

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
     * Comparable to GrGLUniformBuilder::addUniform().
     */
    virtual SeparableVaryingHandle addSeparableVarying(
        const char* name, GrGLVertToFrag*, GrSLPrecision fsPrecision = kDefault_GrSLPrecision) = 0;

    // TODO rename getFragmentBuilder
    virtual GrGLFragmentBuilder* getFragmentShaderBuilder() = 0;
    virtual GrGLVertexBuilder* getVertexShaderBuilder() = 0;

    /*
     * *NOTE* NO MEMBERS ALLOWED, MULTIPLE INHERITANCE
     */
};

/* a specializations for FPs. Lets the user add uniforms and FS code */
class GrGLFPBuilder : public virtual GrGLUniformBuilder {
public:
    virtual GrGLFragmentBuilder* getFragmentShaderBuilder() = 0;

    /*
     * *NOTE* NO MEMBERS ALLOWED, MULTIPLE INHERITANCE
     */
};

/* a specializations for XPs. Lets the user add uniforms and FS code */
class GrGLXPBuilder : public virtual GrGLUniformBuilder {
public:
    virtual GrGLXPFragmentBuilder* getFragmentShaderBuilder() = 0;

    /*
     * *NOTE* NO MEMBERS ALLOWED, MULTIPLE INHERITANCE
     */
};

/**
 * The below struct represent processors installed in programs.
 */
template <class Proc>
struct GrGLInstalledProc {
    SkDEBUGCODE(int fSamplersIdx;)
    SkAutoTDelete<Proc> fGLProc;
};

typedef GrGLInstalledProc<GrGLPrimitiveProcessor> GrGLInstalledGeoProc;
typedef GrGLInstalledProc<GrGLXferProcessor> GrGLInstalledXferProc;
typedef GrGLInstalledProc<GrGLFragmentProcessor> GrGLInstalledFragProc;

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
class GrGLProgramBuilder : public GrGLGPBuilder,
                           public GrGLFPBuilder,
                           public GrGLXPBuilder {
public:
    typedef GrGpu::DrawArgs DrawArgs;
    /** Generates a shader program.
     *
     * The program implements what is specified in the stages given as input.
     * After successful generation, the builder result objects are available
     * to be used.
     * @return true if generation was successful.
     */
    static GrGLProgram* CreateProgram(const DrawArgs&, GrGLGpu*);

    UniformHandle addUniformArray(uint32_t visibility,
                                  GrSLType type,
                                  GrSLPrecision precision,
                                  const char* name,
                                  int arrayCount,
                                  const char** outName) override;

    const GrGLShaderVar& getUniformVariable(UniformHandle u) const override {
        return fUniforms[u.toShaderBuilderIndex()].fVariable;
    }

    const char* getUniformCStr(UniformHandle u) const override {
        return this->getUniformVariable(u).c_str();
    }

    const GrGLContextInfo& ctxInfo() const override;

    GrGLGpu* gpu() const override { return fGpu; }

    GrGLXPFragmentBuilder* getFragmentShaderBuilder() override { return &fFS; }
    GrGLVertexBuilder* getVertexShaderBuilder() override { return &fVS; }

    void addVarying(
            const char* name,
            GrGLVarying*,
            GrSLPrecision fsPrecision = kDefault_GrSLPrecision) override;

    void addPassThroughAttribute(const GrPrimitiveProcessor::Attribute*,
                                 const char* output) override;

    SeparableVaryingHandle addSeparableVarying(
        const char* name,
        GrGLVertToFrag*,
        GrSLPrecision fsPrecision = kDefault_GrSLPrecision) override;

    // Handles for program uniforms (other than per-effect uniforms)
    struct BuiltinUniformHandles {
        UniformHandle       fRTAdjustmentUni;

        // We use the render target height to provide a y-down frag coord when specifying
        // origin_upper_left is not supported.
        UniformHandle       fRTHeightUni;
    };

protected:
    typedef GrGLProgramDataManager::UniformInfo UniformInfo;
    typedef GrGLProgramDataManager::UniformInfoArray UniformInfoArray;

    static GrGLProgramBuilder* CreateProgramBuilder(const DrawArgs&, GrGLGpu*);

    GrGLProgramBuilder(GrGLGpu*, const DrawArgs&);

    const GrPrimitiveProcessor& primitiveProcessor() const { return *fArgs.fPrimitiveProcessor; }
    const GrPipeline& pipeline() const { return *fArgs.fPipeline; }
    const GrProgramDesc& desc() const { return *fArgs.fDesc; }
    const GrBatchTracker& batchTracker() const { return *fArgs.fBatchTracker; }
    const GrProgramDesc::KeyHeader& header() const { return fArgs.fDesc->header(); }

    // Generates a name for a variable. The generated string will be name prefixed by the prefix
    // char (unless the prefix is '\0'). It also mangles the name to be stage-specific if we're
    // generating stage code.
    void nameVariable(SkString* out, char prefix, const char* name);
    // Generates a possibly mangled name for a stage variable and writes it to the fragment shader.
    // If GrGLSLExpr4 has a valid name then it will use that instead
    void nameExpression(GrGLSLExpr4*, const char* baseName);
    bool emitAndInstallProcs(GrGLSLExpr4* inputColor, GrGLSLExpr4* inputCoverage);
    void emitAndInstallFragProcs(int procOffset, int numProcs, GrGLSLExpr4* inOut);
    void emitAndInstallProc(const GrPendingFragmentStage&,
                            int index,
                            const GrGLSLExpr4& input,
                            GrGLSLExpr4* output);

    void emitAndInstallProc(const GrPrimitiveProcessor&,
                            GrGLSLExpr4* outputColor,
                            GrGLSLExpr4* outputCoverage);

    // these emit functions help to keep the createAndEmitProcessors template general
    void emitAndInstallProc(const GrPendingFragmentStage&,
                            int index,
                            const char* outColor,
                            const char* inColor);
    void emitAndInstallProc(const GrPrimitiveProcessor&,
                            const char* outColor,
                            const char* outCoverage);
    void emitAndInstallXferProc(const GrXferProcessor&,
                                const GrGLSLExpr4& colorIn,
                                const GrGLSLExpr4& coverageIn);

    void verify(const GrPrimitiveProcessor&);
    void verify(const GrXferProcessor&);
    void verify(const GrFragmentProcessor&);
    template <class Proc>
    void emitSamplers(const GrProcessor&,
                      GrGLProcessor::TextureSamplerArray* outSamplers,
                      GrGLInstalledProc<Proc>*);

    GrGLProgram* finalize();
    virtual void bindProgramResourceLocations(GrGLuint programID);
    bool checkLinkStatus(GrGLuint programID);
    virtual void resolveProgramResourceLocations(GrGLuint programID);
    void cleanupProgram(GrGLuint programID, const SkTDArray<GrGLuint>& shaderIDs);
    void cleanupShaders(const SkTDArray<GrGLuint>& shaderIDs);

    // Subclasses create different programs
    virtual GrGLProgram* createProgram(GrGLuint programID);

    void appendUniformDecls(ShaderVisibility, SkString*) const;

    // reset is called by program creator between each processor's emit code.  It increments the
    // stage offset for variable name mangling, and also ensures verfication variables in the
    // fragment shader are cleared.
    void reset() {
        this->enterStage();
        this->addStage();
        fFS.reset();
    }
    void addStage() { fStageIndex++; }

    // This simple class exits the stage and then restores the stage when it goes out of scope
    class AutoStageRestore {
    public:
        AutoStageRestore(GrGLProgramBuilder* pb)
            : fPB(pb), fOutOfStage(pb->fOutOfStage) { pb->exitStage(); }
        ~AutoStageRestore() { fPB->fOutOfStage = fOutOfStage; }
    private:
        GrGLProgramBuilder* fPB;
        bool fOutOfStage;
    };
    class AutoStageAdvance {
    public:
        AutoStageAdvance(GrGLProgramBuilder* pb)
            : fPB(pb) {
            fPB->reset();
            // Each output to the fragment processor gets its own code section
            fPB->fFS.nextStage();
        }
        ~AutoStageAdvance() { fPB->exitStage(); }
    private:
        GrGLProgramBuilder* fPB;
    };
    void exitStage() { fOutOfStage = true; }
    void enterStage() { fOutOfStage = false; }
    int stageIndex() const { return fStageIndex; }

    const char* rtAdjustment() const { return "rtAdjustment"; }

    // number of each input/output type in a single allocation block, used by many builders
    static const int kVarsPerBlock;

    BuiltinUniformHandles fUniformHandles;
    GrGLVertexBuilder fVS;
    GrGLGeometryBuilder fGS;
    GrGLFragmentShaderBuilder fFS;
    bool fOutOfStage;
    int fStageIndex;

    GrGLInstalledGeoProc* fGeometryProcessor;
    GrGLInstalledXferProc* fXferProcessor;
    SkAutoTUnref<GrGLInstalledFragProcs> fFragmentProcessors;

    const DrawArgs& fArgs;
    GrGLGpu* fGpu;
    UniformInfoArray fUniforms;
    GrGLPrimitiveProcessor::TransformsIn fCoordTransforms;
    GrGLPrimitiveProcessor::TransformsOut fOutCoords;
    SkTArray<UniformHandle> fSamplerUniforms;

    friend class GrGLShaderBuilder;
    friend class GrGLVertexBuilder;
    friend class GrGLFragmentShaderBuilder;
    friend class GrGLGeometryBuilder;
};
#endif
