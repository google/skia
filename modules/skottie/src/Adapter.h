/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkottieAdapter_DEFINED
#define SkottieAdapter_DEFINED

#include "modules/skottie/src/animator/Animator.h"

namespace skottie {
namespace internal {

template <typename AdapterT, typename T>
class DiscardableAdapterBase : public AnimatablePropertyContainer {
public:
    template <typename... Args>
    static sk_sp<AdapterT> Make(Args&&... args) {
        sk_sp<AdapterT> adapter(new AdapterT(std::forward<Args>(args)...));
        adapter->shrink_to_fit();
        return adapter;
    }

    const sk_sp<T>& node() const { return fNode; }

protected:
    DiscardableAdapterBase()
        : fNode(T::Make()) {}

    explicit DiscardableAdapterBase(sk_sp<T> node)
        : fNode(std::move(node)) {}

private:
    const sk_sp<T> fNode;
};

} // namespace internal
} // namespace skottie

#endif // SkottieAdapter_DEFINED
