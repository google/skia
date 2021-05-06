/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_DSL_OPTIONAL
#define SKSL_DSL_OPTIONAL

#include <memory>

namespace SkSL {

namespace dsl {

/**
 * Simple drop-in replacement for std::optional until we move to C++17. This does not have all of
 * std::optional's capabilities, but it covers our needs for the time being.
 */
template<typename T>
class DSLOptional {
public:
    DSLOptional(T&& value)
        : fHasValue(true) {
        fPayload.fValue = std::move(value);
    }

    template<typename... Args>
    DSLOptional(Args... args)
        : DSLOptional(T(std::forward<Args...>(args...))) {}

    DSLOptional()
        : fHasValue(false) {}

    DSLOptional(DSLOptional&& other) {
        fHasValue = other.fHasValue;
        if (fHasValue) {
            fPayload.fValue = std::move(other.fPayload.fValue);
        }
    }

    ~DSLOptional() {
        if (fHasValue) {
            fPayload.fValue.~T();
        }
    }

    DSLOptional& operator=(DSLOptional&& other) {
        fHasValue = other.fHasValue;
        if (fHasValue) {
            fPayload.fValue = std::move(other.fPayload.fValue);
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

    operator bool() const {
        return fHasValue;
    }

private:
    bool fHasValue;

    union Payload {
        T fValue;

        Payload() {}

        ~Payload() {}
    } fPayload;
};

} // namespace dsl

} // namespace SkSL

#endif
