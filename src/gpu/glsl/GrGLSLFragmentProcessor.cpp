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

void GrGLSLFragmentProcessor::emitChildFunction(int childIndex, EmitArgs& args) {
    GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;
    while (childIndex >= (int) fFunctionNames.size()) {
        fFunctionNames.emplace_back();
    }

    // Emit the child's helper function if this is the first time we've seen a call
    if (fFunctionNames[childIndex].size() == 0) {
        TransformedCoordVars coordVars = args.fTransformedCoords.childInputs(childIndex);
        TextureSamplers textureSamplers = args.fTexSamplers.childInputs(childIndex);

        EmitArgs childArgs(fragBuilder,
                           args.fUniformHandler,
                           args.fShaderCaps,
                           args.fFp.childProcessor(childIndex),
                           "_output",
                           "_input",
                           "_coords",
                           coordVars,
                           textureSamplers);
        fFunctionNames[childIndex] =
                fragBuilder->writeProcessorFunction(this->childProcessor(childIndex), childArgs);
    }
}

SkString GrGLSLFragmentProcessor::invokeChild(int childIndex, const char* inputColor,
                                              EmitArgs& args, SkSL::String skslCoords) {
    this->emitChildFunction(childIndex, args);

    if (skslCoords.empty()) {
        // Empty coords means passing through the coords of the parent
        skslCoords = args.fSampleCoord;
    }

    const GrFragmentProcessor& childProc = args.fFp.childProcessor(childIndex);

    if (childProc.isSampledWithExplicitCoords()) {
        // The child's function takes a half4 color and a float2 coordinate
        return SkStringPrintf("%s(%s, %s)", fFunctionNames[childIndex].c_str(),
                                            inputColor ? inputColor : "half4(1)",
                                            skslCoords.c_str());
    } else {
        // The child's function just takes a color; we should only get here for a call to
        // sample(color) without explicit coordinates, so assert that the child has no sample matrix
        // and skslCoords is _coords (a const/uniform sample call would go through
        // invokeChildWithMatrix, and if a child was sampled with sample(matrix) and sample(), it
        // should have been flagged as variable and hit the branch above).
        SkASSERT(skslCoords == args.fSampleCoord && childProc.sampleMatrix().isNoOp());
        return SkStringPrintf("%s(%s)", fFunctionNames[childIndex].c_str(),
                                        inputColor ? inputColor : "half4(1)");
    }
}

SkString GrGLSLFragmentProcessor::invokeChildWithMatrix(int childIndex, const char* inputColor,
                                                        EmitArgs& args,
                                                        SkSL::String skslMatrix) {
    this->emitChildFunction(childIndex, args);

    const GrFragmentProcessor& childProc = args.fFp.childProcessor(childIndex);

    // Since this is const/uniform, the provided sksl expression should exactly match the
    // expression stored on the FP, or it should match the mangled uniform name.
    if (skslMatrix.empty()) {
        // Empty matrix expression replaces with the sampleMatrix expression stored on the FP, but
        // that is only valid for const/uniform sampled FPs
        SkASSERT(childProc.sampleMatrix().isConstUniform());
        skslMatrix.assign(childProc.sampleMatrix().fExpression);
    }

    if (childProc.sampleMatrix().isConstUniform()) {
        // Attempt to resolve the uniform name from the raw name that was stored in the sample
        // matrix. Since this is const/uniform, the provided expression better match what was given
        // to the FP.
        SkASSERT(childProc.sampleMatrix().fExpression == skslMatrix);
        GrShaderVar uniform = args.fUniformHandler->getUniformMapping(
                args.fFp, SkString(childProc.sampleMatrix().fExpression));
        if (uniform.getType() != kVoid_GrSLType) {
            // Found the uniform, so replace the expression with the actual uniform name
            SkASSERT(uniform.getType() == kFloat3x3_GrSLType);
            skslMatrix = uniform.getName().c_str();
        } // else assume it's a constant expression
    }

    // Produce a string containing the call to the helper function. sample(matrix) is special where
    // the provided skslMatrix expression means that the child FP should be invoked with coords
    // equal to matrix * parent coords. However, if matrix is a constant/uniform AND the parent
    // coords were produced by const/uniform transforms, then this expression is lifted to a vertex
    // shader and is stored in a varying. In that case, childProc will not have a variable sample
    // matrix and will not be sampled explicitly, so its function signature will not take in coords.
    //
    // In all other cases, we need to insert sksl to compute matrix * parent coords and then invoke
    // the function.
    if (childProc.isSampledWithExplicitCoords()) {
        SkASSERT(!childProc.sampleMatrix().isNoOp());
        // Only check perspective for this specific matrix transform, not the aggregate FP property.
        // Any parent perspective will have already been applied when evaluated in the FS.
        if (childProc.sampleMatrix().fHasPerspective) {
            SkString coords3 = args.fFragBuilder->newTmpVarName("coords3");
            args.fFragBuilder->codeAppendf("float3 %s = (%s) * %s.xy1;\n",
                                           coords3.c_str(), skslMatrix.c_str(), args.fSampleCoord);
            return SkStringPrintf("%s(%s, %s.xy / %s.z)", fFunctionNames[childIndex].c_str(),
                                  inputColor ? inputColor : "half4(1)", coords3.c_str(),
                                  coords3.c_str());
        } else {
            return SkStringPrintf("%s(%s, ((%s) * %s.xy1).xy)",
                                  fFunctionNames[childIndex].c_str(),
                                  inputColor ? inputColor : "half4(1)",
                                  skslMatrix.c_str(), args.fSampleCoord);
        }
    } else {
        // A variable matrix expression should mark the child as explicitly sampled. A no-op
        // matrix should match sample(color), not sample(color, matrix).
        SkASSERT(childProc.sampleMatrix().isConstUniform());

        // Since this is const/uniform and not explicitly sampled, it's transform has been
        // promoted to the vertex shader and the signature doesn't take a float2 coord.
        return SkStringPrintf("%s(%s)", fFunctionNames[childIndex].c_str(),
                                        inputColor ? inputColor : "half4(1)");
    }
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
