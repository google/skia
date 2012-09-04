/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkTDLinkedList_DEFINED
#define SkTDLinkedList_DEFINED

#include "SkTypes.h"

/**
 * Helper class to automatically initialize the doubly linked list
 * created pointers.
 */
template <typename T> class SkPtrWrapper {
  public:
      SkPtrWrapper() : fPtr(NULL) {}
      SkPtrWrapper& operator =(T* ptr) { fPtr = ptr; return *this; }
      operator T*() const { return fPtr; }
      T* operator->() { return fPtr; }
  private:
      T* fPtr;
};


/**
 * This macro creates the member variables required by
 * the SkTDLinkedList class. It should be placed in the private section
 * of any class that will be stored in a double linked list.
 */
#define SK_DEFINE_DLINKEDLIST_INTERFACE(ClassName)              \
    friend class SkTDLinkedList<ClassName>;                     \
    /* back pointer to the owning list - for debugging */       \
    SkDEBUGCODE(SkPtrWrapper<SkTDLinkedList<ClassName> > fList;)\
    SkPtrWrapper<ClassName> fPrev;                              \
    SkPtrWrapper<ClassName> fNext;

/**
 * This class implements a templated internal doubly linked list data structure.
 */
template <class T> class SkTDLinkedList : public SkNoncopyable {
public:
    SkTDLinkedList()
        : fHead(NULL)
        , fTail(NULL) {
    }

    void remove(T* entry) {
        SkASSERT(NULL != fHead && NULL != fTail);
        SkASSERT(this->isInList(entry));

        T* prev = entry->fPrev;
        T* next = entry->fNext;

        if (NULL != prev) {
            prev->fNext = next;
        } else {
            fHead = next;
        }
        if (NULL != next) {
            next->fPrev = prev;
        } else {
            fTail = prev;
        }

        entry->fPrev = NULL;
        entry->fNext = NULL;

#ifdef SK_DEBUG
        entry->fList = NULL;
#endif
    }

    void addToHead(T* entry) {
        SkASSERT(NULL == entry->fPrev && NULL == entry->fNext);
        SkASSERT(NULL == entry->fList);

        entry->fPrev = NULL;
        entry->fNext = fHead;
        if (NULL != fHead) {
            fHead->fPrev = entry;
        }
        fHead = entry;
        if (NULL == fTail) {
            fTail = entry;
        }

#ifdef SK_DEBUG
        entry->fList = this;
#endif
    }

    bool isEmpty() const {
        return NULL == fHead && NULL == fTail;
    }

    T* head() { return fHead; }
    T* tail() { return fTail; }

    class Iter {
    public:
        enum IterStart {
            kHead_IterStart,
            kTail_IterStart
        };

        Iter() : fCur(NULL) {}

        T* init(SkTDLinkedList& list, IterStart startLoc) {
            if (kHead_IterStart == startLoc) {
                fCur = list.fHead;
            } else {
                SkASSERT(kTail_IterStart == startLoc);
                fCur = list.fTail;
            }

            return fCur;
        }

        /**
         * Return the next/previous element in the list or NULL if at the end.
         */
        T* next() {
            if (NULL == fCur) {
                return NULL;
            }

            fCur = fCur->fNext;
            return fCur;
        }

        T* prev() {
            if (NULL == fCur) {
                return NULL;
            }

            fCur = fCur->fPrev;
            return fCur;
        }

    private:
        T* fCur;
    };

#ifdef SK_DEBUG
    void validate() const {
        GrAssert(!fHead == !fTail);
    }

    /**
     * Debugging-only method that uses the list back pointer to check if
     * 'entry' is indeed in 'this' list.
     */
    bool isInList(const T* entry) const {
        return entry->fList == this;
    }

    /**
     * Debugging-only method that laboriously counts the list entries.
     */
    int countEntries() const {
        int count = 0;
        for (T* entry = fHead; NULL != entry; entry = entry->fNext) {
            ++count;
        }
        return count;
    }
#endif // SK_DEBUG

private:
    T* fHead;
    T* fTail;

    typedef SkNoncopyable INHERITED;
};

#endif // SkTDLinkedList_DEFINED
