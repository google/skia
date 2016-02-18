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
     * We now want to find the subset of coords and samplers that belong to the child and its
     * descendants and put that into childCoords and childSamplers. To do so, we'll do a forwards
     * linear search.
     *
     * Explanation:
     * Each GrFragmentProcessor has a copy of all the transforms and textures of itself and
     * all procs in its subtree. For example, suppose we have frag proc A, who has two children B
     * and D. B has a child C, and D has two children E and F. Each frag proc's transforms array
     * contains its own transforms, followed by the transforms of all its descendants (i.e. preorder
     * traversal). Suppose procs A, B, C, D, E, F have 1, 2, 1, 1, 3, 2 transforms respectively.
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
     * EmitArgs that's passed onto D to only contain its and its descendants' coords. The
     * EmitArgs given to A would contain the transforms [a1,b1,b2,c1,d1,e1,e2,e3,f1,f2], and we want
     * to extract the subset [d1,e1,e2,e3,f1,f2] to pass on to D. We can do this with a linear
     * search since we know that A has 1 transform (using A.numTransformsExclChildren()), and B's
     * subtree has 3 transforms (using B.numTransforms()), so we know the start of D's transforms is
     * 4 after the start of A's transforms.
     * Textures work the same way as transforms.
     */
    int firstCoordAt = args.fFp.numTransformsExclChildren();
    int firstSamplerAt = args.fFp.numTexturesExclChildren();
    for (int i = 0; i < childIndex; ++i) {
        firstCoordAt += args.fFp.childProcessor(i).numTransforms();
        firstSamplerAt += args.fFp.childProcessor(i).numTextures();
    }
    GrGLSLTransformedCoordsArray childCoords;
    TextureSamplerArray childSamplers;
    if (childProc.numTransforms() > 0) {
        childCoords.push_back_n(childProc.numTransforms(), &args.fCoords[firstCoordAt]);
    }
    if (childProc.numTextures() > 0) {
        childSamplers.push_back_n(childProc.numTextures(), &args.fSamplers[firstSamplerAt]);
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
                       childCoords,
                       childSamplers);
    this->childProcessor(childIndex)->emitCode(childArgs);
    fragBuilder->codeAppend("}\n");

    fragBuilder->onAfterChildProcEmitCode();
}
