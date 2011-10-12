
/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */



#ifndef GrInstanceCounter_DEFINED
#define GrInstanceCounter_DEFINED

#include "GrTypes.h"

template <typename T> class GrInstanceCounter {
public:
    GrInstanceCounter() {
        ++gCounter;
        GrPrintf("+ %s %d\n", T::InstanceCounterClassName(), gCounter);
    }

    ~GrInstanceCounter() {
        --gCounter;
        GrPrintf("- %s %d\n", T::InstanceCounterClassName(), gCounter);
    }

private:
    static int gCounter;
};

template <typename T> int GrInstanceCounter<T>::gCounter;

#define DECLARE_INSTANCE_COUNTER(T)                                 \
    static const char* InstanceCounterClassName() { return #T; }    \
    friend class GrInstanceCounter<T>;                              \
    GrInstanceCounter<T> fInstanceCounter

#endif

