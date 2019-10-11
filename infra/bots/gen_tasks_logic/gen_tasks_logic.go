// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package gen_tasks_logic

/*
	Generate the tasks.json file.
*/

import (
	"encoding/json"
	"fmt"
	"io/ioutil"
	"os"
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
	BUILD_TASK_DRIVERS_NAME    = "Housekeeper-PerCommit-BuildTaskDrivers"
	BUNDLE_RECIPES_NAME        = "Housekeeper-PerCommit-BundleRecipes"
	ISOLATE_GCLOUD_LINUX_NAME  = "Housekeeper-PerCommit-IsolateGCloudLinux"
	ISOLATE_SKIMAGE_NAME       = "Housekeeper-PerCommit-IsolateSkImage"
	ISOLATE_SKP_NAME           = "Housekeeper-PerCommit-IsolateSKP"
	ISOLATE_MSKP_NAME          = "Housekeeper-PerCommit-IsolateMSKP"
	ISOLATE_SVG_NAME           = "Housekeeper-PerCommit-IsolateSVG"
	ISOLATE_NDK_LINUX_NAME     = "Housekeeper-PerCommit-IsolateAndroidNDKLinux"
	ISOLATE_SDK_LINUX_NAME     = "Housekeeper-PerCommit-IsolateAndroidSDKLinux"
	ISOLATE_WIN_TOOLCHAIN_NAME = "Housekeeper-PerCommit-IsolateWinToolchain"

	DEFAULT_OS_DEBIAN    = "Debian-9.4"
	DEFAULT_OS_LINUX_GCE = "Debian-9.8"
	DEFAULT_OS_MAC       = "Mac-10.14.6"
	DEFAULT_OS_WIN       = "Windows-Server-14393"

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
)

var (
	// "Constants"

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
		&specs.Cache{
			Name: "gopath",
			Path: "cache/gopath",
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
			Version: "git_revision:f96db4b66034c859090be3c47eb38227277f228b",
		},
	}

	CIPD_PKGS_CPYTHON = []*specs.CipdPackage{
		&specs.CipdPackage{
			Name:    "infra/python/cpython/${platform}",
			Path:    "cipd_bin_packages",
			Version: "version:2.7.15.chromium14",
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

	// These properties are required by some tasks, eg. for running
	// bot_update, but they prevent de-duplication, so they should only be
	// used where necessary.
	EXTRA_PROPS = map[string]string{
		"buildbucket_build_id": specs.PLACEHOLDER_BUILDBUCKET_BUILD_ID,
		"patch_issue":          specs.PLACEHOLDER_ISSUE_INT,
		"patch_ref":            specs.PLACEHOLDER_PATCH_REF,
		"patch_repo":           specs.PLACEHOLDER_PATCH_REPO,
		"patch_set":            specs.PLACEHOLDER_PATCHSET_INT,
		"patch_storage":        specs.PLACEHOLDER_PATCH_STORAGE,
		"repository":           specs.PLACEHOLDER_REPO,
		"revision":             specs.PLACEHOLDER_REVISION,
		"task_id":              specs.PLACEHOLDER_TASK_ID,
	}

	// ISOLATE_ASSET_MAPPING maps the name of a task to the configuration
	// for how the CIPD package should be installed for that task, in order
	// for it to be uploaded to the isolate server.
	ISOLATE_ASSET_MAPPING = map[string]isolateAssetCfg{
		ISOLATE_GCLOUD_LINUX_NAME: {
			cipdPkg: "gcloud_linux",
			path:    "gcloud_linux",
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
		ISOLATE_MSKP_NAME: {
			cipdPkg: "mskp",
			path:    "mskp",
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

	// BUILD_STATS_NO_UPLOAD indicates which BuildStats tasks should not
	// have their results uploaded.
	BUILD_STATS_NO_UPLOAD = []string{"BuildStats-Debian9-Clang-x86_64-Release"}
)

// Config contains general configuration information.
type Config struct {
	// Directory containing assets. Assumed to be relative to the directory
	// which contains the calling gen_tasks.go file. If not specified, uses
	// the infra/bots/assets from this repo.
	AssetsDir string `json:"assets_dir"`

	// Path to the builder name schema JSON file. Assumed to be relative to
	// the directory which contains the calling gen_tasks.go file. If not
	// specified, uses infra/bots/recipe_modules/builder_name_schema/builder_name_schema.json
	// from this repo.
	BuilderNameSchemaFile string `json:"builder_name_schema"`

	// URL of the Skia Gold known hashes endpoint.
	GoldHashesURL string `json:"gold_hashes_url"`

	// GCS bucket used for Calmbench results.
	GsBucketCalm string `json:"gs_bucket_calm"`

	// GCS bucket used for GM results.
	GsBucketGm string `json:"gs_bucket_gm"`

	// GCS bucket used for Nanobench results.
	GsBucketNano string `json:"gs_bucket_nano"`

	// Optional function which returns a bot ID for internal devices.
	InternalHardwareLabel func(parts map[string]string) *int `json:"-"`

	// List of task names for which we'll never upload results.
	NoUpload []string `json:"no_upload"`

	// Swarming pool used for triggering tasks.
	Pool string `json:"pool"`

	// LUCI project associated with this repo.
	Project string `json:"project"`

	// Service accounts.
	ServiceAccountCompile         string `json:"service_account_compile"`
	ServiceAccountHousekeeper     string `json:"service_account_housekeeper"`
	ServiceAccountRecreateSKPs    string `json:"service_account_recreate_skps"`
	ServiceAccountUploadBinary    string `json:"service_account_upload_binary"`
	ServiceAccountUploadCalmbench string `json:"service_account_upload_calmbench"`
	ServiceAccountUploadGM        string `json:"service_account_upload_gm"`
	ServiceAccountUploadNano      string `json:"service_account_upload_nano"`

	// Optional override function which derives Swarming bot dimensions
	// from parts of task names.
	SwarmDimensions func(parts map[string]string) []string `json:"-"`
}

// LoadConfig loads the Config from a cfg.json file which is the sibling of the
// calling gen_tasks.go file.
func LoadConfig() *Config {
	cfgDir := getCallingDirName()
	var cfg Config
	LoadJson(filepath.Join(cfgDir, "cfg.json"), &cfg)
	return &cfg
}

// CheckoutRoot is a wrapper around specs.GetCheckoutRoot which prevents the
// caller from needing a dependency on the specs package.
func CheckoutRoot() string {
	root, err := specs.GetCheckoutRoot()
	if err != nil {
		glog.Fatal(err)
	}
	return root
}

// LoadJson loads JSON from the given file and unmarshals it into the given
// destination.
func LoadJson(filename string, dest interface{}) {
	b, err := ioutil.ReadFile(filename)
	if err != nil {
		glog.Fatalf("Unable to read %q: %s", filename, err)
	}
	if err := json.Unmarshal(b, dest); err != nil {
		glog.Fatalf("Unable to parse %q: %s", filename, err)
	}
}

// GenTasks regenerates the tasks.json file. Loads the job list from a jobs.json
// file which is the sibling of the calling gen_tasks.go file. If cfg is nil, it
// is similarly loaded from a cfg.json file which is the sibling of the calling
// gen_tasks.go file.
func GenTasks(cfg *Config) {
	b := specs.MustNewTasksCfgBuilder()

	// Find the paths to the infra/bots directories in this repo and the
	// repo of the calling file.
	relpathTargetDir := getThisDirName()
	relpathBaseDir := getCallingDirName()

	var jobs []string
	LoadJson(filepath.Join(relpathBaseDir, "jobs.json"), &jobs)

	if cfg == nil {
		cfg = new(Config)
		LoadJson(filepath.Join(relpathBaseDir, "cfg.json"), cfg)
	}

	// Create the JobNameSchema.
	builderNameSchemaFile := filepath.Join(relpathTargetDir, "recipe_modules", "builder_name_schema", "builder_name_schema.json")
	if cfg.BuilderNameSchemaFile != "" {
		builderNameSchemaFile = filepath.Join(relpathBaseDir, cfg.BuilderNameSchemaFile)
	}
	schema, err := NewJobNameSchema(builderNameSchemaFile)
	if err != nil {
		glog.Fatal(err)
	}

	// Set the assets dir.
	assetsDir := filepath.Join(relpathTargetDir, "assets")
	if cfg.AssetsDir != "" {
		assetsDir = filepath.Join(relpathBaseDir, cfg.AssetsDir)
	}
	b.SetAssetsDir(assetsDir)

	// Create Tasks and Jobs.
	builder := &builder{
		TasksCfgBuilder:  b,
		cfg:              cfg,
		jobNameSchema:    schema,
		jobs:             jobs,
		relpathBaseDir:   relpathBaseDir,
		relpathTargetDir: relpathTargetDir,
	}
	for _, name := range jobs {
		builder.process(name)
	}
	builder.MustFinish()
}

// getThisDirName returns the infra/bots directory which is an ancestor of this
// file.
func getThisDirName() string {
	_, thisFileName, _, ok := runtime.Caller(0)
	if !ok {
		sklog.Fatal("Unable to find path to current file.")
	}
	return filepath.Dir(filepath.Dir(thisFileName))
}

// getCallingDirName returns the infra/bots directory which is an ancestor of
// the calling gen_tasks.go file. WARNING: assumes that the calling gen_tasks.go
// file appears two steps up the stack; do not call from a function which is not
// directly called by gen_tasks.go.
func getCallingDirName() string {
	_, callingFileName, _, ok := runtime.Caller(2)
	if !ok {
		sklog.Fatal("Unable to find path to calling file.")
	}
	return filepath.Dir(callingFileName)
}

// builder is a wrapper for specs.TasksCfgBuilder.
type builder struct {
	*specs.TasksCfgBuilder
	cfg              *Config
	jobNameSchema    *JobNameSchema
	jobs             []string
	relpathBaseDir   string
	relpathTargetDir string
}

// logdogAnnotationUrl builds the LogDog annotation URL.
func (b *builder) logdogAnnotationUrl() string {
	return fmt.Sprintf("logdog://logs.chromium.org/%s/${SWARMING_TASK_ID}/+/annotations", b.cfg.Project)
}

// props creates a properties JSON string.
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
func (b *builder) kitchenTask(name, recipe, isolate, serviceAccount string, dimensions []string, extraProps map[string]string, outputDir string) *specs.TaskSpec {
	cipd := append([]*specs.CipdPackage{}, CIPD_PKGS_KITCHEN...)
	if strings.Contains(name, "Win") && !strings.Contains(name, "LenovoYogaC630") {
		cipd = append(cipd, CIPD_PKGS_CPYTHON...)
	} else if strings.Contains(name, "P30") {
		cipd = append(cipd, CIPD_PKGS_CPYTHON...)
	}
	properties := map[string]string{
		"buildername":   name,
		"swarm_out_dir": outputDir,
	}
	for k, v := range extraProps {
		properties[k] = v
	}
	var outputs []string = nil
	if outputDir != OUTPUT_NONE {
		outputs = []string{outputDir}
	}
	python := "cipd_bin_packages/vpython${EXECUTABLE_SUFFIX}"
	task := &specs.TaskSpec{
		Caches: []*specs.Cache{
			&specs.Cache{
				Name: "vpython",
				Path: "cache/vpython",
			},
		},
		CipdPackages: cipd,
		Command:      []string{python, "skia/infra/bots/run_recipe.py", "${ISOLATED_OUTDIR}", recipe, props(properties), b.cfg.Project},
		Dependencies: []string{BUNDLE_RECIPES_NAME},
		Dimensions:   dimensions,
		EnvPrefixes: map[string][]string{
			"PATH":                    []string{"cipd_bin_packages", "cipd_bin_packages/bin"},
			"VPYTHON_VIRTUALENV_ROOT": []string{"cache/vpython"},
		},
		ExtraTags: map[string]string{
			"log_location": b.logdogAnnotationUrl(),
		},
		Isolate:        b.relpath(isolate),
		MaxAttempts:    attempts(name),
		Outputs:        outputs,
		ServiceAccount: serviceAccount,
	}
	timeout(task, time.Hour)
	return task
}

// internalHardwareLabel returns the internal ID for the bot, if any.
func (b *builder) internalHardwareLabel(parts map[string]string) *int {
	if b.cfg.InternalHardwareLabel != nil {
		return b.cfg.InternalHardwareLabel(parts)
	}
	return nil
}

// linuxGceDimensions are the Swarming bot dimensions for Linux GCE instances.
func (b *builder) linuxGceDimensions(machineType string) []string {
	return []string{
		// Specify CPU to avoid running builds on bots with a more unique CPU.
		"cpu:x86-64-Haswell_GCE",
		"gpu:none",
		// Currently all Linux GCE tasks run on 16-CPU machines.
		fmt.Sprintf("machine_type:%s", machineType),
		fmt.Sprintf("os:%s", DEFAULT_OS_LINUX_GCE),
		fmt.Sprintf("pool:%s", b.cfg.Pool),
	}
}

// dockerGceDimensions are the Swarming bot dimensions for Linux GCE instances
// which have Docker installed.
func (b *builder) dockerGceDimensions() []string {
	// There's limited parallelism for WASM builds, so we can get away with the medium
	// instance instead of the beefy large instance.
	// Docker being installed is the most important part.
	return append(b.linuxGceDimensions(MACHINE_TYPE_MEDIUM), "docker_installed:true")
}

// deriveCompileTaskName returns the name of a compile task based on the given
// job name.
func (b *builder) deriveCompileTaskName(jobName string, parts map[string]string) string {
	if parts["role"] == "Test" || parts["role"] == "Perf" || parts["role"] == "Calmbench" {
		task_os := parts["os"]
		ec := []string{}
		if val := parts["extra_config"]; val != "" {
			ec = strings.Split(val, "_")
			ignore := []string{
				"Skpbench", "AbandonGpuContext", "PreAbandonGpuContext", "Valgrind",
				"ReleaseAndAbandonGpuContext", "CCPR", "FSAA", "FAAA", "FDAA", "NativeFonts", "GDI",
				"NoGPUThreads", "ProcDump", "DDL1", "DDL3", "T8888", "DDLTotal", "DDLRecord", "9x9",
				"BonusConfigs", "SkottieTracing", "SkottieWASM", "NonNVPR", "Mskp"}
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
		if strings.Contains(jobName, "CanvasKit") || strings.Contains(jobName, "SkottieWASM") {
			if parts["cpu_or_gpu"] == "CPU" {
				ec = []string{"CanvasKit_CPU"}
			} else {
				ec = []string{"CanvasKit"}
			}

		}
		if len(ec) > 0 {
			jobNameMap["extra_config"] = strings.Join(ec, "_")
		}
		name, err := b.jobNameSchema.MakeJobName(jobNameMap)
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
func (b *builder) swarmDimensions(parts map[string]string) []string {
	if b.cfg.SwarmDimensions != nil {
		dims := b.cfg.SwarmDimensions(parts)
		if dims != nil {
			return dims
		}
	}
	return b.defaultSwarmDimensions(parts)
}

// defaultSwarmDimensions generates default swarming bot dimensions for the given task.
func (b *builder) defaultSwarmDimensions(parts map[string]string) []string {
	d := map[string]string{
		"pool": b.cfg.Pool,
	}
	if os, ok := parts["os"]; ok {
		d["os"], ok = map[string]string{
			"Android":    "Android",
			"Chromecast": "Android",
			"ChromeOS":   "ChromeOS",
			"Debian9":    DEFAULT_OS_DEBIAN,
			"Mac":        DEFAULT_OS_MAC,
			"Mac10.13":   "Mac-10.13.6",
			"Mac10.14":   "Mac-10.14.3",
			"Ubuntu18":   "Ubuntu-18.04",
			"Win":        DEFAULT_OS_WIN,
			"Win10":      "Windows-10-18362",
			"Win2016":    DEFAULT_OS_WIN,
			"Win7":       "Windows-7-SP1",
			"Win8":       "Windows-8.1-SP0",
			"iOS":        "iOS-11.4.1",
		}[os]
		if !ok {
			glog.Fatalf("Entry %q not found in OS mapping.", os)
		}
		if os == "Win10" && parts["model"] == "Golo" {
			// ChOps-owned machines have Windows 10 v1709.
			d["os"] = "Windows-10-16299"
		}
		if os == "Mac10.14" && parts["model"] == "VMware7.1" {
			// ChOps VMs are at a newer version of MacOS.
			d["os"] = "Mac-10.14.6"
		}
		if d["os"] == DEFAULT_OS_WIN {
			// Upgrades result in a new image but not a new OS version.
			d["image"] = "windows-server-2016-dc-v20190108"
		}
		if parts["model"] == "LenovoYogaC630" {
			// This is currently a unique snowflake.
			d["os"] = "Windows-10"
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
				"GalaxyS6":        {"zerofltetmo", "NRD90M_G920TUVS6FRC1"},
				"GalaxyS7_G930FD": {"herolte", "R16NW_G930FXXS2ERH6"}, // This is Oreo.
				"GalaxyS9":        {"starlte", "R16NW_G960FXXU2BRJ8"}, // This is Oreo.
				"MotoG4":          {"athene", "NPJS25.93-14.7-8"},
				"NVIDIA_Shield":   {"foster", "OPR6.170623.010_3507953_1441.7411"},
				"Nexus5":          {"hammerhead", "M4B30Z_3437181"},
				"Nexus5x":         {"bullhead", "OPR6.170623.023"},
				"Nexus7":          {"grouper", "LMY47V_1836172"}, // 2012 Nexus 7
				"P30":             {"HWELE", "HUAWEIELE-L29"},
				"Pixel":           {"sailfish", "PPR1.180610.009"},
				"Pixel2XL":        {"taimen", "PPR1.180610.009"},
				"Pixel3":          {"blueline", "PQ1A.190105.004"},
				"Pixel3a":         {"sargo", "QP1A.190711.020"},
				"TecnoSpark3Pro":  {"TECNO-KB8", "PPR1.180610.011"},
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
				"iPhone8":   "iPhone10,1",
				"iPadPro":   "iPad6,3",
			}[parts["model"]]
			if !ok {
				glog.Fatalf("Entry %q not found in iOS mapping.", parts["model"])
			}
			d["device_type"] = device
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
					"Golo":      "x86-64-E5-2670",
					"VMware7.1": "x86-64-E5-2697_v2",
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
				"Snapdragon850": {
					"LenovoYogaC630": "arm64-64-Snapdragon850",
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
				return b.dockerGceDimensions()
			} else if strings.Contains(parts["os"], "Win") {
				gpu, ok := map[string]string{
					// At some point this might use the device ID, but for now it's like Chromebooks.
					"Adreno630":     "Adreno630",
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
				return b.dockerGceDimensions()
			}
			if parts["role"] == "BuildStats" {
				// Doesn't require a lot of resources, but some steps require docker
				return b.dockerGceDimensions()
			}
			// Use many-core machines for Build tasks.
			return b.linuxGceDimensions(MACHINE_TYPE_LARGE)
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
func (b *builder) relpath(f string) string {
	target := filepath.Join(b.relpathTargetDir, f)
	rv, err := filepath.Rel(b.relpathBaseDir, target)
	if err != nil {
		sklog.Fatal(err)
	}
	return rv
}

// bundleRecipes generates the task to bundle and isolate the recipes.
func (b *builder) bundleRecipes() string {
	pkgs := append([]*specs.CipdPackage{}, CIPD_PKGS_GIT...)
	pkgs = append(pkgs, CIPD_PKGS_PYTHON...)
	b.MustAddTask(BUNDLE_RECIPES_NAME, &specs.TaskSpec{
		CipdPackages: pkgs,
		Command: []string{
			"/bin/bash", "skia/infra/bots/bundle_recipes.sh", specs.PLACEHOLDER_ISOLATED_OUTDIR,
		},
		Dimensions: b.linuxGceDimensions(MACHINE_TYPE_SMALL),
		EnvPrefixes: map[string][]string{
			"PATH": []string{"cipd_bin_packages", "cipd_bin_packages/bin"},
		},
		Idempotent: true,
		Isolate:    b.relpath("recipes.isolate"),
	})
	return BUNDLE_RECIPES_NAME
}

// buildTaskDrivers generates the task to compile the task driver code to run on
// all platforms.
func (b *builder) buildTaskDrivers() string {
	b.MustAddTask(BUILD_TASK_DRIVERS_NAME, &specs.TaskSpec{
		Caches:       CACHES_GO,
		CipdPackages: append(CIPD_PKGS_GIT, b.MustGetCipdPackageFromAsset("go")),
		Command: []string{
			"/bin/bash", "skia/infra/bots/build_task_drivers.sh", specs.PLACEHOLDER_ISOLATED_OUTDIR,
		},
		Dimensions: b.linuxGceDimensions(MACHINE_TYPE_SMALL),
		EnvPrefixes: map[string][]string{
			"PATH": {"cipd_bin_packages", "cipd_bin_packages/bin", "go/go/bin"},
		},
		Idempotent: true,
		Isolate:    "task_drivers.isolate",
	})
	return BUILD_TASK_DRIVERS_NAME
}

// updateGoDeps generates the task to update Go dependencies.
func (b *builder) updateGoDeps(name string) string {
	cipd := append([]*specs.CipdPackage{}, CIPD_PKGS_GIT...)
	cipd = append(cipd, b.MustGetCipdPackageFromAsset("go"))
	cipd = append(cipd, b.MustGetCipdPackageFromAsset("protoc"))

	machineType := MACHINE_TYPE_MEDIUM
	t := &specs.TaskSpec{
		Caches:       CACHES_GO,
		CipdPackages: cipd,
		Command: []string{
			"./update_go_deps",
			"--project_id", "skia-swarming-bots",
			"--task_id", specs.PLACEHOLDER_TASK_ID,
			"--task_name", name,
			"--workdir", ".",
			"--gerrit_project", "skia",
			"--gerrit_url", "https://skia-review.googlesource.com",
			"--repo", specs.PLACEHOLDER_REPO,
			"--reviewers", "borenet@google.com",
			"--revision", specs.PLACEHOLDER_REVISION,
			"--patch_issue", specs.PLACEHOLDER_ISSUE,
			"--patch_set", specs.PLACEHOLDER_PATCHSET,
			"--patch_server", specs.PLACEHOLDER_CODEREVIEW_SERVER,
			"--alsologtostderr",
		},
		Dependencies: []string{BUILD_TASK_DRIVERS_NAME},
		Dimensions:   b.linuxGceDimensions(machineType),
		EnvPrefixes: map[string][]string{
			"PATH": {"cipd_bin_packages", "cipd_bin_packages/bin", "go/go/bin"},
		},
		Isolate:        "empty.isolate",
		ServiceAccount: b.cfg.ServiceAccountRecreateSKPs,
	}
	b.MustAddTask(name, t)
	return name
}

// isolateAssetConfig represents a task which copies a CIPD package into
// isolate.
type isolateAssetCfg struct {
	cipdPkg string
	path    string
}

// isolateCIPDAsset generates a task to isolate the given CIPD asset.
func (b *builder) isolateCIPDAsset(name string) string {
	asset := ISOLATE_ASSET_MAPPING[name]
	b.MustAddTask(name, &specs.TaskSpec{
		CipdPackages: []*specs.CipdPackage{
			b.MustGetCipdPackageFromAsset(asset.cipdPkg),
		},
		Command:    []string{"/bin/cp", "-rL", asset.path, "${ISOLATED_OUTDIR}"},
		Dimensions: b.linuxGceDimensions(MACHINE_TYPE_SMALL),
		Idempotent: true,
		Isolate:    b.relpath("empty.isolate"),
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
		deps = append(deps, ISOLATE_MSKP_NAME)
	} else if util.In(o, rpiOS) {
		deps = append(deps, ISOLATE_SKP_NAME)
		deps = append(deps, ISOLATE_SVG_NAME)
		deps = append(deps, ISOLATE_SKIMAGE_NAME)
	}

	return deps
}

// usesGit adds attributes to tasks which use git.
func (b *builder) usesGit(t *specs.TaskSpec, name string) {
	t.Caches = append(t.Caches, CACHES_GIT...)
	if !strings.Contains(name, "NoDEPS") {
		t.Caches = append(t.Caches, CACHES_WORKDIR...)
	}
	t.CipdPackages = append(t.CipdPackages, CIPD_PKGS_GIT...)
}

// usesGo adds attributes to tasks which use go. Recipes should use
// "with api.context(env=api.infra.go_env)".
func (b *builder) usesGo(t *specs.TaskSpec, name string) {
	t.Caches = append(t.Caches, CACHES_GO...)
	pkg := b.MustGetCipdPackageFromAsset("go")
	if strings.Contains(name, "Win") {
		pkg = b.MustGetCipdPackageFromAsset("go_win")
		pkg.Path = "go"
	}
	t.CipdPackages = append(t.CipdPackages, pkg)
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
	if strings.Contains(name, "Android_Framework") || strings.Contains(name, "G3_Framework") {
		// Both bots can be long running. No need to retry them.
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
func (b *builder) compile(name string, parts map[string]string) string {
	recipe := "compile"
	isolate := "compile.isolate"
	var props map[string]string
	needSync := false
	if strings.Contains(name, "NoDEPS") ||
		strings.Contains(name, "CMake") ||
		strings.Contains(name, "CommandBuffer") ||
		strings.Contains(name, "Flutter") ||
		strings.Contains(name, "ParentRevision") ||
		strings.Contains(name, "SKQP") {
		recipe = "sync_and_compile"
		isolate = "swarm_recipe.isolate"
		props = EXTRA_PROPS
		needSync = true
	}
	task := b.kitchenTask(name, recipe, isolate, b.cfg.ServiceAccountCompile, b.swarmDimensions(parts), props, OUTPUT_BUILD)
	if needSync {
		b.usesGit(task, name)
	} else {
		task.Idempotent = true
	}
	usesDocker(task, name)

	// Android bots require a toolchain.
	if strings.Contains(name, "Android") {
		if strings.Contains(name, "Mac") {
			task.CipdPackages = append(task.CipdPackages, b.MustGetCipdPackageFromAsset("android_ndk_darwin"))
		} else if strings.Contains(name, "Win") {
			pkg := b.MustGetCipdPackageFromAsset("android_ndk_windows")
			pkg.Path = "n"
			task.CipdPackages = append(task.CipdPackages, pkg)
		} else if !strings.Contains(name, "SKQP") {
			task.CipdPackages = append(task.CipdPackages, b.MustGetCipdPackageFromAsset("android_ndk_linux"))
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
		task.Dependencies = append(task.Dependencies, b.isolateCIPDAsset(ISOLATE_WIN_TOOLCHAIN_NAME))
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
		if strings.Contains(name, "iOS") {
			task.CipdPackages = append(task.CipdPackages, b.MustGetCipdPackageFromAsset("provisioning_profile_ios"))
		}
	}

	// Add the task.
	b.MustAddTask(name, task)

	// All compile tasks are runnable as their own Job. Assert that the Job
	// is listed in jobs.
	if !util.In(name, b.jobs) {
		glog.Fatalf("Job %q is missing from the jobs list!", name)
	}

	return name
}

// recreateSKPs generates a RecreateSKPs task. Returns the name of the last
// task in the generated chain of tasks, which the Job should add as a
// dependency.
func (b *builder) recreateSKPs(name string) string {
	dims := []string{
		"pool:SkiaCT",
		fmt.Sprintf("os:%s", DEFAULT_OS_LINUX_GCE),
	}
	task := b.kitchenTask(name, "recreate_skps", "swarm_recipe.isolate", b.cfg.ServiceAccountRecreateSKPs, dims, EXTRA_PROPS, OUTPUT_NONE)
	task.CipdPackages = append(task.CipdPackages, CIPD_PKGS_GIT...)
	b.usesGo(task, name)
	timeout(task, 4*time.Hour)
	b.MustAddTask(name, task)
	return name
}

// checkGeneratedFiles verifies that no generated SKSL files have been edited
// by hand.
func (b *builder) checkGeneratedFiles(name string) string {
	task := b.kitchenTask(name, "check_generated_files", "swarm_recipe.isolate", b.cfg.ServiceAccountCompile, b.linuxGceDimensions(MACHINE_TYPE_LARGE), EXTRA_PROPS, OUTPUT_NONE)
	task.Caches = append(task.Caches, CACHES_WORKDIR...)
	b.usesGo(task, name)
	b.MustAddTask(name, task)
	return name
}

// housekeeper generates a Housekeeper task. Returns the name of the last task
// in the generated chain of tasks, which the Job should add as a dependency.
func (b *builder) housekeeper(name string) string {
	task := b.kitchenTask(name, "housekeeper", "swarm_recipe.isolate", b.cfg.ServiceAccountHousekeeper, b.linuxGceDimensions(MACHINE_TYPE_SMALL), EXTRA_PROPS, OUTPUT_NONE)
	b.usesGit(task, name)
	b.MustAddTask(name, task)
	return name
}

// androidFrameworkCompile generates an Android Framework Compile task. Returns
// the name of the last task in the generated chain of tasks, which the Job
// should add as a dependency.
func (b *builder) androidFrameworkCompile(name string) string {
	task := b.kitchenTask(name, "android_compile", "compile_android_framework.isolate", b.cfg.ServiceAccountCompile, b.linuxGceDimensions(MACHINE_TYPE_SMALL), EXTRA_PROPS, OUTPUT_NONE)
	timeout(task, 2*time.Hour)
	b.MustAddTask(name, task)
	return name
}

// g3FrameworkCompile generates a G3 Framework Compile task. Returns
// the name of the last task in the generated chain of tasks, which the Job
// should add as a dependency.
func (b *builder) g3FrameworkCompile(name string) string {
	task := b.kitchenTask(name, "g3_compile", "compile_g3_framework.isolate", b.cfg.ServiceAccountCompile, b.linuxGceDimensions(MACHINE_TYPE_SMALL), EXTRA_PROPS, OUTPUT_NONE)
	timeout(task, 3*time.Hour)
	b.MustAddTask(name, task)
	return name
}

// infra generates an infra_tests task. Returns the name of the last task in the
// generated chain of tasks, which the Job should add as a dependency.
func (b *builder) infra(name string) string {
	dims := b.linuxGceDimensions(MACHINE_TYPE_SMALL)
	if strings.Contains(name, "Win") {
		dims = []string{
			// Specify CPU to avoid running builds on bots with a more unique CPU.
			"cpu:x86-64-Haswell_GCE",
			"gpu:none",
			fmt.Sprintf("machine_type:%s", MACHINE_TYPE_MEDIUM), // We don't have any small Windows instances.
			fmt.Sprintf("os:%s", DEFAULT_OS_WIN),
			fmt.Sprintf("pool:%s", b.cfg.Pool),
		}
	}
	extraProps := map[string]string{
		"repository": specs.PLACEHOLDER_REPO,
	}
	task := b.kitchenTask(name, "infra", "infra_tests.isolate", b.cfg.ServiceAccountCompile, dims, extraProps, OUTPUT_NONE)
	task.CipdPackages = append(task.CipdPackages, CIPD_PKGS_GSUTIL...)
	task.Idempotent = true
	// Repos which call into Skia's gen_tasks.go should define their own
	// infra_tests.isolate and therefore should not use relpath().
	task.Isolate = "infra_tests.isolate"
	b.usesGit(task, name) // We don't run bot_update, but Go needs a git repo.
	b.usesGo(task, name)
	b.MustAddTask(name, task)
	return name
}

// buildstats generates a builtstats task, which compiles code and generates
// statistics about the build.
func (b *builder) buildstats(name string, parts map[string]string, compileTaskName string) string {
	task := b.kitchenTask(name, "compute_buildstats", "swarm_recipe.isolate", "", b.swarmDimensions(parts), EXTRA_PROPS, OUTPUT_PERF)
	task.Dependencies = append(task.Dependencies, compileTaskName)
	task.CipdPackages = append(task.CipdPackages, b.MustGetCipdPackageFromAsset("bloaty"))
	b.MustAddTask(name, task)

	// Upload release results (for tracking in perf)
	// We have some jobs that are FYI (e.g. Debug-CanvasKit, tree-map generator)
	if strings.Contains(name, "Release") && !util.In(name, BUILD_STATS_NO_UPLOAD) {
		uploadName := fmt.Sprintf("%s%s%s", PREFIX_UPLOAD, b.jobNameSchema.Sep, name)
		extraProps := map[string]string{
			"gs_bucket": b.cfg.GsBucketNano,
		}
		for k, v := range EXTRA_PROPS {
			extraProps[k] = v
		}
		uploadTask := b.kitchenTask(name, "upload_buildstats_results", "swarm_recipe.isolate", b.cfg.ServiceAccountUploadNano, b.linuxGceDimensions(MACHINE_TYPE_SMALL), extraProps, OUTPUT_NONE)
		uploadTask.CipdPackages = append(uploadTask.CipdPackages, CIPD_PKGS_GSUTIL...)
		uploadTask.Dependencies = append(uploadTask.Dependencies, name)
		b.MustAddTask(uploadName, uploadTask)
		return uploadName
	}

	return name
}

// getParentRevisionName returns the name of a compile task which builds
// against a "parent" revision.
func getParentRevisionName(compileTaskName string, parts map[string]string) string {
	if parts["extra_config"] == "" {
		return compileTaskName + "-ParentRevision"
	} else {
		return compileTaskName + "_ParentRevision"
	}
}

// calmbench generates a calmbench task. Returns the name of the last task in the
// generated chain of tasks, which the Job should add as a dependency.
func (b *builder) calmbench(name string, parts map[string]string, compileTaskName, compileParentName string) string {
	task := b.kitchenTask(name, "calmbench", "calmbench.isolate", "", b.swarmDimensions(parts), EXTRA_PROPS, OUTPUT_PERF)
	b.usesGit(task, name)
	task.Dependencies = append(task.Dependencies, compileTaskName, compileParentName, ISOLATE_SKP_NAME, ISOLATE_SVG_NAME)
	b.MustAddTask(name, task)

	// Upload results if necessary.
	if strings.Contains(name, "Release") && b.doUpload(name) {
		uploadName := fmt.Sprintf("%s%s%s", PREFIX_UPLOAD, b.jobNameSchema.Sep, name)
		extraProps := map[string]string{
			"gs_bucket": b.cfg.GsBucketCalm,
		}
		for k, v := range EXTRA_PROPS {
			extraProps[k] = v
		}
		uploadTask := b.kitchenTask(name, "upload_calmbench_results", "swarm_recipe.isolate", b.cfg.ServiceAccountUploadCalmbench, b.linuxGceDimensions(MACHINE_TYPE_SMALL), extraProps, OUTPUT_NONE)
		uploadTask.CipdPackages = append(uploadTask.CipdPackages, CIPD_PKGS_GSUTIL...)
		uploadTask.Dependencies = append(uploadTask.Dependencies, name)
		b.MustAddTask(uploadName, uploadTask)
		return uploadName
	}

	return name
}

// doUpload indicates whether the given Job should upload its results.
func (b *builder) doUpload(name string) bool {
	for _, s := range b.cfg.NoUpload {
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
func (b *builder) test(name string, parts map[string]string, compileTaskName string, pkgs []*specs.CipdPackage) string {
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
		"gold_hashes_url": b.cfg.GoldHashesURL,
	}
	for k, v := range EXTRA_PROPS {
		extraProps[k] = v
	}
	iid := b.internalHardwareLabel(parts)
	if iid != nil {
		extraProps["internal_hardware_label"] = strconv.Itoa(*iid)
	}
	isolate := "test_skia_bundled.isolate"
	if strings.Contains(name, "CanvasKit") || strings.Contains(name, "Emulator") || strings.Contains(name, "LottieWeb") || strings.Contains(name, "PathKit") {
		isolate = "swarm_recipe.isolate"
	}
	task := b.kitchenTask(name, recipe, isolate, "", b.swarmDimensions(parts), extraProps, OUTPUT_TEST)
	task.CipdPackages = append(task.CipdPackages, pkgs...)
	if strings.Contains(name, "Lottie") {
		task.CipdPackages = append(task.CipdPackages, b.MustGetCipdPackageFromAsset("lottie-samples"))
	}
	if !strings.Contains(name, "LottieWeb") {
		// Test.+LottieWeb doesn't require anything in Skia to be compiled.
		task.Dependencies = append(task.Dependencies, compileTaskName)
	}

	if strings.Contains(name, "Android_ASAN") {
		task.Dependencies = append(task.Dependencies, b.isolateCIPDAsset(ISOLATE_NDK_LINUX_NAME))
	}
	if strings.Contains(name, "SKQP") {
		if !strings.Contains(name, "Emulator") {
			task.Dependencies = append(task.Dependencies, b.isolateCIPDAsset(ISOLATE_GCLOUD_LINUX_NAME))
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
	if b.doUpload(name) {
		uploadName := fmt.Sprintf("%s%s%s", PREFIX_UPLOAD, b.jobNameSchema.Sep, name)
		extraProps := map[string]string{
			"gs_bucket": b.cfg.GsBucketGm,
		}
		for k, v := range EXTRA_PROPS {
			extraProps[k] = v
		}
		uploadTask := b.kitchenTask(name, "upload_dm_results", "swarm_recipe.isolate", b.cfg.ServiceAccountUploadGM, b.linuxGceDimensions(MACHINE_TYPE_SMALL), extraProps, OUTPUT_NONE)
		uploadTask.CipdPackages = append(uploadTask.CipdPackages, CIPD_PKGS_GSUTIL...)
		uploadTask.Dependencies = append(uploadTask.Dependencies, name)
		b.MustAddTask(uploadName, uploadTask)
		return uploadName
	}

	return name
}

// perf generates a Perf task. Returns the name of the last task in the
// generated chain of tasks, which the Job should add as a dependency.
func (b *builder) perf(name string, parts map[string]string, compileTaskName string, pkgs []*specs.CipdPackage) string {
	recipe := "perf"
	isolate := b.relpath("perf_skia_bundled.isolate")
	if strings.Contains(parts["extra_config"], "Skpbench") {
		recipe = "skpbench"
		isolate = b.relpath("skpbench_skia_bundled.isolate")
	} else if strings.Contains(name, "PathKit") {
		recipe = "perf_pathkit"
	} else if strings.Contains(name, "CanvasKit") {
		recipe = "perf_canvaskit"
	} else if strings.Contains(name, "SkottieTracing") {
		recipe = "perf_skottietrace"
	} else if strings.Contains(name, "SkottieWASM") || strings.Contains(name, "LottieWeb") {
		recipe = "perf_skottiewasm_lottieweb"
	}
	task := b.kitchenTask(name, recipe, isolate, "", b.swarmDimensions(parts), EXTRA_PROPS, OUTPUT_PERF)
	task.CipdPackages = append(task.CipdPackages, pkgs...)
	if !strings.Contains(name, "LottieWeb") {
		// Perf.+LottieWeb doesn't require anything in Skia to be compiled.
		task.Dependencies = append(task.Dependencies, compileTaskName)
	}
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
	} else if strings.Contains(parts["extra_config"], "SkottieWASM") || strings.Contains(parts["extra_config"], "LottieWeb") {
		task.CipdPackages = append(task.CipdPackages, b.MustGetCipdPackageFromAsset("node"))
		task.CipdPackages = append(task.CipdPackages, b.MustGetCipdPackageFromAsset("lottie-samples"))
		task.CipdPackages = append(task.CipdPackages, CIPD_PKGS_GIT...)
	} else if strings.Contains(parts["extra_config"], "Skottie") {
		task.CipdPackages = append(task.CipdPackages, b.MustGetCipdPackageFromAsset("lottie-samples"))
	}
	iid := b.internalHardwareLabel(parts)
	if iid != nil {
		task.Command = append(task.Command, fmt.Sprintf("internal_hardware_label=%d", *iid))
	}
	b.MustAddTask(name, task)

	// Upload results if necessary.
	if strings.Contains(name, "Release") && b.doUpload(name) {
		uploadName := fmt.Sprintf("%s%s%s", PREFIX_UPLOAD, b.jobNameSchema.Sep, name)
		extraProps := map[string]string{
			"gs_bucket": b.cfg.GsBucketNano,
		}
		for k, v := range EXTRA_PROPS {
			extraProps[k] = v
		}
		uploadTask := b.kitchenTask(name, "upload_nano_results", "swarm_recipe.isolate", b.cfg.ServiceAccountUploadNano, b.linuxGceDimensions(MACHINE_TYPE_SMALL), extraProps, OUTPUT_NONE)
		uploadTask.CipdPackages = append(uploadTask.CipdPackages, CIPD_PKGS_GSUTIL...)
		uploadTask.Dependencies = append(uploadTask.Dependencies, name)
		b.MustAddTask(uploadName, uploadTask)
		return uploadName
	}
	return name
}

// presubmit generates a task which runs the presubmit for this repo.
func (b *builder) presubmit(name string) string {
	extraProps := map[string]string{
		"category":         "cq",
		"patch_gerrit_url": "https://skia-review.googlesource.com",
		"patch_project":    "skia",
		"patch_ref":        specs.PLACEHOLDER_PATCH_REF,
		"reason":           "CQ",
		"repo_name":        "skia",
	}
	for k, v := range EXTRA_PROPS {
		extraProps[k] = v
	}
	// Use MACHINE_TYPE_LARGE because it seems to save time versus MEDIUM and we want presubmit to be
	// fast.
	task := b.kitchenTask(name, "run_presubmit", "run_recipe.isolate", b.cfg.ServiceAccountCompile, b.linuxGceDimensions(MACHINE_TYPE_LARGE), extraProps, OUTPUT_NONE)
	b.usesGit(task, name)
	task.CipdPackages = append(task.CipdPackages, &specs.CipdPackage{
		Name:    "infra/recipe_bundles/chromium.googlesource.com/chromium/tools/build",
		Path:    "recipe_bundle",
		Version: "git_revision:617e0fd3186eaae8bcb7521def0d6d3b4a5bcaf1",
	})
	task.Dependencies = []string{} // No bundled recipes for this one.
	b.MustAddTask(name, task)
	return name
}

// process generates tasks and jobs for the given job name.
func (b *builder) process(name string) {
	var priority float64 // Leave as default for most jobs.
	deps := []string{}

	// Bundle Recipes.
	if name == BUNDLE_RECIPES_NAME {
		deps = append(deps, b.bundleRecipes())
	}
	if name == BUILD_TASK_DRIVERS_NAME {
		deps = append(deps, b.buildTaskDrivers())
	}

	// Isolate CIPD assets.
	if _, ok := ISOLATE_ASSET_MAPPING[name]; ok {
		deps = append(deps, b.isolateCIPDAsset(name))
	}

	parts, err := b.jobNameSchema.ParseJobName(name)
	if err != nil {
		glog.Fatal(err)
	}

	// RecreateSKPs.
	if strings.Contains(name, "RecreateSKPs") {
		deps = append(deps, b.recreateSKPs(name))
	}

	// Update Go Dependencies.
	if strings.Contains(name, "UpdateGoDeps") {
		// Update Go deps bot.
		deps = append(deps, b.updateGoDeps(name))
	}

	// Infra tests.
	if strings.Contains(name, "Housekeeper-PerCommit-InfraTests") {
		deps = append(deps, b.infra(name))
	}

	// Compile bots.
	if parts["role"] == "Build" {
		if parts["extra_config"] == "Android_Framework" {
			// Android Framework compile tasks use a different recipe.
			deps = append(deps, b.androidFrameworkCompile(name))
		} else if parts["extra_config"] == "G3_Framework" {
			// G3 compile tasks use a different recipe.
			deps = append(deps, b.g3FrameworkCompile(name))
		} else {
			deps = append(deps, b.compile(name, parts))
		}
	}

	// Most remaining bots need a compile task.
	compileTaskName := b.deriveCompileTaskName(name, parts)
	compileTaskParts, err := b.jobNameSchema.ParseJobName(compileTaskName)
	if err != nil {
		glog.Fatal(err)
	}
	compileParentName := getParentRevisionName(compileTaskName, compileTaskParts)
	compileParentParts, err := b.jobNameSchema.ParseJobName(compileParentName)
	if err != nil {
		glog.Fatal(err)
	}

	// These bots do not need a compile task.
	if parts["role"] != "Build" &&
		name != "Housekeeper-PerCommit-BundleRecipes" &&
		!strings.Contains(name, "Housekeeper-PerCommit-InfraTests") &&
		name != "Housekeeper-PerCommit-CheckGeneratedFiles" &&
		name != "Housekeeper-Nightly-UpdateGoDeps" &&
		name != "Housekeeper-OnDemand-Presubmit" &&
		name != "Housekeeper-PerCommit" &&
		name != BUILD_TASK_DRIVERS_NAME &&
		!strings.Contains(name, "Android_Framework") &&
		!strings.Contains(name, "G3_Framework") &&
		!strings.Contains(name, "RecreateSKPs") &&
		!strings.Contains(name, "Housekeeper-PerCommit-Isolate") &&
		!strings.Contains(name, "SkottieWASM") &&
		!strings.Contains(name, "LottieWeb") {
		b.compile(compileTaskName, compileTaskParts)
		if parts["role"] == "Calmbench" {
			b.compile(compileParentName, compileParentParts)
		}
	}

	// Housekeepers.
	if name == "Housekeeper-PerCommit" {
		deps = append(deps, b.housekeeper(name))
	}
	if name == "Housekeeper-PerCommit-CheckGeneratedFiles" {
		deps = append(deps, b.checkGeneratedFiles(name))
	}
	if name == "Housekeeper-OnDemand-Presubmit" {
		priority = 1
		deps = append(deps, b.presubmit(name))
	}

	// Common assets needed by the remaining bots.

	pkgs := []*specs.CipdPackage{}

	if deps := getIsolatedCIPDDeps(parts); len(deps) == 0 {
		// for desktop machines
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
	if strings.Contains(name, "CanvasKit") || (parts["role"] == "Test" && strings.Contains(name, "LottieWeb")) || strings.Contains(name, "PathKit") {
		// Docker-based tests that don't need the standard CIPD assets
		pkgs = []*specs.CipdPackage{}
	}

	// Test bots.
	if parts["role"] == "Test" {
		deps = append(deps, b.test(name, parts, compileTaskName, pkgs))
	}

	// Perf bots.
	if parts["role"] == "Perf" {
		deps = append(deps, b.perf(name, parts, compileTaskName, pkgs))
	}

	// Calmbench bots.
	if parts["role"] == "Calmbench" {
		deps = append(deps, b.calmbench(name, parts, compileTaskName, compileParentName))
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
		deps = append(deps, b.buildstats(name, parts, compileTaskName))
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
	} else if strings.Contains(name, "-OnDemand-") || strings.Contains(name, "Android_Framework") || strings.Contains(name, "G3_Framework") {
		j.Trigger = specs.TRIGGER_ON_DEMAND
	}
	b.MustAddJob(name, j)
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
