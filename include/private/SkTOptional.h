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
 * An empty optional is represented with `nullopt`.
 */
struct nullopt_t {
    struct tag {};

    // nullopt_t must not be default-constructible.
    explicit constexpr nullopt_t(tag) {}
};

inline constexpr nullopt_t nullopt{nullopt_t::tag{}};

/**
 * Simple drop-in replacement for std::optional until we move to C++17. This does not have all of
 * std::optional's capabilities, but it covers our needs for the time being.
 */
template<typename T>
class optional {
public:
    optional(const T& value)
        : fHasValue(true) {
        new(&fPayload.fValue) T(value);
    }

    optional(T&& value)
        : fHasValue(true) {
        new(&fPayload.fValue) T(std::move(value));
    }

    optional() {}

    optional(const optional& other) {
        *this = other;
    }

    // Construction with nullopt is the same as default construction.
    optional(nullopt_t) : optional() {}

    // We need a non-const copy constructor because otherwise optional(nonConstSrc) isn't an exact
    // match for the copy constructor, and we'd end up invoking the Args&&... template by mistake.
    optional(optional& other) {
        *this = other;
    }

    optional(optional&& other) {
        *this = std::move(other);
    }

    template<typename... Args>
    optional(Args&&... args) {
        fHasValue = true;
        new(&fPayload.fValue) T(std::forward<Args...>(args...));
    }

    ~optional() {
        this->reset();
    }

    optional& operator=(const optional& other) {
        if (this != &other) {
            if (fHasValue) {
                if (other.fHasValue) {
                    fPayload.fValue = other.fPayload.fValue;
                } else {
                    this->reset();
                }
            } else {
                if (other.fHasValue) {
                    fHasValue = true;
                    new (&fPayload.fValue) T(other.fPayload.fValue);
                } else {
                    // do nothing, no value on either side
                }
            }
        }
        return *this;
    }

    optional& operator=(optional&& other) {
        if (this != &other) {
            if (fHasValue) {
                if (other.fHasValue) {
                    fPayload.fValue = std::move(other.fPayload.fValue);
                } else {
                    this->reset();
                }
            } else {
                if (other.fHasValue) {
                    fHasValue = true;
                    new (&fPayload.fValue) T(std::move(other.fPayload.fValue));
                } else {
                    // do nothing, no value on either side
                }
            }
        }
        return *this;
    }

    // Assignment to nullopt is the same as reset().
    optional& operator=(nullopt_t) {
        this->reset();
        return *this;
    }

    const T& value() const {
        SkASSERT(fHasValue);
        return fPayload.fValue;
    }

    T& value() {
        return fPayload.fValue;
    }

    T& operator*() {
        SkASSERT(fHasValue);
        return this->value();
    }

    T* operator->() {
        SkASSERT(fHasValue);
        return &this->value();
    }

    const T& operator*() const {
        return this->value();
    }

    const T* operator->() const {
        SkASSERT(fHasValue);
        return &this->value();
    }

    bool has_value() const {
        return fHasValue;
    }

    explicit operator bool() const {
        return this->has_value();
    }

    void reset() {
        if (fHasValue) {
            fPayload.fValue.~T();
            fHasValue = false;
        }
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
