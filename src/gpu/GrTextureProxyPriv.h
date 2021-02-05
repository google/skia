/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrTextureProxyPriv_DEFINED
#define GrTextureProxyPriv_DEFINED

#include "src/gpu/GrTextureProxy.h"

class GrOpFlushState;

/**
 * This class hides the more specialized capabilities of GrTextureProxy.
 */
class GrTextureProxyPriv {
public:

private:
    explicit GrTextureProxyPriv(GrTextureProxy* textureProxy) : fTextureProxy(textureProxy) {}
    GrTextureProxyPriv(const GrTextureProxyPriv&) = delete;
    GrTextureProxyPriv& operator=(const GrTextureProxyPriv&) = delete;

    // No taking addresses of this type.
    const GrTextureProxyPriv* operator&() const;
    GrTextureProxyPriv* operator&();

    GrTextureProxy* fTextureProxy;

    friend class GrTextureProxy;  // to construct/copy this type.
};

inline GrTextureProxyPriv GrTextureProxy::texPriv() { return GrTextureProxyPriv(this); }

inline const GrTextureProxyPriv GrTextureProxy::texPriv() const {  // NOLINT(readability-const-return-type)
    return GrTextureProxyPriv(const_cast<GrTextureProxy*>(this));
}

#endif
