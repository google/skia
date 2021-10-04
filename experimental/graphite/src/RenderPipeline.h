/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_RenderPipeline_DEFINED
#define skgpu_RenderPipeline_DEFINED

namespace skgpu {

// TODO: derive this from something like GrManagedResource
class RenderPipeline {
public:
    virtual ~RenderPipeline();

protected:
    RenderPipeline();

private:
};

} // namespace skgpu

#endif // skgpu_RenderPipeline_DEFINED
