// Copyright 2023 Google LLC
//
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package device_specific_configs

import (
	"fmt"
	"sort"
)

// Config represents a Bazel config that communicates information about the device under test.
//
// This struct is used to generate file //bazel/devicesrc.
//
// Configurations of this kind should not be used to set build time settings, such as the target
// Bazel platform (e.g. Linux, Android), optimization level (e.g. Debug, Release) or local vs. RBE.
// For that kind of information, please pass a second --config flag using one of the configurations
// defined in //bazel/buildrc.
type Config struct {
	// Name of the config (the <foo> that gets passed to Bazel via --config=<foo>).
	Name string

	// Any device-specific key/value pairs to include in Gold and Perf traces produced by GM and
	// benchmark tests, except for keys "cpu_or_gpu" and "cpu_or_gpu_value". See fields CPU and GPU.
	Keys map[string]string

	// CPU is the name of the CPU on this device.
	//
	// When a GM or benchmark test case is executed, the corresponding test runner will set the
	// "cpu_or_gpu_value" key of the resulting Gold or Perf trace with the contents of this field
	// if the test case is CPU-bound, in which case it will also set the "cpu_or_gpu" key to "CPU".
	CPU string

	// GPU is the name of the GPU on this device.
	//
	// This field is the GPU analogous of the CPU field; see its documentation for details.
	GPU string

	// SwarmingDimensions are the Swarming dimensions that a CI task must exhibit in order to get
	// scheduled to run on a machine that corresponds with the device under test indicated by the
	// Bazel config.
	SwarmingDimensions map[string]string
}

// Model returns the "model" key in the Keys dictionary.
func (d Config) Model() string {
	model, ok := d.Keys["model"]
	if !ok {
		// Should never happen. We have a unit test that ensures all configs have this key.
		panic(fmt.Sprintf("config %q does not contain key \"model\"", d.Name))
	}
	return model
}

// TestRunnerArgs returns the command-line arguments that should be passed to the Bazel test
// target.
func (d Config) TestRunnerArgs() []string {
	args := []string{
		// Pass the name of the Bazel configuration as an argument. Android tests use this to infer the
		// model of the device under test. Specifically, adb_test_runner.go will take device-specific
		// setup and teardown steps based on the model. C++ tests running on the same machine as Bazel
		// will ignore this flag.
		"--device-specific-bazel-config",
		d.Name,
	}

	// Sort keys for determinism.
	var keys []string
	for key := range d.Keys {
		keys = append(keys, key)
	}
	sort.Strings(keys)

	// Add key/value pairs.
	args = append(args, "--key")
	for _, key := range keys {
		args = append(args, key, d.Keys[key])
	}

	if d.CPU != "" {
		args = append(args, "--cpuName", d.CPU)
	}

	if d.GPU != "" {
		args = append(args, "--gpuName", d.GPU)
	}

	return args
}

// Configs contains all known device-specific Bazel configs.
//
// The contents of this map are used to generate file //bazel/devicesrc.
//
// TODO(lovisolo): Populate field SwarmingDimensions for all configs.
var Configs = map[string]Config{
	"AlphaR2": {
		Name: "AlphaR2",
		Keys: map[string]string{
			"arch":  "x86_64",
			"model": "AlphaR2",
			"os":    "Win10",
		},
		GPU: "RadeonR9M470X",
	},
	"AndroidOne": {
		Name: "AndroidOne",
		Keys: map[string]string{
			"arch":  "arm",
			"model": "AndroidOne",
			"os":    "Android",
		},
		GPU: "Mali400MP2",
	},
	"GCE_Debian10_AVX2": {
		Name: "GCE_Debian10_AVX2",
		Keys: map[string]string{
			"arch":  "x86_64",
			"model": "GCE",
			"os":    "Debian10",
		},
		CPU: "AVX2",
		GPU: "SwiftShader",
	},
	"GCE_Debian10_AVX512": {
		Name: "GCE_Debian10_AVX512",
		Keys: map[string]string{
			"arch":  "x86_64",
			"model": "GCE",
			"os":    "Debian10",
		},
		CPU: "AVX512",
		GPU: "SwiftShader",
	},
	"GCE_Debian10_Rome": {
		Name: "GCE_Debian10_Rome",
		Keys: map[string]string{
			"arch":  "x86_64",
			"model": "GCE",
			"os":    "Debian10",
		},
		CPU: "Rome",
		GPU: "SwiftShader",
	},
	"GCE_Win2019_AVX2": {
		Name: "GCE_Win2019_AVX2",
		Keys: map[string]string{
			"arch":  "x86_64",
			"model": "GCE",
			"os":    "Win2019",
		},
		CPU: "AVX2",
		GPU: "SwiftShader",
	},
	"GCE_Win2019_AVX512": {
		Name: "GCE_Win2019_AVX512",
		Keys: map[string]string{
			"arch":  "x86_64",
			"model": "GCE",
			"os":    "Win2019",
		},
		CPU: "AVX512",
		GPU: "SwiftShader",
	},
	"GCE_Win2019_Rome": {
		Name: "GCE_Win2019_Rome",
		Keys: map[string]string{
			"arch":  "x86_64",
			"model": "GCE",
			"os":    "Win2019",
		},
		CPU: "Rome",
		GPU: "SwiftShader",
	},
	"GCE_x86_Debian10_AVX2": {
		Name: "GCE_x86_Debian10_AVX2",
		Keys: map[string]string{
			"arch":  "x86",
			"model": "GCE",
			"os":    "Debian10",
		},
		CPU: "AVX2",
		GPU: "SwiftShader",
	},
	"GCE_x86_Debian10_AVX512": {
		Name: "GCE_x86_Debian10_AVX512",
		Keys: map[string]string{
			"arch":  "x86",
			"model": "GCE",
			"os":    "Debian10",
		},
		CPU: "AVX512",
		GPU: "SwiftShader",
	},
	"GCE_x86_Debian10_Rome": {
		Name: "GCE_x86_Debian10_Rome",
		Keys: map[string]string{
			"arch":  "x86",
			"model": "GCE",
			"os":    "Debian10",
		},
		CPU: "Rome",
		GPU: "SwiftShader",
	},
	"GCE_x86_Win2019_AVX2": {
		Name: "GCE_x86_Win2019_AVX2",
		Keys: map[string]string{
			"arch":  "x86",
			"model": "GCE",
			"os":    "Win2019",
		},
		CPU: "AVX2",
		GPU: "SwiftShader",
	},
	"GCE_x86_Win2019_AVX512": {
		Name: "GCE_x86_Win2019_AVX512",
		Keys: map[string]string{
			"arch":  "x86",
			"model": "GCE",
			"os":    "Win2019",
		},
		CPU: "AVX512",
		GPU: "SwiftShader",
	},
	"GCE_x86_Win2019_Rome": {
		Name: "GCE_x86_Win2019_Rome",
		Keys: map[string]string{
			"arch":  "x86",
			"model": "GCE",
			"os":    "Win2019",
		},
		CPU: "Rome",
		GPU: "SwiftShader",
	},
	"GalaxyS20": {
		Name: "GalaxyS20",
		Keys: map[string]string{
			"arch":  "arm64",
			"model": "GalaxyS20",
			"os":    "Android",
		},
		GPU: "MaliG77",
	},
	"GalaxyS7_G930FD": {
		Name: "GalaxyS7_G930FD",
		Keys: map[string]string{
			"arch":  "arm64",
			"model": "GalaxyS7_G930FD",
			"os":    "Android",
		},
		GPU: "MaliT880",
	},
	"GalaxyS9": {
		Name: "GalaxyS9",
		Keys: map[string]string{
			"arch":  "arm64",
			"model": "GalaxyS9",
			"os":    "Android",
		},
		GPU: "MaliG72",
	},
	"Golo_wasm_Ubuntu18": {
		Name: "Golo_wasm_Ubuntu18",
		Keys: map[string]string{
			"arch":  "wasm",
			"model": "Golo",
			"os":    "Ubuntu18",
		},
		GPU: "QuadroP400",
	},
	"Golo_wasm_Win10": {
		Name: "Golo_wasm_Win10",
		Keys: map[string]string{
			"arch":  "wasm",
			"model": "Golo",
			"os":    "Win10",
		},
		GPU: "QuadroP400",
	},
	"Golo_x86_64_Ubuntu18": {
		Name: "Golo_x86_64_Ubuntu18",
		Keys: map[string]string{
			"arch":  "x86_64",
			"model": "Golo",
			"os":    "Ubuntu18",
		},
		GPU: "QuadroP400",
	},
	"Golo_x86_64_Win10": {
		Name: "Golo_x86_64_Win10",
		Keys: map[string]string{
			"arch":  "x86_64",
			"model": "Golo",
			"os":    "Win10",
		},
		GPU: "QuadroP400",
	},
	"JioNext": {
		Name: "JioNext",
		Keys: map[string]string{
			"arch":  "arm",
			"model": "JioNext",
			"os":    "Android",
		},
		CPU: "SnapdragonQM215",
		GPU: "Adreno308",
	},
	"Kevin": {
		Name: "Kevin",
		Keys: map[string]string{
			"arch":  "arm",
			"model": "Kevin",
			"os":    "ChromeOS",
		},
		GPU: "MaliT860",
	},
	"MacBook10.1": {
		Name: "MacBook10.1",
		Keys: map[string]string{
			"arch":  "x86_64",
			"model": "MacBook10.1",
			"os":    "Mac10.13",
		},
		GPU: "IntelHD615",
	},
	"MacBookAir7.2": {
		Name: "MacBookAir7.2",
		Keys: map[string]string{
			"arch":  "x86_64",
			"model": "MacBookAir7.2",
			"os":    "Mac10.15.1",
		},
		GPU: "IntelHD6000",
	},
	"MacBookPro11.5_Mac10.13": {
		Name: "MacBookPro11.5_Mac10.13",
		Keys: map[string]string{
			"arch":  "x86_64",
			"model": "MacBookPro11.5",
			"os":    "Mac10.13",
		},
		CPU: "AVX2",
		GPU: "RadeonHD8870M",
	},
	"MacBookPro11.5_Mac10.15.7": {
		Name: "MacBookPro11.5_Mac10.15.7",
		Keys: map[string]string{
			"arch":  "x86_64",
			"model": "MacBookPro11.5",
			"os":    "Mac10.15.7",
		},
		CPU: "AVX2",
		GPU: "RadeonHD8870M",
	},
	"MacBookPro16.2": {
		Name: "MacBookPro16.2",
		Keys: map[string]string{
			"arch":  "x86_64",
			"model": "MacBookPro16.2",
			"os":    "Mac12",
		},
		CPU: "AppleIntel",
		GPU: "IntelIrisPlus",
	},
	"MacMini7.1_Mac10.13": {
		Name: "MacMini7.1_Mac10.13",
		Keys: map[string]string{
			"arch":  "x86_64",
			"model": "MacMini7.1",
			"os":    "Mac10.13",
		},
		CPU: "AVX2",
		GPU: "IntelIris5100",
	},
	"MacMini7.1_Mac10.14": {
		Name: "MacMini7.1_Mac10.14",
		Keys: map[string]string{
			"arch":  "x86_64",
			"model": "MacMini7.1",
			"os":    "Mac10.14",
		},
		CPU: "AVX2",
		GPU: "IntelIris5100",
	},
	"MacMini7.1_Mac10.15.7": {
		Name: "MacMini7.1_Mac10.15.7",
		Keys: map[string]string{
			"arch":  "x86_64",
			"model": "MacMini7.1",
			"os":    "Mac10.15.7",
		},
		CPU: "AVX2",
		GPU: "IntelIris5100",
	},
	"MacMini9.1_Mac11": {
		Name: "MacMini9.1_Mac11",
		Keys: map[string]string{
			"arch":  "arm64",
			"model": "MacMini9.1",
			"os":    "Mac11",
		},
		CPU: "AppleM1",
		GPU: "AppleM1",
	},
	"MacMini9.1_Mac12": {
		Name: "MacMini9.1_Mac12",
		Keys: map[string]string{
			"arch":  "arm64",
			"model": "MacMini9.1",
			"os":    "Mac12",
		},
		CPU: "AppleM1",
		GPU: "AppleM1",
	},
	"MacMini9.1_Mac13": {
		Name: "MacMini9.1_Mac13",
		Keys: map[string]string{
			"arch":  "arm64",
			"model": "MacMini9.1",
			"os":    "Mac13",
		},
		CPU: "AppleM1",
		GPU: "AppleM1",
	},
	"NUC11TZi5_Debian11": {
		Name: "NUC11TZi5_Debian11",
		Keys: map[string]string{
			"arch":  "x86_64",
			"model": "NUC11TZi5",
			"os":    "Debian11",
		},
		CPU: "AVX2",
		GPU: "IntelIrisXe",
	},
	"NUC11TZi5_Win10": {
		Name: "NUC11TZi5_Win10",
		Keys: map[string]string{
			"arch":  "x86_64",
			"model": "NUC11TZi5",
			"os":    "Win10",
		},
		CPU: "AVX2",
		GPU: "IntelIrisXe",
	},
	"NUC5PPYH": {
		Name: "NUC5PPYH",
		Keys: map[string]string{
			"arch":  "x86_64",
			"model": "NUC5PPYH",
			"os":    "Debian10",
		},
		GPU: "IntelHD405",
	},
	"NUC5i7RYH": {
		Name: "NUC5i7RYH",
		Keys: map[string]string{
			"arch":  "x86_64",
			"model": "NUC5i7RYH",
			"os":    "Win10",
		},
		CPU: "AVX2",
		GPU: "IntelIris6100",
	},
	"NUC6i5SYK": {
		Name: "NUC6i5SYK",
		Keys: map[string]string{
			"arch":  "x86_64",
			"model": "NUC6i5SYK",
			"os":    "Win10",
		},
		GPU: "IntelIris540",
	},
	"NUC8i5BEK": {
		Name: "NUC8i5BEK",
		Keys: map[string]string{
			"arch":  "x86_64",
			"model": "NUC8i5BEK",
			"os":    "Win10",
		},
		GPU: "IntelIris655",
	},
	"NUC9i7QN_Debian11": {
		Name: "NUC9i7QN_Debian11",
		Keys: map[string]string{
			"arch":  "x86_64",
			"model": "NUC9i7QN",
			"os":    "Debian11",
		},
		CPU: "AVX2",
		GPU: "RTX3060",
		// Based on
		// https://skia.googlesource.com/skia/+/5606ef899116266132253e979a793fea97f12604/infra/bots/gen_tasks_logic/gen_tasks_logic.go#952.
		SwarmingDimensions: map[string]string{
			"os":  "Debian-11.5",
			"cpu": "x86-64-i7-9750H",
		},
	},
	"NUC9i7QN_Win10": {
		Name: "NUC9i7QN_Win10",
		Keys: map[string]string{
			"arch":  "x86_64",
			"model": "NUC9i7QN",
			"os":    "Win10",
		},
		CPU: "AVX2",
		GPU: "RTX3060",
	},
	"NUCD34010WYKH": {
		Name: "NUCD34010WYKH",
		Keys: map[string]string{
			"arch":  "x86_64",
			"model": "NUCD34010WYKH",
			"os":    "Win10",
		},
		GPU: "IntelHD4400",
	},
	"NUCDE3815TYKHE": {
		Name: "NUCDE3815TYKHE",
		Keys: map[string]string{
			"arch":  "x86_64",
			"model": "NUCDE3815TYKHE",
			"os":    "Debian10",
		},
		GPU: "IntelBayTrail",
	},
	"Nexus7": {
		Name: "Nexus7",
		Keys: map[string]string{
			"arch":  "arm",
			"model": "Nexus7",
			"os":    "Android",
		},
		GPU: "Tegra3",
	},
	"P30": {
		Name: "P30",
		Keys: map[string]string{
			"arch":  "arm64",
			"model": "P30",
			"os":    "Android",
		},
		GPU: "MaliG76",
	},
	"Pixel2XL": {
		Name: "Pixel2XL",
		Keys: map[string]string{
			"arch":  "arm64",
			"model": "Pixel2XL",
			"os":    "Android",
		},
		GPU: "Adreno540",
	},
	"Pixel3": {
		Name: "Pixel3",
		Keys: map[string]string{
			"arch":  "arm64",
			"model": "Pixel3",
			"os":    "Android",
		},
		GPU: "Adreno630",
	},
	"Pixel4": {
		Name: "Pixel4",
		Keys: map[string]string{
			"arch":  "arm64",
			"model": "Pixel4",
			"os":    "Android",
		},
		CPU: "Snapdragon855",
		GPU: "Adreno640",
	},
	"Pixel4XL": {
		Name: "Pixel4XL",
		Keys: map[string]string{
			"arch":  "arm64",
			"model": "Pixel4XL",
			"os":    "Android",
		},
		GPU: "Adreno640",
	},
	"Pixel5": {
		Name: "Pixel5",
		Keys: map[string]string{
			"arch":  "arm64",
			"model": "Pixel5",
			"os":    "Android",
		},
		GPU: "Adreno620",
		// Based on
		// https://skia.googlesource.com/skia/+/f8daeeb7f092abe1674bc2303c0781f9fb1756ab/infra/bots/gen_tasks_logic/gen_tasks_logic.go#836.
		SwarmingDimensions: map[string]string{
			"os":          "Android",
			"device_type": "redfin",
			"device_os":   "RD1A.200810.022.A4",
		},
	},
	"Pixel5_Android12": {
		Name: "Pixel5_Android12",
		Keys: map[string]string{
			"arch":  "arm64",
			"model": "Pixel5",
			"os":    "Android12",
		},
		GPU: "Adreno620",
		// Based on
		// https://skia.googlesource.com/skia/+/f8daeeb7f092abe1674bc2303c0781f9fb1756ab/infra/bots/gen_tasks_logic/gen_tasks_logic.go#910.
		SwarmingDimensions: map[string]string{
			"os":          "Android",
			"device_type": "redfin",
			"device_os":   "SP2A.220305.012",
		},
	},
	"Pixel6": {
		Name: "Pixel6",
		Keys: map[string]string{
			"arch":  "arm64",
			"model": "Pixel6",
			"os":    "Android",
		},
		GPU: "MaliG78",
	},
	"Pixel7": {
		Name: "Pixel7",
		Keys: map[string]string{
			"arch":  "arm64",
			"model": "Pixel7",
			"os":    "Android",
		},
		GPU: "MaliG710",
	},
	"RUBYR5_Debian11": {
		Name: "RUBYR5_Debian11",
		Keys: map[string]string{
			"arch":  "x86_64",
			"model": "RUBYR5",
			"os":    "Debian11",
		},
		GPU: "RadeonVega6",
	},
	"RUBYR5_Win10": {
		Name: "RUBYR5_Win10",
		Keys: map[string]string{
			"arch":  "x86_64",
			"model": "RUBYR5",
			"os":    "Win10",
		},
		GPU: "RadeonVega6",
	},
	"ShuttleA_Debian10_GTX660": {
		Name: "ShuttleA_Debian10_GTX660",
		Keys: map[string]string{
			"arch":  "x86_64",
			"model": "ShuttleA",
			"os":    "Debian10",
		},
		GPU: "GTX660",
	},
	"ShuttleA_Debian10_IntelHD2000": {
		Name: "ShuttleA_Debian10_IntelHD2000",
		Keys: map[string]string{
			"arch":  "x86_64",
			"model": "ShuttleA",
			"os":    "Debian10",
		},
		GPU: "IntelHD2000",
	},
	"ShuttleA_Debian10_RadeonHD7770": {
		Name: "ShuttleA_Debian10_RadeonHD7770",
		Keys: map[string]string{
			"arch":  "x86_64",
			"model": "ShuttleA",
			"os":    "Debian10",
		},
		GPU: "RadeonHD7770",
	},
	"ShuttleA_Win10_GTX660": {
		Name: "ShuttleA_Win10_GTX660",
		Keys: map[string]string{
			"arch":  "x86_64",
			"model": "ShuttleA",
			"os":    "Win10",
		},
		GPU: "GTX660",
	},
	"ShuttleA_Win10_IntelHD2000": {
		Name: "ShuttleA_Win10_IntelHD2000",
		Keys: map[string]string{
			"arch":  "x86_64",
			"model": "ShuttleA",
			"os":    "Win10",
		},
		GPU: "IntelHD2000",
	},
	"ShuttleA_Win10_RadeonHD7770": {
		Name: "ShuttleA_Win10_RadeonHD7770",
		Keys: map[string]string{
			"arch":  "x86_64",
			"model": "ShuttleA",
			"os":    "Win10",
		},
		GPU: "RadeonHD7770",
	},
	"ShuttleC": {
		Name: "ShuttleC",
		Keys: map[string]string{
			"arch":  "x86_64",
			"model": "ShuttleC",
			"os":    "Win10",
		},
		GPU: "GTX960",
	},
	"Sparky360": {
		Name: "Sparky360",
		Keys: map[string]string{
			"arch":  "x86_64",
			"model": "Sparky360",
			"os":    "ChromeOS",
		},
		GPU: "IntelUHDGraphics605",
	},
	"Spin513": {
		Name: "Spin513",
		Keys: map[string]string{
			"arch":  "arm",
			"model": "Spin513",
			"os":    "ChromeOS",
		},
		GPU: "Adreno618",
	},
	"Spin514": {
		Name: "Spin514",
		Keys: map[string]string{
			"arch":  "x86_64",
			"model": "Spin514",
			"os":    "ChromeOS",
		},
		GPU: "RadeonVega3",
	},
	"VMware7.1_Mac10.13": {
		Name: "VMware7.1_Mac10.13",
		Keys: map[string]string{
			"arch":  "x86_64",
			"model": "VMware7.1",
			"os":    "Mac10.13",
		},
		CPU: "AVX",
	},
	"VMware7.1_Mac10.14": {
		Name: "VMware7.1_Mac10.14",
		Keys: map[string]string{
			"arch":  "x86_64",
			"model": "VMware7.1",
			"os":    "Mac10.14",
		},
		CPU: "AVX",
	},
	"VMware7.1_Mac10.15.7": {
		Name: "VMware7.1_Mac10.15.7",
		Keys: map[string]string{
			"arch":  "x86_64",
			"model": "VMware7.1",
			"os":    "Mac10.15.7",
		},
		CPU: "AVX",
	},
	"Wembley": {
		Name: "Wembley",
		Keys: map[string]string{
			"arch":  "arm",
			"model": "Wembley",
			"os":    "Android",
		},
		GPU: "PowerVRGE8320",
	},
	"iPadPro": {
		Name: "iPadPro",
		Keys: map[string]string{
			"arch":  "arm64",
			"model": "iPadPro",
			"os":    "iOS",
		},
		GPU: "PowerVRGT7800",
	},
	"iPhone11": {
		Name: "iPhone11",
		Keys: map[string]string{
			"arch":  "arm64",
			"model": "iPhone11",
			"os":    "iOS",
		},
		GPU: "AppleA13",
	},
	"iPhone7": {
		Name: "iPhone7",
		Keys: map[string]string{
			"arch":  "arm64",
			"model": "iPhone7",
			"os":    "iOS",
		},
		GPU: "PowerVRGT7600",
	},
	"iPhone8": {
		Name: "iPhone8",
		Keys: map[string]string{
			"arch":  "arm64",
			"model": "iPhone8",
			"os":    "iOS",
		},
		GPU: "AppleA11",
	},
}
