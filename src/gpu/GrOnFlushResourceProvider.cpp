/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrOnFlushResourceProvider.h"

#include "include/gpu/GrDirectContext.h"
#include "include/gpu/GrRecordingContext.h"
#include "src/gpu/GrDirectContextPriv.h"
#include "src/gpu/GrDrawingManager.h"
#include "src/gpu/GrProxyProvider.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/GrResourceProvider.h"
#include "src/gpu/GrSurfaceProxy.h"
#include "src/gpu/GrSurfaceProxyPriv.h"
#include "src/gpu/GrTextureResolveRenderTask.h"

bool GrOnFlushResourceProvider::instatiateProxy(GrSurfaceProxy* proxy) {
    SkASSERT(proxy->canSkipResourceAllocator());

    // TODO: this class should probably just get a GrDirectContext
    auto direct = fDrawingMgr->getContext()->asDirectContext();
    if (!direct) {
        return false;
    }

    auto resourceProvider = direct->priv().resourceProvider();

    if (proxy->isLazy()) {
        return proxy->priv().doLazyInstantiation(resourceProvider);
    }

    return proxy->instantiate(resourceProvider);
}

const GrCaps* GrOnFlushResourceProvider::caps() const {
    return fDrawingMgr->getContext()->priv().caps();
}
