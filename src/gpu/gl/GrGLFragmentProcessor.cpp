/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrGLFragmentProcessor.h"
#include "GrFragmentProcessor.h"
#include "builders/GrGLFragmentShaderBuilder.h"
#include "builders/GrGLProgramBuilder.h"

void GrGLFragmentProcessor::setData(const GrGLProgramDataManager& pdman,
                                    const GrFragmentProcessor& processor) {
    this->onSetData(pdman, processor);
    SkASSERT(fChildProcessors.count() == processor.numChildProcessors());
    for (int i = 0; i < fChildProcessors.count(); ++i) {
        fChildProcessors[i]->setData(pdman, processor.childProcessor(i));
    }
}

void GrGLFragmentProcessor::emitChild(int childIndex, const char* inputColor,
                                      SkString* outputColor, EmitArgs& args) {
    GrGLFragmentBuilder* fb = args.fBuilder->getFragmentShaderBuilder();
    fb->onBeforeChildProcEmitCode();  // call first so mangleString is updated

    const GrFragmentProcessor& childProc = args.fFp.childProcessor(childIndex);

    // Mangle the name of the outputColor
    outputColor->set(args.fOutputColor);
    outputColor->append(fb->getMangleStringThisLevel());

    /*
     * We now want to find the subset of coords and samplers that belong to the child and its
     * descendants and put that into childCoords and childSamplers. To do so, we must do a
     * backwards linear search on coords and samplers.
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
     * So if we're inside proc A's emitCode, and A is about to call emitCode on proc B, we want the
     * EmitArgs that's passed onto B to only contain its and its descendants' coords. The
     * EmitArgs given to A would contain the transforms [a1,b1,b2,c1,d1,e1,e2,e3,f1,f2], and we want
     * to extract the subset [b1,b2,c1] to pass on to B. We can do this with a backwards linear
     * search since we know that D's subtree has 6 transforms and B's subtree has 3 transforms (by
     * calling D.numTextures() and B.numTextures()), so we know the start of B's transforms is 9
     * from the end of A's transforms.  We cannot do this with a forwards linear search since we
     * don't know how many transforms belong to A (A.numTextures() will return 10, not 1), so
     * we wouldn't know how many transforms to initially skip in A's array if using a forward linear
     * search.
     * Textures work the same way as transforms.
     */
    int firstCoordAt = args.fFp.numTransforms();
    int firstSamplerAt = args.fFp.numTextures();
    for (int i = args.fFp.numChildProcessors() - 1; i >= childIndex; --i) {
        firstCoordAt -= args.fFp.childProcessor(i).numTransforms();
        firstSamplerAt -= args.fFp.childProcessor(i).numTextures();
    }
    TransformedCoordsArray childCoords;
    TextureSamplerArray childSamplers;
    if (childProc.numTransforms() > 0) {
        childCoords.push_back_n(childProc.numTransforms(), &args.fCoords[firstCoordAt]);
    }
    if (childProc.numTextures() > 0) {
        childSamplers.push_back_n(childProc.numTextures(), &args.fSamplers[firstSamplerAt]);
    }

    // emit the code for the child in its own scope
    fb->codeAppendf("vec4 %s;\n", outputColor->c_str());
    fb->codeAppend("{\n");
    fb->codeAppendf("// Child Index %d (mangle: %s): %s\n", childIndex,
                    fb->getMangleString().c_str(), childProc.name());
    EmitArgs childArgs(args.fBuilder,
                       childProc,
                       outputColor->c_str(),
                       inputColor,
                       childCoords,
                       childSamplers);
    this->childProcessor(childIndex)->emitCode(childArgs);
    fb->codeAppend("}\n");

    fb->onAfterChildProcEmitCode();
}
