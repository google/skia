/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrBaseContextPriv_DEFINED
#define GrBaseContextPriv_DEFINED

#include "include/private/GrContext_Base.h"

/** Class that exposes methods on GrContext_Base that are only intended for use internal to Skia.
    This class is purely a privileged window into GrContext_Base. It should never have
    additional data members or virtual methods. */
class GrBaseContextPriv {
public:
    // from GrContext_Base
    uint32_t contextID() const { return fContext->contextID(); }

    bool matches(GrContext_Base* candidate) const { return fContext->matches(candidate); }

    const GrContextOptions& options() const { return fContext->options(); }

    const GrCaps* caps() const { return fContext->caps(); }
    sk_sp<const GrCaps> refCaps() const;

    GrImageContext* asImageContext() { return fContext->asImageContext(); }
    GrRecordingContext* asRecordingContext() { return fContext->asRecordingContext(); }
    GrContext* asDirectContext() { return fContext->asDirectContext(); }

    GrContextOptions::ShaderErrorHandler* getShaderErrorHandler() const;

private:
    explicit GrBaseContextPriv(GrContext_Base* context) : fContext(context) {}
    GrBaseContextPriv(const GrBaseContextPriv&); // unimpl
    GrBaseContextPriv& operator=(const GrBaseContextPriv&); // unimpl

    // No taking addresses of this type.
    const GrBaseContextPriv* operator&() const;
    GrBaseContextPriv* operator&();

    GrContext_Base* fContext;

    friend class GrContext_Base; // to construct/copy this type.
};

inline GrBaseContextPriv GrContext_Base::priv() { return GrBaseContextPriv(this); }

inline const GrBaseContextPriv GrContext_Base::priv () const {
    return GrBaseContextPriv(const_cast<GrContext_Base*>(this));
}

#endif
