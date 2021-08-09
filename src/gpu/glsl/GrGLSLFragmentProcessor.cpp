/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/glsl/GrGLSLFragmentProcessor.h"

#include "src/gpu/GrFragmentProcessor.h"
#include "src/gpu/GrProcessor.h"
#include "src/gpu/GrShaderCaps.h"
#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/glsl/GrGLSLProgramBuilder.h"
#include "src/gpu/glsl/GrGLSLUniformHandler.h"

using ProgramImpl = GrFragmentProcessor::ProgramImpl;

void ProgramImpl::setData(const GrGLSLProgramDataManager& pdman,
                          const GrFragmentProcessor& processor) {
    this->onSetData(pdman, processor);
}

void ProgramImpl::emitChildFunctions(EmitArgs& args) {
    for (int i = 0; i < this->numChildProcessors(); ++i) {
        ProgramImpl* childGLSLFP = this->childProcessor(i);
        if (!childGLSLFP) {
            continue;
        }

        const GrFragmentProcessor* childFP = args.fFp.childProcessor(i);
        SkASSERT(childFP);

        EmitArgs childArgs(args.fFragBuilder,
                           args.fUniformHandler,
                           args.fShaderCaps,
                           *childFP,
                           childFP->isBlendFunction() ? "_src" : "_input",
                           "_dst",
                           "_coords");
        args.fFragBuilder->writeProcessorFunction(childGLSLFP, childArgs);
    }
}

SkString ProgramImpl::invokeChild(int childIndex,
                                  const char* inputColor,
                                  const char* destColor,
                                  EmitArgs& args,
                                  SkSL::String skslCoords) {
    SkASSERT(childIndex >= 0);

    if (!inputColor) {
        inputColor = args.fInputColor;
    }

    const GrFragmentProcessor* childProc = args.fFp.childProcessor(childIndex);
    if (!childProc) {
        // If no child processor is provided, return the input color as-is.
        return SkString(inputColor);
    }

    auto invocation = SkStringPrintf("%s(%s", this->childProcessor(childIndex)->functionName(),
                                              inputColor);

    if (childProc->isBlendFunction()) {
        if (!destColor) {
            destColor = args.fFp.isBlendFunction() ? args.fDestColor : "half4(1)";
        }
        invocation.appendf(", %s", destColor);
    }

    // Assert that the child has no sample matrix. A uniform matrix sample call would go through
    // invokeChildWithMatrix, not here.
    SkASSERT(!childProc->sampleUsage().isUniformMatrix());

    if (args.fFragBuilder->getProgramBuilder()->fragmentProcessorHasCoordsParam(childProc)) {
        SkASSERT(!childProc->sampleUsage().isFragCoord() || skslCoords == "sk_FragCoord.xy");
        // The child's function takes a half4 color and a float2 coordinate
        invocation.appendf(", %s", skslCoords.empty() ? args.fSampleCoord : skslCoords.c_str());
    }

    invocation.append(")");
    return invocation;
}

SkString ProgramImpl::invokeChildWithMatrix(int childIndex,
                                            const char* inputColor,
                                            const char* destColor,
                                            EmitArgs& args) {
    SkASSERT(childIndex >= 0);

    if (!inputColor) {
        inputColor = args.fInputColor;
    }

    const GrFragmentProcessor* childProc = args.fFp.childProcessor(childIndex);
    if (!childProc) {
        // If no child processor is provided, return the input color as-is.
        return SkString(inputColor);
    }

    SkASSERT(childProc->sampleUsage().isUniformMatrix());

    // Every uniform matrix has the same (initial) name. Resolve that into the mangled name:
    GrShaderVar uniform = args.fUniformHandler->getUniformMapping(
            args.fFp, SkString(SkSL::SampleUsage::MatrixUniformName()));
    SkASSERT(uniform.getType() == kFloat3x3_GrSLType);
    const SkString& matrixName(uniform.getName());

    auto invocation = SkStringPrintf("%s(%s", this->childProcessor(childIndex)->functionName(),
                                              inputColor);

    if (childProc->isBlendFunction()) {
        if (!destColor) {
            destColor = args.fFp.isBlendFunction() ? args.fDestColor : "half4(1)";
        }
        invocation.appendf(", %s", destColor);
    }

    // Produce a string containing the call to the helper function. We have a uniform variable
    // containing our transform (matrixName). If the parent coords were produced by uniform
    // transforms, then the entire expression (matrixName * coords) is lifted to a vertex shader
    // and is stored in a varying. In that case, childProc will not be sampled explicitly, so its
    // function signature will not take in coords.
    //
    // In all other cases, we need to insert sksl to compute matrix * parent coords and then invoke
    // the function.
    if (args.fFragBuilder->getProgramBuilder()->fragmentProcessorHasCoordsParam(childProc)) {
        // Only check perspective for this specific matrix transform, not the aggregate FP property.
        // Any parent perspective will have already been applied when evaluated in the FS.
        if (childProc->sampleUsage().hasPerspective()) {
            invocation.appendf(", proj((%s) * %s.xy1)", matrixName.c_str(), args.fSampleCoord);
        } else if (args.fShaderCaps->nonsquareMatrixSupport()) {
            invocation.appendf(", float3x2(%s) * %s.xy1", matrixName.c_str(), args.fSampleCoord);
        } else {
            invocation.appendf(", ((%s) * %s.xy1).xy", matrixName.c_str(), args.fSampleCoord);
        }
    }

    invocation.append(")");
    return invocation;
}
