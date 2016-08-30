/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSVGContainer.h"

SkSVGContainer::SkSVGContainer(SkSVGTag t) : INHERITED(t) { }

void SkSVGContainer::appendChild(sk_sp<SkSVGNode> node) {
    SkASSERT(node);
    fChildren.push_back(std::move(node));
}

void SkSVGContainer::onRender(const SkSVGRenderContext& ctx) const {
    for (int i = 0; i < fChildren.count(); ++i) {
        fChildren[i]->render(ctx);
    }
}
