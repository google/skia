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
#include "glsl/GrGLSLUniformHandler.h"

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
    using TexelBufferHandle  = GrGLSLUniformHandler::TexelBufferHandle;
    using ImageStorageHandle = GrGLSLUniformHandler::ImageStorageHandle;

private:
    /**
     * This class allows the shader builder to provide each GrGLSLFragmentProcesor with an array of
     * generated variables where each generated variable corresponds to an element of an array on
     * the GrFragmentProcessor that generated the GLSLFP. For example, this is used to provide a
     * variable holding transformed coords for each GrCoordTransform owned by the FP.
     */
    template <typename T, typename FPBASE, int (FPBASE::*COUNT)() const>
    class BuilderInputProvider {
    public:
        BuilderInputProvider(const GrFragmentProcessor* fp, const T* ts) : fFP(fp) , fTs(ts) {}

        const T& operator[] (int i) const {
            SkASSERT(i >= 0 && i < (fFP->*COUNT)());
            return fTs[i];
        }

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
    using TransformedCoordVars = BuilderInputProvider<GrShaderVar, GrFragmentProcessor,
                                                      &GrFragmentProcessor::numCoordTransforms>;
    using TextureSamplers = BuilderInputProvider<SamplerHandle, GrResourceIOProcessor,
                                                 &GrResourceIOProcessor::numTextureSamplers>;
    using TexelBuffers = BuilderInputProvider<TexelBufferHandle, GrResourceIOProcessor,
                                                &GrResourceIOProcessor::numBuffers>;
    using ImageStorages = BuilderInputProvider<ImageStorageHandle, GrResourceIOProcessor,
                                               &GrResourceIOProcessor::numImageStorages>;

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
        @param texSamplers       Contains one entry for each TextureSampler  of the GrProcessor.
                                 These can be passed to the builder to emit texture reads in the
                                 generated code.
        @param bufferSamplers    Contains one entry for each BufferAccess of the GrProcessor. These
                                 can be passed to the builder to emit buffer reads in the generated
                                 code.
        @param imageStorages     Contains one entry for each ImageStorageAccess of the GrProcessor.
                                 These can be passed to the builder to emit image loads and stores
                                 in the generated code.
     */
    struct EmitArgs {
        EmitArgs(GrGLSLFPFragmentBuilder* fragBuilder,
                 GrGLSLUniformHandler* uniformHandler,
                 const GrShaderCaps* caps,
                 const GrFragmentProcessor& fp,
                 const char* outputColor,
                 const char* inputColor,
                 const TransformedCoordVars& transformedCoordVars,
                 const TextureSamplers& textureSamplers,
                 const TexelBuffers& texelBuffers,
                 const ImageStorages& imageStorages)
                : fFragBuilder(fragBuilder)
                , fUniformHandler(uniformHandler)
                , fShaderCaps(caps)
                , fFp(fp)
                , fOutputColor(outputColor)
                , fInputColor(inputColor)
                , fTransformedCoords(transformedCoordVars)
                , fTexSamplers(textureSamplers)
                , fTexelBuffers(texelBuffers)
                , fImageStorages(imageStorages) {}
        GrGLSLFPFragmentBuilder* fFragBuilder;
        GrGLSLUniformHandler* fUniformHandler;
        const GrShaderCaps* fShaderCaps;
        const GrFragmentProcessor& fFp;
        const char* fOutputColor;
        const char* fInputColor;
        const TransformedCoordVars& fTransformedCoords;
        const TextureSamplers& fTexSamplers;
        const TexelBuffers& fTexelBuffers;
        const ImageStorages& fImageStorages;
    };

    virtual void emitCode(EmitArgs&) = 0;

    void setData(const GrGLSLProgramDataManager& pdman, const GrFragmentProcessor& processor);

    static void GenKey(const GrProcessor&, const GrShaderCaps&, GrProcessorKeyBuilder*) {}

    int numChildProcessors() const { return fChildProcessors.count(); }

    GrGLSLFragmentProcessor* childProcessor(int index) {
        return fChildProcessors[index];
    }

    inline void emitChild(int childIndex, SkString* outputColor, EmitArgs& parentArgs) {
        this->emitChild(childIndex, "vec4(1.0)", outputColor, parentArgs);
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

    inline void emitChild(int childIndex, EmitArgs& args) {
        this->emitChild(childIndex, "vec4(1.0)", args);
    }

    /** Variation that uses the parent's output color variable to hold the child's output.*/
    void emitChild(int childIndex, const char* inputColor, EmitArgs& parentArgs);

    /**
     * Pre-order traversal of a GLSLFP hierarchy, or of multiple trees with roots in an array of
     * GLSLFPS. This agrees with the traversal order of GrFragmentProcessor::Iter
     */
    class Iter : public SkNoncopyable {
    public:
        explicit Iter(GrGLSLFragmentProcessor* fp) { fFPStack.push_back(fp); }
        explicit Iter(GrGLSLFragmentProcessor* fps[], int cnt) {
            for (int i = cnt - 1; i >= 0; --i) {
                fFPStack.push_back(fps[i]);
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
    void internalEmitChild(int, const char*, const char*, EmitArgs&);

    SkTArray<GrGLSLFragmentProcessor*, true> fChildProcessors;

    friend class GrFragmentProcessor;
};

#endif
