/*
 * Copyright 2018 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrSkSLFPFactoryCache_DEFINED
#define GrSkSLFPFactoryCache_DEFINED

#include <vector>
#include "include/core/SkRefCnt.h"

class GrShaderCaps;
class GrSkSLFPFactory;

// This is a cache used by GrSkSLFP to retain GrSkSLFPFactory instances, so we don't have to
// re-process the SkSL source code every time we create a GrSkSLFP instance.
// For thread safety, it is important that GrSkSLFP only interact with the cache from methods that
// are only called from within the rendering thread, like onCreateGLSLInstance and
// onGetGLSLProcessorKey.
class GrSkSLFPFactoryCache : public SkNVRefCnt<GrSkSLFPFactoryCache> {
public:
    GrSkSLFPFactoryCache(sk_sp<const GrShaderCaps> shaderCaps) : fShaderCaps(shaderCaps) {}
    ~GrSkSLFPFactoryCache();

    sk_sp<GrSkSLFPFactory> findOrCreate(int index, const char* name, const char* skSL);

private:
    // Returns a factory by its numeric index, or null if no such factory exists. Indices are
    // allocated by GrSkSLFP::NewIndex().
    sk_sp<GrSkSLFPFactory> get(int index);

    // Stores a new factory with the given index.
    void set(int index, sk_sp<GrSkSLFPFactory> factory);

    mutable SkMutex fCacheMutex;

    std::vector<sk_sp<GrSkSLFPFactory>> fFactories;
    sk_sp<const GrShaderCaps> fShaderCaps;
};

#endif
