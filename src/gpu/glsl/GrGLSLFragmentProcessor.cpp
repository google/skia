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
    SkASSERT(inputColor);
    GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;

    fragBuilder->onBeforeChildProcEmitCode();  // call first so mangleString is updated

    const GrFragmentProcessor& childProc = args.fFp.childProcessor(childIndex);

    // emit the code for the child in its own scope
    fragBuilder->codeAppend("{\n");
    fragBuilder->codeAppendf("// Child Index %d (mangle: %s): %s\n", childIndex,
                             fragBuilder->getMangleString().c_str(), childProc.name());
    TransformedCoordVars coordVars = args.fTransformedCoords.childInputs(childIndex);
    TextureSamplers textureSamplers = args.fTexSamplers.childInputs(childIndex);
    TexelBuffers texelBuffers = args.fTexelBuffers.childInputs(childIndex);
    ImageStorages imageStorages = args.fImageStorages.childInputs(childIndex);
    EmitArgs childArgs(fragBuilder,
                       args.fUniformHandler,
                       args.fShaderCaps,
                       childProc,
                       outputColor,
                       inputColor,
                       coordVars,
                       textureSamplers,
                       texelBuffers,
                       imageStorages);
    this->childProcessor(childIndex)->emitCode(childArgs);
    fragBuilder->codeAppend("}\n");

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
