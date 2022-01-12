/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_ContextPriv_DEFINED
#define skgpu_ContextPriv_DEFINED

#include "experimental/graphite/include/Context.h"

namespace skgpu {

class Gpu;
class ResourceProvider;

/** Class that adds methods to Context that are only intended for use internal to Skia.
    This class is purely a privileged window into Context. It should never have additional
    data members or virtual methods. */
class ContextPriv {
public:
    Gpu* gpu();
    const Gpu* gpu() const;

    ResourceProvider* resourceProvider();

    SkShaderCodeDictionary* shaderCodeDictionary();
    const SkShaderCodeDictionary* shaderCodeDictionary() const;

private:
    friend class Context; // to construct/copy this type.

    explicit ContextPriv(Context* context) : fContext(context) {}

    // Required until C++17 copy elision
    ContextPriv(const ContextPriv&) = default;
    ContextPriv& operator=(const ContextPriv&) = delete;

    // No taking addresses of this type.
    const ContextPriv* operator&() const;
    ContextPriv *operator&();

    Context* fContext;
};

inline ContextPriv Context::priv() { return ContextPriv(this); }

// NOLINTNEXTLINE(readability-const-return-type)
inline const ContextPriv Context::priv() const {
    return ContextPriv(const_cast<Context *>(this));
}

} // namespace skgpu

#endif // skgpu_ContextPriv_DEFINED
