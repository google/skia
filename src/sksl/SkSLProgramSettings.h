/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_PROGRAMSETTINGS
#define SKSL_PROGRAMSETTINGS

#include "include/private/SkSLDefines.h"
#include "include/private/SkSLProgramKind.h"

#include <vector>

namespace SkSL {

class ExternalFunction;

/**
 * Holds the compiler settings for a program.
 */
struct ProgramSettings {
    // If true the destination fragment color is read sk_FragColor. It must be declared inout.
    bool fFragColorIsInOut = false;
    // if true, Setting objects (e.g. sk_Caps.fbFetchSupport) should be replaced with their
    // constant equivalents during compilation
    bool fReplaceSettings = true;
    // if true, all halfs are forced to be floats
    bool fForceHighPrecision = false;
    // if true, add -0.5 bias to LOD of all texture lookups
    bool fSharpenTextures = false;
    // if the program needs to create an RTFlip uniform, this is its offset in the uniform buffer
    int fRTFlipOffset = -1;
    // if the program needs to create an RTFlip uniform and is creating SPIR-V, this is the binding
    // and set number of the uniform buffer.
    int fRTFlipBinding = -1;
    int fRTFlipSet = -1;
    // If layout(set=S, binding=B) is not specified for a uniform, these values will be used.
    // At present, zero is always used by our backends.
    int fDefaultUniformSet = 0;
    int fDefaultUniformBinding = 0;
    // Enables the SkSL optimizer. Note that we never disable optimizations which are needed to
    // fully evaluate constant-expressions, like constant folding or constant-intrinsic evaluation.
    bool fOptimize = true;
    // (Requires fOptimize = true) Removes any uncalled functions other than main(). Note that a
    // function which starts out being used may end up being uncalled after optimization.
    bool fRemoveDeadFunctions = true;
    // (Requires fOptimize = true) Removes variables which are never used.
    bool fRemoveDeadVariables = true;
    // (Requires fOptimize = true) When greater than zero, enables the inliner. The threshold value
    // sets an upper limit on the acceptable amount of code growth from inlining.
    int fInlineThreshold = SkSL::kDefaultInlineThreshold;
    // If true, every function in the generated program will be given the `noinline` modifier.
    bool fForceNoInline = false;
    // If true, implicit conversions to lower precision numeric types are allowed (e.g., float to
    // half). These are always allowed when compiling Runtime Effects.
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
    // If true, configurations which demand strict ES2 conformance (runtime effects, generic
    // programs, and SkVM rendering) will fail during compilation if ES2 restrictions are violated.
    bool fEnforceES2Restrictions = true;
    // If true, SkVM debug traces will contain the `trace_var` opcode. This opcode can cause the
    // generated code to contain a lot of extra computations, because we need to explicitly compute
    // every temporary value, even ones that would otherwise be optimized away entirely. The other
    // debug opcodes are much less invasive on the generated code.
    bool fAllowTraceVarInSkVMDebugTrace = true;
    // If true, the DSL should automatically mangle symbol names.
    bool fDSLMangling = true;
    // If true, the DSL should automatically mark variables declared upon creation.
    bool fDSLMarkVarsDeclared = false;
    // If true, the DSL should install a memory pool when possible.
    bool fDSLUseMemoryPool = true;
    // If true, DSL objects assert that they were used prior to destruction
    bool fAssertDSLObjectsReleased = true;
    // External functions available for use in runtime effects. These values are registered in the
    // symbol table of the Program, but ownership is *not* transferred. It is up to the caller to
    // keep them alive.
    const std::vector<std::unique_ptr<ExternalFunction>>* fExternalFunctions = nullptr;
};

/**
 * All the configuration data for a given program.
 */
struct ProgramConfig {
    /** True if we are currently processing one of the built-in SkSL include modules. */
    bool fIsBuiltinCode;
    ProgramKind fKind;
    ProgramSettings fSettings;

    bool strictES2Mode() const {
        return fSettings.fEnforceES2Restrictions &&
               (IsRuntimeEffect(fKind) || fKind == ProgramKind::kGeneric);
    }

    static bool IsRuntimeEffect(ProgramKind kind) {
        return (kind == ProgramKind::kRuntimeColorFilter ||
                kind == ProgramKind::kRuntimeShader ||
                kind == ProgramKind::kRuntimeBlender);
    }
};

}  // namespace SkSL

#endif
