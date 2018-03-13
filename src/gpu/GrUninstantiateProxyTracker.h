/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrUninstantiateProxyTracker_DEFINED
#define GrUninstantiateProxyTracker_DEFINED

#include "GrSurfaceProxy.h"
#include "SkTArray.h"

class GrUninstantiateProxyTracker {
public:
    GrUninstantiateProxyTracker() {}

    // Adds a proxy which will be uninstantiated at the end of flush. The same proxy may not be
    // added multiple times.
    void addProxy(GrSurfaceProxy* proxy);

    // Loops through all tracked proxies and uninstantiates them.
    void uninstantiateAllProxies();

private:
    SkTArray<sk_sp<GrSurfaceProxy>> fProxies;
};

#endif
