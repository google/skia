/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSGScene_DEFINED
#define SkSGScene_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/core/SkTypes.h"

#include <memory>
#include <vector>

class SkCanvas;
struct SkPoint;

namespace sksg {

class RenderNode;

/**
 * Base class for animators.
 *
 */
class Animator {
public:
    virtual ~Animator();
    Animator(const Animator&) = delete;
    Animator& operator=(const Animator&) = delete;

    void tick(float t);

protected:
    Animator();

    virtual void onTick(float t) = 0;
};

using AnimatorList = std::vector<std::unique_ptr<Animator>>;

class GroupAnimator : public Animator {
protected:
    explicit GroupAnimator(AnimatorList&&);

    void onTick(float t) override;

private:
    const AnimatorList fAnimators;

    using INHERITED = Animator;
};

/**
 * Holds a scene root and a list of animators.
 *
 * Provides high-level mehods for driving rendering and animations.
 *
 */
class Scene final {
public:
    static std::unique_ptr<Scene> Make(sk_sp<RenderNode> root, AnimatorList&& animators);
    ~Scene();
    Scene(const Scene&) = delete;
    Scene& operator=(const Scene&) = delete;

    void render(SkCanvas*) const;
    void animate(float t);
    const RenderNode* nodeAt(const SkPoint&) const;

    void setShowInval(bool show) { fShowInval = show; }

private:
    Scene(sk_sp<RenderNode> root, AnimatorList&& animators);

    const sk_sp<RenderNode> fRoot;
    const AnimatorList      fAnimators;

    bool                    fShowInval = false;
};

} // namespace sksg

#endif // SkSGScene_DEFINED
