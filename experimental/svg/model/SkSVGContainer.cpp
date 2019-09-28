/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/svg/model/SkSVGContainer.h"

#include "include/core/SkPath.h"
#include "include/pathops/SkPathOps.h"

SkSVGContainer::SkSVGContainer(SkSVGTag t) : INHERITED(t) { }

void SkSVGContainer::appendChild(sk_sp<SkSVGNode> node) {
    SkASSERT(node);
    fChildren.push_back(std::move(node));
}

bool SkSVGContainer::hasChildren() const {
    return !fChildren.empty();
}

void SkSVGContainer::onRender(const SkSVGRenderContext& ctx) const {
    for (int i = 0; i < fChildren.count(); ++i) {
        fChildren[i]->render(ctx);
    }
}

SkPath SkSVGContainer::onAsPath(const SkSVGRenderContext& ctx) const {
    SkPath path;

    for (int i = 0; i < fChildren.count(); ++i) {
        const SkPath childPath = fChildren[i]->asPath(ctx);

        Op(path, childPath, kUnion_SkPathOp, &path);
    }

    this->mapToParent(&path);
    return path;
}
