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
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/gl/GrGLTypes.h"
#include "include/private/GrGLTypesPriv.h"
#include "src/gpu/GrDirectContextPriv.h"
#include "src/gpu/GrTexture.h"
#include "src/gpu/GrTextureProxy.h"
#include "src/gpu/gl/GrGLCaps.h"
#include "src/gpu/gl/GrGLTexture.h"
#include "src/image/SkImage_Base.h"
#include "tools/gpu/ProxyUtils.h"

static bool sampler_params_invalid(const GrGLTextureParameters& parameters) {
    return SkScalarIsNaN(parameters.samplerOverriddenState().fMaxLOD);
}

static bool nonsampler_params_invalid(const GrGLTextureParameters& parameters) {
    GrGLTextureParameters::NonsamplerState nsState = parameters.nonsamplerState();
    GrGLTextureParameters::NonsamplerState invalidNSState;
    invalidNSState.invalidate();
    return nsState.fBaseMipMapLevel == invalidNSState.fBaseMipMapLevel &&
           nsState.fMaxMipmapLevel  == invalidNSState.fMaxMipmapLevel  &&
           nsState.fSwizzleIsRGBA   == invalidNSState.fSwizzleIsRGBA;
}

static bool params_invalid(const GrGLTextureParameters& parameters) {
    return sampler_params_invalid(parameters) && nonsampler_params_invalid(parameters);
}

static bool params_valid(const GrGLTextureParameters& parameters, const GrGLCaps* caps) {
    if (nonsampler_params_invalid(parameters)) {
        return false;
    }
    // We should only set the texture params that are equivalent to sampler param to valid if we're
    // not using sampler objects.
    return caps->useSamplerObjects() == sampler_params_invalid(parameters);
}

DEF_GPUTEST_FOR_ALL_GL_CONTEXTS(GLTextureParameters, reporter, ctxInfo) {
    auto dContext = ctxInfo.directContext();
    auto caps = static_cast<const GrGLCaps*>(dContext->priv().caps());

    GrBackendTexture backendTex = dContext->createBackendTexture(
            1, 1, kRGBA_8888_SkColorType, GrMipmapped::kNo, GrRenderable::kNo, GrProtected::kNo);
    REPORTER_ASSERT(reporter, backendTex.isValid());

    GrGLTextureInfo info;
    REPORTER_ASSERT(reporter, backendTex.getGLTextureInfo(&info));

    GrBackendTexture backendTexCopy = backendTex;
    REPORTER_ASSERT(reporter, backendTexCopy.isSameTexture(backendTex));

    sk_sp<SkImage> wrappedImage =
            SkImage::MakeFromTexture(dContext, backendTex, kTopLeft_GrSurfaceOrigin,
                                     kRGBA_8888_SkColorType, kPremul_SkAlphaType, nullptr);
    REPORTER_ASSERT(reporter, wrappedImage);

    GrSurfaceProxy* proxy = sk_gpu_test::GetTextureImageProxy(wrappedImage.get(), dContext);
    REPORTER_ASSERT(reporter, proxy);
    REPORTER_ASSERT(reporter, proxy->isInstantiated());
    auto texture = static_cast<GrGLTexture*>(proxy->peekTexture());
    REPORTER_ASSERT(reporter, texture);
    auto parameters = texture->parameters();
    REPORTER_ASSERT(reporter, parameters);
    GrGLTextureParameters::SamplerOverriddenState invalidSState;
    invalidSState.invalidate();
    GrGLTextureParameters::NonsamplerState invalidNSState;
    invalidNSState.invalidate();

    auto surf = SkSurface::MakeRenderTarget(
            dContext, SkBudgeted::kYes,
            SkImageInfo::Make(1, 1, kRGBA_8888_SkColorType, kPremul_SkAlphaType), 1, nullptr);
    REPORTER_ASSERT(reporter, surf);

    // Test invalidating from the GL backend texture.
    backendTex.glTextureParametersModified();
    REPORTER_ASSERT(reporter, params_invalid(*parameters));

    REPORTER_ASSERT(reporter, surf);
    surf->getCanvas()->drawImage(wrappedImage, 0, 0);
    surf->flushAndSubmit();
    REPORTER_ASSERT(reporter, params_valid(*parameters, caps));

    // Test invalidating from the copy.
    backendTexCopy.glTextureParametersModified();
    REPORTER_ASSERT(reporter, params_invalid(*parameters));

    // Check that we can do things like assigning the backend texture to invalid one, assign an
    // invalid one, assign a backend texture to itself etc. Success here is that we don't hit any
    // of our ref counting asserts.
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
    dContext->flush();
    dContext->submit(true);
    dContext->deleteBackendTexture(backendTex);
}
#endif
