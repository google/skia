/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLSLFragmentProcessor_DEFINED
#define GrGLSLFragmentProcessor_DEFINED

#include "src/gpu/GrFragmentProcessor.h"
#include "src/gpu/GrShaderVar.h"
#include "src/gpu/glsl/GrGLSLProgramDataManager.h"
#include "src/gpu/glsl/GrGLSLUniformHandler.h"

class GrProcessor;
class GrProcessorKeyBuilder;
class GrGLSLFPBuilder;
class GrGLSLFPFragmentBuilder;

class GrGLSLFragmentProcessor {
public:
    GrGLSLFragmentProcessor() {}

    virtual ~GrGLSLFragmentProcessor() {
        for (int i = 0; i < fChildProcessors.count(); ++i) {
            delete fChildProcessors[i];
        }
    }

    using UniformHandle      = GrGLSLUniformHandler::UniformHandle;
    using SamplerHandle      = GrGLSLUniformHandler::SamplerHandle;

private:
    /**
     * This class allows the shader builder to provide each GrGLSLFragmentProcesor with an array of
     * generated variables where each generated variable corresponds to an element of an array on
     * the GrFragmentProcessor that generated the GLSLFP. For example, this is used to provide a
     * variable holding transformed coords for each GrCoordTransform owned by the FP.
     */
    template <typename T, int (GrFragmentProcessor::*COUNT)() const>
    class BuilderInputProvider {
    public:
        BuilderInputProvider(const GrFragmentProcessor* fp, const T* ts) : fFP(fp) , fTs(ts) {}

        const T& operator[] (int i) const {
            SkASSERT(i >= 0 && i < (fFP->*COUNT)());
            return fTs[i];
        }

        int count() const { return (fFP->*COUNT)(); }

        BuilderInputProvider childInputs(int childIdx) const {
            const GrFragmentProcessor* child = &fFP->childProcessor(childIdx);
            GrFragmentProcessor::Iter iter(fFP);
            int numToSkip = 0;
            while (true) {
                const GrFragmentProcessor* fp = iter.next();
                if (fp == child) {
                    return BuilderInputProvider(child, fTs + numToSkip);
                }
                numToSkip += (fp->*COUNT)();
            }
        }

    private:
        const GrFragmentProcessor* fFP;
        const T*                   fTs;
    };

public:
    using TransformedCoordVars =
            BuilderInputProvider<GrShaderVar, &GrFragmentProcessor::numCoordTransforms>;
    using TextureSamplers =
            BuilderInputProvider<SamplerHandle, &GrFragmentProcessor::numTextureSamplers>;

    /** Called when the program stage should insert its code into the shaders. The code in each
        shader will be in its own block ({}) and so locally scoped names will not collide across
        stages.

        @param fragBuilder       Interface used to emit code in the shaders.
        @param fp                The processor that generated this program stage.
        @param key               The key that was computed by GenKey() from the generating
                                 GrProcessor.
        @param outputColor       A predefined half4 in the FS in which the stage should place its
                                 output color (or coverage).
        @param inputColor        A half4 that holds the input color to the stage in the FS. This may
                                 be nullptr in which case the fInputColor is set to "half4(1.0)"
                                 (solid white) so this is guaranteed non-null.
                                 TODO: Better system for communicating optimization info
                                 (e.g. input color is solid white, trans black, known to be opaque,
                                 etc.) that allows the processor to communicate back similar known
                                 info about its output.
        @param transformedCoords Fragment shader variables containing the coords computed using
                                 each of the GrFragmentProcessor's GrCoordTransforms.
        @param texSamplers       Contains one entry for each TextureSampler  of the GrProcessor.
                                 These can be passed to the builder to emit texture reads in the
                                 generated code.
     */
    struct EmitArgs {
        EmitArgs(GrGLSLFPFragmentBuilder* fragBuilder,
                 GrGLSLUniformHandler* uniformHandler,
                 const GrShaderCaps* caps,
                 const GrFragmentProcessor& fp,
                 const char* outputColor,
                 const char* inputColor,
                 const TransformedCoordVars& transformedCoordVars,
                 const TextureSamplers& textureSamplers)
                : fFragBuilder(fragBuilder)
                , fUniformHandler(uniformHandler)
                , fShaderCaps(caps)
                , fFp(fp)
                , fOutputColor(outputColor)
                , fInputColor(inputColor ? inputColor : "half4(1.0)")
                , fTransformedCoords(transformedCoordVars)
                , fTexSamplers(textureSamplers) {}
        GrGLSLFPFragmentBuilder* fFragBuilder;
        GrGLSLUniformHandler* fUniformHandler;
        const GrShaderCaps* fShaderCaps;
        const GrFragmentProcessor& fFp;
        const char* fOutputColor;
        const char* fInputColor;
        const TransformedCoordVars& fTransformedCoords;
        const TextureSamplers& fTexSamplers;
    };

    virtual void emitCode(EmitArgs&) = 0;

    // This does not recurse to any attached child processors. Recursing the entire processor tree
    // is the responsibility of the caller.
    void setData(const GrGLSLProgramDataManager& pdman, const GrFragmentProcessor& processor);

    int numChildProcessors() const { return fChildProcessors.count(); }

    GrGLSLFragmentProcessor* childProcessor(int index) {
        return fChildProcessors[index];
    }

    // Invoke the child with the default input color (solid white)
    inline void invokeChild(int childIndex, const char* outputColor, EmitArgs& parentArgs) {
        this->invokeChild(childIndex, nullptr, outputColor, parentArgs);
    }

    /** Will emit the code of a child proc in its own scope. Pass in the parent's EmitArgs and
     *  invokeChild will automatically extract the coords and samplers of that child and pass them
     *  on to the child's emitCode(). Also, any uniforms or functions emitted by the child will
     *  have their names mangled to prevent redefinitions. The outputColor is the name of a
     *  variable in which to store the resulting color. It is assumed to have already been declared
     *  and is not declared by invokeChild. It is legal to pass nullptr as inputColor, since all
     *  fragment processors are required to work without an input color.
     */
    void invokeChild(int childIndex, const char* inputColor, const char* outputColor,
                     EmitArgs& parentArgs);

    // Use the parent's output color to hold child's output, and use the
    // default input color of solid white
    inline void invokeChild(int childIndex, EmitArgs& args) {
        // null pointer cast required to disambiguate the function call
        this->invokeChild(childIndex, args.fOutputColor, args);
    }

    /**
     * Pre-order traversal of a GLSLFP hierarchy, or of multiple trees with roots in an array of
     * GLSLFPS. This agrees with the traversal order of GrFragmentProcessor::Iter
     */
    class Iter : public SkNoncopyable {
    public:
        explicit Iter(GrGLSLFragmentProcessor* fp) { fFPStack.push_back(fp); }
        explicit Iter(std::unique_ptr<GrGLSLFragmentProcessor> fps[], int cnt) {
            for (int i = cnt - 1; i >= 0; --i) {
                fFPStack.push_back(fps[i].get());
            }
        }
        GrGLSLFragmentProcessor* next();

    private:
        SkSTArray<4, GrGLSLFragmentProcessor*, true> fFPStack;
    };

protected:
    /** A GrGLSLFragmentProcessor instance can be reused with any GrFragmentProcessor that produces
    the same stage key; this function reads data from a GrFragmentProcessor and uploads any
    uniform variables required by the shaders created in emitCode(). The GrFragmentProcessor
    parameter is guaranteed to be of the same type that created this GrGLSLFragmentProcessor and
    to have an identical processor key as the one that created this GrGLSLFragmentProcessor.  */
    virtual void onSetData(const GrGLSLProgramDataManager&, const GrFragmentProcessor&) {}

private:
    void internalEmitChild(int, const char*, EmitArgs&);

    // one per child; either not present or empty string if not yet emitted
    SkTArray<SkString> fFunctionNames;

    SkTArray<GrGLSLFragmentProcessor*, true> fChildProcessors;

    friend class GrFragmentProcessor;
};

#endif
