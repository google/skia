/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrImageContext_DEFINED
#define GrImageContext_DEFINED

#include "GrContext_Base.h"

class GrImageContextPriv;

class SK_API GrImageContext : public GrContext_Base {
public:
    ~GrImageContext() override;

    // Provides access to functions that aren't part of the public API.
    GrImageContextPriv priv();
    const GrImageContextPriv priv() const;

protected:
    friend class GrImageContextPriv; // for hidden functions

    GrImageContext(GrBackendApi backend, uint32_t uniqueID);

    GrImageContext* asImageContext() override { return this; }

private:
    typedef GrContext_Base INHERITED;
};

#endif
