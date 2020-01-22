/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLUtil.h"

#include "src/sksl/SkSLStringStream.h"

#if !defined(SKSL_STANDALONE) & SK_SUPPORT_GPU
#include "include/gpu/GrContextOptions.h"
#endif

#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif

namespace SkSL {

#if defined(SKSL_STANDALONE) || !SK_SUPPORT_GPU
StandaloneShaderCaps standaloneCaps;
#endif

void sksl_abort() {
#ifdef SKSL_STANDALONE
    abort();
#else
    sk_abort_no_print();
    exit(1);
#endif
}

void write_stringstream(const StringStream& s, OutputStream& out) {
    out.write(s.str().c_str(), s.str().size());
}

bool is_assignment(Token::Kind op) {
    switch (op) {
        case Token::EQ:           // fall through
        case Token::PLUSEQ:       // fall through
        case Token::MINUSEQ:      // fall through
        case Token::STAREQ:       // fall through
        case Token::SLASHEQ:      // fall through
        case Token::PERCENTEQ:    // fall through
        case Token::SHLEQ:        // fall through
        case Token::SHREQ:        // fall through
        case Token::BITWISEOREQ:  // fall through
        case Token::BITWISEXOREQ: // fall through
        case Token::BITWISEANDEQ: // fall through
        case Token::LOGICALOREQ:  // fall through
        case Token::LOGICALXOREQ: // fall through
        case Token::LOGICALANDEQ:
            return true;
        default:
            return false;
    }
}

Token::Kind remove_assignment(Token::Kind op) {
    switch (op) {
        case Token::PLUSEQ:       return Token::PLUS;
        case Token::MINUSEQ:      return Token::MINUS;
        case Token::STAREQ:       return Token::STAR;
        case Token::SLASHEQ:      return Token::SLASH;
        case Token::PERCENTEQ:    return Token::PERCENT;
        case Token::SHLEQ:        return Token::SHL;
        case Token::SHREQ:        return Token::SHR;
        case Token::BITWISEOREQ:  return Token::BITWISEOR;
        case Token::BITWISEXOREQ: return Token::BITWISEXOR;
        case Token::BITWISEANDEQ: return Token::BITWISEAND;
        case Token::LOGICALOREQ:  return Token::LOGICALOR;
        case Token::LOGICALXOREQ: return Token::LOGICALXOR;
        case Token::LOGICALANDEQ: return Token::LOGICALAND;
        default: return op;
    }
}

#if !defined(SKSL_STANDALONE) & SK_SUPPORT_GPU
sk_sp<GrShaderCaps> ShaderCapsFactory::Default() {
    sk_sp<GrShaderCaps> result = sk_make_sp<GrShaderCaps>(GrContextOptions());
    result->fVersionDeclString = "#version 400";
    result->fShaderDerivativeSupport = true;
    return result;
}

sk_sp<GrShaderCaps> ShaderCapsFactory::Version450Core() {
    sk_sp<GrShaderCaps> result = sk_make_sp<GrShaderCaps>(GrContextOptions());
    result->fVersionDeclString = "#version 450 core";
    return result;
}

sk_sp<GrShaderCaps> ShaderCapsFactory::Version110() {
    sk_sp<GrShaderCaps> result = sk_make_sp<GrShaderCaps>(GrContextOptions());
    result->fVersionDeclString = "#version 110";
    result->fGLSLGeneration = GrGLSLGeneration::k110_GrGLSLGeneration;
    return result;
}

sk_sp<GrShaderCaps> ShaderCapsFactory::UsesPrecisionModifiers() {
    sk_sp<GrShaderCaps> result = sk_make_sp<GrShaderCaps>(GrContextOptions());
    result->fVersionDeclString = "#version 400";
    result->fUsesPrecisionModifiers = true;
    return result;
}

sk_sp<GrShaderCaps> ShaderCapsFactory::CannotUseMinAndAbsTogether() {
    sk_sp<GrShaderCaps> result = sk_make_sp<GrShaderCaps>(GrContextOptions());
    result->fVersionDeclString = "#version 400";
    result->fCanUseMinAndAbsTogether = false;
    return result;
}

sk_sp<GrShaderCaps> ShaderCapsFactory::CannotUseFractForNegativeValues() {
    sk_sp<GrShaderCaps> result = sk_make_sp<GrShaderCaps>(GrContextOptions());
    result->fVersionDeclString = "#version 400";
    result->fCanUseFractForNegativeValues = false;
    return result;
}

sk_sp<GrShaderCaps> ShaderCapsFactory::MustForceNegatedAtanParamToFloat() {
    sk_sp<GrShaderCaps> result = sk_make_sp<GrShaderCaps>(GrContextOptions());
    result->fVersionDeclString = "#version 400";
    result->fMustForceNegatedAtanParamToFloat = true;
    return result;
}

sk_sp<GrShaderCaps> ShaderCapsFactory::ShaderDerivativeExtensionString() {
    sk_sp<GrShaderCaps> result = sk_make_sp<GrShaderCaps>(GrContextOptions());
    result->fVersionDeclString = "#version 400";
    result->fShaderDerivativeSupport = true;
    result->fShaderDerivativeExtensionString = "GL_OES_standard_derivatives";
    result->fUsesPrecisionModifiers = true;
    return result;
}

sk_sp<GrShaderCaps> ShaderCapsFactory::FragCoordsOld() {
    sk_sp<GrShaderCaps> result = sk_make_sp<GrShaderCaps>(GrContextOptions());
    result->fVersionDeclString = "#version 110";
    result->fGLSLGeneration = GrGLSLGeneration::k110_GrGLSLGeneration;
    result->fFragCoordConventionsExtensionString = "GL_ARB_fragment_coord_conventions";
    return result;
}

sk_sp<GrShaderCaps> ShaderCapsFactory::FragCoordsNew() {
    sk_sp<GrShaderCaps> result = sk_make_sp<GrShaderCaps>(GrContextOptions());
    result->fVersionDeclString = "#version 400";
    result->fFragCoordConventionsExtensionString = "GL_ARB_fragment_coord_conventions";
    return result;
}

sk_sp<GrShaderCaps> ShaderCapsFactory::GeometryShaderSupport() {
    sk_sp<GrShaderCaps> result = sk_make_sp<GrShaderCaps>(GrContextOptions());
    result->fVersionDeclString = "#version 400";
    result->fGeometryShaderSupport = true;
    result->fGSInvocationsSupport = true;
    return result;
}

sk_sp<GrShaderCaps> ShaderCapsFactory::NoGSInvocationsSupport() {
    sk_sp<GrShaderCaps> result = sk_make_sp<GrShaderCaps>(GrContextOptions());
    result->fVersionDeclString = "#version 400";
    result->fGeometryShaderSupport = true;
    result->fGSInvocationsSupport = false;
    return result;
}

sk_sp<GrShaderCaps> ShaderCapsFactory::GeometryShaderExtensionString() {
    sk_sp<GrShaderCaps> result = sk_make_sp<GrShaderCaps>(GrContextOptions());
    result->fVersionDeclString = "#version 310es";
    result->fGeometryShaderSupport = true;
    result->fGeometryShaderExtensionString = "GL_EXT_geometry_shader";
    result->fGSInvocationsSupport = true;
    return result;
}

sk_sp<GrShaderCaps> ShaderCapsFactory::GSInvocationsExtensionString() {
    sk_sp<GrShaderCaps> result = sk_make_sp<GrShaderCaps>(GrContextOptions());
    result->fVersionDeclString = "#version 400";
    result->fGeometryShaderSupport = true;
    result->fGSInvocationsSupport = true;
    result->fGSInvocationsExtensionString = "GL_ARB_gpu_shader5";
    return result;
}

sk_sp<GrShaderCaps> ShaderCapsFactory::VariousCaps() {
    sk_sp<GrShaderCaps> result = sk_make_sp<GrShaderCaps>(GrContextOptions());
    result->fVersionDeclString = "#version 400";
    result->fExternalTextureSupport = true;
    result->fFBFetchSupport = false;
    result->fCanUseAnyFunctionInShader = false;
    return result;
}

sk_sp<GrShaderCaps> ShaderCapsFactory::CannotUseFragCoord() {
    sk_sp<GrShaderCaps> result = sk_make_sp<GrShaderCaps>(GrContextOptions());
    result->fVersionDeclString = "#version 400";
    result->fCanUseFragCoord = false;
    return result;
}

sk_sp<GrShaderCaps> ShaderCapsFactory::IncompleteShortIntPrecision() {
    sk_sp<GrShaderCaps> result = sk_make_sp<GrShaderCaps>(GrContextOptions());
    result->fVersionDeclString = "#version 310es";
    result->fUsesPrecisionModifiers = true;
    result->fIncompleteShortIntPrecision = true;
    return result;
}

sk_sp<GrShaderCaps> ShaderCapsFactory::AddAndTrueToLoopCondition() {
    sk_sp<GrShaderCaps> result = sk_make_sp<GrShaderCaps>(GrContextOptions());
    result->fVersionDeclString = "#version 400";
    result->fAddAndTrueToLoopCondition = true;
    return result;
}

sk_sp<GrShaderCaps> ShaderCapsFactory::UnfoldShortCircuitAsTernary() {
    sk_sp<GrShaderCaps> result = sk_make_sp<GrShaderCaps>(GrContextOptions());
    result->fVersionDeclString = "#version 400";
    result->fUnfoldShortCircuitAsTernary = true;
    return result;
}

sk_sp<GrShaderCaps> ShaderCapsFactory::EmulateAbsIntFunction() {
    sk_sp<GrShaderCaps> result = sk_make_sp<GrShaderCaps>(GrContextOptions());
    result->fVersionDeclString = "#version 400";
    result->fEmulateAbsIntFunction = true;
    return result;
}

sk_sp<GrShaderCaps> ShaderCapsFactory::RewriteDoWhileLoops() {
    sk_sp<GrShaderCaps> result = sk_make_sp<GrShaderCaps>(GrContextOptions());
    result->fVersionDeclString = "#version 400";
    result->fRewriteDoWhileLoops = true;
    return result;
}

sk_sp<GrShaderCaps> ShaderCapsFactory::RemovePowWithConstantExponent() {
    sk_sp<GrShaderCaps> result = sk_make_sp<GrShaderCaps>(GrContextOptions());
    result->fVersionDeclString = "#version 400";
    result->fRemovePowWithConstantExponent = true;
    return result;
}

sk_sp<GrShaderCaps> ShaderCapsFactory::SampleMaskSupport() {
    sk_sp<GrShaderCaps> result = Default();
    result->fSampleMaskSupport = true;
    return result;
}
#endif

} // namespace
