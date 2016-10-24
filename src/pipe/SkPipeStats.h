/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPipeStats_DEFINED
#define SkPipeStats_DEFINED

#include "SkPipeFormat.h"

struct SkPipeStat {
    int     fCount;
    int64_t fBytes;
};

struct SkPipeStats {
    enum {
        N = static_cast<int>(SkPipeVerb::kLastEnum) + 1
    };
    SkPipeStat fStats[N];

    SkPipeStats() {
        sk_bzero(fStats, sizeof(fStats));
    }

    void record(SkPipeVerb verb, size_t bytes) {
        fStats[static_cast<int>(verb)].fCount += 1;
        fStats[static_cast<int>(verb)].fBytes += bytes;
    }

    void dump() const;
};

#endif
