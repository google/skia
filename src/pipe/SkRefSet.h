/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkRefSet_DEFINED
#define SkRefSet_DEFINED

#include "SkRefCnt.h"
#include "SkTArray.h"

template <typename T> class SkRefSet {
public:
    T* get(int index) const {
        SkASSERT((unsigned)index < (unsigned)fArray.count());
        return fArray[index].get();
    }

    bool set(int index, sk_sp<T> value) {
        if (index < fArray.count()) {
            fArray[index] = std::move(value);
            return true;
        }
        if (fArray.count() == index && value) {
            fArray.emplace_back(std::move(value));
            return true;
        }
        SkDebugf("SkRefSet: index [%d] out of range %d\n", index, fArray.count());
        return false;
    }

private:
    SkTArray<sk_sp<T>, true> fArray;
};

#endif
