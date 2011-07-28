
/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkTRelay_DEFINED
#define SkTRelay_DEFINED

#include "SkRefCnt.h"

/**
 *  Similar to a weakptr in java, a Relay allows for a back-ptr to an
 *  object to be "safe", without using a hard reference-count.
 *
 *  Typically, the target creates a Relay with a pointer to itself. Whenever it
 *  wants to have another object maintain a safe-ptr to it, it gives them a
 *  Relay, which they ref()/unref(). Through the Relay each external object can
 *  retrieve a pointer to the Target. However, when the Target goes away, it
 *  clears the Relay pointer to it (relay->set(NULL)) and then unref()s the
 *  Relay. The other objects still have a ref on the Relay, but now when they
 *  call get() the receive a NULL.
 */
template <template T> class SkTRelay : public SkRefCnt {
public:
    SkTRelay(T* ptr) : fPtr(ptr) {}

    // consumers call this
    T* get() const { return fPtr; }

    // producer calls this
    void set(T* ptr) { fPtr = ptr; }

    void clear() { this->set(NULL); }

private:
    T* fPtr;
};

#endif
