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

#include <iostream>

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
    fragBuilder->codeAppendf("half4 %s;", outputColor->c_str());
    this->internalEmitChild(childIndex, inputColor, outputColor->c_str(), args);
}

void GrGLSLFragmentProcessor::internalEmitChild(int childIndex, const char* inputColor,
                                                const char* outputColor, EmitArgs& args) {
    GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;

    fragBuilder->onBeforeChildProcEmitCode();  // call first so mangleString is updated

    // Prepare a mangled input color variable if the default is not used
    // TODO(discuss): should this be before or after onBeforeChildProcEmitCode()?
    // I think its correct to have the declaration outside of the child scope,
    // but should it use child's mangle string or the caller's mangle string?
    // If not the child's, _childInput%s should probably be updated to
    // _childInput_%d_%s and include the childIndex, but is basically equivalent
    // to what the child's mangle string would be.  The only weird thing about it
    // is that we're currently using the mangle string before the block that uses
    // it is actually started.
    SkString inputName;
    if (inputColor&& strcmp("half4(1.0)", inputColor) != 0 && strcmp("half4(1)", inputColor) != 0) {
        inputName.appendf("_childInput%s", fragBuilder->getMangleString().c_str());
        fragBuilder->codeAppendf("half4 %s = %s;", inputName.c_str(), inputColor);
    }

    const GrFragmentProcessor& childProc = args.fFp.childProcessor(childIndex);

    // emit the code for the child in its own scope
    fragBuilder->codeAppend("{\n");
    fragBuilder->codeAppendf("// Child Index %d (mangle: %s): %s\n", childIndex,
                             fragBuilder->getMangleString().c_str(), childProc.name());
    TransformedCoordVars coordVars = args.fTransformedCoords.childInputs(childIndex);
    TextureSamplers textureSamplers = args.fTexSamplers.childInputs(childIndex);

    // EmitArgs properly updates inputColor to half4(1) if it was null
    EmitArgs childArgs(fragBuilder,
                       args.fUniformHandler,
                       args.fShaderCaps,
                       childProc,
                       outputColor,
                       inputName.size() > 0 ? inputName.c_str() : nullptr,
                       coordVars,
                       textureSamplers);
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
