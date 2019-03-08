/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrDeinstantiateProxyTracker.h"

#include "GrSurfaceProxy.h"
#include "GrSurfaceProxyPriv.h"

void GrDeinstantiateProxyTracker::addProxy(GrSurfaceProxy* proxy) {
#ifdef SK_DEBUG
    using LazyType = GrSurfaceProxy::LazyInstantiationType;
    SkASSERT(LazyType::kDeinstantiate == proxy->priv().lazyInstantiationType());
    for (int i = 0; i < fProxies.count(); ++i) {
        SkASSERT(proxy != fProxies[i].get());
    }
#endif
    fProxies.push_back(sk_ref_sp(proxy));
}

void GrDeinstantiateProxyTracker::deinstantiateAllProxies() {
    for (int i = 0; i < fProxies.count(); ++i) {
        GrSurfaceProxy* proxy = fProxies[i].get();
        SkASSERT(proxy->priv().isSafeToDeinstantiate());
        proxy->deinstantiate();
    }

    fProxies.reset();
}
