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
    // If the fragment processor is invoked with overridden coordinates, it must *always* be invoked
    // with overridden coords
    if (!args.fFp.coordTransformsApplyToLocalCoords() && skslCoords.length() == 0) {
        skslCoords = "_coords";
    }

    // Emit the child's helper function if this is the first time we've seen a call
    if (fFunctionNames[childIndex].size() == 0) {
        fragBuilder->onBeforeChildProcEmitCode();  // call first so mangleString is updated

        EmitArgs childArgs(fragBuilder,
                           args.fUniformHandler,
                           args.fShaderCaps,
                           args.fFp.childProcessor(childIndex),
                           "_output",
                           "_input",
                           args.fTransformedCoords.childInputs(childIndex),
                           args.fTexSamplers.childInputs(childIndex));
        fFunctionNames[childIndex] =
                fragBuilder->writeProcessorFunction(this->childProcessor(childIndex), childArgs);

        fragBuilder->onAfterChildProcEmitCode();
    }

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
