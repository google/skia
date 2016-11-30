// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package main

/*
	Generate the tasks.json file.
*/

import (
	"encoding/json"
	"fmt"
	"os"
	"path"
	"sort"
	"strings"
	"time"

	"github.com/skia-dev/glog"
	"go.skia.org/infra/go/util"
	"go.skia.org/infra/task_scheduler/go/specs"
)

const (
	DEFAULT_OS = "Ubuntu"

	// Pool for Skia bots.
	POOL_SKIA = "Skia"

	// Name prefix for upload jobs.
	PREFIX_UPLOAD = "Upload"
)

var (
	// "Constants"

	// Top-level list of all jobs to run at each commit.
	JOBS = []string{
		"Build-Mac-Clang-Arm7-Debug-iOS",
		"Build-Mac-Clang-Arm7-Release-iOS",
		"Build-Mac-Clang-arm64-Debug-GN_Android",
		"Build-Mac-Clang-arm64-Debug-GN_iOS",
		"Build-Mac-Clang-x86_64-Debug-GN",
		"Build-Mac-Clang-x86_64-Release-GN",
		"Build-Ubuntu-Clang-arm-Debug-GN_Android",
		"Build-Ubuntu-Clang-arm-Release-GN_Android",
		"Build-Ubuntu-Clang-arm64-Debug-GN_Android",
		"Build-Ubuntu-Clang-arm64-Debug-GN_Android_FrameworkDefs",
		"Build-Ubuntu-Clang-arm64-Debug-GN_Android_Vulkan",
		"Build-Ubuntu-Clang-arm64-Release-GN_Android",
		"Build-Ubuntu-Clang-arm64-Release-GN_Android_Vulkan",
		"Build-Ubuntu-Clang-mips64el-Debug-GN_Android",
		"Build-Ubuntu-Clang-mips64el-Release-GN_Android",
		"Build-Ubuntu-Clang-mipsel-Debug-GN_Android",
		"Build-Ubuntu-Clang-mipsel-Release-GN_Android",
		"Build-Ubuntu-Clang-x64-Debug-GN_Android",
		"Build-Ubuntu-Clang-x64-Release-GN_Android",
		"Build-Ubuntu-Clang-x86-Debug-GN_Android",
		"Build-Ubuntu-Clang-x86-Debug-GN_Android_Vulkan",
		"Build-Ubuntu-Clang-x86-Release-GN_Android",
		"Build-Ubuntu-Clang-x86-Release-GN_Android_Vulkan",
		"Build-Ubuntu-Clang-x86_64-Debug-GN",
		"Build-Ubuntu-Clang-x86_64-Release-GN",
		"Build-Ubuntu-GCC-x86-Debug",
		"Build-Ubuntu-GCC-x86-Release",
		"Build-Ubuntu-GCC-x86_64-Debug-GN",
		"Build-Ubuntu-GCC-x86_64-Debug-NoGPU",
		"Build-Ubuntu-GCC-x86_64-Release-ANGLE",
		"Build-Ubuntu-GCC-x86_64-Release-GN",
		"Build-Ubuntu-GCC-x86_64-Release-Mesa",
		"Build-Ubuntu-GCC-x86_64-Release-NoGPU",
		"Build-Ubuntu-GCC-x86_64-Release-PDFium",
		"Build-Ubuntu-GCC-x86_64-Release-Valgrind",
		"Build-Win-Clang-arm64-Release-GN_Android",
		"Build-Win-MSVC-x86-Debug",
		"Build-Win-MSVC-x86-Debug-ANGLE",
		"Build-Win-MSVC-x86-Debug-Exceptions",
		"Build-Win-MSVC-x86-Debug-GDI",
		"Build-Win-MSVC-x86-Release",
		"Build-Win-MSVC-x86-Release-ANGLE",
		"Build-Win-MSVC-x86-Release-GDI",
		"Build-Win-MSVC-x86_64-Debug",
		"Build-Win-MSVC-x86_64-Debug-GN",
		"Build-Win-MSVC-x86_64-Release",
		"Build-Win-MSVC-x86_64-Release-GN",
		"Build-Win-MSVC-x86_64-Release-Vulkan",
		"Housekeeper-Nightly-RecreateSKPs_Canary",
		"Housekeeper-PerCommit-InfraTests",
		"Housekeeper-Weekly-RecreateSKPs",
		"Perf-Android-Clang-AndroidOne-CPU-MT6582-arm-Debug-GN_Android",
		"Perf-Android-Clang-AndroidOne-CPU-MT6582-arm-Release-GN_Android",
		"Perf-Android-Clang-AndroidOne-GPU-Mali400MP2-arm-Debug-GN_Android",
		"Perf-Android-Clang-AndroidOne-GPU-Mali400MP2-arm-Release-GN_Android",
		"Perf-Android-Clang-GalaxyS7-GPU-Adreno530-arm64-Debug-GN_Android",
		"Perf-Android-Clang-GalaxyS7-GPU-Adreno530-arm64-Debug-GN_Android_Vulkan",
		"Perf-Android-Clang-GalaxyS7-GPU-Adreno530-arm64-Release-GN_Android",
		"Perf-Android-Clang-GalaxyS7-GPU-Adreno530-arm64-Release-GN_Android_Vulkan",
		"Perf-Android-Clang-NVIDIA_Shield-GPU-TegraX1-arm64-Debug-GN_Android",
		"Perf-Android-Clang-NVIDIA_Shield-GPU-TegraX1-arm64-Debug-GN_Android_Vulkan",
		"Perf-Android-Clang-NVIDIA_Shield-GPU-TegraX1-arm64-Release-GN_Android",
		"Perf-Android-Clang-NVIDIA_Shield-GPU-TegraX1-arm64-Release-GN_Android_Vulkan",
		"Perf-Android-Clang-Nexus10-GPU-MaliT604-arm-Debug-GN_Android",
		"Perf-Android-Clang-Nexus10-GPU-MaliT604-arm-Release-GN_Android",
		"Perf-Android-Clang-Nexus5-GPU-Adreno330-arm-Debug-GN_Android",
		"Perf-Android-Clang-Nexus5-GPU-Adreno330-arm-Release-GN_Android",
		"Perf-Android-Clang-Nexus6p-CPU-Snapdragon810-arm64-Debug-GN_Android",
		"Perf-Android-Clang-Nexus6p-CPU-Snapdragon810-arm64-Release-GN_Android",
		"Perf-Android-Clang-Nexus6p-GPU-Adreno430-arm64-Debug-GN_Android",
		"Perf-Android-Clang-Nexus6p-GPU-Adreno430-arm64-Debug-GN_Android_Vulkan",
		"Perf-Android-Clang-Nexus6p-GPU-Adreno430-arm64-Release-GN_Android",
		"Perf-Android-Clang-Nexus6p-GPU-Adreno430-arm64-Release-GN_Android_Vulkan",
		"Perf-Android-Clang-Nexus7-GPU-Tegra3-arm-Debug-GN_Android",
		"Perf-Android-Clang-Nexus7-GPU-Tegra3-arm-Release-GN_Android",
		"Perf-Android-Clang-Nexus9-GPU-TegraK1-arm64-Debug-GN_Android",
		"Perf-Android-Clang-Nexus9-GPU-TegraK1-arm64-Release-GN_Android",
		"Perf-Android-Clang-NexusPlayer-CPU-Moorefield-x86-Debug-GN_Android",
		"Perf-Android-Clang-NexusPlayer-CPU-Moorefield-x86-Release-GN_Android",
		"Perf-Android-Clang-NexusPlayer-GPU-PowerVR-x86-Debug-GN_Android",
		"Perf-Android-Clang-NexusPlayer-GPU-PowerVR-x86-Debug-GN_Android_Vulkan",
		"Perf-Android-Clang-NexusPlayer-GPU-PowerVR-x86-Release-GN_Android",
		"Perf-Android-Clang-NexusPlayer-GPU-PowerVR-x86-Release-GN_Android_Vulkan",
		"Perf-Android-Clang-Pixel-GPU-Adreno530-arm64-Debug-GN_Android",
		"Perf-Android-Clang-Pixel-GPU-Adreno530-arm64-Debug-GN_Android_Vulkan",
		"Perf-Android-Clang-Pixel-GPU-Adreno530-arm64-Release-GN_Android",
		"Perf-Android-Clang-Pixel-GPU-Adreno530-arm64-Release-GN_Android_Vulkan",
		"Perf-Android-Clang-PixelC-GPU-TegraX1-arm64-Release-GN_Android_Skpbench",
		"Perf-Android-Clang-PixelC-GPU-TegraX1-arm64-Release-GN_Android_Vulkan_Skpbench",
		"Perf-Mac-Clang-MacMini4.1-GPU-GeForce320M-x86_64-Debug-GN",
		"Perf-Mac-Clang-MacMini4.1-GPU-GeForce320M-x86_64-Release-GN",
		"Perf-Mac-Clang-MacMini6.2-CPU-AVX-x86_64-Debug-GN",
		"Perf-Mac-Clang-MacMini6.2-CPU-AVX-x86_64-Release-GN",
		"Perf-Mac-Clang-MacMini6.2-GPU-HD4000-x86_64-Debug-GN",
		"Perf-Mac-Clang-MacMini6.2-GPU-HD4000-x86_64-Release-CommandBuffer",
		"Perf-Mac-Clang-MacMini6.2-GPU-HD4000-x86_64-Release-GN",
		"Perf-Ubuntu-Clang-GCE-CPU-AVX2-x86_64-Debug-ASAN",
		"Perf-Ubuntu-Clang-GCE-CPU-AVX2-x86_64-Debug-GN",
		"Perf-Ubuntu-Clang-GCE-CPU-AVX2-x86_64-Debug-MSAN",
		"Perf-Ubuntu-Clang-GCE-CPU-AVX2-x86_64-Release-GN",
		"Perf-Ubuntu-Clang-Golo-GPU-GT610-x86_64-Debug-ASAN",
		"Perf-Ubuntu-GCC-GCE-CPU-AVX2-x86-Debug",
		"Perf-Ubuntu-GCC-GCE-CPU-AVX2-x86_64-Debug-GN",
		"Perf-Ubuntu-GCC-GCE-CPU-AVX2-x86_64-Debug-SK_USE_DISCARDABLE_SCALEDIMAGECACHE",
		"Perf-Ubuntu-GCC-GCE-CPU-AVX2-x86_64-Release-CT_BENCH_1k_SKPs",
		"Perf-Ubuntu-GCC-GCE-CPU-AVX2-x86_64-Release-GN",
		"Perf-Ubuntu-GCC-Golo-GPU-GT610-x86_64-Release-CT_BENCH_1k_SKPs",
		"Perf-Ubuntu-GCC-ShuttleA-GPU-GTX550Ti-x86_64-Release-Valgrind",
		"Perf-Ubuntu-GCC-ShuttleA-GPU-GTX660-x86_64-Debug-GN",
		"Perf-Ubuntu-GCC-ShuttleA-GPU-GTX660-x86_64-Release-GN",
		"Perf-Win-MSVC-GCE-CPU-AVX2-x86-Debug",
		"Perf-Win-MSVC-GCE-CPU-AVX2-x86_64-Debug",
		"Perf-Win-MSVC-GCE-CPU-AVX2-x86_64-Debug-GDI",
		"Perf-Win-MSVC-GCE-CPU-AVX2-x86_64-Release",
		"Perf-Win-MSVC-Golo-GPU-GT610-x86_64-Release",
		"Perf-Win-MSVC-NUC-GPU-IntelIris6100-x86_64-Debug",
		"Perf-Win-MSVC-NUC-GPU-IntelIris6100-x86_64-Release",
		"Perf-Win-MSVC-ShuttleC-GPU-GTX960-x86_64-Debug",
		"Perf-Win-MSVC-ShuttleC-GPU-GTX960-x86_64-Debug-ANGLE",
		"Perf-Win-MSVC-ShuttleC-GPU-GTX960-x86_64-Release",
		"Perf-Win-MSVC-ShuttleC-GPU-GTX960-x86_64-Release-ANGLE",
		"Perf-Win-MSVC-ShuttleC-GPU-iHD530-x86_64-Debug",
		"Perf-Win-MSVC-ShuttleC-GPU-iHD530-x86_64-Release",
		"Perf-Win10-MSVC-ShuttleA-GPU-GTX660-x86_64-Debug",
		"Perf-Win10-MSVC-ShuttleA-GPU-GTX660-x86_64-Debug-Vulkan",
		"Perf-Win10-MSVC-ShuttleA-GPU-GTX660-x86_64-Release",
		"Perf-Win10-MSVC-ShuttleA-GPU-GTX660-x86_64-Release-Vulkan",
		"Perf-Win8-MSVC-ShuttleA-GPU-HD7770-x86_64-Debug",
		"Perf-Win8-MSVC-ShuttleA-GPU-HD7770-x86_64-Release",
		"Perf-Win8-MSVC-ShuttleB-GPU-HD4600-x86_64-Debug",
		"Perf-Win8-MSVC-ShuttleB-GPU-HD4600-x86_64-Release",
		"Perf-iOS-Clang-iPadMini4-GPU-GX6450-Arm7-Debug",
		"Perf-iOS-Clang-iPadMini4-GPU-GX6450-Arm7-Release",
		"Test-Android-Clang-AndroidOne-CPU-MT6582-arm-Debug-GN_Android",
		"Test-Android-Clang-AndroidOne-CPU-MT6582-arm-Release-GN_Android",
		"Test-Android-Clang-AndroidOne-GPU-Mali400MP2-arm-Debug-GN_Android",
		"Test-Android-Clang-AndroidOne-GPU-Mali400MP2-arm-Release-GN_Android",
		"Test-Android-Clang-GalaxyS7-GPU-Adreno530-arm64-Debug-GN_Android",
		"Test-Android-Clang-GalaxyS7-GPU-Adreno530-arm64-Debug-GN_Android_Vulkan",
		"Test-Android-Clang-GalaxyS7-GPU-Adreno530-arm64-Release-GN_Android",
		"Test-Android-Clang-GalaxyS7-GPU-Adreno530-arm64-Release-GN_Android_Vulkan",
		"Test-Android-Clang-NVIDIA_Shield-GPU-TegraX1-arm64-Debug-GN_Android",
		"Test-Android-Clang-NVIDIA_Shield-GPU-TegraX1-arm64-Debug-GN_Android_Vulkan",
		"Test-Android-Clang-NVIDIA_Shield-GPU-TegraX1-arm64-Release-GN_Android",
		"Test-Android-Clang-NVIDIA_Shield-GPU-TegraX1-arm64-Release-GN_Android_Vulkan",
		"Test-Android-Clang-Nexus10-GPU-MaliT604-arm-Debug-GN_Android",
		"Test-Android-Clang-Nexus10-GPU-MaliT604-arm-Release-GN_Android",
		"Test-Android-Clang-Nexus5-GPU-Adreno330-arm-Debug-GN_Android",
		"Test-Android-Clang-Nexus5-GPU-Adreno330-arm-Release-GN_Android",
		"Test-Android-Clang-Nexus6p-CPU-Snapdragon810-arm64-Debug-GN_Android",
		"Test-Android-Clang-Nexus6p-CPU-Snapdragon810-arm64-Release-GN_Android",
		"Test-Android-Clang-Nexus6p-GPU-Adreno430-arm64-Debug-GN_Android",
		"Test-Android-Clang-Nexus6p-GPU-Adreno430-arm64-Debug-GN_Android_Vulkan",
		"Test-Android-Clang-Nexus6p-GPU-Adreno430-arm64-Release-GN_Android",
		"Test-Android-Clang-Nexus6p-GPU-Adreno430-arm64-Release-GN_Android_Vulkan",
		"Test-Android-Clang-Nexus7-GPU-Tegra3-arm-Debug-GN_Android",
		"Test-Android-Clang-Nexus7-GPU-Tegra3-arm-Release-GN_Android",
		"Test-Android-Clang-Nexus9-GPU-TegraK1-arm64-Debug-GN_Android",
		"Test-Android-Clang-Nexus9-GPU-TegraK1-arm64-Release-GN_Android",
		"Test-Android-Clang-NexusPlayer-CPU-Moorefield-x86-Debug-GN_Android",
		"Test-Android-Clang-NexusPlayer-CPU-Moorefield-x86-Release-GN_Android",
		"Test-Android-Clang-NexusPlayer-GPU-PowerVR-x86-Debug-GN_Android",
		"Test-Android-Clang-NexusPlayer-GPU-PowerVR-x86-Debug-GN_Android_Vulkan",
		"Test-Android-Clang-NexusPlayer-GPU-PowerVR-x86-Release-GN_Android",
		"Test-Android-Clang-NexusPlayer-GPU-PowerVR-x86-Release-GN_Android_Vulkan",
		"Test-Android-Clang-PixelXL-GPU-Adreno530-arm64-Debug-GN_Android",
		"Test-Android-Clang-PixelXL-GPU-Adreno530-arm64-Debug-GN_Android_Vulkan",
		"Test-Android-Clang-PixelXL-GPU-Adreno530-arm64-Release-GN_Android",
		"Test-Android-Clang-PixelXL-GPU-Adreno530-arm64-Release-GN_Android_Vulkan",
		"Test-Mac-Clang-MacMini4.1-GPU-GeForce320M-x86_64-Debug-GN",
		"Test-Mac-Clang-MacMini4.1-GPU-GeForce320M-x86_64-Release-GN",
		"Test-Mac-Clang-MacMini6.2-CPU-AVX-x86_64-Debug-GN",
		"Test-Mac-Clang-MacMini6.2-CPU-AVX-x86_64-Release-GN",
		"Test-Mac-Clang-MacMini6.2-GPU-HD4000-x86_64-Debug-CommandBuffer",
		"Test-Mac-Clang-MacMini6.2-GPU-HD4000-x86_64-Debug-GN",
		"Test-Mac-Clang-MacMini6.2-GPU-HD4000-x86_64-Release-GN",
		"Test-Ubuntu-Clang-GCE-CPU-AVX2-x86_64-Debug-ASAN",
		"Test-Ubuntu-Clang-GCE-CPU-AVX2-x86_64-Debug-GN",
		"Test-Ubuntu-Clang-GCE-CPU-AVX2-x86_64-Debug-MSAN",
		"Test-Ubuntu-Clang-GCE-CPU-AVX2-x86_64-Release-GN",
		"Test-Ubuntu-Clang-GCE-CPU-AVX2-x86_64-Release-TSAN",
		"Test-Ubuntu-Clang-Golo-GPU-GT610-x86_64-Debug-ASAN",
		"Test-Ubuntu-Clang-Golo-GPU-GT610-x86_64-Release-TSAN",
		"Test-Ubuntu-GCC-GCE-CPU-AVX2-x86-Debug",
		"Test-Ubuntu-GCC-GCE-CPU-AVX2-x86_64-Debug-CT_DM_100k_SKPs",
		"Test-Ubuntu-GCC-GCE-CPU-AVX2-x86_64-Debug-CT_DM_1m_SKPs",
		"Test-Ubuntu-GCC-GCE-CPU-AVX2-x86_64-Debug-CT_IMG_DECODE_100k_SKPs",
		"Test-Ubuntu-GCC-GCE-CPU-AVX2-x86_64-Debug-GN",
		"Test-Ubuntu-GCC-GCE-CPU-AVX2-x86_64-Debug-SK_USE_DISCARDABLE_SCALEDIMAGECACHE",
		"Test-Ubuntu-GCC-GCE-CPU-AVX2-x86_64-Release-Fast",
		"Test-Ubuntu-GCC-GCE-CPU-AVX2-x86_64-Release-GN",
		"Test-Ubuntu-GCC-GCE-CPU-AVX2-x86_64-Release-SKNX_NO_SIMD",
		"Test-Ubuntu-GCC-ShuttleA-GPU-GTX550Ti-x86_64-Release-Valgrind",
		"Test-Ubuntu-GCC-ShuttleA-GPU-GTX660-x86_64-Debug-GN",
		"Test-Ubuntu-GCC-ShuttleA-GPU-GTX660-x86_64-Release-GN",
		"Test-Win-MSVC-GCE-CPU-AVX2-x86-Debug",
		"Test-Win-MSVC-GCE-CPU-AVX2-x86-Release",
		"Test-Win-MSVC-GCE-CPU-AVX2-x86_64-Debug",
		"Test-Win-MSVC-GCE-CPU-AVX2-x86_64-Debug-GDI",
		"Test-Win-MSVC-GCE-CPU-AVX2-x86_64-Release",
		"Test-Win-MSVC-GCE-CPU-AVX2-x86_64-Release-GDI",
		"Test-Win-MSVC-Golo-GPU-GT610-x86_64-Release",
		"Test-Win-MSVC-NUC-GPU-IntelIris6100-x86_64-Debug",
		"Test-Win-MSVC-NUC-GPU-IntelIris6100-x86_64-Release",
		"Test-Win-MSVC-ShuttleC-GPU-GTX960-x86_64-Debug",
		"Test-Win-MSVC-ShuttleC-GPU-GTX960-x86_64-Debug-ANGLE",
		"Test-Win-MSVC-ShuttleC-GPU-GTX960-x86_64-Release",
		"Test-Win-MSVC-ShuttleC-GPU-GTX960-x86_64-Release-ANGLE",
		"Test-Win-MSVC-ShuttleC-GPU-iHD530-x86_64-Debug",
		"Test-Win-MSVC-ShuttleC-GPU-iHD530-x86_64-Release",
		"Test-Win10-MSVC-ShuttleA-GPU-GTX660-x86_64-Debug",
		"Test-Win10-MSVC-ShuttleA-GPU-GTX660-x86_64-Debug-Vulkan",
		"Test-Win10-MSVC-ShuttleA-GPU-GTX660-x86_64-Release",
		"Test-Win8-MSVC-ShuttleA-GPU-HD7770-x86_64-Debug",
		"Test-Win8-MSVC-ShuttleA-GPU-HD7770-x86_64-Release",
		"Test-Win8-MSVC-ShuttleB-GPU-HD4600-x86_64-Debug",
		"Test-Win8-MSVC-ShuttleB-GPU-HD4600-x86_64-Release",
		"Test-iOS-Clang-iPadMini4-GPU-GX6450-Arm7-Debug",
		"Test-iOS-Clang-iPadMini4-GPU-GX6450-Arm7-Release",
	}

	// LINUX_GCE_DIMENSIONS are the Swarming dimensions for Linux GCE
	// instances.
	LINUX_GCE_DIMENSIONS = []string{
		"cpu:x86-64-avx2",
		"gpu:none",
		"os:Ubuntu",
		fmt.Sprintf("pool:%s", POOL_SKIA),
	}

	// Defines the structure of job names.
	jobNameSchema *JobNameSchema
)

// deriveCompileTaskName returns the name of a compile task based on the given
// job name.
func deriveCompileTaskName(jobName string, parts map[string]string) string {
	if parts["role"] == "Housekeeper" {
		return "Build-Ubuntu-GCC-x86_64-Release-Shared"
	} else if parts["role"] == "Test" || parts["role"] == "Perf" {
		task_os := parts["os"]
		ec := parts["extra_config"]
		ec = strings.TrimSuffix(ec, "_Skpbench")
		if task_os == "Android" {
			if ec == "Vulkan" {
				ec = "Android_Vulkan"
			} else if !strings.Contains(ec, "GN_Android") {
				ec = task_os
			}
			task_os = "Ubuntu"
		} else if task_os == "iOS" {
			ec = task_os
			task_os = "Mac"
		} else if strings.Contains(task_os, "Win") {
			task_os = "Win"
		}
		jobNameMap := map[string]string{
			"role":          "Build",
			"os":            task_os,
			"compiler":      parts["compiler"],
			"target_arch":   parts["arch"],
			"configuration": parts["configuration"],
		}
		if ec != "" {
			jobNameMap["extra_config"] = ec
		}
		name, err := jobNameSchema.MakeJobName(jobNameMap)
		if err != nil {
			glog.Fatal(err)
		}
		return name
	} else {
		return jobName
	}
}

// swarmDimensions generates swarming bot dimensions for the given task.
func swarmDimensions(parts map[string]string) []string {
	d := map[string]string{
		"pool": POOL_SKIA,
	}
	if os, ok := parts["os"]; ok {
		d["os"] = os
	} else {
		d["os"] = DEFAULT_OS
	}
	if strings.Contains(d["os"], "Win") {
		d["os"] = "Windows"
	}
	if parts["role"] == "Test" || parts["role"] == "Perf" {
		if strings.Contains(parts["os"], "Android") {
			// For Android, the device type is a better dimension
			// than CPU or GPU.
			deviceInfo := map[string][]string{
				"AndroidOne":    {"sprout", "MOB30Q"},
				"GalaxyS7":      {"heroqlteatt", "MMB29M"},
				"NVIDIA_Shield": {"foster", "MRA58K"},
				"Nexus10":       {"manta", "LMY49J"},
				"Nexus5":        {"hammerhead", "MOB31E"},
				"Nexus6":        {"shamu", "M"},
				"Nexus6p":       {"angler", "NMF26C"},
				"Nexus7":        {"grouper", "LMY47V"},
				"Nexus7v2":      {"flo", "M"},
				"Nexus9":        {"flounder", "NRD91D"},
				"NexusPlayer":   {"fugu", "NRD90R"},
				"Pixel":         {"sailfish", "NMF25"},
				"PixelC":        {"dragon", "NMF26C"},
				"PixelXL":       {"marlin", "NMF25"},
			}[parts["model"]]
			d["device_type"] = deviceInfo[0]
			d["device_os"] = deviceInfo[1]
		} else if strings.Contains(parts["os"], "iOS") {
			d["device"] = map[string]string{
				"iPadMini4": "iPad5,1",
			}[parts["model"]]
			// TODO(borenet): Replace this hack with something
			// better.
			d["os"] = "iOS-9.3.1"
		} else if parts["cpu_or_gpu"] == "CPU" {
			d["gpu"] = "none"
			d["cpu"] = map[string]string{
				"AVX":  "x86-64",
				"AVX2": "x86-64-avx2",
				"SSE4": "x86-64",
			}[parts["cpu_or_gpu_value"]]
			if strings.Contains(parts["os"], "Win") && parts["cpu_or_gpu_value"] == "AVX2" {
				// AVX2 is not correctly detected on Windows. Fall back on other
				// dimensions to ensure that we correctly target machines which we know
				// have AVX2 support.
				d["cpu"] = "x86-64"
				d["os"] = "Windows-2008ServerR2-SP1"
			}
		} else {
			d["gpu"] = map[string]string{
				"GeForce320M":   "10de:08a4",
				"GT610":         "10de:104a",
				"GTX550Ti":      "10de:1244",
				"GTX660":        "10de:11c0",
				"GTX960":        "10de:1401",
				"HD4000":        "8086:0a2e",
				"HD4600":        "8086:0412",
				"HD7770":        "1002:683d",
				"iHD530":        "8086:1912",
				"IntelIris6100": "8086:162b",
			}[parts["cpu_or_gpu_value"]]
		}
	} else {
		d["gpu"] = "none"
	}
	rv := make([]string, 0, len(d))
	for k, v := range d {
		rv = append(rv, fmt.Sprintf("%s:%s", k, v))
	}
	sort.Strings(rv)
	return rv
}

// compile generates a compile task. Returns the name of the last task in the
// generated chain of tasks, which the Job should add as a dependency.
func compile(b *specs.TasksCfgBuilder, name string, parts map[string]string) string {
	// Collect the necessary CIPD packages.
	pkgs := []*specs.CipdPackage{}

	// Android bots require a toolchain.
	if strings.Contains(name, "Android") {
		if strings.Contains(name, "Mac") {
			pkgs = append(pkgs, b.MustGetCipdPackageFromAsset("android_ndk_darwin"))
		} else if strings.Contains(name, "Win") {
			pkg := b.MustGetCipdPackageFromAsset("android_ndk_windows")
			pkg.Path = "n"
			pkgs = append(pkgs, pkg)
		} else {
			pkgs = append(pkgs, b.MustGetCipdPackageFromAsset("android_ndk_linux"))
		}
	} else if strings.Contains(name, "Ubuntu") && strings.Contains(name, "Clang") {
		pkgs = append(pkgs, b.MustGetCipdPackageFromAsset("clang_linux"))
	} else if strings.Contains(name, "Win") {
		pkgs = append(pkgs, b.MustGetCipdPackageFromAsset("win_toolchain"))
		if strings.Contains(name, "Vulkan") {
			pkgs = append(pkgs, b.MustGetCipdPackageFromAsset("win_vulkan_sdk"))
		}
	}

	// Add the task.
	b.MustAddTask(name, &specs.TaskSpec{
		CipdPackages: pkgs,
		Dimensions:   swarmDimensions(parts),
		ExtraArgs: []string{
			"--workdir", "../../..", "swarm_compile",
			fmt.Sprintf("repository=%s", specs.PLACEHOLDER_REPO),
			fmt.Sprintf("buildername=%s", name),
			"mastername=fake-master",
			"buildnumber=2",
			"slavename=fake-buildslave",
			"nobuildbot=True",
			fmt.Sprintf("swarm_out_dir=%s", specs.PLACEHOLDER_ISOLATED_OUTDIR),
			fmt.Sprintf("revision=%s", specs.PLACEHOLDER_REVISION),
			fmt.Sprintf("patch_storage=%s", specs.PLACEHOLDER_PATCH_STORAGE),
			fmt.Sprintf("patch_issue=%s", specs.PLACEHOLDER_ISSUE),
			fmt.Sprintf("patch_set=%s", specs.PLACEHOLDER_PATCHSET),
		},
		Isolate:  "compile_skia.isolate",
		Priority: 0.8,
	})
	// All compile tasks are runnable as their own Job.
	b.AddJob(name, &specs.JobSpec{
		Priority:  0.8,
		TaskSpecs: []string{name},
	})
	return name
}

// recreateSKPs generates a RecreateSKPs task. Returns the name of the last
// task in the generated chain of tasks, which the Job should add as a
// dependency.
func recreateSKPs(b *specs.TasksCfgBuilder, name string) string {
	b.MustAddTask(name, &specs.TaskSpec{
		CipdPackages:     []*specs.CipdPackage{},
		Dimensions:       LINUX_GCE_DIMENSIONS,
		ExecutionTimeout: 4 * time.Hour,
		ExtraArgs: []string{
			"--workdir", "../../..", "swarm_RecreateSKPs",
			fmt.Sprintf("repository=%s", specs.PLACEHOLDER_REPO),
			fmt.Sprintf("buildername=%s", name),
			"mastername=fake-master",
			"buildnumber=2",
			"slavename=fake-buildslave",
			"nobuildbot=True",
			fmt.Sprintf("swarm_out_dir=%s", specs.PLACEHOLDER_ISOLATED_OUTDIR),
			fmt.Sprintf("revision=%s", specs.PLACEHOLDER_REVISION),
			fmt.Sprintf("patch_storage=%s", specs.PLACEHOLDER_PATCH_STORAGE),
			fmt.Sprintf("patch_issue=%s", specs.PLACEHOLDER_ISSUE),
			fmt.Sprintf("patch_set=%s", specs.PLACEHOLDER_PATCHSET),
		},
		IoTimeout: 40 * time.Minute,
		Isolate:   "compile_skia.isolate",
		Priority:  0.8,
	})
	return name
}

// ctSKPs generates a CT SKPs task. Returns the name of the last task in the
// generated chain of tasks, which the Job should add as a dependency.
func ctSKPs(b *specs.TasksCfgBuilder, name string) string {
	b.MustAddTask(name, &specs.TaskSpec{
		CipdPackages:     []*specs.CipdPackage{},
		Dimensions:       []string{"pool:SkiaCT"},
		ExecutionTimeout: 24 * time.Hour,
		ExtraArgs: []string{
			"--workdir", "../../..", "swarm_ct_skps",
			fmt.Sprintf("repository=%s", specs.PLACEHOLDER_REPO),
			fmt.Sprintf("buildername=%s", name),
			"mastername=fake-master",
			"buildnumber=2",
			"slavename=fake-buildslave",
			"nobuildbot=True",
			fmt.Sprintf("swarm_out_dir=%s", specs.PLACEHOLDER_ISOLATED_OUTDIR),
			fmt.Sprintf("revision=%s", specs.PLACEHOLDER_REVISION),
			fmt.Sprintf("patch_storage=%s", specs.PLACEHOLDER_PATCH_STORAGE),
			fmt.Sprintf("patch_issue=%s", specs.PLACEHOLDER_ISSUE),
			fmt.Sprintf("patch_set=%s", specs.PLACEHOLDER_PATCHSET),
		},
		IoTimeout: time.Hour,
		Isolate:   "ct_skps_skia.isolate",
		Priority:  0.8,
	})
	return name
}

// housekeeper generates a Housekeeper task. Returns the name of the last task
// in the generated chain of tasks, which the Job should add as a dependency.
func housekeeper(b *specs.TasksCfgBuilder, name, compileTaskName string) string {
	b.MustAddTask(name, &specs.TaskSpec{
		CipdPackages: []*specs.CipdPackage{},
		Dependencies: []string{compileTaskName},
		Dimensions:   LINUX_GCE_DIMENSIONS,
		ExtraArgs: []string{
			"--workdir", "../../..", "swarm_housekeeper",
			fmt.Sprintf("repository=%s", specs.PLACEHOLDER_REPO),
			fmt.Sprintf("buildername=%s", name),
			"mastername=fake-master",
			"buildnumber=2",
			"slavename=fake-buildslave",
			"nobuildbot=True",
			fmt.Sprintf("swarm_out_dir=%s", specs.PLACEHOLDER_ISOLATED_OUTDIR),
			fmt.Sprintf("revision=%s", specs.PLACEHOLDER_REVISION),
			fmt.Sprintf("patch_storage=%s", specs.PLACEHOLDER_PATCH_STORAGE),
			fmt.Sprintf("patch_issue=%s", specs.PLACEHOLDER_ISSUE),
			fmt.Sprintf("patch_set=%s", specs.PLACEHOLDER_PATCHSET),
		},
		Isolate:  "housekeeper_skia.isolate",
		Priority: 0.8,
	})
	return name
}

// infra generates an infra_tests task. Returns the name of the last task in the
// generated chain of tasks, which the Job should add as a dependency.
func infra(b *specs.TasksCfgBuilder, name string) string {
	b.MustAddTask(name, &specs.TaskSpec{
		CipdPackages: []*specs.CipdPackage{},
		Dimensions:   LINUX_GCE_DIMENSIONS,
		ExtraArgs: []string{
			"--workdir", "../../..", "swarm_infra",
			fmt.Sprintf("repository=%s", specs.PLACEHOLDER_REPO),
			fmt.Sprintf("buildername=%s", name),
			"mastername=fake-master",
			"buildnumber=2",
			"slavename=fake-buildslave",
			"nobuildbot=True",
			fmt.Sprintf("swarm_out_dir=%s", specs.PLACEHOLDER_ISOLATED_OUTDIR),
			fmt.Sprintf("revision=%s", specs.PLACEHOLDER_REVISION),
			fmt.Sprintf("patch_storage=%s", specs.PLACEHOLDER_PATCH_STORAGE),
			fmt.Sprintf("patch_issue=%s", specs.PLACEHOLDER_ISSUE),
			fmt.Sprintf("patch_set=%s", specs.PLACEHOLDER_PATCHSET),
		},
		Isolate:  "infra_skia.isolate",
		Priority: 0.8,
	})
	return name
}

// doUpload indicates whether the given Job should upload its results.
func doUpload(name string) bool {
	skipUploadBots := []string{
		"ASAN",
		"Coverage",
		"MSAN",
		"TSAN",
		"UBSAN",
		"Valgrind",
	}
	for _, s := range skipUploadBots {
		if strings.Contains(name, s) {
			return false
		}
	}
	return true
}

// test generates a Test task. Returns the name of the last task in the
// generated chain of tasks, which the Job should add as a dependency.
func test(b *specs.TasksCfgBuilder, name string, parts map[string]string, compileTaskName string, pkgs []*specs.CipdPackage) string {
	s := &specs.TaskSpec{
		CipdPackages:     pkgs,
		Dependencies:     []string{compileTaskName},
		Dimensions:       swarmDimensions(parts),
		ExecutionTimeout: 4 * time.Hour,
		Expiration:       20 * time.Hour,
		ExtraArgs: []string{
			"--workdir", "../../..", "swarm_test",
			fmt.Sprintf("repository=%s", specs.PLACEHOLDER_REPO),
			fmt.Sprintf("buildername=%s", name),
			"mastername=fake-master",
			"buildnumber=2",
			"slavename=fake-buildslave",
			"nobuildbot=True",
			fmt.Sprintf("swarm_out_dir=%s", specs.PLACEHOLDER_ISOLATED_OUTDIR),
			fmt.Sprintf("revision=%s", specs.PLACEHOLDER_REVISION),
			fmt.Sprintf("patch_storage=%s", specs.PLACEHOLDER_PATCH_STORAGE),
			fmt.Sprintf("patch_issue=%s", specs.PLACEHOLDER_ISSUE),
			fmt.Sprintf("patch_set=%s", specs.PLACEHOLDER_PATCHSET),
		},
		IoTimeout: 40 * time.Minute,
		Isolate:   "test_skia.isolate",
		Priority:  0.8,
	}
	if strings.Contains(parts["extra_config"], "Valgrind") {
		s.ExecutionTimeout = 9 * time.Hour
		s.Expiration = 48 * time.Hour
		s.IoTimeout = time.Hour
	} else if strings.Contains(parts["extra_config"], "MSAN") {
		s.ExecutionTimeout = 9 * time.Hour
	}
	b.MustAddTask(name, s)

	// Upload results if necessary.
	if doUpload(name) {
		uploadName := fmt.Sprintf("%s%s%s", PREFIX_UPLOAD, jobNameSchema.Sep, name)
		b.MustAddTask(uploadName, &specs.TaskSpec{
			Dependencies: []string{name},
			Dimensions:   LINUX_GCE_DIMENSIONS,
			ExtraArgs: []string{
				"--workdir", "../../..", "upload_dm_results",
				fmt.Sprintf("repository=%s", specs.PLACEHOLDER_REPO),
				fmt.Sprintf("buildername=%s", name),
				"mastername=fake-master",
				"buildnumber=2",
				"slavename=fake-buildslave",
				"nobuildbot=True",
				fmt.Sprintf("swarm_out_dir=%s", specs.PLACEHOLDER_ISOLATED_OUTDIR),
				fmt.Sprintf("revision=%s", specs.PLACEHOLDER_REVISION),
				fmt.Sprintf("patch_storage=%s", specs.PLACEHOLDER_PATCH_STORAGE),
				fmt.Sprintf("patch_issue=%s", specs.PLACEHOLDER_ISSUE),
				fmt.Sprintf("patch_set=%s", specs.PLACEHOLDER_PATCHSET),
			},
			Isolate:  "upload_dm_results.isolate",
			Priority: 0.8,
		})
		return uploadName
	}
	return name
}

// perf generates a Perf task. Returns the name of the last task in the
// generated chain of tasks, which the Job should add as a dependency.
func perf(b *specs.TasksCfgBuilder, name string, parts map[string]string, compileTaskName string, pkgs []*specs.CipdPackage) string {
	recipe := "swarm_perf"
	isolate := "perf_skia.isolate"
	if strings.Contains(parts["extra_config"], "Skpbench") {
		recipe = "swarm_skpbench"
		isolate = "skpbench_skia.isolate"
	}
	s := &specs.TaskSpec{
		CipdPackages:     pkgs,
		Dependencies:     []string{compileTaskName},
		Dimensions:       swarmDimensions(parts),
		ExecutionTimeout: 4 * time.Hour,
		Expiration:       20 * time.Hour,
		ExtraArgs: []string{
			"--workdir", "../../..", recipe,
			fmt.Sprintf("repository=%s", specs.PLACEHOLDER_REPO),
			fmt.Sprintf("buildername=%s", name),
			"mastername=fake-master",
			"buildnumber=2",
			"slavename=fake-buildslave",
			"nobuildbot=True",
			fmt.Sprintf("swarm_out_dir=%s", specs.PLACEHOLDER_ISOLATED_OUTDIR),
			fmt.Sprintf("revision=%s", specs.PLACEHOLDER_REVISION),
			fmt.Sprintf("patch_storage=%s", specs.PLACEHOLDER_PATCH_STORAGE),
			fmt.Sprintf("patch_issue=%s", specs.PLACEHOLDER_ISSUE),
			fmt.Sprintf("patch_set=%s", specs.PLACEHOLDER_PATCHSET),
		},
		IoTimeout: 40 * time.Minute,
		Isolate:   isolate,
		Priority:  0.8,
	}
	if strings.Contains(parts["extra_config"], "Valgrind") {
		s.ExecutionTimeout = 9 * time.Hour
		s.Expiration = 48 * time.Hour
		s.IoTimeout = time.Hour
	} else if strings.Contains(parts["extra_config"], "MSAN") {
		s.ExecutionTimeout = 9 * time.Hour
	}
	b.MustAddTask(name, s)

	// Upload results if necessary.
	if strings.Contains(name, "Release") && doUpload(name) {
		uploadName := fmt.Sprintf("%s%s%s", PREFIX_UPLOAD, jobNameSchema.Sep, name)
		b.MustAddTask(uploadName, &specs.TaskSpec{
			Dependencies: []string{name},
			Dimensions:   LINUX_GCE_DIMENSIONS,
			ExtraArgs: []string{
				"--workdir", "../../..", "upload_nano_results",
				fmt.Sprintf("repository=%s", specs.PLACEHOLDER_REPO),
				fmt.Sprintf("buildername=%s", name),
				"mastername=fake-master",
				"buildnumber=2",
				"slavename=fake-buildslave",
				"nobuildbot=True",
				fmt.Sprintf("swarm_out_dir=%s", specs.PLACEHOLDER_ISOLATED_OUTDIR),
				fmt.Sprintf("revision=%s", specs.PLACEHOLDER_REVISION),
				fmt.Sprintf("patch_storage=%s", specs.PLACEHOLDER_PATCH_STORAGE),
				fmt.Sprintf("patch_issue=%s", specs.PLACEHOLDER_ISSUE),
				fmt.Sprintf("patch_set=%s", specs.PLACEHOLDER_PATCHSET),
			},
			Isolate:  "upload_nano_results.isolate",
			Priority: 0.8,
		})
		return uploadName
	}
	return name
}

// process generates tasks and jobs for the given job name.
func process(b *specs.TasksCfgBuilder, name string) {
	deps := []string{}

	parts, err := jobNameSchema.ParseJobName(name)
	if err != nil {
		glog.Fatal(err)
	}

	// RecreateSKPs.
	if strings.Contains(name, "RecreateSKPs") {
		deps = append(deps, recreateSKPs(b, name))
	}

	// CT bots.
	if strings.Contains(name, "-CT_") {
		deps = append(deps, ctSKPs(b, name))
	}

	// Infra tests.
	if name == "Housekeeper-PerCommit-InfraTests" {
		deps = append(deps, infra(b, name))
	}

	// Compile bots.
	if parts["role"] == "Build" {
		deps = append(deps, compile(b, name, parts))
	}

	// Most remaining bots need a compile task.
	compileTaskName := deriveCompileTaskName(name, parts)
	compileTaskParts, err := jobNameSchema.ParseJobName(compileTaskName)
	if err != nil {
		glog.Fatal(err)
	}
	// These bots do not need a compile task.
	if parts["role"] != "Build" &&
		name != "Housekeeper-PerCommit-InfraTests" &&
		!strings.Contains(name, "RecreateSKPs") &&
		!strings.Contains(name, "-CT_") {
		compile(b, compileTaskName, compileTaskParts)
	}

	// Housekeeper.
	if parts["role"] == "Housekeeper-PerCommit" {
		deps = append(deps, housekeeper(b, name, compileTaskName))
	}

	// Common assets needed by the remaining bots.
	pkgs := []*specs.CipdPackage{
		b.MustGetCipdPackageFromAsset("skimage"),
		b.MustGetCipdPackageFromAsset("skp"),
		b.MustGetCipdPackageFromAsset("svg"),
	}
	if strings.Contains(name, "Ubuntu") && strings.Contains(name, "SAN") {
		pkgs = append(pkgs, b.MustGetCipdPackageFromAsset("clang_linux"))
	}
	// Skpbench only needs skps
	if strings.Contains(name, "Skpbench") {
		pkgs = []*specs.CipdPackage{
			b.MustGetCipdPackageFromAsset("skp"),
		}
	}

	// Test bots.
	if parts["role"] == "Test" && !strings.Contains(name, "-CT_") {
		deps = append(deps, test(b, name, parts, compileTaskName, pkgs))
	}

	// Perf bots.
	if parts["role"] == "Perf" && !strings.Contains(name, "-CT_") {
		deps = append(deps, perf(b, name, parts, compileTaskName, pkgs))
	}

	// Add the Job spec.
	j := &specs.JobSpec{
		Priority:  0.8,
		TaskSpecs: deps,
	}
	if name == "Housekeeper-Nightly-RecreateSKPs_Canary" {
		j.Trigger = "nightly"
	}
	if name == "Housekeeper-Weekly-RecreateSKPs" {
		j.Trigger = "weekly"
	}
	if name == "Test-Ubuntu-GCC-GCE-CPU-AVX2-x86_64-Debug-CT_DM_1m_SKPs" {
		j.Trigger = "weekly"
	}
	b.AddJob(name, j)
}

// Regenerate the tasks.json file.
func main() {
	b := specs.MustNewTasksCfgBuilder()
	// Create the JobNameSchema.
	schema, err := NewJobNameSchema(path.Join(b.CheckoutRoot(), "infra", "bots", "recipe_modules", "builder_name_schema", "builder_name_schema.json"))
	if err != nil {
		glog.Fatal(err)
	}
	jobNameSchema = schema

	// Create Tasks and Jobs.
	for _, name := range JOBS {
		process(b, name)
	}

	b.MustFinish()
}

// TODO(borenet): The below really belongs in its own file, probably next to the
// builder_name_schema.json file.

// JobNameSchema is a struct used for (de)constructing Job names in a
// predictable format.
type JobNameSchema struct {
	Schema map[string][]string `json:"builder_name_schema"`
	Sep    string              `json:"builder_name_sep"`
}

// NewJobNameSchema returns a JobNameSchema instance based on the given JSON
// file.
func NewJobNameSchema(jsonFile string) (*JobNameSchema, error) {
	var rv JobNameSchema
	f, err := os.Open(jsonFile)
	if err != nil {
		return nil, err
	}
	defer util.Close(f)
	if err := json.NewDecoder(f).Decode(&rv); err != nil {
		return nil, err
	}
	return &rv, nil
}

// ParseJobName splits the given Job name into its component parts, according
// to the schema.
func (s *JobNameSchema) ParseJobName(n string) (map[string]string, error) {
	split := strings.Split(n, s.Sep)
	if len(split) < 2 {
		return nil, fmt.Errorf("Invalid job name: %q", n)
	}
	role := split[0]
	split = split[1:]
	keys, ok := s.Schema[role]
	if !ok {
		return nil, fmt.Errorf("Invalid job name; %q is not a valid role.", role)
	}
	extraConfig := ""
	if len(split) == len(keys)+1 {
		extraConfig = split[len(split)-1]
		split = split[:len(split)-1]
	}
	if len(split) != len(keys) {
		return nil, fmt.Errorf("Invalid job name; %q has incorrect number of parts.", n)
	}
	rv := make(map[string]string, len(keys)+2)
	rv["role"] = role
	if extraConfig != "" {
		rv["extra_config"] = extraConfig
	}
	for i, k := range keys {
		rv[k] = split[i]
	}
	return rv, nil
}

// MakeJobName assembles the given parts of a Job name, according to the schema.
func (s *JobNameSchema) MakeJobName(parts map[string]string) (string, error) {
	role, ok := parts["role"]
	if !ok {
		return "", fmt.Errorf("Invalid job parts; jobs must have a role.")
	}
	keys, ok := s.Schema[role]
	if !ok {
		return "", fmt.Errorf("Invalid job parts; unknown role %q", role)
	}
	rvParts := make([]string, 0, len(parts))
	rvParts = append(rvParts, role)
	for _, k := range keys {
		v, ok := parts[k]
		if !ok {
			return "", fmt.Errorf("Invalid job parts; missing %q", k)
		}
		rvParts = append(rvParts, v)
	}
	if _, ok := parts["extra_config"]; ok {
		rvParts = append(rvParts, parts["extra_config"])
	}
	return strings.Join(rvParts, s.Sep), nil
}
