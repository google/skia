/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/sksg/include/SkSGScene.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "modules/sksg/include/SkSGInvalidationController.h"
#include "modules/sksg/include/SkSGRenderNode.h"

namespace sksg {

std::unique_ptr<Scene> Scene::Make(sk_sp<RenderNode> root) {
    return root ? std::unique_ptr<Scene>(new Scene(std::move(root))) : nullptr;
}

Scene::Scene(sk_sp<RenderNode> root) : fRoot(std::move(root)) {}

Scene::~Scene() = default;

void Scene::render(SkCanvas* canvas) const {
    // Ensure the SG is revalidated.
    // Note: this is a no-op if the scene has already been revalidated - e.g. in animate().
    fRoot->revalidate(nullptr, SkMatrix::I());

    fRoot->render(canvas);
}

void Scene::revalidate(InvalidationController* ic) {
    fRoot->revalidate(ic, SkMatrix::I());
}

const RenderNode* Scene::nodeAt(const SkPoint& p) const {
    return fRoot->nodeAt(p);
}

} // namespace sksg
