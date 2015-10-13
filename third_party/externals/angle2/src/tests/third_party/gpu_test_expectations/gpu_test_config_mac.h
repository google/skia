//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// gpu_test_config_mac.h:
//   Helper functions for gpu_test_config that have to be compiled in ObjectiveC++
//

#ifndef GPU_TEST_EXPECTATIONS_GPU_TEST_CONFIG_MAC_H_
#define GPU_TEST_EXPECTATIONS_GPU_TEST_CONFIG_MAC_H_

#include "gpu_info.h"

namespace base {

class SysInfo
{
  public:
    static void OperatingSystemVersionNumbers(
        int32 *major_version, int32 *minor_version, int32 *bugfix_version);
};

} // namespace base

gpu::GPUInfo::GPUDevice GetActiveGPU();

#endif // GPU_TEST_EXPECTATIONS_GPU_TEST_CONFIG_MAC_H_
