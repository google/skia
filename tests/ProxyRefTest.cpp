/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// This is a GPU-backend specific test.

#include "tests/Test.h"

#include "include/gpu/GrTexture.h"
#include "include/private/GrRenderTargetProxy.h"
#include "include/private/GrSurfaceProxy.h"
#include "include/private/GrTextureProxy.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrPendingIOResource.h"
#include "src/gpu/GrProxyProvider.h"
#include "src/gpu/GrResourceProvider.h"

int32_t GrIORefProxy::getBackingRefCnt_TestOnly() const {
    if (fTarget) {
        return fTarget->fRefCnt;
    }

    return -1; // no backing GrSurface
}

int32_t GrIORefProxy::getPendingReadCnt_TestOnly() const {
    if (fTarget) {
        return fTarget->fPendingReads;
    }

    return fPendingReads;
}

int32_t GrIORefProxy::getPendingWriteCnt_TestOnly() const {
    if (fTarget) {
        return fTarget->fPendingWrites;
    }

    return fPendingWrites;
}

static const int kWidthHeight = 128;

static void check_refs(skiatest::Reporter* reporter,
                       GrTextureProxy* proxy,
                       int32_t expectedProxyRefs,
                       int32_t expectedBackingRefs,
                       int32_t expectedNumReads,
                       int32_t expectedNumWrites) {
    REPORTER_ASSERT(reporter, proxy->priv().getProxyRefCnt() == expectedProxyRefs);
    REPORTER_ASSERT(reporter, proxy->getBackingRefCnt_TestOnly() == expectedBackingRefs);
    REPORTER_ASSERT(reporter, proxy->getPendingReadCnt_TestOnly() == expectedNumReads);
    REPORTER_ASSERT(reporter, proxy->getPendingWriteCnt_TestOnly() == expectedNumWrites);

    SkASSERT(proxy->priv().getProxyRefCnt() == expectedProxyRefs);
    SkASSERT(proxy->getBackingRefCnt_TestOnly() == expectedBackingRefs);
    SkASSERT(proxy->getPendingReadCnt_TestOnly() == expectedNumReads);
    SkASSERT(proxy->getPendingWriteCnt_TestOnly() == expectedNumWrites);
}

static sk_sp<GrTextureProxy> make_deferred(GrProxyProvider* proxyProvider, const GrCaps* caps) {
    GrSurfaceDesc desc;
    desc.fFlags = kRenderTarget_GrSurfaceFlag;
    desc.fWidth = kWidthHeight;
    desc.fHeight = kWidthHeight;
    desc.fConfig = kRGBA_8888_GrPixelConfig;

    const GrBackendFormat format = caps->getBackendFormatFromColorType(kRGBA_8888_SkColorType);
    return proxyProvider->createProxy(format, desc, kBottomLeft_GrSurfaceOrigin,
                                      SkBackingFit::kApprox, SkBudgeted::kYes);
}

static sk_sp<GrTextureProxy> make_wrapped(GrProxyProvider* proxyProvider, const GrCaps* caps) {
    GrSurfaceDesc desc;
    desc.fFlags = kRenderTarget_GrSurfaceFlag;
    desc.fWidth = kWidthHeight;
    desc.fHeight = kWidthHeight;
    desc.fConfig = kRGBA_8888_GrPixelConfig;

    return proxyProvider->testingOnly_createInstantiatedProxy(
            desc, kBottomLeft_GrSurfaceOrigin, SkBackingFit::kExact, SkBudgeted::kNo);
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(ProxyRefTest, reporter, ctxInfo) {
    GrProxyProvider* proxyProvider = ctxInfo.grContext()->priv().proxyProvider();
    GrResourceProvider* resourceProvider = ctxInfo.grContext()->priv().resourceProvider();
    const GrCaps* caps = ctxInfo.grContext()->priv().caps();

    for (auto make : { make_deferred, make_wrapped }) {
        // A single write
        {
            sk_sp<GrTextureProxy> proxy((*make)(proxyProvider, caps));
            if (proxy.get()) {
                GrPendingIOResource<GrSurfaceProxy, kWrite_GrIOType> fWrite(proxy.get());

                static const int kExpectedReads = 0;
                static const int kExpectedWrites = 1;

                int backingRefs = proxy->isWrapped_ForTesting() ? 1 : -1;

                check_refs(reporter, proxy.get(), 1, backingRefs, kExpectedReads, kExpectedWrites);

                proxy->instantiate(resourceProvider);

                // In the deferred case, this checks that the refs transfered to the GrSurface
                check_refs(reporter, proxy.get(), 1, 1, kExpectedReads, kExpectedWrites);
            }
        }

        // A single read
        {
            sk_sp<GrTextureProxy> proxy((*make)(proxyProvider, caps));
            if (proxy.get()) {
                GrPendingIOResource<GrSurfaceProxy, kRead_GrIOType> fRead(proxy.get());

                static const int kExpectedReads = 1;
                static const int kExpectedWrites = 0;

                int backingRefs = proxy->isWrapped_ForTesting() ? 1 : -1;

                check_refs(reporter, proxy.get(), 1, backingRefs, kExpectedReads, kExpectedWrites);

                proxy->instantiate(resourceProvider);

                // In the deferred case, this checks that the refs transfered to the GrSurface
                check_refs(reporter, proxy.get(), 1, 1, kExpectedReads, kExpectedWrites);
            }
        }

        // A single read/write pair
        {
            sk_sp<GrTextureProxy> proxy((*make)(proxyProvider, caps));
            if (proxy.get()) {
                GrPendingIOResource<GrSurfaceProxy, kRW_GrIOType> fRW(proxy.get());

                static const int kExpectedReads = 1;
                static const int kExpectedWrites = 1;

                int backingRefs = proxy->isWrapped_ForTesting() ? 1 : -1;

                check_refs(reporter, proxy.get(), 1, backingRefs, kExpectedReads, kExpectedWrites);

                proxy->instantiate(resourceProvider);

                // In the deferred case, this checks that the refs transferred to the GrSurface
                check_refs(reporter, proxy.get(), 1, 1, kExpectedReads, kExpectedWrites);
            }
        }

        // Multiple normal refs
        {
            sk_sp<GrTextureProxy> proxy((*make)(proxyProvider, caps));
            if (proxy.get()) {
                proxy->ref();
                proxy->ref();

                static const int kExpectedReads = 0;
                static const int kExpectedWrites = 0;

                int backingRefs = proxy->isWrapped_ForTesting() ? 3 : -1;

                check_refs(reporter, proxy.get(), 3, backingRefs, kExpectedReads, kExpectedWrites);

                proxy->instantiate(resourceProvider);

                // In the deferred case, this checks that the refs transferred to the GrSurface
                check_refs(reporter, proxy.get(), 3, 3, kExpectedReads, kExpectedWrites);

                proxy->unref();
                proxy->unref();
            }
        }

        // Continue using (reffing) proxy after instantiation
        {
            sk_sp<GrTextureProxy> proxy((*make)(proxyProvider, caps));
            if (proxy.get()) {
                proxy->ref();

                GrPendingIOResource<GrSurfaceProxy, kWrite_GrIOType> fWrite(proxy.get());

                static const int kExpectedWrites = 1;

                int backingRefs = proxy->isWrapped_ForTesting() ? 2 : -1;

                check_refs(reporter, proxy.get(), 2, backingRefs, 0, kExpectedWrites);

                proxy->instantiate(resourceProvider);

                // In the deferred case, this checks that the refs transfered to the GrSurface
                check_refs(reporter, proxy.get(), 2, 2, 0, kExpectedWrites);

                proxy->unref();
                check_refs(reporter, proxy.get(), 1, 1, 0, kExpectedWrites);

                GrPendingIOResource<GrSurfaceProxy, kRead_GrIOType> fRead(proxy.get());
                check_refs(reporter, proxy.get(), 1, 1, 1, kExpectedWrites);
            }
        }
    }
}
