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

void GrGLSLFragmentProcessor::emitChildFunctions(EmitArgs& args) {
    for (int i = 0; i < this->numChildProcessors(); ++i) {
        GrGLSLFragmentProcessor* childGLSLFP = this->childProcessor(i);
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

SkString GrGLSLFragmentProcessor::invokeChild(int childIndex,
                                              const char* inputColor, const char* destColor,
                                              EmitArgs& args, SkSL::String skslCoords) {
    SkASSERT(childIndex >= 0);

    if (!inputColor) {
        inputColor = args.fInputColor;
    }
    if (!destColor) {
        destColor = "half4(1)";
    }

    const GrFragmentProcessor* childProc = args.fFp.childProcessor(childIndex);
    if (!childProc) {
        // If no child processor is provided, return the input color as-is.
        return SkString(inputColor);
    }

    const char* functionName = this->childProcessor(childIndex)->functionName();
    if (childProc->isSampledWithExplicitCoords()) {
        SkASSERT(!childProc->isBlendFunction());

        // Empty coords means passing through the coords of the parent
        const char* coords = (!skslCoords.empty()) ? skslCoords.c_str() : args.fSampleCoord;

        // The child's function takes a half4 color and a float2 coordinate
        return SkStringPrintf("%s(%s, %s)", functionName, inputColor, coords);
    }

    // Assert that the child has no sample matrix and skslCoords matches the default. (A uniform
    // matrix sample call would go through invokeChildWithMatrix, not here.)
    SkASSERT(skslCoords.empty() || skslCoords == args.fSampleCoord);
    SkASSERT(childProc->sampleUsage().isPassThrough());

    return childProc->isBlendFunction()
          ? SkStringPrintf("%s(%s, %s)", functionName, inputColor, destColor)
          : SkStringPrintf("%s(%s)", functionName, inputColor);
}

SkString GrGLSLFragmentProcessor::invokeChildWithMatrix(int childIndex, const char* inputColor,
                                                        const char* destColor, EmitArgs& args) {
    SkASSERT(childIndex >= 0);

    if (!inputColor) {
        inputColor = args.fInputColor;
    }
    if (!destColor) {
        destColor = "half4(1)";
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

    // Produce a string containing the call to the helper function. We have a uniform variable
    // containing our transform (matrixName). If the parent coords were produced by uniform
    // transforms, then the entire expression (matrixName * coords) is lifted to a vertex shader
    // and is stored in a varying. In that case, childProc will not be sampled explicitly, so its
    // function signature will not take in coords.
    //
    // In all other cases, we need to insert sksl to compute matrix * parent coords and then invoke
    // the function.
    const char* functionName = this->childProcessor(childIndex)->functionName();
    if (childProc->isSampledWithExplicitCoords()) {
        SkASSERT(!childProc->isBlendFunction());

        // Only check perspective for this specific matrix transform, not the aggregate FP property.
        // Any parent perspective will have already been applied when evaluated in the FS.
        if (childProc->sampleUsage().fHasPerspective) {
            return SkStringPrintf("%s(%s, proj((%s) * %s.xy1))", functionName,
                                  inputColor, matrixName.c_str(), args.fSampleCoord);
        } else if (args.fShaderCaps->nonsquareMatrixSupport()) {
            return SkStringPrintf("%s(%s, float3x2(%s) * %s.xy1)",
                                  functionName, inputColor,
                                  matrixName.c_str(), args.fSampleCoord);
        } else {
            return SkStringPrintf("%s(%s, ((%s) * %s.xy1).xy)", functionName,
                                  inputColor, matrixName.c_str(), args.fSampleCoord);
        }
    }

    // Since this is uniform and not explicitly sampled, its transform has been promoted to
    // the vertex shader and the signature doesn't take a float2 coord.
    return childProc->isBlendFunction()
                   ? SkStringPrintf("%s(%s, %s)", functionName, inputColor, destColor)
                   : SkStringPrintf("%s(%s)", functionName, inputColor);
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
