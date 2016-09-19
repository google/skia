/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrGLSLFragmentProcessor.h"
#include "GrFragmentProcessor.h"
#include "GrProcessor.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"
#include "glsl/GrGLSLUniformHandler.h"

void GrGLSLFragmentProcessor::setData(const GrGLSLProgramDataManager& pdman,
                                      const GrFragmentProcessor& processor) {
    this->onSetData(pdman, processor);
    SkASSERT(fChildProcessors.count() == processor.numChildProcessors());
    for (int i = 0; i < fChildProcessors.count(); ++i) {
        fChildProcessors[i]->setData(pdman, processor.childProcessor(i));
    }
}

void GrGLSLFragmentProcessor::emitChild(int childIndex, const char* inputColor, EmitArgs& args) {
    this->internalEmitChild(childIndex, inputColor, args.fOutputColor, args);
}

void GrGLSLFragmentProcessor::emitChild(int childIndex, const char* inputColor,
                                        SkString* outputColor, EmitArgs& args) {

    SkASSERT(outputColor);
    GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;
    outputColor->append(fragBuilder->getMangleString());
    fragBuilder->codeAppendf("vec4 %s;", outputColor->c_str());
    this->internalEmitChild(childIndex, inputColor, outputColor->c_str(), args);
}

void GrGLSLFragmentProcessor::internalEmitChild(int childIndex, const char* inputColor,
                                                const char* outputColor, EmitArgs& args) {
    GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;

    fragBuilder->onBeforeChildProcEmitCode();  // call first so mangleString is updated

    const GrFragmentProcessor& childProc = args.fFp.childProcessor(childIndex);

    /*
     * TODO: Move textures and buffers to the iterator model used by coords.
     * We now want to find the subset of samplers that belong to the child and its descendants and
     * put that into childSamplers. To do so, we'll do a forwards linear search.
     *
     * Explanation:
     * Each GrFragmentProcessor has a copy of all the textures of itself and all procs in its
     * subtree. For example, suppose we have frag proc A, who has two children B and D. B has a
     * child C, and D has two children E and F. Each frag proc's textures array contains its own
     * textures, followed by the textures of all its descendants (i.e. preorder traversal). Suppose
     * procs A, B, C, D, E, F have 1, 2, 1, 1, 3, 2 textures respectively.
     *
     *                                   (A)
     *                        [a1,b1,b2,c1,d1,e1,e2,e3,f1,f2]
     *                                  /    \
     *                                /        \
     *                            (B)           (D)
     *                        [b1,b2,c1]   [d1,e1,e2,e3,f1,f2]
     *                          /             /    \
     *                        /             /        \
     *                      (C)          (E)          (F)
     *                     [c1]      [e1,e2,e3]      [f1,f2]
     *
     * So if we're inside proc A's emitCode, and A is about to call emitCode on proc D, we want the
     * EmitArgs that's passed onto D to only contain its and its descendants' textures. The
     * EmitArgs given to A would contain the textures [a1,b1,b2,c1,d1,e1,e2,e3,f1,f2], and we want
     * to extract the subset [d1,e1,e2,e3,f1,f2] to pass on to D. We can do this with a linear
     * search since we know that A has 1 texture (using A.numTexturesExclChildren()), and B's
     * subtree has 3 textures (using B.numTextures()), so we know the start of D's textures is
     * 4 after the start of A's textures.
     * Textures work the same way as textures.
     */
    int firstTextureAt = args.fFp.numTexturesExclChildren();
    int firstBufferAt = args.fFp.numBuffersExclChildren();
    for (int i = 0; i < childIndex; ++i) {
        firstTextureAt += args.fFp.childProcessor(i).numTextures();
        firstBufferAt += args.fFp.childProcessor(i).numBuffers();
    }
    const SamplerHandle* childTexSamplers = nullptr;
    const SamplerHandle* childBufferSamplers =  nullptr;
    if (childProc.numTextures() > 0) {
        childTexSamplers = &args.fTexSamplers[firstTextureAt];
    }
    if (childProc.numBuffers() > 0) {
        childBufferSamplers = &args.fBufferSamplers[firstBufferAt];
    }

    // emit the code for the child in its own scope
    fragBuilder->codeAppend("{\n");
    fragBuilder->codeAppendf("// Child Index %d (mangle: %s): %s\n", childIndex,
                             fragBuilder->getMangleString().c_str(), childProc.name());
    EmitArgs childArgs(fragBuilder,
                       args.fUniformHandler,
                       args.fGLSLCaps,
                       childProc,
                       outputColor,
                       inputColor,
                       args.fTransformedCoords.childTransforms(childIndex),
                       childTexSamplers,
                       childBufferSamplers,
                       args.fGpImplementsDistanceVector);
    this->childProcessor(childIndex)->emitCode(childArgs);
    fragBuilder->codeAppend("}\n");

    fragBuilder->onAfterChildProcEmitCode();
}

//////////////////////////////////////////////////////////////////////////////

using TransformedCoordVars = GrGLSLFragmentProcessor::TransformedCoordVars;
TransformedCoordVars TransformedCoordVars::childTransforms(int childIdx) const {
    const GrFragmentProcessor* child = &fFP->childProcessor(childIdx);
    GrFragmentProcessor::Iter iter(fFP);
    int numToSkip = 0;
    while (true) {
        const GrFragmentProcessor* fp = iter.next();
        if (fp == child) {
            return TransformedCoordVars(child, fTransformedVars + numToSkip);
        }
        numToSkip += fp->numCoordTransforms();
    }
}
