/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSGTransformPriv_DEFINED
#define SkSGTransformPriv_DEFINED

#include "modules/sksg/include/SkSGTransform.h"

namespace sksg {

// Helper for accessing implementation-private Transform methods.
class TransformPriv final {
public:

    static bool Is44(const sk_sp<Transform>&t) { return t->is44(); }

    template <typename T, typename = std::enable_if<std::is_same<T, SkMatrix>::value ||
                                                    std::is_same<T, SkM44   >::value >>
    static T As(const sk_sp<Transform>&);

private:
    TransformPriv() = delete;
};

template <>
inline SkMatrix TransformPriv::As<SkMatrix>(const sk_sp<Transform>& t) {
    return t->asMatrix();
}

template <>
inline SkM44 TransformPriv::As<SkM44>(const sk_sp<Transform>& t) {
    return t->asM44();
}

} // namespace sksg

#endif // SkSGTransformPriv_DEFINED
