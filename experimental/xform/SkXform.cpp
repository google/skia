/*
 * Copyright 2019 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/xform/SkXform.h"
#include "include/private/SkTArray.h"

void Xform::visit(XformResolver* resolver) {
    constexpr int kPreAllocated = 32;   // each is small: just a pointer
    constexpr bool kCanMemMove = true;  // since each is just a pointer
    SkSTArray<kPreAllocated, Xform*, kCanMemMove> stack;

    for (Xform* xform = this; xform; xform = xform->parent()) {
        stack.push_back(xform);
    }
    for (int i = stack.count() - 1; i >= 0; --i) {
        stack[i]->onVisit(resolver);
    }
}

void MatrixXF::onVisit(XformResolver* resolver) {
    resolver->concat(fLocalMatrix);
}

void ClipXF::onVisit(XformResolver* resolver) {
    resolver->clipRect(fRect, fOp);
}
