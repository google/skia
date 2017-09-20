/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrTextureProxyPriv_DEFINED
#define GrTextureProxyPriv_DEFINED

#include "GrTextureProxy.h"

/** Class that adds methods to GrTextureProxy that are rather specialized and shouldn't be
    widely used. This class is purely a privileged window into GrTextureProxy. It should never
    have additional data members or virtual methods. */
class GrTextureProxyPriv {
protected:
    // There are only accessible by GrResourceCache (via friending)
    friend class GrResourceCache;

    void setUniqueKey(GrResourceCache* cache, const GrUniqueKey& key) {
        fTextureProxy->setUniqueKey(cache, key);
    }

    void clearUniqueKey() {
        fTextureProxy->clearUniqueKey();
    }

private:
    explicit GrTextureProxyPriv(GrTextureProxy* textureProxy) : fTextureProxy(textureProxy) {}
    GrTextureProxyPriv(const GrTextureProxyPriv&) {} // unimpl
    GrTextureProxyPriv& operator=(const GrTextureProxyPriv&); // unimpl

    // No taking addresses of this type.
    const GrTextureProxyPriv* operator&() const;
    GrTextureProxyPriv* operator&();

    GrTextureProxy* fTextureProxy;

    friend class GrTextureProxy; // to construct/copy this type.
};

inline GrTextureProxyPriv GrTextureProxy::texPriv() { return GrTextureProxyPriv(this); }

inline const GrTextureProxyPriv GrTextureProxy::texPriv () const {
    return GrTextureProxyPriv(const_cast<GrTextureProxy*>(this));
}

#endif
