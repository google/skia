/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkImageFilterCache.h"

#include <vector>

#include "include/core/SkImageFilter.h"
#include "include/core/SkRefCnt.h"
#include "include/private/base/SkMutex.h"
#include "include/private/base/SkOnce.h"
#include "src/base/SkTInternalLList.h"
#include "src/core/SkOpts.h"
#include "src/core/SkSpecialImage.h"
#include "src/core/SkTDynamicHash.h"
#include "src/core/SkTHash.h"

#ifdef SK_BUILD_FOR_IOS
  enum { kDefaultCacheSize = 2 * 1024 * 1024 };
#else
  enum { kDefaultCacheSize = 128 * 1024 * 1024 };
#endif

namespace {

class CacheImpl : public SkImageFilterCache {
public:
    typedef SkImageFilterCacheKey Key;
    CacheImpl(size_t maxBytes) : fMaxBytes(maxBytes), fCurrentBytes(0) { }
    ~CacheImpl() override {
        fLookup.foreach([&](Value* v) { delete v; });
    }
    struct Value {
        Value(const Key& key, const skif::FilterResult& image,
              const SkImageFilter* filter)
            : fKey(key), fImage(image), fFilter(filter) {}

        Key fKey;
        skif::FilterResult fImage;
        const SkImageFilter* fFilter;
        static const Key& GetKey(const Value& v) {
            return v.fKey;
        }
        static uint32_t Hash(const Key& key) {
            return SkOpts::hash(reinterpret_cast<const uint32_t*>(&key), sizeof(Key));
        }
        SK_DECLARE_INTERNAL_LLIST_INTERFACE(Value);
    };

    bool get(const Key& key, skif::FilterResult* result) const override {
        SkASSERT(result);

        SkAutoMutexExclusive mutex(fMutex);
        if (Value* v = fLookup.find(key)) {
            if (v != fLRU.head()) {
                fLRU.remove(v);
                fLRU.addToHead(v);
            }

            *result = v->fImage;
            return true;
        }
        return false;
    }

    void set(const Key& key, const SkImageFilter* filter,
             const skif::FilterResult& result) override {
        SkAutoMutexExclusive mutex(fMutex);
        if (Value* v = fLookup.find(key)) {
            this->removeInternal(v);
        }
        Value* v = new Value(key, result, filter);
        fLookup.add(v);
        fLRU.addToHead(v);
        fCurrentBytes += result.image() ? result.image()->getSize() : 0;
        if (auto* values = fImageFilterValues.find(filter)) {
            values->push_back(v);
        } else {
            fImageFilterValues.set(filter, {v});
        }

        while (fCurrentBytes > fMaxBytes) {
            Value* tail = fLRU.tail();
            SkASSERT(tail);
            if (tail == v) {
                break;
            }
            this->removeInternal(tail);
        }
    }

    void purge() override {
        SkAutoMutexExclusive mutex(fMutex);
        while (fCurrentBytes > 0) {
            Value* tail = fLRU.tail();
            SkASSERT(tail);
            this->removeInternal(tail);
        }
    }

    void purgeByImageFilter(const SkImageFilter* filter) override {
        SkAutoMutexExclusive mutex(fMutex);
        auto* values = fImageFilterValues.find(filter);
        if (!values) {
            return;
        }
        for (Value* v : *values) {
            // We set the filter to be null so that removeInternal() won't delete from values while
            // we're iterating over it.
            v->fFilter = nullptr;
            this->removeInternal(v);
        }
        fImageFilterValues.remove(filter);
    }

    SkDEBUGCODE(int count() const override { return fLookup.count(); })
private:
    void removeInternal(Value* v) {
        if (v->fFilter) {
            if (auto* values = fImageFilterValues.find(v->fFilter)) {
                if (values->size() == 1 && (*values)[0] == v) {
                    fImageFilterValues.remove(v->fFilter);
                } else {
                    for (auto it = values->begin(); it != values->end(); ++it) {
                        if (*it == v) {
                            values->erase(it);
                            break;
                        }
                    }
                }
            }
        }
        fCurrentBytes -= v->fImage.image() ? v->fImage.image()->getSize() : 0;
        fLRU.remove(v);
        fLookup.remove(v->fKey);
        delete v;
    }
private:
    SkTDynamicHash<Value, Key>                            fLookup;
    mutable SkTInternalLList<Value>                       fLRU;
    // Value* always points to an item in fLookup.
    SkTHashMap<const SkImageFilter*, std::vector<Value*>> fImageFilterValues;
    size_t                                                fMaxBytes;
    size_t                                                fCurrentBytes;
    mutable SkMutex                                       fMutex;
};

} // namespace

SkImageFilterCache* SkImageFilterCache::Create(size_t maxBytes) {
    return new CacheImpl(maxBytes);
}

SkImageFilterCache* SkImageFilterCache::Get() {
    static SkOnce once;
    static SkImageFilterCache* cache;

    once([]{ cache = SkImageFilterCache::Create(kDefaultCacheSize); });
    return cache;
}
