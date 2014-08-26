
/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrResourceCache2_DEFINED
#define GrResourceCache2_DEFINED

#include "GrTypes.h"
#include "SkTInternalLList.h"

class GrGpuResource;

/**
 *  Eventual replacement for GrResourceCache. Currently it simply holds a list
 *  of all GrGpuResource objects for a GrContext. It is used to invalidate all
 *  the resources when necessary.
 */
class GrResourceCache2 {
public:
    GrResourceCache2() : fCount(0) {};
    ~GrResourceCache2();

    void insertResource(GrGpuResource* resource);

    void removeResource(GrGpuResource* resource);

    void abandonAll();

    void releaseAll();

private:
#ifdef SK_DEBUG
    bool isInCache(const GrGpuResource* r) const {
        return fResources.isInList(r);
    }
#endif

    int                             fCount;
    SkTInternalLList<GrGpuResource> fResources;
};

#endif
