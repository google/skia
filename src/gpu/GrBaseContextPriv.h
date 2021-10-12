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
    GrContext_Base* context() { return fContext; }
    const GrContext_Base* context() const { return fContext; }

    uint32_t contextID() const { return this->context()->contextID(); }

    bool matches(GrContext_Base* candidate) const { return this->context()->matches(candidate); }

    const GrContextOptions& options() const { return this->context()->options(); }

    const GrCaps* caps() const { return this->context()->caps(); }
    sk_sp<const GrCaps> refCaps() const;

    GrImageContext* asImageContext() { return this->context()->asImageContext(); }
    GrRecordingContext* asRecordingContext() { return this->context()->asRecordingContext(); }
    GrDirectContext* asDirectContext() { return this->context()->asDirectContext(); }

    GrContextOptions::ShaderErrorHandler* getShaderErrorHandler() const;

protected:
    // Required until C++17 copy elision
    GrBaseContextPriv(const GrBaseContextPriv&) = default;
    explicit GrBaseContextPriv(GrContext_Base* context) : fContext(context) {}

    GrContext_Base* fContext;

private:
    GrBaseContextPriv& operator=(const GrBaseContextPriv&) = delete;

    // No taking addresses of this type.
    const GrBaseContextPriv* operator&() const;
    GrBaseContextPriv* operator&();

    friend class GrContext_Base; // to construct/copy this type.
};

inline GrBaseContextPriv GrContext_Base::priv() { return GrBaseContextPriv(this); }

inline const GrBaseContextPriv GrContext_Base::priv () const {  // NOLINT(readability-const-return-type)
    return GrBaseContextPriv(const_cast<GrContext_Base*>(this));
}

#endif
