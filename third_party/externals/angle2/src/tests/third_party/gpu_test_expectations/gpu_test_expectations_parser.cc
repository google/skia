// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "gpu_test_expectations_parser.h"

#include "common/angleutils.h"

namespace base {

namespace {

bool StartsWithASCII(const std::string& str,
                     const std::string& search,
                     bool case_sensitive) {
  ASSERT(!case_sensitive);
  return str.compare(0, search.length(), search) == 0;
}

template <class Char> inline Char ToLowerASCII(Char c) {
  return (c >= 'A' && c <= 'Z') ? (c + ('a' - 'A')) : c;
}

template<typename Iter>
static inline bool DoLowerCaseEqualsASCII(Iter a_begin,
                                          Iter a_end,
                                          const char* b) {
  for (Iter it = a_begin; it != a_end; ++it, ++b) {
    if (!*b || base::ToLowerASCII(*it) != *b)
      return false;
  }
  return *b == 0;
}

bool LowerCaseEqualsASCII(const std::string& a, const char* b) {
  return DoLowerCaseEqualsASCII(a.begin(), a.end(), b);
}

} // anonymous namespace

} // namespace base

namespace gpu {

namespace {

enum LineParserStage {
  kLineParserBegin = 0,
  kLineParserBugID,
  kLineParserConfigs,
  kLineParserColon,
  kLineParserTestName,
  kLineParserEqual,
  kLineParserExpectations,
};

enum Token {
  // os
  kConfigWinXP = 0,
  kConfigWinVista,
  kConfigWin7,
  kConfigWin8,
  kConfigWin10,
  kConfigWin,
  kConfigMacLeopard,
  kConfigMacSnowLeopard,
  kConfigMacLion,
  kConfigMacMountainLion,
  kConfigMacMavericks,
  kConfigMacYosemite,
  kConfigMac,
  kConfigLinux,
  kConfigChromeOS,
  kConfigAndroid,
  // gpu vendor
  kConfigNVidia,
  kConfigAMD,
  kConfigIntel,
  kConfigVMWare,
  // build type
  kConfigRelease,
  kConfigDebug,
  // ANGLE renderer
  kConfigD3D9,
  kConfigD3D11,
  kConfigGLDesktop,
  kConfigGLES,
  // expectation
  kExpectationPass,
  kExpectationFail,
  kExpectationFlaky,
  kExpectationTimeout,
  kExpectationSkip,
  // separator
  kSeparatorColon,
  kSeparatorEqual,

  kNumberOfExactMatchTokens,

  // others
  kConfigGPUDeviceID,
  kTokenComment,
  kTokenWord,
};

struct TokenInfo {
  const char* name;
  int32 flag;
};

const TokenInfo kTokenData[] = {
    {"xp", GPUTestConfig::kOsWinXP},
    {"vista", GPUTestConfig::kOsWinVista},
    {"win7", GPUTestConfig::kOsWin7},
    {"win8", GPUTestConfig::kOsWin8},
    {"win10", GPUTestConfig::kOsWin10},
    {"win", GPUTestConfig::kOsWin},
    {"leopard", GPUTestConfig::kOsMacLeopard},
    {"snowleopard", GPUTestConfig::kOsMacSnowLeopard},
    {"lion", GPUTestConfig::kOsMacLion},
    {"mountainlion", GPUTestConfig::kOsMacMountainLion},
    {"mavericks", GPUTestConfig::kOsMacMavericks},
    {"yosemite", GPUTestConfig::kOsMacYosemite},
    {"mac", GPUTestConfig::kOsMac},
    {"linux", GPUTestConfig::kOsLinux},
    {"chromeos", GPUTestConfig::kOsChromeOS},
    {"android", GPUTestConfig::kOsAndroid},
    {"nvidia", 0x10DE},
    {"amd", 0x1002},
    {"intel", 0x8086},
    {"vmware", 0x15ad},
    {"release", GPUTestConfig::kBuildTypeRelease},
    {"debug", GPUTestConfig::kBuildTypeDebug},
    {"d3d9", GPUTestConfig::kAPID3D9},
    {"d3d11", GPUTestConfig::kAPID3D11},
    {"opengl", GPUTestConfig::kAPIGLDesktop},
    {"gles", GPUTestConfig::kAPIGLES},
    {"pass", GPUTestExpectationsParser::kGpuTestPass},
    {"fail", GPUTestExpectationsParser::kGpuTestFail},
    {"flaky", GPUTestExpectationsParser::kGpuTestFlaky},
    {"timeout", GPUTestExpectationsParser::kGpuTestTimeout},
    {"skip", GPUTestExpectationsParser::kGpuTestSkip},
    {":", 0},
    {"=", 0},
};

enum ErrorType {
  kErrorFileIO = 0,
  kErrorIllegalEntry,
  kErrorInvalidEntry,
  kErrorEntryWithOsConflicts,
  kErrorEntryWithGpuVendorConflicts,
  kErrorEntryWithBuildTypeConflicts,
  kErrorEntryWithAPIConflicts,
  kErrorEntryWithGpuDeviceIdConflicts,
  kErrorEntryWithExpectationConflicts,
  kErrorEntriesOverlap,

  kNumberOfErrors,
};

const char* kErrorMessage[] = {
    "file IO failed",
    "entry with wrong format",
    "entry invalid, likely wrong modifiers combination",
    "entry with OS modifier conflicts",
    "entry with GPU vendor modifier conflicts",
    "entry with GPU build type conflicts",
    "entry with GPU API conflicts",
    "entry with GPU device id conflicts or malformat",
    "entry with expectation modifier conflicts",
    "two entries' configs overlap",
};

Token ParseToken(const std::string& word) {
  if (base::StartsWithASCII(word, "//", false))
    return kTokenComment;
  if (base::StartsWithASCII(word, "0x", false))
    return kConfigGPUDeviceID;

  for (int32 i = 0; i < kNumberOfExactMatchTokens; ++i) {
    if (base::LowerCaseEqualsASCII(word, kTokenData[i].name))
      return static_cast<Token>(i);
  }
  return kTokenWord;
}

// reference name can have the last character as *.
bool NamesMatching(const std::string& ref, const std::string& test_name) {
  size_t len = ref.length();
  if (len == 0)
    return false;
  if (ref[len - 1] == '*') {
    if (test_name.length() > len -1 &&
        ref.compare(0, len - 1, test_name, 0, len - 1) == 0)
      return true;
    return false;
  }
  return (ref == test_name);
}

}  // namespace anonymous

GPUTestExpectationsParser::GPUTestExpectationsParser() {
  // Some sanity check.
  static_assert(static_cast<unsigned int>(kNumberOfExactMatchTokens) ==
                sizeof(kTokenData) / sizeof(kTokenData[0]), "sanity check");
  static_assert(static_cast<unsigned int>(kNumberOfErrors) ==
                sizeof(kErrorMessage) / sizeof(kErrorMessage[0]), "sanity check");
}

GPUTestExpectationsParser::~GPUTestExpectationsParser() {
}

bool GPUTestExpectationsParser::LoadTestExpectations(const std::string& data) {
  entries_.clear();
  error_messages_.clear();

  std::vector<std::string> lines;
  base::SplitString(data, '\n', &lines);
  bool rt = true;
  for (size_t i = 0; i < lines.size(); ++i) {
    if (!ParseLine(lines[i], i + 1))
      rt = false;
  }
  if (DetectConflictsBetweenEntries()) {
    entries_.clear();
    rt = false;
  }

  return rt;
}

bool GPUTestExpectationsParser::LoadTestExpectationsFromFile(
    const std::string& path) {
  entries_.clear();
  error_messages_.clear();

  std::string data;
  if (!base::ReadFileToString(path, &data)) {
    error_messages_.push_back(kErrorMessage[kErrorFileIO]);
    return false;
  }
  return LoadTestExpectations(data);
}

int32 GPUTestExpectationsParser::GetTestExpectation(
    const std::string& test_name,
    const GPUTestBotConfig& bot_config) const {
  for (size_t i = 0; i < entries_.size(); ++i) {
    if (NamesMatching(entries_[i].test_name, test_name) &&
        bot_config.Matches(entries_[i].test_config))
      return entries_[i].test_expectation;
  }
  return kGpuTestPass;
}

const std::vector<std::string>&
GPUTestExpectationsParser::GetErrorMessages() const {
  return error_messages_;
}

bool GPUTestExpectationsParser::ParseConfig(
    const std::string& config_data, GPUTestConfig* config) {
  DCHECK(config);
  std::vector<std::string> tokens;
  base::SplitStringAlongWhitespace(config_data, &tokens);

  for (size_t i = 0; i < tokens.size(); ++i) {
    Token token = ParseToken(tokens[i]);
    switch (token) {
      case kConfigWinXP:
      case kConfigWinVista:
      case kConfigWin7:
      case kConfigWin8:
      case kConfigWin10:
      case kConfigWin:
      case kConfigMacLeopard:
      case kConfigMacSnowLeopard:
      case kConfigMacLion:
      case kConfigMacMountainLion:
      case kConfigMacMavericks:
      case kConfigMacYosemite:
      case kConfigMac:
      case kConfigLinux:
      case kConfigChromeOS:
      case kConfigAndroid:
      case kConfigNVidia:
      case kConfigAMD:
      case kConfigIntel:
      case kConfigVMWare:
      case kConfigRelease:
      case kConfigDebug:
      case kConfigD3D9:
      case kConfigD3D11:
      case kConfigGLDesktop:
      case kConfigGLES:
      case kConfigGPUDeviceID:
        if (token == kConfigGPUDeviceID) {
          if (!UpdateTestConfig(config, tokens[i], 0))
            return false;
        } else {
          if (!UpdateTestConfig(config, token, 0))
            return false;
        }
        break;
      default:
        return false;
    }
  }
  return true;
}

bool GPUTestExpectationsParser::ParseLine(
    const std::string& line_data, size_t line_number) {
  std::vector<std::string> tokens;
  base::SplitStringAlongWhitespace(line_data, &tokens);
  int32 stage = kLineParserBegin;
  GPUTestExpectationEntry entry;
  entry.line_number = line_number;
  GPUTestConfig& config = entry.test_config;
  bool comments_encountered = false;
  for (size_t i = 0; i < tokens.size() && !comments_encountered; ++i) {
    Token token = ParseToken(tokens[i]);
    switch (token) {
      case kTokenComment:
        comments_encountered = true;
        break;
      case kConfigWinXP:
      case kConfigWinVista:
      case kConfigWin7:
      case kConfigWin8:
      case kConfigWin10:
      case kConfigWin:
      case kConfigMacLeopard:
      case kConfigMacSnowLeopard:
      case kConfigMacLion:
      case kConfigMacMountainLion:
      case kConfigMacMavericks:
      case kConfigMacYosemite:
      case kConfigMac:
      case kConfigLinux:
      case kConfigChromeOS:
      case kConfigAndroid:
      case kConfigNVidia:
      case kConfigAMD:
      case kConfigIntel:
      case kConfigVMWare:
      case kConfigRelease:
      case kConfigDebug:
      case kConfigD3D9:
      case kConfigD3D11:
      case kConfigGLDesktop:
      case kConfigGLES:
      case kConfigGPUDeviceID:
        // MODIFIERS, could be in any order, need at least one.
        if (stage != kLineParserConfigs && stage != kLineParserBugID) {
          PushErrorMessage(kErrorMessage[kErrorIllegalEntry],
                           line_number);
          return false;
        }
        if (token == kConfigGPUDeviceID) {
          if (!UpdateTestConfig(&config, tokens[i], line_number))
            return false;
        } else {
          if (!UpdateTestConfig(&config, token, line_number))
            return false;
        }
        if (stage == kLineParserBugID)
          stage++;
        break;
      case kSeparatorColon:
        // :
        if (stage != kLineParserConfigs) {
          PushErrorMessage(kErrorMessage[kErrorIllegalEntry],
                           line_number);
          return false;
        }
        stage++;
        break;
      case kSeparatorEqual:
        // =
        if (stage != kLineParserTestName) {
          PushErrorMessage(kErrorMessage[kErrorIllegalEntry],
                           line_number);
          return false;
        }
        stage++;
        break;
      case kTokenWord:
        // BUG_ID or TEST_NAME
        if (stage == kLineParserBegin) {
          // Bug ID is not used for anything; ignore it.
        } else if (stage == kLineParserColon) {
          entry.test_name = tokens[i];
        } else {
          PushErrorMessage(kErrorMessage[kErrorIllegalEntry],
                           line_number);
          return false;
        }
        stage++;
        break;
      case kExpectationPass:
      case kExpectationFail:
      case kExpectationFlaky:
      case kExpectationTimeout:
      case kExpectationSkip:
        // TEST_EXPECTATIONS
        if (stage != kLineParserEqual && stage != kLineParserExpectations) {
          PushErrorMessage(kErrorMessage[kErrorIllegalEntry],
                           line_number);
          return false;
        }
        if ((kTokenData[token].flag & entry.test_expectation) != 0) {
          PushErrorMessage(kErrorMessage[kErrorEntryWithExpectationConflicts],
                           line_number);
          return false;
        }
        entry.test_expectation =
            (kTokenData[token].flag | entry.test_expectation);
        if (stage == kLineParserEqual)
          stage++;
        break;
      default:
        UNREACHABLE();
        break;
    }
  }
  if (stage == kLineParserBegin) {
    // The whole line is empty or all comments
    return true;
  }
  if (stage == kLineParserExpectations) {
    if (!config.IsValid()) {
        PushErrorMessage(kErrorMessage[kErrorInvalidEntry], line_number);
        return false;
    }
    entries_.push_back(entry);
    return true;
  }
  PushErrorMessage(kErrorMessage[kErrorIllegalEntry], line_number);
  return false;
}

bool GPUTestExpectationsParser::UpdateTestConfig(
    GPUTestConfig* config, int32 token, size_t line_number) {
  DCHECK(config);
  switch (token) {
    case kConfigWinXP:
    case kConfigWinVista:
    case kConfigWin7:
    case kConfigWin8:
    case kConfigWin10:
    case kConfigWin:
    case kConfigMacLeopard:
    case kConfigMacSnowLeopard:
    case kConfigMacLion:
    case kConfigMacMountainLion:
    case kConfigMacMavericks:
    case kConfigMacYosemite:
    case kConfigMac:
    case kConfigLinux:
    case kConfigChromeOS:
    case kConfigAndroid:
      if ((config->os() & kTokenData[token].flag) != 0) {
        PushErrorMessage(kErrorMessage[kErrorEntryWithOsConflicts],
                         line_number);
        return false;
      }
      config->set_os(config->os() | kTokenData[token].flag);
      break;
    case kConfigNVidia:
    case kConfigAMD:
    case kConfigIntel:
    case kConfigVMWare:
      {
        uint32 gpu_vendor =
            static_cast<uint32>(kTokenData[token].flag);
        for (size_t i = 0; i < config->gpu_vendor().size(); ++i) {
          if (config->gpu_vendor()[i] == gpu_vendor) {
            PushErrorMessage(
                kErrorMessage[kErrorEntryWithGpuVendorConflicts],
                line_number);
            return false;
          }
        }
        config->AddGPUVendor(gpu_vendor);
      }
      break;
    case kConfigRelease:
    case kConfigDebug:
      if ((config->build_type() & kTokenData[token].flag) != 0) {
        PushErrorMessage(
            kErrorMessage[kErrorEntryWithBuildTypeConflicts],
            line_number);
        return false;
      }
      config->set_build_type(
          config->build_type() | kTokenData[token].flag);
      break;
    case kConfigD3D9:
    case kConfigD3D11:
    case kConfigGLDesktop:
    case kConfigGLES:
      if ((config->api() & kTokenData[token].flag) != 0) {
        PushErrorMessage(kErrorMessage[kErrorEntryWithAPIConflicts],
                         line_number);
        return false;
      }
      config->set_api(config->api() | kTokenData[token].flag);
      break;
    default:
      UNREACHABLE();
      break;
  }
  return true;
}

bool GPUTestExpectationsParser::UpdateTestConfig(
    GPUTestConfig* config,
    const std::string& gpu_device_id,
    size_t line_number) {
  DCHECK(config);
  uint32 device_id = 0;
  if (config->gpu_device_id() != 0 ||
      !base::HexStringToUInt(gpu_device_id, &device_id) ||
      device_id == 0) {
    PushErrorMessage(kErrorMessage[kErrorEntryWithGpuDeviceIdConflicts],
                     line_number);
    return false;
  }
  config->set_gpu_device_id(device_id);
  return true;
}

bool GPUTestExpectationsParser::DetectConflictsBetweenEntries() {
  bool rt = false;
  for (size_t i = 0; i < entries_.size(); ++i) {
    for (size_t j = i + 1; j < entries_.size(); ++j) {
      if (entries_[i].test_name == entries_[j].test_name &&
          entries_[i].test_config.OverlapsWith(entries_[j].test_config)) {
        PushErrorMessage(kErrorMessage[kErrorEntriesOverlap],
                         entries_[i].line_number,
                         entries_[j].line_number);
        rt = true;
      }
    }
  }
  return rt;
}

void GPUTestExpectationsParser::PushErrorMessage(
    const std::string& message, size_t line_number) {
  error_messages_.push_back(
      base::StringPrintf("Line %d : %s",
                         static_cast<int>(line_number), message.c_str()));
}

void GPUTestExpectationsParser::PushErrorMessage(
    const std::string& message,
    size_t entry1_line_number,
    size_t entry2_line_number) {
  error_messages_.push_back(
      base::StringPrintf("Line %d and %d : %s",
                         static_cast<int>(entry1_line_number),
                         static_cast<int>(entry2_line_number),
                         message.c_str()));
}

GPUTestExpectationsParser:: GPUTestExpectationEntry::GPUTestExpectationEntry()
    : test_expectation(0),
      line_number(0) {
}

}  // namespace gpu

