/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkTLazy_DEFINED
#define SkTLazy_DEFINED

#include "../private/SkTemplates.h"
#include "SkTypes.h"
#include <new>
#include <utility>

/**
 *  Efficient way to defer allocating/initializing a class until it is needed
 *  (if ever).
 */
template <typename T> class SkTLazy {
public:
    SkTLazy() : fPtr(nullptr) {}

    explicit SkTLazy(const T* src)
        : fPtr(src ? new (fStorage.get()) T(*src) : nullptr) {}

    SkTLazy(const SkTLazy& src) : fPtr(nullptr) { *this = src; }

    ~SkTLazy() {
        if (this->isValid()) {
            fPtr->~T();
        }
    }

    SkTLazy& operator=(const SkTLazy& src) {
        if (src.isValid()) {
            this->set(*src.get());
        } else {
            this->reset();
        }
        return *this;
    }

    /**
     *  Return a pointer to an instance of the class initialized with 'args'.
     *  If a previous instance had been initialized (either from init() or
     *  set()) it will first be destroyed, so that a freshly initialized
     *  instance is always returned.
     */
    template <typename... Args> T* init(Args&&... args) {
        if (this->isValid()) {
            fPtr->~T();
        }
        fPtr = new (SkTCast<T*>(fStorage.get())) T(std::forward<Args>(args)...);
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
            fPtr = new (SkTCast<T*>(fStorage.get())) T(src);
        }
        return fPtr;
    }

    /**
     * Destroy the lazy object (if it was created via init() or set())
     */
    void reset() {
        if (this->isValid()) {
            fPtr->~T();
            fPtr = nullptr;
        }
    }

    /**
     *  Returns true if a valid object has been initialized in the SkTLazy,
     *  false otherwise.
     */
    bool isValid() const { return SkToBool(fPtr); }

    /**
     * Returns the object. This version should only be called when the caller
     * knows that the object has been initialized.
     */
    T* get() const { SkASSERT(this->isValid()); return fPtr; }

    /**
     * Like above but doesn't assert if object isn't initialized (in which case
     * nullptr is returned).
     */
    T* getMaybeNull() const { return fPtr; }

private:
    SkAlignedSTStorage<1, T> fStorage;
    T*                       fPtr; // nullptr or fStorage
};

/**
 * A helper built on top of SkTLazy to do copy-on-first-write. The object is initialized
 * with a const pointer but provides a non-const pointer accessor. The first time the
 * accessor is called (if ever) the object is cloned.
 *
 * In the following example at most one copy of constThing is made:
 *
 * SkTCopyOnFirstWrite<Thing> thing(&constThing);
 * ...
 * function_that_takes_a_const_thing_ptr(thing); // constThing is passed
 * ...
 * if (need_to_modify_thing()) {
 *    thing.writable()->modifyMe(); // makes a copy of constThing
 * }
 * ...
 * x = thing->readSomething();
 * ...
 * if (need_to_modify_thing_now()) {
 *    thing.writable()->changeMe(); // makes a copy of constThing if we didn't call modifyMe()
 * }
 *
 * consume_a_thing(thing); // could be constThing or a modified copy.
 */
template <typename T>
class SkTCopyOnFirstWrite {
public:
    SkTCopyOnFirstWrite(const T& initial) : fObj(&initial) {}

    SkTCopyOnFirstWrite(const T* initial) : fObj(initial) {}

    // Constructor for delayed initialization.
    SkTCopyOnFirstWrite() : fObj(nullptr) {}

    // Should only be called once, and only if the default constructor was used.
    void init(const T& initial) {
        SkASSERT(nullptr == fObj);
        SkASSERT(!fLazy.isValid());
        fObj = &initial;
    }

    /**
     * Returns a writable T*. The first time this is called the initial object is cloned.
     */
    T* writable() {
        SkASSERT(fObj);
        if (!fLazy.isValid()) {
            fLazy.set(*fObj);
            fObj = fLazy.get();
        }
        return const_cast<T*>(fObj);
    }

    /**
     * Operators for treating this as though it were a const pointer.
     */

    const T *operator->() const { return fObj; }

    operator const T*() const { return fObj; }

    const T& operator *() const { return *fObj; }

private:
    const T*    fObj;
    SkTLazy<T>  fLazy;
};

#endif
