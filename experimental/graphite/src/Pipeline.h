/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_Pipeline_DEFINED
#define skgpu_Pipeline_DEFINED

namespace skgpu {

// TODO: derive this from something like GrManagedResource
class Pipeline {
public:
    virtual ~Pipeline();

protected:
    Pipeline();

private:
};

} // namespace skgpu

#endif // skgpu_Pipeline_DEFINED
