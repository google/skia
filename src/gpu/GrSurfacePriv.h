/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrSurfacePriv_DEFINED
#define GrSurfacePriv_DEFINED

#include "GrSurface.h"

/** Class that adds methods to GrSurface that are only intended for use internal to Skia.
    This class is purely a privileged window into GrSurface. It should never have additional data
    members or virtual methods.
    Non-static methods that are not trivial inlines should be spring-boarded (e.g. declared and
    implemented privately in GrSurface with a inline public method here). */
class GrSurfacePriv {
public:
    /**
     * Derive a SkImageInfo from the surface's descriptor. This is lossy as ImageInfo has fields not
     * known to GrSurface (e.g. alphaType).
     */
    SkImageInfo info() const { return fSurface->info(); }

    /**
     * Checks whether this GrSurface refers to the same GPU object as other. This
     * catches the case where a GrTexture and GrRenderTarget refer to the same
     * GPU memory.
     */
    bool isSameAs(const GrSurface* other) const { return fSurface->isSameAs(other); }

    /**
     * Write the contents of the surface to a PNG. Returns true if successful.
     * @param filename      Full path to desired file
     */
    bool savePixels(const char* filename) { return fSurface->savePixels(filename); }

    bool hasPendingRead() const { return fSurface->hasPendingRead(); }
    bool hasPendingWrite() const { return fSurface->hasPendingWrite(); }
    bool hasPendingIO() const { return fSurface->hasPendingIO(); }

private:
    GrSurfacePriv(GrSurface* surface) : fSurface(surface) { }
    GrSurfacePriv(const GrSurfacePriv& that) : fSurface(that.fSurface) { }
    GrSurfacePriv& operator=(const GrSurface&); // unimpl

    // No taking addresses of this type.
    const GrSurfacePriv* operator&() const;
    GrSurfacePriv* operator&();

    GrSurface* fSurface;

    friend class GrSurface; // to construct/copy this type.
};

inline GrSurfacePriv GrSurface::surfacePriv() { return GrSurfacePriv(this); }

inline const GrSurfacePriv GrSurface::surfacePriv() const {
    return GrSurfacePriv(const_cast<GrSurface*>(this));
}

#endif
