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
	"strings"
	"time"

	"github.com/skia-dev/glog"
	"go.skia.org/infra/go/sklog"
	"go.skia.org/infra/go/util"
	"go.skia.org/infra/task_scheduler/go/specs"
)

const (
	BUNDLE_RECIPES_NAME  = "Housekeeper-PerCommit-BundleRecipes"
	ISOLATE_SKIMAGE_NAME = "Housekeeper-PerCommit-IsolateSkImage"
	ISOLATE_SKP_NAME     = "Housekeeper-PerCommit-IsolateSKP"
	ISOLATE_SVG_NAME     = "Housekeeper-PerCommit-IsolateSVG"

	DEFAULT_OS_DEBIAN = "Debian-9.1"
	DEFAULT_OS_UBUNTU = "Ubuntu-14.04"

	// Name prefix for upload jobs.
	PREFIX_UPLOAD = "Upload"
)

var (
	// "Constants"

	// Top-level list of all jobs to run at each commit; loaded from
	// jobs.json.
	JOBS []string

	// General configuration information.
	CONFIG struct {
		GsBucketGm   string   `json:"gs_bucket_gm"`
		GsBucketNano string   `json:"gs_bucket_nano"`
		NoUpload     []string `json:"no_upload"`
		Pool         string   `json:"pool"`
	}

	// alternateSwarmDimensions can be set in an init function to override the default swarming bot
	// dimensions for the given task.
	alternateSwarmDimensions func(parts map[string]string) []string

	// Defines the structure of job names.
	jobNameSchema *JobNameSchema

	// Git 2.13.
	cipdGit1 = &specs.CipdPackage{
		Name:    fmt.Sprintf("infra/git/${platform}"),
		Path:    "git",
		Version: fmt.Sprintf("version:2.13.0.chromium9"),
	}
	cipdGit2 = &specs.CipdPackage{
		Name:    fmt.Sprintf("infra/tools/git/${platform}"),
		Path:    "git",
		Version: fmt.Sprintf("git_revision:a78b5f3658c0578a017db48df97d20ac09822bcd"),
	}

	// Flags.
	builderNameSchemaFile = flag.String("builder_name_schema", "", "Path to the builder_name_schema.json file. If not specified, uses infra/bots/recipe_modules/builder_name_schema/builder_name_schema.json from this repo.")
	assetsDir             = flag.String("assets_dir", "", "Directory containing assets.")
	cfgFile               = flag.String("cfg_file", "", "JSON file containing general configuration information.")
	jobsFile              = flag.String("jobs", "", "JSON file containing jobs to run.")
)

// linuxGceDimensions are the Swarming dimensions for Linux GCE
// instances.
func linuxGceDimensions() []string {
	return []string{
		"cpu:x86-64-avx2",
		"gpu:none",
		fmt.Sprintf("os:%s", DEFAULT_OS_DEBIAN),
		fmt.Sprintf("pool:%s", CONFIG.Pool),
	}
}

// deriveCompileTaskName returns the name of a compile task based on the given
// job name.
func deriveCompileTaskName(jobName string, parts map[string]string) string {
	if parts["role"] == "Housekeeper" {
		return "Build-Debian9-GCC-x86_64-Release-Shared"
	} else if parts["role"] == "Test" || parts["role"] == "Perf" {
		task_os := parts["os"]
		ec := []string{}
		if val := parts["extra_config"]; val != "" {
			ec = strings.Split(val, "_")
			ignore := []string{"Skpbench", "AbandonGpuContext", "PreAbandonGpuContext", "Valgrind", "ReleaseAndAbandonGpuContext", "CCPR"}
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
			ec = append([]string{"Chromebook", "ARM", "GLES"}, ec...)
			task_os = "Debian9"
		} else if task_os == "iOS" {
			ec = append([]string{task_os}, ec...)
			task_os = "Mac"
		} else if strings.Contains(task_os, "Win") {
			task_os = "Win"
		} else if strings.Contains(task_os, "Ubuntu") || strings.Contains(task_os, "Debian") {
			task_os = "Debian9"
		}
		jobNameMap := map[string]string{
			"role":          "Build",
			"os":            task_os,
			"compiler":      parts["compiler"],
			"target_arch":   parts["arch"],
			"configuration": parts["configuration"],
		}
		if len(ec) > 0 {
			jobNameMap["extra_config"] = strings.Join(ec, "_")
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
			"Mac":        "Mac-10.11",
			"Ubuntu14":   DEFAULT_OS_UBUNTU,
			"Ubuntu16":   "Ubuntu-16.10",
			"Ubuntu17":   "Ubuntu-17.04",
			"Win":        "Windows-2008ServerR2-SP1",
			"Win10":      "Windows-10-15063",
			"Win2k8":     "Windows-2008ServerR2-SP1",
			"Win7":       "Windows-7-SP1",
			"Win8":       "Windows-8.1-SP0",
			"iOS":        "iOS-10.3.1",
		}[os]
		if !ok {
			glog.Fatalf("Entry %q not found in OS mapping.", os)
		}
		// Chrome Golo has a different Windows image.
		if parts["model"] == "Golo" && os == "Win10" {
			d["os"] = "Windows-10-10586"
		}
	} else {
		d["os"] = DEFAULT_OS_DEBIAN
	}
	if parts["role"] == "Test" || parts["role"] == "Perf" {
		if strings.Contains(parts["os"], "Android") || strings.Contains(parts["os"], "Chromecast") {
			// For Android, the device type is a better dimension
			// than CPU or GPU.
			deviceInfo, ok := map[string][]string{
				"AndroidOne":      {"sprout", "MOB30Q"},
				"Chorizo":         {"chorizo", "1.24_82923"},
				"Ci20":            {"ci20", "NRD90M"},
				"GalaxyJ5":        {"j5xnlte", "MMB29M"},
				"GalaxyS6":        {"zerofltetmo", "MMB29K"},
				"GalaxyS7_G930A":  {"heroqlteatt", "NRD90M_G930AUCS4BQC2"},
				"GalaxyS7_G930FD": {"herolte", "NRD90M_G930FXXU1DQAS"},
				"GalaxyTab3":      {"goyawifi", "JDQ39"},
				"MotoG4":          {"athene", "NPJ25.93-14"},
				"NVIDIA_Shield":   {"foster", "NRD90M"},
				"Nexus10":         {"manta", "LMY49J"},
				"Nexus5":          {"hammerhead", "M4B30Z"},
				"Nexus6":          {"shamu", "M"},
				"Nexus6p":         {"angler", "OPP1.170223.012"},
				"Nexus7":          {"grouper", "LMY47V"},
				"Nexus7v2":        {"flo", "M"},
				"NexusPlayer":     {"fugu", "OPP2.170420.017"},
				"Pixel":           {"sailfish", "NMF26Q"},
				"PixelC":          {"dragon", "N2G47D"},
				"PixelXL":         {"marlin", "OPP3.170518.006"},
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
		} else if parts["cpu_or_gpu"] == "CPU" {
			d["gpu"] = "none"
			cpu, ok := map[string]string{
				"AVX":  "x86-64",
				"AVX2": "x86-64-avx2",
				"SSE4": "x86-64",
			}[parts["cpu_or_gpu_value"]]
			if !ok {
				glog.Fatalf("Entry %q not found in CPU mapping.", parts["cpu_or_gpu_value"])
			}
			d["cpu"] = cpu
			if strings.Contains(parts["os"], "Win") && parts["cpu_or_gpu_value"] == "AVX2" {
				// AVX2 is not correctly detected on Windows. Fall back on other
				// dimensions to ensure that we correctly target machines which we know
				// have AVX2 support.
				d["cpu"] = "x86-64"
				if parts["model"] != "GCE" {
					glog.Fatalf("Please double-check that %q supports AVX2 and update this assertion.", parts["model"])
				}
			}
		} else {
			if strings.Contains(parts["os"], "Win") {
				gpu, ok := map[string]string{
					"AMDHD7770":     "1002:683d-22.19.165.512",
					"GT610":         "10de:104a-21.21.13.7619",
					"GTX1070":       "10de:1ba1-22.21.13.8205",
					"GTX660":        "10de:11c0-22.21.13.8205",
					"GTX960":        "10de:1401-22.21.13.8205",
					"IntelHD530":    "8086:1912-21.20.16.4590",
					"IntelHD4400":   "8086:0a16-20.19.15.4624",
					"IntelHD4600":   "8086:0412-20.19.15.4624",
					"IntelIris540":  "8086:1926-21.20.16.4590",
					"IntelIris6100": "8086:162b-20.19.15.4624",
					"RadeonR9M470X": "1002:6646-22.19.165.512",
				}[parts["cpu_or_gpu_value"]]
				if !ok {
					glog.Fatalf("Entry %q not found in Win GPU mapping.", parts["cpu_or_gpu_value"])
				}
				d["gpu"] = gpu

				// Specify cpu dimension for NUCs and ShuttleCs. We temporarily have two
				// types of machines with a GTX960.
				cpu, ok := map[string]string{
					"NUC6i7KYK": "x86-64-i7-6770HQ",
					"ShuttleC":  "x86-64-i7-6700K",
				}[parts["model"]]
				if ok {
					d["cpu"] = cpu
				}
			} else if strings.Contains(parts["os"], "Ubuntu") || strings.Contains(parts["os"], "Debian") {
				gpu, ok := map[string]string{
					"GT610":    "10de:104a-340.102",
					"GTX550Ti": "10de:1244-375.66",
					"GTX660":   "10de:11c0-375.66",
					"GTX960":   "10de:1401-375.66",
					// Intel drivers come from CIPD, so no need to specify the version here.
					"IntelBayTrail": "8086:0f31",
					"IntelHD2000":   "8086:0102",
					"IntelHD405":    "8086:22b1",
					"IntelIris540":  "8086:1926",
				}[parts["cpu_or_gpu_value"]]
				if !ok {
					glog.Fatalf("Entry %q not found in Ubuntu GPU mapping.", parts["cpu_or_gpu_value"])
				}
				d["gpu"] = gpu
			} else if strings.Contains(parts["os"], "Mac") {
				gpu, ok := map[string]string{
					// TODO(benjaminwagner): GPU name doesn't match device ID.
					"IntelHD4000": "8086:0a2e",
				}[parts["cpu_or_gpu_value"]]
				if !ok {
					glog.Fatalf("Entry %q not found in Mac GPU mapping.", parts["cpu_or_gpu_value"])
				}
				d["gpu"] = gpu
			} else if strings.Contains(parts["os"], "ChromeOS") {
				gpu, ok := map[string]string{
					"MaliT604": "MaliT604",
					"MaliT764": "MaliT764",
					"MaliT860": "MaliT860",
					"TegraK1":  "TegraK1",
				}[parts["cpu_or_gpu_value"]]
				if !ok {
					glog.Fatalf("Entry %q not found in ChromeOS GPU mapping.", parts["cpu_or_gpu_value"])
				}
				d["gpu"] = gpu
			} else {
				glog.Fatalf("Unknown GPU mapping for OS %q.", parts["os"])
			}
		}
	} else {
		d["gpu"] = "none"
		if d["os"] == DEFAULT_OS_DEBIAN {
			return linuxGceDimensions()
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
	b.MustAddTask(BUNDLE_RECIPES_NAME, &specs.TaskSpec{
		CipdPackages: []*specs.CipdPackage{cipdGit1, cipdGit2},
		Dimensions:   linuxGceDimensions(),
		ExtraArgs: []string{
			"--workdir", "../../..", "bundle_recipes",
			fmt.Sprintf("buildername=%s", BUNDLE_RECIPES_NAME),
			fmt.Sprintf("swarm_out_dir=%s", specs.PLACEHOLDER_ISOLATED_OUTDIR),
		},
		Isolate:  relpath("bundle_recipes.isolate"),
		Priority: 0.7,
	})
	return BUNDLE_RECIPES_NAME
}

// useBundledRecipes returns true iff the given bot should use bundled recipes
// instead of syncing recipe DEPS itself.
func useBundledRecipes(parts map[string]string) bool {
	// Use bundled recipes for all test/perf tasks.
	return true
}

type isolateAssetCfg struct {
	isolateFile string
	cipdPkg     string
}

var ISOLATE_ASSET_MAPPING = map[string]isolateAssetCfg{
	ISOLATE_SKIMAGE_NAME: {
		isolateFile: "isolate_skimage.isolate",
		cipdPkg:     "skimage",
	},
	ISOLATE_SKP_NAME: {
		isolateFile: "isolate_skp.isolate",
		cipdPkg:     "skp",
	},
	ISOLATE_SVG_NAME: {
		isolateFile: "isolate_svg.isolate",
		cipdPkg:     "svg",
	},
}

// bundleRecipes generates the task to bundle and isolate the recipes.
func isolateCIPDAsset(b *specs.TasksCfgBuilder, name string) string {
	b.MustAddTask(name, &specs.TaskSpec{
		CipdPackages: []*specs.CipdPackage{
			b.MustGetCipdPackageFromAsset(ISOLATE_ASSET_MAPPING[name].cipdPkg),
		},
		Dimensions: linuxGceDimensions(),
		Isolate:    relpath(ISOLATE_ASSET_MAPPING[name].isolateFile),
		Priority:   0.7,
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
	} else if strings.Contains(name, "Chromecast") {
		pkgs = append(pkgs, b.MustGetCipdPackageFromAsset("cast_toolchain"))
		pkgs = append(pkgs, b.MustGetCipdPackageFromAsset("chromebook_arm_gles"))
	} else if strings.Contains(name, "Chromebook") {
		pkgs = append(pkgs, b.MustGetCipdPackageFromAsset("clang_linux"))
		pkgs = append(pkgs, b.MustGetCipdPackageFromAsset("armhf_sysroot"))
		pkgs = append(pkgs, b.MustGetCipdPackageFromAsset("chromebook_arm_gles"))
	} else if strings.Contains(name, "Debian") {
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

	// TODO(stephana): Remove this once all Mac machines are on the same
	// OS version again. Move the call to swarmDimensions back to the
	// creation of the TaskSpec struct below.
	dimensions := swarmDimensions(parts)
	if strings.Contains(name, "Mac") {
		for idx, dim := range dimensions {
			if strings.HasPrefix(dim, "os") {
				dimensions[idx] = "os:Mac-10.12"
				break
			}
		}
	}

	// Add the task.
	b.MustAddTask(name, &specs.TaskSpec{
		CipdPackages: pkgs,
		Dimensions:   dimensions,
		ExtraArgs: []string{
			"--workdir", "../../..", "compile",
			fmt.Sprintf("repository=%s", specs.PLACEHOLDER_REPO),
			fmt.Sprintf("buildername=%s", name),
			fmt.Sprintf("swarm_out_dir=%s", specs.PLACEHOLDER_ISOLATED_OUTDIR),
			fmt.Sprintf("revision=%s", specs.PLACEHOLDER_REVISION),
			fmt.Sprintf("patch_repo=%s", specs.PLACEHOLDER_PATCH_REPO),
			fmt.Sprintf("patch_storage=%s", specs.PLACEHOLDER_PATCH_STORAGE),
			fmt.Sprintf("patch_issue=%s", specs.PLACEHOLDER_ISSUE),
			fmt.Sprintf("patch_set=%s", specs.PLACEHOLDER_PATCHSET),
		},
		Isolate:  relpath("compile_skia.isolate"),
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
		CipdPackages:     []*specs.CipdPackage{b.MustGetCipdPackageFromAsset("go")},
		Dimensions:       linuxGceDimensions(),
		ExecutionTimeout: 4 * time.Hour,
		ExtraArgs: []string{
			"--workdir", "../../..", "recreate_skps",
			fmt.Sprintf("repository=%s", specs.PLACEHOLDER_REPO),
			fmt.Sprintf("buildername=%s", name),
			fmt.Sprintf("swarm_out_dir=%s", specs.PLACEHOLDER_ISOLATED_OUTDIR),
			fmt.Sprintf("revision=%s", specs.PLACEHOLDER_REVISION),
			fmt.Sprintf("patch_repo=%s", specs.PLACEHOLDER_PATCH_REPO),
			fmt.Sprintf("patch_storage=%s", specs.PLACEHOLDER_PATCH_STORAGE),
			fmt.Sprintf("patch_issue=%s", specs.PLACEHOLDER_ISSUE),
			fmt.Sprintf("patch_set=%s", specs.PLACEHOLDER_PATCHSET),
		},
		IoTimeout: 40 * time.Minute,
		Isolate:   relpath("compile_skia.isolate"),
		Priority:  0.8,
	})
	return name
}

// updateMetaConfig generates a UpdateMetaConfig task. Returns the name of the
// last task in the generated chain of tasks, which the Job should add as a
// dependency.
func updateMetaConfig(b *specs.TasksCfgBuilder, name string) string {
	b.MustAddTask(name, &specs.TaskSpec{
		CipdPackages: []*specs.CipdPackage{},
		Dimensions:   linuxGceDimensions(),
		ExtraArgs: []string{
			"--workdir", "../../..", "update_meta_config",
			fmt.Sprintf("repository=%s", specs.PLACEHOLDER_REPO),
			fmt.Sprintf("buildername=%s", name),
			fmt.Sprintf("swarm_out_dir=%s", specs.PLACEHOLDER_ISOLATED_OUTDIR),
			fmt.Sprintf("revision=%s", specs.PLACEHOLDER_REVISION),
			fmt.Sprintf("patch_repo=%s", specs.PLACEHOLDER_PATCH_REPO),
			fmt.Sprintf("patch_storage=%s", specs.PLACEHOLDER_PATCH_STORAGE),
			fmt.Sprintf("patch_issue=%s", specs.PLACEHOLDER_ISSUE),
			fmt.Sprintf("patch_set=%s", specs.PLACEHOLDER_PATCHSET),
		},
		Isolate:  relpath("meta_config.isolate"),
		Priority: 0.8,
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
			"--workdir", "../../..", "ct_skps",
			fmt.Sprintf("repository=%s", specs.PLACEHOLDER_REPO),
			fmt.Sprintf("buildername=%s", name),
			fmt.Sprintf("swarm_out_dir=%s", specs.PLACEHOLDER_ISOLATED_OUTDIR),
			fmt.Sprintf("revision=%s", specs.PLACEHOLDER_REVISION),
			fmt.Sprintf("patch_repo=%s", specs.PLACEHOLDER_PATCH_REPO),
			fmt.Sprintf("patch_storage=%s", specs.PLACEHOLDER_PATCH_STORAGE),
			fmt.Sprintf("patch_issue=%s", specs.PLACEHOLDER_ISSUE),
			fmt.Sprintf("patch_set=%s", specs.PLACEHOLDER_PATCHSET),
		},
		IoTimeout: time.Hour,
		Isolate:   relpath("ct_skps_skia.isolate"),
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
			"--workdir", "../../..", "housekeeper",
			fmt.Sprintf("repository=%s", specs.PLACEHOLDER_REPO),
			fmt.Sprintf("buildername=%s", name),
			fmt.Sprintf("swarm_out_dir=%s", specs.PLACEHOLDER_ISOLATED_OUTDIR),
			fmt.Sprintf("revision=%s", specs.PLACEHOLDER_REVISION),
			fmt.Sprintf("patch_repo=%s", specs.PLACEHOLDER_PATCH_REPO),
			fmt.Sprintf("patch_storage=%s", specs.PLACEHOLDER_PATCH_STORAGE),
			fmt.Sprintf("patch_issue=%s", specs.PLACEHOLDER_ISSUE),
			fmt.Sprintf("patch_set=%s", specs.PLACEHOLDER_PATCHSET),
		},
		Isolate:  relpath("housekeeper_skia.isolate"),
		Priority: 0.8,
	})
	return name
}

// infra generates an infra_tests task. Returns the name of the last task in the
// generated chain of tasks, which the Job should add as a dependency.
func infra(b *specs.TasksCfgBuilder, name string) string {
	b.MustAddTask(name, &specs.TaskSpec{
		CipdPackages: []*specs.CipdPackage{b.MustGetCipdPackageFromAsset("go")},
		Dimensions:   linuxGceDimensions(),
		ExtraArgs: []string{
			"--workdir", "../../..", "infra",
			fmt.Sprintf("repository=%s", specs.PLACEHOLDER_REPO),
			fmt.Sprintf("buildername=%s", name),
			fmt.Sprintf("swarm_out_dir=%s", specs.PLACEHOLDER_ISOLATED_OUTDIR),
			fmt.Sprintf("revision=%s", specs.PLACEHOLDER_REVISION),
			fmt.Sprintf("patch_repo=%s", specs.PLACEHOLDER_PATCH_REPO),
			fmt.Sprintf("patch_storage=%s", specs.PLACEHOLDER_PATCH_STORAGE),
			fmt.Sprintf("patch_issue=%s", specs.PLACEHOLDER_ISSUE),
			fmt.Sprintf("patch_set=%s", specs.PLACEHOLDER_PATCHSET),
		},
		Isolate:  relpath("infra_skia.isolate"),
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
			"--workdir", "../../..", "test",
			fmt.Sprintf("repository=%s", specs.PLACEHOLDER_REPO),
			fmt.Sprintf("buildername=%s", name),
			fmt.Sprintf("swarm_out_dir=%s", specs.PLACEHOLDER_ISOLATED_OUTDIR),
			fmt.Sprintf("revision=%s", specs.PLACEHOLDER_REVISION),
			fmt.Sprintf("patch_repo=%s", specs.PLACEHOLDER_PATCH_REPO),
			fmt.Sprintf("patch_storage=%s", specs.PLACEHOLDER_PATCH_STORAGE),
			fmt.Sprintf("patch_issue=%s", specs.PLACEHOLDER_ISSUE),
			fmt.Sprintf("patch_set=%s", specs.PLACEHOLDER_PATCHSET),
		},
		IoTimeout:   40 * time.Minute,
		Isolate:     relpath("test_skia.isolate"),
		MaxAttempts: 1,
		Priority:    0.8,
	}
	if useBundledRecipes(parts) {
		s.Dependencies = append(s.Dependencies, BUNDLE_RECIPES_NAME)
		if strings.Contains(parts["os"], "Win") {
			s.Isolate = relpath("test_skia_bundled_win.isolate")
		} else {
			s.Isolate = relpath("test_skia_bundled_unix.isolate")
		}
	}
	if deps := getIsolatedCIPDDeps(parts); len(deps) > 0 {
		s.Dependencies = append(s.Dependencies, deps...)
	}
	if strings.Contains(parts["extra_config"], "Valgrind") {
		s.ExecutionTimeout = 9 * time.Hour
		s.Expiration = 48 * time.Hour
		s.IoTimeout = time.Hour
		s.CipdPackages = append(s.CipdPackages, b.MustGetCipdPackageFromAsset("valgrind"))
	} else if strings.Contains(parts["extra_config"], "MSAN") {
		s.ExecutionTimeout = 9 * time.Hour
	} else if parts["arch"] == "x86" && parts["configuration"] == "Debug" {
		// skia:6737
		s.ExecutionTimeout = 6 * time.Hour
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
				fmt.Sprintf("swarm_out_dir=%s", specs.PLACEHOLDER_ISOLATED_OUTDIR),
				fmt.Sprintf("revision=%s", specs.PLACEHOLDER_REVISION),
				fmt.Sprintf("patch_repo=%s", specs.PLACEHOLDER_PATCH_REPO),
				fmt.Sprintf("patch_storage=%s", specs.PLACEHOLDER_PATCH_STORAGE),
				fmt.Sprintf("patch_issue=%s", specs.PLACEHOLDER_ISSUE),
				fmt.Sprintf("patch_set=%s", specs.PLACEHOLDER_PATCHSET),
				fmt.Sprintf("gs_bucket=%s", CONFIG.GsBucketGm),
			},
			Isolate:  relpath("upload_dm_results.isolate"),
			Priority: 0.8,
		})
		return uploadName
	}
	return name
}

// perf generates a Perf task. Returns the name of the last task in the
// generated chain of tasks, which the Job should add as a dependency.
func perf(b *specs.TasksCfgBuilder, name string, parts map[string]string, compileTaskName string, pkgs []*specs.CipdPackage) string {
	recipe := "perf"
	isolate := relpath("perf_skia.isolate")
	if strings.Contains(parts["extra_config"], "Skpbench") {
		recipe = "skpbench"
		isolate = relpath("skpbench_skia.isolate")
		if useBundledRecipes(parts) {
			if strings.Contains(parts["os"], "Win") {
				isolate = relpath("skpbench_skia_bundled_win.isolate")
			} else {
				isolate = relpath("skpbench_skia_bundled_unix.isolate")
			}
		}
	} else if useBundledRecipes(parts) {
		if strings.Contains(parts["os"], "Win") {
			isolate = relpath("perf_skia_bundled_win.isolate")
		} else {
			isolate = relpath("perf_skia_bundled_unix.isolate")
		}
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
			fmt.Sprintf("swarm_out_dir=%s", specs.PLACEHOLDER_ISOLATED_OUTDIR),
			fmt.Sprintf("revision=%s", specs.PLACEHOLDER_REVISION),
			fmt.Sprintf("patch_repo=%s", specs.PLACEHOLDER_PATCH_REPO),
			fmt.Sprintf("patch_storage=%s", specs.PLACEHOLDER_PATCH_STORAGE),
			fmt.Sprintf("patch_issue=%s", specs.PLACEHOLDER_ISSUE),
			fmt.Sprintf("patch_set=%s", specs.PLACEHOLDER_PATCHSET),
		},
		IoTimeout:   40 * time.Minute,
		Isolate:     isolate,
		MaxAttempts: 1,
		Priority:    0.8,
	}
	if useBundledRecipes(parts) {
		s.Dependencies = append(s.Dependencies, BUNDLE_RECIPES_NAME)
	}
	if deps := getIsolatedCIPDDeps(parts); len(deps) > 0 {
		s.Dependencies = append(s.Dependencies, deps...)
	}

	if strings.Contains(parts["extra_config"], "Valgrind") {
		s.ExecutionTimeout = 9 * time.Hour
		s.Expiration = 48 * time.Hour
		s.IoTimeout = time.Hour
		s.CipdPackages = append(s.CipdPackages, b.MustGetCipdPackageFromAsset("valgrind"))
	} else if strings.Contains(parts["extra_config"], "MSAN") {
		s.ExecutionTimeout = 9 * time.Hour
	} else if parts["arch"] == "x86" && parts["configuration"] == "Debug" {
		// skia:6737
		s.ExecutionTimeout = 6 * time.Hour
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
				fmt.Sprintf("swarm_out_dir=%s", specs.PLACEHOLDER_ISOLATED_OUTDIR),
				fmt.Sprintf("revision=%s", specs.PLACEHOLDER_REVISION),
				fmt.Sprintf("patch_repo=%s", specs.PLACEHOLDER_PATCH_REPO),
				fmt.Sprintf("patch_storage=%s", specs.PLACEHOLDER_PATCH_STORAGE),
				fmt.Sprintf("patch_issue=%s", specs.PLACEHOLDER_ISSUE),
				fmt.Sprintf("patch_set=%s", specs.PLACEHOLDER_PATCHSET),
				fmt.Sprintf("gs_bucket=%s", CONFIG.GsBucketNano),
			},
			Isolate:  relpath("upload_nano_results.isolate"),
			Priority: 0.8,
		})
		return uploadName
	}
	return name
}

// process generates tasks and jobs for the given job name.
func process(b *specs.TasksCfgBuilder, name string) {
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

	// UpdateMetaConfig bot.
	if strings.Contains(name, "UpdateMetaConfig") {
		deps = append(deps, updateMetaConfig(b, name))
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
		name != "Housekeeper-PerCommit-BundleRecipes" &&
		name != "Housekeeper-PerCommit-InfraTests" &&
		!strings.Contains(name, "RecreateSKPs") &&
		!strings.Contains(name, "UpdateMetaConfig") &&
		!strings.Contains(name, "-CT_") &&
		!strings.Contains(name, "Housekeeper-PerCommit-Isolate") {
		compile(b, compileTaskName, compileTaskParts)
	}

	// Housekeeper.
	if name == "Housekeeper-PerCommit" {
		deps = append(deps, housekeeper(b, name, compileTaskName))
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

	if (strings.Contains(name, "Ubuntu") || strings.Contains(name, "Debian")) && strings.Contains(name, "SAN") {
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
	if name == "Housekeeper-Nightly-UpdateMetaConfig" {
		j.Trigger = "nightly"
	}
	if name == "Housekeeper-Weekly-RecreateSKPs" {
		j.Trigger = "weekly"
	}
	if name == "Test-Ubuntu14-GCC-GCE-CPU-AVX2-x86_64-Debug-CT_DM_1m_SKPs" {
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
