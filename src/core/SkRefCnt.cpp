/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkRefCnt.h"

SkRefCntBase::~SkRefCntBase() {
#ifdef SK_DEBUG
    SkASSERTF(this->getRefCnt() == 1, "fRefCnt was %d", this->getRefCnt());
    // illegal value, to catch us if we reuse after delete
    fRefCnt.store(0, std::memory_order_relaxed);
#endif
}

void SkRefCntBase::unref() const {
    SkASSERT(this->getRefCnt() > 0);
    // A release here acts in place of all releases we "should" have been doing in ref().
    if (1 == fRefCnt.fetch_add(-1, std::memory_order_acq_rel)) {
        // Like unique(), the acquire is only needed on success, to make sure
        // code in internal_dispose() doesn't happen before the decrement.
        this->internal_dispose();
    }
}

void SkRefCntBase::internal_dispose() const {
#ifdef SK_DEBUG
    SkASSERT(0 == this->getRefCnt());
    fRefCnt.store(1, std::memory_order_relaxed);
#endif
    delete this;
}
