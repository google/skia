/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "VkTestUtils.h"

#ifdef SK_VULKAN

#include "../ports/SkOSLibrary.h"

namespace sk_gpu_test {

bool LoadVkLibraryAndGetProcAddrFuncs(PFN_vkGetInstanceProcAddr* instProc,
                                                   PFN_vkGetDeviceProcAddr* devProc) {
    static void* vkLib = nullptr;
    static PFN_vkGetInstanceProcAddr localInstProc = nullptr;
    static PFN_vkGetDeviceProcAddr localDevProc = nullptr;
    if (!vkLib) {
#if defined _WIN32
        vkLib = DynamicLoadLibrary("vulkan-1.dll");
#else
        vkLib = DynamicLoadLibrary("libvulkan.so");
#endif
        if (!vkLib) {
            return false;
        }
        localInstProc = (PFN_vkGetInstanceProcAddr) GetProcedureAddress(vkLib,
                                                                        "vkGetInstanceProcAddr");
        localDevProc = (PFN_vkGetDeviceProcAddr) GetProcedureAddress(vkLib,
                                                                     "vkGetDeviceProcAddr");
    }
    if (!localInstProc || !localDevProc) {
        return false;
    }
    *instProc = localInstProc;
    *devProc = localDevProc;
    return true;
}
}

#endif
