/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLGeometryProcessor_DEFINED
#define GrGLGeometryProcessor_DEFINED

#include "GrGLProcessor.h"

class GrBatchTracker;
class GrFragmentProcessor;
class GrGLGPBuilder;

class GrGLPrimitiveProcessor {
public:
    GrGLPrimitiveProcessor() : fViewMatrixName(NULL) { fViewMatrix = SkMatrix::InvalidMatrix(); }
    virtual ~GrGLPrimitiveProcessor() {}

    typedef GrGLProgramDataManager::UniformHandle UniformHandle;
    typedef GrGLProcessor::TextureSamplerArray TextureSamplerArray;

    typedef SkSTArray<2, const GrCoordTransform*, true> ProcCoords;
    typedef SkSTArray<8, ProcCoords> TransformsIn;
    typedef SkSTArray<8, GrGLProcessor::TransformedCoordsArray> TransformsOut;

    struct EmitArgs {
        EmitArgs(GrGLGPBuilder* pb,
                 const GrPrimitiveProcessor& gp,
                 const GrBatchTracker& bt,
                 const char* outputColor,
                 const char* outputCoverage,
                 const TextureSamplerArray& samplers,
                 const TransformsIn& transformsIn,
                 TransformsOut* transformsOut)
            : fPB(pb)
            , fGP(gp)
            , fBT(bt)
            , fOutputColor(outputColor)
            , fOutputCoverage(outputCoverage)
            , fSamplers(samplers)
            , fTransformsIn(transformsIn)
            , fTransformsOut(transformsOut) {}
        GrGLGPBuilder* fPB;
        const GrPrimitiveProcessor& fGP;
        const GrBatchTracker& fBT;
        const char* fOutputColor;
        const char* fOutputCoverage;
        const TextureSamplerArray& fSamplers;
        const TransformsIn& fTransformsIn;
        TransformsOut* fTransformsOut;
    };

    /**
     * This is similar to emitCode() in the base class, except it takes a full shader builder.
     * This allows the effect subclass to emit vertex code.
     */
    virtual void emitCode(EmitArgs&) = 0;


    /** A GrGLPrimitiveProcessor instance can be reused with any GrGLPrimitiveProcessor that
        produces the same stage key; this function reads data from a GrGLPrimitiveProcessor and
        uploads any uniform variables required  by the shaders created in emitCode(). The
        GrPrimitiveProcessor parameter is guaranteed to be of the same type that created this
        GrGLPrimitiveProcessor and to have an identical processor key as the one that created this
        GrGLPrimitiveProcessor.  */
    virtual void setData(const GrGLProgramDataManager&,
                         const GrPrimitiveProcessor&,
                         const GrBatchTracker&) = 0;

    static SkMatrix GetTransformMatrix(const SkMatrix& localMatrix, const GrCoordTransform&);

protected:
    /** a helper which can setup vertex, constant, or uniform color depending on inputType.
     *  This function will only do the minimum required to emit the correct shader code.  If
     *  inputType == attribute, then colorAttr must not be NULL.  Likewise, if inputType == Uniform
     *  then colorUniform must not be NULL.
     */
    void setupColorPassThrough(GrGLGPBuilder* pb,
                               GrGPInput inputType,
                               const char* inputName,
                               const GrGeometryProcessor::GrAttribute* colorAttr,
                               UniformHandle* colorUniform);

    const char* uViewM() const { return fViewMatrixName; }

    /** a helper function to setup the uniform handle for the uniform view matrix */
    void addUniformViewMatrix(GrGLGPBuilder*);


    /** a helper function to upload a uniform viewmatrix.
     * TODO we can remove this function when we have deferred geometry in place
     */
    void setUniformViewMatrix(const GrGLProgramDataManager&,
                              const SkMatrix& viewMatrix);

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

    SkSTArray<8, SkSTArray<2, Transform, true> > fInstalledTransforms;

private:
    UniformHandle fViewMatrixUniform;
    SkMatrix fViewMatrix;
    const char* fViewMatrixName;
};

class GrGLPathRendering;
/**
 * If a GL effect needs a GrGLFullShaderBuilder* object to emit vertex code, then it must inherit
 * from this class. Since paths don't have vertices, this class is only meant to be used internally
 * by skia, for special cases.
 */
class GrGLGeometryProcessor : public GrGLPrimitiveProcessor {
public:
    /* Any general emit code goes in the base class emitCode.  Subclasses override onEmitCode */
    void emitCode(EmitArgs&) SK_OVERRIDE;

    void setTransformData(const GrPrimitiveProcessor*,
                          const GrGLProgramDataManager&,
                          int index,
                          const SkTArray<const GrCoordTransform*, true>& transforms);

protected:
    const char* position() const { return "pos3"; }

    // Many GrGeometryProcessors do not need explicit local coords
    void emitTransforms(GrGLGPBuilder* gp,
                        const char* position,
                        const SkMatrix& localMatrix,
                        const TransformsIn& tin,
                        TransformsOut* tout) {
        this->emitTransforms(gp, position, position, localMatrix, tin, tout);
    }

    void emitTransforms(GrGLGPBuilder*,
                        const char* position,
                        const char* localCoords,
                        const SkMatrix& localMatrix,
                        const TransformsIn&,
                        TransformsOut*);

private:
    virtual void onEmitCode(EmitArgs&) = 0;

    typedef GrGLPrimitiveProcessor INHERITED;
};

class GrGLGpu;

class GrGLPathProcessor : public GrGLPrimitiveProcessor {
public:
    GrGLPathProcessor(const GrPathProcessor&, const GrBatchTracker&);

    static void GenKey(const GrPathProcessor&,
                       const GrBatchTracker& bt,
                       const GrGLCaps&,
                       GrProcessorKeyBuilder* b);

    void emitCode(EmitArgs&) SK_OVERRIDE;

    virtual void emitTransforms(GrGLGPBuilder*, const TransformsIn&, TransformsOut*) = 0;

    virtual void resolveSeparableVaryings(GrGLGpu* gpu, GrGLuint programId) {}

    void setData(const GrGLProgramDataManager&,
                 const GrPrimitiveProcessor&,
                 const GrBatchTracker&) SK_OVERRIDE;

    virtual void setTransformData(const GrPrimitiveProcessor*,
                                  int index,
                                  const SkTArray<const GrCoordTransform*, true>& transforms,
                                  GrGLPathRendering*,
                                  GrGLuint programID) = 0;

    virtual void didSetData(GrGLPathRendering*) {}

private:
    UniformHandle fColorUniform;
    GrColor fColor;

    typedef GrGLPrimitiveProcessor INHERITED;
};

#endif
