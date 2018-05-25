/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSGScene.h"

#include "SkCanvas.h"
#include "SkMatrix.h"
#include "SkPaint.h"
#include "SkSGInvalidationController.h"
#include "SkSGRenderNode.h"

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
    InvalidationController ic;
    fRoot->revalidate(&ic, SkMatrix::I());
    fRoot->render(canvas);

    if (fShowInval) {
        SkPaint fill, stroke;
        fill.setAntiAlias(true);
        fill.setColor(0x40ff0000);
        stroke.setAntiAlias(true);
        stroke.setColor(0xffff0000);
        stroke.setStyle(SkPaint::kStroke_Style);

        for (const auto& r : ic) {
            canvas->drawRect(r, fill);
            canvas->drawRect(r, stroke);
        }
    }
}

void Scene::animate(float t) {
    for (const auto& anim : fAnimators) {
        anim->tick(t);
    }
}

} // namespace sksg
