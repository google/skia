/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrTextureProxyPriv_DEFINED
#define GrTextureProxyPriv_DEFINED

#include "src/gpu/GrTextureProxy.h"

class GrDeferredProxyUploader;
class GrOpFlushState;

/**
 * This class hides the more specialized capabilities of GrTextureProxy.
 */
class GrTextureProxyPriv {
public:
    // Attach a deferred uploader to the proxy. Holds data being prepared by a worker thread.
    void setDeferredUploader(std::unique_ptr<GrDeferredProxyUploader>);
    bool isDeferred() const { return SkToBool(fTextureProxy->fDeferredUploader.get()); }
    // For a deferred proxy (one that has a deferred uploader attached), this schedules an ASAP
    // upload of that data to the instantiated texture.
    void scheduleUpload(GrOpFlushState*);
    // Clears any deferred uploader object on the proxy. Used to free the CPU data after the
    // contents have been uploaded.
    void resetDeferredUploader();

private:
    explicit GrTextureProxyPriv(GrTextureProxy* textureProxy) : fTextureProxy(textureProxy) {}
    GrTextureProxyPriv(const GrTextureProxyPriv&) {} // unimpl
    GrTextureProxyPriv& operator=(const GrTextureProxyPriv&); // unimpl

    // No taking addresses of this type.
    const GrTextureProxyPriv* operator&() const;
    GrTextureProxyPriv* operator&();

    GrTextureProxy* fTextureProxy;

    friend class GrTextureProxy;  // to construct/copy this type.
};

inline GrTextureProxyPriv GrTextureProxy::texPriv() { return GrTextureProxyPriv(this); }

inline const GrTextureProxyPriv GrTextureProxy::texPriv() const {
    return GrTextureProxyPriv(const_cast<GrTextureProxy*>(this));
}

#endif
