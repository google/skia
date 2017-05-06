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
	"regexp"
	"sort"
	"strings"
	"time"

	"github.com/skia-dev/glog"
	"go.skia.org/infra/go/util"
	"go.skia.org/infra/task_scheduler/go/specs"
)

const (
	DEFAULT_OS       = DEFAULT_OS_LINUX
	DEFAULT_OS_LINUX = "Ubuntu-14.04"

	// Name prefix for upload jobs.
	PREFIX_UPLOAD = "Upload"
)

var (
	// "Constants"

	// Top-level list of all jobs to run at each commit; loaded from
	// jobs.json.
	JOBS []string

	// Mapping of human-friendly Android device names to a pair of {device_type, device_os}.
	ANDROID_MAPPING map[string][]string

	// General configuration information.
	CONFIG struct {
		GsBucketGm   string   `json:"gs_bucket_gm"`
		GsBucketNano string   `json:"gs_bucket_nano"`
		NoUpload     []string `json:"no_upload"`
		Pool         string   `json:"pool"`
	}

	// Mapping of human-friendly GPU names to PCI IDs.
	GPU_MAPPING map[string]string

	// Defines the structure of job names.
	jobNameSchema *JobNameSchema

	// Flags.
	androidMapFile        = flag.String("android_map", "", "JSON file containing a mapping of human-friendly Android device names to a pair of {device_type, device_os}.")
	builderNameSchemaFile = flag.String("builder_name_schema", "", "Path to the builder_name_schema.json file. If not specified, uses infra/bots/recipe_modules/builder_name_schema/builder_name_schema.json from this repo.")
	assetsDir             = flag.String("assets_dir", "", "Directory containing assets.")
	cfgFile               = flag.String("cfg_file", "", "JSON file containing general configuration information.")
	gpuMapFile            = flag.String("gpu_map", "", "JSON file containing a mapping of human-friendly GPU names to PCI IDs.")
	jobsFile              = flag.String("jobs", "", "JSON file containing jobs to run.")
)

// linuxGceDimensions are the Swarming dimensions for Linux GCE
// instances.
func linuxGceDimensions() []string {
	return []string{
		"cpu:x86-64-avx2",
		"gpu:none",
		fmt.Sprintf("os:%s", DEFAULT_OS_LINUX),
		fmt.Sprintf("pool:%s", CONFIG.Pool),
	}
}

// deriveCompileTaskName returns the name of a compile task based on the given
// job name.
func deriveCompileTaskName(jobName string, parts map[string]string) string {
	if parts["role"] == "Housekeeper" {
		return "Build-Ubuntu-GCC-x86_64-Release-Shared"
	} else if parts["role"] == "Test" || parts["role"] == "Perf" {
		task_os := parts["os"]
		ec := parts["extra_config"]
		ec = strings.TrimSuffix(ec, "_Skpbench")
		ec = strings.TrimSuffix(ec, "_AbandonGpuContext")
		ec = strings.TrimSuffix(ec, "_PreAbandonGpuContext")
		if ec == "Valgrind" {
			// skia:6267
			ec = ""
		}
		if task_os == "Android" {
			if ec == "Vulkan" {
				ec = "Android_Vulkan"
			}
			task_os = "Ubuntu"
		} else if task_os == "iOS" {
			ec = task_os
			task_os = "Mac"
		} else if strings.Contains(task_os, "Win") {
			task_os = "Win"
		} else if strings.Contains(task_os, "Ubuntu") {
			task_os = "Ubuntu"
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
		"pool": CONFIG.Pool,
	}
	if os, ok := parts["os"]; ok {
		d["os"] = map[string]string{
			"Android":  "Android",
			"Mac":      "Mac-10.11",
			"Ubuntu":   DEFAULT_OS_LINUX,
			"Ubuntu16": "Ubuntu-16.10",
			"Win":      "Windows-2008ServerR2-SP1",
			"Win10":    "Windows-10-14393",
			"Win2k8":   "Windows-2008ServerR2-SP1",
			"Win8":     "Windows-8.1-SP0",
			"iOS":      "iOS-9.3.1",
		}[os]
		// Chrome Golo has a different Windows image.
		if parts["model"] == "Golo" && os == "Win10" {
			d["os"] = "Windows-10-10586"
		}
	} else {
		d["os"] = DEFAULT_OS
	}
	if parts["role"] == "Test" || parts["role"] == "Perf" {
		if strings.Contains(parts["os"], "Android") {
			// For Android, the device type is a better dimension
			// than CPU or GPU.
			deviceInfo, ok := ANDROID_MAPPING[parts["model"]]
			if !ok {
				glog.Fatalf("Entry %q not found in Android mapping: %v", parts["model"], ANDROID_MAPPING)
			}
			d["device_type"] = deviceInfo[0]
			d["device_os"] = deviceInfo[1]
		} else if strings.Contains(parts["os"], "iOS") {
			d["device"] = map[string]string{
				"iPadMini4": "iPad5,1",
			}[parts["model"]]
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
			gpu, ok := GPU_MAPPING[parts["cpu_or_gpu_value"]]
			if !ok {
				glog.Fatalf("Entry %q not found in GPU mapping: %v", parts["cpu_or_gpu_value"], GPU_MAPPING)
			}
			d["gpu"] = gpu

			// Hack: Specify machine_type dimension for NUCs and ShuttleCs. We
			// temporarily have two types of machines with a GTX960. The only way to
			// distinguish these bots is by machine_type.
			machine_type, ok := map[string]string{
				"NUC6i7KYK": "n1-highcpu-8",
				"ShuttleC":  "n1-standard-8",
			}[parts["model"]]
			if ok {
				d["machine_type"] = machine_type
			}
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
	} else if strings.Contains(name, "Ubuntu") {
		if strings.Contains(name, "Clang") {
			pkgs = append(pkgs, b.MustGetCipdPackageFromAsset("clang_linux"))
		}
		if strings.Contains(name, "Vulkan") {
			pkgs = append(pkgs, b.MustGetCipdPackageFromAsset("linux_vulkan_sdk"))
		}
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
	// All compile tasks are runnable as their own Job. Assert that the Job
	// is listed in JOBS.
	if !util.In(name, JOBS) {
		glog.Fatalf("Job %q is missing from the JOBS list!", name)
	}
	return name
}

// recreateSKPs generates a RecreateSKPs task. Returns the name of the last
// task in the generated chain of tasks, which the Job should add as a
// dependency.
func recreateSKPs(b *specs.TasksCfgBuilder, name string) string {
	b.MustAddTask(name, &specs.TaskSpec{
		CipdPackages:     []*specs.CipdPackage{},
		Dimensions:       linuxGceDimensions(),
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
		CipdPackages: []*specs.CipdPackage{b.MustGetCipdPackageFromAsset("go")},
		Dependencies: []string{compileTaskName},
		Dimensions:   linuxGceDimensions(),
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
		Dimensions:   linuxGceDimensions(),
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
		IoTimeout:   40 * time.Minute,
		Isolate:     "test_skia.isolate",
		MaxAttempts: 1,
		Priority:    0.8,
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
			Dimensions:   linuxGceDimensions(),
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
				fmt.Sprintf("gs_bucket=%s", CONFIG.GsBucketGm),
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
		IoTimeout:   40 * time.Minute,
		Isolate:     isolate,
		MaxAttempts: 1,
		Priority:    0.8,
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
			Dimensions:   linuxGceDimensions(),
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
				fmt.Sprintf("gs_bucket=%s", CONFIG.GsBucketNano),
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
	if name == "Housekeeper-PerCommit" {
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
	if strings.Contains(name, "Ubuntu16") {
		if strings.Contains(name, "Vulkan") {
			pkgs = append(pkgs, b.MustGetCipdPackageFromAsset("linux_vulkan_sdk"))
		}
		if strings.Contains(name, "Release") {
			pkgs = append(pkgs, b.MustGetCipdPackageFromAsset("linux_vulkan_intel_driver_release"))
		} else {
			pkgs = append(pkgs, b.MustGetCipdPackageFromAsset("linux_vulkan_intel_driver_debug"))
		}
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

	// Load the GPU mapping from a JSON file.
	loadJson(gpuMapFile, path.Join(infraBots, "gpu_map.json"), &GPU_MAPPING)

	// Load the Android device mapping from a JSON file.
	loadJson(androidMapFile, path.Join(infraBots, "android_map.json"), &ANDROID_MAPPING)

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
