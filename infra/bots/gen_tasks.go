// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package main

/*
	Generate the tasks.json file.
*/

import (
	"encoding/json"
	"flag"
	"fmt"
	"io/ioutil"
	"os"
	"path"
	"path/filepath"
	"regexp"
	"runtime"
	"sort"
	"strconv"
	"strings"
	"time"

	"github.com/golang/glog"
	"go.skia.org/infra/go/sklog"
	"go.skia.org/infra/go/util"
	"go.skia.org/infra/task_scheduler/go/specs"
)

const (
	BUNDLE_RECIPES_NAME        = "Housekeeper-PerCommit-BundleRecipes"
	ISOLATE_GCLOUD_LINUX_NAME  = "Housekeeper-PerCommit-IsolateGCloudLinux"
	ISOLATE_GO_DEPS_NAME       = "Housekeeper-PerCommit-IsolateGoDeps"
	ISOLATE_SKIMAGE_NAME       = "Housekeeper-PerCommit-IsolateSkImage"
	ISOLATE_SKP_NAME           = "Housekeeper-PerCommit-IsolateSKP"
	ISOLATE_SVG_NAME           = "Housekeeper-PerCommit-IsolateSVG"
	ISOLATE_NDK_LINUX_NAME     = "Housekeeper-PerCommit-IsolateAndroidNDKLinux"
	ISOLATE_SDK_LINUX_NAME     = "Housekeeper-PerCommit-IsolateAndroidSDKLinux"
	ISOLATE_WIN_TOOLCHAIN_NAME = "Housekeeper-PerCommit-IsolateWinToolchain"

	DEFAULT_OS_DEBIAN    = "Debian-9.4"
	DEFAULT_OS_LINUX_GCE = "Debian-9.8"
	DEFAULT_OS_MAC       = "Mac-10.13.6"
	DEFAULT_OS_WIN       = "Windows-2016Server-14393"

	DEFAULT_PROJECT = "skia"

	// Small is a 2-core machine.
	// TODO(dogben): Would n1-standard-1 or n1-standard-2 be sufficient?
	MACHINE_TYPE_SMALL = "n1-highmem-2"
	// Medium is a 16-core machine
	MACHINE_TYPE_MEDIUM = "n1-standard-16"
	// Large is a 64-core machine. (We use "highcpu" because we don't need more than 57GB memory for
	// any of our tasks.)
	MACHINE_TYPE_LARGE = "n1-highcpu-64"

	// Swarming output dirs.
	OUTPUT_NONE  = "output_ignored" // This will result in outputs not being isolated.
	OUTPUT_BUILD = "build"
	OUTPUT_TEST  = "test"
	OUTPUT_PERF  = "perf"

	// Name prefix for upload jobs.
	PREFIX_UPLOAD = "Upload"

	SERVICE_ACCOUNT_COMPILE            = "skia-external-compile-tasks@skia-swarming-bots.iam.gserviceaccount.com"
	SERVICE_ACCOUNT_HOUSEKEEPER        = "skia-external-housekeeper@skia-swarming-bots.iam.gserviceaccount.com"
	SERVICE_ACCOUNT_RECREATE_SKPS      = "skia-recreate-skps@skia-swarming-bots.iam.gserviceaccount.com"
	SERVICE_ACCOUNT_UPDATE_GO_DEPS     = "skia-recreate-skps@skia-swarming-bots.iam.gserviceaccount.com"
	SERVICE_ACCOUNT_UPDATE_META_CONFIG = "skia-update-meta-config@skia-swarming-bots.iam.gserviceaccount.com"
	SERVICE_ACCOUNT_UPLOAD_BINARY      = "skia-external-binary-uploader@skia-swarming-bots.iam.gserviceaccount.com"
	SERVICE_ACCOUNT_UPLOAD_CALMBENCH   = "skia-external-calmbench-upload@skia-swarming-bots.iam.gserviceaccount.com"
	SERVICE_ACCOUNT_UPLOAD_GM          = "skia-external-gm-uploader@skia-swarming-bots.iam.gserviceaccount.com"
	SERVICE_ACCOUNT_UPLOAD_NANO        = "skia-external-nano-uploader@skia-swarming-bots.iam.gserviceaccount.com"
)

var (
	// "Constants"

	// Top-level list of all jobs to run at each commit; loaded from
	// jobs.json.
	JOBS []string

	// General configuration information.
	CONFIG struct {
		GsBucketGm    string   `json:"gs_bucket_gm"`
		GoldHashesURL string   `json:"gold_hashes_url"`
		GsBucketNano  string   `json:"gs_bucket_nano"`
		GsBucketCalm  string   `json:"gs_bucket_calm"`
		NoUpload      []string `json:"no_upload"`
		Pool          string   `json:"pool"`
	}

	// alternateProject can be set in an init function to override the default project ID.
	alternateProject string

	// alternateServiceAccount can be set in an init function to override the normal service accounts.
	// Takes one of SERVICE_ACCOUNT_* constants as an argument and returns the service account that
	// should be used, or uses sklog.Fatal to indicate a problem.
	alternateServiceAccount func(serviceAccountEnum string) string

	// alternateSwarmDimensions can be set in an init function to override the default swarming bot
	// dimensions for the given task.
	alternateSwarmDimensions func(parts map[string]string) []string

	// internalHardwareLabelFn can be set in an init function to provide an
	// internal_hardware_label variable to the recipe.
	internalHardwareLabelFn func(parts map[string]string) *int

	// Defines the structure of job names.
	jobNameSchema *JobNameSchema

	// Named caches used by tasks.
	CACHES_GIT = []*specs.Cache{
		&specs.Cache{
			Name: "git",
			Path: "cache/git",
		},
		&specs.Cache{
			Name: "git_cache",
			Path: "cache/git_cache",
		},
	}
	CACHES_GO = []*specs.Cache{
		&specs.Cache{
			Name: "go_cache",
			Path: "cache/go_cache",
		},
	}
	CACHES_WORKDIR = []*specs.Cache{
		&specs.Cache{
			Name: "work",
			Path: "cache/work",
		},
	}
	CACHES_DOCKER = []*specs.Cache{
		&specs.Cache{
			Name: "docker",
			Path: "cache/docker",
		},
	}
	// Versions of the following copied from
	// https://chrome-internal.googlesource.com/infradata/config/+/master/configs/cr-buildbucket/swarming_task_template_canary.json#42
	// to test the fix for chromium:836196.
	// (In the future we may want to use versions from
	// https://chrome-internal.googlesource.com/infradata/config/+/master/configs/cr-buildbucket/swarming_task_template.json#42)
	// TODO(borenet): Roll these versions automatically!
	CIPD_PKGS_PYTHON = []*specs.CipdPackage{
		&specs.CipdPackage{
			Name:    "infra/tools/luci/vpython/${platform}",
			Path:    "cipd_bin_packages",
			Version: "git_revision:96f81e737868d43124b4661cf1c325296ca04944",
		},
	}

	CIPD_PKGS_CPYTHON = []*specs.CipdPackage{
		&specs.CipdPackage{
			Name:    "infra/python/cpython/${platform}",
			Path:    "cipd_bin_packages",
			Version: "version:2.7.14.chromium14",
		},
	}

	CIPD_PKGS_KITCHEN = append([]*specs.CipdPackage{
		&specs.CipdPackage{
			Name:    "infra/tools/luci/kitchen/${platform}",
			Path:    ".",
			Version: "git_revision:d8f38ca9494b5af249942631f9cee45927f6b4bc",
		},
		&specs.CipdPackage{
			Name:    "infra/tools/luci-auth/${platform}",
			Path:    "cipd_bin_packages",
			Version: "git_revision:2c805f1c716f6c5ad2126b27ec88b8585a09481e",
		},
	}, CIPD_PKGS_PYTHON...)

	CIPD_PKGS_GIT = []*specs.CipdPackage{
		&specs.CipdPackage{
			Name:    "infra/git/${platform}",
			Path:    "cipd_bin_packages",
			Version: "version:2.17.1.chromium15",
		},
		&specs.CipdPackage{
			Name:    "infra/tools/git/${platform}",
			Path:    "cipd_bin_packages",
			Version: "git_revision:c9c8a52bfeaf8bc00ece22fdfd447822c8fcad77",
		},
		&specs.CipdPackage{
			Name:    "infra/tools/luci/git-credential-luci/${platform}",
			Path:    "cipd_bin_packages",
			Version: "git_revision:2c805f1c716f6c5ad2126b27ec88b8585a09481e",
		},
	}

	CIPD_PKGS_GSUTIL = []*specs.CipdPackage{
		&specs.CipdPackage{
			Name:    "infra/gsutil",
			Path:    "cipd_bin_packages",
			Version: "version:4.28",
		},
	}

	CIPD_PKGS_XCODE = []*specs.CipdPackage{
		// https://chromium.googlesource.com/chromium/tools/build/+/e19b7d9390e2bb438b566515b141ed2b9ed2c7c2/scripts/slave/recipe_modules/ios/api.py#317
		// This package is really just an installer for XCode.
		&specs.CipdPackage{
			Name: "infra/tools/mac_toolchain/${platform}",
			Path: "mac_toolchain",
			// When this is updated, also update
			// https://skia.googlesource.com/skcms.git/+/f1e2b45d18facbae2dece3aca673fe1603077846/infra/bots/gen_tasks.go#56
			Version: "git_revision:796d2b92cff93fc2059623ce0a66284373ceea0a",
		},
	}

	// Flags.
	builderNameSchemaFile = flag.String("builder_name_schema", "", "Path to the builder_name_schema.json file. If not specified, uses infra/bots/recipe_modules/builder_name_schema/builder_name_schema.json from this repo.")
	assetsDir             = flag.String("assets_dir", "", "Directory containing assets.")
	cfgFile               = flag.String("cfg_file", "", "JSON file containing general configuration information.")
	jobsFile              = flag.String("jobs", "", "JSON file containing jobs to run.")
)

// Build the LogDog annotation URL.
func logdogAnnotationUrl() string {
	project := DEFAULT_PROJECT
	if alternateProject != "" {
		project = alternateProject
	}
	return fmt.Sprintf("logdog://logs.chromium.org/%s/%s/+/annotations", project, specs.PLACEHOLDER_TASK_ID)
}

// Create a properties JSON string.
func props(p map[string]string) string {
	d := make(map[string]interface{}, len(p)+1)
	for k, v := range p {
		d[k] = interface{}(v)
	}
	d["$kitchen"] = struct {
		DevShell bool `json:"devshell"`
		GitAuth  bool `json:"git_auth"`
	}{
		DevShell: true,
		GitAuth:  true,
	}

	j, err := json.Marshal(d)
	if err != nil {
		sklog.Fatal(err)
	}
	return strings.Replace(string(j), "\\u003c", "<", -1)
}

// kitchenTask returns a specs.TaskSpec instance which uses Kitchen to run a
// recipe.
func kitchenTask(name, recipe, isolate, serviceAccount string, dimensions []string, extraProps map[string]string, outputDir string) *specs.TaskSpec {
	if serviceAccount != "" && alternateServiceAccount != nil {
		serviceAccount = alternateServiceAccount(serviceAccount)
	}
	cipd := append([]*specs.CipdPackage{}, CIPD_PKGS_KITCHEN...)
	if strings.Contains(name, "Win") {
		cipd = append(cipd, CIPD_PKGS_CPYTHON...)
	}
	properties := map[string]string{
		"buildbucket_build_id": specs.PLACEHOLDER_BUILDBUCKET_BUILD_ID,
		"buildername":          name,
		"patch_issue":          specs.PLACEHOLDER_ISSUE,
		"patch_ref":            specs.PLACEHOLDER_PATCH_REF,
		"patch_repo":           specs.PLACEHOLDER_PATCH_REPO,
		"patch_set":            specs.PLACEHOLDER_PATCHSET,
		"patch_storage":        specs.PLACEHOLDER_PATCH_STORAGE,
		"repository":           specs.PLACEHOLDER_REPO,
		"revision":             specs.PLACEHOLDER_REVISION,
		"swarm_out_dir":        outputDir,
		"task_id":              specs.PLACEHOLDER_TASK_ID,
	}
	for k, v := range extraProps {
		properties[k] = v
	}
	var outputs []string = nil
	if outputDir != OUTPUT_NONE {
		outputs = []string{outputDir}
	}
	task := &specs.TaskSpec{
		Caches: []*specs.Cache{
			&specs.Cache{
				Name: "vpython",
				Path: "cache/vpython",
			},
		},
		CipdPackages: cipd,
		Command: []string{
			"./kitchen${EXECUTABLE_SUFFIX}", "cook",
			"-checkout-dir", "recipe_bundle",
			"-mode", "swarming",
			"-luci-system-account", "system",
			"-cache-dir", "cache",
			"-temp-dir", "tmp",
			"-known-gerrit-host", "android.googlesource.com",
			"-known-gerrit-host", "boringssl.googlesource.com",
			"-known-gerrit-host", "chromium.googlesource.com",
			"-known-gerrit-host", "dart.googlesource.com",
			"-known-gerrit-host", "fuchsia.googlesource.com",
			"-known-gerrit-host", "go.googlesource.com",
			"-known-gerrit-host", "llvm.googlesource.com",
			"-known-gerrit-host", "skia.googlesource.com",
			"-known-gerrit-host", "webrtc.googlesource.com",
			"-output-result-json", "${ISOLATED_OUTDIR}/build_result_filename",
			"-workdir", ".",
			"-recipe", recipe,
			"-properties", props(properties),
			"-logdog-annotation-url", logdogAnnotationUrl(),
		},
		Dependencies: []string{BUNDLE_RECIPES_NAME},
		Dimensions:   dimensions,
		EnvPrefixes: map[string][]string{
			"PATH":                    []string{"cipd_bin_packages", "cipd_bin_packages/bin"},
			"VPYTHON_VIRTUALENV_ROOT": []string{"cache/vpython"},
		},
		ExtraTags: map[string]string{
			"log_location": logdogAnnotationUrl(),
		},
		Isolate:        relpath(isolate),
		MaxAttempts:    attempts(name),
		Outputs:        outputs,
		ServiceAccount: serviceAccount,
	}
	timeout(task, time.Hour)
	return task
}

// internalHardwareLabel returns the internal ID for the bot, if any.
func internalHardwareLabel(parts map[string]string) *int {
	if internalHardwareLabelFn != nil {
		return internalHardwareLabelFn(parts)
	}
	return nil
}

// linuxGceDimensions are the Swarming dimensions for Linux GCE instances.
func linuxGceDimensions(machineType string) []string {
	return []string{
		// Specify CPU to avoid running builds on bots with a more unique CPU.
		"cpu:x86-64-Haswell_GCE",
		"gpu:none",
		// Currently all Linux GCE tasks run on 16-CPU machines.
		fmt.Sprintf("machine_type:%s", machineType),
		fmt.Sprintf("os:%s", DEFAULT_OS_LINUX_GCE),
		fmt.Sprintf("pool:%s", CONFIG.Pool),
	}
}

func dockerGceDimensions() []string {
	// There's limited parallelism for WASM builds, so we can get away with the medium
	// instance instead of the beefy large instance.
	// Docker being installed is the most important part.
	return append(linuxGceDimensions(MACHINE_TYPE_MEDIUM), "docker_installed:true")
}

// deriveCompileTaskName returns the name of a compile task based on the given
// job name.
func deriveCompileTaskName(jobName string, parts map[string]string) string {
	if parts["role"] == "Test" || parts["role"] == "Perf" || parts["role"] == "Calmbench" {
		task_os := parts["os"]
		ec := []string{}
		if val := parts["extra_config"]; val != "" {
			ec = strings.Split(val, "_")
			ignore := []string{"Skpbench", "AbandonGpuContext", "PreAbandonGpuContext", "Valgrind", "ReleaseAndAbandonGpuContext", "CCPR", "FSAA", "FAAA", "FDAA", "NativeFonts", "GDI", "NoGPUThreads", "ProcDump", "DDL1", "DDL3", "T8888", "DDLTotal", "DDLRecord", "9x9", "BonusConfigs", "SkottieTracing"}
			keep := make([]string, 0, len(ec))
			for _, part := range ec {
				if !util.In(part, ignore) {
					keep = append(keep, part)
				}
			}
			ec = keep
		}
		if task_os == "Android" {
			if !util.In("Android", ec) {
				ec = append([]string{"Android"}, ec...)
			}
			task_os = "Debian9"
		} else if task_os == "Chromecast" {
			task_os = "Debian9"
			ec = append([]string{"Chromecast"}, ec...)
		} else if strings.Contains(task_os, "ChromeOS") {
			ec = append([]string{"Chromebook", "GLES"}, ec...)
			task_os = "Debian9"
		} else if task_os == "iOS" {
			ec = append([]string{task_os}, ec...)
			task_os = "Mac"
		} else if strings.Contains(task_os, "Win") {
			task_os = "Win"
		} else if strings.Contains(task_os, "Ubuntu") || strings.Contains(task_os, "Debian") {
			task_os = "Debian9"
		} else if strings.Contains(task_os, "Mac") {
			task_os = "Mac"
		}
		jobNameMap := map[string]string{
			"role":          "Build",
			"os":            task_os,
			"compiler":      parts["compiler"],
			"target_arch":   parts["arch"],
			"configuration": parts["configuration"],
		}
		if strings.Contains(jobName, "PathKit") {
			ec = []string{"PathKit"}
		}
		if strings.Contains(jobName, "CanvasKit") {
			if parts["cpu_or_gpu"] == "CPU" {
				ec = []string{"CanvasKit_CPU"}
			} else {
				ec = []string{"CanvasKit"}
			}

		}
		if len(ec) > 0 {
			jobNameMap["extra_config"] = strings.Join(ec, "_")
		}
		name, err := jobNameSchema.MakeJobName(jobNameMap)
		if err != nil {
			glog.Fatal(err)
		}
		return name
	} else if parts["role"] == "BuildStats" {
		return strings.Replace(jobName, "BuildStats", "Build", 1)
	} else {
		return jobName
	}
}

// swarmDimensions generates swarming bot dimensions for the given task.
func swarmDimensions(parts map[string]string) []string {
	if alternateSwarmDimensions != nil {
		return alternateSwarmDimensions(parts)
	}
	return defaultSwarmDimensions(parts)
}

// defaultSwarmDimensions generates default swarming bot dimensions for the given task.
func defaultSwarmDimensions(parts map[string]string) []string {
	d := map[string]string{
		"pool": CONFIG.Pool,
	}
	if os, ok := parts["os"]; ok {
		d["os"], ok = map[string]string{
			"Android":    "Android",
			"Chromecast": "Android",
			"ChromeOS":   "ChromeOS",
			"Debian9":    DEFAULT_OS_DEBIAN,
			"Mac":        DEFAULT_OS_MAC,
			"Mac10.13":   DEFAULT_OS_MAC,
			"Mac10.14":   "Mac-10.14.3",
			"Ubuntu18":   "Ubuntu-18.04",
			"Win":        DEFAULT_OS_WIN,
			"Win10":      "Windows-10-17763.557",
			"Win2016":    DEFAULT_OS_WIN,
			"Win7":       "Windows-7-SP1",
			"Win8":       "Windows-8.1-SP0",
			"iOS":        "iOS-11.4.1",
		}[os]
		if !ok {
			glog.Fatalf("Entry %q not found in OS mapping.", os)
		}
		if os == "Win10" && parts["model"] == "Golo" {
			// ChOps-owned machines have Windows 10 v1709, but a slightly different version than Skolo.
			d["os"] = "Windows-10-16299.309"
		}
		if d["os"] == DEFAULT_OS_WIN {
			// Upgrades result in a new image but not a new OS version.
			d["image"] = "windows-server-2016-dc-v20190108"
		}
	} else {
		d["os"] = DEFAULT_OS_DEBIAN
	}
	if parts["role"] == "Test" || parts["role"] == "Perf" || parts["role"] == "Calmbench" {
		if strings.Contains(parts["os"], "Android") || strings.Contains(parts["os"], "Chromecast") {
			// For Android, the device type is a better dimension
			// than CPU or GPU.
			deviceInfo, ok := map[string][]string{
				"AndroidOne":      {"sprout", "MOB30Q"},
				"Chorizo":         {"chorizo", "1.30_109591"},
				"GalaxyS6":        {"zerofltetmo", "NRD90M_G920TUVU5FQK1"},
				"GalaxyS7_G930FD": {"herolte", "R16NW_G930FXXS2ERH6"}, // This is Oreo.
				"GalaxyS9":        {"starlte", "R16NW_G960FXXU2BRJ8"}, // This is Oreo.
				"MotoG4":          {"athene", "NPJS25.93-14.7-8"},
				"NVIDIA_Shield":   {"foster", "OPR6.170623.010"},
				"Nexus5":          {"hammerhead", "M4B30Z_3437181"},
				"Nexus5x":         {"bullhead", "OPR6.170623.023"},
				"Nexus7":          {"grouper", "LMY47V_1836172"}, // 2012 Nexus 7
				"NexusPlayer":     {"fugu", "OPR2.170623.027"},
				"Pixel":           {"sailfish", "PPR1.180610.009"},
				"Pixel2XL":        {"taimen", "PPR1.180610.009"},
				"Pixel3":          {"blueline", "PQ1A.190105.004"},
			}[parts["model"]]
			if !ok {
				glog.Fatalf("Entry %q not found in Android mapping.", parts["model"])
			}
			d["device_type"] = deviceInfo[0]
			d["device_os"] = deviceInfo[1]
		} else if strings.Contains(parts["os"], "iOS") {
			device, ok := map[string]string{
				"iPadMini4": "iPad5,1",
				"iPhone6":   "iPhone7,2",
				"iPhone7":   "iPhone9,1",
				"iPadPro":   "iPad6,3",
			}[parts["model"]]
			if !ok {
				glog.Fatalf("Entry %q not found in iOS mapping.", parts["model"])
			}
			d["device"] = device
		} else if strings.Contains(parts["extra_config"], "SwiftShader") {
			if parts["model"] != "GCE" || d["os"] != DEFAULT_OS_DEBIAN || parts["cpu_or_gpu_value"] != "SwiftShader" {
				glog.Fatalf("Please update defaultSwarmDimensions for SwiftShader %s %s %s.", parts["os"], parts["model"], parts["cpu_or_gpu_value"])
			}
			d["cpu"] = "x86-64-Haswell_GCE"
			d["os"] = DEFAULT_OS_LINUX_GCE
			d["machine_type"] = MACHINE_TYPE_SMALL
		} else if strings.Contains(parts["extra_config"], "SKQP") && parts["cpu_or_gpu_value"] == "Emulator" {
			if parts["model"] != "NUC7i5BNK" || d["os"] != DEFAULT_OS_DEBIAN {
				glog.Fatalf("Please update defaultSwarmDimensions for SKQP::Emulator %s %s.", parts["os"], parts["model"])
			}
			d["cpu"] = "x86-64-i5-7260U"
			d["os"] = DEFAULT_OS_DEBIAN
			// KVM means Kernel-based Virtual Machine, that is, can this vm virtualize commands
			// For us, this means, can we run an x86 android emulator on it.
			// kjlubick tried running this on GCE, but it was a bit too slow on the large install.
			// So, we run on bare metal machines in the Skolo (that should also have KVM).
			d["kvm"] = "1"
			d["docker_installed"] = "true"
		} else if parts["cpu_or_gpu"] == "CPU" {
			modelMapping, ok := map[string]map[string]string{
				"AVX": {
					"Golo": "x86-64-E5-2670",
				},
				"AVX2": {
					"GCE":            "x86-64-Haswell_GCE",
					"MacBookAir7.2":  "x86-64-i5-5350U",
					"MacBookPro11.5": "x86-64-i7-4870HQ",
					"NUC5i7RYH":      "x86-64-i7-5557U",
				},
				"AVX512": {
					"GCE": "x86-64-Skylake_GCE",
				},
			}[parts["cpu_or_gpu_value"]]
			if !ok {
				glog.Fatalf("Entry %q not found in CPU mapping.", parts["cpu_or_gpu_value"])
			}
			cpu, ok := modelMapping[parts["model"]]
			if !ok {
				glog.Fatalf("Entry %q not found in %q model mapping.", parts["model"], parts["cpu_or_gpu_value"])
			}
			d["cpu"] = cpu
			if parts["model"] == "GCE" && d["os"] == DEFAULT_OS_DEBIAN {
				d["os"] = DEFAULT_OS_LINUX_GCE
			}
			if parts["model"] == "GCE" && d["cpu"] == "x86-64-Haswell_GCE" {
				d["machine_type"] = MACHINE_TYPE_MEDIUM
			}
		} else {
			if strings.Contains(parts["extra_config"], "CanvasKit") {
				// GPU is defined for the WebGL version of CanvasKit, but
				// it can still run on a GCE instance.
				return dockerGceDimensions()
			} else if strings.Contains(parts["os"], "Win") {
				gpu, ok := map[string]string{
					"GT610":         "10de:104a-23.21.13.9101",
					"GTX660":        "10de:11c0-25.21.14.1634",
					"GTX960":        "10de:1401-25.21.14.1634",
					"IntelHD4400":   "8086:0a16-20.19.15.4963",
					"IntelIris540":  "8086:1926-25.20.100.6519",
					"IntelIris6100": "8086:162b-20.19.15.4963",
					"IntelIris655":  "8086:3ea5-25.20.100.6519",
					"RadeonHD7770":  "1002:683d-24.20.13001.1010",
					"RadeonR9M470X": "1002:6646-24.20.13001.1010",
					"QuadroP400":    "10de:1cb3-25.21.14.1678",
				}[parts["cpu_or_gpu_value"]]
				if !ok {
					glog.Fatalf("Entry %q not found in Win GPU mapping.", parts["cpu_or_gpu_value"])
				}
				d["gpu"] = gpu
			} else if strings.Contains(parts["os"], "Ubuntu") || strings.Contains(parts["os"], "Debian") {
				gpu, ok := map[string]string{
					// Intel drivers come from CIPD, so no need to specify the version here.
					"IntelBayTrail": "8086:0f31",
					"IntelHD2000":   "8086:0102",
					"IntelHD405":    "8086:22b1",
					"IntelIris640":  "8086:5926",
					"QuadroP400":    "10de:1cb3-430.14",
				}[parts["cpu_or_gpu_value"]]
				if !ok {
					glog.Fatalf("Entry %q not found in Ubuntu GPU mapping.", parts["cpu_or_gpu_value"])
				}
				d["gpu"] = gpu
			} else if strings.Contains(parts["os"], "Mac") {
				gpu, ok := map[string]string{
					"IntelHD6000":   "8086:1626",
					"IntelHD615":    "8086:591e",
					"IntelIris5100": "8086:0a2e",
					"RadeonHD8870M": "1002:6821-4.0.20-3.2.8",
				}[parts["cpu_or_gpu_value"]]
				if !ok {
					glog.Fatalf("Entry %q not found in Mac GPU mapping.", parts["cpu_or_gpu_value"])
				}
				d["gpu"] = gpu
				// Yuck. We have two different types of MacMini7,1 with the same GPU but different CPUs.
				if parts["cpu_or_gpu_value"] == "IntelIris5100" {
					// Run all tasks on Golo machines for now.
					d["cpu"] = "x86-64-i7-4578U"
				}
			} else if strings.Contains(parts["os"], "ChromeOS") {
				version, ok := map[string]string{
					"MaliT604":           "10575.22.0",
					"MaliT764":           "10575.22.0",
					"MaliT860":           "10575.22.0",
					"PowerVRGX6250":      "10575.22.0",
					"TegraK1":            "10575.22.0",
					"IntelHDGraphics615": "10575.22.0",
				}[parts["cpu_or_gpu_value"]]
				if !ok {
					glog.Fatalf("Entry %q not found in ChromeOS GPU mapping.", parts["cpu_or_gpu_value"])
				}
				d["gpu"] = parts["cpu_or_gpu_value"]
				d["release_version"] = version
			} else {
				glog.Fatalf("Unknown GPU mapping for OS %q.", parts["os"])
			}
		}
	} else {
		d["gpu"] = "none"
		if d["os"] == DEFAULT_OS_DEBIAN {
			if strings.Contains(parts["extra_config"], "PathKit") || strings.Contains(parts["extra_config"], "CanvasKit") || strings.Contains(parts["extra_config"], "CMake") {
				return dockerGceDimensions()
			}
			if parts["role"] == "BuildStats" {
				// Doesn't require a lot of resources, but some steps require docker
				return dockerGceDimensions()
			}
			// Use many-core machines for Build tasks.
			return linuxGceDimensions(MACHINE_TYPE_LARGE)
		} else if d["os"] == DEFAULT_OS_WIN {
			// Windows CPU bots.
			d["cpu"] = "x86-64-Haswell_GCE"
			// Use many-core machines for Build tasks.
			d["machine_type"] = MACHINE_TYPE_LARGE
		} else if d["os"] == DEFAULT_OS_MAC {
			// Mac CPU bots.
			d["cpu"] = "x86-64-E5-2697_v2"
		}
	}

	rv := make([]string, 0, len(d))
	for k, v := range d {
		rv = append(rv, fmt.Sprintf("%s:%s", k, v))
	}
	sort.Strings(rv)
	return rv
}

// relpath returns the relative path to the given file from the config file.
func relpath(f string) string {
	_, filename, _, _ := runtime.Caller(0)
	dir := path.Dir(filename)
	rel := dir
	if *cfgFile != "" {
		rel = path.Dir(*cfgFile)
	}
	rv, err := filepath.Rel(rel, path.Join(dir, f))
	if err != nil {
		sklog.Fatal(err)
	}
	return rv
}

// bundleRecipes generates the task to bundle and isolate the recipes.
func bundleRecipes(b *specs.TasksCfgBuilder) string {
	pkgs := append([]*specs.CipdPackage{}, CIPD_PKGS_GIT...)
	pkgs = append(pkgs, CIPD_PKGS_PYTHON...)
	b.MustAddTask(BUNDLE_RECIPES_NAME, &specs.TaskSpec{
		CipdPackages: pkgs,
		Command: []string{
			"/bin/bash", "skia/infra/bots/bundle_recipes.sh", specs.PLACEHOLDER_ISOLATED_OUTDIR,
		},
		Dimensions: linuxGceDimensions(MACHINE_TYPE_SMALL),
		EnvPrefixes: map[string][]string{
			"PATH": []string{"cipd_bin_packages", "cipd_bin_packages/bin"},
		},
		Isolate: relpath("swarm_recipe.isolate"),
	})
	return BUNDLE_RECIPES_NAME
}

type isolateAssetCfg struct {
	cipdPkg string
	path    string
}

var ISOLATE_ASSET_MAPPING = map[string]isolateAssetCfg{
	ISOLATE_GCLOUD_LINUX_NAME: {
		cipdPkg: "gcloud_linux",
		path:    "gcloud_linux",
	},
	ISOLATE_GO_DEPS_NAME: {
		cipdPkg: "go_deps",
		path:    "go_deps",
	},
	ISOLATE_SKIMAGE_NAME: {
		cipdPkg: "skimage",
		path:    "skimage",
	},
	ISOLATE_SKP_NAME: {
		cipdPkg: "skp",
		path:    "skp",
	},
	ISOLATE_SVG_NAME: {
		cipdPkg: "svg",
		path:    "svg",
	},
	ISOLATE_NDK_LINUX_NAME: {
		cipdPkg: "android_ndk_linux",
		path:    "android_ndk_linux",
	},
	ISOLATE_SDK_LINUX_NAME: {
		cipdPkg: "android_sdk_linux",
		path:    "android_sdk_linux",
	},
	ISOLATE_WIN_TOOLCHAIN_NAME: {
		cipdPkg: "win_toolchain",
		path:    "win_toolchain",
	},
}

// isolateCIPDAsset generates a task to isolate the given CIPD asset.
func isolateCIPDAsset(b *specs.TasksCfgBuilder, name string) string {
	asset := ISOLATE_ASSET_MAPPING[name]
	b.MustAddTask(name, &specs.TaskSpec{
		CipdPackages: []*specs.CipdPackage{
			b.MustGetCipdPackageFromAsset(asset.cipdPkg),
		},
		Command:    []string{"/bin/cp", "-rL", asset.path, "${ISOLATED_OUTDIR}"},
		Dimensions: linuxGceDimensions(MACHINE_TYPE_SMALL),
		Isolate:    relpath("empty.isolate"),
	})
	return name
}

// getIsolatedCIPDDeps returns the slice of Isolate_* tasks a given task needs.
// This allows us to  save time on I/O bound bots, like the RPIs.
func getIsolatedCIPDDeps(parts map[string]string) []string {
	deps := []string{}
	// Only do this on the RPIs for now. Other, faster machines shouldn't see much
	// benefit and we don't need the extra complexity, for now
	rpiOS := []string{"Android", "ChromeOS", "iOS"}

	if o := parts["os"]; strings.Contains(o, "Chromecast") {
		// Chromecasts don't have enough disk space to fit all of the content,
		// so we do a subset of the skps.
		deps = append(deps, ISOLATE_SKP_NAME)
	} else if e := parts["extra_config"]; strings.Contains(e, "Skpbench") {
		// Skpbench only needs skps
		deps = append(deps, ISOLATE_SKP_NAME)
	} else if util.In(o, rpiOS) {
		deps = append(deps, ISOLATE_SKP_NAME)
		deps = append(deps, ISOLATE_SVG_NAME)
		deps = append(deps, ISOLATE_SKIMAGE_NAME)
	}

	return deps
}

// usesGit adds attributes to tasks which use git.
func usesGit(t *specs.TaskSpec, name string) {
	t.Caches = append(t.Caches, CACHES_GIT...)
	if !strings.Contains(name, "NoDEPS") {
		t.Caches = append(t.Caches, CACHES_WORKDIR...)
	}
	t.CipdPackages = append(t.CipdPackages, CIPD_PKGS_GIT...)
}

// usesGo adds attributes to tasks which use go. Recipes should use
// "with api.context(env=api.infra.go_env)".
// (Not needed for tasks that just want to run Go code from the infra repo -- instead use go_deps.)
func usesGo(b *specs.TasksCfgBuilder, t *specs.TaskSpec) {
	t.Caches = append(t.Caches, CACHES_GO...)
	t.CipdPackages = append(t.CipdPackages, b.MustGetCipdPackageFromAsset("go"))
	t.Dependencies = append(t.Dependencies, isolateCIPDAsset(b, ISOLATE_GO_DEPS_NAME))
}

// usesDocker adds attributes to tasks which use docker.
func usesDocker(t *specs.TaskSpec, name string) {
	if strings.Contains(name, "EMCC") || strings.Contains(name, "SKQP") || strings.Contains(name, "LottieWeb") || strings.Contains(name, "CMake") {
		t.Caches = append(t.Caches, CACHES_DOCKER...)
	}
}

// timeout sets the timeout(s) for this task.
func timeout(task *specs.TaskSpec, timeout time.Duration) {
	task.ExecutionTimeout = timeout
	task.IoTimeout = timeout // With kitchen, step logs don't count toward IoTimeout.
}

// attempts returns the desired MaxAttempts for this task.
func attempts(name string) int {
	if strings.Contains(name, "Android_Framework") {
		// The reason for this has been lost to time.
		return 1
	}
	if !(strings.HasPrefix(name, "Build-") || strings.HasPrefix(name, "Upload-")) {
		for _, extraConfig := range []string{"ASAN", "MSAN", "TSAN", "UBSAN", "Valgrind"} {
			if strings.Contains(name, extraConfig) {
				// Sanitizers often find non-deterministic issues that retries would hide.
				return 1
			}
		}
	}
	// Retry by default to hide random bot/hardware failures.
	return 2
}

// compile generates a compile task. Returns the name of the last task in the
// generated chain of tasks, which the Job should add as a dependency.
func compile(b *specs.TasksCfgBuilder, name string, parts map[string]string) string {
	task := kitchenTask(name, "compile", "swarm_recipe.isolate", SERVICE_ACCOUNT_COMPILE, swarmDimensions(parts), nil, OUTPUT_BUILD)
	usesGit(task, name)
	usesDocker(task, name)

	// Android bots require a toolchain.
	if strings.Contains(name, "Android") {
		if parts["extra_config"] == "Android_Framework" {
			// Do not need a toolchain when building the
			// Android Framework.
		} else if strings.Contains(name, "Mac") {
			task.CipdPackages = append(task.CipdPackages, b.MustGetCipdPackageFromAsset("android_ndk_darwin"))
		} else if strings.Contains(name, "Win") {
			pkg := b.MustGetCipdPackageFromAsset("android_ndk_windows")
			pkg.Path = "n"
			task.CipdPackages = append(task.CipdPackages, pkg)
		} else if !strings.Contains(name, "SKQP") {
			task.Dependencies = append(task.Dependencies, isolateCIPDAsset(b, ISOLATE_NDK_LINUX_NAME))
		}
	} else if strings.Contains(name, "Chromecast") {
		task.CipdPackages = append(task.CipdPackages, b.MustGetCipdPackageFromAsset("cast_toolchain"))
		task.CipdPackages = append(task.CipdPackages, b.MustGetCipdPackageFromAsset("chromebook_arm_gles"))
	} else if strings.Contains(name, "Chromebook") {
		task.CipdPackages = append(task.CipdPackages, b.MustGetCipdPackageFromAsset("clang_linux"))
		if parts["target_arch"] == "x86_64" {
			task.CipdPackages = append(task.CipdPackages, b.MustGetCipdPackageFromAsset("chromebook_x86_64_gles"))
		} else if parts["target_arch"] == "arm" {
			task.CipdPackages = append(task.CipdPackages, b.MustGetCipdPackageFromAsset("armhf_sysroot"))
			task.CipdPackages = append(task.CipdPackages, b.MustGetCipdPackageFromAsset("chromebook_arm_gles"))
		}
	} else if strings.Contains(name, "Debian") {
		if strings.Contains(name, "Clang") {
			task.CipdPackages = append(task.CipdPackages, b.MustGetCipdPackageFromAsset("clang_linux"))
		}
		if parts["target_arch"] == "mips64el" || parts["target_arch"] == "loongson3a" {
			if parts["compiler"] != "GCC" {
				glog.Fatalf("mips64el toolchain is GCC, but compiler is %q in %q", parts["compiler"], name)
			}
			task.CipdPackages = append(task.CipdPackages, b.MustGetCipdPackageFromAsset("mips64el_toolchain_linux"))
		}
		if strings.Contains(name, "SwiftShader") {
			task.CipdPackages = append(task.CipdPackages, b.MustGetCipdPackageFromAsset("cmake_linux"))
		}
		if strings.Contains(name, "OpenCL") {
			task.CipdPackages = append(task.CipdPackages,
				b.MustGetCipdPackageFromAsset("opencl_headers"),
				b.MustGetCipdPackageFromAsset("opencl_ocl_icd_linux"),
			)
		}
	} else if strings.Contains(name, "Win") {
		task.Dependencies = append(task.Dependencies, isolateCIPDAsset(b, ISOLATE_WIN_TOOLCHAIN_NAME))
		if strings.Contains(name, "Clang") {
			task.CipdPackages = append(task.CipdPackages, b.MustGetCipdPackageFromAsset("clang_win"))
		}
		if strings.Contains(name, "OpenCL") {
			task.CipdPackages = append(task.CipdPackages,
				b.MustGetCipdPackageFromAsset("opencl_headers"),
			)
		}
	} else if strings.Contains(name, "Mac") {
		task.CipdPackages = append(task.CipdPackages, CIPD_PKGS_XCODE...)
		task.Caches = append(task.Caches, &specs.Cache{
			Name: "xcode",
			Path: "cache/Xcode.app",
		})
		if strings.Contains(name, "CommandBuffer") {
			timeout(task, 2*time.Hour)
		}
		if strings.Contains(name, "MoltenVK") {
			task.CipdPackages = append(task.CipdPackages, b.MustGetCipdPackageFromAsset("moltenvk"))
		}
	}

	// Add the task.
	b.MustAddTask(name, task)

	// All compile tasks are runnable as their own Job. Assert that the Job
	// is listed in JOBS.
	if !util.In(name, JOBS) {
		glog.Fatalf("Job %q is missing from the JOBS list!", name)
	}

	// Upload the skiaserve binary only for Linux Android compile bots.
	// See skbug.com/7399 for context.
	if parts["configuration"] == "Release" &&
		parts["extra_config"] == "Android" &&
		!strings.Contains(parts["os"], "Win") &&
		!strings.Contains(parts["os"], "Mac") {
		uploadName := fmt.Sprintf("%s%s%s", PREFIX_UPLOAD, jobNameSchema.Sep, name)
		task := kitchenTask(uploadName, "upload_skiaserve", "swarm_recipe.isolate", SERVICE_ACCOUNT_UPLOAD_BINARY, linuxGceDimensions(MACHINE_TYPE_SMALL), nil, OUTPUT_NONE)
		task.Dependencies = append(task.Dependencies, name)
		b.MustAddTask(uploadName, task)
		return uploadName
	}

	return name
}

// recreateSKPs generates a RecreateSKPs task. Returns the name of the last
// task in the generated chain of tasks, which the Job should add as a
// dependency.
func recreateSKPs(b *specs.TasksCfgBuilder, name string) string {
	dims := []string{
		"pool:SkiaCT",
		fmt.Sprintf("os:%s", DEFAULT_OS_LINUX_GCE),
	}
	task := kitchenTask(name, "recreate_skps", "swarm_recipe.isolate", SERVICE_ACCOUNT_RECREATE_SKPS, dims, nil, OUTPUT_NONE)
	task.CipdPackages = append(task.CipdPackages, CIPD_PKGS_GIT...)
	usesGo(b, task)
	timeout(task, 4*time.Hour)
	b.MustAddTask(name, task)
	return name
}

// updateGoDEPS generates an UpdateGoDEPS task. Returns the name of the last
// task in the generated chain of tasks, which the Job should add as a
// dependency.
func updateGoDEPS(b *specs.TasksCfgBuilder, name string) string {
	dims := linuxGceDimensions(MACHINE_TYPE_LARGE)
	task := kitchenTask(name, "update_go_deps", "swarm_recipe.isolate", SERVICE_ACCOUNT_UPDATE_GO_DEPS, dims, nil, OUTPUT_NONE)
	task.CipdPackages = append(task.CipdPackages, CIPD_PKGS_GIT...)
	usesGo(b, task)
	b.MustAddTask(name, task)
	return name
}

// checkGeneratedFiles verifies that no generated SKSL files have been edited
// by hand.
func checkGeneratedFiles(b *specs.TasksCfgBuilder, name string) string {
	task := kitchenTask(name, "check_generated_files", "swarm_recipe.isolate", SERVICE_ACCOUNT_COMPILE, linuxGceDimensions(MACHINE_TYPE_LARGE), nil, OUTPUT_NONE)
	task.Caches = append(task.Caches, CACHES_WORKDIR...)
	usesGo(b, task)
	b.MustAddTask(name, task)
	return name
}

// housekeeper generates a Housekeeper task. Returns the name of the last task
// in the generated chain of tasks, which the Job should add as a dependency.
func housekeeper(b *specs.TasksCfgBuilder, name string) string {
	task := kitchenTask(name, "housekeeper", "swarm_recipe.isolate", SERVICE_ACCOUNT_HOUSEKEEPER, linuxGceDimensions(MACHINE_TYPE_SMALL), nil, OUTPUT_NONE)
	usesGit(task, name)
	b.MustAddTask(name, task)
	return name
}

// androidFrameworkCompile generates an Android Framework Compile task. Returns
// the name of the last task in the generated chain of tasks, which the Job
// should add as a dependency.
func androidFrameworkCompile(b *specs.TasksCfgBuilder, name string) string {
	task := kitchenTask(name, "android_compile", "swarm_recipe.isolate", SERVICE_ACCOUNT_COMPILE, linuxGceDimensions(MACHINE_TYPE_SMALL), nil, OUTPUT_NONE)
	timeout(task, time.Hour)
	b.MustAddTask(name, task)
	return name
}

// infra generates an infra_tests task. Returns the name of the last task in the
// generated chain of tasks, which the Job should add as a dependency.
func infra(b *specs.TasksCfgBuilder, name string) string {
	task := kitchenTask(name, "infra", "swarm_recipe.isolate", SERVICE_ACCOUNT_COMPILE, linuxGceDimensions(MACHINE_TYPE_SMALL), nil, OUTPUT_NONE)
	usesGit(task, name)
	usesGo(b, task)
	b.MustAddTask(name, task)
	return name
}

var BUILD_STATS_NO_UPLOAD = []string{"BuildStats-Debian9-Clang-x86_64-Release"}

func buildstats(b *specs.TasksCfgBuilder, name string, parts map[string]string, compileTaskName string) string {
	task := kitchenTask(name, "compute_buildstats", "swarm_recipe.isolate", "", swarmDimensions(parts), nil, OUTPUT_PERF)
	task.Dependencies = append(task.Dependencies, compileTaskName)
	task.CipdPackages = append(task.CipdPackages, b.MustGetCipdPackageFromAsset("bloaty"))
	b.MustAddTask(name, task)

	// Upload release results (for tracking in perf)
	// We have some jobs that are FYI (e.g. Debug-CanvasKit, tree-map generator)
	if strings.Contains(name, "Release") && !util.In(name, BUILD_STATS_NO_UPLOAD) {
		uploadName := fmt.Sprintf("%s%s%s", PREFIX_UPLOAD, jobNameSchema.Sep, name)
		extraProps := map[string]string{
			"gs_bucket": CONFIG.GsBucketNano,
		}
		uploadTask := kitchenTask(name, "upload_buildstats_results", "swarm_recipe.isolate", SERVICE_ACCOUNT_UPLOAD_NANO, linuxGceDimensions(MACHINE_TYPE_SMALL), extraProps, OUTPUT_NONE)
		uploadTask.CipdPackages = append(uploadTask.CipdPackages, CIPD_PKGS_GSUTIL...)
		uploadTask.Dependencies = append(uploadTask.Dependencies, name)
		b.MustAddTask(uploadName, uploadTask)
		return uploadName
	}

	return name
}

func getParentRevisionName(compileTaskName string, parts map[string]string) string {
	if parts["extra_config"] == "" {
		return compileTaskName + "-ParentRevision"
	} else {
		return compileTaskName + "_ParentRevision"
	}
}

// calmbench generates a calmbench task. Returns the name of the last task in the
// generated chain of tasks, which the Job should add as a dependency.
func calmbench(b *specs.TasksCfgBuilder, name string, parts map[string]string, compileTaskName, compileParentName string) string {
	task := kitchenTask(name, "calmbench", "calmbench.isolate", "", swarmDimensions(parts), nil, OUTPUT_PERF)
	usesGit(task, name)
	task.Dependencies = append(task.Dependencies, compileTaskName, compileParentName, ISOLATE_SKP_NAME, ISOLATE_SVG_NAME)
	b.MustAddTask(name, task)

	// Upload results if necessary.
	if strings.Contains(name, "Release") && doUpload(name) {
		uploadName := fmt.Sprintf("%s%s%s", PREFIX_UPLOAD, jobNameSchema.Sep, name)
		extraProps := map[string]string{
			"gs_bucket": CONFIG.GsBucketCalm,
		}
		uploadTask := kitchenTask(name, "upload_calmbench_results", "swarm_recipe.isolate", SERVICE_ACCOUNT_UPLOAD_CALMBENCH, linuxGceDimensions(MACHINE_TYPE_SMALL), extraProps, OUTPUT_NONE)
		uploadTask.CipdPackages = append(uploadTask.CipdPackages, CIPD_PKGS_GSUTIL...)
		uploadTask.Dependencies = append(uploadTask.Dependencies, name)
		b.MustAddTask(uploadName, uploadTask)
		return uploadName
	}

	return name
}

// doUpload indicates whether the given Job should upload its results.
func doUpload(name string) bool {
	for _, s := range CONFIG.NoUpload {
		m, err := regexp.MatchString(s, name)
		if err != nil {
			glog.Fatal(err)
		}
		if m {
			return false
		}
	}
	return true
}

// test generates a Test task. Returns the name of the last task in the
// generated chain of tasks, which the Job should add as a dependency.
func test(b *specs.TasksCfgBuilder, name string, parts map[string]string, compileTaskName string, pkgs []*specs.CipdPackage) string {
	recipe := "test"
	if strings.Contains(name, "SKQP") {
		recipe = "skqp_test"
		if strings.Contains(name, "Emulator") {
			recipe = "test_skqp_emulator"
		}
	} else if strings.Contains(name, "OpenCL") {
		// TODO(dogben): Longer term we may not want this to be called a "Test" task, but until we start
		// running hs_bench or kx, it will be easier to fit into the current job name schema.
		recipe = "compute_test"
	} else if strings.Contains(name, "PathKit") {
		recipe = "test_pathkit"
	} else if strings.Contains(name, "CanvasKit") {
		recipe = "test_canvaskit"
	} else if strings.Contains(name, "LottieWeb") {
		recipe = "test_lottie_web"
	}
	extraProps := map[string]string{
		"gold_hashes_url": CONFIG.GoldHashesURL,
	}
	iid := internalHardwareLabel(parts)
	if iid != nil {
		extraProps["internal_hardware_label"] = strconv.Itoa(*iid)
	}
	isolate := "test_skia_bundled.isolate"
	if strings.Contains(name, "CanvasKit") || strings.Contains(name, "Emulator") || strings.Contains(name, "LottieWeb") || strings.Contains(name, "PathKit") {
		isolate = "swarm_recipe.isolate"
	}
	task := kitchenTask(name, recipe, isolate, "", swarmDimensions(parts), extraProps, OUTPUT_TEST)
	task.CipdPackages = append(task.CipdPackages, pkgs...)
	if strings.Contains(name, "Lottie") {
		task.CipdPackages = append(task.CipdPackages, b.MustGetCipdPackageFromAsset("lottie-samples"))
	}
	if !strings.Contains(name, "LottieWeb") {
		// Test.+LottieWeb doesn't require anything in Skia to be compiled.
		task.Dependencies = append(task.Dependencies, compileTaskName)
	}

	if strings.Contains(name, "Android_ASAN") {
		task.Dependencies = append(task.Dependencies, isolateCIPDAsset(b, ISOLATE_NDK_LINUX_NAME))
	}
	if strings.Contains(name, "SKQP") {
		if !strings.Contains(name, "Emulator") {
			task.Dependencies = append(task.Dependencies, isolateCIPDAsset(b, ISOLATE_GCLOUD_LINUX_NAME))
		}
	}
	if deps := getIsolatedCIPDDeps(parts); len(deps) > 0 {
		task.Dependencies = append(task.Dependencies, deps...)
	}
	task.Expiration = 20 * time.Hour

	timeout(task, 4*time.Hour)
	if strings.Contains(parts["extra_config"], "Valgrind") {
		timeout(task, 9*time.Hour)
		task.Expiration = 48 * time.Hour
		task.CipdPackages = append(task.CipdPackages, b.MustGetCipdPackageFromAsset("valgrind"))
		// Since Valgrind runs on the same bots as the CQ, we restrict Valgrind to a subset of the bots
		// to ensure there are always bots free for CQ tasks.
		task.Dimensions = append(task.Dimensions, "valgrind:1")
	} else if strings.Contains(parts["extra_config"], "MSAN") {
		timeout(task, 9*time.Hour)
	} else if parts["arch"] == "x86" && parts["configuration"] == "Debug" {
		// skia:6737
		timeout(task, 6*time.Hour)
	}
	b.MustAddTask(name, task)

	// Upload results if necessary. TODO(kjlubick): If we do coverage analysis at the same
	// time as normal tests (which would be nice), cfg.json needs to have Coverage removed.
	if doUpload(name) {
		uploadName := fmt.Sprintf("%s%s%s", PREFIX_UPLOAD, jobNameSchema.Sep, name)
		extraProps := map[string]string{
			"gs_bucket": CONFIG.GsBucketGm,
		}
		uploadTask := kitchenTask(name, "upload_dm_results", "swarm_recipe.isolate", SERVICE_ACCOUNT_UPLOAD_GM, linuxGceDimensions(MACHINE_TYPE_SMALL), extraProps, OUTPUT_NONE)
		uploadTask.CipdPackages = append(uploadTask.CipdPackages, CIPD_PKGS_GSUTIL...)
		uploadTask.Dependencies = append(uploadTask.Dependencies, name)
		b.MustAddTask(uploadName, uploadTask)
		return uploadName
	}

	return name
}

// perf generates a Perf task. Returns the name of the last task in the
// generated chain of tasks, which the Job should add as a dependency.
func perf(b *specs.TasksCfgBuilder, name string, parts map[string]string, compileTaskName string, pkgs []*specs.CipdPackage) string {
	recipe := "perf"
	isolate := relpath("perf_skia_bundled.isolate")
	if strings.Contains(parts["extra_config"], "Skpbench") {
		recipe = "skpbench"
		isolate = relpath("skpbench_skia_bundled.isolate")
	} else if strings.Contains(name, "PathKit") {
		recipe = "perf_pathkit"
	} else if strings.Contains(name, "CanvasKit") {
		recipe = "perf_canvaskit"
	} else if strings.Contains(name, "SkottieTracing") {
		recipe = "perf_skottietrace"
	}
	task := kitchenTask(name, recipe, isolate, "", swarmDimensions(parts), nil, OUTPUT_PERF)
	task.CipdPackages = append(task.CipdPackages, pkgs...)
	task.Dependencies = append(task.Dependencies, compileTaskName)
	task.Expiration = 20 * time.Hour
	timeout(task, 4*time.Hour)
	if deps := getIsolatedCIPDDeps(parts); len(deps) > 0 {
		task.Dependencies = append(task.Dependencies, deps...)
	}

	if strings.Contains(parts["extra_config"], "Valgrind") {
		timeout(task, 9*time.Hour)
		task.Expiration = 48 * time.Hour
		task.CipdPackages = append(task.CipdPackages, b.MustGetCipdPackageFromAsset("valgrind"))
		// Since Valgrind runs on the same bots as the CQ, we restrict Valgrind to a subset of the bots
		// to ensure there are always bots free for CQ tasks.
		task.Dimensions = append(task.Dimensions, "valgrind:1")
	} else if strings.Contains(parts["extra_config"], "MSAN") {
		timeout(task, 9*time.Hour)
	} else if parts["arch"] == "x86" && parts["configuration"] == "Debug" {
		// skia:6737
		timeout(task, 6*time.Hour)
	} else if strings.Contains(parts["extra_config"], "Skottie") {
		task.CipdPackages = append(task.CipdPackages, b.MustGetCipdPackageFromAsset("lottie-samples"))
	}
	iid := internalHardwareLabel(parts)
	if iid != nil {
		task.Command = append(task.Command, fmt.Sprintf("internal_hardware_label=%d", *iid))
	}
	b.MustAddTask(name, task)

	// Upload results if necessary.
	if strings.Contains(name, "Release") && doUpload(name) {
		uploadName := fmt.Sprintf("%s%s%s", PREFIX_UPLOAD, jobNameSchema.Sep, name)
		extraProps := map[string]string{
			"gs_bucket": CONFIG.GsBucketNano,
		}
		uploadTask := kitchenTask(name, "upload_nano_results", "swarm_recipe.isolate", SERVICE_ACCOUNT_UPLOAD_NANO, linuxGceDimensions(MACHINE_TYPE_SMALL), extraProps, OUTPUT_NONE)
		uploadTask.CipdPackages = append(uploadTask.CipdPackages, CIPD_PKGS_GSUTIL...)
		uploadTask.Dependencies = append(uploadTask.Dependencies, name)
		b.MustAddTask(uploadName, uploadTask)
		return uploadName
	}
	return name
}

// Run the presubmit.
func presubmit(b *specs.TasksCfgBuilder, name string) string {
	extraProps := map[string]string{
		"category":         "cq",
		"patch_gerrit_url": "https://skia-review.googlesource.com",
		"patch_project":    "skia",
		"patch_ref":        specs.PLACEHOLDER_PATCH_REF,
		"reason":           "CQ",
		"repo_name":        "skia",
	}
	// Use MACHINE_TYPE_LARGE because it seems to save time versus MEDIUM and we want presubmit to be
	// fast.
	task := kitchenTask(name, "run_presubmit", "empty.isolate", SERVICE_ACCOUNT_COMPILE, linuxGceDimensions(MACHINE_TYPE_LARGE), extraProps, OUTPUT_NONE)
	usesGit(task, name)
	task.CipdPackages = append(task.CipdPackages, &specs.CipdPackage{
		Name:    "infra/recipe_bundles/chromium.googlesource.com/chromium/tools/build",
		Path:    "recipe_bundle",
		Version: "refs/heads/master",
	})
	task.Dependencies = []string{} // No bundled recipes for this one.
	b.MustAddTask(name, task)
	return name
}

// process generates tasks and jobs for the given job name.
func process(b *specs.TasksCfgBuilder, name string) {
	var priority float64 // Leave as default for most jobs.
	deps := []string{}

	// Bundle Recipes.
	if name == BUNDLE_RECIPES_NAME {
		deps = append(deps, bundleRecipes(b))
	}

	// Isolate CIPD assets.
	if _, ok := ISOLATE_ASSET_MAPPING[name]; ok {
		deps = append(deps, isolateCIPDAsset(b, name))
	}

	parts, err := jobNameSchema.ParseJobName(name)
	if err != nil {
		glog.Fatal(err)
	}

	// RecreateSKPs.
	if strings.Contains(name, "RecreateSKPs") {
		deps = append(deps, recreateSKPs(b, name))
	}

	// Update Go DEPS.
	if strings.Contains(name, "UpdateGoDEPS") {
		deps = append(deps, updateGoDEPS(b, name))
	}

	// Infra tests.
	if name == "Housekeeper-PerCommit-InfraTests" {
		deps = append(deps, infra(b, name))
	}

	// Compile bots.
	if parts["role"] == "Build" {
		if parts["extra_config"] == "Android_Framework" {
			// Android Framework compile tasks use a different recipe.
			deps = append(deps, androidFrameworkCompile(b, name))
		} else {
			deps = append(deps, compile(b, name, parts))
		}
	}

	// Most remaining bots need a compile task.
	compileTaskName := deriveCompileTaskName(name, parts)
	compileTaskParts, err := jobNameSchema.ParseJobName(compileTaskName)
	if err != nil {
		glog.Fatal(err)
	}
	compileParentName := getParentRevisionName(compileTaskName, compileTaskParts)
	compileParentParts, err := jobNameSchema.ParseJobName(compileParentName)
	if err != nil {
		glog.Fatal(err)
	}

	// These bots do not need a compile task.
	if parts["role"] != "Build" &&
		name != "Housekeeper-Nightly-UpdateGoDEPS" &&
		name != "Housekeeper-PerCommit-BundleRecipes" &&
		name != "Housekeeper-PerCommit-InfraTests" &&
		name != "Housekeeper-PerCommit-CheckGeneratedFiles" &&
		name != "Housekeeper-OnDemand-Presubmit" &&
		name != "Housekeeper-PerCommit" &&
		!strings.Contains(name, "Android_Framework") &&
		!strings.Contains(name, "RecreateSKPs") &&
		!strings.Contains(name, "Housekeeper-PerCommit-Isolate") &&
		!strings.Contains(name, "LottieWeb") {
		compile(b, compileTaskName, compileTaskParts)
		if parts["role"] == "Calmbench" {
			compile(b, compileParentName, compileParentParts)
		}
	}

	// Housekeepers.
	if name == "Housekeeper-PerCommit" {
		deps = append(deps, housekeeper(b, name))
	}
	if name == "Housekeeper-PerCommit-CheckGeneratedFiles" {
		deps = append(deps, checkGeneratedFiles(b, name))
	}
	if name == "Housekeeper-OnDemand-Presubmit" {
		priority = 1
		deps = append(deps, presubmit(b, name))
	}

	// Common assets needed by the remaining bots.

	pkgs := []*specs.CipdPackage{}

	if deps := getIsolatedCIPDDeps(parts); len(deps) == 0 {
		pkgs = []*specs.CipdPackage{
			b.MustGetCipdPackageFromAsset("skimage"),
			b.MustGetCipdPackageFromAsset("skp"),
			b.MustGetCipdPackageFromAsset("svg"),
		}
	}

	if strings.Contains(name, "Ubuntu") || strings.Contains(name, "Debian") {
		if strings.Contains(name, "SAN") {
			pkgs = append(pkgs, b.MustGetCipdPackageFromAsset("clang_linux"))
		}
		if strings.Contains(name, "Vulkan") {
			pkgs = append(pkgs, b.MustGetCipdPackageFromAsset("linux_vulkan_sdk"))
		}
		if strings.Contains(name, "Intel") && strings.Contains(name, "GPU") {
			pkgs = append(pkgs, b.MustGetCipdPackageFromAsset("mesa_intel_driver_linux"))
		}
		if strings.Contains(name, "OpenCL") {
			pkgs = append(pkgs,
				b.MustGetCipdPackageFromAsset("opencl_ocl_icd_linux"),
				b.MustGetCipdPackageFromAsset("opencl_intel_neo_linux"),
			)
		}
	}
	if strings.Contains(name, "ProcDump") {
		pkgs = append(pkgs, b.MustGetCipdPackageFromAsset("procdump_win"))
	}
	if strings.Contains(name, "CanvasKit") || strings.Contains(name, "LottieWeb") || strings.Contains(name, "PathKit") {
		// Docker-based tests that don't need the standard CIPD assets
		pkgs = []*specs.CipdPackage{}
	}

	// Test bots.
	if parts["role"] == "Test" {
		deps = append(deps, test(b, name, parts, compileTaskName, pkgs))
	}

	// Perf bots.
	if parts["role"] == "Perf" {
		deps = append(deps, perf(b, name, parts, compileTaskName, pkgs))
	}

	// Calmbench bots.
	if parts["role"] == "Calmbench" {
		deps = append(deps, calmbench(b, name, parts, compileTaskName, compileParentName))
	}

	// Valgrind runs at a low priority so that it doesn't occupy all the bots.
	if strings.Contains(name, "Valgrind") {
		// Priority of 0.085 should result in Valgrind tasks with a blamelist of ~10 commits having the
		// same score as other tasks with a blamelist of 1 commit, when we have insufficient bot
		// capacity to run more frequently.
		priority = 0.085
	}

	// BuildStats bots. This computes things like binary size.
	if parts["role"] == "BuildStats" {
		deps = append(deps, buildstats(b, name, parts, compileTaskName))
	}

	// Add the Job spec.
	j := &specs.JobSpec{
		Priority:  priority,
		TaskSpecs: deps,
		Trigger:   specs.TRIGGER_ANY_BRANCH,
	}
	if strings.Contains(name, "-Nightly-") {
		j.Trigger = specs.TRIGGER_NIGHTLY
	} else if strings.Contains(name, "-Weekly-") {
		j.Trigger = specs.TRIGGER_WEEKLY
	} else if strings.Contains(name, "Flutter") || strings.Contains(name, "CommandBuffer") {
		j.Trigger = specs.TRIGGER_MASTER_ONLY
	} else if strings.Contains(name, "-OnDemand-") || strings.Contains(name, "Android_Framework") {
		j.Trigger = specs.TRIGGER_ON_DEMAND
	}
	b.MustAddJob(name, j)
}

func loadJson(flag *string, defaultFlag string, val interface{}) {
	if *flag == "" {
		*flag = defaultFlag
	}
	b, err := ioutil.ReadFile(*flag)
	if err != nil {
		glog.Fatal(err)
	}
	if err := json.Unmarshal(b, val); err != nil {
		glog.Fatal(err)
	}
}

// Regenerate the tasks.json file.
func main() {
	b := specs.MustNewTasksCfgBuilder()
	b.SetAssetsDir(*assetsDir)
	infraBots := path.Join(b.CheckoutRoot(), "infra", "bots")

	// Load the jobs from a JSON file.
	loadJson(jobsFile, path.Join(infraBots, "jobs.json"), &JOBS)

	// Load general config information from a JSON file.
	loadJson(cfgFile, path.Join(infraBots, "cfg.json"), &CONFIG)

	// Create the JobNameSchema.
	if *builderNameSchemaFile == "" {
		*builderNameSchemaFile = path.Join(b.CheckoutRoot(), "infra", "bots", "recipe_modules", "builder_name_schema", "builder_name_schema.json")
	}
	schema, err := NewJobNameSchema(*builderNameSchemaFile)
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

// schema is a sub-struct of JobNameSchema.
type schema struct {
	Keys         []string `json:"keys"`
	OptionalKeys []string `json:"optional_keys"`
	RecurseRoles []string `json:"recurse_roles"`
}

// JobNameSchema is a struct used for (de)constructing Job names in a
// predictable format.
type JobNameSchema struct {
	Schema map[string]*schema `json:"builder_name_schema"`
	Sep    string             `json:"builder_name_sep"`
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
	popFront := func(items []string) (string, []string, error) {
		if len(items) == 0 {
			return "", nil, fmt.Errorf("Invalid job name: %s (not enough parts)", n)
		}
		return items[0], items[1:], nil
	}

	result := map[string]string{}

	var parse func(int, string, []string) ([]string, error)
	parse = func(depth int, role string, parts []string) ([]string, error) {
		s, ok := s.Schema[role]
		if !ok {
			return nil, fmt.Errorf("Invalid job name; %q is not a valid role.", role)
		}
		if depth == 0 {
			result["role"] = role
		} else {
			result[fmt.Sprintf("sub-role-%d", depth)] = role
		}
		var err error
		for _, key := range s.Keys {
			var value string
			value, parts, err = popFront(parts)
			if err != nil {
				return nil, err
			}
			result[key] = value
		}
		for _, subRole := range s.RecurseRoles {
			if len(parts) > 0 && parts[0] == subRole {
				parts, err = parse(depth+1, parts[0], parts[1:])
				if err != nil {
					return nil, err
				}
			}
		}
		for _, key := range s.OptionalKeys {
			if len(parts) > 0 {
				var value string
				value, parts, err = popFront(parts)
				if err != nil {
					return nil, err
				}
				result[key] = value
			}
		}
		if len(parts) > 0 {
			return nil, fmt.Errorf("Invalid job name: %s (too many parts)", n)
		}
		return parts, nil
	}

	split := strings.Split(n, s.Sep)
	if len(split) < 2 {
		return nil, fmt.Errorf("Invalid job name: %s (not enough parts)", n)
	}
	role := split[0]
	split = split[1:]
	_, err := parse(0, role, split)
	return result, err
}

// MakeJobName assembles the given parts of a Job name, according to the schema.
func (s *JobNameSchema) MakeJobName(parts map[string]string) (string, error) {
	rvParts := make([]string, 0, len(parts))

	var process func(int, map[string]string) (map[string]string, error)
	process = func(depth int, parts map[string]string) (map[string]string, error) {
		roleKey := "role"
		if depth != 0 {
			roleKey = fmt.Sprintf("sub-role-%d", depth)
		}
		role, ok := parts[roleKey]
		if !ok {
			return nil, fmt.Errorf("Invalid job parts; missing key %q", roleKey)
		}

		s, ok := s.Schema[role]
		if !ok {
			return nil, fmt.Errorf("Invalid job parts; unknown role %q", role)
		}
		rvParts = append(rvParts, role)
		delete(parts, roleKey)

		for _, key := range s.Keys {
			value, ok := parts[key]
			if !ok {
				return nil, fmt.Errorf("Invalid job parts; missing %q", key)
			}
			rvParts = append(rvParts, value)
			delete(parts, key)
		}

		if len(s.RecurseRoles) > 0 {
			subRoleKey := fmt.Sprintf("sub-role-%d", depth+1)
			subRole, ok := parts[subRoleKey]
			if !ok {
				return nil, fmt.Errorf("Invalid job parts; missing %q", subRoleKey)
			}
			rvParts = append(rvParts, subRole)
			delete(parts, subRoleKey)
			found := false
			for _, recurseRole := range s.RecurseRoles {
				if recurseRole == subRole {
					found = true
					var err error
					parts, err = process(depth+1, parts)
					if err != nil {
						return nil, err
					}
					break
				}
			}
			if !found {
				return nil, fmt.Errorf("Invalid job parts; unknown sub-role %q", subRole)
			}
		}
		for _, key := range s.OptionalKeys {
			if value, ok := parts[key]; ok {
				rvParts = append(rvParts, value)
				delete(parts, key)
			}
		}
		if len(parts) > 0 {
			return nil, fmt.Errorf("Invalid job parts: too many parts: %v", parts)
		}
		return parts, nil
	}

	// Copy the parts map, so that we can modify at will.
	partsCpy := make(map[string]string, len(parts))
	for k, v := range parts {
		partsCpy[k] = v
	}
	if _, err := process(0, partsCpy); err != nil {
		return "", err
	}
	return strings.Join(rvParts, s.Sep), nil
}
