/*
 * Copyright 2022 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_piet_PietTypes_DEFINED
#define skgpu_piet_PietTypes_DEFINED

#include <pgpu.h>

#include <memory>
#include <optional>

namespace skgpu::piet {

using Transform = PgpuTransform;

inline Transform identity_transform() { return {1, 0, 0, 1, 0, 0}; }

/**
 * Wrapper around a pgpu-render type that automatically frees resources on destruction by calling
 * the appropriate pgpu-render API function.
 */
template <typename Type, void (*DtorFunc)(Type*)> class Object {
public:
    explicit Object(Type* handle) : fHandle(handle) {}
    Object() : fHandle(nullptr) {}

    Object(Object&&) = default;
    Object& operator=(Object&&) = default;

    virtual ~Object() = default;

    Type* get() const { return fHandle.get(); }

    operator bool() const { return this->get() != nullptr; }

protected:
    void reset() { fHandle = nullptr; }

private:
    Object(const Object&) = delete;
    Object& operator=(const Object&) = delete;

    struct Deleter {
        void operator()(Type* t) {
            if (t) {
                DtorFunc(t);
            }
        }
    };
    std::unique_ptr<Type, Deleter> fHandle;
};

}  // namespace skgpu::piet

#endif  // skgpu_piet_PietTypes_DEFINED
