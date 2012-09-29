/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkThreadUtils.h"
#include "SkThreadUtils_pthread.h"

#include <mach/mach.h>
#include <mach/thread_policy.h>
#include <pthread.h>

bool SkThread::setProcessorAffinity(unsigned int processor) {
    SkThread_PThreadData* pthreadData = static_cast<SkThread_PThreadData*>(fData);
    if (!pthreadData->fValidPThread) {
        return false;
    }

    mach_port_t tid = pthread_mach_thread_np(pthreadData->fPThread);

    thread_affinity_policy_data_t policy;
    policy.affinity_tag = processor;

    return 0 == thread_policy_set(tid,
                                  THREAD_AFFINITY_POLICY,
                                  (thread_policy_t) &policy,
                                  THREAD_AFFINITY_POLICY_COUNT);
}
