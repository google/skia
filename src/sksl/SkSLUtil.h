/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_UTIL
#define SKSL_UTIL

#include <cstdarg>
#include <memory>
#include "stdlib.h"
#include "string.h"
#include "src/sksl/SkSLDefines.h"
#include "src/sksl/SkSLLexer.h"

#ifndef SKSL_STANDALONE
#include "include/core/SkTypes.h"
#if SK_SUPPORT_GPU
#include "include/core/SkRefCnt.h"
#include "src/gpu/GrShaderCaps.h"
#endif // SK_SUPPORT_GPU
#endif // SKSL_STANDALONE

namespace SkSL {

class OutputStream;
class StringStream;

#if defined(SKSL_STANDALONE) || !SK_SUPPORT_GPU

// we're being compiled standalone, so we don't have access to caps...
enum GrGLSLGeneration {
    k110_GrGLSLGeneration,
    k130_GrGLSLGeneration,
    k140_GrGLSLGeneration,
    k150_GrGLSLGeneration,
    k330_GrGLSLGeneration,
    k400_GrGLSLGeneration,
    k420_GrGLSLGeneration,
    k310es_GrGLSLGeneration,
    k320es_GrGLSLGeneration,
};

#define SKSL_CAPS_CLASS StandaloneShaderCaps
class StandaloneShaderCaps {
public:
    GrGLSLGeneration generation() const {
        return k400_GrGLSLGeneration;
    }

    bool atan2ImplementedAsAtanYOverX() const {
        return false;
    }

    bool canUseMinAndAbsTogether() const {
        return true;
    }

    bool mustForceNegatedAtanParamToFloat() const {
        return false;
    }

    bool shaderDerivativeSupport() const {
        return true;
    }

    bool usesPrecisionModifiers() const {
        return true;
    }

    bool mustDeclareFragmentShaderOutput() const {
        return true;
    }

    bool fbFetchSupport() const {
        return true;
    }

    bool fbFetchNeedsCustomOutput() const {
        return false;
    }

    bool flatInterpolationSupport() const {
        return true;
    }

    bool noperspectiveInterpolationSupport() const {
        return true;
    }

    bool multisampleInterpolationSupport() const {
        return true;
    }

    bool sampleMaskSupport() const {
        return true;
    }

    bool externalTextureSupport() const {
        return true;
    }

    bool mustDoOpBetweenFloorAndAbs() const {
        return false;
    }

    bool mustGuardDivisionEvenAfterExplicitZeroCheck() const {
        return false;
    }

    bool inBlendModesFailRandomlyForAllZeroVec() const {
        return false;
    }

    bool mustEnableAdvBlendEqs() const {
        return false;
    }

    bool mustEnableSpecificAdvBlendEqs() const {
        return false;
    }

    bool canUseAnyFunctionInShader() const {
        return false;
    }

    bool noDefaultPrecisionForExternalSamplers() const {
        return false;
    }

    bool floatIs32Bits() const {
        return true;
    }

    bool integerSupport() const {
        return false;
    }

    bool builtinFMASupport() const {
        return true;
    }

    const char* shaderDerivativeExtensionString() const {
        return nullptr;
    }

    const char* fragCoordConventionsExtensionString() const {
        return nullptr;
    }

    const char* geometryShaderExtensionString() const {
        return nullptr;
    }

    const char* gsInvocationsExtensionString() const {
        return nullptr;
    }

    const char* externalTextureExtensionString() const {
        return nullptr;
    }

    const char* secondExternalTextureExtensionString() const {
        return nullptr;
    }

    const char* versionDeclString() const {
        return "";
    }

    bool gsInvocationsSupport() const {
        return true;
    }

    bool canUseFractForNegativeValues() const {
        return true;
    }

    bool canUseFragCoord() const {
        return true;
    }

    bool incompleteShortIntPrecision() const {
        return false;
    }

    bool addAndTrueToLoopCondition() const {
        return false;
    }

    bool unfoldShortCircuitAsTernary() const {
        return false;
    }

    bool emulateAbsIntFunction() const {
        return false;
    }

    bool rewriteDoWhileLoops() const {
        return false;
    }

    bool removePowWithConstantExponent() const {
        return false;
    }

    const char* fbFetchColorName() const {
        return nullptr;
    }
};

extern StandaloneShaderCaps standaloneCaps;

#else

#define SKSL_CAPS_CLASS GrShaderCaps
// Various sets of caps for use in tests
class ShaderCapsFactory {
public:
    static sk_sp<GrShaderCaps> Default();

    static sk_sp<GrShaderCaps> Version450Core();

    static sk_sp<GrShaderCaps> Version110();

    static sk_sp<GrShaderCaps> UsesPrecisionModifiers();

    static sk_sp<GrShaderCaps> CannotUseMinAndAbsTogether();

    static sk_sp<GrShaderCaps> CannotUseFractForNegativeValues();

    static sk_sp<GrShaderCaps> MustForceNegatedAtanParamToFloat();

    static sk_sp<GrShaderCaps> ShaderDerivativeExtensionString();

    static sk_sp<GrShaderCaps> FragCoordsOld();

    static sk_sp<GrShaderCaps> FragCoordsNew();

    static sk_sp<GrShaderCaps> GeometryShaderSupport();

    static sk_sp<GrShaderCaps> NoGSInvocationsSupport();

    static sk_sp<GrShaderCaps> GeometryShaderExtensionString();

    static sk_sp<GrShaderCaps> GSInvocationsExtensionString();

    static sk_sp<GrShaderCaps> VariousCaps();

    static sk_sp<GrShaderCaps> CannotUseFragCoord();

    static sk_sp<GrShaderCaps> IncompleteShortIntPrecision();

    static sk_sp<GrShaderCaps> AddAndTrueToLoopCondition();

    static sk_sp<GrShaderCaps> UnfoldShortCircuitAsTernary();

    static sk_sp<GrShaderCaps> EmulateAbsIntFunction();

    static sk_sp<GrShaderCaps> RewriteDoWhileLoops();

    static sk_sp<GrShaderCaps> RemovePowWithConstantExponent();

    static sk_sp<GrShaderCaps> SampleMaskSupport();
};
#endif

void write_stringstream(const StringStream& d, OutputStream& out);

// Returns true if op is '=' or any compound assignment operator ('+=', '-=', etc.)
bool is_assignment(Token::Kind op);

// Given a compound assignment operator, returns the non-assignment version of the operator (e.g.
// '+=' becomes '+')
Token::Kind remove_assignment(Token::Kind op);

NORETURN void sksl_abort();

} // namespace

#endif
