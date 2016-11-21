/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrSurfaceContext_DEFINED
#define GrSurfaceContext_DEFINED

#include "GrContext.h"
#include "GrSurface.h"
#include "SkRefCnt.h"
#include "../private/GrSingleOwner.h"
#include "../private/GrSurfaceProxy.h"

/*
 * A helper object to orchestrate commands for a particular surface
 */
class SK_API GrSurfaceContext : public SkRefCnt {
public:
    ~GrSurfaceContext() override {}

    virtual bool copySurface(GrSurface* src, const SkIRect& srcRect, const SkIPoint& dstPoint) = 0;

    GrAuditTrail* auditTrail() { return fAuditTrail; }

protected:
    GrSurfaceContext(GrContext*, GrAuditTrail*, GrSingleOwner*);

    SkDEBUGCODE(GrSingleOwner* singleOwner() { return fSingleOwner; })

    GrContext*            fContext;
    GrAuditTrail*         fAuditTrail;

    // In debug builds we guard against improper thread handling
    SkDEBUGCODE(mutable GrSingleOwner* fSingleOwner;)
};

#endif
