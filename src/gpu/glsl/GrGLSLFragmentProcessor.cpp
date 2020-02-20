/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrCoordTransform.h"
#include "src/gpu/GrFragmentProcessor.h"
#include "src/gpu/GrProcessor.h"
#include "src/gpu/glsl/GrGLSLFragmentProcessor.h"
#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/glsl/GrGLSLUniformHandler.h"

void GrGLSLFragmentProcessor::setData(const GrGLSLProgramDataManager& pdman,
                                      const GrFragmentProcessor& processor) {
    this->onSetData(pdman, processor);
}

SkString GrGLSLFragmentProcessor::invokeChild(int childIndex, const char* inputColor,
                                              EmitArgs& args, SkSL::String skslCoords) {
    GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;
    while (childIndex >= (int) fFunctionNames.size()) {
        fFunctionNames.emplace_back();
    }

    // Subtle bug workaround: If an FP (this) has a child, and wishes to sample it, but does not
    // want to *force* explicit coord sampling, then the obvious solution is to call it with
    // invokeChild and no coords. However, if this FP is then adopted as a child of another FP that
    // does want to sample with explicit coords, that property is propagated (recursively) to all
    // children, and we need to supply explicit coords. So we propagate our own "_coords" (this is
    // the name of our explicit coords parameter generated in the helper function).
    if (args.fFp.isSampledWithExplicitCoords() && skslCoords.length() == 0) {
        skslCoords = "_coords";
    }

    const GrFragmentProcessor& childProc = args.fFp.childProcessor(childIndex);

    // Emit the child's helper function if this is the first time we've seen a call
    if (fFunctionNames[childIndex].size() == 0) {
        fragBuilder->onBeforeChildProcEmitCode();  // call first so mangleString is updated

        TransformedCoordVars coordVars = args.fTransformedCoords.childInputs(childIndex);
        TextureSamplers textureSamplers = args.fTexSamplers.childInputs(childIndex);

        EmitArgs childArgs(fragBuilder,
                           args.fUniformHandler,
                           args.fShaderCaps,
                           childProc,
                           "_output",
                           "_input",
                           coordVars,
                           textureSamplers);
        fFunctionNames[childIndex] =
                fragBuilder->writeProcessorFunction(this->childProcessor(childIndex), childArgs);

        fragBuilder->onAfterChildProcEmitCode();
    }

    // If the fragment processor is invoked with overridden coordinates, it must *always* be invoked
    // with overridden coords.
    SkASSERT(childProc.isSampledWithExplicitCoords() == !skslCoords.empty());

    // Produce a string containing the call to the helper function
    SkString result = SkStringPrintf("%s(%s", fFunctionNames[childIndex].c_str(),
                                              inputColor ? inputColor : "half4(1)");
    if (skslCoords.length()) {
        result.appendf(", %s", skslCoords.c_str());
    }
    result.append(")");
    return result;
}

//////////////////////////////////////////////////////////////////////////////

GrGLSLFragmentProcessor::Iter::Iter(std::unique_ptr<GrGLSLFragmentProcessor> fps[], int cnt) {
    for (int i = cnt - 1; i >= 0; --i) {
        fFPStack.push_back(fps[i].get());
    }
}

GrGLSLFragmentProcessor& GrGLSLFragmentProcessor::Iter::operator*() const {
    return *fFPStack.back();
}

GrGLSLFragmentProcessor* GrGLSLFragmentProcessor::Iter::operator->() const {
    return fFPStack.back();
}

GrGLSLFragmentProcessor::Iter& GrGLSLFragmentProcessor::Iter::operator++() {
    SkASSERT(!fFPStack.empty());
    const GrGLSLFragmentProcessor* back = fFPStack.back();
    fFPStack.pop_back();
    for (int i = back->numChildProcessors() - 1; i >= 0; --i) {
        fFPStack.push_back(back->childProcessor(i));
    }
    return *this;
}
