/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSGTransformPriv_DEFINED
#define SkSGTransformPriv_DEFINED

#include "SkSGTransform.h"

namespace sksg {

// Helper for accessing implementation-private Transform methods.
class TransformPriv final {
public:
    static SkMatrix   AsMatrix  (const sk_sp<Transform>& t) { return t->asMatrix();   }
    static SkMatrix44 AsMatrix44(const sk_sp<Transform>& t) { return t->asMatrix44(); }

private:
    TransformPriv() = delete;
};

} // namespace sksg

#endif // SkSGTransformPriv_DEFINED
