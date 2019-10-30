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

Animator::Animator()  = default;
Animator::~Animator() = default;

void Animator::tick(float t) {
    this->onTick(t);
}

GroupAnimator::GroupAnimator(AnimatorList&& animators)
    : fAnimators(std::move(animators)) {}

void GroupAnimator::onTick(float t) {
    for (const auto& a : fAnimators) {
        a->tick(t);
    }
}

std::unique_ptr<Scene> Scene::Make(sk_sp<RenderNode> root, AnimatorList&& anims) {
    return root ? std::unique_ptr<Scene>(new Scene(std::move(root), std::move(anims))) : nullptr;
}

Scene::Scene(sk_sp<RenderNode> root, AnimatorList&& animators)
    : fRoot(std::move(root))
    , fAnimators(std::move(animators)) {}

Scene::~Scene() = default;

void Scene::render(SkCanvas* canvas) const {
    // Ensure the SG is revalidated.
    // Note: this is a no-op if the scene has already been revalidated - e.g. in animate().
    fRoot->revalidate(nullptr, SkMatrix::I());
    fRoot->render(canvas);
}

void Scene::animate(float t, InvalidationController* ic) {
    for (const auto& anim : fAnimators) {
        anim->tick(t);
    }

    fRoot->revalidate(ic, SkMatrix::I());
}

const RenderNode* Scene::nodeAt(const SkPoint& p) const {
    return fRoot->nodeAt(p);
}

} // namespace sksg
