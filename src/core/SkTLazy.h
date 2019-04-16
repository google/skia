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
#include <type_traits>
#include <utility>

/**
 *  Efficient way to defer allocating/initializing a class until it is needed
 *  (if ever).
 */
template <typename T> class SkTLazy {
public:
    SkTLazy() = default;
    explicit SkTLazy(const T* src) : fPtr(src ? new (&fStorage) T(*src) : nullptr) {}
    SkTLazy(const SkTLazy& that) : fPtr(that.fPtr ? new (&fStorage) T(*that.fPtr) : nullptr) {}
    SkTLazy(SkTLazy&& that) : fPtr(that.fPtr ? new (&fStorage) T(std::move(*that.fPtr)) : nullptr){}

    ~SkTLazy() { this->reset(); }

    SkTLazy& operator=(const SkTLazy& that) {
        if (that.isValid()) {
            this->set(*that);
        } else {
            this->reset();
        }
        return *this;
    }

    SkTLazy& operator=(SkTLazy&& that) {
        if (that.isValid()) {
            this->set(std::move(*that));
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
        this->reset();
        fPtr = new (&fStorage) T(std::forward<Args>(args)...);
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
            fPtr = new (&fStorage) T(src);
        }
        return fPtr;
    }

    T* set(T&& src) {
        if (this->isValid()) {
            *fPtr = std::move(src);
        } else {
            fPtr = new (&fStorage) T(std::move(src));
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
    T* operator->() const { return this->get(); }
    T& operator*() const { return *this->get(); }

    /**
     * Like above but doesn't assert if object isn't initialized (in which case
     * nullptr is returned).
     */
    T* getMaybeNull() const { return fPtr; }

private:
    typename std::aligned_storage<sizeof(T), alignof(T)>::type fStorage;
    T*                                                         fPtr{nullptr}; // nullptr or fStorage
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
    explicit SkTCopyOnFirstWrite(const T& initial) : fObj(&initial) {}

    explicit SkTCopyOnFirstWrite(const T* initial) : fObj(initial) {}

    // Constructor for delayed initialization.
    SkTCopyOnFirstWrite() : fObj(nullptr) {}

    SkTCopyOnFirstWrite(const SkTCopyOnFirstWrite&  that) { *this = that;            }
    SkTCopyOnFirstWrite(      SkTCopyOnFirstWrite&& that) { *this = std::move(that); }

    SkTCopyOnFirstWrite& operator=(const SkTCopyOnFirstWrite& that) {
        fLazy = that.fLazy;
        fObj  = fLazy.isValid() ? fLazy.get() : that.fObj;
        return *this;
    }

    SkTCopyOnFirstWrite& operator=(SkTCopyOnFirstWrite&& that) {
        fLazy = std::move(that.fLazy);
        fObj  = fLazy.isValid() ? fLazy.get() : that.fObj;
        return *this;
    }

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

    const T* get() const { return fObj; }

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
