
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */



#ifndef SkTLazy_DEFINED
#define SkTLazy_DEFINED

#include "SkTypes.h"
#include <new>

/**
 *  Efficient way to defer allocating/initializing a class until it is needed
 *  (if ever).
 */
template <typename T> class SkTLazy {
public:
    SkTLazy() : fPtr(NULL) {}

    explicit SkTLazy(const T* src) : fPtr(NULL) {
        if (src) {
            fPtr = new (fStorage) T(*src);
        }
    }

    SkTLazy(const SkTLazy<T>& src) : fPtr(NULL) {
        if (src.isValid()) {
            fPtr = new (fStorage) T(*src->get());
        } else {
            fPtr = NULL;
        }
    }

    ~SkTLazy() {
        if (this->isValid()) {
            fPtr->~T();
        }
    }

    /**
     *  Return a pointer to a default-initialized instance of the class. If a
     *  previous instance had been initialzied (either from init() or set()) it
     *  will first be destroyed, so that a freshly initialized instance is
     *  always returned.
     */
    T* init() {
        if (this->isValid()) {
            fPtr->~T();
        }
        fPtr = new (SkTCast<T*>(fStorage)) T;
        return fPtr;
    }

    /**
     *  Copy src into this, and return a pointer to a copy of it. Note this
     *  will always return the same pointer, so if it is called on a lazy that
     *  has already been initialized, then this will copy over the previous
     *  contents.
     */
    T* set(const T& src) {
        if (this->isValid()) {
            *fPtr = src;
        } else {
            fPtr = new (SkTCast<T*>(fStorage)) T(src);
        }
        return fPtr;
    }

    /**
     *  Returns true if a valid object has been initialized in the SkTLazy,
     *  false otherwise.
     */
    bool isValid() const { return NULL != fPtr; }
    
    /**
     *  Returns either NULL, or a copy of the object that was passed to
     *  set() or the constructor.
     */
    T* get() const { SkASSERT(this->isValid()); return fPtr; }
    
private:
    T*   fPtr; // NULL or fStorage
    char fStorage[sizeof(T)];
};

#endif

