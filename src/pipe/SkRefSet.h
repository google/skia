/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkRefSet_DEFINED
#define SkRefSet_DEFINED

#include "SkRefCnt.h"
#include "SkTDArray.h"

template <typename T> class SkRefSet {
public:
    ~SkRefSet() { fArray.unrefAll(); }

    T* get(int index) const {
        SkASSERT((unsigned)index < (unsigned)fArray.count());
        return fArray[index];
    }

    bool set(int index, T* value) {
        if ((unsigned)index < (unsigned)fArray.count()) {
            SkRefCnt_SafeAssign(fArray[index], value);
            return true;
        }
        if (fArray.count() == index && value) {
            *fArray.append() = SkRef(value);
            return true;
        }
        SkDebugf("SkRefSet: index [%d] out of range %d\n", index, fArray.count());
        return false;
    }

private:
    SkTDArray<T*> fArray;
};

#endif
