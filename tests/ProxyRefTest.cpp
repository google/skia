/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// This is a GPU-backend specific test.

#include "Test.h"

#if SK_SUPPORT_GPU
#include "GrSurfaceProxy.h"
#include "GrTextureProxy.h"
#include "GrRenderTargetPriv.h"
#include "GrRenderTargetProxy.h"

static const int kWidthHeight = 128;

int32_t GrIORefProxy::getProxyRefCnt_TestOnly() const {
    return fRefCnt;
}

int32_t GrIORefProxy::getBackingRefCnt_TestOnly() const {
    if (fTarget) {
        return fTarget->fRefCnt;
    }

    return fRefCnt;
}

int32_t GrIORefProxy::getPendingReadCnt_TestOnly() const {
    if (fTarget) {
        SkASSERT(!fPendingReads);
        return fTarget->fPendingReads;
    }

    return fPendingReads;
}

int32_t GrIORefProxy::getPendingWriteCnt_TestOnly() const {
    if (fTarget) {
        SkASSERT(!fPendingWrites);
        return fTarget->fPendingWrites;
    }

    return fPendingWrites;
}

static void check_refs(skiatest::Reporter* reporter,
                       GrSurfaceProxy* proxy,
                       int32_t expectedProxyRefs,
                       int32_t expectedBackingRefs,
                       int32_t expectedNumReads,
                       int32_t expectedNumWrites) {
    REPORTER_ASSERT(reporter, proxy->getProxyRefCnt_TestOnly() == expectedProxyRefs);
    REPORTER_ASSERT(reporter, proxy->getBackingRefCnt_TestOnly() == expectedBackingRefs);
    REPORTER_ASSERT(reporter, proxy->getPendingReadCnt_TestOnly() == expectedNumReads);
    REPORTER_ASSERT(reporter, proxy->getPendingWriteCnt_TestOnly() == expectedNumWrites);

    SkASSERT(proxy->getProxyRefCnt_TestOnly() == expectedProxyRefs);
    SkASSERT(proxy->getBackingRefCnt_TestOnly() == expectedBackingRefs);
    SkASSERT(proxy->getPendingReadCnt_TestOnly() == expectedNumReads);
    SkASSERT(proxy->getPendingWriteCnt_TestOnly() == expectedNumWrites);
}

static sk_sp<GrSurfaceProxy> make_deferred(const GrCaps& caps, GrTextureProvider* provider) {
    GrSurfaceDesc desc;
    desc.fFlags = kRenderTarget_GrSurfaceFlag;
    desc.fWidth = kWidthHeight;
    desc.fHeight = kWidthHeight;
    desc.fConfig = kRGBA_8888_GrPixelConfig;

    return GrSurfaceProxy::MakeDeferred(caps, desc, SkBackingFit::kApprox, SkBudgeted::kYes);
}

static sk_sp<GrSurfaceProxy> make_wrapped(const GrCaps& caps, GrTextureProvider* provider) {
    GrSurfaceDesc desc;
    desc.fFlags = kRenderTarget_GrSurfaceFlag;
    desc.fWidth = kWidthHeight;
    desc.fHeight = kWidthHeight;
    desc.fConfig = kRGBA_8888_GrPixelConfig;

    sk_sp<GrTexture> tex(provider->createTexture(desc, SkBudgeted::kNo));

    // Flush the IOWrite from the initial discard or it will confuse the later ref count checks
    tex->flushWrites();

    return GrSurfaceProxy::MakeWrapped(std::move(tex));
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(ProxyRefTest, reporter, ctxInfo) {
    GrTextureProvider* provider = ctxInfo.grContext()->textureProvider();
    const GrCaps& caps = *ctxInfo.grContext()->caps();

    for (auto make : { make_deferred, make_wrapped }) {

        // A single write
        {
            sk_sp<GrSurfaceProxy> sProxy((*make)(caps, provider));

            GrPendingIOResource<GrSurfaceProxy, kWrite_GrIOType> fWrite(sProxy.get());

            check_refs(reporter, sProxy.get(), 1, 1, 0, 1);

            // In the deferred case, the discard batch created on instantiation adds an
            // extra ref and write
            int expectedRefs = !sProxy->isWrapped_ForTesting() ? 2 : 1;

            sProxy->instantiate(provider);

            // In the deferred case, this checks that the refs transfered to the GrSurface
            check_refs(reporter, sProxy.get(), 1, expectedRefs, 0, expectedRefs);
        }

        // A single read
        {
            sk_sp<GrSurfaceProxy> sProxy((*make)(caps, provider));

            GrPendingIOResource<GrSurfaceProxy, kRead_GrIOType> fRead(sProxy.get());

            check_refs(reporter, sProxy.get(), 1, 1, 1, 0);

            // In the deferred case, the discard batch created on instantiation adds an
            // extra ref and write
            int expectedBackingRefs = !sProxy->isWrapped_ForTesting() ? 2 : 1;
            int expectedWrites = !sProxy->isWrapped_ForTesting() ? 1 : 0;

            sProxy->instantiate(provider);

            // In the deferred case, this checks that the refs transfered to the GrSurface
            check_refs(reporter, sProxy.get(), 1, expectedBackingRefs, 1, expectedWrites);
        }

        // A single read/write pair
        {
            sk_sp<GrSurfaceProxy> sProxy((*make)(caps, provider));

            GrPendingIOResource<GrSurfaceProxy, kRW_GrIOType> fRW(sProxy.get());

            check_refs(reporter, sProxy.get(), 1, 1, 1, 1);

            // In the deferred case, the discard batch created on instantiation adds an
            // extra ref and write
            int expectedBackingRefs = !sProxy->isWrapped_ForTesting() ? 2 : 1;
            int expectedWrites = !sProxy->isWrapped_ForTesting() ? 2 : 1;

            sProxy->instantiate(provider);

            // In the deferred case, this checks that the refs transfered to the GrSurface
            check_refs(reporter, sProxy.get(), 1, expectedBackingRefs, 1, expectedWrites);
        }

        // Multiple normal refs
        {
            sk_sp<GrSurfaceProxy> sProxy((*make)(caps, provider));
            sProxy->ref();
            sProxy->ref();

            check_refs(reporter, sProxy.get(), 3, 3, 0, 0);

            int expectedBackingRefs = !sProxy->isWrapped_ForTesting() ? 4 : 3;
            int expectedWrites = !sProxy->isWrapped_ForTesting() ? 1 : 0;

            sProxy->instantiate(provider);

            // In the deferred case, this checks that the refs transfered to the GrSurface
            check_refs(reporter, sProxy.get(), 3, expectedBackingRefs, 0, expectedWrites);

            sProxy->unref();
            sProxy->unref();
        }

        // Continue using (reffing) proxy after instantiation
        {
            sk_sp<GrSurfaceProxy> sProxy((*make)(caps, provider));
            sProxy->ref();

            GrPendingIOResource<GrSurfaceProxy, kWrite_GrIOType> fWrite(sProxy.get());

            check_refs(reporter, sProxy.get(), 2, 2, 0, 1);

            int expectedBackingRefs = !sProxy->isWrapped_ForTesting() ? 3 : 2;
            int expectedWrites = !sProxy->isWrapped_ForTesting() ? 2 : 1;

            sProxy->instantiate(provider);

            // In the deferred case, this checks that the refs transfered to the GrSurface
            check_refs(reporter, sProxy.get(), 2, expectedBackingRefs, 0, expectedWrites);

            sProxy->unref();
            check_refs(reporter, sProxy.get(), 1, expectedBackingRefs-1, 0, expectedWrites);

            GrPendingIOResource<GrSurfaceProxy, kRead_GrIOType> fRead(sProxy.get());
            check_refs(reporter, sProxy.get(), 1, expectedBackingRefs-1, 1, expectedWrites);
        }
    }
}

#endif
