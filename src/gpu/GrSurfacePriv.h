/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrSurfacePriv_DEFINED
#define GrSurfacePriv_DEFINED

#include "include/gpu/GrSurface.h"

/** Class that adds methods to GrSurface that are only intended for use internal to Skia.
    This class is purely a privileged window into GrSurface. It should never have additional data
    members or virtual methods.
    Non-static methods that are not trivial inlines should be spring-boarded (e.g. declared and
    implemented privately in GrSurface with a inline public method here). */
class GrSurfacePriv {
public:
    GrInternalSurfaceFlags flags() const { return fSurface->fSurfaceFlags; }

private:
    explicit GrSurfacePriv(GrSurface* surface) : fSurface(surface) {}
    GrSurfacePriv(const GrSurfacePriv&); // unimpl
    GrSurfacePriv& operator=(const GrSurfacePriv&); // unimpl

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
