/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSGScene_DEFINED
#define SkSGScene_DEFINED

#include "SkRefCnt.h"
#include "SkTypes.h"

#include <memory>
#include <vector>

class SkCanvas;

namespace sksg {

class RenderNode;

/**
 * Base class for animators.
 *
 */
class Animator : public SkNoncopyable {
public:
    virtual ~Animator();

    void tick(float t);

protected:
    Animator();

    virtual void onTick(float t) = 0;

private:
    using INHERITED = SkNoncopyable;
};

/**
 * Holds a scene root and a list of animators.
 *
 * Provides high-level mehods for driving rendering and animations.
 *
 */
class Scene final : SkNoncopyable {
public:
    using AnimatorList = std::vector<std::unique_ptr<Animator>>;

    static std::unique_ptr<Scene> Make(sk_sp<RenderNode> root, AnimatorList&& animators);
    ~Scene();

    void render(SkCanvas*) const;
    void animate(float t);

    void setShowInval(bool show) { fShowInval = show; }

private:
    Scene(sk_sp<RenderNode> root, AnimatorList&& animators);

    const sk_sp<RenderNode> fRoot;
    const AnimatorList      fAnimators;

    bool                    fShowInval = false;

    using INHERITED = SkNoncopyable;
};

} // namespace sksg

#endif // SkSGScene_DEFINED
