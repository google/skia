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
#include "src/gpu/glsl/GrGLSLProgramDataManager.h"
#include "src/gpu/glsl/GrGLSLUniformHandler.h"

class GrGLSLFPFragmentBuilder;

class GrFragmentProcessor::ProgramImpl {
public:
    ProgramImpl() = default;

    virtual ~ProgramImpl() = default;

    using UniformHandle      = GrGLSLUniformHandler::UniformHandle;
    using SamplerHandle      = GrGLSLUniformHandler::SamplerHandle;

    /** Called when the program stage should insert its code into the shaders. The code in each
        shader will be in its own block ({}) and so locally scoped names will not collide across
        stages.

        @param fragBuilder       Interface used to emit code in the shaders.
        @param uniformHandler    Interface used for accessing information about our uniforms
        @param caps              The capabilities of the GPU which will render this FP
        @param fp                The processor that generated this program stage.
        @param inputColor        A half4 that holds the input color to the stage in the FS (or the
                                 source color, for blend processors). nullptr inputs are converted
                                 to "half4(1.0)" (solid white) during construction.
                                 TODO: Better system for communicating optimization info
                                 (e.g. input color is solid white, trans black, known to be opaque,
                                 etc.) that allows the processor to communicate back similar known
                                 info about its output.
        @param destColor         A half4 that holds the dest color to the stage. Only meaningful
                                 when the "is blend processor" FP flag is set.
        @param sampleCoord       The name of a local coord reference to a float2 variable. Only
                                 meaningful when the "references sample coords" FP flag is set.
     */
    struct EmitArgs {
        EmitArgs(GrGLSLFPFragmentBuilder* fragBuilder,
                 GrGLSLUniformHandler* uniformHandler,
                 const GrShaderCaps* caps,
                 const GrFragmentProcessor& fp,
                 const char* inputColor,
                 const char* destColor,
                 const char* sampleCoord)
                : fFragBuilder(fragBuilder)
                , fUniformHandler(uniformHandler)
                , fShaderCaps(caps)
                , fFp(fp)
                , fInputColor(inputColor ? inputColor : "half4(1.0)")
                , fDestColor(destColor)
                , fSampleCoord(sampleCoord) {}
        GrGLSLFPFragmentBuilder* fFragBuilder;
        GrGLSLUniformHandler* fUniformHandler;
        const GrShaderCaps* fShaderCaps;
        const GrFragmentProcessor& fFp;
        const char* fInputColor;
        const char* fDestColor;
        const char* fSampleCoord;
    };

    virtual void emitCode(EmitArgs&) = 0;

    // This does not recurse to any attached child processors. Recursing the entire processor tree
    // is the responsibility of the caller.
    void setData(const GrGLSLProgramDataManager& pdman, const GrFragmentProcessor& processor);

    int numChildProcessors() const { return fChildProcessors.count(); }

    ProgramImpl* childProcessor(int index) const { return fChildProcessors[index].get(); }

    void setFunctionName(SkString name) {
        SkASSERT(fFunctionName.isEmpty());
        fFunctionName = std::move(name);
    }

    const char* functionName() const {
        SkASSERT(!fFunctionName.isEmpty());
        return fFunctionName.c_str();
    }

    void emitChildFunctions(EmitArgs& parentArgs);

    // Invoke the child with the default input and destination colors (solid white)
    inline SkString invokeChild(int childIndex, EmitArgs& parentArgs,
                                SkSL::String skslCoords = "") {
        return this->invokeChild(childIndex, /*inputColor=*/nullptr, /*destColor=*/nullptr,
                                 parentArgs, skslCoords);
    }

    inline SkString invokeChildWithMatrix(int childIndex, EmitArgs& parentArgs) {
        return this->invokeChildWithMatrix(childIndex, /*inputColor=*/nullptr,
                                           /*destColor=*/nullptr, parentArgs);
    }

    // Invoke the child with the default destination color (solid white)
    inline SkString invokeChild(int childIndex, const char* inputColor, EmitArgs& parentArgs,
                                SkSL::String skslCoords = "") {
        return this->invokeChild(childIndex, inputColor, /*destColor=*/nullptr, parentArgs,
                                 skslCoords);
    }

    inline SkString invokeChildWithMatrix(int childIndex, const char* inputColor,
                                          EmitArgs& parentArgs) {
        return this->invokeChildWithMatrix(childIndex, inputColor, /*destColor=*/nullptr,
                                           parentArgs);
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
    SkString invokeChild(int childIndex, const char* inputColor, const char* destColor,
                         EmitArgs& parentArgs, SkSL::String skslCoords = "");

    /**
     * As invokeChild, but transforms the coordinates according to the matrix expression attached
     * to the child's SampleUsage object. This is only valid if the child is sampled with a
     * const-uniform matrix.
     */
    SkString invokeChildWithMatrix(int childIndex, const char* inputColor, const char* destColor,
                                   EmitArgs& parentArgs);

    /**
     * Pre-order traversal of a GLSLFP hierarchy, or of multiple trees with roots in an array of
     * GLSLFPS. If initialized with an array color followed by coverage processors installed in a
     * program thenthe iteration order will agree with a GrFragmentProcessor::Iter initialized with
     * a GrPipeline that produces the same program key.
     */
    class Iter {
    public:
        Iter(std::unique_ptr<ProgramImpl> fps[], int cnt);
        Iter(ProgramImpl& fp) { fFPStack.push_back(&fp); }

        ProgramImpl& operator*() const;
        ProgramImpl* operator->() const;
        Iter& operator++();
        operator bool() const { return !fFPStack.empty(); }

        // Because each iterator carries a stack we want to avoid copies.
        Iter(const Iter&) = delete;
        Iter& operator=(const Iter&) = delete;

    private:
        SkSTArray<4, ProgramImpl*, true> fFPStack;
    };

protected:
    /**
     * A ProgramImpl instance can be reused with any GrFragmentProcessor that produces the same
     * the same key; this function reads data from a GrFragmentProcessor and uploads any
     * uniform variables required by the shaders created in emitCode(). The GrFragmentProcessor
     * parameter is guaranteed to be of the same type that created this ProgramImpl and
     * to have an identical key as the one that created this ProgramImpl.
     */
    virtual void onSetData(const GrGLSLProgramDataManager&, const GrFragmentProcessor&) {}

private:
    // The (mangled) name of our entry-point function
    SkString fFunctionName;

    SkTArray<std::unique_ptr<ProgramImpl>, true> fChildProcessors;

    friend class GrFragmentProcessor;
};

#endif
