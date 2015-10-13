// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "gpu_test_config.h"

#include "gpu_info.h"
#include "gpu_test_expectations_parser.h"

#if defined(OS_LINUX)
extern "C" {
#   include <pci/pci.h>
}
#endif

#if defined(OS_MACOSX)
#include "gpu_test_config_mac.h"
#endif

using namespace gpu;

#if defined(OS_WIN)

namespace base {

namespace {

// Disable the deprecated function warning for GetVersionEx
#pragma warning(disable: 4996)

class SysInfo
{
  public:
    static void OperatingSystemVersionNumbers(
        int32 *major_version, int32 *minor_version, int32 *bugfix_version);
};

// static
void SysInfo::OperatingSystemVersionNumbers(
    int32 *major_version, int32 *minor_version, int32 *bugfix_version)
{
  OSVERSIONINFOEX version_info = { sizeof version_info };
  ::GetVersionEx(reinterpret_cast<OSVERSIONINFO*>(&version_info));
  *major_version = version_info.dwMajorVersion;
  *minor_version = version_info.dwMinorVersion;
  *bugfix_version = version_info.dwBuildNumber;
}

} // anonymous namespace

} // namespace base

void DeviceIDToVendorAndDevice(const std::string& id,
                               uint32* vendor_id,
                               uint32* device_id) {
  *vendor_id = 0;
  *device_id = 0;
  if (id.length() < 21)
    return;
  std::string vendor_id_string = id.substr(8, 4);
  std::string device_id_string = id.substr(17, 4);
  base::HexStringToUInt(vendor_id_string, vendor_id);
  base::HexStringToUInt(device_id_string, device_id);
}

CollectInfoResult CollectGpuID(uint32* vendor_id, uint32* device_id) {
  DCHECK(vendor_id && device_id);
  *vendor_id = 0;
  *device_id = 0;

  // Taken from http://developer.nvidia.com/object/device_ids.html
  DISPLAY_DEVICEA dd;
  dd.cb = sizeof(DISPLAY_DEVICEA);
  std::string id;
  for (int i = 0; EnumDisplayDevicesA(NULL, i, &dd, 0); ++i) {
    if (dd.StateFlags & DISPLAY_DEVICE_PRIMARY_DEVICE) {
      id = dd.DeviceID;
      break;
    }
  }

  if (id.length() > 20) {
    DeviceIDToVendorAndDevice(id, vendor_id, device_id);
    if (*vendor_id != 0 && *device_id != 0)
      return kCollectInfoSuccess;
  }
  return kCollectInfoNonFatalFailure;
}

#endif // defined(OS_WIN)

#if defined(OS_LINUX)

const uint32 kVendorIDIntel = 0x8086;
const uint32 kVendorIDNVidia = 0x10de;
const uint32 kVendorIDAMD = 0x1002;

CollectInfoResult CollectPCIVideoCardInfo(GPUInfo* gpu_info) {
  DCHECK(gpu_info);

  struct pci_access* access = pci_alloc();
  DCHECK(access != NULL);
  pci_init(access);
  pci_scan_bus(access);

  bool primary_gpu_identified = false;
  for (pci_dev* device = access->devices;
       device != NULL; device = device->next) {
    pci_fill_info(device, 33);
    bool is_gpu = false;
    switch (device->device_class) {
      case PCI_CLASS_DISPLAY_VGA:
      case PCI_CLASS_DISPLAY_XGA:
      case PCI_CLASS_DISPLAY_3D:
        is_gpu = true;
        break;
      case PCI_CLASS_DISPLAY_OTHER:
      default:
        break;
    }
    if (!is_gpu)
      continue;
    if (device->vendor_id == 0 || device->device_id == 0)
      continue;

    GPUInfo::GPUDevice gpu;
    gpu.vendor_id = device->vendor_id;
    gpu.device_id = device->device_id;

    if (!primary_gpu_identified) {
      primary_gpu_identified = true;
      gpu_info->gpu = gpu;
    } else {
      // TODO(zmo): if there are multiple GPUs, we assume the non Intel
      // one is primary. Revisit this logic because we actually don't know
      // which GPU we are using at this point.
      if (gpu_info->gpu.vendor_id == kVendorIDIntel &&
          gpu.vendor_id != kVendorIDIntel) {
        gpu_info->secondary_gpus.push_back(gpu_info->gpu);
        gpu_info->gpu = gpu;
      } else {
        gpu_info->secondary_gpus.push_back(gpu);
      }
    }
  }

  // Detect Optimus or AMD Switchable GPU.
  if (gpu_info->secondary_gpus.size() == 1 &&
      gpu_info->secondary_gpus[0].vendor_id == kVendorIDIntel) {
    if (gpu_info->gpu.vendor_id == kVendorIDNVidia)
      gpu_info->optimus = true;
    if (gpu_info->gpu.vendor_id == kVendorIDAMD)
      gpu_info->amd_switchable = true;
  }

  pci_cleanup(access);
  if (!primary_gpu_identified)
    return kCollectInfoNonFatalFailure;
  return kCollectInfoSuccess;
}

CollectInfoResult CollectGpuID(uint32* vendor_id, uint32* device_id) {
  DCHECK(vendor_id && device_id);
  *vendor_id = 0;
  *device_id = 0;

  GPUInfo gpu_info;
  CollectInfoResult result = CollectPCIVideoCardInfo(&gpu_info);
  if (result == kCollectInfoSuccess) {
    *vendor_id = gpu_info.gpu.vendor_id;
    *device_id = gpu_info.gpu.device_id;
  }
  return result;
}

#endif // defined(OS_LINUX)

#if defined(OS_MACOSX)

CollectInfoResult CollectGpuID(uint32* vendor_id, uint32* device_id) {
  DCHECK(vendor_id && device_id);

  GPUInfo::GPUDevice gpu = GetActiveGPU();
  *vendor_id = gpu.vendor_id;
  *device_id = gpu.device_id;

  if (*vendor_id != 0 && *device_id != 0)
    return kCollectInfoSuccess;
  return kCollectInfoNonFatalFailure;
}

#endif

namespace gpu {

namespace {

GPUTestConfig::OS GetCurrentOS() {
#if defined(OS_CHROMEOS)
  return GPUTestConfig::kOsChromeOS;
#elif defined(OS_LINUX) || defined(OS_OPENBSD)
  return GPUTestConfig::kOsLinux;
#elif defined(OS_WIN)
  int32 major_version = 0;
  int32 minor_version = 0;
  int32 bugfix_version = 0;
  base::SysInfo::OperatingSystemVersionNumbers(
      &major_version, &minor_version, &bugfix_version);
  if (major_version == 5)
    return GPUTestConfig::kOsWinXP;
  if (major_version == 6 && minor_version == 0)
    return GPUTestConfig::kOsWinVista;
  if (major_version == 6 && minor_version == 1)
    return GPUTestConfig::kOsWin7;
  if (major_version == 6 && (minor_version == 2 || minor_version == 3))
    return GPUTestConfig::kOsWin8;
  if (major_version == 10)
    return GPUTestConfig::kOsWin10;
#elif defined(OS_MACOSX)
  int32 major_version = 0;
  int32 minor_version = 0;
  int32 bugfix_version = 0;
  base::SysInfo::OperatingSystemVersionNumbers(
      &major_version, &minor_version, &bugfix_version);
  if (major_version == 10) {
    switch (minor_version) {
      case 5:
        return GPUTestConfig::kOsMacLeopard;
      case 6:
        return GPUTestConfig::kOsMacSnowLeopard;
      case 7:
        return GPUTestConfig::kOsMacLion;
      case 8:
        return GPUTestConfig::kOsMacMountainLion;
      case 9:
        return GPUTestConfig::kOsMacMavericks;
      case 10:
        return GPUTestConfig::kOsMacYosemite;
    }
  }
#elif defined(OS_ANDROID)
  return GPUTestConfig::kOsAndroid;
#endif
  return GPUTestConfig::kOsUnknown;
}

}  // namespace anonymous

GPUTestConfig::GPUTestConfig()
    : validate_gpu_info_(true),
      os_(kOsUnknown),
      gpu_device_id_(0),
      build_type_(kBuildTypeUnknown),
      api_(kAPIUnknown) {}

GPUTestConfig::~GPUTestConfig() {
}

void GPUTestConfig::set_os(int32 os) {
  DCHECK_EQ(0, os & ~(kOsAndroid | kOsWin | kOsMac | kOsLinux | kOsChromeOS));
  os_ = os;
}

void GPUTestConfig::AddGPUVendor(uint32 gpu_vendor) {
  DCHECK_NE(0u, gpu_vendor);
  for (size_t i = 0; i < gpu_vendor_.size(); ++i)
    DCHECK_NE(gpu_vendor_[i], gpu_vendor);
  gpu_vendor_.push_back(gpu_vendor);
}

void GPUTestConfig::set_gpu_device_id(uint32 id) {
  gpu_device_id_ = id;
}

void GPUTestConfig::set_build_type(int32 build_type) {
  DCHECK_EQ(0, build_type & ~(kBuildTypeRelease | kBuildTypeDebug));
  build_type_ = build_type;
}

void GPUTestConfig::set_api(int32 api) {
  DCHECK_EQ(0, api & ~(kAPID3D9 | kAPID3D11 | kAPIGLDesktop | kAPIGLES));
  api_ = api;
}

bool GPUTestConfig::IsValid() const {
  if (!validate_gpu_info_)
    return true;
  if (gpu_device_id_ != 0 && (gpu_vendor_.size() != 1 || gpu_vendor_[0] == 0))
    return false;
  return true;
}

bool GPUTestConfig::OverlapsWith(const GPUTestConfig& config) const {
  DCHECK(IsValid());
  DCHECK(config.IsValid());
  if (config.os_ != kOsUnknown && os_ != kOsUnknown &&
      (os_ & config.os_) == 0)
    return false;
  if (config.gpu_vendor_.size() > 0 && gpu_vendor_.size() > 0) {
    bool shared = false;
    for (size_t i = 0; i < config.gpu_vendor_.size() && !shared; ++i) {
      for (size_t j = 0; j < gpu_vendor_.size(); ++j) {
        if (config.gpu_vendor_[i] == gpu_vendor_[j]) {
          shared = true;
          break;
        }
      }
    }
    if (!shared)
      return false;
  }
  if (config.gpu_device_id_ != 0 && gpu_device_id_ != 0 &&
      gpu_device_id_ != config.gpu_device_id_)
    return false;
  if (config.build_type_ != kBuildTypeUnknown &&
      build_type_ != kBuildTypeUnknown &&
      (build_type_ & config.build_type_) == 0)
    return false;
  return true;
}

void GPUTestConfig::DisableGPUInfoValidation() {
  validate_gpu_info_ = false;
}

void GPUTestConfig::ClearGPUVendor() {
  gpu_vendor_.clear();
}

GPUTestBotConfig::~GPUTestBotConfig() {
}

void GPUTestBotConfig::AddGPUVendor(uint32 gpu_vendor) {
  DCHECK_EQ(0u, GPUTestConfig::gpu_vendor().size());
  GPUTestConfig::AddGPUVendor(gpu_vendor);
}

bool GPUTestBotConfig::SetGPUInfo(const GPUInfo& gpu_info) {
  DCHECK(validate_gpu_info_);
  if (gpu_info.gpu.device_id == 0 || gpu_info.gpu.vendor_id == 0)
    return false;
  ClearGPUVendor();
  AddGPUVendor(gpu_info.gpu.vendor_id);
  set_gpu_device_id(gpu_info.gpu.device_id);
  return true;
}

bool GPUTestBotConfig::IsValid() const {
  switch (os()) {
    case kOsWinXP:
    case kOsWinVista:
    case kOsWin7:
    case kOsWin8:
    case kOsWin10:
    case kOsMacLeopard:
    case kOsMacSnowLeopard:
    case kOsMacLion:
    case kOsMacMountainLion:
    case kOsMacMavericks:
    case kOsMacYosemite:
    case kOsLinux:
    case kOsChromeOS:
    case kOsAndroid:
      break;
    default:
      return false;
  }
  if (validate_gpu_info_) {
    if (gpu_vendor().size() != 1 || gpu_vendor()[0] == 0)
      return false;
    if (gpu_device_id() == 0)
      return false;
  }
  switch (build_type()) {
    case kBuildTypeRelease:
    case kBuildTypeDebug:
      break;
    default:
      return false;
  }
  return true;
}

bool GPUTestBotConfig::Matches(const GPUTestConfig& config) const {
  DCHECK(IsValid());
  DCHECK(config.IsValid());
  if (config.os() != kOsUnknown && (os() & config.os()) == 0)
    return false;
  if (config.gpu_vendor().size() > 0) {
    bool contained = false;
    for (size_t i = 0; i < config.gpu_vendor().size(); ++i) {
      if (config.gpu_vendor()[i] == gpu_vendor()[0]) {
        contained = true;
        break;
      }
    }
    if (!contained)
      return false;
  }
  if (config.gpu_device_id() != 0 &&
      gpu_device_id() != config.gpu_device_id())
    return false;
  if (config.build_type() != kBuildTypeUnknown &&
      (build_type() & config.build_type()) == 0)
    return false;
  if (config.api() != 0 && (api() & config.api()) == 0)
    return false;
  return true;
}

bool GPUTestBotConfig::Matches(const std::string& config_data) const {
  GPUTestExpectationsParser parser;
  GPUTestConfig config;

  if (!parser.ParseConfig(config_data, &config))
    return false;
  return Matches(config);
}

bool GPUTestBotConfig::LoadCurrentConfig(const GPUInfo* gpu_info) {
  bool rt;
  if (gpu_info == NULL) {
    GPUInfo my_gpu_info;
    CollectInfoResult result = CollectGpuID(
        &my_gpu_info.gpu.vendor_id, &my_gpu_info.gpu.device_id);
    if (result != kCollectInfoSuccess) {
      LOG(ERROR) << "Fail to identify GPU";
      DisableGPUInfoValidation();
      rt = true;
    } else {
      rt = SetGPUInfo(my_gpu_info);
    }
  } else {
    rt = SetGPUInfo(*gpu_info);
  }
  set_os(GetCurrentOS());
  if (os() == kOsUnknown) {
    LOG(ERROR) << "Unknown OS";
    rt = false;
  }
#if defined(NDEBUG)
  set_build_type(kBuildTypeRelease);
#else
  set_build_type(kBuildTypeDebug);
#endif
  return rt;
}

// static
bool GPUTestBotConfig::CurrentConfigMatches(const std::string& config_data) {
  GPUTestBotConfig my_config;
  if (!my_config.LoadCurrentConfig(NULL))
    return false;
  return my_config.Matches(config_data);
}

// static
bool GPUTestBotConfig::CurrentConfigMatches(
    const std::vector<std::string>& configs) {
  GPUTestBotConfig my_config;
  if (!my_config.LoadCurrentConfig(NULL))
    return false;
  for (size_t i = 0 ; i < configs.size(); ++i) {
    if (my_config.Matches(configs[i]))
      return true;
  }
  return false;
}

}  // namespace gpu

