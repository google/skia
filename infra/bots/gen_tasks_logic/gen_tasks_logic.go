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
	"log"
	"os"
	"path"
	"path/filepath"
	"regexp"
	"runtime"
	"sort"
	"strconv"
	"strings"
	"time"

	"go.skia.org/infra/go/cas/rbe"
	"go.skia.org/infra/go/cipd"
	"go.skia.org/infra/task_scheduler/go/specs"
	"go.skia.org/skia/bazel/device_specific_configs"
)

const (
	CAS_BAZEL         = "bazel"
	CAS_CANVASKIT     = "canvaskit"
	CAS_COMPILE       = "compile"
	CAS_EMPTY         = "empty" // TODO(borenet): It'd be nice if this wasn't necessary.
	CAS_LOTTIE_CI     = "lottie-ci"
	CAS_LOTTIE_WEB    = "lottie-web"
	CAS_PERF          = "perf"
	CAS_PUPPETEER     = "puppeteer"
	CAS_RUN_RECIPE    = "run-recipe"
	CAS_RECIPES       = "recipes"
	CAS_RECREATE_SKPS = "recreate-skps"
	CAS_SKOTTIE_WASM  = "skottie-wasm"
	CAS_TASK_DRIVERS  = "task-drivers"
	CAS_TEST          = "test"
	CAS_WASM_GM       = "wasm-gm"
	CAS_WHOLE_REPO    = "whole-repo"

	BUILD_TASK_DRIVERS_PREFIX  = "Housekeeper-PerCommit-BuildTaskDrivers"
	BUNDLE_RECIPES_NAME        = "Housekeeper-PerCommit-BundleRecipes"
	ISOLATE_GCLOUD_LINUX_NAME  = "Housekeeper-PerCommit-IsolateGCloudLinux"
	ISOLATE_SKIMAGE_NAME       = "Housekeeper-PerCommit-IsolateSkImage"
	ISOLATE_SKP_NAME           = "Housekeeper-PerCommit-IsolateSKP"
	ISOLATE_MSKP_NAME          = "Housekeeper-PerCommit-IsolateMSKP"
	ISOLATE_SVG_NAME           = "Housekeeper-PerCommit-IsolateSVG"
	ISOLATE_NDK_LINUX_NAME     = "Housekeeper-PerCommit-IsolateAndroidNDKLinux"
	ISOLATE_SDK_LINUX_NAME     = "Housekeeper-PerCommit-IsolateAndroidSDKLinux"
	ISOLATE_WIN_TOOLCHAIN_NAME = "Housekeeper-PerCommit-IsolateWinToolchain"

	DEBIAN_11_OS         = "Debian-11.5"
	DEBIAN_10_OS         = "Debian-10.10"
	DEFAULT_OS_LINUX_GCE = UBUNTU_24_04_OS
	DEFAULT_OS_MAC       = "Mac-14.5"
	DEFAULT_OS_WIN_GCE   = "Windows-11-22631"
	UBUNTU_20_04_OS      = "Ubuntu-20.04"
	UBUNTU_22_04_OS      = "Ubuntu-22.04"
	UBUNTU_24_04_OS      = "Ubuntu-24.04"

	// Small is a 2-core machine.
	// TODO(dogben): Would n1-standard-1 or n1-standard-2 be sufficient?
	MACHINE_TYPE_SMALL = "n1-highmem-2"
	// Medium is a 16-core machine
	MACHINE_TYPE_MEDIUM = "n1-standard-16"
	// Large is a 64-core machine. (We use "highcpu" because we don't need more than 57GB memory for
	// any of our tasks.)
	MACHINE_TYPE_LARGE = "n1-highcpu-64"

	// Swarming output dirs.
	OUTPUT_NONE          = "output_ignored" // This will result in outputs not being isolated.
	OUTPUT_BUILD         = "build"
	OUTPUT_BUILD_NOPATCH = "build_nopatch"
	OUTPUT_TEST          = "test"
	OUTPUT_PERF          = "perf"
	OUTPUT_BAZEL         = "bazel_output"

	// Name prefix for upload jobs.
	PREFIX_UPLOAD = "Upload"

	// This will have to kept in sync with the kMin_Version in
	// src/core/SkPicturePriv.h
	// See the comment in that file on how to find the version to use here.
	oldestSupportedSkpVersion = 293

	// bazelCacheDirOnGCELinux is the path where Bazel should write its cache on Linux GCE machines.
	// The Bazel cache can grow large (>10GB), so this should be in a partition with enough free
	// space.
	bazelCacheDirOnGCELinux = "/home/chrome-bot/bazel_cache"

	// bazelCacheDirOnSkoloLinux is like bazelCacheDirOnGCELinux for Skolo Linux machines. Unlike GCE
	// Linux machines, the partition mounted at / on Skolo Linux machines is large enough. While
	// using the default Bazel cache path would work, our Bazel task drivers demand an explicit path.
	// We store the Bazel cache at /home/chrome-bot/bazel_cache rather than on the default location
	// of /home/chrome-bot/cache/.bazel to make it obvious to someone examining a Skolo machine that
	// we are overriding the default location.
	bazelCacheDirOnSkoloLinux = "/home/chrome-bot/bazel_cache"

	// bazelCacheDirOnWindows is like bazelCacheDirOnSkoloLinux. Unlike GCE Linux machines, we only
	// have a single partition. While using the default cache path would work, our Bazel task
	// drivers demand an explicit path. We store the Bazel cache at /home/chrome-bot/bazel_cache
	// rather than on the default location of %APPDATA% to make it obvious to someone examining a
	// Skolo machine that we are overriding the default location. Note that double-escaping the
	// path separator is necessary because this string is passed to Bazel via multiple levels of
	// subprocesses.
	bazelCacheDirOnWindows = `C:\\Users\\chrome-bot\\bazel_cache`
)

var (
	// "Constants"

	// Named caches used by tasks.
	CACHES_GIT = []*specs.Cache{
		{
			Name: "git",
			Path: "cache/git",
		},
		{
			Name: "git_cache",
			Path: "cache/git_cache",
		},
	}
	CACHES_GO = []*specs.Cache{
		{
			Name: "go_cache",
			Path: "cache/go_cache",
		},
		{
			Name: "gopath",
			Path: "cache/gopath",
		},
	}
	CACHES_WORKDIR = []*specs.Cache{
		{
			Name: "work",
			Path: "cache/work",
		},
	}
	CACHES_CCACHE = []*specs.Cache{
		{
			Name: "ccache",
			Path: "cache/ccache",
		},
	}
	// The "docker" cache is used as a persistent working directory for
	// tasks which use Docker. It is not to be confused with Docker's own
	// cache, which stores images. We do not currently use a named Swarming
	// cache for the latter.
	// TODO(borenet): We should ensure that any task which uses Docker does
	// not also use the normal "work" cache, to prevent issues like
	// https://bugs.chromium.org/p/skia/issues/detail?id=9749.
	CACHES_DOCKER = []*specs.Cache{
		{
			Name: "docker",
			Path: "cache/docker",
		},
	}

	// CAS_SPEC_LOTTIE_CI is a CasSpec which includes the files needed for
	// lottie-ci.  This is global so that it can be overridden by other
	// repositories which import this file.
	CAS_SPEC_LOTTIE_CI = &specs.CasSpec{
		Root: "..",
		Paths: []string{
			"skia/.vpython3",
			"skia/infra/bots/run_recipe.py",
			"skia/infra/lottiecap",
			"skia/tools/lottie-web-perf",
			"skia/tools/lottiecap",
		},
		Excludes: []string{rbe.ExcludeGitDir},
	}

	// CAS_SPEC_WHOLE_REPO is a CasSpec which includes the entire repo. This is
	// global so that it can be overridden by other repositories which import
	// this file.
	CAS_SPEC_WHOLE_REPO = &specs.CasSpec{
		Root:     "..",
		Paths:    []string{"skia"},
		Excludes: []string{rbe.ExcludeGitDir},
	}

	// TODO(borenet): This hacky and bad.
	CIPD_PKG_LUCI_AUTH = cipd.MustGetPackage("infra/tools/luci-auth/${platform}")

	CIPD_PKGS_GOLDCTL = cipd.MustGetPackage("skia/tools/goldctl/${platform}")

	CIPD_PKGS_XCODE = []*specs.CipdPackage{
		// https://chromium.googlesource.com/chromium/tools/build/+/e19b7d9390e2bb438b566515b141ed2b9ed2c7c2/scripts/slave/recipe_modules/ios/api.py#317
		// This package is really just an installer for XCode.
		{
			Name: "infra/tools/mac_toolchain/${platform}",
			Path: "mac_toolchain",
			// When this is updated, also update
			// https://skia.googlesource.com/skcms.git/+/f1e2b45d18facbae2dece3aca673fe1603077846/infra/bots/gen_tasks.go#56
			// and
			// https://skia.googlesource.com/skia.git/+/main/infra/bots/recipe_modules/xcode/api.py#38
			Version: "git_revision:0cb1e51344de158f72524c384f324465aebbcef2",
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

	// ISOLATE_ASSET_MAPPING maps the name of an asset to the configuration
	// for how the CIPD package should be installed for a given task.
	ISOLATE_ASSET_MAPPING = map[string]uploadAssetCASCfg{
		"gcloud_linux": {
			uploadTaskName: ISOLATE_GCLOUD_LINUX_NAME,
			path:           "gcloud_linux",
		},
		"skimage": {
			uploadTaskName: ISOLATE_SKIMAGE_NAME,
			path:           "skimage",
		},
		"skp": {
			uploadTaskName: ISOLATE_SKP_NAME,
			path:           "skp",
		},
		"svg": {
			uploadTaskName: ISOLATE_SVG_NAME,
			path:           "svg",
		},
		"mskp": {
			uploadTaskName: ISOLATE_MSKP_NAME,
			path:           "mskp",
		},
		"android_ndk_linux": {
			uploadTaskName: ISOLATE_NDK_LINUX_NAME,
			path:           "android_ndk_linux",
		},
		"android_sdk_linux": {
			uploadTaskName: ISOLATE_SDK_LINUX_NAME,
			path:           "android_sdk_linux",
		},
		"win_toolchain": {
			alwaysIsolate:  true,
			uploadTaskName: ISOLATE_WIN_TOOLCHAIN_NAME,
			path:           "win_toolchain",
		},
	}

	// Set dontReduceOpsTaskSplitting option on these models
	DONT_REDUCE_OPS_TASK_SPLITTING_MODELS = []string{
		"NUC5PPYH",
	}
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

	// GCS bucket used for GM results.
	GsBucketGm string `json:"gs_bucket_gm"`

	// GCS bucket used for Nanobench results.
	GsBucketNano string `json:"gs_bucket_nano"`

	// Optional function which returns a bot ID for internal devices.
	InternalHardwareLabel func(parts map[string]string) *int `json:"-"`

	// List of task names for which we'll never upload results.
	NoUpload []string `json:"no_upload"`

	// PathToSkia is the relative path from the root of the current checkout to
	// the root of the Skia checkout.
	PathToSkia string `json:"path_to_skia"`

	// Swarming pool used for triggering tasks.
	Pool string `json:"pool"`

	// LUCI project associated with this repo.
	Project string `json:"project"`

	// Service accounts.
	ServiceAccountCanary       string `json:"service_account_canary"`
	ServiceAccountCompile      string `json:"service_account_compile"`
	ServiceAccountHousekeeper  string `json:"service_account_housekeeper"`
	ServiceAccountRecreateSKPs string `json:"service_account_recreate_skps"`
	ServiceAccountUploadBinary string `json:"service_account_upload_binary"`
	ServiceAccountUploadGM     string `json:"service_account_upload_gm"`
	ServiceAccountUploadNano   string `json:"service_account_upload_nano"`

	// Optional override function which derives Swarming bot dimensions
	// from parts of task names.
	SwarmDimensions func(parts map[string]string) []string `json:"-"`
	// Optional function called just before adding a task.
	AddTaskCallback func(tb *TaskBuilder)
}

// JobInfo is the type of each entry in the jobs.json file.
type JobInfo struct {
	// The name of the job.
	Name string `json:"name"`

	// The optional CQ config of this job. If the CQ config is missing then the
	// job will not be added to the CQ of this branch.
	CQConfig *specs.CommitQueueJobConfig `json:"cq_config,omitempty"`
}

// LoadConfig loads the Config from a cfg.json file which is the sibling of the
// calling gen_tasks.go file.
func LoadConfig() *Config {
	cfgDir := getCallingDirName()
	var cfg Config
	LoadJSON(filepath.Join(cfgDir, "cfg.json"), &cfg)
	return &cfg
}

// CheckoutRoot is a wrapper around specs.GetCheckoutRoot which prevents the
// caller from needing a dependency on the specs package.
func CheckoutRoot() string {
	root, err := specs.GetCheckoutRoot()
	if err != nil {
		log.Fatal(err)
	}
	return root
}

// LoadJSON loads JSON from the given file and unmarshals it into the given destination.
func LoadJSON(filename string, dest interface{}) {
	b, err := os.ReadFile(filename)
	if err != nil {
		log.Fatalf("Unable to read %q: %s", filename, err)
	}
	if err := json.Unmarshal(b, dest); err != nil {
		log.Fatalf("Unable to parse %q: %s", filename, err)
	}
}

// In returns true if |s| is *in* |a| slice.
// TODO(borenet): This is copied from go.skia.org/infra/go/util to avoid the
// huge set of additional dependencies added by that package.
func In(s string, a []string) bool {
	for _, x := range a {
		if x == s {
			return true
		}
	}
	return false
}

// GenTasks regenerates the tasks.json file. Loads the job list from a jobs.json
// file which is the sibling of the calling gen_tasks.go file. If cfg is nil, it
// is similarly loaded from a cfg.json file which is the sibling of the calling
// gen_tasks.go file.
func FormatJobsJSON(jobsFilePath string) []*JobInfo {
	var jobsWithInfo []*JobInfo
	LoadJSON(jobsFilePath, &jobsWithInfo)

	// Deduplicate jobs based on the "name" key.
	seen := make(map[string]bool)
	uniqueJobs := make([]*JobInfo, 0)
	for _, job := range jobsWithInfo {
		if _, ok := seen[job.Name]; !ok {
			seen[job.Name] = true
			uniqueJobs = append(uniqueJobs, job)
		}
	}
	jobsWithInfo = uniqueJobs

	// Sort the jobs by the "name" key.
	sort.Slice(jobsWithInfo, func(i, j int) bool {
		return jobsWithInfo[i].Name < jobsWithInfo[j].Name
	})

	// Pretty print and write back to jobs.json.
	updatedJobsJson, err := json.MarshalIndent(jobsWithInfo, "", "  ")
	if err != nil {
		log.Fatalf("Unable to marshal jobs.json: %s", err)
	}
	if err := os.WriteFile(jobsFilePath, updatedJobsJson, 0644); err != nil {
		log.Fatalf("Unable to write jobs.json: %s", err)
	}
	return jobsWithInfo
}

func GenTasks(cfg *Config) {
	b := specs.MustNewTasksCfgBuilder()

	// Find the paths to the infra/bots directories in this repo and the
	// repo of the calling file.
	relpathTargetDir := getThisDirName()
	relpathBaseDir := getCallingDirName()

	// Format and load jobs.json.
	jobsFilePath := filepath.Join(relpathBaseDir, "jobs.json")
	jobsWithInfo := FormatJobsJSON(jobsFilePath)
	// Create a slice with only job names.
	jobs := []string{}
	for _, j := range jobsWithInfo {
		jobs = append(jobs, j.Name)
	}

	if cfg == nil {
		cfg = new(Config)
		LoadJSON(filepath.Join(relpathBaseDir, "cfg.json"), cfg)
	}

	// Create the JobNameSchema.
	builderNameSchemaFile := filepath.Join(relpathTargetDir, "recipe_modules", "builder_name_schema", "builder_name_schema.json")
	if cfg.BuilderNameSchemaFile != "" {
		builderNameSchemaFile = filepath.Join(relpathBaseDir, cfg.BuilderNameSchemaFile)
	}
	schema, err := NewJobNameSchema(builderNameSchemaFile)
	if err != nil {
		log.Fatal(err)
	}

	// Set the assets dir.
	assetsDir := filepath.Join(relpathTargetDir, "assets")
	if cfg.AssetsDir != "" {
		assetsDir = filepath.Join(relpathBaseDir, cfg.AssetsDir)
	}
	b.SetAssetsDir(assetsDir)

	// Create Tasks and Jobs.
	builder := &builder{
		TasksCfgBuilder: b,
		cfg:             cfg,
		jobNameSchema:   schema,
		jobs:            jobs,
	}
	for _, j := range jobsWithInfo {
		jb := newJobBuilder(builder, j.Name)
		jb.genTasksForJob()
		jb.finish()

		// Add the CQ spec if it is a CQ job.
		if j.CQConfig != nil {
			b.MustAddCQJob(j.Name, j.CQConfig)
		}
	}

	// Create CasSpecs.
	b.MustAddCasSpec(CAS_BAZEL, &specs.CasSpec{
		Root: "..",
		Paths: []string{
			// Source code.
			"skia/example",
			"skia/include",
			"skia/modules",
			"skia/rust",
			"skia/src",
			"skia/tests",
			"skia/third_party",
			"skia/tools",
			// Needed for tests.
			"skia/bench", // Needed to run benchmark tests with Bazel.
			"skia/dm",    // Needed to run tests with Bazel.
			"skia/gm",    // Needed to run GMs with Bazel.
			"skia/gn",    // Some Python scripts still live here.
			"skia/resources",
			"skia/package.json",
			"skia/package-lock.json",
			"skia/DEPS",   // Needed to check generation.
			"skia/infra",  // Many Go tests and Bazel tools live here.
			"skia/go.mod", // Needed by Gazelle.
			"skia/go.sum", // Needed by Gazelle.
			// Needed to run Bazel.
			"skia/.bazelignore",
			"skia/.bazelrc",
			"skia/.bazelversion",
			"skia/BUILD.bazel",
			"skia/LICENSE", // Referred to by default_applicable_licenses
			"skia/MODULE.bazel",
			"skia/MODULE.bazel.lock",
			"skia/WORKSPACE.bazel",
			"skia/bazel",
			"skia/requirements.txt",
			"skia/toolchain",
		},
		Excludes: []string{
			rbe.ExcludeGitDir,
			"skia/third_party/externals",
		},
	})
	b.MustAddCasSpec(CAS_CANVASKIT, &specs.CasSpec{
		Root: "..",
		Paths: []string{
			"skia/.vpython3",
			"skia/infra/bots/run_recipe.py",
			"skia/infra/canvaskit",
			"skia/modules/canvaskit",
		},
		Excludes: []string{rbe.ExcludeGitDir},
	})
	b.MustAddCasSpec(CAS_EMPTY, specs.EmptyCasSpec)
	b.MustAddCasSpec(CAS_LOTTIE_CI, CAS_SPEC_LOTTIE_CI)
	b.MustAddCasSpec(CAS_LOTTIE_WEB, &specs.CasSpec{
		Root: "..",
		Paths: []string{
			"skia/.vpython3",
			"skia/infra/bots/run_recipe.py",
			"skia/tools/lottie-web-perf",
		},
		Excludes: []string{rbe.ExcludeGitDir},
	})
	b.MustAddCasSpec(CAS_PERF, &specs.CasSpec{
		Root: "..",
		Paths: []string{
			"skia/.vpython3",
			"skia/infra/bots/assets",
			"skia/infra/bots/run_recipe.py",
			"skia/platform_tools/ios/bin",
			"skia/resources",
		},
		Excludes: []string{rbe.ExcludeGitDir},
	})
	b.MustAddCasSpec(CAS_PUPPETEER, &specs.CasSpec{
		Root: "../skia", // Needed for other repos.
		Paths: []string{
			".vpython3",
			"tools/perf-canvaskit-puppeteer",
		},
		Excludes: []string{rbe.ExcludeGitDir},
	})
	b.MustAddCasSpec(CAS_RECIPES, &specs.CasSpec{
		Root: "..",
		Paths: []string{
			"skia/.vpython3",
			"skia/infra/config/recipes.cfg",
			"skia/infra/bots/bundle_recipes.sh",
			"skia/infra/bots/README.recipes.md",
			"skia/infra/bots/recipe_modules",
			"skia/infra/bots/recipes",
			"skia/infra/bots/recipes.py",
		},
		Excludes: []string{rbe.ExcludeGitDir},
	})
	b.MustAddCasSpec(CAS_RUN_RECIPE, &specs.CasSpec{
		Root: "..",
		Paths: []string{
			"skia/.vpython3",
			"skia/infra/bots/run_recipe.py",
		},
		Excludes: []string{rbe.ExcludeGitDir},
	})
	b.MustAddCasSpec(CAS_SKOTTIE_WASM, &specs.CasSpec{
		Root: "..",
		Paths: []string{
			"skia/.vpython3",
			"skia/infra/bots/run_recipe.py",
			"skia/tools/skottie-wasm-perf",
		},
		Excludes: []string{rbe.ExcludeGitDir},
	})
	b.MustAddCasSpec(CAS_TASK_DRIVERS, &specs.CasSpec{
		Root: "..",
		Paths: []string{
			// Deps needed to use Bazel
			"skia/.bazelrc",
			"skia/.bazelversion",
			"skia/BUILD.bazel",
			"skia/LICENSE",
			"skia/MODULE.bazel",
			"skia/MODULE.bazel.lock",
			"skia/WORKSPACE.bazel",
			"skia/bazel",
			"skia/go.mod",
			"skia/go.sum",
			"skia/include/config", // There's a Bazel workspace in here
			"skia/requirements.txt",
			"skia/toolchain",
			// Needed for icu_utils bazel_dep
			"skia/third_party",
			// Actually needed to build the task drivers
			"skia/infra/bots/BUILD.bazel",
			"skia/infra/bots/build_task_drivers.sh",
			"skia/infra/bots/task_drivers",
		},
		Excludes: []string{rbe.ExcludeGitDir},
	})
	b.MustAddCasSpec(CAS_TEST, &specs.CasSpec{
		Root: "..",
		Paths: []string{
			"skia/.vpython3",
			"skia/infra/bots/assets",
			"skia/infra/bots/run_recipe.py",
			"skia/platform_tools/ios/bin",
			"skia/resources",
		},
		Excludes: []string{rbe.ExcludeGitDir},
	})
	b.MustAddCasSpec(CAS_WASM_GM, &specs.CasSpec{
		Root: "../skia", // Needed for other repos.
		Paths: []string{
			".vpython3",
			"resources",
			"tools/run-wasm-gm-tests",
		},
		Excludes: []string{rbe.ExcludeGitDir},
	})
	b.MustAddCasSpec(CAS_WHOLE_REPO, CAS_SPEC_WHOLE_REPO)
	b.MustAddCasSpec(CAS_RECREATE_SKPS, &specs.CasSpec{
		Root: "..",
		Paths: []string{
			"skia/.vpython3",
			"skia/DEPS",
			"skia/bin/fetch-sk",
			"skia/infra/bots/assets/skp",
			"skia/infra/bots/utils.py",
			"skia/tools/skp",
		},
		Excludes: []string{rbe.ExcludeGitDir},
	})
	generateCompileCAS(b, cfg)

	builder.MustFinish()
}

// getThisDirName returns the infra/bots directory which is an ancestor of this
// file.
func getThisDirName() string {
	_, thisFileName, _, ok := runtime.Caller(0)
	if !ok {
		log.Fatal("Unable to find path to current file.")
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
		log.Fatal("Unable to find path to calling file.")
	}
	return filepath.Dir(callingFileName)
}

// builder is a wrapper for specs.TasksCfgBuilder.
type builder struct {
	*specs.TasksCfgBuilder
	cfg           *Config
	jobNameSchema *JobNameSchema
	jobs          []string
}

// marshalJson encodes the given data as JSON and fixes escaping of '<' which Go
// does by default.
func marshalJson(data interface{}) string {
	j, err := json.Marshal(data)
	if err != nil {
		log.Fatal(err)
	}
	return strings.Replace(string(j), "\\u003c", "<", -1)
}

// kitchenTaskNoBundle sets up the task to run a recipe via Kitchen, without the
// recipe bundle.
func (b *TaskBuilder) kitchenTaskNoBundle(recipe string, outputDir string) {
	b.usesLUCIAuth()
	b.cipd(cipd.MustGetPackage("infra/tools/luci/kitchen/${platform}"))
	b.env("RECIPES_USE_PY3", "true")
	b.envPrefixes("VPYTHON_DEFAULT_SPEC", "skia/.vpython3")
	b.usesPython()
	b.recipeProp("swarm_out_dir", outputDir)
	if outputDir != OUTPUT_NONE {
		b.output(outputDir)
	}
	const python = "cipd_bin_packages/vpython3${EXECUTABLE_SUFFIX}"
	b.cmd(python, "-u", "skia/infra/bots/run_recipe.py", "${ISOLATED_OUTDIR}", recipe, b.getRecipeProps(), b.cfg.Project)
	// Most recipes want this isolate; they can override if necessary.
	b.cas(CAS_RUN_RECIPE)
	b.timeout(time.Hour)
	b.Spec.ExtraTags = map[string]string{
		"log_location": fmt.Sprintf("logdog://logs.chromium.org/%s/${SWARMING_TASK_ID}/+/annotations", b.cfg.Project),
	}

	// Attempts.
	if !b.Role("Build", "Upload") && b.ExtraConfig("ASAN", "HWASAN", "MSAN", "TSAN") {
		// Sanitizers often find non-deterministic issues that retries would hide.
		b.attempts(1)
	} else {
		// Retry by default to hide random bot/hardware failures.
		b.attempts(2)
	}
}

// kitchenTask sets up the task to run a recipe via Kitchen.
func (b *TaskBuilder) kitchenTask(recipe string, outputDir string) {
	b.kitchenTaskNoBundle(recipe, outputDir)
	b.dep(b.bundleRecipes())
}

// internalHardwareLabel returns the internal ID for the bot, if any.
func (b *TaskBuilder) internalHardwareLabel() *int {
	if b.cfg.InternalHardwareLabel != nil {
		return b.cfg.InternalHardwareLabel(b.Parts)
	}
	return nil
}

// getLinuxGceDimensions returns a map of default Swarming bot dimensions for
// Linux GCE instances.
func (b *TaskBuilder) getLinuxGceDimensions(machineType string) map[string]string {
	return map[string]string{
		// Specify CPU to avoid running builds on bots with a more unique CPU.
		"cpu": "x86-64-Haswell_GCE",
		"gpu": "none",
		// Currently all Linux GCE tasks run on 16-CPU machines.
		"machine_type": machineType,
		"os":           DEFAULT_OS_LINUX_GCE,
		"pool":         b.cfg.Pool,
	}
}

// linuxGceDimensions adds the Swarming bot dimensions for Linux GCE instances.
func (b *TaskBuilder) linuxGceDimensions(machineType string) {
	dims := b.getLinuxGceDimensions(machineType)
	dimsSlice := make([]string, 0, len(dims))
	for k, v := range dims {
		dimsSlice = append(dimsSlice, fmt.Sprintf("%s:%s", k, v))
	}
	sort.Strings(dimsSlice)
	b.dimension(dimsSlice...)
}

// codesizeTaskNameRegexp captures the "CodeSize-<binary name>-" prefix of a CodeSize task name.
var codesizeTaskNameRegexp = regexp.MustCompile("^CodeSize-[a-zA-Z0-9_]+-")

// deriveCompileTaskName returns the name of a compile task based on the given
// job name.
func (b *jobBuilder) deriveCompileTaskName() string {
	if b.Role("Test", "Perf") {
		task_os := b.Parts["os"]
		ec := []string{}
		if val := b.Parts["extra_config"]; val != "" {
			ec = strings.Split(val, "_")
			ignore := []string{
				"AbandonGpuContext", "PreAbandonGpuContext",
				"FailFlushTimeCallbacks", "ReleaseAndAbandonGpuContext",
				"NativeFonts", "GDI", "NoGPUThreads", "DDL1", "DDL3",
				"DDLRecord", "BonusConfigs", "ColorSpaces", "GL",
				"SkottieTracing", "SkottieWASM", "GpuTess", "DMSAAStats", "Docker", "PDF",
				"Puppeteer", "SkottieFrames", "RenderSKP", "CanvasPerf", "AllPathsVolatile",
				"WebGL2", "i5", "OldestSupportedSkpVersion", "FakeWGPU", "Protected",
				"AndroidNDKFonts", "Upload", "TestPrecompile"}
			keep := make([]string, 0, len(ec))
			for _, part := range ec {
				if !In(part, ignore) {
					keep = append(keep, part)
				}
			}
			ec = keep
		}
		if b.MatchOs("Android") {
			if !In("Android", ec) {
				ec = append([]string{"Android"}, ec...)
			}
			task_os = DEFAULT_OS_LINUX_GCE
		} else if b.Os("ChromeOS") {
			ec = append([]string{"Chromebook", "GLES"}, ec...)
			task_os = UBUNTU_22_04_OS
		} else if b.MatchOs("iOS") {
			ec = append([]string{task_os}, ec...)
			task_os = "Mac"
		} else if b.MatchOs("Win") {
			task_os = "Win"
		} else if b.ExtraConfig("WasmGMTests") {
			task_os = DEFAULT_OS_LINUX_GCE
		} else if b.Compiler("GCC") {
			// GCC compiles are now on a Docker container. We use the same OS and
			// version to compile as to test.
			ec = append([]string{"Docker"}, ec...)
		} else if b.MatchOs("Mac") {
			task_os = "Mac"
		}
		jobNameMap := map[string]string{
			"role":          "Build",
			"os":            strings.ReplaceAll(task_os, "-", ""),
			"compiler":      b.Parts["compiler"],
			"target_arch":   b.Parts["arch"],
			"configuration": b.Parts["configuration"],
		}
		if b.ExtraConfig("CanvasKit", "SkottieWASM", "Puppeteer") {
			if b.CPU() {
				ec = []string{"CanvasKit_CPU"}
			} else {
				ec = []string{"CanvasKit"}
			}
			// We prefer to compile this in the cloud because we have more resources there
			jobNameMap["os"] = strings.ReplaceAll(DEFAULT_OS_LINUX_GCE, "-", "")
		}
		if len(ec) > 0 {
			jobNameMap["extra_config"] = strings.Join(ec, "_")
		}
		name, err := b.jobNameSchema.MakeJobName(jobNameMap)
		if err != nil {
			log.Fatal(err)
		}
		return name
	} else if b.Role("BuildStats") {
		return strings.Replace(b.Name, "BuildStats", "Build", 1)
	} else if b.Role("CodeSize") {
		return codesizeTaskNameRegexp.ReplaceAllString(b.Name, "Build-")
	} else {
		return b.Name
	}
}

// swarmDimensions generates swarming bot dimensions for the given task.
func (b *TaskBuilder) swarmDimensions() {
	if b.cfg.SwarmDimensions != nil {
		dims := b.cfg.SwarmDimensions(b.Parts)
		if dims != nil {
			b.dimension(dims...)
			return
		}
	}
	b.defaultSwarmDimensions()
}

// androidDeviceInfo maps Android models (as in the "model" part of a task) to the device_type and
// device_os Swarming dimensions.
var androidDeviceInfos = map[string][]string{
	"AndroidOne":      {"sprout", "MOB30Q"},
	"GalaxyS7_G930FD": {"herolte", "R16NW"}, // This is Oreo.
	"GalaxyS9":        {"exynos9810", "QP1A.190711.020"},
	"GalaxyS20":       {"exynos990", "QP1A.190711.020"},
	"GalaxyS24":       {"pineapple", "UP1A.231005.007"},
	"JioNext":         {"msm8937", "RKQ1.210602.002"},
	"Mokey":           {"mokey", "UP1A.231105.001"},
	"MokeyGo32":       {"mokey_go32", "UQ1A.240105.003.A1"},
	"MotoG73":         {"devonf", "U1TN34.82-12-17"},
	"Nexus5":          {"hammerhead", "M4B30Z_3437181"},
	"Nexus7":          {"grouper", "LMY47V"}, // 2012 Nexus 7
	"P30":             {"HWELE", "HUAWEIELE-L29"},
	"Pixel3a":         {"sargo", "QP1A.190711.020"},
	"Pixel4":          {"flame", "RPB2.200611.009"}, // R Preview
	"Pixel4a":         {"sunfish", "AOSP.MASTER"},   // Pixel4a flashed with an Android HWASan build.
	"Pixel4XL":        {"coral", "QD1A.190821.011.C4"},
	"Pixel5":          {"redfin", "RD1A.200810.022.A4"},
	"Pixel6":          {"oriole", "SD1A.210817.037"},
	"Pixel7":          {"panther", "AP4A.241205.013"},
	"Pixel7Pro":       {"cheetah", "TD1A.221105.002"},
	"Pixel9":          {"tokay", "AP4A.241205.013"},
	"Pixel10":         {"frankel", "BD1A.250702.001"},
	"TecnoSpark3Pro":  {"TECNO-KB8", "PPR1.180610.011"},
	"Wembley":         {"wembley", "SP2A.220505.008"},
}

// defaultSwarmDimensions generates default swarming bot dimensions for the given task.
func (b *TaskBuilder) defaultSwarmDimensions() {
	d := map[string]string{
		"pool": b.cfg.Pool,
	}
	if os, ok := b.Parts["os"]; ok {
		d["os"], ok = map[string]string{
			"Android":     "Android",
			"Android12":   "Android",
			"ChromeOS":    "ChromeOS",
			"Debian9":     DEFAULT_OS_LINUX_GCE, // Runs in Deb9 Docker.
			"Debian11":    DEBIAN_11_OS,
			"Mac":         DEFAULT_OS_MAC,
			"Mac11":       "Mac-11",
			"Mac12":       "Mac-12",
			"Mac13":       "Mac-13",
			"Mac14":       "Mac-14.7", // Builds run on 14.5, tests on 14.7.
			"Mac15":       "Mac-15.3",
			"Mokey":       "Android",
			"MokeyGo32":   "Android",
			"Ubuntu20.04": UBUNTU_20_04_OS,
			"Ubuntu22.04": UBUNTU_22_04_OS,
			"Ubuntu24.04": UBUNTU_24_04_OS,
			"Win":         DEFAULT_OS_WIN_GCE,
			"Win10":       "Windows-10-19045",
			"Win11":       "Windows-11-26100.1742",
			"iOS":         "iOS-13.3.1",
			"iOS18":       "iOS-18.2.1",
		}[os]
		if !ok {
			log.Fatalf("Entry %q not found in OS mapping.", os)
		}
		if os == "Debian11" && b.ExtraConfig("Docker") {
			d["os"] = DEFAULT_OS_LINUX_GCE
			d["gce"] = "1"
		}
		if os == "Win11" && b.Model("GCE") {
			d["os"] = DEFAULT_OS_WIN_GCE
			d["gce"] = "1"
		}
		if strings.Contains(os, "iOS") {
			d["pool"] = "SkiaIOS"
			if b.Model("iPhone11") {
				d["os"] = "iOS-18.4"
			}
		}
		if b.Parts["model"] == "iPadPro" {
			d["os"] = "iOS-13.6"
		}
	} else {
		d["os"] = DEFAULT_OS_LINUX_GCE
	}
	if b.Role("Test", "Perf") {
		if b.Os("Android") {
			// For Android, the device type is a better dimension
			// than CPU or GPU.
			deviceInfo, ok := androidDeviceInfos[b.Parts["model"]]
			if !ok {
				log.Fatalf("Entry %q not found in Android mapping.", b.Parts["model"])
			}
			d["device_type"] = deviceInfo[0]
			d["device_os"] = deviceInfo[1]
		} else if b.Os("Android12") {
			// For Android, the device type is a better dimension
			// than CPU or GPU.
			deviceInfo, ok := map[string][]string{
				"Pixel5": {"redfin", "SP2A.220305.012"},
			}[b.Parts["model"]]
			if !ok {
				log.Fatalf("Entry %q not found in Android mapping.", b.Parts["model"])
			}
			d["device_type"] = deviceInfo[0]
			d["device_os"] = deviceInfo[1]

			// Tests using Android's HWAddress Sanitizer require an HWASan build of Android.
			// See https://developer.android.com/ndk/guides/hwasan.
			if b.ExtraConfig("HWASAN") {
				d["android_hwasan_build"] = "1"
			}
		} else if b.Os("ChromeOS") {
			deviceOS, ok := map[string]string{
				"Cherry":   "16002.30.0",
				"Guybrush": "16002.27.0",
				"Octopus":  "16002.21.0",
				"Trogdor":  "16002.26.0",
			}[b.Parts["model"]]
			if !ok {
				log.Fatalf("Entry %q not found in ChromeOS mapping.", b.Parts["model"])
			}
			d["device_os"] = deviceOS
			d["device_type"] = strings.ToLower(b.Parts["model"])
		} else if b.MatchOs("iOS") {
			device, ok := map[string]string{
				"iPadMini4":   "iPad5,1",
				"iPhone11":    "iPhone12,1",
				"iPhone15Pro": "iPhone16,1",
				"iPhone7":     "iPhone9,1",
				"iPhone8":     "iPhone10,1",
				"iPadPro":     "iPad6,3",
			}[b.Parts["model"]]
			if !ok {
				log.Fatalf("Entry %q not found in iOS mapping.", b.Parts["model"])
			}
			d["device"] = device
		} else if b.CPU() || b.ExtraConfig("CanvasKit", "Docker", "SwiftShader") {
			modelMapping, ok := map[string]map[string]map[string]string{
				"AppleM1": {
					"MacMini9.1": {"cpu": "arm64-64-Apple_M1"},
				},
				"AppleM3": {
					"MacBookPro15.3": {"cpu": "arm64-64-Apple_M3"},
				},
				"AppleIntel": {
					"MacBookPro15.1": {"cpu": "x86-64"},
					"MacBookPro16.2": {"cpu": "x86-64"},
				},
				"AVX": {
					"VMware7.1": {"cpu": "x86-64"},
				},
				"AVX2": {
					"GCE":            {"cpu": "x86-64-Haswell_GCE"},
					"Golo":           {"cpu": "x86-64-E3-1230_v5"},
					"MacBookAir7.2":  {"cpu": "x86-64-i5-5350U"},
					"MacBookPro11.5": {"cpu": "x86-64-i7-4870HQ"},
					"MacMini7.1":     {"cpu": "x86-64-i5-4278U"},
					"MacMini8.1":     {"cpu": "x86-64-i7-8700B"},
					"NUC5i7RYH":      {"cpu": "x86-64-i7-5557U"},
					"NUC9i7QN":       {"cpu": "x86-64-i7-9750H"},
					// Unfortunately, these machines don't have a more-specific
					// CPU dimension we can use. However, they do have integrated
					// GPUs whose models differ from our other machines, so we
					// specify the GPU dimension even when running CPU tests.
					"NUC11TZi5": {"cpu": "x86-64", "gpu": "8086:9a49"},
				},
				"AVX512": {
					"GCE":  {"cpu": "x86-64-Skylake_GCE"},
					"Golo": {"cpu": "Intel64_Family_6_Model_85_Stepping_7__GenuineIntel"},
				},
				"Rome": {
					"GCE": {"cpu": "x86-64"},
				},
				"SwiftShader": {
					"GCE": {"cpu": "x86-64-Haswell_GCE"},
				},
			}[b.Parts["cpu_or_gpu_value"]]
			if !ok {
				log.Fatalf("Entry %q not found in CPU mapping.", b.Parts["cpu_or_gpu_value"])
			}
			dims, ok := modelMapping[b.Parts["model"]]
			if !ok {
				log.Fatalf("Entry %q not found in %q model mapping.", b.Parts["model"], b.Parts["cpu_or_gpu_value"])
			}
			for k, v := range dims {
				d[k] = v
			}
			if b.Model("GCE") && b.MatchOs("Debian") {
				d["os"] = DEFAULT_OS_LINUX_GCE
			}
			if b.Model("GCE") && d["cpu"] == "x86-64-Haswell_GCE" {
				d["machine_type"] = MACHINE_TYPE_MEDIUM
			}
			if b.Model("GCE") && b.CPU("Rome") {
				d["machine_type"] = "n2d-standard-16"
			}
		} else {
			// It's a GPU job.
			if b.MatchOs("Win") {
				gpu, ok := map[string]string{
					"GTX1660":       "10de:2184-31.0.15.4601",
					"IntelHD4400":   "8086:0a16-10.0.26100.1",
					"IntelIris540":  "8086:1926-26.20.100.7528",
					"IntelIris6100": "8086:162b-20.19.15.5171",
					"IntelIris655":  "8086:3ea5-26.20.100.7463",
					"IntelIrisXe":   "8086:9a49-31.0.101.5333",
					"RadeonHD7770":  "1002:683d-26.20.13031.18002",
					"RadeonR9M470X": "1002:6646-21.19.136.0",
					"QuadroP400":    "10de:1cb3-31.0.15.5222",
					"RadeonVega6":   "1002:1636-31.0.14057.5006",
					"RadeonVega8":   "1002:1638-31.0.21916.2",
					"RTX3060":       "10de:2489-32.0.15.7270",
				}[b.Parts["cpu_or_gpu_value"]]
				if !ok {
					log.Fatalf("Entry %q not found in Win GPU mapping.", b.Parts["cpu_or_gpu_value"])
				}
				// TODO(borenet): Remove this block once these machines are all
				// migrated.
				if b.Os("Win10") && b.Parts["cpu_or_gpu_value"] == "RTX3060" {
					gpu = "10de:2489-32.0.15.6094"
				}
				d["gpu"] = gpu
			} else if b.IsLinux() {
				gpu, ok := map[string]string{
					// Intel drivers come from CIPD, so no need to specify the version here.
					"IntelHD405": "8086:22b1",
					// The version is not set on these as of Nov 5 2025
					"QuadroP400":  "10de:1cb3",
					"IntelIrisXe": "8086:9a49",
					"RadeonVega8": "1002:1638-23.2.1",
				}[b.Parts["cpu_or_gpu_value"]]
				if !ok {
					log.Fatalf("Entry %q not found in Linux GPU mapping.", b.Parts["cpu_or_gpu_value"])
				}
				d["gpu"] = gpu
			} else if b.MatchOs("Mac") {
				gpu, ok := map[string]string{
					"AppleM1":             "AppleM1",
					"AppleM3":             "apple:m3",
					"IntelHD6000":         "8086:1626",
					"IntelHD615":          "8086:591e",
					"IntelIris5100":       "8086:0a2e",
					"IntelIrisPlus":       "8086:8a53",
					"IntelUHDGraphics630": "8086:3e9b",
					"RadeonHD8870M":       "1002:6821-4.0.20-3.2.8",
				}[b.Parts["cpu_or_gpu_value"]]
				if !ok {
					log.Fatalf("Entry %q not found in Mac GPU mapping.", b.Parts["cpu_or_gpu_value"])
				}
				if gpu == "AppleM1" {
					// No GPU dimension yet, but we can constrain by CPU.
					d["cpu"] = "arm64-64-Apple_M1"
				} else {
					d["gpu"] = gpu
				}
				// We have two different types of MacMini7,1 with the same GPU but different CPUs.
				if b.GPU("IntelIris5100") {
					if b.ExtraConfig("i5") {
						// If we say "i5", run on our MacMini7,1s in the Skolo:
						d["cpu"] = "x86-64-i5-4278U"
					} else {
						// Otherwise, run on Golo machines, just because that's
						// where those jobs have always run. Plus, some of them
						// are Perf jobs, which we want to keep consistent.
						d["cpu"] = "x86-64-i7-4578U"
					}
				}
			} else {
				log.Fatalf("Unknown GPU mapping for OS %q.", b.Parts["os"])
			}
		}
		if b.MatchOs("Mac") {
			// TODO(borenet): Remove empty and nested entries after all Macs
			// are migrated to the new lab.
			if macModel, ok := map[string]interface{}{
				"MacBookAir7.2":  "",
				"MacBookPro11.5": "MacBookPro11,5",
				"MacBookPro15.1": "MacBookPro15,1",
				"MacBookPro15.3": "Mac15,3",
				"MacBookPro16.2": "",
				"MacMini7.1":     "",
				"MacMini8.1":     "Macmini8,1",
				"MacMini9.1": map[string]string{
					"Mac12": "",
					"Mac13": "",
					"Mac14": "Macmini9,1",
				},
				// TODO(borenet): This is currently resolving to multiple
				// different actual device types.
				"VMware7.1": "",
			}[b.Parts["model"]]; ok {
				if macModel != "" {
					macModelDim, ok := macModel.(string)
					if !ok {
						macModelDim = macModel.(map[string]string)[b.Parts["os"]]
					}
					if macModelDim != "" {
						d["mac_model"] = macModelDim
					}
				}
			} else {
				log.Fatalf("No mac_model found for %q", b.Parts["model"])
			}
		}
	} else {
		d["gpu"] = "none"
		if d["os"] == DEFAULT_OS_LINUX_GCE {
			if b.ExtraConfig("CanvasKit", "CMake", "Docker") || b.Role("BuildStats", "CodeSize") {
				b.linuxGceDimensions(MACHINE_TYPE_MEDIUM)
			} else {
				// Use many-core machines for Build tasks.
				b.linuxGceDimensions(MACHINE_TYPE_LARGE)
			}
		} else if d["os"] == DEFAULT_OS_WIN_GCE {
			// Windows CPU bots.
			d["cpu"] = "x86-64-Haswell_GCE"
			d["gce"] = "1"
			// Use many-core machines for Build tasks.
			d["machine_type"] = MACHINE_TYPE_LARGE
		} else if d["os"] == DEFAULT_OS_MAC {
			// Mac CPU bots are no longer VMs.
			d["cpu"] = "x86-64"
			d["cores"] = "12"
			delete(d, "gpu")
		}
	}

	dims := make([]string, 0, len(d))
	for k, v := range d {
		dims = append(dims, fmt.Sprintf("%s:%s", k, v))
	}
	sort.Strings(dims)
	b.dimension(dims...)
}

// bundleRecipes generates the task to bundle and isolate the recipes. Returns
// the name of the task, which may be added as a dependency.
func (b *jobBuilder) bundleRecipes() string {
	b.addTask(BUNDLE_RECIPES_NAME, func(b *TaskBuilder) {
		b.usesGit()
		b.cmd("/bin/bash", "skia/infra/bots/bundle_recipes.sh", specs.PLACEHOLDER_ISOLATED_OUTDIR)
		b.linuxGceDimensions(MACHINE_TYPE_SMALL)
		b.idempotent()
		b.cas(CAS_RECIPES)
		b.usesPython()
	})
	return BUNDLE_RECIPES_NAME
}

// buildTaskDrivers generates the task to compile the task driver code to run on
// all platforms. Returns the name of the task, which may be added as a
// dependency.
func (b *jobBuilder) buildTaskDrivers(goos, goarch string) string {
	name := BUILD_TASK_DRIVERS_PREFIX + "_" + goos + "_" + goarch
	b.addTask(name, func(b *TaskBuilder) {
		b.cmd(
			"luci-auth", "context",
			"/bin/bash", "skia/infra/bots/build_task_drivers.sh",
			specs.PLACEHOLDER_ISOLATED_OUTDIR,
			goos+"_"+goarch)
		b.linuxGceDimensions(MACHINE_TYPE_MEDIUM)
		b.usesBazel("linux_x64")
		b.usesLUCIAuth()
		b.idempotent()
		b.cas(CAS_TASK_DRIVERS)
		b.serviceAccount(b.cfg.ServiceAccountCompile)
	})
	return name
}

var iosRegex = regexp.MustCompile(`os:iOS-(.*)`)

func (b *TaskBuilder) maybeAddIosDevImage() {
	for _, dim := range b.Spec.Dimensions {
		if m := iosRegex.FindStringSubmatch(dim); len(m) >= 2 {
			var asset string
			switch m[1] {
			// Other patch versions can be added to the same case.
			case "11.4.1":
				asset = "ios-dev-image-11.4"
			case "13.3.1":
				asset = "ios-dev-image-13.3"
			case "13.4.1":
				asset = "ios-dev-image-13.4"
			case "13.5.1":
				asset = "ios-dev-image-13.5"
			case "13.6":
				asset = "ios-dev-image-13.6"
			case "18.2.1", "18.4":
				// Newer iOS versions don't use a pre-packaged dev image.
			default:
				log.Fatalf("Unable to determine correct ios-dev-image asset for %s. If %s is a new iOS release, you must add a CIPD package containing the corresponding iOS dev image; see ios-dev-image-11.4 for an example.", b.Name, m[1])
			}
			if asset != "" {
				b.asset(asset)
			}
			break
		} else if strings.Contains(dim, "iOS") {
			log.Fatalf("Must specify iOS version for %s to obtain correct dev image; os dimension is missing version: %s", b.Name, dim)
		}
	}
}

// compile generates a compile task. Returns the name of the compile task.
func (b *jobBuilder) compile() string {
	name := b.deriveCompileTaskName()
	if b.ExtraConfig("WasmGMTests") {
		b.compileWasmGMTests(name)
	} else {
		b.addTask(name, func(b *TaskBuilder) {
			recipe := "compile"
			casSpec := CAS_COMPILE
			if b.ExtraConfig("NoDEPS", "CMake", "Flutter", "NoPatch") || b.shellsOutToBazel() {
				recipe = "sync_and_compile"
				casSpec = CAS_RUN_RECIPE
				b.recipeProps(EXTRA_PROPS)
				b.usesGit()
				if !b.ExtraConfig("NoDEPS") {
					b.cache(CACHES_WORKDIR...)
				}
			} else {
				b.idempotent()
			}
			if b.ExtraConfig("NoPatch") {
				b.kitchenTask(recipe, OUTPUT_BUILD_NOPATCH)
			} else {
				b.kitchenTask(recipe, OUTPUT_BUILD)
			}
			b.cas(casSpec)
			b.serviceAccount(b.cfg.ServiceAccountCompile)
			b.swarmDimensions()
			if b.ExtraConfig("Docker", "LottieWeb", "CMake") || b.Compiler("EMCC") {
				b.usesDocker()
				b.cache(CACHES_DOCKER...)
			}
			if b.ExtraConfig("Dawn") {
				// https://dawn.googlesource.com/dawn/+/516701da8184655a47c92a573cc84da7db5e69d4/generator/dawn_version_generator.py#21
				b.usesGit()
				b.usesCMake()
			}

			// Android bots require a toolchain.
			if b.ExtraConfig("Android") {
				if b.MatchOs("Mac") {
					b.asset("android_ndk_darwin")
				} else if b.MatchOs("Win") {
					pkg := b.MustGetCipdPackageFromAsset("android_ndk_windows")
					pkg.Path = "n"
					b.cipd(pkg)
				} else {
					b.asset("android_ndk_linux")
				}
			} else if b.ExtraConfig("Chromebook") {
				b.asset("clang_linux")
				if b.Arch("x86_64") {
					b.asset("chromebook_x86_64_gles")
				} else if b.Arch("arm") {
					b.asset("armhf_sysroot")
					b.asset("chromebook_arm_gles")
				} else if b.Arch("arm64") {
					b.asset("arm64_sysroot")
					b.asset("chromebook_arm64_gles")
				} else {
					panic(fmt.Sprintf("Unknown arch %q for Chromebook", b.Parts["arch"]))
				}
			} else if b.IsLinux() {
				if b.Compiler("Clang") {
					if b.ExtraConfig("MSAN") {
						b.asset("clang_ubuntu_noble")
					} else {
						b.asset("clang_linux")
					}
				}
				if b.ExtraConfig("SwiftShader") {
					b.usesCMake()
				}
				b.asset("ccache_linux")
				b.usesCCache()
				if b.shellsOutToBazel() {
					b.usesBazel("linux_x64")
					b.attempts(1)
				}
			} else if b.MatchOs("Win") {
				b.asset("win_toolchain")
				if b.Compiler("Clang") {
					b.asset("clang_win")
				}
				if b.ExtraConfig("DWriteCore") {
					b.asset("dwritecore")
				}
			} else if b.MatchOs("Mac") {
				b.cipd(CIPD_PKGS_XCODE...)
				b.Spec.Caches = append(b.Spec.Caches, &specs.Cache{
					Name: "xcode",
					Path: "cache/Xcode.app",
				})
				b.asset("ccache_mac")
				b.usesCCache()
				if b.MatchExtraConfig("iOS.*") {
					b.asset("provisioning_profile_ios")
				}
				if b.shellsOutToBazel() {
					// All of our current Mac compile machines are x64 Mac only.
					b.usesBazel("mac_x64")
					b.attempts(1)
				}
			}
		})
	}

	// All compile tasks are runnable as their own Job. Assert that the Job
	// is listed in jobs.
	if !In(name, b.jobs) {
		log.Fatalf("Job %q is missing from the jobs list! Derived from: %q", name, b.Name)
	}

	return name
}

// recreateSKPs generates a RecreateSKPs task.
func (b *jobBuilder) recreateSKPs() {
	b.addTask(b.Name, func(b *TaskBuilder) {
		cmd := []string{
			b.taskDriver("recreate_skps", false),
			"--local=false",
			"--project_id", "skia-swarming-bots",
			"--task_id", specs.PLACEHOLDER_TASK_ID,
			"--task_name", b.Name,
			"--skia_revision", specs.PLACEHOLDER_REVISION,
			"--patch_ref", specs.PLACEHOLDER_PATCH_REF,
			"--git_cache", "cache/git",
			"--checkout_root", "cache/work",
			"--dm_path", "build/dm",
		}
		if b.MatchExtraConfig("DryRun") {
			cmd = append(cmd, "--dry_run")
		}

		b.cas(CAS_RECREATE_SKPS)
		// We use a build task To get DM.
		b.dep("Build-Ubuntu24.04-Clang-x86_64-Release")
		b.cmd(cmd...)
		b.usesLUCIAuth()
		b.serviceAccount(b.cfg.ServiceAccountRecreateSKPs)
		b.dimension(
			"pool:Skia",
			"cpu:x86-64-Haswell_GCE",
			// TODO(borenet): It'd be much faster to run this on n1-highcpu-64,
			// for which we do have have capacity, but the task gets OOM-killed
			// while building Chrome, presumably because the ratio of RAM to CPU
			// cores is too low.
			"machine_type:n1-standard-16",
			"gce:1",
			fmt.Sprintf("os:%s", DEFAULT_OS_LINUX_GCE),
		)
		b.usesGo()
		b.cache(CACHES_WORKDIR...)
		b.timeout(8 * time.Hour)
		b.usesPython()
		b.attempts(2)
	})
}

// checkGeneratedFiles verifies that no generated SKSL files have been edited by hand, and that
// we do not get any diffs after regenerating all files (go generate, Gazelle, etc.).
func (b *jobBuilder) checkGeneratedFiles() {
	b.addTask(b.Name, func(b *TaskBuilder) {
		b.cas(CAS_BAZEL)
		b.cmd(
			"luci-auth", "context",
			b.taskDriver("check_generated_files", false),
			"--local=false",
			"--git_path=cipd_bin_packages/git",
			"--project_id", "skia-swarming-bots",
			"--task_id", specs.PLACEHOLDER_TASK_ID,
			"--task_name", b.Name,
			"--bazel_cache_dir", bazelCacheDirOnGCELinux,
			"--bazel_arg=--config=for_linux_x64_with_rbe",
			"--bazel_arg=--jobs=100",
		)
		b.usesBazel("linux_x64")
		b.usesGit()
		b.usesLUCIAuth()
		b.linuxGceDimensions(MACHINE_TYPE_MEDIUM)
		b.serviceAccount(b.cfg.ServiceAccountHousekeeper)
	})
}

// goLinters runs various Go linters (gofmt, errcheck, etc.) and fails if there are any errors or
// diffs.
func (b *jobBuilder) goLinters() {
	b.addTask(b.Name, func(b *TaskBuilder) {
		b.cas(CAS_BAZEL)
		b.cmd(
			"luci-auth", "context",
			b.taskDriver("go_linters", false),
			"--local=false",
			"--git_path=cipd_bin_packages/git",
			"--project_id", "skia-swarming-bots",
			"--task_id", specs.PLACEHOLDER_TASK_ID,
			"--task_name", b.Name,
			"--bazel_cache_dir", bazelCacheDirOnGCELinux,
			"--bazel_arg=--config=for_linux_x64_with_rbe",
			"--bazel_arg=--jobs=100",
		)
		b.usesBazel("linux_x64")
		b.usesGit()
		b.usesLUCIAuth()
		b.linuxGceDimensions(MACHINE_TYPE_MEDIUM)
		b.serviceAccount(b.cfg.ServiceAccountHousekeeper)
	})
}

// checkGnToBp verifies that the gn_to_bp.py script continues to work.
func (b *jobBuilder) checkGnToBp() {
	b.addTask(b.Name, func(b *TaskBuilder) {
		b.cas(CAS_COMPILE)
		b.cmd(
			b.taskDriver("run_gn_to_bp", false),
			"--local=false",
			"--project_id", "skia-swarming-bots",
			"--task_id", specs.PLACEHOLDER_TASK_ID,
			"--task_name", b.Name,
		)
		b.linuxGceDimensions(MACHINE_TYPE_SMALL)
		b.usesPython()
		b.serviceAccount(b.cfg.ServiceAccountHousekeeper)
	})
}

// housekeeper generates a Housekeeper task.
func (b *jobBuilder) housekeeper() {
	b.addTask(b.Name, func(b *TaskBuilder) {
		b.recipeProps(EXTRA_PROPS)
		b.kitchenTask("housekeeper", OUTPUT_NONE)
		b.serviceAccount(b.cfg.ServiceAccountHousekeeper)
		b.linuxGceDimensions(MACHINE_TYPE_SMALL)
		b.usesGit()
		b.cache(CACHES_WORKDIR...)
	})
}

// g3FrameworkCanary generates a G3 Framework Canary task. Returns
// the name of the last task in the generated chain of tasks, which the Job
// should add as a dependency.
func (b *jobBuilder) g3FrameworkCanary() {
	b.addTask(b.Name, func(b *TaskBuilder) {
		b.cas(CAS_EMPTY)
		b.cmd(
			b.taskDriver("g3_canary", false),
			"--local=false",
			"--project_id", "skia-swarming-bots",
			"--task_id", specs.PLACEHOLDER_TASK_ID,
			"--task_name", b.Name,
			"--repo", specs.PLACEHOLDER_REPO,
			"--revision", specs.PLACEHOLDER_REVISION,
			"--patch_issue", specs.PLACEHOLDER_ISSUE,
			"--patch_set", specs.PLACEHOLDER_PATCHSET,
			"--patch_server", specs.PLACEHOLDER_CODEREVIEW_SERVER,
		)
		b.linuxGceDimensions(MACHINE_TYPE_SMALL)
		b.usesLUCIAuth()
		b.serviceAccount("skia-g3-framework-compile@skia-swarming-bots.iam.gserviceaccount.com")
		b.timeout(3 * time.Hour)
		b.attempts(1)
	})
}

// infra generates an infra_tests task.
func (b *jobBuilder) infra() {
	b.addTask(b.Name, func(b *TaskBuilder) {
		if b.MatchOs("Win") || b.MatchExtraConfig("Win") {
			b.dimension(
				// Specify CPU to avoid running builds on bots with a more unique CPU.
				"cpu:x86-64-Haswell_GCE",
				"gpu:none",
				fmt.Sprintf("machine_type:%s", MACHINE_TYPE_MEDIUM), // We don't have any small Windows instances.
				fmt.Sprintf("os:%s", DEFAULT_OS_WIN_GCE),
				fmt.Sprintf("pool:%s", b.cfg.Pool),
			)
		} else {
			b.linuxGceDimensions(MACHINE_TYPE_SMALL)
		}
		b.recipeProp("repository", specs.PLACEHOLDER_REPO)
		b.kitchenTask("infra", OUTPUT_NONE)
		b.cas(CAS_WHOLE_REPO)
		b.serviceAccount(b.cfg.ServiceAccountCompile)
		b.usesGSUtil()
		b.idempotent()
		b.usesGo()
	})
}

// buildstats generates a builtstats task, which compiles code and generates
// statistics about the build.
func (b *jobBuilder) buildstats() {
	compileTaskName := b.compile()
	b.addTask(b.Name, func(b *TaskBuilder) {
		b.recipeProps(EXTRA_PROPS)
		b.kitchenTask("compute_buildstats", OUTPUT_PERF)
		b.dep(compileTaskName)
		b.asset("bloaty")
		b.linuxGceDimensions(MACHINE_TYPE_MEDIUM)
		b.usesDocker()
		b.usesGit()
		b.cache(CACHES_WORKDIR...)
	})
	// Upload release results (for tracking in perf)
	// We have some jobs that are FYI (e.g. Debug-CanvasKit, tree-map generator)
	if b.Release() && !b.Arch("x86_64") {
		uploadName := fmt.Sprintf("%s%s%s", PREFIX_UPLOAD, b.jobNameSchema.Sep, b.Name)
		depName := b.Name
		b.addTask(uploadName, func(b *TaskBuilder) {
			b.recipeProp("gs_bucket", b.cfg.GsBucketNano)
			b.recipeProps(EXTRA_PROPS)
			// TODO(borenet): I'm not sure why the upload task is
			// using the BuildStats task name, but I've done this
			// to maintain existing behavior.
			b.Name = depName
			b.kitchenTask("upload_buildstats_results", OUTPUT_NONE)
			b.Name = uploadName
			b.serviceAccount(b.cfg.ServiceAccountUploadNano)
			b.linuxGceDimensions(MACHINE_TYPE_SMALL)
			b.usesGSUtil()
			b.dep(depName)
		})
	}
}

// codesize generates a codesize task, which takes binary produced by a
// compile task, runs Bloaty against it, and uploads the resulting code size
// statistics to the GCS bucket belonging to the codesize.skia.org service.
func (b *jobBuilder) codesize() {
	compileTaskName := b.compile()
	compileTaskNameNoPatch := compileTaskName
	if b.ExtraConfig("Android") {
		compileTaskNameNoPatch += "_NoPatch" // add a second "extra config"
	} else {
		compileTaskNameNoPatch += "-NoPatch" // add the only "extra config"
	}

	bloatyCipdPkg := b.MustGetCipdPackageFromAsset("bloaty")

	b.addTask(b.Name, func(b *TaskBuilder) {
		b.cas(CAS_EMPTY)
		b.dep(compileTaskName)
		b.dep(compileTaskNameNoPatch)
		cmd := []string{
			b.taskDriver("codesize", false),
			"--local=false",
			"--project_id", "skia-swarming-bots",
			"--task_id", specs.PLACEHOLDER_TASK_ID,
			"--task_name", b.Name,
			"--compile_task_name", compileTaskName,
			"--compile_task_name_no_patch", compileTaskNameNoPatch,
			// Note: the binary name cannot contain dashes, otherwise the naming
			// schema logic will partition it into multiple parts.
			//
			// If we ever need to define a CodeSize-* task for a binary with
			// dashes in its name (e.g. "my-binary"), a potential workaround is to
			// create a mapping from a new, non-dashed binary name (e.g. "my_binary")
			// to the actual binary name with dashes. This mapping can be hardcoded
			// in this function; no changes to the task driver would be necessary.
			"--binary_name", b.Parts["binary_name"],
			"--bloaty_cipd_version", bloatyCipdPkg.Version,
			"--bloaty_binary", "bloaty/bloaty",

			"--repo", specs.PLACEHOLDER_REPO,
			"--revision", specs.PLACEHOLDER_REVISION,
			"--patch_issue", specs.PLACEHOLDER_ISSUE,
			"--patch_set", specs.PLACEHOLDER_PATCHSET,
			"--patch_server", specs.PLACEHOLDER_CODEREVIEW_SERVER,
		}
		if strings.Contains(compileTaskName, "Android") {
			b.asset("android_ndk_linux")
			cmd = append(cmd, "--strip_binary",
				"android_ndk_linux/toolchains/llvm/prebuilt/linux-x86_64/bin/llvm-strip")
		} else {
			cmd = append(cmd, "--strip_binary", "/usr/bin/strip")
		}
		b.cmd(cmd...)
		b.linuxGceDimensions(MACHINE_TYPE_SMALL)
		b.cache(CACHES_WORKDIR...)
		b.usesLUCIAuth()
		b.asset("bloaty")
		b.serviceAccount("skia-external-codesize@skia-swarming-bots.iam.gserviceaccount.com")
		b.timeout(20 * time.Minute)
		b.attempts(1)
	})
}

// doUpload indicates whether the given Job should upload its results.
func (b *jobBuilder) doUpload() bool {
	if b.ExtraConfig("Upload") {
		return true
	}
	for _, s := range b.cfg.NoUpload {
		m, err := regexp.MatchString(s, b.Name)
		if err != nil {
			log.Fatal(err)
		}
		if m {
			return false
		}
	}
	return true
}

// commonTestPerfAssets adds the assets needed by Test and Perf tasks.
func (b *TaskBuilder) commonTestPerfAssets() {
	// Docker-based tests don't need the standard CIPD assets
	if b.ExtraConfig("CanvasKit") || (b.Role("Test") && b.ExtraConfig("LottieWeb")) {
		return
	}
	if b.Os("Android", "ChromeOS", "iOS") {
		b.asset("skp", "svg", "skimage")
	} else if b.ExtraConfig("OldestSupportedSkpVersion") {
		b.cipd(&specs.CipdPackage{
			Name:    "skia/bots/skp",
			Path:    "skp",
			Version: fmt.Sprintf("skp_min_version:%d", oldestSupportedSkpVersion),
		})
	} else {
		// for desktop machines
		b.asset("skimage", "skp", "svg")
	}

	if b.IsLinux() && b.MatchExtraConfig("SAN") {
		if b.ExtraConfig("MSAN") {
			b.asset("clang_ubuntu_noble")
		} else {
			b.asset("clang_linux")
		}
	}

	if b.IsLinux() {
		if b.ExtraConfig("Vulkan") {
			b.asset("linux_vulkan_sdk")
		}
		if b.MatchGpu("Intel") {
			if b.MatchGpu("IrisXe") {
				b.asset("mesa_intel_driver_linux_22")
			} else {
				// Use this for legacy drivers that were culled in v22 of Mesa.
				// https://www.phoronix.com/scan.php?page=news_item&px=Mesa-22.0-Drops-OpenSWR
				b.asset("mesa_intel_driver_linux")
			}
		}
	}

	if b.MatchOs("Win") && b.ExtraConfig("DWriteCore") {
		b.asset("dwritecore")
	}
}

// directUpload adds prerequisites for uploading to GCS.
func (b *TaskBuilder) directUpload(gsBucket, serviceAccount string) {
	b.recipeProp("gs_bucket", gsBucket)
	b.serviceAccount(serviceAccount)
	b.usesGSUtil()
}

// dm generates a Test task using dm.
func (b *jobBuilder) dm() {
	compileTaskName := ""
	// LottieWeb doesn't require anything in Skia to be compiled.
	if !b.ExtraConfig("LottieWeb") {
		compileTaskName = b.compile()
	}
	directUpload := false
	b.addTask(b.Name, func(b *TaskBuilder) {
		cas := CAS_TEST
		recipe := "test"
		if b.ExtraConfig("CanvasKit") {
			cas = CAS_CANVASKIT
			recipe = "test_canvaskit"
			if b.doUpload() {
				b.directUpload(b.cfg.GsBucketGm, b.cfg.ServiceAccountUploadGM)
				directUpload = true
			}
		} else if b.ExtraConfig("LottieWeb") {
			// CAS_LOTTIE_CI differs from CAS_LOTTIE_WEB in that it includes
			// more of the files, especially those brought in via DEPS in the
			// lottie-ci repo. The main difference between Perf.+LottieWeb and
			// Test.+LottieWeb is that the former pulls in the lottie build via
			// npm and the latter always tests at lottie's
			// ToT.
			cas = CAS_LOTTIE_CI
			recipe = "test_lottie_web"
			if b.doUpload() {
				b.directUpload(b.cfg.GsBucketGm, b.cfg.ServiceAccountUploadGM)
				directUpload = true
			}
		} else {
			// Default recipe supports direct upload.
			// TODO(skbug.com/40042855): Windows jobs are unable to extract gsutil.
			// https://bugs.chromium.org/p/chromium/issues/detail?id=1192611
			if b.doUpload() && !b.MatchOs("Win") {
				b.directUpload(b.cfg.GsBucketGm, b.cfg.ServiceAccountUploadGM)
				directUpload = true
			}
			if b.MatchOs("iOS") {
				b.Spec.Caches = append(b.Spec.Caches, &specs.Cache{
					Name: "xcode",
					Path: "cache/Xcode.app",
				})
			}
		}
		b.recipeProp("gold_hashes_url", b.cfg.GoldHashesURL)
		b.recipeProps(EXTRA_PROPS)
		iid := b.internalHardwareLabel()
		iidStr := ""
		if iid != nil {
			iidStr = strconv.Itoa(*iid)
			b.recipeProp("internal_hardware_label", iidStr)
		}
		if recipe == "test" {
			b.dmFlags(iidStr)
		}
		b.kitchenTask(recipe, OUTPUT_TEST)
		b.cas(cas)
		b.swarmDimensions()
		if b.ExtraConfig("CanvasKit", "Docker", "LottieWeb") {
			b.usesDocker()
		}
		if compileTaskName != "" {
			b.dep(compileTaskName)
		}
		if b.MatchOs("Android") && b.ExtraConfig("ASAN") {
			b.asset("android_ndk_linux")
		}
		if b.ExtraConfig("NativeFonts") && !b.MatchOs("Android") {
			b.needsFontsForParagraphTests()
		}
		if b.ExtraConfig("Fontations") {
			b.cipd(&specs.CipdPackage{
				Name:    "chromium/third_party/googlefonts_testdata",
				Path:    "googlefonts_testdata",
				Version: "version:20230913",
			})
		}
		b.commonTestPerfAssets()
		if b.MatchExtraConfig("Lottie") {
			b.asset("lottie-samples")
		}
		b.expiration(20 * time.Hour)

		b.timeout(4 * time.Hour)
		if b.ExtraConfig("MSAN") {
			b.timeout(9 * time.Hour)
		} else if b.Arch("x86") && b.Debug() {
			// skbug.com/40037952
			b.timeout(6 * time.Hour)
		} else if b.MatchOs("Mac14") {
			b.timeout(30 * time.Minute)
		}
		b.maybeAddIosDevImage()
	})

	// Upload results if necessary. TODO(kjlubick): If we do coverage analysis at the same
	// time as normal tests (which would be nice), cfg.json needs to have Coverage removed.
	if b.doUpload() && !directUpload {
		uploadName := fmt.Sprintf("%s%s%s", PREFIX_UPLOAD, b.jobNameSchema.Sep, b.Name)
		depName := b.Name
		b.addTask(uploadName, func(b *TaskBuilder) {
			b.recipeProp("gs_bucket", b.cfg.GsBucketGm)
			b.recipeProps(EXTRA_PROPS)
			b.kitchenTask("upload_dm_results", OUTPUT_NONE)
			b.serviceAccount(b.cfg.ServiceAccountUploadGM)
			b.linuxGceDimensions(MACHINE_TYPE_SMALL)
			b.usesGSUtil()
			b.dep(depName)
		})
	}
}

// canary generates a task that uses TaskDrivers to trigger canary manual rolls on autorollers.
// Canary-G3 does not use this path because it is very different from other autorollers.
func (b *jobBuilder) canary(rollerName, canaryCQKeyword, targetProjectBaseURL string) {
	b.addTask(b.Name, func(b *TaskBuilder) {
		b.cas(CAS_EMPTY)
		b.cmd(
			b.taskDriver("canary", false),
			"--local=false",
			"--project_id", "skia-swarming-bots",
			"--task_id", specs.PLACEHOLDER_TASK_ID,
			"--task_name", b.Name,
			"--roller_name", rollerName,
			"--cq_keyword", canaryCQKeyword,
			"--target_project_base_url", targetProjectBaseURL,
			"--repo", specs.PLACEHOLDER_REPO,
			"--revision", specs.PLACEHOLDER_REVISION,
			"--patch_issue", specs.PLACEHOLDER_ISSUE,
			"--patch_set", specs.PLACEHOLDER_PATCHSET,
			"--patch_server", specs.PLACEHOLDER_CODEREVIEW_SERVER,
		)
		b.linuxGceDimensions(MACHINE_TYPE_SMALL)
		b.usesLUCIAuth()
		b.serviceAccount(b.cfg.ServiceAccountCanary)
		b.timeout(3 * time.Hour)
		b.attempts(1)
	})
}

// puppeteer generates a task that uses TaskDrivers combined with a node script and puppeteer to
// benchmark something using Chromium (e.g. CanvasKit, LottieWeb).
func (b *jobBuilder) puppeteer() {
	compileTaskName := b.compile()
	b.addTask(b.Name, func(b *TaskBuilder) {
		b.defaultSwarmDimensions()
		b.usesNode()
		b.usesLUCIAuth()
		b.dep(compileTaskName)
		b.output(OUTPUT_PERF)
		b.timeout(60 * time.Minute)
		b.cas(CAS_PUPPETEER)
		b.serviceAccount(b.cfg.ServiceAccountCompile)

		webglversion := "2"
		if b.ExtraConfig("WebGL1") {
			webglversion = "1"
		}

		if b.ExtraConfig("SkottieFrames") {
			b.cmd(
				b.taskDriver("perf_puppeteer_skottie_frames", false),
				"--project_id", "skia-swarming-bots",
				"--git_hash", specs.PLACEHOLDER_REVISION,
				"--task_id", specs.PLACEHOLDER_TASK_ID,
				"--task_name", b.Name,
				"--canvaskit_bin_path", "./build",
				"--lotties_path", "./lotties_with_assets",
				"--node_bin_path", "./node/node/bin",
				"--benchmark_path", "./tools/perf-canvaskit-puppeteer",
				"--output_path", OUTPUT_PERF,
				"--os_trace", b.Parts["os"],
				"--model_trace", b.Parts["model"],
				"--cpu_or_gpu_trace", b.Parts["cpu_or_gpu"],
				"--cpu_or_gpu_value_trace", b.Parts["cpu_or_gpu_value"],
				"--webgl_version", webglversion, // ignore when running with cpu backend
			)
			b.needsLottiesWithAssets()
		} else if b.ExtraConfig("RenderSKP") {
			b.cmd(
				b.taskDriver("perf_puppeteer_render_skps", false),
				"--project_id", "skia-swarming-bots",
				"--git_hash", specs.PLACEHOLDER_REVISION,
				"--task_id", specs.PLACEHOLDER_TASK_ID,
				"--task_name", b.Name,
				"--canvaskit_bin_path", "./build",
				"--skps_path", "./skp",
				"--node_bin_path", "./node/node/bin",
				"--benchmark_path", "./tools/perf-canvaskit-puppeteer",
				"--output_path", OUTPUT_PERF,
				"--os_trace", b.Parts["os"],
				"--model_trace", b.Parts["model"],
				"--cpu_or_gpu_trace", b.Parts["cpu_or_gpu"],
				"--cpu_or_gpu_value_trace", b.Parts["cpu_or_gpu_value"],
				"--webgl_version", webglversion,
			)
			b.asset("skp")
		} else if b.ExtraConfig("CanvasPerf") { // refers to the canvas_perf.js test suite
			b.cmd(
				b.taskDriver("perf_puppeteer_canvas", false),
				"--project_id", "skia-swarming-bots",
				"--git_hash", specs.PLACEHOLDER_REVISION,
				"--task_id", specs.PLACEHOLDER_TASK_ID,
				"--task_name", b.Name,
				"--canvaskit_bin_path", "./build",
				"--node_bin_path", "./node/node/bin",
				"--benchmark_path", "./tools/perf-canvaskit-puppeteer",
				"--output_path", OUTPUT_PERF,
				"--os_trace", b.Parts["os"],
				"--model_trace", b.Parts["model"],
				"--cpu_or_gpu_trace", b.Parts["cpu_or_gpu"],
				"--cpu_or_gpu_value_trace", b.Parts["cpu_or_gpu_value"],
				"--webgl_version", webglversion,
			)
			b.asset("skp")
		}

	})

	// Upload results to Perf after.
	// TODO(kjlubick,borenet) deduplicate this with the logic in perf().
	uploadName := fmt.Sprintf("%s%s%s", PREFIX_UPLOAD, b.jobNameSchema.Sep, b.Name)
	depName := b.Name
	b.addTask(uploadName, func(b *TaskBuilder) {
		b.recipeProp("gs_bucket", b.cfg.GsBucketNano)
		b.recipeProps(EXTRA_PROPS)
		// TODO(borenet): I'm not sure why the upload task is
		// using the Perf task name, but I've done this to
		// maintain existing behavior.
		b.Name = depName
		b.kitchenTask("upload_nano_results", OUTPUT_NONE)
		b.Name = uploadName
		b.serviceAccount(b.cfg.ServiceAccountUploadNano)
		b.linuxGceDimensions(MACHINE_TYPE_SMALL)
		b.usesGSUtil()
		b.dep(depName)
	})
}

// perf generates a Perf task.
func (b *jobBuilder) perf() {
	compileTaskName := ""
	// LottieWeb doesn't require anything in Skia to be compiled.
	if !b.ExtraConfig("LottieWeb") {
		compileTaskName = b.compile()
	}
	doUpload := !b.Debug() && b.doUpload()
	b.addTask(b.Name, func(b *TaskBuilder) {
		recipe := "perf"
		cas := CAS_PERF
		if b.ExtraConfig("CanvasKit") {
			cas = CAS_CANVASKIT
			recipe = "perf_canvaskit"
		} else if b.ExtraConfig("SkottieTracing") {
			recipe = "perf_skottietrace"
		} else if b.ExtraConfig("SkottieWASM") {
			recipe = "perf_skottiewasm_lottieweb"
			cas = CAS_SKOTTIE_WASM
		} else if b.ExtraConfig("LottieWeb") {
			recipe = "perf_skottiewasm_lottieweb"
			cas = CAS_LOTTIE_WEB
		} else if b.MatchOs("iOS") {
			// We need a service account in order to download the xcode CIPD
			// packages.
			b.serviceAccount(b.cfg.ServiceAccountUploadNano)
			b.Spec.Caches = append(b.Spec.Caches, &specs.Cache{
				Name: "xcode",
				Path: "cache/Xcode.app",
			})
		}
		b.recipeProps(EXTRA_PROPS)
		if recipe == "perf" {
			b.nanobenchFlags(doUpload)
		}
		iid := b.internalHardwareLabel()
		if iid != nil {
			b.recipeProp("internal_hardware_label", strconv.Itoa(*iid))
		}
		b.kitchenTask(recipe, OUTPUT_PERF)
		b.cas(cas)
		b.swarmDimensions()
		if b.ExtraConfig("Docker") {
			b.usesDocker()
		}
		if compileTaskName != "" {
			b.dep(compileTaskName)
		}
		b.commonTestPerfAssets()
		b.expiration(20 * time.Hour)
		b.timeout(4 * time.Hour)

		if b.ExtraConfig("MSAN") {
			b.timeout(9 * time.Hour)
		} else if b.Parts["arch"] == "x86" && b.Parts["configuration"] == "Debug" {
			// skbug.com/40037952
			b.timeout(6 * time.Hour)
		} else if b.MatchOs("Mac14") {
			b.timeout(30 * time.Minute)
		}

		if b.ExtraConfig("LottieWeb", "SkottieWASM") {
			b.asset("node", "lottie-samples")
		} else if b.MatchExtraConfig("SkottieTracing") {
			b.needsLottiesWithAssets()
		} else if b.MatchExtraConfig("Skottie") {
			b.asset("lottie-samples")
		}

		if b.MatchOs("Android") && b.CPU() {
			b.asset("text_blob_traces")
		}
		b.maybeAddIosDevImage()
	})

	// Upload results if necessary.
	if doUpload {
		uploadName := fmt.Sprintf("%s%s%s", PREFIX_UPLOAD, b.jobNameSchema.Sep, b.Name)
		depName := b.Name
		b.addTask(uploadName, func(b *TaskBuilder) {
			b.recipeProp("gs_bucket", b.cfg.GsBucketNano)
			b.recipeProps(EXTRA_PROPS)
			// TODO(borenet): I'm not sure why the upload task is
			// using the Perf task name, but I've done this to
			// maintain existing behavior.
			b.Name = depName
			b.kitchenTask("upload_nano_results", OUTPUT_NONE)
			b.Name = uploadName
			b.serviceAccount(b.cfg.ServiceAccountUploadNano)
			b.linuxGceDimensions(MACHINE_TYPE_SMALL)
			b.usesGSUtil()
			b.dep(depName)
		})
	}
}

// presubmit generates a task which runs the presubmit for this repo.
func (b *jobBuilder) presubmit() {
	b.addTask(b.Name, func(b *TaskBuilder) {
		b.recipeProps(map[string]string{
			"category":         "cq",
			"patch_gerrit_url": "https://skia-review.googlesource.com",
			"patch_project":    "skia",
			"patch_ref":        specs.PLACEHOLDER_PATCH_REF,
			"reason":           "CQ",
			"repo_name":        "skia",
		})
		b.recipeProps(EXTRA_PROPS)
		b.kitchenTaskNoBundle("run_presubmit", OUTPUT_NONE)
		b.cas(CAS_RUN_RECIPE)
		b.serviceAccount(b.cfg.ServiceAccountCompile)
		// Use MACHINE_TYPE_LARGE because it seems to save time versus
		// MEDIUM and we want presubmit to be fast.
		b.linuxGceDimensions(MACHINE_TYPE_LARGE)
		b.usesGit()
		b.cipd(&specs.CipdPackage{
			Name:    "infra/recipe_bundles/chromium.googlesource.com/chromium/tools/build",
			Path:    "recipe_bundle",
			Version: "git_revision:bb122cd16700ab80bfcbd494b605dd11d4f5902d",
		})
	})
}

// compileWasmGMTests uses a task driver to compile the GMs and unit tests for Web Assembly (WASM).
// We can use the same build for both CPU and GPU tests since the latter requires the code for the
// former anyway.
func (b *jobBuilder) compileWasmGMTests(compileName string) {
	b.addTask(compileName, func(b *TaskBuilder) {
		b.attempts(1)
		b.usesDocker()
		b.linuxGceDimensions(MACHINE_TYPE_MEDIUM)
		b.usesLUCIAuth()
		b.output("wasm_out")
		b.timeout(60 * time.Minute)
		b.cas(CAS_COMPILE)
		b.serviceAccount(b.cfg.ServiceAccountCompile)
		b.cache(CACHES_DOCKER...)
		// For now, we only have one compile mode - a GPU release mode. This should be sufficient to
		// run CPU, WebGL1, and WebGL2 tests. Debug mode is not needed for the waterfall because
		// when using puppeteer, stacktraces from exceptions are hard to get access to, so we do not
		// even bother.
		b.cmd(
			b.taskDriver("compile_wasm_gm_tests", false),
			"--project_id", "skia-swarming-bots",
			"--task_id", specs.PLACEHOLDER_TASK_ID,
			"--task_name", compileName,
			"--out_path", "./wasm_out",
			"--skia_path", "./skia",
			"--work_path", "./cache/docker/wasm_gm",
		)
	})
}

// compileWasmGMTests uses a task driver to compile the GMs and unit tests for Web Assembly (WASM).
// We can use the same build for both CPU and GPU tests since the latter requires the code for the
// former anyway.
func (b *jobBuilder) runWasmGMTests() {
	compileTaskName := b.compile()

	b.addTask(b.Name, func(b *TaskBuilder) {
		b.attempts(1)
		b.usesNode()
		b.swarmDimensions()
		b.usesLUCIAuth()
		b.cipd(CIPD_PKGS_GOLDCTL)
		b.dep(compileTaskName)
		b.timeout(60 * time.Minute)
		b.cas(CAS_WASM_GM)
		b.serviceAccount(b.cfg.ServiceAccountUploadGM)
		b.cmd(
			b.taskDriver("run_wasm_gm_tests", false),
			"--project_id", "skia-swarming-bots",
			"--task_id", specs.PLACEHOLDER_TASK_ID,
			"--task_name", b.Name,
			"--test_harness_path", "./tools/run-wasm-gm-tests",
			"--built_path", "./wasm_out",
			"--node_bin_path", "./node/node/bin",
			"--resource_path", "./resources",
			"--work_path", "./wasm_gm/work",
			"--gold_ctl_path", "./cipd_bin_packages/goldctl",
			"--gold_hashes_url", b.cfg.GoldHashesURL,
			"--git_commit", specs.PLACEHOLDER_REVISION,
			"--changelist_id", specs.PLACEHOLDER_ISSUE,
			"--patchset_order", specs.PLACEHOLDER_PATCHSET,
			"--tryjob_id", specs.PLACEHOLDER_BUILDBUCKET_BUILD_ID,
			// TODO(kjlubick) Make these not hard coded if we change the configs we test on.
			"--webgl_version", "2", // 0 means CPU ; this flag controls cpu_or_gpu and extra_config
			"--gold_key", "alpha_type:Premul",
			"--gold_key", "arch:wasm",
			"--gold_key", "browser:Chrome",
			"--gold_key", "color_depth:8888",
			"--gold_key", "config:gles",
			"--gold_key", "configuration:Release",
			"--gold_key", "cpu_or_gpu_value:QuadroP400",
			"--gold_key", "model:Golo",
			"--gold_key", "os:Ubuntu24.04",
		)
	})
}

// labelAndSavedOutputDir contains a Bazel label (e.g. //tests:some_test) and a //bazel-bin
// subdirectory that should be stored into CAS.
type labelAndSavedOutputDir struct {
	label          string
	savedOutputDir string
}

// Maps a shorthand version of a label (which can be an arbitrary string) to an absolute Bazel
// label or "target pattern" https://bazel.build/docs/build#specifying-build-targets
// The reason we need this mapping is because Buildbucket build names cannot have / or : in them.
// TODO(borenet/kjlubick): Is there a way to generate a mapping using `bazel query`?
var shorthandToLabel = map[string]labelAndSavedOutputDir{
	"all_tests":                  {"//tests:linux_rbe_tests", ""},
	"core":                       {"//:core", ""},
	"cpu_8888_benchmark_test":    {"//bench:cpu_8888_test", ""},
	"cpu_gms":                    {"//gm:cpu_gm_tests", ""},
	"dm":                         {"//dm", ""},
	"fontations":                 {"//src/ports:fontmgr_fontations_empty", ""},
	"full_library":               {"//tools:full_build", ""},
	"ganesh_gl":                  {"//:ganesh_gl", ""},
	"hello_bazel_world_test":     {"//gm:hello_bazel_world_test", ""},
	"modules_canvaskit":          {"//modules/canvaskit:canvaskit", ""},
	"modules_canvaskit_js_tests": {"//modules/canvaskit:canvaskit_js_tests", ""},
	"skottie_tool_gpu":           {"//modules/skottie:skottie_tool_gpu", ""},
	"viewer":                     {"//tools/viewer:viewer", ""},
	"decode_everything":          {"//example/external_client:decode_everything", ""},
	"path_combiner":              {"//example/external_client:path_combiner", ""},
	"png_decoder":                {"//example/external_client:png_decoder", ""},
	"shape_text":                 {"//example/external_client:shape_text", ""},
	"svg_with_harfbuzz":          {"//example/external_client:svg_with_harfbuzz", ""},
	"svg_with_primitive":         {"//example/external_client:svg_with_primitive", ""},
	"use_ganesh_gl":              {"//example/external_client:use_ganesh_gl", ""},
	"use_ganesh_vulkan":          {"//example/external_client:use_ganesh_vulkan", ""},
	"use_graphite_native_vulkan": {"//example/external_client:use_graphite_native_vulkan", ""},
	"use_skresources":            {"//example/external_client:use_skresources", ""},
	"write_text_to_png":          {"//example/external_client:write_text_to_png", ""},
	"write_to_pdf":               {"//example/external_client:write_to_pdf", ""},
	"play_skottie":               {"//example/external_client:play_skottie", ""},

	// Currently there is no way to tell Bazel "only test go_test targets", so we must group them
	// under a test_suite.
	//
	// Alternatives:
	//
	// - Use --test_lang_filters, which currently does not work for non-native rules. See
	//   https://github.com/bazelbuild/bazel/issues/12618.
	//
	// - As suggested in the same GitHub issue, "bazel query 'kind(go_test, //...)'" would normally
	//   return the list of labels. However, this fails due to BUILD.bazel files in
	//   //third_party/externals and //bazel/external/vello. We could try either fixing those files
	//   when possible, or adding them to //.bazelignore (either permanently or temporarily inside a
	//   specialized task driver just for Go tests).
	//
	// - Have Gazelle add a tag to all Go tests: go_test(name = "foo_test", tag = "go", ... ). Then,
	//   we can use a wildcard label such as //... and tell Bazel to only test those targets with
	//   said tag, e.g. "bazel test //... --test_tag_filters=go"
	//   (https://bazel.build/reference/command-line-reference#flag--test_tag_filters). Today this
	//   does not work due to the third party and external BUILD.bazel files mentioned in the
	//   previous bullet point.
	"all_go_tests": {"//:all_go_tests", ""},

	// Android tests that run on a device. We store the //bazel-bin/tests directory into CAS for use
	// by subsequent CI tasks.
	"android_math_test":               {"//tests:android_math_test", "tests"},
	"hello_bazel_world_android_test":  {"//gm:hello_bazel_world_android_test", "gm"},
	"cpu_8888_benchmark_android_test": {"//bench:cpu_8888_android_test", "bench"},
}

// bazelBuild adds a task which builds the specified single-target label (//foo:bar) or
// multi-target label (//foo/...) using Bazel. Depending on the host we run this on, we may
// specify additional Bazel args to build faster. Optionally, a subset of the //bazel-bin directory
// will be stored into CAS for use by subsequent tasks.
func (b *jobBuilder) bazelBuild() {
	shorthand, config, host := b.Parts.BazelBuildParts()
	labelAndSavedOutputDir, ok := shorthandToLabel[shorthand]
	if !ok {
		panic("unsupported Bazel label shorthand " + shorthand)
	}

	b.addTask(b.Name, func(b *TaskBuilder) {
		bazelCacheDir, ok := map[string]string{
			// We only run builds in GCE.
			"linux_x64":   bazelCacheDirOnGCELinux,
			"windows_x64": bazelCacheDirOnWindows,
		}[host]
		if !ok {
			panic("unknown Bazel cache dir for Bazel host " + host)
		}

		// Bazel git_repository rules shell out to git. Use the version from
		// CIPD to ensure that we're not using an old locally-installed version.
		b.usesGit()
		b.addToPATH("cipd_bin_packages", "cipd_bin_packages/bin")
		b.usesLUCIAuth()

		cmd := []string{
			"luci-auth", "context",
			b.taskDriver("bazel_build", host != "windows_x64"),
			"--project_id=skia-swarming-bots",
			"--task_id=" + specs.PLACEHOLDER_TASK_ID,
			"--task_name=" + b.Name,
			"--bazel_label=" + labelAndSavedOutputDir.label,
			"--bazel_config=" + config,
			"--bazel_cache_dir=" + bazelCacheDir,
			"--workdir=./skia",
		}

		if labelAndSavedOutputDir.savedOutputDir != "" {
			cmd = append(cmd,
				"--out_path="+OUTPUT_BAZEL,
				// Which //bazel-bin subdirectory to copy into the output dir (flag --out_path).
				"--saved_output_dir="+labelAndSavedOutputDir.savedOutputDir,
			)
		}

		if host == "linux_x64" {
			b.linuxGceDimensions(MACHINE_TYPE_MEDIUM)
			b.usesBazel("linux_x64")
			if labelAndSavedOutputDir.savedOutputDir != "" {
				// We assume that builds which require storing a subset of //bazel-bin to CAS are Android
				// builds. We want such builds to use RBE, and we want to download the built top-level
				// artifacts. Also, we need the adb_test runner to be cross-compiled to run on a Raspberry
				// Pi.
				cmd = append(cmd, "--bazel_arg=--config=linux_rbe")
				cmd = append(cmd, "--bazel_arg=--jobs=100")
				cmd = append(cmd, "--bazel_arg=--remote_download_toplevel")
				cmd = append(cmd, "--bazel_arg=--adb_platform=linux_arm64")
			} else {
				// We want all Linux Bazel Builds to use RBE
				cmd = append(cmd, "--bazel_arg=--config=for_linux_x64_with_rbe")
				cmd = append(cmd, "--bazel_arg=--jobs=100")
				cmd = append(cmd, "--bazel_arg=--remote_download_minimal")
			}
		} else if host == "windows_x64" {
			b.dimension(
				"cpu:x86-64-Haswell_GCE",
				"gpu:none",
				fmt.Sprintf("machine_type:%s", MACHINE_TYPE_LARGE),
				fmt.Sprintf("os:%s", DEFAULT_OS_WIN_GCE),
				"pool:Skia",
			)
			b.usesBazel("windows_x64")
			cmd = append(cmd, "--bazel_arg=--experimental_scale_timeouts=2.0")
		} else {
			panic("unsupported Bazel host " + host)
		}
		b.cmd(cmd...)

		b.idempotent()
		b.cas(CAS_BAZEL)
		b.attempts(1)
		b.serviceAccount(b.cfg.ServiceAccountCompile)
		if labelAndSavedOutputDir.savedOutputDir != "" {
			b.output(OUTPUT_BAZEL)
		}
	})
}

type precompiledBazelTestKind int

const (
	precompiledBazelTestNone precompiledBazelTestKind = iota
	precompiledBenchmarkTest
	precompiledGMTest
	precompiledUnitTest
)

func (b *jobBuilder) bazelTest() {
	taskdriverName, shorthand, buildConfig, host, testConfig := b.Parts.BazelTestParts()
	labelAndSavedOutputDir, ok := shorthandToLabel[shorthand]
	if !ok {
		panic("unsupported Bazel label shorthand " + shorthand)
	}

	// Expand task driver name to keep task names short.
	precompiledKind := precompiledBazelTestNone
	if taskdriverName == "precompiled_benchmark" {
		taskdriverName = "bazel_test_precompiled"
		precompiledKind = precompiledBenchmarkTest
	}
	if taskdriverName == "precompiled_gm" {
		taskdriverName = "bazel_test_precompiled"
		precompiledKind = precompiledGMTest
	}
	if taskdriverName == "precompiled_test" {
		taskdriverName = "bazel_test_precompiled"
		precompiledKind = precompiledUnitTest
	}
	if taskdriverName == "gm" {
		taskdriverName = "bazel_test_gm"
	}
	if taskdriverName == "benchmark" {
		taskdriverName = "bazel_test_benchmark"
	}

	useLUCIAuth := true
	if taskdriverName == "external_client" {
		useLUCIAuth = false
	}

	var deviceSpecificBazelConfig *device_specific_configs.Config
	if testConfig != "" {
		if config, ok := device_specific_configs.Configs[testConfig]; ok {
			deviceSpecificBazelConfig = &config
		} else {
			panic(fmt.Sprintf("Unknown device-specific Bazel config: %q", testConfig))
		}
	}

	bazelCacheDir := bazelCacheDirOnGCELinux
	if deviceSpecificBazelConfig != nil && deviceSpecificBazelConfig.Keys["model"] != "GCE" {
		bazelCacheDir = bazelCacheDirOnSkoloLinux
	}

	b.addTask(b.Name, func(b *TaskBuilder) {
		cmd := []string{}
		if useLUCIAuth {
			cmd = []string{"luci-auth", "context"}
		}
		cmd = append(cmd,
			b.taskDriver(taskdriverName, false),
			"--project_id=skia-swarming-bots",
			"--task_id="+specs.PLACEHOLDER_TASK_ID,
			"--task_name="+b.Name,
			"--workdir=.",
		)
		b.usesLUCIAuth()

		switch taskdriverName {
		case "canvaskit_gold":
			cmd = append(cmd,
				"--bazel_label="+labelAndSavedOutputDir.label,
				"--bazel_config="+buildConfig,
				"--bazel_cache_dir="+bazelCacheDir,
				"--goldctl_path=./cipd_bin_packages/goldctl",
				"--git_commit="+specs.PLACEHOLDER_REVISION,
				"--changelist_id="+specs.PLACEHOLDER_ISSUE,
				"--patchset_order="+specs.PLACEHOLDER_PATCHSET,
				"--tryjob_id="+specs.PLACEHOLDER_BUILDBUCKET_BUILD_ID)
			b.cipd(CIPD_PKGS_GOLDCTL)
			switch buildConfig {
			case "ck_full_cpu_release_chrome":
				cmd = append(cmd, "--cpu_or_gpu=CPU", "--cpu_or_gpu_value=CPU",
					"--compilation_mode=Release", "--browser=Chrome")
			case "ck_full_webgl2_release_chrome":
				cmd = append(cmd, "--cpu_or_gpu=GPU", "--cpu_or_gpu_value=WebGL2",
					"--compilation_mode=Release", "--browser=Chrome")
			default:
				panic("Gold keys not specified for config " + buildConfig)
			}

		case "cpu_tests":
			cmd = append(cmd,
				"--bazel_label="+labelAndSavedOutputDir.label,
				"--bazel_config="+buildConfig,
				"--bazel_cache_dir="+bazelCacheDir)

		case "toolchain_layering_check":
			cmd = append(cmd,
				"--bazel_label="+labelAndSavedOutputDir.label,
				"--bazel_config="+buildConfig,
				"--bazel_cache_dir="+bazelCacheDir)

		case "bazel_test_precompiled":
			// Compute the file name of the test based on its Bazel label. The file name will be relative to
			// the bazel-bin directory, which we receive a subset of as a CAS input.
			command := strings.ReplaceAll(labelAndSavedOutputDir.label, "//", "")
			command = strings.ReplaceAll(command, ":", "/")
			command = path.Join(OUTPUT_BAZEL, command)

			// The test's working directory will be its runfiles directory, which simulates the behavior of
			// the "bazel run" command.
			commandWorkDir := path.Join(command+".runfiles", "skia")

			cmd = append(cmd,
				"--command="+command,
				"--command_workdir="+commandWorkDir)

			switch precompiledKind {
			case precompiledBenchmarkTest:
				cmd = append(cmd,
					"--kind=benchmark",
					"--git_commit="+specs.PLACEHOLDER_REVISION,
					"--changelist_id="+specs.PLACEHOLDER_ISSUE,
					"--patchset_order="+specs.PLACEHOLDER_PATCHSET)

			case precompiledGMTest:
				cmd = append(cmd,
					"--kind=gm",
					"--bazel_label="+labelAndSavedOutputDir.label,
					"--goldctl_path=./cipd_bin_packages/goldctl",
					"--git_commit="+specs.PLACEHOLDER_REVISION,
					"--changelist_id="+specs.PLACEHOLDER_ISSUE,
					"--patchset_order="+specs.PLACEHOLDER_PATCHSET,
					"--tryjob_id="+specs.PLACEHOLDER_BUILDBUCKET_BUILD_ID)
				b.cipd(CIPD_PKGS_GOLDCTL)

			case precompiledUnitTest:
				cmd = append(cmd, "--kind=unit")

			default:
				panic(fmt.Sprintf("Unknown precompiled test kind: %v", precompiledKind))
			}

		case "bazel_test_gm":
			cmd = append(cmd,
				"--bazel_label="+labelAndSavedOutputDir.label,
				"--bazel_config="+buildConfig,
				"--bazel_cache_dir="+bazelCacheDir,
				"--goldctl_path=./cipd_bin_packages/goldctl",
				"--git_commit="+specs.PLACEHOLDER_REVISION,
				"--changelist_id="+specs.PLACEHOLDER_ISSUE,
				"--patchset_order="+specs.PLACEHOLDER_PATCHSET,
				"--tryjob_id="+specs.PLACEHOLDER_BUILDBUCKET_BUILD_ID)
			b.cipd(CIPD_PKGS_GOLDCTL)

		case "bazel_test_benchmark":
			// Note that these tasks run on Skolo machines.
			cmd = append(cmd,
				"--bazel_label="+labelAndSavedOutputDir.label,
				"--bazel_config="+buildConfig,
				"--bazel_cache_dir="+bazelCacheDirOnSkoloLinux,
				"--git_commit="+specs.PLACEHOLDER_REVISION,
				"--changelist_id="+specs.PLACEHOLDER_ISSUE,
				"--patchset_order="+specs.PLACEHOLDER_PATCHSET)

		case "external_client":
			// For external_client, we want to test how an external user would
			// build using Skia. Therefore, we change to the workspace in that
			// directory and use labels relative to it.
			pathInSkia := "example/external_client"
			label := strings.Replace(labelAndSavedOutputDir.label, pathInSkia, "", -1)
			cmd = append(cmd,
				"--bazel_label="+label,
				"--path_in_skia="+pathInSkia,
				"--bazel_cache_dir="+bazelCacheDir)
			b.usesDocker()

		default:
			panic("Unsupported Bazel taskdriver " + taskdriverName)
		}

		if deviceSpecificBazelConfig != nil {
			cmd = append(cmd, "--device_specific_bazel_config="+deviceSpecificBazelConfig.Name)
		}

		if host == "linux_x64" {
			b.usesBazel("linux_x64")
		} else if host == "linux_arm64" || host == "on_rpi" {
			// The RPIs do not run Bazel directly, they have precompiled binary
			// to run instead.
		} else {
			panic("unsupported Bazel host " + host)
		}

		if taskdriverName == "bazel_test_gm" ||
			taskdriverName == "bazel_test_benchmark" ||
			taskdriverName == "bazel_test_precompiled" {
			if taskdriverName == "bazel_test_precompiled" {
				// This task precompiles the test and stores it to CAS.
				b.dep(fmt.Sprintf("BazelBuild-%s-%s-linux_x64", shorthand, buildConfig))
			}

			// Set dimensions.
			if deviceSpecificBazelConfig == nil {
				log.Fatalf("While processing job %q: task driver %q requires a device-specific Bazel config.", b.Name, taskdriverName)
			}
			if len(deviceSpecificBazelConfig.SwarmingDimensions) == 0 {
				log.Fatalf("While processing job %q: device-specific Bazel config %q does not provide Swarming dimensions.", b.Name, deviceSpecificBazelConfig.Name)
			}
			var dimensions []string
			for name, value := range deviceSpecificBazelConfig.SwarmingDimensions {
				dimensions = append(dimensions, fmt.Sprintf("%s:%s", name, value))
			}
			dimensions = append(dimensions, fmt.Sprintf("pool:%s", b.cfg.Pool))
			sort.Strings(dimensions)
			b.dimension(dimensions...)
		} else {
			b.linuxGceDimensions(MACHINE_TYPE_MEDIUM)
		}

		b.cmd(cmd...)
		b.idempotent()
		b.cas(CAS_BAZEL)
		b.attempts(1)
		b.serviceAccount(b.cfg.ServiceAccountCompile)
	})
}
