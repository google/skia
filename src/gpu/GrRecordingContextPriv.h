/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrRecordingContextPriv_DEFINED
#define GrRecordingContextPriv_DEFINED

#include "GrRecordingContext.h"

/** Class that exposes methods to GrRecordingContext that are only intended for use internal to
    Skia. This class is purely a privileged window into GrRecordingContext. It should never have
    additional data members or virtual methods. */
class GrRecordingContextPriv {
public:
    // from GrContext_Base
    uint32_t contextID() const { return fContext->contextID(); }

    // from GrImageContext

    // from GrRecordingContext

private:
    explicit GrRecordingContextPriv(GrRecordingContext* context) : fContext(context) {}
    GrRecordingContextPriv(const GrRecordingContextPriv&); // unimpl
    GrRecordingContextPriv& operator=(const GrRecordingContextPriv&); // unimpl

    // No taking addresses of this type.
    const GrRecordingContextPriv* operator&() const;
    GrRecordingContextPriv* operator&();

    GrRecordingContext* fContext;

    friend class GrRecordingContext; // to construct/copy this type.
};

inline GrRecordingContextPriv GrRecordingContext::priv() { return GrRecordingContextPriv(this); }

inline const GrRecordingContextPriv GrRecordingContext::priv () const {
    return GrRecordingContextPriv(const_cast<GrRecordingContext*>(this));
}

#endif
