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

void GrGLSLFragmentProcessor::invokeChild(int childIndex, const char* inputColor, EmitArgs& args,
                                          SkSL::String skslCoords) {
    while (childIndex >= (int) fFunctionNames.size()) {
        fFunctionNames.emplace_back();
    }
    this->internalInvokeChild(childIndex, inputColor, args.fOutputColor, args, skslCoords);
}

bool find_transformed_coords(GrGLSLFragmentProcessor::TransformedCoordVars coordVars,
                             const GrFragmentProcessor& fp,
                             GrGLSLPrimitiveProcessor::TransformVar* outVar,
                             GrCoordTransform* outTransform) {
    if (coordVars.count()) {
        *outVar = coordVars[0];
        *outTransform = fp.coordTransform(0);
        return true;
    }
    for (int i = 0; i < fp.numChildProcessors(); ++i) {
        if (find_transformed_coords(coordVars.childInputs(i), fp.childProcessor(i), outVar,
                                   outTransform)) {
            return true;
        }
    }
    return false;
}

void GrGLSLFragmentProcessor::invokeChild(int childIndex, const char* inputColor,
                                          SkString* outputColor, EmitArgs& args,
                                          SkSL::String skslCoords) {
    SkASSERT(outputColor);
    GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;
    outputColor->append(fragBuilder->getMangleString());
    fragBuilder->codeAppendf("half4 %s;", outputColor->c_str());
    while (childIndex >= (int) fFunctionNames.size()) {
        fFunctionNames.emplace_back();
    }
    if (fFunctionNames[childIndex].size() == 0) {
        this->internalInvokeChild(childIndex, inputColor, outputColor->c_str(), args, skslCoords);
    } else {
        TransformedCoordVars coordVars = args.fTransformedCoords.childInputs(childIndex);
        std::vector<SkString> coordParams;
        for (int i = 0; i < coordVars.count(); ++i) {
            coordParams.push_back(fragBuilder->ensureCoords2D(coordVars[i].fVaryingPoint));
        }
        bool needRestore = false;
        GrGLSLPrimitiveProcessor::TransformVar coordVar;
        if (skslCoords.length()) {
            GrCoordTransform transform;
            if (find_transformed_coords(args.fTransformedCoords.childInputs(childIndex),
                                        args.fFp.childProcessor(childIndex),
                                        &coordVar,
                                        &transform)) {
                fragBuilder->codeAppendf("float2 %s_saved = %s_current;\n",
                                         coordVar.fVaryingPoint.c_str(),
                                         coordVar.fVaryingPoint.c_str());
                fragBuilder->codeAppendf("%s_current = (float3(%s, 1) * %s).xy;\n",
                                         coordVar.fVaryingPoint.c_str(), skslCoords.c_str(),
                                         coordVar.fMatrixCode.c_str());
                if (coordVar.fUniformMatrix.isValid()) {
                    args.fUniformHandler->updateUniformVisibility(coordVar.fUniformMatrix,
                                                                  kFragment_GrShaderFlag);
                }
                needRestore = true;
            }
        }
        fragBuilder->codeAppendf("%s = %s(%s);", outputColor->c_str(),
                                 fFunctionNames[childIndex].c_str(),
                                 inputColor ? inputColor : "half4(1)");
        if (needRestore) {
            fragBuilder->codeAppendf("%s_current = %s_saved;\n", coordVar.fVaryingPoint.c_str(),
                                     coordVar.fVaryingPoint.c_str());
        }
    }
}

void GrGLSLFragmentProcessor::internalInvokeChild(int childIndex, const char* inputColor,
                                                  const char* outputColor, EmitArgs& args,
                                                  SkSL::String skslCoordsArg) {
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
    std::vector<SkString> coordParams;
    for (int i = 0; i < coordVars.count(); ++i) {
        coordParams.push_back(fragBuilder->ensureCoords2D(coordVars[i].fVaryingPoint));
    }
    bool needRestore = false;
    GrGLSLPrimitiveProcessor::TransformVar coordVar;
    if (skslCoordsArg.length()) {
        GrCoordTransform transform;
        if (find_transformed_coords(args.fTransformedCoords.childInputs(childIndex),
                                    args.fFp.childProcessor(childIndex),
                                    &coordVar,
                                    &transform)) {
            fragBuilder->codeAppendf("float2 %s_saved = %s_current;\n",
                                     coordVar.fVaryingPoint.c_str(),
                                     coordVar.fVaryingPoint.c_str());
            fragBuilder->codeAppendf("%s_current = (float3(%s, 1) * %s).xy;\n",
                                     coordVar.fVaryingPoint.c_str(), skslCoordsArg.c_str(),
                                     coordVar.fMatrixCode.c_str());
            if (coordVar.fUniformMatrix.isValid()) {
                args.fUniformHandler->updateUniformVisibility(coordVar.fUniformMatrix,
                                                              kFragment_GrShaderFlag);
            }
            needRestore = true;
        }
    }
    fragBuilder->codeAppendf("%s = %s(%s);", outputColor,
                             fFunctionNames[childIndex].c_str(),
                             inputColor ? inputColor : "half4(1)");
    if (needRestore) {
        fragBuilder->codeAppendf("%s_current = %s_saved;\n", coordVar.fVaryingPoint.c_str(),
                                 coordVar.fVaryingPoint.c_str());
    }

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
