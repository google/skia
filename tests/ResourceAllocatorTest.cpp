/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// Include here to ensure SK_SUPPORT_GPU is set correctly before it is examined.
#include "SkTypes.h"

#if SK_SUPPORT_GPU
#include "Test.h"

#include "GrResourceAllocator.h"
#include "GrSurfaceProxyPriv.h"
#include "GrTextureProxy.h"

// Basic test that two proxies with overlapping intervals and compatible descriptors are
// assigned different GrSurfaces.
static void overlap_test(skiatest::Reporter* reporter, GrResourceProvider* resourceProvider) {
    GrSurfaceDesc desc;
    desc.fOrigin = kTopLeft_GrSurfaceOrigin;
    desc.fWidth  = 64;
    desc.fHeight = 64;
    desc.fConfig = kRGBA_8888_GrPixelConfig;

    sk_sp<GrSurfaceProxy> p1 = GrSurfaceProxy::MakeDeferred(resourceProvider, desc,
                                                            SkBackingFit::kApprox,
                                                            SkBudgeted::kNo);
    sk_sp<GrSurfaceProxy> p2 = GrSurfaceProxy::MakeDeferred(resourceProvider, desc,
                                                            SkBackingFit::kApprox,
                                                            SkBudgeted::kNo);

    GrResourceAllocator alloc(resourceProvider);

    alloc.addInterval(p1.get(), 0, 4);
    alloc.addInterval(p2.get(), 1, 2);

    alloc.assign();

    REPORTER_ASSERT(reporter, p1->priv().peekSurface());
    REPORTER_ASSERT(reporter, p2->priv().peekSurface());
    REPORTER_ASSERT(reporter, p1->underlyingUniqueID() != p2->underlyingUniqueID());
}

// Basic test that two proxies with non-overlapping intervals and compatible descriptors are
// assigned the same GrSurface. This also acts as a test of the ResourceAllocator's 
// free pool.
static void non_overlap_test(skiatest::Reporter* reporter, GrResourceProvider* resourceProvider) {
    GrSurfaceDesc desc;
    desc.fOrigin = kTopLeft_GrSurfaceOrigin;
    desc.fWidth  = 64;
    desc.fHeight = 64;
    desc.fConfig = kRGBA_8888_GrPixelConfig;

    sk_sp<GrSurfaceProxy> p1 = GrSurfaceProxy::MakeDeferred(resourceProvider, desc,
                                                            SkBackingFit::kApprox,
                                                            SkBudgeted::kNo);
    sk_sp<GrSurfaceProxy> p2 = GrSurfaceProxy::MakeDeferred(resourceProvider, desc,
                                                            SkBackingFit::kApprox,
                                                            SkBudgeted::kNo);

    GrResourceAllocator alloc(resourceProvider);

    alloc.addInterval(p1.get(), 0, 2);
    alloc.addInterval(p2.get(), 3, 5);

    alloc.assign();

    REPORTER_ASSERT(reporter, p1->priv().peekSurface());
    REPORTER_ASSERT(reporter, p2->priv().peekSurface());
    REPORTER_ASSERT(reporter, p1->underlyingUniqueID() == p2->underlyingUniqueID());
}

DEF_GPUTEST_FOR_ALL_CONTEXTS(ResourceAllocatorTest, reporter, ctxInfo) {
    GrResourceProvider* resourceProvider = ctxInfo.grContext()->resourceProvider();

    overlap_test(reporter, resourceProvider);
    non_overlap_test(reporter, resourceProvider);
}

#endif
