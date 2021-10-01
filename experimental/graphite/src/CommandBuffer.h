/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_CommandBuffer_DEFINED
#define skgpu_CommandBuffer_DEFINED

namespace skgpu {

class CommandBuffer {
public:
    virtual ~CommandBuffer() {}

protected:
    CommandBuffer();

private:
};

} // namespace skgpu

#endif // skgpu_CommandBuffer_DEFINED
