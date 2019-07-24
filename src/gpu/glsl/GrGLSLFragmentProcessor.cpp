/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrFragmentProcessor.h"
#include "src/gpu/GrProcessor.h"
#include "src/gpu/glsl/GrGLSLFragmentProcessor.h"
#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/glsl/GrGLSLUniformHandler.h"

void GrGLSLFragmentProcessor::setData(const GrGLSLProgramDataManager& pdman,
                                      const GrFragmentProcessor& processor) {
    this->onSetData(pdman, processor);
}

void GrGLSLFragmentProcessor::invokeChild(int childIndex, const char* inputColor, EmitArgs& args) {
    while (childIndex >= (int) fFunctionNames.size()) {
        fFunctionNames.emplace_back();
    }
    this->internalInvokeChild(childIndex, inputColor, args.fOutputColor, args);
}

void GrGLSLFragmentProcessor::invokeChild(int childIndex, const char* inputColor,
                                          SkString* outputColor, EmitArgs& args) {
    SkASSERT(outputColor);
    GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;
    outputColor->append(fragBuilder->getMangleString());
    fragBuilder->codeAppendf("half4 %s;", outputColor->c_str());
    while (childIndex >= (int) fFunctionNames.size()) {
        fFunctionNames.emplace_back();
    }
    if (fFunctionNames[childIndex].size() == 0) {
        this->internalInvokeChild(childIndex, inputColor, outputColor->c_str(), args);
    } else {
        fragBuilder->codeAppendf("%s = %s(%s);", outputColor->c_str(),
                                 fFunctionNames[childIndex].c_str(),
                                 inputColor ? inputColor : "half4(1)");
    }
}

void GrGLSLFragmentProcessor::internalInvokeChild(int childIndex, const char* inputColor,
                                                  const char* outputColor, EmitArgs& args) {
    GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;

    fragBuilder->onBeforeChildProcEmitCode();  // call first so mangleString is updated

    // Prepare a mangled input color variable if the default is not used,
    // inputName remains the empty string if no variable is needed.
    SkString inputName;
    if (inputColor&& strcmp("half4(1.0)", inputColor) != 0 && strcmp("half4(1)", inputColor) != 0) {
        // The input name is based off of the current mangle string, and
        // since this is called after onBeforeChildProcEmitCode(), it will be
        // unique to the child processor (exactly what we want for its input).
        inputName.appendf("_childInput%s", fragBuilder->getMangleString().c_str());
        fragBuilder->codeAppendf("half4 %s = %s;", inputName.c_str(), inputColor);
    }

    const GrFragmentProcessor& childProc = args.fFp.childProcessor(childIndex);

    TransformedCoordVars coordVars = args.fTransformedCoords.childInputs(childIndex);
    TextureSamplers textureSamplers = args.fTexSamplers.childInputs(childIndex);

    EmitArgs childArgs(fragBuilder,
                       args.fUniformHandler,
                       args.fShaderCaps,
                       childProc,
                       outputColor,
                       "_input",
                       coordVars,
                       textureSamplers);
    fFunctionNames[childIndex] = fragBuilder->writeProcessorFunction(
                                                               this->childProcessor(childIndex),
                                                               childArgs);
    fragBuilder->codeAppendf("%s = %s(%s);\n", outputColor,
                                               fFunctionNames[childIndex].c_str(),
                                               inputName.size() > 0 ? inputName.c_str()
                                                                    : "half4(1)");

    fragBuilder->onAfterChildProcEmitCode();
}

//////////////////////////////////////////////////////////////////////////////

GrGLSLFragmentProcessor* GrGLSLFragmentProcessor::Iter::next() {
    if (fFPStack.empty()) {
        return nullptr;
    }
    GrGLSLFragmentProcessor* back = fFPStack.back();
    fFPStack.pop_back();
    for (int i = back->numChildProcessors() - 1; i >= 0; --i) {
        fFPStack.push_back(back->childProcessor(i));
    }
    return back;
}
