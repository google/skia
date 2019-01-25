/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrContext_Base_DEFINED
#define GrContext_Base_DEFINED

#include "SkRefCnt.h"
#include "GrTypes.h"

class GrBaseContextPriv;
class GrContext;
class GrImageContext;
class GrRecordingContext;

class SK_API GrContext_Base : public SkRefCnt {
public:
    virtual ~GrContext_Base();

    /**
     * An ID associated with this context, guaranteed to be unique.
     */
    uint32_t uniqueID() const { return fUniqueID; }

    // Provides access to functions that aren't part of the public API.
    GrBaseContextPriv priv();
    const GrBaseContextPriv priv() const;

protected:
    friend class GrBaseContextPriv; // for hidden functions

    /*
     * The 3D API backing this context
     */
    GrBackendApi backend() const { return fBackend; }

    GrContext_Base* asBaseContext() { return this; }
    virtual GrImageContext* asImageContext() { return nullptr; }
    virtual GrRecordingContext* asRecordingContext() { return nullptr; }
    virtual GrContext* asDirectContext() { return nullptr; }

private:
    friend class GrImageContext;           // for ctor
    friend class GrContextThreadSafeProxy; // for ctor

    GrContext_Base(GrBackendApi backend, uint32_t uniqueID);

    const GrBackendApi fBackend;
    const uint32_t     fUniqueID;

    typedef SkRefCnt INHERITED;
};

#endif
