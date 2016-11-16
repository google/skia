/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
 
#ifndef SKSL_UTIL
#define SKSL_UTIL

#include <iomanip>
#include <string>
#include <sstream>
#include "stdlib.h"
#include "assert.h"
#include "SkRefCnt.h"
#include "SkTypes.h"
#include "glsl/GrGLSLCaps.h"
#include "GrContextOptions.h"

namespace SkSL {

// Various sets of caps for use in tests
class GLSLCapsFactory {
public:
    static sk_sp<GrGLSLCaps> Default() {
        sk_sp<GrGLSLCaps> result = sk_make_sp<GrGLSLCaps>(GrContextOptions());
        result->fVersionDeclString = "#version 400";
        result->fShaderDerivativeSupport = true;
        return result;
    }

    static sk_sp<GrGLSLCaps> Version450Core() {
        sk_sp<GrGLSLCaps> result = sk_make_sp<GrGLSLCaps>(GrContextOptions());
        result->fVersionDeclString = "#version 450 core";
        return result;
    }

    static sk_sp<GrGLSLCaps> Version110() {
        sk_sp<GrGLSLCaps> result = sk_make_sp<GrGLSLCaps>(GrContextOptions());
        result->fVersionDeclString = "#version 110";
        result->fGLSLGeneration = GrGLSLGeneration::k110_GrGLSLGeneration;
        return result;
    }

    static sk_sp<GrGLSLCaps> UsesPrecisionModifiers() {
        sk_sp<GrGLSLCaps> result = sk_make_sp<GrGLSLCaps>(GrContextOptions());
        result->fVersionDeclString = "#version 400";
        result->fUsesPrecisionModifiers = true;
        return result;
    }

    static sk_sp<GrGLSLCaps> CannotUseMinAndAbsTogether() {
        sk_sp<GrGLSLCaps> result = sk_make_sp<GrGLSLCaps>(GrContextOptions());
        result->fVersionDeclString = "#version 400";
        result->fCanUseMinAndAbsTogether = false;
        return result;
    }

    static sk_sp<GrGLSLCaps> MustForceNegatedAtanParamToFloat() {
        sk_sp<GrGLSLCaps> result = sk_make_sp<GrGLSLCaps>(GrContextOptions());
        result->fVersionDeclString = "#version 400";
        result->fMustForceNegatedAtanParamToFloat = true;
        return result;
    }

    static sk_sp<GrGLSLCaps> ShaderDerivativeExtensionString() {
        sk_sp<GrGLSLCaps> result = sk_make_sp<GrGLSLCaps>(GrContextOptions());
        result->fVersionDeclString = "#version 400";
        result->fShaderDerivativeSupport = true;
        result->fShaderDerivativeExtensionString = "GL_OES_standard_derivatives";
        return result;
    }
};

// our own definitions of certain std:: functions, because they are not always present on Android

std::string to_string(double value);

std::string to_string(int32_t value);

std::string to_string(uint32_t value);

std::string to_string(int64_t value);

std::string to_string(uint64_t value);

#if _MSC_VER
#define NORETURN __declspec(noreturn)
#else
#define NORETURN __attribute__((__noreturn__))
#endif
int stoi(std::string s);

double stod(std::string s);

long stol(std::string s);

NORETURN void sksl_abort();

} // namespace

#ifdef DEBUG
#define ASSERT(x) assert(x)
#define ASSERT_RESULT(x) ASSERT(x);
#else
#define ASSERT(x)
#define ASSERT_RESULT(x) x
#endif

#ifdef SKIA
#define ABORT(...) { SkDebugf(__VA_ARGS__); sksl_abort(); }
#else
#define ABORT(...) { sksl_abort(); }
#endif

#endif
