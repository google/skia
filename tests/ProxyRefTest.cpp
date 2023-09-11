/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// This is a GPU-backend specific test.

#include "include/core/SkRefCnt.h"
#include "include/core/SkTypes.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/GrRecordingContext.h"
#include "include/gpu/GrTypes.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/gpu/SkBackingFit.h"
#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#include "src/gpu/ganesh/GrProxyProvider.h"
#include "src/gpu/ganesh/GrRecordingContextPriv.h"
#include "src/gpu/ganesh/GrTextureProxy.h"
#include "tests/CtsEnforcement.h"
#include "tests/Test.h"
#include "tests/TestUtils.h"

#include <initializer_list>

class GrResourceProvider;
struct GrContextOptions;

static const int kWidthHeight = 128;

static sk_sp<GrTextureProxy> make_deferred(GrRecordingContext* rContext) {
    GrProxyProvider* proxyProvider = rContext->priv().proxyProvider();
    const GrCaps* caps = rContext->priv().caps();

    const GrBackendFormat format = caps->getDefaultBackendFormat(GrColorType::kRGBA_8888,
                                                                 GrRenderable::kYes);
    return proxyProvider->createProxy(format,
                                      {kWidthHeight, kWidthHeight},
                                      GrRenderable::kYes,
                                      1,
                                      skgpu::Mipmapped::kNo,
                                      SkBackingFit::kApprox,
                                      skgpu::Budgeted::kYes,
                                      GrProtected::kNo,
                                      /*label=*/"ProxyRefTest");
}

static sk_sp<GrTextureProxy> make_wrapped(GrRecordingContext* rContext) {
    GrProxyProvider* proxyProvider = rContext->priv().proxyProvider();

    return proxyProvider->testingOnly_createInstantiatedProxy({kWidthHeight, kWidthHeight},
                                                              GrColorType::kRGBA_8888,
                                                              GrRenderable::kYes,
                                                              1,
                                                              SkBackingFit::kExact,
                                                              skgpu::Budgeted::kNo,
                                                              GrProtected::kNo);
}

DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(ProxyRefTest,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kApiLevel_T) {
    auto dContext = ctxInfo.directContext();
    GrResourceProvider* resourceProvider = dContext->priv().resourceProvider();

    for (auto make : { make_deferred, make_wrapped }) {
        // An extra ref
        {
            sk_sp<GrTextureProxy> proxy((*make)(dContext));
            if (proxy) {
                sk_sp<GrTextureProxy> extraRef(proxy);  // NOLINT(performance-unnecessary-copy-initialization)

                int backingRefs = proxy->isInstantiated() ? 1 : -1;

                CheckSingleThreadedProxyRefs(reporter, proxy.get(), 2, backingRefs);

                proxy->instantiate(resourceProvider);

                CheckSingleThreadedProxyRefs(reporter, proxy.get(), 2, 1);
            }
            CheckSingleThreadedProxyRefs(reporter, proxy.get(), 1, 1);
        }

        // Multiple normal refs
        {
            sk_sp<GrTextureProxy> proxy((*make)(dContext));
            if (proxy) {
                proxy->ref();
                proxy->ref();

                int backingRefs = proxy->isInstantiated() ? 1 : -1;

                CheckSingleThreadedProxyRefs(reporter, proxy.get(), 3, backingRefs);

                proxy->instantiate(resourceProvider);

                CheckSingleThreadedProxyRefs(reporter, proxy.get(), 3, 1);

                proxy->unref();
                proxy->unref();
            }
            CheckSingleThreadedProxyRefs(reporter, proxy.get(), 1, 1);
        }

        // Continue using (reffing) proxy after instantiation
        {
            sk_sp<GrTextureProxy> proxy((*make)(dContext));
            if (proxy) {
                sk_sp<GrTextureProxy> firstExtraRef(proxy);  // NOLINT(performance-unnecessary-copy-initialization)

                int backingRefs = proxy->isInstantiated() ? 1 : -1;

                CheckSingleThreadedProxyRefs(reporter, proxy.get(), 2, backingRefs);

                proxy->instantiate(resourceProvider);

                CheckSingleThreadedProxyRefs(reporter, proxy.get(), 2, 1);

                sk_sp<GrTextureProxy> secondExtraRef(proxy);  // NOLINT(performance-unnecessary-copy-initialization)
                CheckSingleThreadedProxyRefs(reporter, proxy.get(), 3, 1);
            }
            CheckSingleThreadedProxyRefs(reporter, proxy.get(), 1, 1);
        }
    }
}
