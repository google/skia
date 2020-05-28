/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrContextFamily_DEFINED
#define GrContextFamily_DEFINED

#include "include/core/SkRefCnt.h"

#include <atomic>

/*
 * A container for the data shared across a context family. All API in this class is thread-safe.
 *
 * Note: Currently this only stores the abandonment state of a context family, but in the future
 * it will hold more e.g. the path mask cache.
 */
class GrContextFamily : public SkRefCnt {
public:
    void abandon() { fAbandoned.store(true, std::memory_order_release); }
    bool isAbandoned() const { return fAbandoned.load(std::memory_order_acquire); }

private:
    std::atomic<bool> fAbandoned{false};
};


#endif
