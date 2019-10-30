/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrSurfaceContextPriv_DEFINED
#define GrSurfaceContextPriv_DEFINED

#include "src/gpu/GrSurfaceContext.h"

/** Class that adds methods to GrSurfaceContext that are only intended for use internal to
    Skia. This class is purely a privileged window into GrSurfaceContext. It should never have
    additional data members or virtual methods. */
class GrSurfaceContextPriv {
public:
    GrRecordingContext* getContext() { return fSurfaceContext->fContext; }

private:
    explicit GrSurfaceContextPriv(GrSurfaceContext* surfaceContext)
        : fSurfaceContext(surfaceContext) {
    }

    GrSurfaceContextPriv(const GrSurfaceContextPriv&) {} // unimpl
    GrSurfaceContextPriv& operator=(const GrSurfaceContextPriv&); // unimpl

    // No taking addresses of this type.
    const GrSurfaceContextPriv* operator&() const;
    GrSurfaceContextPriv* operator&();

    GrSurfaceContext* fSurfaceContext;

    friend class GrSurfaceContext; // to construct/copy this type.
};

inline GrSurfaceContextPriv GrSurfaceContext::surfPriv() {
    return GrSurfaceContextPriv(this);
}

inline const GrSurfaceContextPriv GrSurfaceContext::surfPriv() const {
    return GrSurfaceContextPriv(const_cast<GrSurfaceContext*>(this));
}

#endif
