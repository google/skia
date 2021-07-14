/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrFragmentProcessor.h"
#include "src/gpu/GrProcessor.h"
#include "src/gpu/GrShaderCaps.h"
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
        EmitArgs childArgs(fragBuilder,
                           args.fUniformHandler,
                           args.fShaderCaps,
                           *args.fFp.childProcessor(childIndex),
                           "_input",
                           "_coords");
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
        // is _coords (a uniform matrix sample call would go through invokeChildWithMatrix).
        SkASSERT(skslCoords == args.fSampleCoord && childProc->sampleUsage().isPassThrough());
        return SkStringPrintf("%s(%s)", fFunctionNames[childIndex].c_str(), inputColor);
    }
}

SkString GrGLSLFragmentProcessor::invokeChildWithMatrix(int childIndex, const char* inputColor,
                                                        EmitArgs& args) {
    if (!inputColor) {
        inputColor = args.fInputColor;
    }

    SkASSERT(childIndex >= 0);
    const GrFragmentProcessor* childProc = args.fFp.childProcessor(childIndex);
    if (!childProc) {
        return SkString(inputColor);
    }

    this->emitChildFunction(childIndex, args);

    SkASSERT(childProc->sampleUsage().isUniformMatrix());

    // Every uniform matrix has the same (initial) name. Resolve that into the mangled name:
    GrShaderVar uniform = args.fUniformHandler->getUniformMapping(
            args.fFp, SkString(SkSL::SampleUsage::MatrixUniformName()));
    SkASSERT(uniform.getType() == kFloat3x3_GrSLType);
    const SkString& matrixName(uniform.getName());

    // Produce a string containing the call to the helper function. We have a uniform variable
    // containing our transform (matrixName). If the parent coords were produced by uniform
    // transforms, then the entire expression (matrixName * coords) is lifted to a vertex shader
    // and is stored in a varying. In that case, childProc will not be sampled explicitly, so its
    // function signature will not take in coords.
    //
    // In all other cases, we need to insert sksl to compute matrix * parent coords and then invoke
    // the function.
    if (childProc->isSampledWithExplicitCoords()) {
        // Only check perspective for this specific matrix transform, not the aggregate FP property.
        // Any parent perspective will have already been applied when evaluated in the FS.
        if (childProc->sampleUsage().fHasPerspective) {
            return SkStringPrintf("%s(%s, proj((%s) * %s.xy1))", fFunctionNames[childIndex].c_str(),
                                  inputColor, matrixName.c_str(), args.fSampleCoord);
        } else if (args.fShaderCaps->nonsquareMatrixSupport()) {
            return SkStringPrintf("%s(%s, float3x2(%s) * %s.xy1)",
                                  fFunctionNames[childIndex].c_str(), inputColor,
                                  matrixName.c_str(), args.fSampleCoord);
        } else {
            return SkStringPrintf("%s(%s, ((%s) * %s.xy1).xy)", fFunctionNames[childIndex].c_str(),
                                  inputColor, matrixName.c_str(), args.fSampleCoord);
        }
    } else {
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
