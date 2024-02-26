/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkMatrix.h"
#include "modules/sksg/include/SkSGRenderNode.h"
#include "modules/sksg/include/SkSGScene.h"

#include <utility>

namespace sksg {

std::unique_ptr<Scene> Scene::Make(sk_sp<RenderNode> root) {
    return root ? std::unique_ptr<Scene>(new Scene(std::move(root))) : nullptr;
}

Scene::Scene(sk_sp<RenderNode> root) : fRoot(std::move(root)) {}

Scene::~Scene() = default;

void Scene::render(SkCanvas* canvas) const {
    fRoot->render(canvas);
}

void Scene::revalidate(InvalidationController* ic) {
    fRoot->revalidate(ic, SkMatrix::I());
}

const RenderNode* Scene::nodeAt(const SkPoint& p) const {
    return fRoot->nodeAt(p);
}

} // namespace sksg
