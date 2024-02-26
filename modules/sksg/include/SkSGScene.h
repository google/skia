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

class SkCanvas;
struct SkPoint;

namespace sksg {

class InvalidationController;
class RenderNode;

/**
 * Holds a scene root.  Provides high-level methods for rendering.
 *
 */
class Scene final {
public:
    static std::unique_ptr<Scene> Make(sk_sp<RenderNode> root);
    ~Scene();
    Scene(const Scene&) = delete;
    Scene& operator=(const Scene&) = delete;

    void render(SkCanvas*) const;
    void revalidate(InvalidationController* = nullptr);
    const RenderNode* nodeAt(const SkPoint&) const;

private:
    explicit Scene(sk_sp<RenderNode> root);

    const sk_sp<RenderNode> fRoot;
};

} // namespace sksg

#endif // SkSGScene_DEFINED
