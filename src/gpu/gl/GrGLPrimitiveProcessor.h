/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLPrimitiveProcessor_DEFINED
#define GrGLPrimitiveProcessor_DEFINED

#include "GrPrimitiveProcessor.h"
#include "GrGLProcessor.h"

class GrBatchTracker;
class GrPrimitiveProcessor;
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
                               const GrPrimitiveProcessor::Attribute* colorAttr,
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

#endif
