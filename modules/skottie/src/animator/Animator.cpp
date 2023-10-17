/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/skottie/src/animator/Animator.h"

#include "modules/skottie/src/SkottieJson.h"
#include "modules/skottie/src/SkottiePriv.h"
#include "modules/skottie/src/animator/KeyframeAnimator.h"

namespace skottie::internal {

Animator::StateChanged AnimatablePropertyContainer::onSeek(float t) {
    // The very first seek must trigger a sync, to ensure proper SG setup.
    bool changed = !fHasSynced;

    for (const auto& animator : fAnimators) {
        changed |= animator->seek(t);
    }

    if (changed) {
        this->onSync();
        fHasSynced = true;
    }

    return changed;
}

void AnimatablePropertyContainer::attachDiscardableAdapter(
        sk_sp<AnimatablePropertyContainer> child) {
    if (!child) {
        return;
    }

    if (child->isStatic()) {
        child->seek(0);
        return;
    }

    fAnimators.push_back(std::move(child));
}

void AnimatablePropertyContainer::shrink_to_fit() {
    fAnimators.shrink_to_fit();
}

bool AnimatablePropertyContainer::bindImpl(const AnimationBuilder& abuilder,
                                           const skjson::ObjectValue* jprop,
                                           AnimatorBuilder& builder) {
    if (!jprop) {
        return false;
    }

    if (const skjson::StringValue* jpropSlotID = (*jprop)["sid"] ) {
        if (!abuilder.getSlotsRoot()) {
            abuilder.log(Logger::Level::kWarning, jprop,
                         "Slotid found but no slots were found in the json. Using default values.");
        } else {
            const skjson::ObjectValue* slot = (*(abuilder.getSlotsRoot()))[jpropSlotID->begin()];
            if (!slot) {
                abuilder.log(Logger::Level::kWarning, jprop,
                             "Specified slotID not found in 'slots'. Using default values.");
            } else {
                jprop = (*slot)["p"];
            }
        }
    }

    const auto& jpropA = (*jprop)["a"];
    const auto& jpropK = (*jprop)["k"];

    // Handle expressions on the property.
    if (const skjson::StringValue* expr = (*jprop)["x"]) {
        if (!abuilder.expression_manager()) {
            abuilder.log(Logger::Level::kWarning, jprop,
                         "Expression encountered but ExpressionManager not provided.");
        } else {
            builder.parseValue(abuilder, jpropK);
            sk_sp<Animator> expression_animator = builder.makeFromExpression(
                                                    *abuilder.expression_manager(),
                                                    expr->begin());
            if (expression_animator) {
                fAnimators.push_back(std::move(expression_animator));
                return true;
            }
        }
    }

    // Older Json versions don't have an "a" animation marker.
    // For those, we attempt to parse both ways.
    if (!ParseDefault<bool>(jpropA, false)) {
        if (builder.parseValue(abuilder, jpropK)) {
            // Static property.
            return true;
        }

        if (!jpropA.is<skjson::NullValue>()) {
            abuilder.log(Logger::Level::kError, jprop,
                         "Could not parse (explicit) static property.");
            return false;
        }
    }

    // Keyframed property.
    sk_sp<KeyframeAnimator> animator;
    const skjson::ArrayValue* jkfs = jpropK;
    if (jkfs && jkfs->size() > 0) {
        animator = builder.makeFromKeyframes(abuilder, *jkfs);
    }

    if (!animator) {
        abuilder.log(Logger::Level::kError, jprop, "Could not parse keyframed property.");
        return false;
    }

    if (animator->isConstant()) {
        // If all keyframes are constant, there is no reason to treat this
        // as an animated property - apply immediately and discard the animator.
        animator->seek(0);
    } else {
        fAnimators.push_back(std::move(animator));
    }

    return true;
}

} // namespace skottie::internal
