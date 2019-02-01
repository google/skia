/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrImageContextPriv_DEFINED
#define GrImageContextPriv_DEFINED

#include "GrImageContext.h"

/** Class that exposes methods on GrImageContext that are only intended for use internal to Skia.
    This class is purely a privileged window into GrImageContext. It should never have
    additional data members or virtual methods. */
class GrImageContextPriv {
public:
    // from GrContext_Base
    uint32_t contextID() const { return fContext->contextID(); }

    // from GrImageContext

private:
    explicit GrImageContextPriv(GrImageContext* context) : fContext(context) {}
    GrImageContextPriv(const GrImageContextPriv&); // unimpl
    GrImageContextPriv& operator=(const GrImageContextPriv&); // unimpl

    // No taking addresses of this type.
    const GrImageContextPriv* operator&() const;
    GrImageContextPriv* operator&();

    GrImageContext* fContext;

    friend class GrImageContext; // to construct/copy this type.
};

inline GrImageContextPriv GrImageContext::priv() { return GrImageContextPriv(this); }

inline const GrImageContextPriv GrImageContext::priv () const {
    return GrImageContextPriv(const_cast<GrImageContext*>(this));
}

#endif
