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
	"log"
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
	CAS_PATHKIT       = "pathkit"
	CAS_PERF          = "perf"
	CAS_PUPPETEER     = "puppeteer"
	CAS_RUN_RECIPE    = "run-recipe"
	CAS_RECIPES       = "recipes"
	CAS_RECREATE_SKPS = "recreate-skps"
	CAS_SKOTTIE_WASM  = "skottie-wasm"
	CAS_SKPBENCH      = "skpbench"
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

	DEBIAN_11_OS                   = "Debian-11.5"
	DEFAULT_OS_DEBIAN              = "Debian-10.10"
	DEFAULT_OS_LINUX_GCE           = "Debian-10.3"
	OLD_OS_LINUX_GCE               = "Debian-9.8"
	COMPILE_TASK_NAME_OS_LINUX     = "Debian10"
	COMPILE_TASK_NAME_OS_LINUX_OLD = "Debian9"
	DEFAULT_OS_MAC                 = "Mac-10.15.7"
	DEFAULT_OS_WIN                 = "Windows-Server-17763"

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
	// space. On Linux GCE machines, the partition mounted at /mnt/pd0 is significantly larger than
	// the partition mounted at /.
	bazelCacheDirOnGCELinux = "/mnt/pd0/bazel_cache"

	// bazelCacheDirOnSkoloLinux is like bazelCacheDirOnGCELinux for Skolo Linux machines. Unlike GCE
	// Linux machines, the partition mounted at / on Skolo Linux machines is large enough. While
	// using the default Bazel cache path would work, our Bazel task drivers demand an explicit path.
	// We store the Bazel cache at /home/chrome-bot/bazel_cache rather than on the default location
	// of /home/chrome-bot/cache/.bazel to make it obvious to someone examining a Skolo machine that
	// we are overriding the default location.
	bazelCacheDirOnSkoloLinux = "/home/chrome-bot/bazel_cache"
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
			"skia/.vpython",
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
	LoadJson(filepath.Join(cfgDir, "cfg.json"), &cfg)
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

// LoadJson loads JSON from the given file and unmarshals it into the given
// destination.
func LoadJson(filename string, dest interface{}) {
	b, err := ioutil.ReadFile(filename)
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
func GenTasks(cfg *Config) {
	b := specs.MustNewTasksCfgBuilder()

	// Find the paths to the infra/bots directories in this repo and the
	// repo of the calling file.
	relpathTargetDir := getThisDirName()
	relpathBaseDir := getCallingDirName()

	// Parse jobs.json.
	var jobsWithInfo []*JobInfo
	LoadJson(filepath.Join(relpathBaseDir, "jobs.json"), &jobsWithInfo)
	// Create a slice with only job names.
	jobs := []string{}
	for _, j := range jobsWithInfo {
		jobs = append(jobs, j.Name)
	}

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
			"skia/experimental/bazel_test",
			"skia/include",
			"skia/modules",
			"skia/src",
			"skia/tests",
			"skia/third_party",
			"skia/tools",
			// Needed for tests.
			"skia/bench", // Needed to run benchmark tests with Bazel.
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
			"skia/WORKSPACE.bazel",
			"skia/bazel",
			"skia/defines.bzl",
			"skia/go_repositories.bzl",
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
			"skia/.vpython",
			"skia/infra/bots/run_recipe.py",
			"skia/infra/canvaskit",
			"skia/modules/canvaskit",
			"skia/modules/pathkit/perf/perfReporter.js",
			"skia/modules/pathkit/tests/testReporter.js",
		},
		Excludes: []string{rbe.ExcludeGitDir},
	})
	b.MustAddCasSpec(CAS_EMPTY, specs.EmptyCasSpec)
	b.MustAddCasSpec(CAS_LOTTIE_CI, CAS_SPEC_LOTTIE_CI)
	b.MustAddCasSpec(CAS_LOTTIE_WEB, &specs.CasSpec{
		Root: "..",
		Paths: []string{
			"skia/.vpython",
			"skia/infra/bots/run_recipe.py",
			"skia/tools/lottie-web-perf",
		},
		Excludes: []string{rbe.ExcludeGitDir},
	})
	b.MustAddCasSpec(CAS_PATHKIT, &specs.CasSpec{
		Root: "..",
		Paths: []string{
			"skia/.vpython",
			"skia/infra/bots/run_recipe.py",
			"skia/infra/pathkit",
			"skia/modules/pathkit",
		},
		Excludes: []string{rbe.ExcludeGitDir},
	})
	b.MustAddCasSpec(CAS_PERF, &specs.CasSpec{
		Root: "..",
		Paths: []string{
			"skia/.vpython",
			"skia/infra/bots/assets",
			"skia/infra/bots/run_recipe.py",
			"skia/platform_tools/ios/bin",
			"skia/resources",
			"skia/tools/valgrind.supp",
		},
		Excludes: []string{rbe.ExcludeGitDir},
	})
	b.MustAddCasSpec(CAS_PUPPETEER, &specs.CasSpec{
		Root: "../skia", // Needed for other repos.
		Paths: []string{
			".vpython",
			"tools/perf-canvaskit-puppeteer",
		},
		Excludes: []string{rbe.ExcludeGitDir},
	})
	b.MustAddCasSpec(CAS_RECIPES, &specs.CasSpec{
		Root: "..",
		Paths: []string{
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
			"skia/.vpython",
			"skia/infra/bots/run_recipe.py",
		},
		Excludes: []string{rbe.ExcludeGitDir},
	})
	b.MustAddCasSpec(CAS_SKOTTIE_WASM, &specs.CasSpec{
		Root: "..",
		Paths: []string{
			"skia/.vpython",
			"skia/infra/bots/run_recipe.py",
			"skia/tools/skottie-wasm-perf",
		},
		Excludes: []string{rbe.ExcludeGitDir},
	})
	b.MustAddCasSpec(CAS_SKPBENCH, &specs.CasSpec{
		Root: "..",
		Paths: []string{
			"skia/.vpython",
			"skia/infra/bots/assets",
			"skia/infra/bots/run_recipe.py",
			"skia/tools/skpbench",
			"skia/tools/valgrind.supp",
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
			"skia/WORKSPACE.bazel",
			"skia/bazel",
			"skia/go_repositories.bzl",
			"skia/include/config", // There's a WORKSPACE.bazel in here
			"skia/requirements.txt",
			"skia/toolchain",
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
			"skia/.vpython",
			"skia/infra/bots/assets",
			"skia/infra/bots/run_recipe.py",
			"skia/platform_tools/ios/bin",
			"skia/resources",
			"skia/tools/valgrind.supp",
		},
		Excludes: []string{rbe.ExcludeGitDir},
	})
	b.MustAddCasSpec(CAS_WASM_GM, &specs.CasSpec{
		Root: "../skia", // Needed for other repos.
		Paths: []string{
			".vpython",
			"resources",
			"tools/run-wasm-gm-tests",
		},
		Excludes: []string{rbe.ExcludeGitDir},
	})
	b.MustAddCasSpec(CAS_WHOLE_REPO, CAS_SPEC_WHOLE_REPO)
	b.MustAddCasSpec(CAS_RECREATE_SKPS, &specs.CasSpec{
		Root: "..",
		Paths: []string{
			"skia/.vpython",
			"skia/DEPS",
			"skia/bin/fetch-sk",
			"skia/infra/bots/assets/skp",
			"skia/infra/bots/utils.py",
			"skia/infra/config/recipes.cfg",
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
func (b *taskBuilder) kitchenTaskNoBundle(recipe string, outputDir string) {
	b.cipd(CIPD_PKG_LUCI_AUTH)
	b.cipd(cipd.MustGetPackage("infra/tools/luci/kitchen/${platform}"))
	b.env("RECIPES_USE_PY3", "true")
	b.envPrefixes("VPYTHON_DEFAULT_SPEC", "skia/.vpython")
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
	b.addToPATH("cipd_bin_packages", "cipd_bin_packages/bin")
	b.Spec.ExtraTags = map[string]string{
		"log_location": fmt.Sprintf("logdog://logs.chromium.org/%s/${SWARMING_TASK_ID}/+/annotations", b.cfg.Project),
	}

	// Attempts.
	if !b.role("Build", "Upload") && b.extraConfig("ASAN", "HWASAN", "MSAN", "TSAN", "Valgrind") {
		// Sanitizers often find non-deterministic issues that retries would hide.
		b.attempts(1)
	} else {
		// Retry by default to hide random bot/hardware failures.
		b.attempts(2)
	}
}

// kitchenTask sets up the task to run a recipe via Kitchen.
func (b *taskBuilder) kitchenTask(recipe string, outputDir string) {
	b.kitchenTaskNoBundle(recipe, outputDir)
	b.dep(b.bundleRecipes())
}

// internalHardwareLabel returns the internal ID for the bot, if any.
func (b *taskBuilder) internalHardwareLabel() *int {
	if b.cfg.InternalHardwareLabel != nil {
		return b.cfg.InternalHardwareLabel(b.parts)
	}
	return nil
}

// linuxGceDimensions adds the Swarming bot dimensions for Linux GCE instances.
func (b *taskBuilder) linuxGceDimensions(machineType string) {
	b.dimension(
		// Specify CPU to avoid running builds on bots with a more unique CPU.
		"cpu:x86-64-Haswell_GCE",
		"gpu:none",
		// Currently all Linux GCE tasks run on 16-CPU machines.
		fmt.Sprintf("machine_type:%s", machineType),
		fmt.Sprintf("os:%s", DEFAULT_OS_LINUX_GCE),
		fmt.Sprintf("pool:%s", b.cfg.Pool),
	)
}

// codesizeTaskNameRegexp captures the "CodeSize-<binary name>-" prefix of a CodeSize task name.
var codesizeTaskNameRegexp = regexp.MustCompile("^CodeSize-[a-zA-Z0-9_]+-")

// deriveCompileTaskName returns the name of a compile task based on the given
// job name.
func (b *jobBuilder) deriveCompileTaskName() string {
	if b.role("Test", "Perf") {
		task_os := b.parts["os"]
		ec := []string{}
		if val := b.parts["extra_config"]; val != "" {
			ec = strings.Split(val, "_")
			ignore := []string{
				"Skpbench", "AbandonGpuContext", "PreAbandonGpuContext", "Valgrind",
				"FailFlushTimeCallbacks", "ReleaseAndAbandonGpuContext", "FSAA", "FAAA", "FDAA",
				"NativeFonts", "GDI", "NoGPUThreads", "DDL1", "DDL3",
				"DDLTotal", "DDLRecord", "9x9", "BonusConfigs", "ColorSpaces", "GL",
				"SkottieTracing", "SkottieWASM", "GpuTess", "DMSAAStats", "Mskp", "Docker", "PDF",
				"Puppeteer", "SkottieFrames", "RenderSKP", "CanvasPerf", "AllPathsVolatile",
				"WebGL2", "i5", "OldestSupportedSkpVersion", "NeverYield"}
			keep := make([]string, 0, len(ec))
			for _, part := range ec {
				if !In(part, ignore) {
					keep = append(keep, part)
				}
			}
			ec = keep
		}
		if b.matchOs("Android") {
			if !In("Android", ec) {
				ec = append([]string{"Android"}, ec...)
			}
			task_os = COMPILE_TASK_NAME_OS_LINUX
		} else if b.os("ChromeOS") {
			ec = append([]string{"Chromebook", "GLES"}, ec...)
			task_os = COMPILE_TASK_NAME_OS_LINUX
		} else if b.os("iOS") {
			ec = append([]string{task_os}, ec...)
			task_os = "Mac"
		} else if b.matchOs("Win") {
			task_os = "Win"
		} else if b.compiler("GCC") {
			// GCC compiles are now on a Docker container. We use the same OS and
			// version to compile as to test.
			ec = append(ec, "Docker")
		} else if b.matchOs("Debian11") {
			// We compile using the Debian11 machines in the skolo.
			task_os = "Debian11"
		} else if b.matchOs("Ubuntu", "Debian") {
			task_os = COMPILE_TASK_NAME_OS_LINUX
		} else if b.matchOs("Mac") {
			task_os = "Mac"
		}
		jobNameMap := map[string]string{
			"role":          "Build",
			"os":            task_os,
			"compiler":      b.parts["compiler"],
			"target_arch":   b.parts["arch"],
			"configuration": b.parts["configuration"],
		}
		if b.extraConfig("PathKit") {
			ec = []string{"PathKit"}
			// We prefer to compile this in the cloud because we have more resources there
			jobNameMap["os"] = "Debian10"
		}
		if b.extraConfig("CanvasKit", "SkottieWASM", "Puppeteer") {
			if b.cpu() {
				ec = []string{"CanvasKit_CPU"}
			} else {
				ec = []string{"CanvasKit"}
			}
			// We prefer to compile this in the cloud because we have more resources there
			jobNameMap["os"] = "Debian10"
		}
		if len(ec) > 0 {
			jobNameMap["extra_config"] = strings.Join(ec, "_")
		}
		name, err := b.jobNameSchema.MakeJobName(jobNameMap)
		if err != nil {
			log.Fatal(err)
		}
		return name
	} else if b.role("BuildStats") {
		return strings.Replace(b.Name, "BuildStats", "Build", 1)
	} else if b.role("CodeSize") {
		return codesizeTaskNameRegexp.ReplaceAllString(b.Name, "Build-")
	} else {
		return b.Name
	}
}

// swarmDimensions generates swarming bot dimensions for the given task.
func (b *taskBuilder) swarmDimensions() {
	if b.cfg.SwarmDimensions != nil {
		dims := b.cfg.SwarmDimensions(b.parts)
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
	"GalaxyS7_G930FD": {"herolte", "R16NW_G930FXXS2ERH6"}, // This is Oreo.
	"GalaxyS9":        {"starlte", "QP1A.190711.020"},     // This is Android10.
	"GalaxyS20":       {"exynos990", "QP1A.190711.020"},
	"JioNext":         {"msm8937", "RKQ1.210602.002"},
	"Mokey":           {"mokey", "UDC_11161052"},
	"MokeyGo32":       {"mokey_go32", "UQ1A.240105.003.A1_11159138"},
	"Nexus5":          {"hammerhead", "M4B30Z_3437181"},
	"Nexus7":          {"grouper", "LMY47V_1836172"}, // 2012 Nexus 7
	"P30":             {"HWELE", "HUAWEIELE-L29"},
	"Pixel2XL":        {"taimen", "PPR1.180610.009"},
	"Pixel3":          {"blueline", "PQ1A.190105.004"},
	"Pixel3a":         {"sargo", "QP1A.190711.020"},
	"Pixel4":          {"flame", "RPB2.200611.009"},       // R Preview
	"Pixel4a":         {"sunfish", "AOSP.MASTER_7819821"}, // Pixel4a flashed with an Android HWASan build.
	"Pixel4XL":        {"coral", "QD1A.190821.011.C4"},
	"Pixel5":          {"redfin", "RD1A.200810.022.A4"},
	"Pixel6":          {"oriole", "SD1A.210817.037"},
	"Pixel7":          {"cheetah", "TD1A.221105.002"},
	"TecnoSpark3Pro":  {"TECNO-KB8", "PPR1.180610.011"},
	"Wembley":         {"wembley", "SP2A.220505.008"},
}

// defaultSwarmDimensions generates default swarming bot dimensions for the given task.
func (b *taskBuilder) defaultSwarmDimensions() {
	d := map[string]string{
		"pool": b.cfg.Pool,
	}
	if os, ok := b.parts["os"]; ok {
		d["os"], ok = map[string]string{
			"Android":    "Android",
			"Android12":  "Android",
			"ChromeOS":   "ChromeOS",
			"Debian9":    DEFAULT_OS_LINUX_GCE, // Runs in Deb9 Docker.
			"Debian10":   DEFAULT_OS_LINUX_GCE,
			"Debian11":   DEBIAN_11_OS,
			"Mac":        DEFAULT_OS_MAC,
			"Mac10.12":   "Mac-10.12",
			"Mac10.13":   "Mac-10.13.6",
			"Mac10.14":   "Mac-10.14",
			"Mac10.15.1": "Mac-10.15.1",
			"Mac10.15.7": "Mac-10.15.7", // Same as 'Mac', but explicit.
			"Mac11":      "Mac-11.4",
			"Mac12":      "Mac-12",
			"Mac13":      "Mac-13",
			"Mokey":      "Android",
			"MokeyGo32":  "Android",
			"Ubuntu18":   "Ubuntu-18.04",
			"Win":        DEFAULT_OS_WIN,
			"Win10":      "Windows-10-19045",
			"Win2019":    DEFAULT_OS_WIN,
			"iOS":        "iOS-13.3.1",
		}[os]
		if !ok {
			log.Fatalf("Entry %q not found in OS mapping.", os)
		}
		if os == "Debian11" && b.extraConfig("Docker") {
			d["os"] = DEFAULT_OS_LINUX_GCE
		}
		if os == "Win10" && b.parts["model"] == "Golo" {
			// ChOps-owned machines have Windows 10 21h1.
			d["os"] = "Windows-10-19043"
		}
		if b.parts["model"] == "iPhone11" {
			d["os"] = "iOS-13.6"
		}
		if b.parts["model"] == "iPadPro" {
			d["os"] = "iOS-13.6"
		}
	} else {
		d["os"] = DEFAULT_OS_DEBIAN
	}
	if b.role("Test", "Perf") {
		if b.os("Android") {
			// For Android, the device type is a better dimension
			// than CPU or GPU.
			deviceInfo, ok := androidDeviceInfos[b.parts["model"]]
			if !ok {
				log.Fatalf("Entry %q not found in Android mapping.", b.parts["model"])
			}
			d["device_type"] = deviceInfo[0]
			d["device_os"] = deviceInfo[1]

			// Tests using Android's HWAddress Sanitizer require an HWASan build of Android.
			// See https://developer.android.com/ndk/guides/hwasan.
			if b.extraConfig("HWASAN") {
				d["android_hwasan_build"] = "1"
			}
		} else if b.os("Android12") {
			// For Android, the device type is a better dimension
			// than CPU or GPU.
			deviceInfo, ok := map[string][]string{
				"Pixel5": {"redfin", "SP2A.220305.012"},
			}[b.parts["model"]]
			if !ok {
				log.Fatalf("Entry %q not found in Android mapping.", b.parts["model"])
			}
			d["device_type"] = deviceInfo[0]
			d["device_os"] = deviceInfo[1]

			// Tests using Android's HWAddress Sanitizer require an HWASan build of Android.
			// See https://developer.android.com/ndk/guides/hwasan.
			if b.extraConfig("HWASAN") {
				d["android_hwasan_build"] = "1"
			}
		} else if b.os("iOS") {
			device, ok := map[string]string{
				"iPadMini4": "iPad5,1",
				"iPhone7":   "iPhone9,1",
				"iPhone8":   "iPhone10,1",
				"iPhone11":  "iPhone12,1",
				"iPadPro":   "iPad6,3",
			}[b.parts["model"]]
			if !ok {
				log.Fatalf("Entry %q not found in iOS mapping.", b.parts["model"])
			}
			d["device_type"] = device
		} else if b.cpu() || b.extraConfig("CanvasKit", "Docker", "SwiftShader") {
			modelMapping, ok := map[string]map[string]string{
				"AppleM1": {
					"MacMini9.1": "arm64-64-Apple_M1",
				},
				"AppleIntel": {
					"MacBookPro16.2": "x86-64",
				},
				"AVX": {
					"VMware7.1": "x86-64",
				},
				"AVX2": {
					"GCE":            "x86-64-Haswell_GCE",
					"MacBookAir7.2":  "x86-64-i5-5350U",
					"MacBookPro11.5": "x86-64-i7-4870HQ",
					"MacMini7.1":     "x86-64-i5-4278U",
					"NUC5i7RYH":      "x86-64-i7-5557U",
					"NUC9i7QN":       "x86-64-i7-9750H",
					"NUC11TZi5":      "x86-64-i5-1135G7",
				},
				"AVX512": {
					"GCE":  "x86-64-Skylake_GCE",
					"Golo": "Intel64_Family_6_Model_85_Stepping_7__GenuineIntel",
				},
				"Rome": {
					"GCE": "x86-64-AMD_Rome_GCE",
				},
				"SwiftShader": {
					"GCE": "x86-64-Haswell_GCE",
				},
			}[b.parts["cpu_or_gpu_value"]]
			if !ok {
				log.Fatalf("Entry %q not found in CPU mapping.", b.parts["cpu_or_gpu_value"])
			}
			cpu, ok := modelMapping[b.parts["model"]]
			if !ok {
				log.Fatalf("Entry %q not found in %q model mapping.", b.parts["model"], b.parts["cpu_or_gpu_value"])
			}
			d["cpu"] = cpu
			if b.model("GCE") && b.matchOs("Debian") {
				d["os"] = DEFAULT_OS_LINUX_GCE
			}
			if b.model("GCE") && d["cpu"] == "x86-64-Haswell_GCE" {
				d["machine_type"] = MACHINE_TYPE_MEDIUM
			}
		} else {
			// It's a GPU job.
			if b.matchOs("Win") {
				gpu, ok := map[string]string{
					// At some point this might use the device ID, but for now it's like Chromebooks.
					"GTX660":        "10de:11c0-26.21.14.4120",
					"GTX960":        "10de:1401-31.0.15.3699",
					"IntelHD4400":   "8086:0a16-20.19.15.4963",
					"IntelIris540":  "8086:1926-31.0.101.2115",
					"IntelIris6100": "8086:162b-20.19.15.4963",
					"IntelIris655":  "8086:3ea5-26.20.100.7463",
					"IntelIrisXe":   "8086:9a49-31.0.101.4575",
					"RadeonHD7770":  "1002:683d-26.20.13031.18002",
					"RadeonR9M470X": "1002:6646-26.20.13031.18002",
					"QuadroP400":    "10de:1cb3-30.0.15.1179",
					"RadeonVega6":   "1002:1636-31.0.14057.5006",
					"RTX3060":       "10de:2489-31.0.15.3699",
				}[b.parts["cpu_or_gpu_value"]]
				if !ok {
					log.Fatalf("Entry %q not found in Win GPU mapping.", b.parts["cpu_or_gpu_value"])
				}
				d["gpu"] = gpu
			} else if b.isLinux() {
				gpu, ok := map[string]string{
					// Intel drivers come from CIPD, so no need to specify the version here.
					"IntelBayTrail": "8086:0f31",
					"IntelHD2000":   "8086:0102",
					"IntelHD405":    "8086:22b1",
					"IntelIris640":  "8086:5926",
					"QuadroP400":    "10de:1cb3-510.60.02",
					"RTX3060":       "10de:2489-470.182.03",
					"IntelIrisXe":   "8086:9a49",
					"RadeonVega6":   "1002:1636",
				}[b.parts["cpu_or_gpu_value"]]
				if !ok {
					log.Fatalf("Entry %q not found in Ubuntu GPU mapping.", b.parts["cpu_or_gpu_value"])
				}
				d["gpu"] = gpu

				if b.matchOs("Debian11") {
					d["os"] = DEBIAN_11_OS
				} else if b.matchOs("Debian") {
					// The Debian10 machines in the skolo are 10.10, not 10.3.
					d["os"] = DEFAULT_OS_DEBIAN
				}
				if b.parts["cpu_or_gpu_value"] == "IntelIrisXe" {
					// The Intel Iris Xe devices are Debian 11.3.
					d["os"] = "Debian-bookworm/sid"
				}
			} else if b.matchOs("Mac") {
				gpu, ok := map[string]string{
					"AppleM1":       "AppleM1",
					"IntelHD6000":   "8086:1626",
					"IntelHD615":    "8086:591e",
					"IntelIris5100": "8086:0a2e",
					"IntelIrisPlus": "8086:8a53",
					"RadeonHD8870M": "1002:6821-4.0.20-3.2.8",
				}[b.parts["cpu_or_gpu_value"]]
				if !ok {
					log.Fatalf("Entry %q not found in Mac GPU mapping.", b.parts["cpu_or_gpu_value"])
				}
				if gpu == "AppleM1" {
					// No GPU dimension yet, but we can constrain by CPU.
					d["cpu"] = "arm64-64-Apple_M1"
				} else {
					d["gpu"] = gpu
				}
				// We have two different types of MacMini7,1 with the same GPU but different CPUs.
				if b.gpu("IntelIris5100") {
					if b.extraConfig("i5") {
						// If we say "i5", run on our MacMini7,1s in the Skolo:
						d["cpu"] = "x86-64-i5-4278U"
					} else {
						// Otherwise, run on Golo machines, just because that's
						// where those jobs have always run. Plus, some of them
						// are Perf jobs, which we want to keep consistent.
						d["cpu"] = "x86-64-i7-4578U"
					}
				}
			} else if b.os("ChromeOS") {
				version, ok := map[string]string{
					"IntelUHDGraphics605": "15236.2.0",
					"RadeonVega3":         "14233.0.0",
					"Adreno618":           "14150.39.0",
					"MaliT860":            "14092.77.0",
				}[b.parts["cpu_or_gpu_value"]]
				if !ok {
					log.Fatalf("Entry %q not found in ChromeOS GPU mapping.", b.parts["cpu_or_gpu_value"])
				}
				d["gpu"] = b.parts["cpu_or_gpu_value"]
				d["release_version"] = version
			} else {
				log.Fatalf("Unknown GPU mapping for OS %q.", b.parts["os"])
			}
		}
	} else {
		if d["os"] == DEBIAN_11_OS {
			// The Debian11 compile machines in the skolo have
			// GPUs, but we still use them for compiles also.

			// Dodge Raspberry Pis.
			d["cpu"] = "x86-64"
			// Target the RTX3060 Intel machines, as they are beefy and we have
			// 20 of them, and they are setup to compile.
			d["gpu"] = "10de:2489"
		} else {
			d["gpu"] = "none"
		}
		if d["os"] == DEFAULT_OS_LINUX_GCE {
			if b.extraConfig("CanvasKit", "CMake", "Docker", "PathKit") || b.role("BuildStats", "CodeSize") {
				b.linuxGceDimensions(MACHINE_TYPE_MEDIUM)
				return
			}
			// Use many-core machines for Build tasks.
			b.linuxGceDimensions(MACHINE_TYPE_LARGE)
			return
		} else if d["os"] == DEFAULT_OS_WIN {
			// Windows CPU bots.
			d["cpu"] = "x86-64-Haswell_GCE"
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
	b.addTask(BUNDLE_RECIPES_NAME, func(b *taskBuilder) {
		b.cipd(specs.CIPD_PKGS_GIT_LINUX_AMD64...)
		b.cmd("/bin/bash", "skia/infra/bots/bundle_recipes.sh", specs.PLACEHOLDER_ISOLATED_OUTDIR)
		b.linuxGceDimensions(MACHINE_TYPE_SMALL)
		b.idempotent()
		b.cas(CAS_RECIPES)
		b.usesPython()
		b.addToPATH("cipd_bin_packages", "cipd_bin_packages/bin")
	})
	return BUNDLE_RECIPES_NAME
}

// buildTaskDrivers generates the task to compile the task driver code to run on
// all platforms. Returns the name of the task, which may be added as a
// dependency.
func (b *jobBuilder) buildTaskDrivers(goos, goarch string) string {
	name := BUILD_TASK_DRIVERS_PREFIX + "_" + goos + "_" + goarch
	b.addTask(name, func(b *taskBuilder) {
		b.cmd("/bin/bash", "skia/infra/bots/build_task_drivers.sh",
			specs.PLACEHOLDER_ISOLATED_OUTDIR,
			goos+"_"+goarch)
		b.linuxGceDimensions(MACHINE_TYPE_MEDIUM)
		b.usesBazel("linux_x64")
		b.idempotent()
		b.cas(CAS_TASK_DRIVERS)
	})
	return name
}

// createDockerImage creates the specified docker image. Returns the name of the
// generated task.
func (b *jobBuilder) createDockerImage(wasm bool) string {
	// First, derive the name of the task.
	imageName := "skia-release"
	taskName := "Housekeeper-PerCommit-CreateDockerImage_Skia_Release"
	if wasm {
		imageName = "skia-wasm-release"
		taskName = "Housekeeper-PerCommit-CreateDockerImage_Skia_WASM_Release"
	}
	imageDir := path.Join("docker", imageName)

	// Add the task.
	b.addTask(taskName, func(b *taskBuilder) {
		// TODO(borenet): Make this task not use Git.
		b.usesGit()
		b.cmd(
			"./build_push_docker_image",
			"--image_name", fmt.Sprintf("gcr.io/skia-public/%s", imageName),
			"--dockerfile_dir", imageDir,
			"--project_id", "skia-swarming-bots",
			"--task_id", specs.PLACEHOLDER_TASK_ID,
			"--task_name", b.Name,
			"--workdir", ".",
			"--gerrit_project", "skia",
			"--gerrit_url", "https://skia-review.googlesource.com",
			"--repo", specs.PLACEHOLDER_REPO,
			"--revision", specs.PLACEHOLDER_REVISION,
			"--patch_issue", specs.PLACEHOLDER_ISSUE,
			"--patch_set", specs.PLACEHOLDER_PATCHSET,
			"--patch_server", specs.PLACEHOLDER_CODEREVIEW_SERVER,
			"--swarm_out_dir", specs.PLACEHOLDER_ISOLATED_OUTDIR,
		)
		b.dep(b.buildTaskDrivers("linux", "amd64"))
		b.addToPATH("cipd_bin_packages", "cipd_bin_packages/bin")
		b.cas(CAS_EMPTY)
		b.serviceAccount(b.cfg.ServiceAccountCompile)
		b.linuxGceDimensions(MACHINE_TYPE_MEDIUM)
		b.usesDocker()
		b.cache(CACHES_DOCKER...)
		b.timeout(time.Hour)
	})
	return taskName
}

// createPushAppsFromSkiaDockerImage creates and pushes docker images of some apps
// (eg: fiddler, api) using the skia-release docker image.
func (b *jobBuilder) createPushAppsFromSkiaDockerImage() {
	b.addTask(b.Name, func(b *taskBuilder) {
		// TODO(borenet): Make this task not use Git.
		b.usesGit()
		b.cmd(
			"./push_apps_from_skia_image",
			"--project_id", "skia-swarming-bots",
			"--task_id", specs.PLACEHOLDER_TASK_ID,
			"--task_name", b.Name,
			"--workdir", ".",
			"--repo", specs.PLACEHOLDER_REPO,
			"--revision", specs.PLACEHOLDER_REVISION,
			"--patch_issue", specs.PLACEHOLDER_ISSUE,
			"--patch_set", specs.PLACEHOLDER_PATCHSET,
			"--patch_server", specs.PLACEHOLDER_CODEREVIEW_SERVER,
			"--bazel_cache_dir", bazelCacheDirOnGCELinux,
		)
		b.dep(b.buildTaskDrivers("linux", "amd64"))
		b.dep(b.createDockerImage(false))
		b.addToPATH("cipd_bin_packages", "cipd_bin_packages/bin")
		b.cas(CAS_EMPTY)
		b.usesBazel("linux_x64")
		b.serviceAccount(b.cfg.ServiceAccountCompile)
		b.linuxGceDimensions(MACHINE_TYPE_MEDIUM)
		b.usesDocker()
		b.cache(CACHES_DOCKER...)
		b.timeout(2 * time.Hour)
	})
}

// createPushBazelAppsFromWASMDockerImage pushes those infra apps that have been ported to Bazel
// and require assets built in the WASM docker image.
// TODO(kjlubick) The inputs to this job should not be the docker build, but a Bazel build.
func (b *jobBuilder) createPushBazelAppsFromWASMDockerImage() {
	b.addTask(b.Name, func(b *taskBuilder) {
		// TODO(borenet): Make this task not use Git.
		b.usesGit()
		b.cmd(
			"--project_id", "skia-swarming-bots",
			"--task_id", specs.PLACEHOLDER_TASK_ID,
			"--task_name", b.Name,
			"--workdir", ".",
			"--skia_revision", specs.PLACEHOLDER_REVISION,
			"--bazel_cache_dir", bazelCacheDirOnGCELinux,
		)
		b.dep(b.buildTaskDrivers("linux", "amd64"))
		b.dep(b.createDockerImage(true))
		b.addToPATH("cipd_bin_packages", "cipd_bin_packages/bin")
		b.cas(CAS_EMPTY)
		b.usesBazel("linux_x64")
		b.serviceAccount(b.cfg.ServiceAccountCompile)
		b.linuxGceDimensions(MACHINE_TYPE_MEDIUM)
		b.usesDocker()
		b.cache(CACHES_DOCKER...)
	})
}

var iosRegex = regexp.MustCompile(`os:iOS-(.*)`)

func (b *taskBuilder) maybeAddIosDevImage() {
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
			default:
				log.Fatalf("Unable to determine correct ios-dev-image asset for %s. If %s is a new iOS release, you must add a CIPD package containing the corresponding iOS dev image; see ios-dev-image-11.4 for an example.", b.Name, m[1])
			}
			b.asset(asset)
			break
		} else if strings.Contains(dim, "iOS") {
			log.Fatalf("Must specify iOS version for %s to obtain correct dev image; os dimension is missing version: %s", b.Name, dim)
		}
	}
}

// compile generates a compile task. Returns the name of the compile task.
func (b *jobBuilder) compile() string {
	name := b.deriveCompileTaskName()
	if b.extraConfig("WasmGMTests") {
		b.compileWasmGMTests(name)
	} else {
		b.addTask(name, func(b *taskBuilder) {
			recipe := "compile"
			casSpec := CAS_COMPILE
			if b.extraConfig("NoDEPS", "CMake", "Flutter", "NoPatch", "Vello", "Fontations") {
				recipe = "sync_and_compile"
				casSpec = CAS_RUN_RECIPE
				b.recipeProps(EXTRA_PROPS)
				b.usesGit()
				if !b.extraConfig("NoDEPS") {
					b.cache(CACHES_WORKDIR...)
				}
			} else {
				b.idempotent()
			}
			if b.extraConfig("NoPatch") {
				b.kitchenTask(recipe, OUTPUT_BUILD_NOPATCH)
			} else {
				b.kitchenTask(recipe, OUTPUT_BUILD)
			}
			b.cas(casSpec)
			b.serviceAccount(b.cfg.ServiceAccountCompile)
			b.swarmDimensions()
			if b.extraConfig("Docker", "LottieWeb", "CMake") || b.compiler("EMCC") {
				b.usesDocker()
				b.cache(CACHES_DOCKER...)
			}
			if b.extraConfig("Dawn") {
				// https://dawn.googlesource.com/dawn/+/516701da8184655a47c92a573cc84da7db5e69d4/generator/dawn_version_generator.py#21
				b.usesGit()
			}

			// Android bots require a toolchain.
			if b.extraConfig("Android") {
				if b.matchOs("Mac") {
					b.asset("android_ndk_darwin")
				} else if b.matchOs("Win") {
					pkg := b.MustGetCipdPackageFromAsset("android_ndk_windows")
					pkg.Path = "n"
					b.cipd(pkg)
				} else {
					b.asset("android_ndk_linux")
				}
			} else if b.extraConfig("Chromebook") {
				b.asset("clang_linux")
				if b.arch("x86_64") {
					b.asset("chromebook_x86_64_gles")
				} else if b.arch("arm") {
					b.asset("armhf_sysroot")
					b.asset("chromebook_arm_gles")
				}
			} else if b.isLinux() {
				if b.compiler("Clang") {
					b.asset("clang_linux")
				}
				if b.extraConfig("SwiftShader") {
					b.asset("cmake_linux")
				}
				b.asset("ccache_linux")
				b.usesCCache()
				if b.extraConfig("Vello") || b.extraConfig("Fontations") {
					b.usesBazel("linux_x64")
					b.attempts(1)
				}
			} else if b.matchOs("Win") {
				b.asset("win_toolchain")
				if b.compiler("Clang") {
					b.asset("clang_win")
				}
				if b.extraConfig("DWriteCore") {
					b.asset("dwritecore")
				}
			} else if b.matchOs("Mac") {
				b.cipd(CIPD_PKGS_XCODE...)
				b.Spec.Caches = append(b.Spec.Caches, &specs.Cache{
					Name: "xcode",
					Path: "cache/Xcode.app",
				})
				b.asset("ccache_mac")
				b.usesCCache()
				if b.extraConfig("iOS") {
					b.asset("provisioning_profile_ios")
				}
				if b.extraConfig("Vello") || b.extraConfig("Fontations") {
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
	cmd := []string{
		"./recreate_skps",
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
	if b.matchExtraConfig("DryRun") {
		cmd = append(cmd, "--dry_run")
	}
	b.addTask(b.Name, func(b *taskBuilder) {
		b.cas(CAS_RECREATE_SKPS)
		b.dep(b.buildTaskDrivers("linux", "amd64"))
		b.dep("Build-Debian10-Clang-x86_64-Release") // To get DM.
		b.cmd(cmd...)
		b.cipd(CIPD_PKG_LUCI_AUTH)
		b.serviceAccount(b.cfg.ServiceAccountRecreateSKPs)
		b.dimension(
			"pool:SkiaCT",
			fmt.Sprintf("os:%s", DEFAULT_OS_LINUX_GCE),
		)
		b.usesGo()
		b.cache(CACHES_WORKDIR...)
		b.timeout(6 * time.Hour)
		b.usesPython()
		b.addToPATH("cipd_bin_packages", "cipd_bin_packages/bin")
		b.attempts(2)
	})
}

// checkGeneratedFiles verifies that no generated SKSL files have been edited by hand, and that
// we do not get any diffs after regenerating all files (go generate, Gazelle, etc.).
func (b *jobBuilder) checkGeneratedFiles() {
	b.addTask(b.Name, func(b *taskBuilder) {
		b.cas(CAS_BAZEL)
		b.dep(b.buildTaskDrivers("linux", "amd64"))
		b.cmd("./check_generated_files",
			"--local=false",
			"--git_path=cipd_bin_packages/git",
			"--project_id", "skia-swarming-bots",
			"--task_id", specs.PLACEHOLDER_TASK_ID,
			"--task_name", b.Name,
			"--bazel_cache_dir", bazelCacheDirOnGCELinux,
			"--bazel_arg=--config=for_linux_x64_with_rbe",
			"--bazel_arg=--jobs=100",
		)
		b.cipd(specs.CIPD_PKGS_GIT_LINUX_AMD64...)
		b.usesBazel("linux_x64")
		b.linuxGceDimensions(MACHINE_TYPE_MEDIUM)
		b.serviceAccount(b.cfg.ServiceAccountHousekeeper)
	})
}

// goLinters runs various Go linters (gofmt, errcheck, etc.) and fails if there are any errors or
// diffs.
func (b *jobBuilder) goLinters() {
	b.addTask(b.Name, func(b *taskBuilder) {
		b.cas(CAS_BAZEL)
		b.dep(b.buildTaskDrivers("linux", "amd64"))
		b.cmd("./go_linters",
			"--local=false",
			"--git_path=cipd_bin_packages/git",
			"--project_id", "skia-swarming-bots",
			"--task_id", specs.PLACEHOLDER_TASK_ID,
			"--task_name", b.Name,
			"--bazel_cache_dir", bazelCacheDirOnGCELinux,
			"--bazel_arg=--config=for_linux_x64_with_rbe",
			"--bazel_arg=--jobs=100",
		)
		b.cipd(specs.CIPD_PKGS_GIT_LINUX_AMD64...)
		b.usesBazel("linux_x64")
		b.linuxGceDimensions(MACHINE_TYPE_MEDIUM)
		b.serviceAccount(b.cfg.ServiceAccountHousekeeper)
	})
}

// checkGnToBp verifies that the gn_to_bp.py script continues to work.
func (b *jobBuilder) checkGnToBp() {
	b.addTask(b.Name, func(b *taskBuilder) {
		b.cas(CAS_COMPILE)
		b.dep(b.buildTaskDrivers("linux", "amd64"))
		b.cmd("./run_gn_to_bp",
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
	b.addTask(b.Name, func(b *taskBuilder) {
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
	b.addTask(b.Name, func(b *taskBuilder) {
		b.cas(CAS_EMPTY)
		b.dep(b.buildTaskDrivers("linux", "amd64"))
		b.cmd("./g3_canary",
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
		b.cipd(CIPD_PKG_LUCI_AUTH)
		b.serviceAccount("skia-g3-framework-compile@skia-swarming-bots.iam.gserviceaccount.com")
		b.timeout(3 * time.Hour)
		b.attempts(1)
	})
}

// infra generates an infra_tests task.
func (b *jobBuilder) infra() {
	b.addTask(b.Name, func(b *taskBuilder) {
		if b.matchOs("Win") || b.matchExtraConfig("Win") {
			b.dimension(
				// Specify CPU to avoid running builds on bots with a more unique CPU.
				"cpu:x86-64-Haswell_GCE",
				"gpu:none",
				fmt.Sprintf("machine_type:%s", MACHINE_TYPE_MEDIUM), // We don't have any small Windows instances.
				fmt.Sprintf("os:%s", DEFAULT_OS_WIN),
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
	b.addTask(b.Name, func(b *taskBuilder) {
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
	if b.release() && !b.arch("x86_64") {
		uploadName := fmt.Sprintf("%s%s%s", PREFIX_UPLOAD, b.jobNameSchema.Sep, b.Name)
		depName := b.Name
		b.addTask(uploadName, func(b *taskBuilder) {
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
	if b.extraConfig("Android") {
		compileTaskNameNoPatch += "_NoPatch" // add a second "extra config"
	} else {
		compileTaskNameNoPatch += "-NoPatch" // add the only "extra config"
	}

	bloatyCipdPkg := b.MustGetCipdPackageFromAsset("bloaty")

	b.addTask(b.Name, func(b *taskBuilder) {
		b.cas(CAS_EMPTY)
		b.dep(b.buildTaskDrivers("linux", "amd64"), compileTaskName)
		b.dep(b.buildTaskDrivers("linux", "amd64"), compileTaskNameNoPatch)
		cmd := []string{
			"./codesize",
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
			"--binary_name", b.parts["binary_name"],
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
				"android_ndk_linux/toolchains/arm-linux-androideabi-4.9/prebuilt/linux-x86_64/bin/arm-linux-androideabi-strip")
		} else {
			b.asset("binutils_linux_x64")
			cmd = append(cmd, "--strip_binary", "binutils_linux_x64/strip")
		}
		b.cmd(cmd...)
		b.linuxGceDimensions(MACHINE_TYPE_SMALL)
		b.cache(CACHES_WORKDIR...)
		b.cipd(CIPD_PKG_LUCI_AUTH)
		b.asset("bloaty")
		b.serviceAccount("skia-external-codesize@skia-swarming-bots.iam.gserviceaccount.com")
		b.timeout(20 * time.Minute)
		b.attempts(1)
	})
}

// doUpload indicates whether the given Job should upload its results.
func (b *jobBuilder) doUpload() bool {
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
func (b *taskBuilder) commonTestPerfAssets() {
	// Docker-based tests don't need the standard CIPD assets
	if b.extraConfig("CanvasKit", "PathKit") || (b.role("Test") && b.extraConfig("LottieWeb")) {
		return
	}
	if b.extraConfig("Skpbench") {
		// Skpbench only needs skps
		b.asset("skp", "mskp")
	} else if b.os("Android", "ChromeOS", "iOS") {
		b.asset("skp", "svg", "skimage")
	} else if b.extraConfig("OldestSupportedSkpVersion") {
		b.assetWithVersion("skp", oldestSupportedSkpVersion)
	} else {
		// for desktop machines
		b.asset("skimage", "skp", "svg")
	}

	if b.isLinux() && b.matchExtraConfig("SAN") {
		b.asset("clang_linux")
	}

	if b.isLinux() {
		if b.extraConfig("Vulkan") {
			b.asset("linux_vulkan_sdk")
		}
		if b.matchGpu("Intel") {
			if b.matchGpu("IrisXe") {
				b.asset("mesa_intel_driver_linux_22")
			} else {
				// Use this for legacy drivers that were culled in v22 of Mesa.
				// https://www.phoronix.com/scan.php?page=news_item&px=Mesa-22.0-Drops-OpenSWR
				b.asset("mesa_intel_driver_linux")
			}
		}
	}

	if b.matchOs("Win") && b.extraConfig("DWriteCore") {
		b.asset("dwritecore")
	}
}

// directUpload adds prerequisites for uploading to GCS.
func (b *taskBuilder) directUpload(gsBucket, serviceAccount string) {
	b.recipeProp("gs_bucket", gsBucket)
	b.serviceAccount(serviceAccount)
	b.usesGSUtil()
}

// dm generates a Test task using dm.
func (b *jobBuilder) dm() {
	compileTaskName := ""
	// LottieWeb doesn't require anything in Skia to be compiled.
	if !b.extraConfig("LottieWeb") {
		compileTaskName = b.compile()
	}
	directUpload := false
	b.addTask(b.Name, func(b *taskBuilder) {
		cas := CAS_TEST
		recipe := "test"
		if b.extraConfig("PathKit") {
			cas = CAS_PATHKIT
			recipe = "test_pathkit"
			if b.doUpload() {
				b.directUpload(b.cfg.GsBucketGm, b.cfg.ServiceAccountUploadGM)
				directUpload = true
			}
		} else if b.extraConfig("CanvasKit") {
			cas = CAS_CANVASKIT
			recipe = "test_canvaskit"
			if b.doUpload() {
				b.directUpload(b.cfg.GsBucketGm, b.cfg.ServiceAccountUploadGM)
				directUpload = true
			}
		} else if b.extraConfig("LottieWeb") {
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
			// TODO(http://skbug.com/11785): Windows jobs are unable to extract gsutil.
			// https://bugs.chromium.org/p/chromium/issues/detail?id=1192611
			if b.doUpload() && !b.matchOs("Win") {
				b.directUpload(b.cfg.GsBucketGm, b.cfg.ServiceAccountUploadGM)
				directUpload = true
			}
		}
		b.recipeProp("gold_hashes_url", b.cfg.GoldHashesURL)
		b.recipeProps(EXTRA_PROPS)
		iid := b.internalHardwareLabel()
		iidStr := ""
		if iid != nil {
			iidStr = strconv.Itoa(*iid)
		}
		if recipe == "test" {
			b.dmFlags(iidStr)
		}
		b.kitchenTask(recipe, OUTPUT_TEST)
		b.cas(cas)
		b.swarmDimensions()
		if b.extraConfig("CanvasKit", "Docker", "LottieWeb", "PathKit") {
			b.usesDocker()
		}
		if compileTaskName != "" {
			b.dep(compileTaskName)
		}
		if b.matchOs("Android") && b.extraConfig("ASAN") {
			b.asset("android_ndk_linux")
		}
		if b.extraConfig("NativeFonts") && !b.matchOs("Android") {
			b.needsFontsForParagraphTests()
		}
		if b.extraConfig("Fontations") {
			b.cipd(&specs.CipdPackage{
				Name:    "chromium/third_party/googlefonts_testdata",
				Path:    "googlefonts_testdata",
				Version: "version:20230913",
			})
		}
		b.commonTestPerfAssets()
		if b.matchExtraConfig("Lottie") {
			b.asset("lottie-samples")
		}
		b.expiration(20 * time.Hour)

		b.timeout(4 * time.Hour)
		if b.extraConfig("Valgrind") {
			b.timeout(9 * time.Hour)
			b.expiration(48 * time.Hour)
			b.asset("valgrind")
			// Since Valgrind runs on the same bots as the CQ, we restrict Valgrind to a subset of the bots
			// to ensure there are always bots free for CQ tasks.
			b.dimension("valgrind:1")
		} else if b.extraConfig("MSAN") {
			b.timeout(9 * time.Hour)
		} else if b.arch("x86") && b.debug() {
			// skia:6737
			b.timeout(6 * time.Hour)
		}
		b.maybeAddIosDevImage()
	})

	// Upload results if necessary. TODO(kjlubick): If we do coverage analysis at the same
	// time as normal tests (which would be nice), cfg.json needs to have Coverage removed.
	if b.doUpload() && !directUpload {
		uploadName := fmt.Sprintf("%s%s%s", PREFIX_UPLOAD, b.jobNameSchema.Sep, b.Name)
		depName := b.Name
		b.addTask(uploadName, func(b *taskBuilder) {
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
	b.addTask(b.Name, func(b *taskBuilder) {
		b.cas(CAS_EMPTY)
		b.dep(b.buildTaskDrivers("linux", "amd64"))
		b.cmd("./canary",
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
		b.cipd(CIPD_PKG_LUCI_AUTH)
		b.serviceAccount(b.cfg.ServiceAccountCanary)
		b.timeout(3 * time.Hour)
		b.attempts(1)
	})
}

// puppeteer generates a task that uses TaskDrivers combined with a node script and puppeteer to
// benchmark something using Chromium (e.g. CanvasKit, LottieWeb).
func (b *jobBuilder) puppeteer() {
	compileTaskName := b.compile()
	b.addTask(b.Name, func(b *taskBuilder) {
		b.defaultSwarmDimensions()
		b.usesNode()
		b.cipd(CIPD_PKG_LUCI_AUTH)
		b.dep(b.buildTaskDrivers("linux", "amd64"), compileTaskName)
		b.output(OUTPUT_PERF)
		b.timeout(60 * time.Minute)
		b.cas(CAS_PUPPETEER)
		b.serviceAccount(b.cfg.ServiceAccountCompile)

		webglversion := "2"
		if b.extraConfig("WebGL1") {
			webglversion = "1"
		}

		if b.extraConfig("SkottieFrames") {
			b.cmd(
				"./perf_puppeteer_skottie_frames",
				"--project_id", "skia-swarming-bots",
				"--git_hash", specs.PLACEHOLDER_REVISION,
				"--task_id", specs.PLACEHOLDER_TASK_ID,
				"--task_name", b.Name,
				"--canvaskit_bin_path", "./build",
				"--lotties_path", "./lotties_with_assets",
				"--node_bin_path", "./node/node/bin",
				"--benchmark_path", "./tools/perf-canvaskit-puppeteer",
				"--output_path", OUTPUT_PERF,
				"--os_trace", b.parts["os"],
				"--model_trace", b.parts["model"],
				"--cpu_or_gpu_trace", b.parts["cpu_or_gpu"],
				"--cpu_or_gpu_value_trace", b.parts["cpu_or_gpu_value"],
				"--webgl_version", webglversion, // ignore when running with cpu backend
			)
			b.needsLottiesWithAssets()
		} else if b.extraConfig("RenderSKP") {
			b.cmd(
				"./perf_puppeteer_render_skps",
				"--project_id", "skia-swarming-bots",
				"--git_hash", specs.PLACEHOLDER_REVISION,
				"--task_id", specs.PLACEHOLDER_TASK_ID,
				"--task_name", b.Name,
				"--canvaskit_bin_path", "./build",
				"--skps_path", "./skp",
				"--node_bin_path", "./node/node/bin",
				"--benchmark_path", "./tools/perf-canvaskit-puppeteer",
				"--output_path", OUTPUT_PERF,
				"--os_trace", b.parts["os"],
				"--model_trace", b.parts["model"],
				"--cpu_or_gpu_trace", b.parts["cpu_or_gpu"],
				"--cpu_or_gpu_value_trace", b.parts["cpu_or_gpu_value"],
				"--webgl_version", webglversion,
			)
			b.asset("skp")
		} else if b.extraConfig("CanvasPerf") { // refers to the canvas_perf.js test suite
			b.cmd(
				"./perf_puppeteer_canvas",
				"--project_id", "skia-swarming-bots",
				"--git_hash", specs.PLACEHOLDER_REVISION,
				"--task_id", specs.PLACEHOLDER_TASK_ID,
				"--task_name", b.Name,
				"--canvaskit_bin_path", "./build",
				"--node_bin_path", "./node/node/bin",
				"--benchmark_path", "./tools/perf-canvaskit-puppeteer",
				"--output_path", OUTPUT_PERF,
				"--os_trace", b.parts["os"],
				"--model_trace", b.parts["model"],
				"--cpu_or_gpu_trace", b.parts["cpu_or_gpu"],
				"--cpu_or_gpu_value_trace", b.parts["cpu_or_gpu_value"],
				"--webgl_version", webglversion,
			)
			b.asset("skp")
		}

	})

	// Upload results to Perf after.
	// TODO(kjlubick,borenet) deduplicate this with the logic in perf().
	uploadName := fmt.Sprintf("%s%s%s", PREFIX_UPLOAD, b.jobNameSchema.Sep, b.Name)
	depName := b.Name
	b.addTask(uploadName, func(b *taskBuilder) {
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
	if !b.extraConfig("LottieWeb") {
		compileTaskName = b.compile()
	}
	doUpload := !b.debug() && b.doUpload()
	b.addTask(b.Name, func(b *taskBuilder) {
		recipe := "perf"
		cas := CAS_PERF
		if b.extraConfig("Skpbench") {
			recipe = "skpbench"
			cas = CAS_SKPBENCH
		} else if b.extraConfig("PathKit") {
			cas = CAS_PATHKIT
			recipe = "perf_pathkit"
		} else if b.extraConfig("CanvasKit") {
			cas = CAS_CANVASKIT
			recipe = "perf_canvaskit"
		} else if b.extraConfig("SkottieTracing") {
			recipe = "perf_skottietrace"
		} else if b.extraConfig("SkottieWASM") {
			recipe = "perf_skottiewasm_lottieweb"
			cas = CAS_SKOTTIE_WASM
		} else if b.extraConfig("LottieWeb") {
			recipe = "perf_skottiewasm_lottieweb"
			cas = CAS_LOTTIE_WEB
		}
		b.recipeProps(EXTRA_PROPS)
		if recipe == "perf" {
			b.nanobenchFlags(doUpload)
		} else if recipe == "skpbench" {
			b.skpbenchFlags()
		}
		b.kitchenTask(recipe, OUTPUT_PERF)
		b.cas(cas)
		b.swarmDimensions()
		if b.extraConfig("Docker") {
			b.usesDocker()
		}
		if compileTaskName != "" {
			b.dep(compileTaskName)
		}
		b.commonTestPerfAssets()
		b.expiration(20 * time.Hour)
		b.timeout(4 * time.Hour)

		if b.extraConfig("Valgrind") {
			b.timeout(9 * time.Hour)
			b.expiration(48 * time.Hour)
			b.asset("valgrind")
			// Since Valgrind runs on the same bots as the CQ, we restrict Valgrind to a subset of the bots
			// to ensure there are always bots free for CQ tasks.
			b.dimension("valgrind:1")
		} else if b.extraConfig("MSAN") {
			b.timeout(9 * time.Hour)
		} else if b.parts["arch"] == "x86" && b.parts["configuration"] == "Debug" {
			// skia:6737
			b.timeout(6 * time.Hour)
		} else if b.extraConfig("LottieWeb", "SkottieWASM") {
			b.asset("node", "lottie-samples")
		} else if b.matchExtraConfig("SkottieTracing") {
			b.needsLottiesWithAssets()
		} else if b.matchExtraConfig("Skottie") {
			b.asset("lottie-samples")
		}

		if b.matchOs("Android") && b.cpu() {
			b.asset("text_blob_traces")
		}
		b.maybeAddIosDevImage()

		iid := b.internalHardwareLabel()
		if iid != nil {
			b.Spec.Command = append(b.Spec.Command, fmt.Sprintf("internal_hardware_label=%d", *iid))
		}
	})

	// Upload results if necessary.
	if doUpload {
		uploadName := fmt.Sprintf("%s%s%s", PREFIX_UPLOAD, b.jobNameSchema.Sep, b.Name)
		depName := b.Name
		b.addTask(uploadName, func(b *taskBuilder) {
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
	b.addTask(b.Name, func(b *taskBuilder) {
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
			Version: "git_revision:1a28cb094add070f4beefd052725223930d8c27a",
		})
	})
}

// compileWasmGMTests uses a task driver to compile the GMs and unit tests for Web Assembly (WASM).
// We can use the same build for both CPU and GPU tests since the latter requires the code for the
// former anyway.
func (b *jobBuilder) compileWasmGMTests(compileName string) {
	b.addTask(compileName, func(b *taskBuilder) {
		b.attempts(1)
		b.usesDocker()
		b.linuxGceDimensions(MACHINE_TYPE_MEDIUM)
		b.cipd(CIPD_PKG_LUCI_AUTH)
		b.dep(b.buildTaskDrivers("linux", "amd64"))
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
			"./compile_wasm_gm_tests",
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

	b.addTask(b.Name, func(b *taskBuilder) {
		b.attempts(1)
		b.usesNode()
		b.swarmDimensions()
		b.cipd(CIPD_PKG_LUCI_AUTH)
		b.cipd(CIPD_PKGS_GOLDCTL)
		b.dep(b.buildTaskDrivers("linux", "amd64"))
		b.dep(compileTaskName)
		b.timeout(60 * time.Minute)
		b.cas(CAS_WASM_GM)
		b.serviceAccount(b.cfg.ServiceAccountUploadGM)
		b.cmd(
			"./run_wasm_gm_tests",
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
			// TODO(kjlubick, nifong) Make these not hard coded if we change the configs we test on.
			"--webgl_version", "2", // 0 means CPU ; this flag controls cpu_or_gpu and extra_config
			"--gold_key", "alpha_type:Premul",
			"--gold_key", "arch:wasm",
			"--gold_key", "browser:Chrome",
			"--gold_key", "color_depth:8888",
			"--gold_key", "config:gles",
			"--gold_key", "configuration:Release",
			"--gold_key", "cpu_or_gpu_value:QuadroP400",
			"--gold_key", "model:Golo",
			"--gold_key", "os:Ubuntu18",
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
var shorthandToLabel = map[string]labelAndSavedOutputDir{
	"base":                           {"//src/base:base", ""},
	"example_hello_world_dawn":       {"//example:hello_world_dawn", ""},
	"example_hello_world_gl":         {"//example:hello_world_gl", ""},
	"example_hello_world_vulkan":     {"//example:hello_world_vulkan", ""},
	"modules_canvaskit":              {"//modules/canvaskit:canvaskit", ""},
	"modules_canvaskit_js_tests":     {"//modules/canvaskit:canvaskit_js_tests", ""},
	"skia_public":                    {"//:skia_public", ""},
	"skottie_tool_gpu":               {"//modules/skottie:skottie_tool_gpu", ""},
	"all_tests":                      {"//tests:linux_rbe_tests", ""},
	"experimental_bazel_test_client": {"//experimental/bazel_test/client:client_lib", ""},
	"cpu_gms":                        {"//gm:cpu_gm_tests", ""},
	"hello_bazel_world_test":         {"//gm:hello_bazel_world_test", ""},
	"cpu_8888_benchmark_test":        {"//bench:cpu_8888_test", ""},

	// Note: these paths are relative to the WORKSPACE in //example/external_client
	"path_combiner":     {"//:path_combiner", ""},
	"png_decoder":       {"//:png_decoder", ""},
	"shape_text":        {"//:shape_text", ""},
	"write_text_to_png": {"//:write_text_to_png", ""},

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
	shorthand, config, host := b.parts.bazelBuildParts()
	labelAndSavedOutputDir, ok := shorthandToLabel[shorthand]
	if !ok {
		panic("unsupported Bazel label shorthand " + shorthand)
	}

	b.addTask(b.Name, func(b *taskBuilder) {
		cmd := []string{
			"bazel_build_task_driver/bazel_build",
			"--project_id=skia-swarming-bots",
			"--task_id=" + specs.PLACEHOLDER_TASK_ID,
			"--task_name=" + b.Name,
			"--bazel_label=" + labelAndSavedOutputDir.label,
			"--bazel_config=" + config,
			"--bazel_cache_dir=" + bazelCacheDirOnGCELinux,
			"--workdir=.",
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
			// Use a built task_driver from CIPD instead of building it from scratch. The
			// task_driver should not need to change often, so using a CIPD version should reduce
			// build latency.
			// TODO(kjlubick) For now, this only has the linux version. We could build the task
			//   driver for all hosts that we support running Bazel from in this CIPD package
			//   if/when needed.
			// TODO(kjlubick,lovisolo) Could we get our task drivers built automatically
			// into CIPD instead of this being a manual process?
			b.cipd(b.MustGetCipdPackageFromAsset("bazel_build_task_driver"))

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
	taskdriverName, shorthand, buildConfig, host, testConfig := b.parts.bazelTestParts()
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

	b.addTask(b.Name, func(b *taskBuilder) {
		cmd := []string{"./" + taskdriverName,
			"--project_id=skia-swarming-bots",
			"--task_id=" + specs.PLACEHOLDER_TASK_ID,
			"--task_name=" + b.Name,
			"--workdir=.",
		}

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
			command = filepath.Join(OUTPUT_BAZEL, command)

			// The test's working directory will be its runfiles directory, which simulates the behavior of
			// the "bazel run" command.
			commandWorkDir := filepath.Join(command+".runfiles", "skia")

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
			cmd = append(cmd,
				"--bazel_label="+labelAndSavedOutputDir.label,
				"--path_in_skia=example/external_client",
				"--bazel_cache_dir="+bazelCacheDir)
			b.usesDocker()

		default:
			panic("Unsupported Bazel taskdriver " + taskdriverName)
		}

		if deviceSpecificBazelConfig != nil {
			cmd = append(cmd, "--device_specific_bazel_config="+deviceSpecificBazelConfig.Name)
		}

		if host == "linux_x64" {
			b.dep(b.buildTaskDrivers("linux", "amd64"))
			b.usesBazel("linux_x64")
		} else if host == "linux_arm64" || host == "on_rpi" {
			b.dep(b.buildTaskDrivers("linux", "arm64"))
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
