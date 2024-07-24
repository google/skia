/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_PROGRAMSETTINGS
#define SKSL_PROGRAMSETTINGS

#include "include/sksl/SkSLVersion.h"
#include "src/sksl/SkSLDefines.h"
#include "src/sksl/SkSLModule.h"
#include "src/sksl/SkSLProgramKind.h"

#include <optional>
#include <vector>

namespace SkSL {

enum class ModuleType : int8_t;

/**
 * Holds the compiler settings for a program.
 */
struct ProgramSettings {
    // If true, the destination fragment color can be read from sk_FragColor. It must be declared
    // inout. This is only supported in GLSL, when framebuffer-fetch is used.
    bool fFragColorIsInOut = false;
    // if true, all halfs are forced to be floats
    bool fForceHighPrecision = false;
    // if true, add -0.5 bias to LOD of all texture lookups
    bool fSharpenTextures = false;
    // If true, sk_FragCoord, the dFdy gradient, and sk_Clockwise won't be modified by the
    // rtFlip. Additionally, the program interface's 'fRTFlipUniform' value will be left as None,
    // so no rtFlip uniform will be emitted.
    bool fForceNoRTFlip = false;
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
    // TODO(skia:11209) - Replace this with a "promised" capabilities?
    // Sets a maximum SkSL version. Compilation will fail if the program uses features that aren't
    // allowed at the requested version. For instance, a valid program must have fully-unrollable
    // `for` loops at version 100, but any loop structure is allowed at version 300.
    SkSL::Version fMaxVersionAllowed = SkSL::Version::k100;
    // If true, SkSL will use a memory pool for all IR nodes when compiling a program. This is
    // usually a significant speed increase, but uses more memory, so it is a good idea for programs
    // that will be freed shortly after compilation. It can also be useful to disable this flag when
    // investigating memory corruption. (This controls behavior of the SkSL compiler, not the code
    // we generate.)
    bool fUseMemoryPool = true;
};

/**
 * All the configuration data for a given program.
 */
struct ProgramConfig {
    /**
     * If we are compiling one of the SkSL built-in modules, this field indicates which one.
     * Contains `ModuleType::program` when not compiling a module at all.
     */
    ModuleType fModuleType;
    ProgramKind fKind;
    ProgramSettings fSettings;

    bool isBuiltinCode() {
        return fModuleType != ModuleType::program;
    }

    // When enforcesSkSLVersion() is true, this determines the available feature set that will be
    // enforced. This is set automatically when the `#version` directive is parsed.
    SkSL::Version fRequiredSkSLVersion = SkSL::Version::k100;

    bool enforcesSkSLVersion() const {
        return IsRuntimeEffect(fKind);
    }

    bool strictES2Mode() const {
        // TODO(skia:11209): Remove the first condition - so this is just based on #version.
        //                   Make it more generic (eg, isVersionLT) checking.
        return fSettings.fMaxVersionAllowed == Version::k100 &&
               fRequiredSkSLVersion == Version::k100 &&
               this->enforcesSkSLVersion();
    }

    const char* versionDescription() const {
        if (this->enforcesSkSLVersion()) {
            switch (fRequiredSkSLVersion) {
                case Version::k100: return "#version 100\n";
                case Version::k300: return "#version 300\n";
            }
        }
        return "";
    }

    static bool IsFragment(ProgramKind kind) {
        return kind == ProgramKind::kFragment ||
               kind == ProgramKind::kGraphiteFragment ||
               kind == ProgramKind::kGraphiteFragmentES2;
    }

    static bool IsVertex(ProgramKind kind) {
        return kind == ProgramKind::kVertex ||
               kind == ProgramKind::kGraphiteVertex ||
               kind == ProgramKind::kGraphiteVertexES2;
    }

    static bool IsCompute(ProgramKind kind) {
        return kind == ProgramKind::kCompute;
    }

    static bool IsRuntimeEffect(ProgramKind kind) {
        return (kind == ProgramKind::kRuntimeColorFilter ||
                kind == ProgramKind::kRuntimeShader ||
                kind == ProgramKind::kRuntimeBlender ||
                kind == ProgramKind::kPrivateRuntimeColorFilter ||
                kind == ProgramKind::kPrivateRuntimeShader ||
                kind == ProgramKind::kPrivateRuntimeBlender ||
                kind == ProgramKind::kMeshVertex ||
                kind == ProgramKind::kMeshFragment);
    }

    static bool IsRuntimeShader(ProgramKind kind) {
        return (kind == ProgramKind::kRuntimeShader ||
                kind == ProgramKind::kPrivateRuntimeShader);
    }

    static bool IsRuntimeColorFilter(ProgramKind kind) {
        return (kind == ProgramKind::kRuntimeColorFilter ||
                kind == ProgramKind::kPrivateRuntimeColorFilter);
    }

    static bool IsRuntimeBlender(ProgramKind kind) {
        return (kind == ProgramKind::kRuntimeBlender ||
                kind == ProgramKind::kPrivateRuntimeBlender);
    }

    static bool IsMesh(ProgramKind kind) {
        return (kind == ProgramKind::kMeshVertex ||
                kind == ProgramKind::kMeshFragment);
    }

    static bool AllowsPrivateIdentifiers(ProgramKind kind) {
        return (kind != ProgramKind::kRuntimeColorFilter &&
                kind != ProgramKind::kRuntimeShader &&
                kind != ProgramKind::kRuntimeBlender &&
                kind != ProgramKind::kMeshVertex &&
                kind != ProgramKind::kMeshFragment);
    }
};

}  // namespace SkSL

#endif
