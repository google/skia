/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTypes.h"

#ifdef SK_GL

#include "tests/Test.h"

#include "include/core/SkImage.h"
#include "include/core/SkSurface.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrTexture.h"
#include "include/gpu/gl/GrGLTypes.h"
#include "include/private/GrGLTypesPriv.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrTextureProxy.h"
#include "src/gpu/gl/GrGLCaps.h"
#include "src/gpu/gl/GrGLTexture.h"
#include "src/image/SkImage_Base.h"

static bool sampler_params_invalid(const GrGLTextureParameters& parameters) {
    return SkScalarIsNaN(parameters.samplerOverriddenState().fMaxLOD);
}

static bool nonsampler_params_invalid(const GrGLTextureParameters& parameters) {
    GrGLTextureParameters::NonsamplerState invalidNSState;
    invalidNSState.invalidate();
    return 0 == memcmp(&parameters.nonsamplerState(), &invalidNSState, sizeof(invalidNSState));
}

static bool params_invalid(const GrGLTextureParameters& parameters) {
    return sampler_params_invalid(parameters) && nonsampler_params_invalid(parameters);
}

static bool params_valid(const GrGLTextureParameters& parameters, const GrGLCaps* caps) {
    if (nonsampler_params_invalid(parameters)) {
        return false;
    }
    // We should only set the sampler parameters to valid if we don't have sampler object support.
    return caps->samplerObjectSupport() == sampler_params_invalid(parameters);
}

DEF_GPUTEST_FOR_ALL_GL_CONTEXTS(GLTextureParameters, reporter, ctxInfo) {
    GrContext* context = ctxInfo.grContext();

    GrBackendTexture backendTex = context->createBackendTexture(
            1, 1, kRGBA_8888_SkColorType, GrMipMapped::kNo, GrRenderable::kNo);
    REPORTER_ASSERT(reporter, backendTex.isValid());

    GrGLTextureInfo info;
    REPORTER_ASSERT(reporter, backendTex.getGLTextureInfo(&info));

    GrBackendTexture backendTexCopy = backendTex;
    REPORTER_ASSERT(reporter, backendTexCopy.isSameTexture(backendTex));

    sk_sp<SkImage> wrappedImage =
            SkImage::MakeFromTexture(context, backendTex, kTopLeft_GrSurfaceOrigin,
                                     kRGBA_8888_SkColorType, kPremul_SkAlphaType, nullptr);
    REPORTER_ASSERT(reporter, wrappedImage);

    sk_sp<GrTextureProxy> texProxy = as_IB(wrappedImage)->asTextureProxyRef(context);
    REPORTER_ASSERT(reporter, texProxy.get());
    REPORTER_ASSERT(reporter, texProxy->isInstantiated());
    auto texture = static_cast<GrGLTexture*>(texProxy->peekTexture());
    REPORTER_ASSERT(reporter, texture);
    auto parameters = texture->parameters();
    REPORTER_ASSERT(reporter, parameters);
    GrGLTextureParameters::SamplerOverriddenState invalidSState;
    invalidSState.invalidate();
    GrGLTextureParameters::NonsamplerState invalidNSState;
    invalidNSState.invalidate();

    // After wrapping we should assume the client's texture can be in any state.
    REPORTER_ASSERT(reporter, params_invalid(*parameters));

    auto surf = SkSurface::MakeRenderTarget(
            context, SkBudgeted::kYes,
            SkImageInfo::Make(1, 1, kRGBA_8888_SkColorType, kPremul_SkAlphaType), 1, nullptr);
    REPORTER_ASSERT(reporter, surf);
    surf->getCanvas()->drawImage(wrappedImage, 0, 0);
    surf->flush();

    auto caps = static_cast<const GrGLCaps*>(context->priv().caps());
    // Now the texture should be in a known state.
    REPORTER_ASSERT(reporter, params_valid(*parameters, caps));

    // Test invalidating from the GL backend texture.
    backendTex.glTextureParametersModified();
    REPORTER_ASSERT(reporter, params_invalid(*parameters));

    REPORTER_ASSERT(reporter, surf);
    surf->getCanvas()->drawImage(wrappedImage, 0, 0);
    surf->flush();
    REPORTER_ASSERT(reporter, params_valid(*parameters, caps));

    // Test invalidating from the copy.
    backendTexCopy.glTextureParametersModified();
    REPORTER_ASSERT(reporter, params_invalid(*parameters));

    // Check that we can do things like assigning the backend texture to invalid one, assign an
    // invalid one, assin a backend texture to inself etc. Success here is that we don't hit any of
    // our ref counting asserts.
    REPORTER_ASSERT(reporter, GrBackendTexture::TestingOnly_Equals(backendTex, backendTexCopy));

    GrBackendTexture invalidTexture;
    REPORTER_ASSERT(reporter, !invalidTexture.isValid());
    REPORTER_ASSERT(reporter,
                    !GrBackendTexture::TestingOnly_Equals(invalidTexture, backendTexCopy));

    backendTexCopy = invalidTexture;
    REPORTER_ASSERT(reporter, !backendTexCopy.isValid());
    REPORTER_ASSERT(reporter,
                    !GrBackendTexture::TestingOnly_Equals(invalidTexture, backendTexCopy));

    invalidTexture = backendTex;
    REPORTER_ASSERT(reporter, invalidTexture.isValid());
    REPORTER_ASSERT(reporter, GrBackendTexture::TestingOnly_Equals(invalidTexture, backendTex));

    invalidTexture = static_cast<decltype(invalidTexture)&>(invalidTexture);
    REPORTER_ASSERT(reporter, invalidTexture.isValid());
    REPORTER_ASSERT(reporter, GrBackendTexture::TestingOnly_Equals(invalidTexture, invalidTexture));

    wrappedImage.reset();
    GrFlushInfo flushInfo;
    flushInfo.fFlags = kSyncCpu_GrFlushFlag;
    context->flush(flushInfo);
    context->deleteBackendTexture(backendTex);
}
#endif
