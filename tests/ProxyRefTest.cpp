/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// This is a GPU-backend specific test.

#include "tests/Test.h"

#include "include/gpu/GrTexture.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrPendingIOResource.h"
#include "src/gpu/GrProxyProvider.h"
#include "src/gpu/GrRenderTargetProxy.h"
#include "src/gpu/GrResourceProvider.h"
#include "src/gpu/GrSurfaceProxy.h"
#include "src/gpu/GrTextureProxy.h"

static const int kWidthHeight = 128;

static void check_refs(skiatest::Reporter* reporter,
                       GrTextureProxy* proxy,
                       int32_t expectedProxyRefs,
                       int32_t expectedBackingRefs) {
    int32_t actualProxyRefs = proxy->priv().getProxyRefCnt();
    int32_t actualBackingRefs = proxy->testingOnly_getBackingRefCnt();

    SkASSERT(actualProxyRefs == expectedProxyRefs);
    SkASSERT(actualBackingRefs == expectedBackingRefs);

    REPORTER_ASSERT(reporter, actualProxyRefs == expectedProxyRefs);
    REPORTER_ASSERT(reporter, actualBackingRefs == expectedBackingRefs);
}

static sk_sp<GrTextureProxy> make_deferred(GrContext* context) {
    GrProxyProvider* proxyProvider = context->priv().proxyProvider();
    const GrCaps* caps = context->priv().caps();

    GrSurfaceDesc desc;
    desc.fWidth = kWidthHeight;
    desc.fHeight = kWidthHeight;
    desc.fConfig = kRGBA_8888_GrPixelConfig;

    const GrBackendFormat format = caps->getDefaultBackendFormat(GrColorType::kRGBA_8888,
                                                                 GrRenderable::kYes);
    return proxyProvider->createProxy(format, desc, GrRenderable::kYes, 1,
                                      kBottomLeft_GrSurfaceOrigin, SkBackingFit::kApprox,
                                      SkBudgeted::kYes, GrProtected::kNo);
}

static sk_sp<GrTextureProxy> make_wrapped(GrContext* context) {
    GrProxyProvider* proxyProvider = context->priv().proxyProvider();

    return proxyProvider->testingOnly_createInstantiatedProxy(
            {kWidthHeight, kWidthHeight}, GrColorType::kRGBA_8888, GrRenderable::kYes, 1,
            kBottomLeft_GrSurfaceOrigin, SkBackingFit::kExact, SkBudgeted::kNo, GrProtected::kNo);
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(ProxyRefTest, reporter, ctxInfo) {
    GrResourceProvider* resourceProvider = ctxInfo.grContext()->priv().resourceProvider();

    for (auto make : { make_deferred, make_wrapped }) {
        // Pending IO ref
        {
            sk_sp<GrTextureProxy> proxy((*make)(ctxInfo.grContext()));
            if (proxy.get()) {
                GrProxyPendingIO pendingIO(proxy.get());

                int backingRefs = proxy->isInstantiated() ? 1 : -1;

                check_refs(reporter, proxy.get(), 2, backingRefs);

                proxy->instantiate(resourceProvider);

                check_refs(reporter, proxy.get(), 2, 1);
            }
            check_refs(reporter, proxy.get(), 1, 1);
        }

        // Multiple normal refs
        {
            sk_sp<GrTextureProxy> proxy((*make)(ctxInfo.grContext()));
            if (proxy.get()) {
                proxy->ref();
                proxy->ref();

                int backingRefs = proxy->isInstantiated() ? 1 : -1;

                check_refs(reporter, proxy.get(), 3, backingRefs);

                proxy->instantiate(resourceProvider);

                check_refs(reporter, proxy.get(), 3, 1);

                proxy->unref();
                proxy->unref();
            }
            check_refs(reporter, proxy.get(), 1, 1);
        }

        // Continue using (reffing) proxy after instantiation
        {
            sk_sp<GrTextureProxy> proxy((*make)(ctxInfo.grContext()));
            if (proxy.get()) {
                proxy->ref();

                GrProxyPendingIO pendingIO(proxy.get());

                int backingRefs = proxy->isInstantiated() ? 1 : -1;

                check_refs(reporter, proxy.get(), 3, backingRefs);

                proxy->instantiate(resourceProvider);

                check_refs(reporter, proxy.get(), 3, 1);

                proxy->unref();
                check_refs(reporter, proxy.get(), 2, 1);

                GrProxyPendingIO secondPendingIO(proxy.get());
                check_refs(reporter, proxy.get(), 3, 1);
            }
            check_refs(reporter, proxy.get(), 1, 1);
        }
    }
}
