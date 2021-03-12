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

void GrGLSLFragmentProcessor::emitChildFunction(int childIndex, EmitArgs& args) {
    SkASSERT(childIndex >= 0);
    SkASSERT(args.fFp.childProcessor(childIndex));
    GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;
    while (childIndex >= (int) fFunctionNames.size()) {
        fFunctionNames.emplace_back();
    }

    // Emit the child's helper function if this is the first time we've seen a call
    if (fFunctionNames[childIndex].size() == 0) {
        TransformedCoordVars coordVars = args.fTransformedCoords.childInputs(childIndex);
        EmitArgs childArgs(fragBuilder,
                           args.fUniformHandler,
                           args.fShaderCaps,
                           *args.fFp.childProcessor(childIndex),
                           "_input",
                           "_coords",
                           coordVars);
        fFunctionNames[childIndex] =
                fragBuilder->writeProcessorFunction(this->childProcessor(childIndex), childArgs);
    }
}

SkString GrGLSLFragmentProcessor::invokeChild(int childIndex, const char* inputColor,
                                              EmitArgs& args, SkSL::String skslCoords) {
    if (!inputColor) {
        inputColor = args.fInputColor;
    }

    SkASSERT(childIndex >= 0);
    const GrFragmentProcessor* childProc = args.fFp.childProcessor(childIndex);
    if (!childProc) {
        return SkString(inputColor);
    }

    this->emitChildFunction(childIndex, args);

    if (skslCoords.empty()) {
        // Empty coords means passing through the coords of the parent
        skslCoords = args.fSampleCoord;
    }

    if (childProc->isSampledWithExplicitCoords()) {
        // The child's function takes a half4 color and a float2 coordinate
        return SkStringPrintf("%s(%s, %s)", fFunctionNames[childIndex].c_str(),
                                            inputColor, skslCoords.c_str());
    } else {
        // The child's function just takes a color. We should only get here for a call to sample
        // without explicit coordinates. Assert that the child has no sample matrix and skslCoords
        // is _coords (a uniform matrix sample call would go through invokeChildWithMatrix, and if
        // a child was sampled with sample(matrix) and sample(), it should have been flagged as
        // variable and hit the branch above).
        SkASSERT(skslCoords == args.fSampleCoord && !childProc->sampleUsage().hasMatrix());
        return SkStringPrintf("%s(%s)", fFunctionNames[childIndex].c_str(), inputColor);
    }
}

SkString GrGLSLFragmentProcessor::invokeChildWithMatrix(int childIndex, const char* inputColor,
                                                        EmitArgs& args,
                                                        SkSL::String skslMatrix) {
    if (!inputColor) {
        inputColor = args.fInputColor;
    }

    SkASSERT(childIndex >= 0);
    const GrFragmentProcessor* childProc = args.fFp.childProcessor(childIndex);
    if (!childProc) {
        return SkString(inputColor);
    }

    this->emitChildFunction(childIndex, args);

    SkASSERT(childProc->sampleUsage().hasMatrix());

    // Since this is uniform, the provided sksl expression should exactly match the expression
    // stored on the FP, or it should match the mangled uniform name.
    if (skslMatrix.empty()) {
        // Empty matrix expression replaces with the sample matrix expression stored on the FP, but
        // that is only valid for uniform sampled FPs
        SkASSERT(childProc->sampleUsage().hasUniformMatrix());
        skslMatrix.assign(childProc->sampleUsage().fExpression);
    }

    if (childProc->sampleUsage().hasUniformMatrix()) {
        // Attempt to resolve the uniform name from the raw name stored in the sample usage.
        // Since this is uniform, the provided expression better match what was given to the FP.
        SkASSERT(childProc->sampleUsage().fExpression == skslMatrix);
        GrShaderVar uniform = args.fUniformHandler->getUniformMapping(
                args.fFp, SkString(childProc->sampleUsage().fExpression));
        if (uniform.getType() != kVoid_GrSLType) {
            // Found the uniform, so replace the expression with the actual uniform name
            SkASSERT(uniform.getType() == kFloat3x3_GrSLType);
            skslMatrix = uniform.getName().c_str();
        } // else assume it's a constant expression
    }

    // Produce a string containing the call to the helper function. sample(matrix) is special where
    // the provided skslMatrix expression means that the child FP should be invoked with coords
    // equal to matrix * parent coords. However, if matrix is a uniform expression AND the parent
    // coords were produced by uniform transforms, then this expression is lifted to a vertex
    // shader and is stored in a varying. In that case, childProc will not have a variable sample
    // matrix and will not be sampled explicitly, so its function signature will not take in coords.
    //
    // In all other cases, we need to insert sksl to compute matrix * parent coords and then invoke
    // the function.
    if (childProc->isSampledWithExplicitCoords()) {
        // Only check perspective for this specific matrix transform, not the aggregate FP property.
        // Any parent perspective will have already been applied when evaluated in the FS.
        if (childProc->sampleUsage().fHasPerspective) {
            return SkStringPrintf("%s(%s, proj((%s) * %s.xy1))", fFunctionNames[childIndex].c_str(),
                                  inputColor, skslMatrix.c_str(), args.fSampleCoord);
        } else {
            return SkStringPrintf("%s(%s, ((%s) * %s.xy1).xy)", fFunctionNames[childIndex].c_str(),
                                  inputColor, skslMatrix.c_str(), args.fSampleCoord);
        }
    } else {
        // A variable matrix expression should mark the child as explicitly sampled. A no-op
        // matrix should match sample(color), not sample(color, matrix).
        SkASSERT(childProc->sampleUsage().hasUniformMatrix());

        // Since this is uniform and not explicitly sampled, it's transform has been promoted to
        // the vertex shader and the signature doesn't take a float2 coord.
        return SkStringPrintf("%s(%s)", fFunctionNames[childIndex].c_str(), inputColor);
    }
}

//////////////////////////////////////////////////////////////////////////////

GrGLSLFragmentProcessor::Iter::Iter(std::unique_ptr<GrGLSLFragmentProcessor> fps[], int cnt) {
    for (int i = cnt - 1; i >= 0; --i) {
        fFPStack.push_back(fps[i].get());
    }
}

GrGLSLFragmentProcessor::ParallelIter::ParallelIter(const GrFragmentProcessor& fp,
                                                    GrGLSLFragmentProcessor& glslFP)
        : fpIter(fp), glslIter(glslFP) {}

GrGLSLFragmentProcessor::ParallelIter& GrGLSLFragmentProcessor::ParallelIter::operator++() {
    ++fpIter;
    ++glslIter;
    SkASSERT(static_cast<bool>(fpIter) == static_cast<bool>(glslIter));
    return *this;
}

std::tuple<const GrFragmentProcessor&, GrGLSLFragmentProcessor&>
GrGLSLFragmentProcessor::ParallelIter::operator*() const {
    return {*fpIter, *glslIter};
}

bool GrGLSLFragmentProcessor::ParallelIter::operator==(const ParallelIterEnd& end) const {
    SkASSERT(static_cast<bool>(fpIter) == static_cast<bool>(glslIter));
    return !fpIter;
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
        if (auto child = back->childProcessor(i)) {
            fFPStack.push_back(child);
        }
    }
    return *this;
}

GrGLSLFragmentProcessor::ParallelRange::ParallelRange(const GrFragmentProcessor& fp,
                                                      GrGLSLFragmentProcessor& glslFP)
        : fInitialFP(fp), fInitialGLSLFP(glslFP) {}
