/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_GpuWorkSubmission_DEFINED
#define skgpu_GpuWorkSubmission_DEFINED

namespace skgpu {
class Gpu;

class GpuWorkSubmission {
public:
    virtual ~GpuWorkSubmission() = default;

    virtual bool isFinished() = 0;
    virtual void waitUntilFinished(const Gpu*) = 0;

protected:
    GpuWorkSubmission() = default;

private:
};

} // namespace skgpu

#endif // skgpu_GpuWorkSubmission_DEFINED
