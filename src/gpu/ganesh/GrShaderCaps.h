/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrShaderCaps_DEFINED
#define GrShaderCaps_DEFINED

#include "include/private/base/SkAssert.h"
#include "src/sksl/SkSLUtil.h"

class SkJSONWriter;
struct GrContextOptions;

struct GrShaderCaps : SkSL::ShaderCaps {
    GrShaderCaps() {}

    void dumpJSON(SkJSONWriter*) const;

    const char* noperspectiveInterpolationExtensionString() const {
        SkASSERT(this->fNoPerspectiveInterpolationSupport);
        return fNoPerspectiveInterpolationExtensionString;
    }

    const char* sampleVariablesExtensionString() const {
        SkASSERT(this->fSampleMaskSupport);
        return fSampleVariablesExtensionString;
    }

    void applyOptionsOverrides(const GrContextOptions& options);

    bool fDstReadInShaderSupport = false;
    bool fPreferFlatInterpolation = false;
    bool fVertexIDSupport = false;
    // Returns true if `expr` in `myArray[expr]` can be any integer expression. If false, `expr`
    // must be a constant-index-expression as defined in the OpenGL ES2 specification, Appendix A.5.
    bool fNonconstantArrayIndexSupport = false;
    // frexp(), ldexp(), findMSB(), findLSB().
    bool fBitManipulationSupport = false;
    bool fHalfIs32Bits = false;
    bool fHasLowFragmentPrecision = false;
    // Use a reduced set of rendering algorithms or less optimal effects in order to reduce the
    // number of unique shaders generated.
    bool fReducedShaderMode = false;

    // Used for specific driver bug workarounds
    bool fRequiresLocalOutputColorForFBFetch = false;
    // Workaround for Mali GPU opacity bug with uniform colors.
    bool fMustObfuscateUniformColor = false;
    // On Nexus 6, the GL context can get lost if a shader does not write a value to gl_FragColor.
    // https://bugs.chromium.org/p/chromium/issues/detail?id=445377
    bool fMustWriteToFragColor = false;
    // When we have the option of using either dFdx or dfDy in a shader, this returns whether we
    // should avoid using dFdx. We have found some drivers have bugs or lower precision when using
    // dFdx.
    bool fAvoidDfDxForGradientsWhenPossible = false;

    // This contains the name of an extension that must be enabled in the shader, if such a thing is
    // required in order to use a secondary output in the shader. This returns a nullptr if no such
    // extension is required. However, the return value of this function does not say whether dual
    // source blending is supported.
    const char* fSecondaryOutputExtensionString = nullptr;

    const char* fNoPerspectiveInterpolationExtensionString = nullptr;
    const char* fSampleVariablesExtensionString = nullptr;

    const char* fFBFetchExtensionString = nullptr;

    int fMaxFragmentSamplers = 0;
};

#endif
