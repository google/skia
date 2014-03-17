/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkTInternalSList_DEFINED
#define SkTInternalSList_DEFINED

#include "SkTInternalLList.h" // for SkPtrWrapper

/**
 * This macro creates the methods required by the SkTInternalSList class.
 * It should be instantiated in the private block of the class you want to put
 * into an SkTInternalSList.
 * For most use cases you should use SK_DECLARE_INTERNAL_SLIST_INTERFACE and not
 * this macro. If you care about the field name, or want to re-use an existing
 * field, then you can use this macro to declare the methods pointing to a
 * specific field.
 * Unlike SK_DECLARE_INTERNAL_SLIST_INTERFACE this does not declare the field
 * itself.
 * It also makes SkTInternalSList<ClassName> a friend to give it access to the
 * methods.
 */
#define SK_DECLARE_INTERNAL_SLIST_ADAPTER(ClassName, field)     \
    ClassName* getSListNext() {                                 \
        return this->field;                                     \
    }                                                           \
    void setSListNext(ClassName* next) {                        \
        this->field = next;                                     \
    }                                                           \
    friend class SkTInternalSList<ClassName>

/**
 * This macro declares an fSListNext that auto initializes to NULL and then
 * uses SK_DECLARE_INTERNAL_SLIST_ADAPTER to add the methods needed by
 * SkTInternalSList.
 * It should be instantiated in the private block of the class you want to put
 * into an SkTInternalSList.
 */
#define SK_DECLARE_INTERNAL_SLIST_INTERFACE(ClassName)          \
    SK_DECLARE_INTERNAL_SLIST_ADAPTER(ClassName, fSListNext);   \
    SkPtrWrapper<ClassName> fSListNext

/**
 * An implementation of an intrusive singly linked list.
 * The type T must have a methods getSListNext and setSListNext that are visible
 * to the list. The easiest way to do this is with
 * SK_DECLARE_INTERNAL_SLIST_INTERFACE.
 * The list does not maintain ownership of any of its elements, or ever delete
 * them.
 */
template<typename T> class SkTInternalSList {
public:
    SkTInternalSList() : fHead(NULL), fCount(0) {}

    /**
     * Push an item onto the head of the list.
     * This method is *not* thread safe.
     */
    void push(T* entry) {
        SkASSERT(entry->getSListNext() == NULL);
        entry->setSListNext(fHead);
        fHead = entry;
        ++fCount;
    }

    /**
     * Takes all the items from another list and pushes them into this list.
     * No ordering guarantees are made, the other list will be emptied.
     * This method is *not* thread safe.
     */
    void pushAll(SkTInternalSList<T>* other) {
        if (this->isEmpty()) {
            this->swap(other);
            return;
        }
        while (!other->isEmpty()) {
            this->push(other->pop());
        }
    }

    /**
     * Pop an item from the head of the list.
     * Returns NULL if the list is empty.
     * This method is *not* thread safe.
     */
    T* pop() {
        if (NULL == fHead) {
            return NULL;
        }
        T* result = fHead;
        fHead = result->getSListNext();
        result->setSListNext(NULL);
        --fCount;
        return result;
    }

    T* head() const {
        return fHead;
    }

    /**
     * Returns true if the list has no elements.
     */
    bool isEmpty() const {
        return NULL == fHead;
    }

    /**
     * Swaps the contents of this list with another one.
     * This method is *not* thread safe.
     */
    void swap(SkTInternalSList<T>* other) {
        SkTSwap(fHead, other->fHead);
        SkTSwap(fCount, other->fCount);
    }

    /**
     * Returns the count of elements in the list.
     */
    int getCount() const {
        return fCount;
    }
private:
    T* fHead;
    int fCount;
};


#endif
