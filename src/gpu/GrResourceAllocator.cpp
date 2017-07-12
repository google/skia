/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrResourceAllocator.h"

void GrResourceAllocator::addInterval(unsigned int proxyID, unsigned int start, unsigned int end) {
    SkASSERT(start <= end);

    if (Interval* intvl = m_Hash.find(proxyID)) {
        SkASSERT(intvl->fEnd < start);
        SkDebugf("revising interval for %d from [%d, %d] to [%d, %d]\n",
                 proxyID,
                 intvl->fStart, intvl->fEnd,
                 intvl->fStart, end);
        intvl->fEnd = end;
        return;
    }

    SkDebugf("adding new interval for %d: [ %d, %d ]\n", proxyID, start, end);
    Interval* newIntvl = new Interval(proxyID, start, end);

    if (!m_IntvlListHead) {
        m_IntvlListHead = newIntvl;
    } else if (start <= m_IntvlListHead->fStart) {
        newIntvl->fNext = m_IntvlListHead;
        m_IntvlListHead = newIntvl;
    } else {
        Interval* prev = m_IntvlListHead;
        Interval* next = prev->fNext;
        for (; next && start > next->fStart; prev = next, next = next->fNext) {
        }
        newIntvl->fNext = next;
        prev->fNext = newIntvl;
    }

    m_Hash.add(newIntvl);
}
