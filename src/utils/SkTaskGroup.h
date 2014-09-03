/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkTaskGroup_DEFINED
#define SkTaskGroup_DEFINED

#include "SkTypes.h"
#include "SkRunnable.h"

class SkTaskGroup : SkNoncopyable {
public:
    // Call before creating any SkTaskGroup to set the number of threads all SkTaskGroups share.
    // If not called, we default to the number of system-reported cores.
    static void SetThreadCount(int);

    SkTaskGroup();
    ~SkTaskGroup() { this->wait(); }

    // Add a task to this SkTaskGroup.  It will likely run() on another thread.
    void add(SkRunnable*);

    // Block until all Tasks previously add()ed to this SkTaskGroup have run().
    // You may safely reuse this SkTaskGroup after wait() returns.
    void wait();

private:
    /*atomic*/ int32_t fPending;
};

#endif//SkTaskGroup_DEFINED
