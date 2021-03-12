/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLSLFragmentProcessor_DEFINED
#define GrGLSLFragmentProcessor_DEFINED

#include "include/private/SkSLString.h"
#include "src/gpu/GrFragmentProcessor.h"
#include "src/gpu/GrShaderVar.h"
#include "src/gpu/glsl/GrGLSLPrimitiveProcessor.h"
#include "src/gpu/glsl/GrGLSLProgramDataManager.h"
#include "src/gpu/glsl/GrGLSLUniformHandler.h"

class GrProcessor;
class GrProcessorKeyBuilder;
class GrGLSLFPBuilder;
class GrGLSLFPFragmentBuilder;

class GrGLSLFragmentProcessor {
public:
    GrGLSLFragmentProcessor() = default;

    virtual ~GrGLSLFragmentProcessor() = default;

    using UniformHandle      = GrGLSLUniformHandler::UniformHandle;
    using SamplerHandle      = GrGLSLUniformHandler::SamplerHandle;

private:
    /**
     * This class allows the shader builder to provide each GrGLSLFragmentProcessor with an array of
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
            const GrFragmentProcessor* child = fFP->childProcessor(childIdx);
            SkASSERT(child);
            int numToSkip = 0;
            for (const auto& fp : GrFragmentProcessor::FPRange(*fFP)) {
                if (&fp == child) {
                    return BuilderInputProvider(child, fTs + numToSkip);
                }
                numToSkip += (fp.*COUNT)();
            }
            SK_ABORT("Didn't find the child.");
            return {nullptr, nullptr};
        }

    private:
        const GrFragmentProcessor* fFP;
        const T*                   fTs;
    };

public:
    using TransformedCoordVars =
            BuilderInputProvider<GrShaderVar, &GrFragmentProcessor::numVaryingCoordsUsed>;

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
        @param localCoord        The name of a local coord reference to a float2 variable.
        @param transformedCoords Fragment shader variables containing the coords computed using
                                 each of the GrFragmentProcessor's GrCoordTransforms.
     */
    struct EmitArgs {
        EmitArgs(GrGLSLFPFragmentBuilder* fragBuilder,
                 GrGLSLUniformHandler* uniformHandler,
                 const GrShaderCaps* caps,
                 const GrFragmentProcessor& fp,
                 const char* inputColor,
                 const char* sampleCoord,
                 const TransformedCoordVars& transformedCoordVars)
                : fFragBuilder(fragBuilder)
                , fUniformHandler(uniformHandler)
                , fShaderCaps(caps)
                , fFp(fp)
                , fInputColor(inputColor ? inputColor : "half4(1.0)")
                , fSampleCoord(sampleCoord)
                , fTransformedCoords(transformedCoordVars) {}
        GrGLSLFPFragmentBuilder* fFragBuilder;
        GrGLSLUniformHandler* fUniformHandler;
        const GrShaderCaps* fShaderCaps;
        const GrFragmentProcessor& fFp;
        const char* fInputColor;
        const char* fSampleCoord;
        const TransformedCoordVars& fTransformedCoords;
    };

    virtual void emitCode(EmitArgs&) = 0;

    // This does not recurse to any attached child processors. Recursing the entire processor tree
    // is the responsibility of the caller.
    void setData(const GrGLSLProgramDataManager& pdman, const GrFragmentProcessor& processor);

    int numChildProcessors() const { return fChildProcessors.count(); }

    GrGLSLFragmentProcessor* childProcessor(int index) const {
        return fChildProcessors[index].get();
    }

    void emitChildFunction(int childIndex, EmitArgs& parentArgs);

    // Invoke the child with the default input color (solid white)
    inline SkString invokeChild(int childIndex, EmitArgs& parentArgs,
                                SkSL::String skslCoords = "") {
        return this->invokeChild(childIndex, nullptr, parentArgs, skslCoords);
    }

    inline SkString invokeChildWithMatrix(int childIndex, EmitArgs& parentArgs,
                                          SkSL::String skslMatrix = "") {
        return this->invokeChildWithMatrix(childIndex, nullptr, parentArgs, skslMatrix);
    }

    /** Invokes a child proc in its own scope. Pass in the parent's EmitArgs and invokeChild will
     *  automatically extract the coords and samplers of that child and pass them on to the child's
     *  emitCode(). Also, any uniforms or functions emitted by the child will have their names
     *  mangled to prevent redefinitions. The returned string contains the output color (as a call
     *  to the child's helper function). It is legal to pass nullptr as inputColor, since all
     *  fragment processors are required to work without an input color.
     *
     *  When skslCoords is empty, invokeChild corresponds to a call to "sample(child, color)"
     *  in SkSL. When skslCoords is not empty, invokeChild corresponds to a call to
     *  "sample(child, color, float2)", where skslCoords is an SkSL expression that evaluates to a
     *  float2 and is passed in as the 3rd argument.
     */
    SkString invokeChild(int childIndex, const char* inputColor, EmitArgs& parentArgs,
                         SkSL::String skslCoords = "");

    /**
     * As invokeChild, but transforms the coordinates according to the provided matrix. This variant
     * corresponds to a call of "sample(child, color, matrix)" in SkSL, where skslMatrix is an SkSL
     * expression that evaluates to a float3x3 and is passed in as the 3rd argument.
     *
     * If skslMatrix is the empty string, then it is automatically replaced with the expression
     * attached to the child's SampleUsage object. This is only valid if the child is sampled with
     * a const-uniform matrix. If the sample matrix is const-or-uniform, the expression will be
     * automatically resolved to the mangled uniform name.
     */
    SkString invokeChildWithMatrix(int childIndex, const char* inputColor, EmitArgs& parentArgs,
                                   SkSL::String skslMatrix = "");

    /**
     * Pre-order traversal of a GLSLFP hierarchy, or of multiple trees with roots in an array of
     * GLSLFPS. If initialized with an array color followed by coverage processors installed in a
     * program thenthe iteration order will agree with a GrFragmentProcessor::Iter initialized with
     * a GrPipeline that produces the same program key.
     */
    class Iter {
    public:
        Iter(std::unique_ptr<GrGLSLFragmentProcessor> fps[], int cnt);
        Iter(GrGLSLFragmentProcessor& fp) { fFPStack.push_back(&fp); }

        GrGLSLFragmentProcessor& operator*() const;
        GrGLSLFragmentProcessor* operator->() const;
        Iter& operator++();
        operator bool() const { return !fFPStack.empty(); }

        // Because each iterator carries a stack we want to avoid copies.
        Iter(const Iter&) = delete;
        Iter& operator=(const Iter&) = delete;

    private:
        SkSTArray<4, GrGLSLFragmentProcessor*, true> fFPStack;
    };

    class ParallelIterEnd {};

    /**
     * Walks parallel trees of GrFragmentProcessor and associated GrGLSLFragmentProcessors. The
     * GrGLSLFragmentProcessor used to initialize the iterator must have been created by calling
     * GrFragmentProcessor::createGLSLInstance() on the passed GrFragmentProcessor.
     */
    class ParallelIter {
    public:
        ParallelIter(const GrFragmentProcessor& fp, GrGLSLFragmentProcessor& glslFP);

        ParallelIter& operator++();

        std::tuple<const GrFragmentProcessor&, GrGLSLFragmentProcessor&> operator*() const;

        bool operator==(const ParallelIterEnd& end) const;

        bool operator!=(const ParallelIterEnd& end) const { return !(*this == end); }

    private:
        GrFragmentProcessor::CIter fpIter;
        GrGLSLFragmentProcessor::Iter glslIter;
    };

    class ParallelRange {
    public:
        ParallelRange(const GrFragmentProcessor& fp, GrGLSLFragmentProcessor& glslFP);

        ParallelIter begin() { return {fInitialFP, fInitialGLSLFP}; }

        ParallelIterEnd end() { return {}; }

    private:
        const GrFragmentProcessor& fInitialFP;
        GrGLSLFragmentProcessor& fInitialGLSLFP;
    };

protected:
    /** A GrGLSLFragmentProcessor instance can be reused with any GrFragmentProcessor that produces
    the same stage key; this function reads data from a GrFragmentProcessor and uploads any
    uniform variables required by the shaders created in emitCode(). The GrFragmentProcessor
    parameter is guaranteed to be of the same type that created this GrGLSLFragmentProcessor and
    to have an identical processor key as the one that created this GrGLSLFragmentProcessor.  */
    virtual void onSetData(const GrGLSLProgramDataManager&, const GrFragmentProcessor&) {}

private:
    // one per child; either not present or empty string if not yet emitted
    SkTArray<SkString> fFunctionNames;

    SkTArray<std::unique_ptr<GrGLSLFragmentProcessor>, true> fChildProcessors;

    friend class GrFragmentProcessor;
};

#endif
