/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLSLFragmentProcessor_DEFINED
#define GrGLSLFragmentProcessor_DEFINED

#include "GrFragmentProcessor.h"
#include "GrShaderVar.h"
#include "glsl/GrGLSLProgramDataManager.h"
#include "glsl/GrGLSLSampler.h"

class GrProcessor;
class GrProcessorKeyBuilder;
class GrGLSLCaps;
class GrGLSLFPBuilder;
class GrGLSLFPFragmentBuilder;
class GrGLSLUniformHandler;

class GrGLSLFragmentProcessor {
public:
    GrGLSLFragmentProcessor() {}

    virtual ~GrGLSLFragmentProcessor() {
        for (int i = 0; i < fChildProcessors.count(); ++i) {
            delete fChildProcessors[i];
        }
    }

    typedef GrGLSLProgramDataManager::UniformHandle UniformHandle;
    typedef GrGLSLProgramDataManager::UniformHandle SamplerHandle;

    /**
     * When building a program from a GrPipeline this is used to provide the GrShaderVars that
     * contain the resulting transformed coords from each of a GrFragmentProcessor's
     * GrCoordTransforms. This allows the GrFragmentProcessor subclasses to refer to the transformed
     * coords in fragment code.
     */
    class TransformedCoordVars {
    public:
        TransformedCoordVars(const GrFragmentProcessor* fp, const GrShaderVar* vars)
                : fFP(fp)
                , fTransformedVars(vars) {}

        const GrShaderVar& operator[] (int i) const {
            SkASSERT(i >= 0 && i < fFP->numCoordTransforms());
            return fTransformedVars[i];
        }

        TransformedCoordVars childTransforms(int childIdx) const;

    private:
        const GrFragmentProcessor* fFP;
        const GrShaderVar*         fTransformedVars;
    };

    /** Called when the program stage should insert its code into the shaders. The code in each
        shader will be in its own block ({}) and so locally scoped names will not collide across
        stages.

        @param fragBuilder       Interface used to emit code in the shaders.
        @param fp                The processor that generated this program stage.
        @param key               The key that was computed by GenKey() from the generating
                                 GrProcessor.
        @param outputColor       A predefined vec4 in the FS in which the stage should place its
                                 output color (or coverage).
        @param inputColor        A vec4 that holds the input color to the stage in the FS. This may
                                 be nullptr in which case the implied input is solid white (all
                                 ones). TODO: Better system for communicating optimization info
                                 (e.g. input color is solid white, trans black, known to be opaque,
                                 etc.) that allows the processor to communicate back similar known
                                 info about its output.
        @param transformedCoords Fragment shader variables containing the coords computed using
                                 each of the GrFragmentProcessor's GrCoordTransforms.
        @param texSamplers       Contains one entry for each GrTextureAccess of the GrProcessor.
                                 These can be passed to the builder to emit texture reads in the
                                 generated code.
        @param bufferSamplers    Contains one entry for each GrBufferAccess of the GrProcessor.
                                 These can be passed to the builder to emit buffer reads in the
                                 generated code.
     */
    struct EmitArgs {
        EmitArgs(GrGLSLFPFragmentBuilder* fragBuilder,
                 GrGLSLUniformHandler* uniformHandler,
                 const GrGLSLCaps* caps,
                 const GrFragmentProcessor& fp,
                 const char* outputColor,
                 const char* inputColor,
                 const TransformedCoordVars& transformedCoordVars,
                 const SamplerHandle* texSamplers,
                 const SamplerHandle* bufferSamplers,
                 bool gpImplementsDistanceVector)
            : fFragBuilder(fragBuilder)
            , fUniformHandler(uniformHandler)
            , fGLSLCaps(caps)
            , fFp(fp)
            , fOutputColor(outputColor)
            , fInputColor(inputColor)
            , fTransformedCoords(transformedCoordVars)
            , fTexSamplers(texSamplers)
            , fBufferSamplers(bufferSamplers)
            , fGpImplementsDistanceVector(gpImplementsDistanceVector) {}
        GrGLSLFPFragmentBuilder* fFragBuilder;
        GrGLSLUniformHandler* fUniformHandler;
        const GrGLSLCaps* fGLSLCaps;
        const GrFragmentProcessor& fFp;
        const char* fOutputColor;
        const char* fInputColor;
        const TransformedCoordVars& fTransformedCoords;
        const SamplerHandle* fTexSamplers;
        const SamplerHandle* fBufferSamplers;
        bool fGpImplementsDistanceVector;
    };

    virtual void emitCode(EmitArgs&) = 0;

    void setData(const GrGLSLProgramDataManager& pdman, const GrFragmentProcessor& processor);

    static void GenKey(const GrProcessor&, const GrGLSLCaps&, GrProcessorKeyBuilder*) {}

    int numChildProcessors() const { return fChildProcessors.count(); }

    GrGLSLFragmentProcessor* childProcessor(int index) const {
        return fChildProcessors[index];
    }

    /** Will emit the code of a child proc in its own scope. Pass in the parent's EmitArgs and
     *  emitChild will automatically extract the coords and samplers of that child and pass them
     *  on to the child's emitCode(). Also, any uniforms or functions emitted by the child will
     *  have their names mangled to prevent redefinitions. The output color name is also mangled
     *  therefore in an in/out param. It will be declared in mangled form by emitChild(). It is
     *  legal to pass nullptr as inputColor, since all fragment processors are required to work
     *  without an input color.
     */
    void emitChild(int childIndex, const char* inputColor, SkString* outputColor,
                   EmitArgs& parentArgs);

    /** Variation that uses the parent's output color variable to hold the child's output.*/
    void emitChild(int childIndex, const char* inputColor, EmitArgs& parentArgs);

protected:
    /** A GrGLSLFragmentProcessor instance can be reused with any GrFragmentProcessor that produces
    the same stage key; this function reads data from a GrFragmentProcessor and uploads any
    uniform variables required by the shaders created in emitCode(). The GrFragmentProcessor
    parameter is guaranteed to be of the same type that created this GrGLSLFragmentProcessor and
    to have an identical processor key as the one that created this GrGLSLFragmentProcessor.  */
    // TODO update this to pass in GrFragmentProcessor
    virtual void onSetData(const GrGLSLProgramDataManager&, const GrProcessor&) {}

private:
    void internalEmitChild(int, const char*, const char*, EmitArgs&);

    SkTArray<GrGLSLFragmentProcessor*, true> fChildProcessors;

    friend class GrFragmentProcessor;
};

#endif
