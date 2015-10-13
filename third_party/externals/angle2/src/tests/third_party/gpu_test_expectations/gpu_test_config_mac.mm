// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// gpu_test_config_mac.mm:
//   Helper functions for gpu_test_config that have to be compiled in ObjectiveC++

#include "gpu_test_config_mac.h"

#import <Cocoa/Cocoa.h>

namespace base {

void SysInfo::OperatingSystemVersionNumbers(
    int32 *major_version, int32 *minor_version, int32 *bugfix_version)
{
#if MAC_OS_X_VERSION_MIN_REQUIRED < MAC_OS_X_VERSION_10_8
  Gestalt(gestaltSystemVersionMajor, reinterpret_cast<SInt32*>(major_version));
  Gestalt(gestaltSystemVersionMinor, reinterpret_cast<SInt32*>(minor_version));
  Gestalt(gestaltSystemVersionBugFix, reinterpret_cast<SInt32*>(bugfix_version));
#else
  NSOperatingSystemVersion version = [[NSProcessInfo processInfo] operatingSystemVersion];
  *major_version = version.majorVersion;
  *minor_version = version.minorVersion;
  *bugfix_version = version.patchVersion;
#endif
}

} // namespace base

UInt32 GetEntryProperty(io_registry_entry_t entry, CFStringRef property_name) {
  CFTypeRef type = IORegistryEntrySearchCFProperty(entry,
                                                   kIOServicePlane,
                                                   property_name,
                                                   kCFAllocatorDefault,
                                                   kIORegistryIterateRecursively | kIORegistryIterateParents);
  CFDataRef data = reinterpret_cast<CFDataRef>(type);
  if (!data) {
    CFRelease(data);
    return 0;
  }

  UInt32 value = 0;
  const uint32_t* valuePointer = reinterpret_cast<const uint32_t*>(CFDataGetBytePtr(data));
  if (valuePointer != NULL) {
    value = *valuePointer;
  }
  CFRelease(data);
  return value;
}

gpu::GPUInfo::GPUDevice GetActiveGPU() {
  gpu::GPUInfo::GPUDevice gpu;

  // Ignore the fact that CGDisplayIOServicePort is deprecated as Apple
  // did not provide a good replacement for it as of 10.10.
  // TODO(cwallez) revisit with later systems
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wdeprecated-declarations"
    io_registry_entry_t dsp_port = CGDisplayIOServicePort(kCGDirectMainDisplay);
  #pragma clang diagnostic pop

  gpu.vendor_id = GetEntryProperty(dsp_port, CFSTR("vendor-id"));
  gpu.device_id = GetEntryProperty(dsp_port, CFSTR("device-id"));
  return gpu;
}

