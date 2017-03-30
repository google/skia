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
#include "assert.h"
#include "SkSLString.h"
#include "SkSLStringStream.h"

#ifndef SKSL_STANDALONE
#include "GrContextOptions.h"
#include "GrShaderCaps.h"
#endif

#ifdef SKSL_STANDALONE
#if defined(_WIN32) || defined(__SYMBIAN32__)
#define SKSL_BUILD_FOR_WIN
#endif
#else
#ifdef SK_BUILD_FOR_WIN
#define SKSL_BUILD_FOR_WIN
#endif // SK_BUILD_FOR_WIN
#endif // SKSL_STANDALONE

namespace SkSL {

#ifdef SKSL_STANDALONE

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

    bool bindlessTextureSupport() const {
        return false;
    }

    bool dropsTileOnZeroDivide() const {
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

    bool sampleVariablesSupport() const {
        return true;
    }

    bool sampleMaskOverrideCoverageSupport() const {
        return true;
    }

    bool externalTextureSupport() const {
        return true;
    }

    bool texelFetchSupport() const {
        return true;
    }

    bool imageLoadStoreSupport() const {
        return true;
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

    const char* shaderDerivativeExtensionString() const {
        return nullptr;
    }

    const char* fragCoordConventionsExtensionString() const {
        return nullptr;
    }

    const char* imageLoadStoreExtensionString() const {
        return nullptr;
    }

    const char* versionDeclString() const {
        return "";
    }
};

extern StandaloneShaderCaps standaloneCaps;

#else

#define SKSL_CAPS_CLASS GrShaderCaps
// Various sets of caps for use in tests
class ShaderCapsFactory {
public:
    static sk_sp<GrShaderCaps> Default() {
        sk_sp<GrShaderCaps> result = sk_make_sp<GrShaderCaps>(GrContextOptions());
        result->fVersionDeclString = "#version 400";
        result->fShaderDerivativeSupport = true;
        return result;
    }

    static sk_sp<GrShaderCaps> Version450Core() {
        sk_sp<GrShaderCaps> result = sk_make_sp<GrShaderCaps>(GrContextOptions());
        result->fVersionDeclString = "#version 450 core";
        return result;
    }

    static sk_sp<GrShaderCaps> Version110() {
        sk_sp<GrShaderCaps> result = sk_make_sp<GrShaderCaps>(GrContextOptions());
        result->fVersionDeclString = "#version 110";
        result->fGLSLGeneration = GrGLSLGeneration::k110_GrGLSLGeneration;
        return result;
    }

    static sk_sp<GrShaderCaps> UsesPrecisionModifiers() {
        sk_sp<GrShaderCaps> result = sk_make_sp<GrShaderCaps>(GrContextOptions());
        result->fVersionDeclString = "#version 400";
        result->fUsesPrecisionModifiers = true;
        return result;
    }

    static sk_sp<GrShaderCaps> CannotUseMinAndAbsTogether() {
        sk_sp<GrShaderCaps> result = sk_make_sp<GrShaderCaps>(GrContextOptions());
        result->fVersionDeclString = "#version 400";
        result->fCanUseMinAndAbsTogether = false;
        return result;
    }

    static sk_sp<GrShaderCaps> MustForceNegatedAtanParamToFloat() {
        sk_sp<GrShaderCaps> result = sk_make_sp<GrShaderCaps>(GrContextOptions());
        result->fVersionDeclString = "#version 400";
        result->fMustForceNegatedAtanParamToFloat = true;
        return result;
    }

    static sk_sp<GrShaderCaps> ShaderDerivativeExtensionString() {
        sk_sp<GrShaderCaps> result = sk_make_sp<GrShaderCaps>(GrContextOptions());
        result->fVersionDeclString = "#version 400";
        result->fShaderDerivativeSupport = true;
        result->fShaderDerivativeExtensionString = "GL_OES_standard_derivatives";
        return result;
    }

    static sk_sp<GrShaderCaps> FragCoordsOld() {
        sk_sp<GrShaderCaps> result = sk_make_sp<GrShaderCaps>(GrContextOptions());
        result->fVersionDeclString = "#version 110";
        result->fGLSLGeneration = GrGLSLGeneration::k110_GrGLSLGeneration;
        result->fFragCoordConventionsExtensionString = "GL_ARB_fragment_coord_conventions";
        return result;
    }

    static sk_sp<GrShaderCaps> FragCoordsNew() {
        sk_sp<GrShaderCaps> result = sk_make_sp<GrShaderCaps>(GrContextOptions());
        result->fVersionDeclString = "#version 400";
        result->fFragCoordConventionsExtensionString = "GL_ARB_fragment_coord_conventions";
        return result;
    }

    static sk_sp<GrShaderCaps> VariousCaps() {
        sk_sp<GrShaderCaps> result = sk_make_sp<GrShaderCaps>(GrContextOptions());
        result->fVersionDeclString = "#version 400";
        result->fExternalTextureSupport = true;
        result->fFBFetchSupport = false;
        result->fDropsTileOnZeroDivide = true;
        result->fTexelFetchSupport = true;
        result->fCanUseAnyFunctionInShader = false;
        return result;
    }
};
#endif

void write_stringstream(const StringStream& d, OutputStream& out);

#if _MSC_VER
#define NORETURN __declspec(noreturn)
#else
#define NORETURN __attribute__((__noreturn__))
#endif

NORETURN void sksl_abort();

} // namespace

#ifdef SKSL_STANDALONE
#define ASSERT(x) (void)((x) || (ABORT("failed assert(%s): %s:%d\n", #x, __FILE__, __LINE__), 0))
#define ASSERT_RESULT(x) ASSERT(x)
#define SKSL_DEBUGCODE(x) x
#else
#define ASSERT SkASSERT
#define ASSERT_RESULT(x) SkAssertResult(x)
#define SKSL_DEBUGCODE(x) SkDEBUGCODE(x)
#endif

#define SKSL_WARN_UNUSED_RESULT __attribute__((warn_unused_result))

#if defined(__clang__) || defined(__GNUC__)
#define SKSL_PRINTF_LIKE(A, B) __attribute__((format(printf, (A), (B))))
#else
#define SKSL_PRINTF_LIKE(A, B)
#endif

#define ABORT(...) (printf(__VA_ARGS__), sksl_abort())

#endif
