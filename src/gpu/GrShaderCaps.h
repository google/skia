/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrShaderCaps_DEFINED
#define GrShaderCaps_DEFINED

#include "include/core/SkRefCnt.h"
#include "src/gpu/glsl/GrGLSL.h"
#include "src/sksl/SkSLUtil.h"

struct GrContextOptions;
class SkJSONWriter;

struct GrShaderCaps : SkSL::ShaderCaps {
    GrShaderCaps() {}

    //
    // TODO: Remove these unnecessary accessors
    //
    void dumpJSON(SkJSONWriter*) const;

    bool supportsDistanceFieldText() const { return fShaderDerivativeSupport; }

    bool dstReadInShaderSupport() const { return fDstReadInShaderSupport; }
    bool dualSourceBlendingSupport() const { return fDualSourceBlendingSupport; }

    const char* fbFetchExtensionString() const { return fFBFetchExtensionString; }

    bool preferFlatInterpolation() const { return fPreferFlatInterpolation; }

    bool vertexIDSupport() const { return fVertexIDSupport; }

    // isinf() is defined, and floating point infinities are handled according to IEEE standards.
    bool infinitySupport() const { return fInfinitySupport; }

    // Returns true if `expr` in `myArray[expr]` can be any integer expression. If false, `expr`
    // must be a constant-index-expression as defined in the OpenGL ES2 specification, Appendix A.5.
    bool nonconstantArrayIndexSupport() const {
        return fNonconstantArrayIndexSupport;
    }

    // frexp(), ldexp(), findMSB(), findLSB().
    bool bitManipulationSupport() const { return fBitManipulationSupport; }

    bool halfIs32Bits() const { return fHalfIs32Bits; }

    bool hasLowFragmentPrecision() const { return fHasLowFragmentPrecision; }

    // Use a reduced set of rendering algorithms or less optimal effects in order to
    // reduce the number of unique shaders generated.
    bool reducedShaderMode() const { return fReducedShaderMode; }

    /**
     * SkSL ES3 requires support for derivatives, nonsquare matrices and bitwise integer operations.
     */
    bool supportsSkSLES3() const {
        return fShaderDerivativeSupport && fNonsquareMatrixSupport && fIntegerSupport &&
               fGLSLGeneration >= SkSL::GLSLGeneration::k330;
    }

    // SkSL only.
    bool colorSpaceMathNeedsFloat() const { return fColorSpaceMathNeedsFloat; }

    bool requiresLocalOutputColorForFBFetch() const { return fRequiresLocalOutputColorForFBFetch; }

    bool mustObfuscateUniformColor() const { return fMustObfuscateUniformColor; }

    // On Nexus 6, the GL context can get lost if a shader does not write a value to gl_FragColor.
    // https://bugs.chromium.org/p/chromium/issues/detail?id=445377
    bool mustWriteToFragColor() const { return fMustWriteToFragColor; }

    // Some GPUs produce poor results when enabling Metal's fastmath option
    bool canUseFastMath() const { return fCanUseFastMath; }

    // When we have the option of using either dFdx or dfDy in a shader, this returns whether we
    // should avoid using dFdx. We have found some drivers have bugs or lower precision when using
    // dFdx.
    bool avoidDfDxForGradientsWhenPossible() const { return fAvoidDfDxForGradientsWhenPossible; }

    // This returns the name of an extension that must be enabled in the shader, if such a thing is
    // required in order to use a secondary output in the shader. This returns a nullptr if no such
    // extension is required. However, the return value of this function does not say whether dual
    // source blending is supported.
    const char* secondaryOutputExtensionString() const { return fSecondaryOutputExtensionString; }

    const char* noperspectiveInterpolationExtensionString() const {
        SkASSERT(this->noperspectiveInterpolationSupport());
        return fNoPerspectiveInterpolationExtensionString;
    }

    const char* sampleVariablesExtensionString() const {
        SkASSERT(this->sampleMaskSupport());
        return fSampleVariablesExtensionString;
    }

    const char* tessellationExtensionString() const {
        SkASSERT(this->tessellationSupport());
        return fTessellationExtensionString;
    }

    int maxFragmentSamplers() const { return fMaxFragmentSamplers; }

    // Maximum number of segments a tessellation edge can be divided into.
    int maxTessellationSegments() const { return fMaxTessellationSegments; }

    bool tessellationSupport() const { return SkToBool(fMaxTessellationSegments);}

    void applyOptionsOverrides(const GrContextOptions& options);

    bool fDstReadInShaderSupport = false;
    bool fDualSourceBlendingSupport = false;
    bool fPreferFlatInterpolation = false;
    bool fVertexIDSupport = false;
    bool fInfinitySupport = false;
    bool fNonconstantArrayIndexSupport = false;
    bool fBitManipulationSupport = false;
    bool fHalfIs32Bits = false;
    bool fHasLowFragmentPrecision = false;
    bool fReducedShaderMode = false;

    // Used for specific driver bug work arounds
    bool fRequiresLocalOutputColorForFBFetch = false;
    bool fMustObfuscateUniformColor = false;
    bool fMustWriteToFragColor = false;
    bool fColorSpaceMathNeedsFloat = false;
    bool fCanUseFastMath = false;
    bool fAvoidDfDxForGradientsWhenPossible = false;

    const char* fSecondaryOutputExtensionString = nullptr;
    const char* fNoPerspectiveInterpolationExtensionString = nullptr;
    const char* fSampleVariablesExtensionString = nullptr;
    const char* fTessellationExtensionString = nullptr;

    const char* fFBFetchExtensionString = nullptr;

    int fMaxFragmentSamplers = 0;
    int fMaxTessellationSegments = 0;
};

#endif
