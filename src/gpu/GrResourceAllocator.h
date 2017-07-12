/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrResourceAllocator_DEFINED
#define GrResourceAllocator_DEFINED

#include "SkTDynamicHash.h"

class GrResourceAllocator {
public:
    GrResourceAllocator() : m_numOps(0), m_IntvlListHead(nullptr) { }

    void incOps() { m_numOps++; }
    unsigned int numOps() const { return m_numOps; }

    void addInterval(unsigned int proxyID, unsigned int start, unsigned int end);

private:
    class Interval {
    public:
        Interval(unsigned int proxyID, unsigned int start, unsigned int end)
            : fProxyID(proxyID)
            , fStart(start)
            , fEnd(end)
            , fNext(nullptr) {
        }

        // for SkTDynamicHash
        static const unsigned int& GetKey(const Interval& intvl) { return intvl.fProxyID; }
        static uint32_t Hash(const unsigned int& key) { return key; }

        unsigned int fProxyID;
        unsigned int fStart;
        unsigned int fEnd;
        Interval*    fNext;
    };

    SkTDynamicHash<Interval, unsigned int> m_Hash;

    // All the intervals sorted by increasing start
    Interval* m_IntvlListHead;
    unsigned int m_numOps;
};

#endif // GrResourceAllocator_DEFINED
