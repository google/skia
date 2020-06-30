/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrDirectContext_DEFINED
#define GrDirectContext_DEFINED

#include "include/gpu/GrContext.h"

class GrAtlasManager;

class GrDirectContext : public GrContext {
public:
    GrDirectContext(GrBackendApi backend, const GrContextOptions& options);

    ~GrDirectContext() override;

    void abandonContext() override;

    void releaseResourcesAndAbandonContext() override;

    void freeGpuResources() override;

protected:
    bool init() override;

    GrAtlasManager* onGetAtlasManager() override { return fAtlasManager; }

    GrDirectContext* asDirectContext() override { return this; }

private:
    GrAtlasManager* fAtlasManager;

    typedef GrContext INHERITED;
};


#endif
