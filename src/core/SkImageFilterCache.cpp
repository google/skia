/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkImageFilterCache.h"

#include "SkChecksum.h"
#include "SkMutex.h"
#include "SkOnce.h"
#include "SkRefCnt.h"
#include "SkSpecialImage.h"
#include "SkTDynamicHash.h"
#include "SkTInternalLList.h"

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
        SkTDynamicHash<Value, Key>::Iter iter(&fLookup);

        while (!iter.done()) {
            Value* v = &*iter;
            ++iter;
            delete v;
        }
    }
    struct Value {
        Value(const Key& key, SkSpecialImage* image, const SkIPoint& offset)
            : fKey(key), fImage(SkRef(image)), fOffset(offset) {}

        Key fKey;
        SkAutoTUnref<SkSpecialImage> fImage;
        SkIPoint fOffset;
        static const Key& GetKey(const Value& v) {
            return v.fKey;
        }
        static uint32_t Hash(const Key& key) {
            return SkChecksum::Murmur3(reinterpret_cast<const uint32_t*>(&key), sizeof(Key));
        }
        SK_DECLARE_INTERNAL_LLIST_INTERFACE(Value);
    };

    SkSpecialImage* get(const Key& key, SkIPoint* offset) const override {
        SkAutoMutexAcquire mutex(fMutex);
        if (Value* v = fLookup.find(key)) {
            *offset = v->fOffset;
            if (v != fLRU.head()) {
                fLRU.remove(v);
                fLRU.addToHead(v);
            }
            return v->fImage;
        }
        return nullptr;
    }

    void set(const Key& key, SkSpecialImage* image, const SkIPoint& offset) override {
        SkAutoMutexAcquire mutex(fMutex);
        if (Value* v = fLookup.find(key)) {
            this->removeInternal(v);
        }
        Value* v = new Value(key, image, offset);
        fLookup.add(v);
        fLRU.addToHead(v);
        fCurrentBytes += image->getSize();
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
        SkAutoMutexAcquire mutex(fMutex);
        while (fCurrentBytes > 0) {
            Value* tail = fLRU.tail();
            SkASSERT(tail);
            this->removeInternal(tail);
        }
    }

    void purgeByKeys(const Key keys[], int count) override {
        SkAutoMutexAcquire mutex(fMutex);
        for (int i = 0; i < count; i++) {
            if (Value* v = fLookup.find(keys[i])) {
                this->removeInternal(v);
            }
        }
    }

    SkDEBUGCODE(int count() const override { return fLookup.count(); })
private:
    void removeInternal(Value* v) {
        SkASSERT(v->fImage);
        fCurrentBytes -= v->fImage->getSize();
        fLRU.remove(v);
        fLookup.remove(v->fKey);
        delete v;
    }
private:
    SkTDynamicHash<Value, Key>            fLookup;
    mutable SkTInternalLList<Value>       fLRU;
    size_t                                fMaxBytes;
    size_t                                fCurrentBytes;
    mutable SkMutex                       fMutex;
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
