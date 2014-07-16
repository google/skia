/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkTHASHCACHE_DEFINED
#define	SkTHASHCACHE_DEFINED

#include "SkTypes.h"
#include "SkTDynamicHash.h"

template <typename T,
typename Key,
typename Traits = T,
int kGrowPercent = 75 >
class SkTHashCache : public SkNoncopyable {
public:

    SkTHashCache() {
        this->reset();
    }

    ~SkTHashCache() {
        this->clear();
    }

    T* find(const Key& key) const {
        return fDict->find(key);
    }

    /**
     *  If element already in cache, return immediately the cached value
     */
    T& add(const T& add) {
        Key key = Traits::GetKey(add);
        if (T* val = this->find(key)) {
            return *val;
        }

        T* element = SkNEW_ARGS(T, (add));

        fDict->add(element);

        return *element;
    }

    int size() const {
        return fDict->count();
    }

    void reset() {
        this->clear();

        fDict.reset(SkNEW(DictType));
    }

private:
    typedef SkTDynamicHash<T, Key, Traits, kGrowPercent> DictType;

    void clear() {
        if (fDict.get()) {
            typename DictType::Iter it(fDict.get());

            while (!it.done()) {
                SkDELETE(&(*it));
                ++it;
            }
        }
    }

    SkAutoTDelete<DictType> fDict;
};

#endif	/* SkHASHCACHE_DEFINED */

