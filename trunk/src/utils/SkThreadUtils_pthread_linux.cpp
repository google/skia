/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE //for pthread_setaffinity_np
#endif

#include "SkThreadUtils.h"
#include "SkThreadUtils_pthread.h"

#include <pthread.h>

static int nth_set_cpu(unsigned int n, cpu_set_t* cpuSet) {
    n %= CPU_COUNT(cpuSet);
    for (unsigned int setCpusSeen = 0, currentCpu = 0; true; ++currentCpu) {
        if (CPU_ISSET(currentCpu, cpuSet)) {
            ++setCpusSeen;
            if (setCpusSeen > n) {
                return currentCpu;
            }
        }
    }
}

bool SkThread::setProcessorAffinity(unsigned int processor) {
    SkThread_PThreadData* pthreadData = static_cast<SkThread_PThreadData*>(fData);
    if (!pthreadData->fValidPThread) {
        return false;
    }

    cpu_set_t parentCpuset;
    if (0 != pthread_getaffinity_np(pthread_self(), sizeof(cpu_set_t), &parentCpuset)) {
        return false;
    }

    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(nth_set_cpu(processor, &parentCpuset), &cpuset);
    return 0 == pthread_setaffinity_np(pthreadData->fPThread,
                                       sizeof(cpu_set_t),
                                       &cpuset);
}
