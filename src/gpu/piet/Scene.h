/*
 * Copyright 2022 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkPath.h"
#include "include/core/SkRefCnt.h"
#include "src/gpu/piet/PietTypes.h"

#ifndef skgpu_piet_Scene_DEFINED
#define skgpu_piet_Scene_DEFINED

namespace skgpu::piet {

class Scene final : public SkRefCnt, public Object<PgpuScene, pgpu_scene_destroy> {
public:
    static sk_sp<Scene> Make() { return sk_sp<Scene>(new Scene()); }

    // Insert a single path with the given transform into the scene. This call can be called
    // multiple times to populate a scene. The changes will take effect following a successful call
    // to `finalize()`.
    //
    // TODO(b/241580303): This mode of usage requires the entire scene to be rebuilt before (for
    // even incremental changes) before recording. We should provide an interface for populating the
    // scene via scene fragments instead.
    void addPath(const SkPath& path, const Transform& transform);

    // Returns true if there were any pending changes to the scene that were finalized and the scene
    // is now ready to render.
    //
    // Any subsequent calls to modify the scene will clear the scene contents first.
    bool finalize();

private:
    Scene();

    class Builder final : public Object<PgpuSceneBuilder, pgpu_scene_builder_finish> {
    public:
        explicit Builder(PgpuSceneBuilder* builder) : Object(builder) {}
    };

    void initBuilderIfNecessary();

    std::optional<Builder> fActiveBuilder;
};
}  // namespace skgpu::piet

#endif  // skgpu_piet_Scene_DEFINED
