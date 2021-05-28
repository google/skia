/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkTOptional_DEFINED
#define SkTOptional_DEFINED

#include "include/core/SkTypes.h"

#include <utility>

namespace skstd {

/**
 * Simple drop-in replacement for std::optional until we move to C++17. This does not have all of
 * std::optional's capabilities, but it covers our needs for the time being.
 */
template<typename T>
class optional {
public:
    optional(const optional&) = delete;

    optional(T&& value)
        : fHasValue(true) {
        new(&fPayload.fValue) T(std::move(value));
    }

    template<typename... Args>
    optional(Args&&... args)
        : optional(T(std::forward<Args...>(args...))) {}

    optional() {}

    optional(optional&& other) {
        *this = std::move(other);
    }

    ~optional() {
        if (fHasValue) {
            fPayload.fValue.~T();
        }
    }

    optional& operator=(const optional&) = delete;

    optional& operator=(optional&& other) {
        if (this != &other) {
            if (fHasValue) {
                fPayload.fValue.~T();
            }
            fHasValue = other.fHasValue;
            if (fHasValue) {
                new(&fPayload.fValue) T(std::move(other.fPayload.fValue));
            }
        }
        return *this;
    }

    T& operator*() {
        SkASSERT(fHasValue);
        return fPayload.fValue;
    }

    T* operator->() {
        SkASSERT(fHasValue);
        return &fPayload.fValue;
    }

    const T& operator*() const {
        SkASSERT(fHasValue);
        return fPayload.fValue;
    }

    const T* operator->() const {
        SkASSERT(fHasValue);
        return &fPayload.fValue;
    }

    explicit operator bool() const {
        return fHasValue;
    }

private:
    union Payload {
        T fValue;

        Payload() {}

        ~Payload() {}
    } fPayload;

    bool fHasValue = false;
};

} // namespace skstd

#endif
