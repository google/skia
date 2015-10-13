// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef GPU_CONFIG_GPU_TEST_CONFIG_H_
#define GPU_CONFIG_GPU_TEST_CONFIG_H_

#include <string>
#include <vector>

#include "angle_config.h"

namespace gpu {

struct GPUInfo;

class GPU_EXPORT GPUTestConfig {
 public:
  enum OS {
    kOsUnknown = 0,
    kOsWinXP = 1 << 0,
    kOsWinVista = 1 << 1,
    kOsWin7 = 1 << 2,
    kOsWin8 = 1 << 3,
    kOsMacLeopard = 1 << 4,
    kOsMacSnowLeopard = 1 << 5,
    kOsMacLion = 1 << 6,
    kOsMacMountainLion = 1 << 7,
    kOsMacMavericks = 1 << 8,
    kOsMacYosemite = 1 << 9,
    kOsMac = kOsMacLeopard | kOsMacSnowLeopard | kOsMacLion |
             kOsMacMountainLion | kOsMacMavericks | kOsMacYosemite,
    kOsLinux = 1 << 10,
    kOsChromeOS = 1 << 11,
    kOsAndroid = 1 << 12,
    kOsWin10 = 1 << 13,
    kOsWin = kOsWinXP | kOsWinVista | kOsWin7 | kOsWin8 | kOsWin10,
  };

  enum BuildType {
    kBuildTypeUnknown = 0,
    kBuildTypeRelease = 1 << 0,
    kBuildTypeDebug = 1 << 1,
  };

  enum API {
    kAPIUnknown = 0,
    kAPID3D9 = 1 << 0,
    kAPID3D11 = 1 << 1,
    kAPIGLDesktop = 1 << 2,
    kAPIGLES = 1 << 3,
  };

  GPUTestConfig();
  virtual ~GPUTestConfig();

  void set_os(int32 os);
  void set_gpu_device_id(uint32 id);
  void set_build_type(int32 build_type);
  void set_api(int32 api);

  virtual void AddGPUVendor(uint32 gpu_vendor);

  int32 os() const { return os_; }
  const std::vector<uint32>& gpu_vendor() const { return gpu_vendor_; }
  uint32 gpu_device_id() const { return gpu_device_id_; }
  int32 build_type() const { return build_type_; }
  int32 api() const { return api_; }

  // Check if the config is valid. For example, if gpu_device_id_ is set, but
  // gpu_vendor_ is unknown, then it's invalid.
  virtual bool IsValid() const;

  // Check if two configs overlap, i.e., if there exists a config that matches
  // both configs.
  bool OverlapsWith(const GPUTestConfig& config) const;

  // Disable validation of GPU vendor and device ids.
  void DisableGPUInfoValidation();

 protected:
  void ClearGPUVendor();

  // Indicates that the OS has the notion of a numeric GPU vendor and device id
  // and this data should be validated.
  bool validate_gpu_info_;

 private:
  // operating system.
  int32 os_;

  // GPU vendor.
  std::vector<uint32> gpu_vendor_;

  // GPU device id (unique to each vendor).
  uint32 gpu_device_id_;

  // Release or Debug.
  int32 build_type_;

  // Back-end rendering APIs.
  int32 api_;
};

class GPU_EXPORT GPUTestBotConfig : public GPUTestConfig {
 public:
  GPUTestBotConfig() { }
  ~GPUTestBotConfig() override;

  // This should only be called when no gpu_vendor is added.
  void AddGPUVendor(uint32 gpu_vendor) override;

  // Return false if gpu_info does not have valid vendor_id and device_id.
  bool SetGPUInfo(const GPUInfo& gpu_info);

  // Check if the bot config is valid, i.e., if it is one valid test-bot
  // environment. For example, if a field is unknown, or if OS is not one
  // fully defined OS, then it's valid.
  bool IsValid() const override;

  // Check if a bot config matches a test config, i.e., the test config is a
  // superset of the bot config.
  bool Matches(const GPUTestConfig& config) const;
  bool Matches(const std::string& config_data) const;

  // Setup the config with the current gpu testing environment.
  // If gpu_info is NULL, collect GPUInfo first.
  bool LoadCurrentConfig(const GPUInfo* gpu_info);

  // Check if this bot's config matches |config_data| or any of the |configs|.
  static bool CurrentConfigMatches(const std::string& config_data);
  static bool CurrentConfigMatches(const std::vector<std::string>& configs);
};

}  // namespace gpu

#endif  // GPU_CONFIG_GPU_TEST_CONFIG_H_

