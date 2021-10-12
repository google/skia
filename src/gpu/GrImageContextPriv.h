/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrImageContextPriv_DEFINED
#define GrImageContextPriv_DEFINED

#include "include/private/GrImageContext.h"

#include "include/gpu/GrContextThreadSafeProxy.h"
#include "src/gpu/GrBaseContextPriv.h"

/** Class that exposes methods on GrImageContext that are only intended for use internal to Skia.
    This class is purely a privileged window into GrImageContext. It should never have
    additional data members or virtual methods. */
class GrImageContextPriv : public GrBaseContextPriv {
public:
    GrImageContext* context() { return static_cast<GrImageContext*>(fContext); }
    const GrImageContext* context() const { return static_cast<const GrImageContext*>(fContext); }

    bool abandoned() { return this->context()->abandoned(); }

    static sk_sp<GrImageContext> MakeForPromiseImage(sk_sp<GrContextThreadSafeProxy> tsp) {
        return GrImageContext::MakeForPromiseImage(std::move(tsp));
    }

    /** This is only useful for debug purposes */
    SkDEBUGCODE(GrSingleOwner* singleOwner() const { return this->context()->singleOwner(); } )

protected:
    explicit GrImageContextPriv(GrImageContext* iContext) : GrBaseContextPriv(iContext) {}
    // Required until C++17 copy elision
    GrImageContextPriv(const GrImageContextPriv&) = default;

private:
    GrImageContextPriv& operator=(const GrImageContextPriv&) = delete;

    // No taking addresses of this type.
    const GrImageContextPriv* operator&() const;
    GrImageContextPriv* operator&();

    friend class GrImageContext; // to construct/copy this type.

    using INHERITED = GrBaseContextPriv;
};

inline GrImageContextPriv GrImageContext::priv() { return GrImageContextPriv(this); }

inline const GrImageContextPriv GrImageContext::priv () const {  // NOLINT(readability-const-return-type)
    return GrImageContextPriv(const_cast<GrImageContext*>(this));
}

#endif
