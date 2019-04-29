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

void GrGLSLFragmentProcessor::invokeChild(int childIndex, const char* inputColor,
                                          const char* outputColor, EmitArgs& args) {
    SkASSERT(outputColor);
    GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;
//    if (fFunctionName.size() == 0) {
        this->internalEmitChild(childIndex, outputColor, args);
//    }
    fragBuilder->codeAppendf("/*INVOKE_CHILD*/ %s = %s(%s);", outputColor, fFunctionName.c_str(),
                             inputColor ? inputColor : "half4(1)");
}

void GrGLSLFragmentProcessor::internalEmitChild(int childIndex, const char* outputColor,
                                                EmitArgs& args) {
    GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;

    fragBuilder->onBeforeChildProcEmitCode();  // call first so mangleString is updated

    const GrFragmentProcessor& childProc = args.fFp.childProcessor(childIndex);

    // emit the code for the child in its own function
    TransformedCoordVars coordVars = args.fTransformedCoords.childInputs(childIndex);
    TextureSamplers textureSamplers = args.fTexSamplers.childInputs(childIndex);

    fragBuilder->nextStage();
    const char* input = "_input";
    const char* output = "_result";
    // EmitArgs properly updates inputColor to half4(1) if it was null
    EmitArgs childArgs(fragBuilder,
                       args.fUniformHandler,
                       args.fShaderCaps,
                       childProc,
                       output,
                       input,
                       coordVars,
                       textureSamplers);
    fragBuilder->codeAppendf("half4 /*INTERNAL_EMIT_CHILD*/ %s;\n", output);
    this->childProcessor(childIndex)->emitCode(childArgs);
    fragBuilder->codeAppendf("return %s;", output);
    GrShaderVar inColor(input, kHalf4_GrSLType);
    fragBuilder->emitFunction(kHalf4_GrSLType,
                              "stage",
                              1,
                              &inColor,
                              fragBuilder->code().c_str(),
                              &fFunctionName);
    fragBuilder->deleteStage();

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
