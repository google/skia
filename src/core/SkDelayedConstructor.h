/*
 * Copyright 2019 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkDelayedConstructor_DEFINED
#define SkDelayedConstructor_DEFINED

#include <type_traits>

template <typename T>
class SkDelayedConstructor {
public:
    template <typename... Args>
    T* initialize(Args&&... args) {
        return new (&fObject) T{std::forward<Args>(args)...};
    }

    void destroy() { this->get()->~T(); }

    T* get()        { return reinterpret_cast<T*>(&fObject); }
    T* operator->() { return this->get(); }
    T& operator*()  { return *this->get(); }

    const T* get()        const { return reinterpret_cast<const T*>(&fObject); }
    const T* operator->() const { return this->get(); }
    const T& operator*()  const { return *this->get(); }

private:
    typename std::aligned_storage<sizeof(T), alignof(T)>::type fObject;
};

#endif  // SkDelayedConstructor_DEFINED
