/*
 * Copyright 2009 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef sk_tools_Registry_DEFINED
#define sk_tools_Registry_DEFINED

#include "include/core/SkTypes.h"
#include "include/private/SkNoncopyable.h"

namespace sk_tools {

/** Template class that registers itself (in the constructor) into a linked-list
    and provides a function-pointer. This can be used to auto-register a set of
    services, e.g. a set of image codecs.
 */
template <typename T> class Registry : SkNoncopyable {
public:
    explicit Registry(T value) : fValue(value) {
#ifdef SK_BUILD_FOR_ANDROID
        // work-around for double-initialization bug
        {
            Registry* reg = gHead;
            while (reg) {
                if (reg == this) {
                    return;
                }
                reg = reg->fChain;
            }
        }
#endif
        fChain = gHead;
        gHead  = this;
    }

    static const Registry* Head() { return gHead; }

    const Registry* next() const { return fChain; }
    const T& get() const { return fValue; }

    // for (const T& t : sk_tools::Registry<T>::Range()) { process(t); }
    struct Range {
        struct Iterator {
            const Registry* fPtr;
            const T& operator*() { return SkASSERT(fPtr), fPtr->get(); }
            void operator++() { if (fPtr) { fPtr = fPtr->next(); } }
            bool operator!=(const Iterator& other) const { return fPtr != other.fPtr; }
        };
        Iterator begin() const { return Iterator{Registry::Head()}; }
        Iterator end() const { return Iterator{nullptr}; }
    };

private:
    T fValue;
    Registry* fChain;

    static Registry* gHead;
};

// The caller still needs to declare an instance of this somewhere
template <typename T> Registry<T>* Registry<T>::gHead;

}  // namespace sk_tools

#endif
