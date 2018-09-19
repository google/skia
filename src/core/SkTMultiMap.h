/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkTMultiMap_DEFINED
#define SkTMultiMap_DEFINED

#include "include/gpu/GrTypes.h"
#include "src/core/SkTDynamicHash.h"

/** A set that contains pointers to instances of T. Instances can be looked up with key Key.
 * Multiple (possibly same) values can have the same key.
 */
template <typename T,
          typename Key,
          typename HashTraits=T>
class SkTMultiMap {
    struct ValueList {
        explicit ValueList(T* value) : fValue(value), fNext(nullptr) {}

        static const Key& GetKey(const ValueList& e) { return HashTraits::GetKey(*e.fValue); }
        static uint32_t Hash(const Key& key) { return HashTraits::Hash(key); }
        T* fValue;
        ValueList* fNext;
    };
public:
    SkTMultiMap() : fCount(0) {}

    ~SkTMultiMap() {
        typename SkTDynamicHash<ValueList, Key>::Iter iter(&fHash);
        for ( ; !iter.done(); ++iter) {
            ValueList* next;
            for (ValueList* cur = &(*iter); cur; cur = next) {
                HashTraits::OnFree(cur->fValue);
                next = cur->fNext;
                delete cur;
            }
        }
    }

    void insert(const Key& key, T* value) {
        ValueList* list = fHash.find(key);
        if (list) {
            // The new ValueList entry is inserted as the second element in the
            // linked list, and it will contain the value of the first element.
            ValueList* newEntry = new ValueList(list->fValue);
            newEntry->fNext = list->fNext;
            // The existing first ValueList entry is updated to contain the
            // inserted value.
            list->fNext = newEntry;
            list->fValue = value;
        } else {
            fHash.add(new ValueList(value));
        }

        ++fCount;
    }

    void remove(const Key& key, const T* value) {
        ValueList* list = fHash.find(key);
        // Temporarily making this safe for remove entries not in the map because of
        // crbug.com/877915.
#if 0
        // Since we expect the caller to be fully aware of what is stored, just
        // assert that the caller removes an existing value.
        SkASSERT(list);
        ValueList* prev = nullptr;
        while (list->fValue != value) {
            prev = list;
            list = list->fNext;
        }
        this->internalRemove(prev, list, key);
#else
        ValueList* prev = nullptr;
        while (list && list->fValue != value) {
            prev = list;
            list = list->fNext;
        }
        // Crash in Debug since it'd be great to detect a repro of 877915.
        SkASSERT(list);
        if (list) {
            this->internalRemove(prev, list, key);
        }
#endif
    }

    T* find(const Key& key) const {
        ValueList* list = fHash.find(key);
        if (list) {
            return list->fValue;
        }
        return nullptr;
    }

    template<class FindPredicate>
    T* find(const Key& key, const FindPredicate f) {
        ValueList* list = fHash.find(key);
        while (list) {
            if (f(list->fValue)){
                return list->fValue;
            }
            list = list->fNext;
        }
        return nullptr;
    }

    template<class FindPredicate>
    T* findAndRemove(const Key& key, const FindPredicate f) {
        ValueList* list = fHash.find(key);

        ValueList* prev = nullptr;
        while (list) {
            if (f(list->fValue)){
                T* value = list->fValue;
                this->internalRemove(prev, list, key);
                return value;
            }
            prev = list;
            list = list->fNext;
        }
        return nullptr;
    }

    int count() const { return fCount; }

#ifdef SK_DEBUG
    class ConstIter {
    public:
        explicit ConstIter(const SkTMultiMap* mmap)
            : fIter(&(mmap->fHash))
            , fList(nullptr) {
            if (!fIter.done()) {
                fList = &(*fIter);
            }
        }

        bool done() const {
            return fIter.done();
        }

        const T* operator*() {
            SkASSERT(fList);
            return fList->fValue;
        }

        void operator++() {
            if (fList) {
                fList = fList->fNext;
            }
            if (!fList) {
                ++fIter;
                if (!fIter.done()) {
                    fList = &(*fIter);
                }
            }
        }

    private:
        typename SkTDynamicHash<ValueList, Key>::ConstIter fIter;
        const ValueList* fList;
    };

    bool has(const T* value, const Key& key) const {
        for (ValueList* list = fHash.find(key); list; list = list->fNext) {
            if (list->fValue == value) {
                return true;
            }
        }
        return false;
    }

    // This is not particularly fast and only used for validation, so debug only.
    int countForKey(const Key& key) const {
        int count = 0;
        ValueList* list = fHash.find(key);
        while (list) {
            list = list->fNext;
            ++count;
        }
        return count;
    }
#endif

private:
    SkTDynamicHash<ValueList, Key> fHash;
    int fCount;

    void internalRemove(ValueList* prev, ValueList* elem, const Key& key) {
        if (elem->fNext) {
            ValueList* next = elem->fNext;
            elem->fValue = next->fValue;
            elem->fNext = next->fNext;
            delete next;
        } else if (prev) {
            prev->fNext = nullptr;
            delete elem;
        } else {
            fHash.remove(key);
            delete elem;
        }

        --fCount;
    }

};

#endif
