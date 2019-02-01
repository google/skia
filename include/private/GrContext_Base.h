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

    /*
     * The 3D API backing this context
     */
    GrBackendApi backend() const { return fBackend; }

    // Provides access to functions that aren't part of the public API.
    GrBaseContextPriv priv();
    const GrBaseContextPriv priv() const;

protected:
    friend class GrBaseContextPriv; // for hidden functions

    GrContext_Base(GrBackendApi backend, uint32_t uniqueID);

    /**
     * An identifier for this context. The id is used by all compatible contexts. For example,
     * if SkImages are created on one thread using an image creation context, then fed into a
     * DDL Recorder on second thread (which has a recording context) and finally replayed on
     * a third thread with a direct context, then all three contexts will report the same id.
     * It is an error for an image to be used with contexts that report different ids.
     */
    uint32_t contextID() const { return fContextID; }

    GrContext_Base* asBaseContext() { return this; }
    virtual GrImageContext* asImageContext() { return nullptr; }
    virtual GrRecordingContext* asRecordingContext() { return nullptr; }
    virtual GrContext* asDirectContext() { return nullptr; }

private:
    const GrBackendApi fBackend;
    const uint32_t     fContextID;

    typedef SkRefCnt INHERITED;
};

#endif
