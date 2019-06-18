/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrDeinstantiateProxyTracker_DEFINED
#define GrDeinstantiateProxyTracker_DEFINED

#include "include/private/SkTArray.h"
#include "src/gpu/GrSurfaceProxy.h"

class GrResourceCache;

class GrDeinstantiateProxyTracker {
public:
    GrDeinstantiateProxyTracker() {}

    // Adds a proxy which will be deinstantiated at the end of flush. The same proxy may not be
    // added multiple times.
    void addProxy(GrSurfaceProxy* proxy);

    // Loops through all tracked proxies and deinstantiates them.
    void deinstantiateAllProxies();

private:
    SkTArray<sk_sp<GrSurfaceProxy>> fProxies;
};

#endif
