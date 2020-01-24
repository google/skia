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

SkString GrGLSLFragmentProcessor::writeChildCall(GrGLSLFPFragmentBuilder* fragBuilder,
                                                 int childIndex,
                                                 const char* inputColor,
                                                 const GrFragmentProcessor& childFP,
                                                 SkSL::String skslCoords) {
    // if the fragment processor is invoked with overridden coordinates, it must *always* be invoked
    // with overridden coords
    SkASSERT(childFP.coordTransformsApplyToLocalCoords() == (skslCoords.length() == 0));
    SkString result = SkStringPrintf("%s(%s", fFunctionNames[childIndex].c_str(),
                                              inputColor ? inputColor : "half4(1)");
    if (skslCoords.length()) {
        result.appendf(", %s", skslCoords.c_str());
    }
    result.append(")");
    return result;
}

SkString GrGLSLFragmentProcessor::invokeChild(int childIndex, const char* inputColor,
                                              EmitArgs& args, SkSL::String skslCoords) {
    GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;
    while (childIndex >= (int) fFunctionNames.size()) {
        fFunctionNames.emplace_back();
    }
    if (!args.fFp.coordTransformsApplyToLocalCoords() && skslCoords.length() == 0) {
        skslCoords = "_coords";
    }
    if (fFunctionNames[childIndex].size() == 0) {
        return this->internalInvokeChild(childIndex, inputColor, args, skslCoords);
    } else {
        const GrFragmentProcessor& childProc = args.fFp.childProcessor(childIndex);
        return this->writeChildCall(fragBuilder, childIndex, inputColor, childProc, skslCoords);
    }
}

SkString GrGLSLFragmentProcessor::internalInvokeChild(int childIndex, const char* inputColor,
                                                      EmitArgs& args, SkSL::String skslCoords) {
    GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;

    fragBuilder->onBeforeChildProcEmitCode();  // call first so mangleString is updated

    const GrFragmentProcessor& childProc = args.fFp.childProcessor(childIndex);
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
    SkString result =
            this->writeChildCall(fragBuilder, childIndex, inputColor, childProc, skslCoords);
    fragBuilder->onAfterChildProcEmitCode();
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
