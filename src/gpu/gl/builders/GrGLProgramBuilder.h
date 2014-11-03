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
#include "../GrGLUniformHandle.h"
#include "../GrGLGeometryProcessor.h"

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
        kVertex_Visibility   = 0x1,
        kGeometry_Visibility = 0x2,
        kFragment_Visibility = 0x4,
    };

    virtual ~GrGLUniformBuilder() {}

    typedef GrGLProgramDataManager::UniformHandle UniformHandle;

    /** Add a uniform variable to the current program, that has visibility in one or more shaders.
        visibility is a bitfield of ShaderVisibility values indicating from which shaders the
        uniform should be accessible. At least one bit must be set. Geometry shader uniforms are not
        supported at this time. The actual uniform name will be mangled. If outName is not NULL then
        it will refer to the final uniform name after return. Use the addUniformArray variant to add
        an array of uniforms. */
    virtual UniformHandle addUniform(uint32_t visibility,
                                     GrSLType type,
                                     const char* name,
                                     const char** outName = NULL) = 0;
    virtual UniformHandle addUniformArray(uint32_t visibility,
                                          GrSLType type,
                                          const char* name,
                                          int arrayCount,
                                          const char** outName = NULL) = 0;

    virtual const GrGLShaderVar& getUniformVariable(UniformHandle u) const = 0;

    /**
     * Shortcut for getUniformVariable(u).c_str()
     */
    virtual const char* getUniformCStr(UniformHandle u) const = 0;

    virtual const GrGLContextInfo& ctxInfo() const = 0;

    virtual GrGpuGL* gpu() const = 0;

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
    virtual void addVarying(const char* name,
                            GrGLVarying*,
                            GrGLShaderVar::Precision fsPrecision=GrGLShaderVar::kDefault_Precision) = 0;

    // TODO rename getFragmentBuilder
    virtual GrGLGPFragmentBuilder* getFragmentShaderBuilder() = 0;
    virtual GrGLVertexBuilder* getVertexShaderBuilder() = 0;

    /*
     * *NOTE* NO MEMBERS ALLOWED, MULTIPLE INHERITANCE
     */
};

/* a specializations for FPs. Lets the user add uniforms and FS code */
class GrGLFPBuilder : public virtual GrGLUniformBuilder {
public:
    virtual GrGLFPFragmentBuilder* getFragmentShaderBuilder() = 0;

    /*
     * *NOTE* NO MEMBERS ALLOWED, MULTIPLE INHERITANCE
     */
};

struct GrGLInstalledProc;
struct GrGLInstalledGeoProc;
struct GrGLInstalledFragProc;
struct GrGLInstalledFragProcs;

/*
 * Please note - no diamond problems because of virtual inheritance.  Also, both base classes
 * are pure virtual with no data members.  This is the base class for program building.
 * Subclasses are nearly identical but each has their own way of emitting transforms.  State for
 * each of the elements of the shader pipeline, ie vertex, fragment, geometry, etc, lives in those
 * respective builders
*/
class GrGLProgramBuilder : public GrGLGPBuilder,
                           public GrGLFPBuilder {
public:
    /** Generates a shader program.
     *
     * The program implements what is specified in the stages given as input.
     * After successful generation, the builder result objects are available
     * to be used.
     * @return true if generation was successful.
     */
    static GrGLProgram* CreateProgram(const GrOptDrawState&, GrGpu::DrawType, GrGpuGL*);

    virtual UniformHandle addUniform(uint32_t visibility,
                                     GrSLType type,
                                     const char* name,
                                     const char** outName = NULL) SK_OVERRIDE {
        return this->addUniformArray(visibility, type, name, GrGLShaderVar::kNonArray, outName);
    }
    virtual UniformHandle addUniformArray(uint32_t visibility,
                                          GrSLType type,
                                          const char* name,
                                          int arrayCount,
                                          const char** outName = NULL) SK_OVERRIDE;

    virtual const GrGLShaderVar& getUniformVariable(UniformHandle u) const SK_OVERRIDE {
        return fUniforms[u.toShaderBuilderIndex()].fVariable;
    }

    virtual const char* getUniformCStr(UniformHandle u) const SK_OVERRIDE {
        return this->getUniformVariable(u).c_str();
    }

    virtual const GrGLContextInfo& ctxInfo() const SK_OVERRIDE;

    virtual GrGpuGL* gpu() const SK_OVERRIDE { return fGpu; }

    virtual GrGLFPFragmentBuilder* getFragmentShaderBuilder() SK_OVERRIDE { return &fFS; }
    virtual GrGLVertexBuilder* getVertexShaderBuilder() SK_OVERRIDE { return &fVS; }

    virtual void addVarying(
            const char* name,
            GrGLVarying*,
            GrGLShaderVar::Precision fsPrecision=GrGLShaderVar::kDefault_Precision) SK_OVERRIDE;

    // Handles for program uniforms (other than per-effect uniforms)
    struct BuiltinUniformHandles {
        UniformHandle       fViewMatrixUni;
        UniformHandle       fRTAdjustmentUni;
        UniformHandle       fColorUni;
        UniformHandle       fCoverageUni;

        // We use the render target height to provide a y-down frag coord when specifying
        // origin_upper_left is not supported.
        UniformHandle       fRTHeightUni;

        // Uniforms for computing texture coords to do the dst-copy lookup
        UniformHandle       fDstCopyTopLeftUni;
        UniformHandle       fDstCopyScaleUni;
        UniformHandle       fDstCopySamplerUni;
    };

protected:
    typedef GrProgramDesc::ProcKeyProvider ProcKeyProvider;
    typedef GrGLProgramDataManager::UniformInfo UniformInfo;
    typedef GrGLProgramDataManager::UniformInfoArray UniformInfoArray;

    static GrGLProgramBuilder* CreateProgramBuilder(const GrOptDrawState&,
                                                    GrGpu::DrawType,
                                                    bool hasGeometryProcessor,
                                                    GrGpuGL*);

    GrGLProgramBuilder(GrGpuGL*, const GrOptDrawState&);

    const GrOptDrawState& optState() const { return fOptState; }
    const GrProgramDesc& desc() const { return fDesc; }
    const GrProgramDesc::KeyHeader& header() const { return fDesc.header(); }

    // Generates a name for a variable. The generated string will be name prefixed by the prefix
    // char (unless the prefix is '\0'). It also mangles the name to be stage-specific if we're
    // generating stage code.
    void nameVariable(SkString* out, char prefix, const char* name);
    void setupUniformColorAndCoverageIfNeeded(GrGLSLExpr4* inputColor, GrGLSLExpr1* inputCoverage);
    void emitAndInstallProcs(const GrOptDrawState& optState,
                             GrGLSLExpr4* inputColor,
                             GrGLSLExpr4* inputCoverage);
    void emitAndInstallFragProcs(int procOffset, int numProcs, GrGLSLExpr4* inOut);
    template <class Proc>
    void emitAndInstallProc(const Proc&,
                            int index,
                            const ProcKeyProvider&,
                            const GrGLSLExpr4& input,
                            GrGLSLExpr4* output);

    // these emit functions help to keep the createAndEmitProcessors template general
    void emitAndInstallProc(const GrFragmentStage&,
                            const GrProcessorKey&,
                            const char* outColor,
                            const char* inColor);
    void emitAndInstallProc(const GrGeometryProcessor&,
                            const GrProcessorKey&,
                            const char* outColor,
                            const char* inColor);
    void verify(const GrGeometryProcessor&);
    void verify(const GrFragmentProcessor&);
    void emitSamplers(const GrProcessor&,
                      GrGLProcessor::TextureSamplerArray* outSamplers,
                      GrGLInstalledProc*);

    // each specific program builder has a distinct transform and must override this function
    virtual void emitTransforms(const GrFragmentStage&,
                                GrGLProcessor::TransformedCoordsArray* outCoords,
                                GrGLInstalledFragProc*);
    GrGLProgram* finalize();
    void bindUniformLocations(GrGLuint programID);
    bool checkLinkStatus(GrGLuint programID);
    void resolveUniformLocations(GrGLuint programID);
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
        AutoStageAdvance(GrGLProgramBuilder* pb) : fPB(pb) { fPB->reset(); }
        ~AutoStageAdvance() { fPB->exitStage(); }
    private:
        GrGLProgramBuilder* fPB;
    };
    void exitStage() { fOutOfStage = true; }
    void enterStage() { fOutOfStage = false; }
    int stageIndex() const { return fStageIndex; }

    // number of each input/output type in a single allocation block, used by many builders
    static const int kVarsPerBlock;

    BuiltinUniformHandles fUniformHandles;
    GrGLVertexBuilder fVS;
    GrGLGeometryBuilder fGS;
    GrGLFragmentShaderBuilder fFS;
    bool fOutOfStage;
    int fStageIndex;

    GrGLInstalledGeoProc* fGeometryProcessor;
    SkAutoTUnref<GrGLInstalledFragProcs> fFragmentProcessors;

    const GrOptDrawState& fOptState;
    const GrProgramDesc& fDesc;
    GrGpuGL* fGpu;
    UniformInfoArray fUniforms;

    friend class GrGLShaderBuilder;
    friend class GrGLVertexBuilder;
    friend class GrGLFragmentShaderBuilder;
    friend class GrGLGeometryBuilder;
};

/**
 * The below structs represent processors installed in programs.  All processors can have texture
 * samplers, but only frag processors have coord transforms, hence the need for different structs
 */
struct GrGLInstalledProc {
     typedef GrGLProgramDataManager::UniformHandle UniformHandle;

     struct Sampler {
         SkDEBUGCODE(Sampler() : fTextureUnit(-1) {})
         UniformHandle  fUniform;
         int            fTextureUnit;
     };
     SkSTArray<4, Sampler, true> fSamplers;
};

struct GrGLInstalledGeoProc : public GrGLInstalledProc {
    SkAutoTDelete<GrGLGeometryProcessor> fGLProc;
};

struct GrGLInstalledFragProc : public GrGLInstalledProc {
    GrGLInstalledFragProc(bool useLocalCoords) : fGLProc(NULL), fLocalCoordAttrib(useLocalCoords) {}
    class ShaderVarHandle {
    public:
        bool isValid() const { return fHandle > -1; }
        ShaderVarHandle() : fHandle(-1) {}
        ShaderVarHandle(int value) : fHandle(value) { SkASSERT(this->isValid()); }
        int handle() const { SkASSERT(this->isValid()); return fHandle; }
        UniformHandle convertToUniformHandle() {
            SkASSERT(this->isValid());
            return GrGLProgramDataManager::UniformHandle::CreateFromUniformIndex(fHandle);
        }

    private:
        int fHandle;
    };

    struct Transform {
        Transform() : fType(kVoid_GrSLType) { fCurrentValue = SkMatrix::InvalidMatrix(); }
        ShaderVarHandle fHandle;
        SkMatrix       fCurrentValue;
        GrSLType       fType;
    };

    SkAutoTDelete<GrGLFragmentProcessor> fGLProc;
    SkSTArray<2, Transform, true>        fTransforms;
    bool                                 fLocalCoordAttrib;
};

struct GrGLInstalledFragProcs : public SkRefCnt {
    virtual ~GrGLInstalledFragProcs();
    SkSTArray<8, GrGLInstalledFragProc*, true> fProcs;
};

#endif
