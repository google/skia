/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_PROGRAMSETTINGS
#define SKSL_PROGRAMSETTINGS

#include "include/private/SkSLDefines.h"

namespace SkSL {

/**
 * SkSL supports several different program kinds.
 */
enum class ProgramKind : int8_t {
    kFragment,
    kVertex,
    kGeometry,
    kFragmentProcessor,
    kRuntimeEffect,
    kGeneric,
};

/**
 * Holds the compiler settings for a program.
 */
struct ProgramSettings {
    // if false, sk_FragCoord is exactly the same as gl_FragCoord. If true, the y coordinate
    // must be flipped.
    bool fFlipY = false;
    // If true the destination fragment color is read sk_FragColor. It must be declared inout.
    bool fFragColorIsInOut = false;
    // if true, Setting objects (e.g. sk_Caps.fbFetchSupport) should be replaced with their
    // constant equivalents during compilation
    bool fReplaceSettings = true;
    // if true, all halfs are forced to be floats
    bool fForceHighPrecision = false;
    // if true, add -0.5 bias to LOD of all texture lookups
    bool fSharpenTextures = false;
    // if the program needs to create an RTHeight uniform, this is its offset in the uniform
    // buffer
    int fRTHeightOffset = -1;
    // if the program needs to create an RTHeight uniform and is creating spriv, this is the
    // binding and set number of the uniform buffer.
    int fRTHeightBinding = -1;
    int fRTHeightSet = -1;
    // If layout(set=S, binding=B) is not specified for a uniform, these values will be used.
    // At present, zero is always used by our backends.
    int fDefaultUniformSet = 0;
    int fDefaultUniformBinding = 0;
    // Enables the SkSL optimizer.
    bool fOptimize = true;
    // (Requires fOptimize = true) Removes any uncalled functions other than main(). Note that a
    // function which starts out being used may end up being uncalled after optimization.
    bool fRemoveDeadFunctions = true;
    // (Requires fOptimize = true) Removes global variables which are never used.
    bool fRemoveDeadVariables = true;
    // (Requires fOptimize = true) When greater than zero, enables the inliner. The threshold value
    // sets an upper limit on the acceptable amount of code growth from inlining.
    int fInlineThreshold = SkSL::kDefaultInlineThreshold;
    // If true, implicit conversions to lower precision numeric types are allowed
    // (eg, float to half)
    bool fAllowNarrowingConversions = false;
    // If true, then Debug code will run SPIR-V output through the validator to ensure its
    // correctness
    bool fValidateSPIRV = true;
    // If true, any synthetic uniforms must use push constant syntax
    bool fUsePushConstants = false;
    // Permits static if/switch statements to be used with non-constant tests. This is used when
    // producing H and CPP code; the static tests don't have to have constant values *yet*, but
    // the generated code will contain a static test which then does have to be a constant.
    bool fPermitInvalidStaticTests = false;
};

/**
 * All the configuration data for a given program.
 */
struct ProgramConfig {
    ProgramKind fKind;
    ProgramSettings fSettings;

    bool strictES2Mode() const {
        return fKind == ProgramKind::kRuntimeEffect || fKind == ProgramKind::kGeneric;
    }
};

}  // namespace SkSL

#endif
