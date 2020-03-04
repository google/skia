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
	"path"
	"path/filepath"
	"regexp"
	"runtime"
	"sort"
	"strconv"
	"strings"
	"time"

	"github.com/golang/glog"
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
	DEFAULT_OS_WIN       = "Windows-Server-17763"

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
	CACHES_CCACHE = []*specs.Cache{
		&specs.Cache{
			Name: "ccache",
			Path: "cache/ccache",
		},
	}
	CACHES_DOCKER = []*specs.Cache{
		&specs.Cache{
			Name: "docker",
			Path: "cache/docker",
		},
	}

	// TODO(borenet): This hacky and bad.
	CIPD_PKGS_KITCHEN = append(specs.CIPD_PKGS_KITCHEN[:2], specs.CIPD_PKGS_PYTHON[1])
	CIPD_PKG_CPYTHON  = specs.CIPD_PKGS_PYTHON[0]

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
	BUILD_STATS_NO_UPLOAD = []string{
		"BuildStats-Debian9-Clang-x86_64-Release",
		"BuildStats-Debian9-Clang-x86_64-Release-Vulkan",
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

	// Swarming pool used for triggering tasks.
	Pool string `json:"pool"`

	// LUCI project associated with this repo.
	Project string `json:"project"`

	// Service accounts.
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
		jb := newJobBuilder(builder, name)
		jb.process()
		jb.finish()
	}
	builder.MustFinish()
}

// getThisDirName returns the infra/bots directory which is an ancestor of this
// file.
func getThisDirName() string {
	_, thisFileName, _, ok := runtime.Caller(0)
	if !ok {
		glog.Fatal("Unable to find path to current file.")
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
		glog.Fatal("Unable to find path to calling file.")
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

// marshalJson encodes the given data as JSON and fixes escaping of '<' which Go
// does by default.
func marshalJson(data interface{}) string {
	j, err := json.Marshal(data)
	if err != nil {
		glog.Fatal(err)
	}
	return strings.Replace(string(j), "\\u003c", "<", -1)
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
	return marshalJson(d)
}

// jobBuilder provides helpers for creating a job.
type jobBuilder struct {
	*builder
	parts
	Name string
	Spec *specs.JobSpec
}

// newJobBuilder returns a jobBuilder for the given job name.
func newJobBuilder(b *builder, name string) *jobBuilder {
	p, err := b.jobNameSchema.ParseJobName(name)
	if err != nil {
		glog.Fatal(err)
	}
	return &jobBuilder{
		builder: b,
		parts:   p,
		Name:    name,
		Spec:    &specs.JobSpec{},
	}
}

type parts map[string]string

// equal returns true if the given part of this job's name equals any of the
// given values. If no values are provided, equal returns true if the given
// part has a non-empty value.
func (p parts) equal(part string, eq ...string) bool {
	v := p[part]
	if len(eq) == 0 {
		return v != ""
	}
	for _, e := range eq {
		if v == e {
			return true
		}
	}
	return false
}

// role returns true if the role for this job equals any of the given
// regular expressions. If no regular expressions are provided, role returns
// true if this task has a non-empty role.
func (p parts) role(eq ...string) bool {
	return p.equal("role", eq...)
}

// os returns true if the OS for this job equals any of the given regular
// expressions. If no regular expressions are provided, os returns true if this
// task has a non-empty os.
func (p parts) os(eq ...string) bool {
	return p.equal("os", eq...)
}

// compiler returns true if the compiler for this job equals any of the
// given regular expressions. If no regular expressions are provided, compiler
// returns true if this task has a non-empty compiler.
func (p parts) compiler(eq ...string) bool {
	return p.equal("compiler", eq...)
}

// model returns true if the model for this job equals any of the given
// regular expressions. If no regular expressions are provided, model returns
// true if this task has a non-empty model.
func (p parts) model(eq ...string) bool {
	return p.equal("model", eq...)
}

// cpu returns true if the task's cpu_or_gpu is "CPU" and the CPU for this
// task equals any of the given regular expressions. If no regular expressions
// are provided, cpu returns true if this task runs on CPU.
func (p parts) cpu(eq ...string) bool {
	if p["cpu_or_gpu"] == "CPU" {
		if len(eq) == 0 {
			return true
		}
		return p.equal("cpu_or_gpu_value", eq...)
	}
	return false
}

// gpu returns true if the task's cpu_or_gpu is "GPU" and the GPU for this task
// equals any of the given regular expressions. If no regular expressions are
// provided, gpu returns true if this task runs on GPU.
func (p parts) gpu(eq ...string) bool {
	if p["cpu_or_gpu"] == "GPU" {
		if len(eq) == 0 {
			return true
		}
		return p.equal("cpu_or_gpu_value", eq...)
	}
	return false
}

// arch returns true if the architecture for this job equals any of the
// given regular expressions. If no regular expressions are provided, arch
// returns true if this task has a non-empty architecture.
func (p parts) arch(eq ...string) bool {
	return p.equal("arch", eq...) || p.equal("target_arch", eq...)
}

// extraConfig returns true if any of the extra_configs for this job equals
// any of the given regular expressions. If the extra_config starts with "SK_",
// it is considered to be a single config. If no regular expressions are
// provided, extraConfig returns true if this task has a non-empty extra_config.
func (p parts) extraConfig(eq ...string) bool {
	ec := p["extra_config"]
	var cfgs []string
	if strings.HasPrefix(ec, "SK_") {
		cfgs = []string{ec}
	} else {
		cfgs = strings.Split(ec, "_")
	}
	if len(eq) == 0 {
		return len(cfgs) > 0
	}
	for _, c := range cfgs {
		for _, e := range eq {
			if c == e {
				return true
			}
		}
	}
	return false
}

// matchPart returns true if the given part of this job's name matches any of
// the given regular expressions. If no regular expressions are provided,
// matchPart returns true if the given part has a non-empty value.
func (p parts) matchPart(part string, re ...string) bool {
	v := p[part]
	if len(re) == 0 {
		return v != ""
	}
	for _, r := range re {
		if regexp.MustCompile(r).MatchString(v) {
			return true
		}
	}
	return false
}

// matchRole returns true if the role for this job matches any of the given
// regular expressions. If no regular expressions are provided, role returns
// true if this task has a non-empty role.
func (p parts) matchRole(re ...string) bool {
	return p.matchPart("role", re...)
}

// matchOs returns true if the OS for this job matches any of the given regular
// expressions. If no regular expressions are provided, os returns true if this
// task has a non-empty os.
func (p parts) matchOs(re ...string) bool {
	return p.matchPart("os", re...)
}

// matchCompiler returns true if the compiler for this job matches any of the
// given regular expressions. If no regular expressions are provided, compiler
// returns true if this task has a non-empty compiler.
func (p parts) matchCompiler(re ...string) bool {
	return p.matchPart("compiler", re...)
}

// matchModel returns true if the model for this job matches any of the given
// regular expressions. If no regular expressions are provided, model returns
// true if this task has a non-empty model.
func (p parts) matchModel(re ...string) bool {
	return p.matchPart("model", re...)
}

// matchCpu returns true if the task's cpu_or_gpu is "CPU" and the CPU for this
// task matches any of the given regular expressions. If no regular expressions
// are provided, cpu returns true if this task runs on CPU.
func (p parts) matchCpu(re ...string) bool {
	if p["cpu_or_gpu"] == "CPU" {
		if len(re) == 0 {
			return true
		}
		return p.matchPart("cpu_or_gpu_value", re...)
	}
	return false
}

// matchGpu returns true if the task's cpu_or_gpu is "GPU" and the GPU for this task
// matches any of the given regular expressions. If no regular expressions are
// provided, gpu returns true if this task runs on GPU.
func (p parts) matchGpu(re ...string) bool {
	if p["cpu_or_gpu"] == "GPU" {
		if len(re) == 0 {
			return true
		}
		return p.matchPart("cpu_or_gpu_value", re...)
	}
	return false
}

// matchArch returns true if the architecture for this job matches any of the
// given regular expressions. If no regular expressions are provided, arch
// returns true if this task has a non-empty architecture.
func (p parts) matchArch(re ...string) bool {
	return p.matchPart("arch", re...) || p.matchPart("target_arch", re...)
}

// matchExtraConfig returns true if any of the extra_configs for this job matches
// any of the given regular expressions. If the extra_config starts with "SK_",
// it is considered to be a single config. If no regular expressions are
// provided, matchExtraConfig returns true if this task has a non-empty extra_config.
func (p parts) matchExtraConfig(re ...string) bool {
	ec := p["extra_config"]
	var cfgs []string
	if strings.HasPrefix(ec, "SK_") {
		cfgs = []string{ec}
	} else {
		cfgs = strings.Split(ec, "_")
	}
	if len(re) == 0 {
		return len(cfgs) > 0
	}
	compiled := make([]*regexp.Regexp, 0, len(re))
	for _, r := range re {
		compiled = append(compiled, regexp.MustCompile(r))
	}
	for _, c := range cfgs {
		for _, r := range compiled {
			if r.MatchString(c) {
				return true
			}
		}
	}
	return false
}

// debug returns true if this task runs in debug mode.
func (p parts) debug() bool {
	return p["configuration"] == "Debug"
}

// isLinux returns true if the task runs on Linux.
func (p parts) isLinux() bool {
	return p.matchOs("Debian", "Ubuntu")
}

// taskBuilder is a helper for creating a task.
type taskBuilder struct {
	*jobBuilder
	parts
	Name string
	Spec *specs.TaskSpec
}

// newTaskBuilder returns a taskBuilder instance.
func newTaskBuilder(b *jobBuilder, name string) *taskBuilder {
	parts, err := b.jobNameSchema.ParseJobName(name)
	if err != nil {
		glog.Fatal(err)
	}
	return &taskBuilder{
		jobBuilder: b,
		parts:      parts,
		Name:       name,
		Spec:       &specs.TaskSpec{},
	}
}

// Create a taskBuilder and run the given function for it.
func (b *jobBuilder) addTask(name string, fn func(*taskBuilder)) {
	tb := newTaskBuilder(b, name)
	fn(tb)
	b.MustAddTask(tb.Name, tb.Spec)
	// Add the task to the Job's dependency set, removing any which are
	// accounted for by the new task's dependencies.
	b.Spec.TaskSpecs = append(b.Spec.TaskSpecs, tb.Name)
	newSpecs := make([]string, 0, len(b.Spec.TaskSpecs))
	for _, t := range b.Spec.TaskSpecs {
		found := false
		for _, dep := range tb.Spec.Dependencies {
			if t == dep {
				found = true
				break
			}
		}
		if !found {
			newSpecs = append(newSpecs, t)
		}
	}
	b.Spec.TaskSpecs = newSpecs
}

// kitchenTask returns a specs.TaskSpec instance which uses Kitchen to run a
// recipe.
func (b *taskBuilder) kitchenTask(recipe, isolate, serviceAccount string, dimensions []string, extraProps map[string]string, outputDir string) {
	cipd := append([]*specs.CipdPackage{}, CIPD_PKGS_KITCHEN...)
	if b.matchOs("Win") && !b.model("LenovoYogaC630") {
		cipd = append(cipd, CIPD_PKG_CPYTHON)
	} else if b.os("Mac10.15") && b.model("VMware7.1") {
		cipd = append(cipd, CIPD_PKG_CPYTHON)
	}
	properties := map[string]string{
		"buildername":   b.Name,
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
	b.Spec = &specs.TaskSpec{
		Caches: []*specs.Cache{
			&specs.Cache{
				Name: "vpython",
				Path: "cache/vpython",
			},
		},
		CipdPackages: cipd,
		Command:      []string{python, "-u", "skia/infra/bots/run_recipe.py", "${ISOLATED_OUTDIR}", recipe, props(properties), b.cfg.Project},
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
		MaxAttempts:    b.attempts(),
		Outputs:        outputs,
		ServiceAccount: serviceAccount,
	}
	b.timeout(time.Hour)
}

// internalHardwareLabel returns the internal ID for the bot, if any.
func (b *taskBuilder) internalHardwareLabel() *int {
	if b.cfg.InternalHardwareLabel != nil {
		return b.cfg.InternalHardwareLabel(b.parts)
	}
	return nil
}

// linuxGceDimensions are the Swarming bot dimensions for Linux GCE instances.
func (b *taskBuilder) linuxGceDimensions(machineType string) []string {
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
func (b *taskBuilder) dockerGceDimensions() []string {
	// There's limited parallelism for WASM builds, so we can get away with the medium
	// instance instead of the beefy large instance.
	// Docker being installed is the most important part.
	return append(b.linuxGceDimensions(MACHINE_TYPE_MEDIUM), "docker_installed:true")
}

// deriveCompileTaskName returns the name of a compile task based on the given
// job name.
func (b *jobBuilder) deriveCompileTaskName() string {
	if b.parts["role"] == "Test" || b.parts["role"] == "Perf" {
		task_os := b.parts["os"]
		ec := []string{}
		if val := b.parts["extra_config"]; val != "" {
			ec = strings.Split(val, "_")
			ignore := []string{
				"Skpbench", "AbandonGpuContext", "PreAbandonGpuContext", "Valgrind",
				"ReleaseAndAbandonGpuContext", "CCPR", "FSAA", "FAAA", "FDAA", "NativeFonts", "GDI",
				"NoGPUThreads", "ProcDump", "DDL1", "DDL3", "T8888", "DDLTotal", "DDLRecord", "9x9",
				"BonusConfigs", "SkottieTracing", "SkottieWASM", "GpuTess", "NonNVPR", "Mskp",
				"Docker", "PDF"}
			keep := make([]string, 0, len(ec))
			for _, part := range ec {
				if !In(part, ignore) {
					keep = append(keep, part)
				}
			}
			ec = keep
		}
		if task_os == "Android" {
			if !In("Android", ec) {
				ec = append([]string{"Android"}, ec...)
			}
			task_os = "Debian9"
		} else if strings.Contains(task_os, "ChromeOS") {
			ec = append([]string{"Chromebook", "GLES"}, ec...)
			task_os = "Debian9"
		} else if task_os == "iOS" {
			ec = append([]string{task_os}, ec...)
			task_os = "Mac"
		} else if strings.Contains(task_os, "Win") {
			task_os = "Win"
		} else if b.parts["compiler"] == "GCC" {
			// GCC compiles are now on a Docker container. We use the same OS and
			// version to compile as to test.
			ec = append(ec, "Docker")
		} else if strings.Contains(task_os, "Ubuntu") || strings.Contains(task_os, "Debian") {
			task_os = "Debian9"
		} else if strings.Contains(task_os, "Mac") {
			task_os = "Mac"
		}
		jobNameMap := map[string]string{
			"role":          "Build",
			"os":            task_os,
			"compiler":      b.parts["compiler"],
			"target_arch":   b.parts["arch"],
			"configuration": b.parts["configuration"],
		}
		if strings.Contains(b.Name, "PathKit") {
			ec = []string{"PathKit"}
		}
		if strings.Contains(b.Name, "CanvasKit") || strings.Contains(b.Name, "SkottieWASM") {
			if b.parts["cpu_or_gpu"] == "CPU" {
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
	} else if b.parts["role"] == "BuildStats" {
		return strings.Replace(b.Name, "BuildStats", "Build", 1)
	} else {
		return b.Name
	}
}

// swarmDimensions generates swarming bot dimensions for the given task.
func (b *taskBuilder) swarmDimensions() []string {
	if b.cfg.SwarmDimensions != nil {
		dims := b.cfg.SwarmDimensions(b.parts)
		if dims != nil {
			return dims
		}
	}
	return b.defaultSwarmDimensions()
}

// defaultSwarmDimensions generates default swarming bot dimensions for the given task.
func (b *taskBuilder) defaultSwarmDimensions() []string {
	d := map[string]string{
		"pool": b.cfg.Pool,
	}
	if b.extraConfig("Docker") && (b.role("Build") || (b.cpu() && b.model("GCE"))) {
		return b.dockerGceDimensions()
	}
	if os, ok := b.parts["os"]; ok {
		d["os"], ok = map[string]string{
			"Android":  "Android",
			"ChromeOS": "ChromeOS",
			"Debian9":  DEFAULT_OS_DEBIAN,
			"Mac":      DEFAULT_OS_MAC,
			"Mac10.13": "Mac-10.13.6",
			"Mac10.14": "Mac-10.14.3",
			"Mac10.15": "Mac-10.15.1",
			"Ubuntu18": "Ubuntu-18.04",
			"Win":      DEFAULT_OS_WIN,
			"Win10":    "Windows-10-18363",
			"Win2019":  DEFAULT_OS_WIN,
			"Win7":     "Windows-7-SP1",
			"Win8":     "Windows-8.1-SP0",
			"iOS":      "iOS-13.3.1",
		}[os]
		if !ok {
			glog.Fatalf("Entry %q not found in OS mapping.", os)
		}
		if os == "Win10" && b.parts["model"] == "Golo" {
			// ChOps-owned machines have Windows 10 v1709.
			d["os"] = "Windows-10-16299"
		}
		if os == "Mac10.14" && b.parts["model"] == "VMware7.1" {
			// ChOps VMs are at a newer version of MacOS.
			d["os"] = "Mac-10.14.6"
		}
		if b.parts["model"] == "LenovoYogaC630" {
			// This is currently a unique snowflake.
			d["os"] = "Windows-10"
		}
		if b.parts["model"] == "iPhone6" {
			// This is the latest iOS that supports iPhone6.
			d["os"] = "iOS-12.4.5"
		}
	} else {
		d["os"] = DEFAULT_OS_DEBIAN
	}
	if b.role("Test", "Perf") {
		if b.os("Android") {
			// For Android, the device type is a better dimension
			// than CPU or GPU.
			deviceInfo, ok := map[string][]string{
				"AndroidOne":      {"sprout", "MOB30Q"},
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
				"Pixel4":          {"flame", "QD1A.190821.011.C4"},
				"TecnoSpark3Pro":  {"TECNO-KB8", "PPR1.180610.011"},
			}[b.parts["model"]]
			if !ok {
				glog.Fatalf("Entry %q not found in Android mapping.", b.parts["model"])
			}
			d["device_type"] = deviceInfo[0]
			d["device_os"] = deviceInfo[1]
		} else if b.os("iOS") {
			device, ok := map[string]string{
				"iPadMini4": "iPad5,1",
				"iPhone6":   "iPhone7,2",
				"iPhone7":   "iPhone9,1",
				"iPhone8":   "iPhone10,1",
				"iPadPro":   "iPad6,3",
			}[b.parts["model"]]
			if !ok {
				glog.Fatalf("Entry %q not found in iOS mapping.", b.parts["model"])
			}
			d["device_type"] = device
			// Temporarily use this dimension to ensure we only use the new libimobiledevice, since the
			// old version won't work with current recipes.
			d["libimobiledevice"] = "1582155448"
		} else if b.extraConfig("SKQP") && b.cpu("Emulator") {
			if !b.model("NUC7i5BNK") || d["os"] != DEFAULT_OS_DEBIAN {
				glog.Fatalf("Please update defaultSwarmDimensions for SKQP::Emulator %s %s.", b.parts["os"], b.parts["model"])
			}
			d["cpu"] = "x86-64-i5-7260U"
			d["os"] = DEFAULT_OS_DEBIAN
			// KVM means Kernel-based Virtual Machine, that is, can this vm virtualize commands
			// For us, this means, can we run an x86 android emulator on it.
			// kjlubick tried running this on GCE, but it was a bit too slow on the large install.
			// So, we run on bare metal machines in the Skolo (that should also have KVM).
			d["kvm"] = "1"
			d["docker_installed"] = "true"
		} else if b.cpu() || b.extraConfig("SwiftShader") {
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
				"SwiftShader": {
					"GCE": "x86-64-Haswell_GCE",
				},
			}[b.parts["cpu_or_gpu_value"]]
			if !ok {
				glog.Fatalf("Entry %q not found in CPU mapping.", b.parts["cpu_or_gpu_value"])
			}
			cpu, ok := modelMapping[b.parts["model"]]
			if !ok {
				glog.Fatalf("Entry %q not found in %q model mapping.", b.parts["model"], b.parts["cpu_or_gpu_value"])
			}
			d["cpu"] = cpu
			if b.model("GCE") && b.os(DEFAULT_OS_DEBIAN) {
				d["os"] = DEFAULT_OS_LINUX_GCE
			}
			if b.model("GCE") && d["cpu"] == "x86-64-Haswell_GCE" {
				d["machine_type"] = MACHINE_TYPE_MEDIUM
			}
		} else {
			if b.extraConfig("CanvasKit") {
				// GPU is defined for the WebGL version of CanvasKit, but
				// it can still run on a GCE instance.
				return b.dockerGceDimensions()
			} else if b.matchOs("Win") {
				gpu, ok := map[string]string{
					// At some point this might use the device ID, but for now it's like Chromebooks.
					"Adreno630":     "Adreno630",
					"GT610":         "10de:104a-23.21.13.9101",
					"GTX660":        "10de:11c0-26.21.14.4120",
					"GTX960":        "10de:1401-26.21.14.4120",
					"IntelHD4400":   "8086:0a16-20.19.15.4963",
					"IntelIris540":  "8086:1926-26.20.100.7463",
					"IntelIris6100": "8086:162b-20.19.15.4963",
					"IntelIris655":  "8086:3ea5-26.20.100.7463",
					"RadeonHD7770":  "1002:683d-26.20.13031.18002",
					"RadeonR9M470X": "1002:6646-26.20.13031.18002",
					"QuadroP400":    "10de:1cb3-25.21.14.1678",
				}[b.parts["cpu_or_gpu_value"]]
				if !ok {
					glog.Fatalf("Entry %q not found in Win GPU mapping.", b.parts["cpu_or_gpu_value"])
				}
				d["gpu"] = gpu
			} else if b.isLinux() {
				gpu, ok := map[string]string{
					// Intel drivers come from CIPD, so no need to specify the version here.
					"IntelBayTrail": "8086:0f31",
					"IntelHD2000":   "8086:0102",
					"IntelHD405":    "8086:22b1",
					"IntelIris640":  "8086:5926",
					"QuadroP400":    "10de:1cb3-430.14",
				}[b.parts["cpu_or_gpu_value"]]
				if !ok {
					glog.Fatalf("Entry %q not found in Ubuntu GPU mapping.", b.parts["cpu_or_gpu_value"])
				}
				d["gpu"] = gpu
			} else if b.matchOs("Mac") {
				gpu, ok := map[string]string{
					"IntelHD6000":   "8086:1626",
					"IntelHD615":    "8086:591e",
					"IntelIris5100": "8086:0a2e",
					"RadeonHD8870M": "1002:6821-4.0.20-3.2.8",
				}[b.parts["cpu_or_gpu_value"]]
				if !ok {
					glog.Fatalf("Entry %q not found in Mac GPU mapping.", b.parts["cpu_or_gpu_value"])
				}
				d["gpu"] = gpu
				// Yuck. We have two different types of MacMini7,1 with the same GPU but different CPUs.
				if b.gpu("IntelIris5100") {
					// Run all tasks on Golo machines for now.
					d["cpu"] = "x86-64-i7-4578U"
				}
			} else if b.os("ChromeOS") {
				version, ok := map[string]string{
					"MaliT604":           "10575.22.0",
					"MaliT764":           "10575.22.0",
					"MaliT860":           "10575.22.0",
					"PowerVRGX6250":      "10575.22.0",
					"TegraK1":            "10575.22.0",
					"IntelHDGraphics615": "10575.22.0",
				}[b.parts["cpu_or_gpu_value"]]
				if !ok {
					glog.Fatalf("Entry %q not found in ChromeOS GPU mapping.", b.parts["cpu_or_gpu_value"])
				}
				d["gpu"] = b.parts["cpu_or_gpu_value"]
				d["release_version"] = version
			} else {
				glog.Fatalf("Unknown GPU mapping for OS %q.", b.parts["os"])
			}
		}
	} else {
		d["gpu"] = "none"
		if d["os"] == DEFAULT_OS_DEBIAN {
			if b.extraConfig("PathKit", "CanvasKit", "CMake") {
				return b.dockerGceDimensions()
			}
			if b.role("BuildStats") {
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
		glog.Fatal(err)
	}
	return rv
}

// bundleRecipes generates the task to bundle and isolate the recipes.
func (b *jobBuilder) bundleRecipes() {
	b.addTask(BUNDLE_RECIPES_NAME, func(b *taskBuilder) {
		pkgs := append([]*specs.CipdPackage{}, specs.CIPD_PKGS_GIT...)
		pkgs = append(pkgs, specs.CIPD_PKGS_PYTHON...)
		b.Spec = &specs.TaskSpec{
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
		}
	})
}

// buildTaskDrivers generates the task to compile the task driver code to run on
// all platforms.
func (b *jobBuilder) buildTaskDrivers() {
	b.addTask(BUILD_TASK_DRIVERS_NAME, func(b *taskBuilder) {
		b.Spec = &specs.TaskSpec{
			Caches:       CACHES_GO,
			CipdPackages: append(specs.CIPD_PKGS_GIT, b.MustGetCipdPackageFromAsset("go")),
			Command: []string{
				"/bin/bash", "skia/infra/bots/build_task_drivers.sh", specs.PLACEHOLDER_ISOLATED_OUTDIR,
			},
			Dimensions: b.linuxGceDimensions(MACHINE_TYPE_SMALL),
			EnvPrefixes: map[string][]string{
				"PATH": {"cipd_bin_packages", "cipd_bin_packages/bin", "go/go/bin"},
			},
			Idempotent: true,
			Isolate:    "task_drivers.isolate",
		}
	})
}

// updateGoDeps generates the task to update Go dependencies.
func (b *jobBuilder) updateGoDeps() {
	b.addTask(b.Name, func(b *taskBuilder) {
		cipd := append([]*specs.CipdPackage{}, specs.CIPD_PKGS_GIT...)
		cipd = append(cipd, b.MustGetCipdPackageFromAsset("go"))
		cipd = append(cipd, b.MustGetCipdPackageFromAsset("protoc"))

		machineType := MACHINE_TYPE_MEDIUM
		b.Spec = &specs.TaskSpec{
			Caches:       CACHES_GO,
			CipdPackages: cipd,
			Command: []string{
				"./update_go_deps",
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
	})
}

// createDockerImage creates the specified docker image.
func (b *jobBuilder) createDockerImage(imageName, imageDir string) {
	b.addTask(b.Name, func(b *taskBuilder) {
		cipd := append([]*specs.CipdPackage{}, specs.CIPD_PKGS_GIT...)
		cipd = append(cipd, b.MustGetCipdPackageFromAsset("go"))
		cipd = append(cipd, b.MustGetCipdPackageFromAsset("protoc"))

		b.Spec = &specs.TaskSpec{
			Caches:       append(CACHES_GO, CACHES_DOCKER...),
			CipdPackages: cipd,
			Command: []string{
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
				"--alsologtostderr",
			},
			Dependencies: []string{BUILD_TASK_DRIVERS_NAME},
			Dimensions:   b.dockerGceDimensions(),
			EnvPrefixes: map[string][]string{
				"PATH": {"cipd_bin_packages", "cipd_bin_packages/bin", "go/go/bin"},
			},
			Isolate:        "empty.isolate",
			ServiceAccount: b.cfg.ServiceAccountCompile,
		}
	})
}

// createPushAppsFromSkiaDockerImage creates and pushes docker images of some apps
// (eg: fiddler, debugger, api) using the skia-release docker image.
func (b *jobBuilder) createPushAppsFromSkiaDockerImage() {
	b.addTask(b.Name, func(b *taskBuilder) {
		cipd := append([]*specs.CipdPackage{}, specs.CIPD_PKGS_GIT...)
		cipd = append(cipd, b.MustGetCipdPackageFromAsset("go"))
		cipd = append(cipd, b.MustGetCipdPackageFromAsset("protoc"))

		b.Spec = &specs.TaskSpec{
			Caches:       append(CACHES_GO, CACHES_DOCKER...),
			CipdPackages: cipd,
			Command: []string{
				"./push_apps_from_skia_image",
				"--project_id", "skia-swarming-bots",
				"--task_id", specs.PLACEHOLDER_TASK_ID,
				"--task_name", b.Name,
				"--workdir", ".",
				"--gerrit_project", "buildbot",
				"--gerrit_url", "https://skia-review.googlesource.com",
				"--repo", specs.PLACEHOLDER_REPO,
				"--revision", specs.PLACEHOLDER_REVISION,
				"--patch_issue", specs.PLACEHOLDER_ISSUE,
				"--patch_set", specs.PLACEHOLDER_PATCHSET,
				"--patch_server", specs.PLACEHOLDER_CODEREVIEW_SERVER,
				"--alsologtostderr",
			},
			Dependencies: []string{
				BUILD_TASK_DRIVERS_NAME,
				"Housekeeper-PerCommit-CreateDockerImage_Skia_Release",
			},
			Dimensions: b.dockerGceDimensions(),
			EnvPrefixes: map[string][]string{
				"PATH": {"cipd_bin_packages", "cipd_bin_packages/bin", "go/go/bin"},
			},
			Isolate:        "empty.isolate",
			ServiceAccount: b.cfg.ServiceAccountCompile,
		}
	})
}

// createPushAppsFromWASMDockerImage creates and pushes docker images of some apps
// (eg: jsfiddle, skottie, particles) using the skia-wasm-release docker image.
func (b *jobBuilder) createPushAppsFromWASMDockerImage() {
	b.addTask(b.Name, func(b *taskBuilder) {
		cipd := append([]*specs.CipdPackage{}, specs.CIPD_PKGS_GIT...)
		cipd = append(cipd, b.MustGetCipdPackageFromAsset("go"))
		cipd = append(cipd, b.MustGetCipdPackageFromAsset("protoc"))

		b.Spec = &specs.TaskSpec{
			Caches:       append(CACHES_GO, CACHES_DOCKER...),
			CipdPackages: cipd,
			Command: []string{
				"./push_apps_from_wasm_image",
				"--project_id", "skia-swarming-bots",
				"--task_id", specs.PLACEHOLDER_TASK_ID,
				"--task_name", b.Name,
				"--workdir", ".",
				"--gerrit_project", "buildbot",
				"--gerrit_url", "https://skia-review.googlesource.com",
				"--repo", specs.PLACEHOLDER_REPO,
				"--revision", specs.PLACEHOLDER_REVISION,
				"--patch_issue", specs.PLACEHOLDER_ISSUE,
				"--patch_set", specs.PLACEHOLDER_PATCHSET,
				"--patch_server", specs.PLACEHOLDER_CODEREVIEW_SERVER,
				"--alsologtostderr",
			},
			Dependencies: []string{
				BUILD_TASK_DRIVERS_NAME,
				"Housekeeper-PerCommit-CreateDockerImage_Skia_WASM_Release",
			},
			Dimensions: b.dockerGceDimensions(),
			EnvPrefixes: map[string][]string{
				"PATH": {"cipd_bin_packages", "cipd_bin_packages/bin", "go/go/bin"},
			},
			Isolate:        "empty.isolate",
			ServiceAccount: b.cfg.ServiceAccountCompile,
		}
	})
}

// createPushAppsFromSkiaWASMDockerImages creates and pushes docker images of some apps
// (eg: debugger-assets) using the skia-release and skia-wasm-release
// docker images.
func (b *jobBuilder) createPushAppsFromSkiaWASMDockerImages() {
	b.addTask(b.Name, func(b *taskBuilder) {
		cipd := append([]*specs.CipdPackage{}, specs.CIPD_PKGS_GIT...)
		cipd = append(cipd, b.MustGetCipdPackageFromAsset("go"))
		cipd = append(cipd, b.MustGetCipdPackageFromAsset("protoc"))

		b.Spec = &specs.TaskSpec{
			Caches:       append(CACHES_GO, CACHES_DOCKER...),
			CipdPackages: cipd,
			Command: []string{
				"./push_apps_from_skia_wasm_images",
				"--project_id", "skia-swarming-bots",
				"--task_id", specs.PLACEHOLDER_TASK_ID,
				"--task_name", b.Name,
				"--workdir", ".",
				"--gerrit_project", "buildbot",
				"--gerrit_url", "https://skia-review.googlesource.com",
				"--repo", specs.PLACEHOLDER_REPO,
				"--revision", specs.PLACEHOLDER_REVISION,
				"--patch_issue", specs.PLACEHOLDER_ISSUE,
				"--patch_set", specs.PLACEHOLDER_PATCHSET,
				"--patch_server", specs.PLACEHOLDER_CODEREVIEW_SERVER,
				"--alsologtostderr",
			},
			Dependencies: []string{
				BUILD_TASK_DRIVERS_NAME,
				"Housekeeper-PerCommit-CreateDockerImage_Skia_Release",
				"Housekeeper-PerCommit-CreateDockerImage_Skia_WASM_Release",
			},
			Dimensions: b.dockerGceDimensions(),
			EnvPrefixes: map[string][]string{
				"PATH": {"cipd_bin_packages", "cipd_bin_packages/bin", "go/go/bin"},
			},
			Isolate:        "empty.isolate",
			ServiceAccount: b.cfg.ServiceAccountCompile,
		}
	})
}

// isolateAssetConfig represents a task which copies a CIPD package into
// isolate.
type isolateAssetCfg struct {
	cipdPkg string
	path    string
}

// isolateCIPDAsset generates a task to isolate the given CIPD asset.
func (b *jobBuilder) isolateCIPDAsset(name string) string {
	b.addTask(name, func(b *taskBuilder) {
		asset := ISOLATE_ASSET_MAPPING[name]
		b.Spec = &specs.TaskSpec{
			CipdPackages: []*specs.CipdPackage{
				b.MustGetCipdPackageFromAsset(asset.cipdPkg),
			},
			Command:    []string{"/bin/cp", "-rL", asset.path, "${ISOLATED_OUTDIR}"},
			Dimensions: b.linuxGceDimensions(MACHINE_TYPE_SMALL),
			Idempotent: true,
			Isolate:    b.relpath("empty.isolate"),
		}
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

	if e := parts["extra_config"]; strings.Contains(e, "Skpbench") {
		// Skpbench only needs skps
		deps = append(deps, ISOLATE_SKP_NAME)
		deps = append(deps, ISOLATE_MSKP_NAME)
	} else if o := parts["os"]; In(o, rpiOS) {
		deps = append(deps, ISOLATE_SKP_NAME)
		deps = append(deps, ISOLATE_SVG_NAME)
		deps = append(deps, ISOLATE_SKIMAGE_NAME)
	}

	return deps
}

// usesCCache adds attributes to tasks which use ccache.
func (b *taskBuilder) usesCCache() {
	b.Spec.Caches = append(b.Spec.Caches, CACHES_CCACHE...)
}

// usesGit adds attributes to tasks which use git.
func (b *taskBuilder) usesGit() {
	b.Spec.Caches = append(b.Spec.Caches, CACHES_GIT...)
	if !b.extraConfig("NoDEPS") {
		b.Spec.Caches = append(b.Spec.Caches, CACHES_WORKDIR...)
	}
	b.Spec.CipdPackages = append(b.Spec.CipdPackages, specs.CIPD_PKGS_GIT...)
}

// usesGo adds attributes to tasks which use go. Recipes should use
// "with api.context(env=api.infra.go_env)".
func (b *taskBuilder) usesGo() {
	b.Spec.Caches = append(b.Spec.Caches, CACHES_GO...)
	pkg := b.MustGetCipdPackageFromAsset("go")
	if b.matchOs("Win") {
		pkg = b.MustGetCipdPackageFromAsset("go_win")
		pkg.Path = "go"
	}
	b.Spec.CipdPackages = append(b.Spec.CipdPackages, pkg)
}

// usesDocker adds attributes to tasks which use docker.
func (b *taskBuilder) usesDocker() {
	if b.extraConfig("Docker", "LottieWeb", "SKQP") || b.compiler("EMCC", "CMake") {
		b.Spec.Caches = append(b.Spec.Caches, CACHES_DOCKER...)
	}
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
			case "12.4.5":
				asset = "ios-dev-image-12.4"
			case "13.3.1":
				asset = "ios-dev-image-13.3"
			default:
				glog.Fatalf("Unable to determine correct ios-dev-image asset for %s. If %s is a new iOS release, you must add a CIPD package containing the corresponding iOS dev image; see ios-dev-image-11.4 for an example.", b.Name, m[1])
			}
			b.Spec.CipdPackages = append(b.Spec.CipdPackages, b.MustGetCipdPackageFromAsset(asset))
			break
		} else if strings.Contains(dim, "iOS") {
			glog.Fatalf("Must specify iOS version for %s to obtain correct dev image; os dimension is missing version: %s", b.Name, dim)
		}
	}
}

// timeout sets the timeout(s) for this task.
func (b *taskBuilder) timeout(timeout time.Duration) {
	b.Spec.ExecutionTimeout = timeout
	b.Spec.IoTimeout = timeout // With kitchen, step logs don't count toward IoTimeout.
}

// attempts returns the desired MaxAttempts for this task.
func (b *taskBuilder) attempts() int {
	if b.extraConfig("Framework") && b.extraConfig("Android", "G3") {
		// Both bots can be long running. No need to retry them.
		return 1
	}
	if !b.role("Build", "Upload") {
		if b.extraConfig("ASAN", "MSAN", "TSAN", "Valgrind") {
			// Sanitizers often find non-deterministic issues that retries would hide.
			return 1
		}
	}
	// Retry by default to hide random bot/hardware failures.
	return 2
}

// compile generates a compile task. Returns the name of the last task in the
// generated chain of tasks, which the Job should add as a dependency.
func (b *jobBuilder) compile(name string) {
	b.addTask(name, func(b *taskBuilder) {
		recipe := "compile"
		isolate := "compile.isolate"
		var props map[string]string
		needSync := false
		if strings.Contains(name, "NoDEPS") ||
			strings.Contains(name, "CMake") ||
			strings.Contains(name, "CommandBuffer") ||
			strings.Contains(name, "Flutter") ||
			strings.Contains(name, "SKQP") {
			recipe = "sync_and_compile"
			isolate = "swarm_recipe.isolate"
			props = EXTRA_PROPS
			needSync = true
		}
		b.kitchenTask(recipe, isolate, b.cfg.ServiceAccountCompile, b.swarmDimensions(), props, OUTPUT_BUILD)
		if needSync {
			b.usesGit()
		} else {
			b.Spec.Idempotent = true
		}
		b.usesDocker()

		// Android bots require a toolchain.
		if strings.Contains(name, "Android") {
			if strings.Contains(name, "Mac") {
				b.Spec.CipdPackages = append(b.Spec.CipdPackages, b.MustGetCipdPackageFromAsset("android_ndk_darwin"))
			} else if strings.Contains(name, "Win") {
				pkg := b.MustGetCipdPackageFromAsset("android_ndk_windows")
				pkg.Path = "n"
				b.Spec.CipdPackages = append(b.Spec.CipdPackages, pkg)
			} else if !strings.Contains(name, "SKQP") {
				b.Spec.CipdPackages = append(b.Spec.CipdPackages, b.MustGetCipdPackageFromAsset("android_ndk_linux"))
			}
		} else if strings.Contains(name, "Chromebook") {
			b.Spec.CipdPackages = append(b.Spec.CipdPackages, b.MustGetCipdPackageFromAsset("clang_linux"))
			if b.parts["target_arch"] == "x86_64" {
				b.Spec.CipdPackages = append(b.Spec.CipdPackages, b.MustGetCipdPackageFromAsset("chromebook_x86_64_gles"))
			} else if b.parts["target_arch"] == "arm" {
				b.Spec.CipdPackages = append(b.Spec.CipdPackages, b.MustGetCipdPackageFromAsset("armhf_sysroot"))
				b.Spec.CipdPackages = append(b.Spec.CipdPackages, b.MustGetCipdPackageFromAsset("chromebook_arm_gles"))
			}
		} else if strings.Contains(name, "Debian") {
			if strings.Contains(name, "Clang") {
				b.Spec.CipdPackages = append(b.Spec.CipdPackages, b.MustGetCipdPackageFromAsset("clang_linux"))
			}
			if strings.Contains(name, "SwiftShader") {
				b.Spec.CipdPackages = append(b.Spec.CipdPackages, b.MustGetCipdPackageFromAsset("cmake_linux"))
			}
			if strings.Contains(name, "OpenCL") {
				b.Spec.CipdPackages = append(b.Spec.CipdPackages,
					b.MustGetCipdPackageFromAsset("opencl_headers"),
					b.MustGetCipdPackageFromAsset("opencl_ocl_icd_linux"),
				)
			}
			b.Spec.CipdPackages = append(b.Spec.CipdPackages, b.MustGetCipdPackageFromAsset("ccache_linux"))
			b.usesCCache()
		} else if strings.Contains(name, "Win") {
			b.Spec.Dependencies = append(b.Spec.Dependencies, b.isolateCIPDAsset(ISOLATE_WIN_TOOLCHAIN_NAME))
			if strings.Contains(name, "Clang") {
				b.Spec.CipdPackages = append(b.Spec.CipdPackages, b.MustGetCipdPackageFromAsset("clang_win"))
			}
			if strings.Contains(name, "OpenCL") {
				b.Spec.CipdPackages = append(b.Spec.CipdPackages,
					b.MustGetCipdPackageFromAsset("opencl_headers"),
				)
			}
		} else if strings.Contains(name, "Mac") {
			b.Spec.CipdPackages = append(b.Spec.CipdPackages, CIPD_PKGS_XCODE...)
			b.Spec.Caches = append(b.Spec.Caches, &specs.Cache{
				Name: "xcode",
				Path: "cache/Xcode.app",
			})
			b.Spec.CipdPackages = append(b.Spec.CipdPackages, b.MustGetCipdPackageFromAsset("ccache_mac"))
			b.usesCCache()
			if strings.Contains(name, "CommandBuffer") {
				b.timeout(2 * time.Hour)
			}
			if strings.Contains(name, "MoltenVK") {
				b.Spec.CipdPackages = append(b.Spec.CipdPackages, b.MustGetCipdPackageFromAsset("moltenvk"))
			}
			if strings.Contains(name, "iOS") {
				b.Spec.CipdPackages = append(b.Spec.CipdPackages, b.MustGetCipdPackageFromAsset("provisioning_profile_ios"))
			}
		}
	})

	// All compile tasks are runnable as their own Job. Assert that the Job
	// is listed in jobs.
	if !In(name, b.jobs) {
		glog.Fatalf("Job %q is missing from the jobs list!", name)
	}
}

// recreateSKPs generates a RecreateSKPs task. Returns the name of the last
// task in the generated chain of tasks, which the Job should add as a
// dependency.
func (b *jobBuilder) recreateSKPs() {
	b.addTask(b.Name, func(b *taskBuilder) {
		dims := []string{
			"pool:SkiaCT",
			fmt.Sprintf("os:%s", DEFAULT_OS_LINUX_GCE),
		}
		b.kitchenTask("recreate_skps", "swarm_recipe.isolate", b.cfg.ServiceAccountRecreateSKPs, dims, EXTRA_PROPS, OUTPUT_NONE)
		b.usesGit()
		b.usesGo()
		b.timeout(4 * time.Hour)
	})
}

// checkGeneratedFiles verifies that no generated SKSL files have been edited
// by hand.
func (b *jobBuilder) checkGeneratedFiles() {
	b.addTask(b.Name, func(b *taskBuilder) {
		b.kitchenTask("check_generated_files", "swarm_recipe.isolate", b.cfg.ServiceAccountCompile, b.linuxGceDimensions(MACHINE_TYPE_LARGE), EXTRA_PROPS, OUTPUT_NONE)
		b.usesGit()
		b.usesGo()
		b.Spec.CipdPackages = append(b.Spec.CipdPackages, b.MustGetCipdPackageFromAsset("clang_linux"))
		b.Spec.CipdPackages = append(b.Spec.CipdPackages, b.MustGetCipdPackageFromAsset("ccache_linux"))
		b.usesCCache()
	})
}

// housekeeper generates a Housekeeper task. Returns the name of the last task
// in the generated chain of tasks, which the Job should add as a dependency.
func (b *jobBuilder) housekeeper() {
	b.addTask(b.Name, func(b *taskBuilder) {
		b.kitchenTask("housekeeper", "swarm_recipe.isolate", b.cfg.ServiceAccountHousekeeper, b.linuxGceDimensions(MACHINE_TYPE_SMALL), EXTRA_PROPS, OUTPUT_NONE)
		b.usesGit()
	})
}

// androidFrameworkCompile generates an Android Framework Compile task. Returns
// the name of the last task in the generated chain of tasks, which the Job
// should add as a dependency.
func (b *jobBuilder) androidFrameworkCompile() {
	b.addTask(b.Name, func(b *taskBuilder) {
		b.kitchenTask("android_compile", "compile_android_framework.isolate", "skia-android-framework-compile@skia-swarming-bots.iam.gserviceaccount.com", b.linuxGceDimensions(MACHINE_TYPE_SMALL), EXTRA_PROPS, OUTPUT_NONE)
		b.timeout(2 * time.Hour)
		b.usesGit()
	})
}

// g3FrameworkCompile generates a G3 Framework Compile task. Returns
// the name of the last task in the generated chain of tasks, which the Job
// should add as a dependency.
func (b *jobBuilder) g3FrameworkCompile() {
	b.addTask(b.Name, func(b *taskBuilder) {
		b.kitchenTask("g3_compile", "compile_g3_framework.isolate", "skia-g3-framework-compile@skia-swarming-bots.iam.gserviceaccount.com", b.linuxGceDimensions(MACHINE_TYPE_SMALL), EXTRA_PROPS, OUTPUT_NONE)
		b.timeout(3 * time.Hour)
		b.usesGit()
	})
}

// infra generates an infra_tests task. Returns the name of the last task in the
// generated chain of tasks, which the Job should add as a dependency.
func (b *jobBuilder) infra() {
	b.addTask(b.Name, func(b *taskBuilder) {
		dims := b.linuxGceDimensions(MACHINE_TYPE_SMALL)
		if strings.Contains(b.Name, "Win") {
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
		b.kitchenTask("infra", "infra_tests.isolate", b.cfg.ServiceAccountCompile, dims, extraProps, OUTPUT_NONE)
		b.Spec.CipdPackages = append(b.Spec.CipdPackages, specs.CIPD_PKGS_GSUTIL...)
		b.Spec.Idempotent = true
		// Repos which call into Skia's gen_tasks.go should define their own
		// infra_tests.isolate and therefore should not use relpath().
		b.Spec.Isolate = "infra_tests.isolate"
		b.usesGit() // We don't run bot_update, but Go needs a git repo.
		b.usesGo()
	})
}

// buildstats generates a builtstats task, which compiles code and generates
// statistics about the build.
func (b *jobBuilder) buildstats(compileTaskName string) {
	b.addTask(b.Name, func(b *taskBuilder) {
		b.kitchenTask("compute_buildstats", "swarm_recipe.isolate", "", b.swarmDimensions(), EXTRA_PROPS, OUTPUT_PERF)
		b.Spec.Dependencies = append(b.Spec.Dependencies, compileTaskName)
		b.Spec.CipdPackages = append(b.Spec.CipdPackages, b.MustGetCipdPackageFromAsset("bloaty"))
		b.usesGit()

		// Upload release results (for tracking in perf)
		// We have some jobs that are FYI (e.g. Debug-CanvasKit, tree-map generator)
		if strings.Contains(b.Name, "Release") && !In(b.Name, BUILD_STATS_NO_UPLOAD) {
			uploadName := fmt.Sprintf("%s%s%s", PREFIX_UPLOAD, b.jobNameSchema.Sep, b.Name)
			depName := b.Name
			b.addTask(uploadName, func(b *taskBuilder) {
				extraProps := map[string]string{
					"gs_bucket": b.cfg.GsBucketNano,
				}
				for k, v := range EXTRA_PROPS {
					extraProps[k] = v
				}
				b.kitchenTask("upload_buildstats_results", "swarm_recipe.isolate", b.cfg.ServiceAccountUploadNano, b.linuxGceDimensions(MACHINE_TYPE_SMALL), extraProps, OUTPUT_NONE)
				b.Spec.CipdPackages = append(b.Spec.CipdPackages, specs.CIPD_PKGS_GSUTIL...)
				b.Spec.Dependencies = append(b.Spec.Dependencies, depName)
			})
		}
	})
}

// doUpload indicates whether the given Job should upload its results.
func (b *jobBuilder) doUpload() bool {
	for _, s := range b.cfg.NoUpload {
		m, err := regexp.MatchString(s, b.Name)
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
func (b *jobBuilder) test(compileTaskName string, pkgs []*specs.CipdPackage) {
	b.addTask(b.Name, func(b *taskBuilder) {
		isolate := "test_skia_bundled.isolate"
		recipe := "test"
		if b.extraConfig("SKQP") {
			isolate = "skqp.isolate"
			recipe = "skqp_test"
			if b.cpu("Emulator") {
				recipe = "test_skqp_emulator"
			}
		} else if b.extraConfig("OpenCL") {
			// TODO(dogben): Longer term we may not want this to be called a "Test" task, but until we start
			// running hs_bench or kx, it will be easier to fit into the current job name schema.
			recipe = "compute_test"
		} else if b.extraConfig("PathKit") {
			isolate = "pathkit.isolate"
			recipe = "test_pathkit"
		} else if b.extraConfig("CanvasKit") {
			isolate = "canvaskit.isolate"
			recipe = "test_canvaskit"
		} else if b.extraConfig("LottieWeb") {
			isolate = "lottie_web.isolate"
			recipe = "test_lottie_web"
		}
		extraProps := map[string]string{
			"gold_hashes_url": b.cfg.GoldHashesURL,
		}
		for k, v := range EXTRA_PROPS {
			extraProps[k] = v
		}
		iid := b.internalHardwareLabel()
		iidStr := ""
		if iid != nil {
			iidStr = strconv.Itoa(*iid)
		}
		if recipe == "test" {
			flags, props := b.dmFlags(iidStr)
			extraProps["dm_flags"] = marshalJson(flags)
			extraProps["dm_properties"] = marshalJson(props)
		}
		b.kitchenTask(recipe, isolate, "", b.swarmDimensions(), extraProps, OUTPUT_TEST)
		b.Spec.CipdPackages = append(b.Spec.CipdPackages, pkgs...)
		if b.matchExtraConfig("Lottie") {
			b.Spec.CipdPackages = append(b.Spec.CipdPackages, b.MustGetCipdPackageFromAsset("lottie-samples"))
		}
		if !b.extraConfig("LottieWeb") {
			// Test.+LottieWeb doesn't require anything in Skia to be compiled.
			b.Spec.Dependencies = append(b.Spec.Dependencies, compileTaskName)
		}

		if b.os("Android") && b.extraConfig("ASAN") {
			b.Spec.Dependencies = append(b.Spec.Dependencies, b.isolateCIPDAsset(ISOLATE_NDK_LINUX_NAME))
		}
		if b.extraConfig("SKQP") {
			if !b.cpu("Emulator") {
				b.Spec.Dependencies = append(b.Spec.Dependencies, b.isolateCIPDAsset(ISOLATE_GCLOUD_LINUX_NAME))
			}
		}
		if deps := getIsolatedCIPDDeps(b.parts); len(deps) > 0 {
			b.Spec.Dependencies = append(b.Spec.Dependencies, deps...)
		}
		b.Spec.Expiration = 20 * time.Hour

		b.timeout(4 * time.Hour)
		if b.extraConfig("Valgrind") {
			b.timeout(9 * time.Hour)
			b.Spec.Expiration = 48 * time.Hour
			b.Spec.CipdPackages = append(b.Spec.CipdPackages, b.MustGetCipdPackageFromAsset("valgrind"))
			// Since Valgrind runs on the same bots as the CQ, we restrict Valgrind to a subset of the bots
			// to ensure there are always bots free for CQ tasks.
			b.Spec.Dimensions = append(b.Spec.Dimensions, "valgrind:1")
		} else if b.extraConfig("MSAN") {
			b.timeout(9 * time.Hour)
		} else if b.arch("x86") && b.debug() {
			// skia:6737
			b.timeout(6 * time.Hour)
		}
		b.maybeAddIosDevImage()

		// Upload results if necessary. TODO(kjlubick): If we do coverage analysis at the same
		// time as normal tests (which would be nice), cfg.json needs to have Coverage removed.
		if b.doUpload() {
			uploadName := fmt.Sprintf("%s%s%s", PREFIX_UPLOAD, b.jobNameSchema.Sep, b.Name)
			depName := b.Name
			b.addTask(uploadName, func(b *taskBuilder) {
				extraProps := map[string]string{
					"gs_bucket": b.cfg.GsBucketGm,
				}
				for k, v := range EXTRA_PROPS {
					extraProps[k] = v
				}
				b.kitchenTask("upload_dm_results", "swarm_recipe.isolate", b.cfg.ServiceAccountUploadGM, b.linuxGceDimensions(MACHINE_TYPE_SMALL), extraProps, OUTPUT_NONE)
				b.Spec.CipdPackages = append(b.Spec.CipdPackages, specs.CIPD_PKGS_GSUTIL...)
				b.Spec.Dependencies = append(b.Spec.Dependencies, depName)
			})
		}
	})
}

// perf generates a Perf task. Returns the name of the last task in the
// generated chain of tasks, which the Job should add as a dependency.
func (b *jobBuilder) perf(compileTaskName string, pkgs []*specs.CipdPackage) {
	b.addTask(b.Name, func(b *taskBuilder) {
		recipe := "perf"
		isolate := b.relpath("perf_skia_bundled.isolate")
		if strings.Contains(b.parts["extra_config"], "Skpbench") {
			recipe = "skpbench"
			isolate = b.relpath("skpbench_skia_bundled.isolate")
		} else if strings.Contains(b.Name, "PathKit") {
			isolate = "pathkit.isolate"
			recipe = "perf_pathkit"
		} else if strings.Contains(b.Name, "CanvasKit") {
			isolate = "canvaskit.isolate"
			recipe = "perf_canvaskit"
		} else if strings.Contains(b.Name, "SkottieTracing") {
			recipe = "perf_skottietrace"
		} else if strings.Contains(b.Name, "SkottieWASM") {
			recipe = "perf_skottiewasm_lottieweb"
			isolate = "skottie_wasm.isolate"
		} else if strings.Contains(b.Name, "LottieWeb") {
			recipe = "perf_skottiewasm_lottieweb"
			isolate = "lottie_web.isolate"
		}
		doUpload := strings.Contains(b.Name, "Release") && b.doUpload()
		extraProps := map[string]string{}
		for k, v := range EXTRA_PROPS {
			extraProps[k] = v
		}
		if recipe == "perf" {
			flags, props := nanobenchFlags(b.Name, b.parts, doUpload)
			extraProps["nanobench_flags"] = marshalJson(flags)
			extraProps["nanobench_properties"] = marshalJson(props)
		}
		b.kitchenTask(recipe, isolate, "", b.swarmDimensions(), extraProps, OUTPUT_PERF)
		b.Spec.CipdPackages = append(b.Spec.CipdPackages, pkgs...)
		if !strings.Contains(b.Name, "LottieWeb") {
			// Perf.+LottieWeb doesn't require anything in Skia to be compiled.
			b.Spec.Dependencies = append(b.Spec.Dependencies, compileTaskName)
		}
		b.Spec.Expiration = 20 * time.Hour
		b.timeout(4 * time.Hour)
		if deps := getIsolatedCIPDDeps(b.parts); len(deps) > 0 {
			b.Spec.Dependencies = append(b.Spec.Dependencies, deps...)
		}

		if strings.Contains(b.parts["extra_config"], "Valgrind") {
			b.timeout(9 * time.Hour)
			b.Spec.Expiration = 48 * time.Hour
			b.Spec.CipdPackages = append(b.Spec.CipdPackages, b.MustGetCipdPackageFromAsset("valgrind"))
			// Since Valgrind runs on the same bots as the CQ, we restrict Valgrind to a subset of the bots
			// to ensure there are always bots free for CQ tasks.
			b.Spec.Dimensions = append(b.Spec.Dimensions, "valgrind:1")
		} else if strings.Contains(b.parts["extra_config"], "MSAN") {
			b.timeout(9 * time.Hour)
		} else if b.parts["arch"] == "x86" && b.parts["configuration"] == "Debug" {
			// skia:6737
			b.timeout(6 * time.Hour)
		} else if strings.Contains(b.parts["extra_config"], "SkottieWASM") || strings.Contains(b.parts["extra_config"], "LottieWeb") {
			b.Spec.CipdPackages = append(b.Spec.CipdPackages, b.MustGetCipdPackageFromAsset("node"))
			b.Spec.CipdPackages = append(b.Spec.CipdPackages, b.MustGetCipdPackageFromAsset("lottie-samples"))
		} else if strings.Contains(b.parts["extra_config"], "Skottie") {
			b.Spec.CipdPackages = append(b.Spec.CipdPackages, b.MustGetCipdPackageFromAsset("lottie-samples"))
		}

		if strings.Contains(b.Name, "Android") && strings.Contains(b.Name, "CPU") {
			b.Spec.CipdPackages = append(b.Spec.CipdPackages, b.MustGetCipdPackageFromAsset("text_blob_traces"))
		}
		b.maybeAddIosDevImage()

		iid := b.internalHardwareLabel()
		if iid != nil {
			b.Spec.Command = append(b.Spec.Command, fmt.Sprintf("internal_hardware_label=%d", *iid))
		}

		// Upload results if necessary.
		if doUpload {
			uploadName := fmt.Sprintf("%s%s%s", PREFIX_UPLOAD, b.jobNameSchema.Sep, b.Name)
			depName := b.Name
			b.addTask(uploadName, func(b *taskBuilder) {
				extraProps := map[string]string{
					"gs_bucket": b.cfg.GsBucketNano,
				}
				for k, v := range EXTRA_PROPS {
					extraProps[k] = v
				}
				b.kitchenTask("upload_nano_results", "swarm_recipe.isolate", b.cfg.ServiceAccountUploadNano, b.linuxGceDimensions(MACHINE_TYPE_SMALL), extraProps, OUTPUT_NONE)
				b.Spec.CipdPackages = append(b.Spec.CipdPackages, specs.CIPD_PKGS_GSUTIL...)
				b.Spec.Dependencies = append(b.Spec.Dependencies, depName)
			})
		}
	})
}

// presubmit generates a task which runs the presubmit for this repo.
func (b *jobBuilder) presubmit() {
	b.addTask(b.Name, func(b *taskBuilder) {
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
		b.kitchenTask("run_presubmit", "run_recipe.isolate", b.cfg.ServiceAccountCompile, b.linuxGceDimensions(MACHINE_TYPE_LARGE), extraProps, OUTPUT_NONE)
		b.usesGit()
		b.Spec.CipdPackages = append(b.Spec.CipdPackages, &specs.CipdPackage{
			Name:    "infra/recipe_bundles/chromium.googlesource.com/chromium/tools/build",
			Path:    "recipe_bundle",
			Version: "git_revision:a8bcedad6768e206c4d2bd1718caa849f29cd42d",
		})
		b.Spec.Dependencies = []string{} // No bundled recipes for this one.
	})
}

// process generates tasks and jobs for the given job name.
func (b *jobBuilder) process() {
	// Bundle Recipes.
	if b.Name == BUNDLE_RECIPES_NAME {
		b.bundleRecipes()
		return
	}
	if b.Name == BUILD_TASK_DRIVERS_NAME {
		b.buildTaskDrivers()
		return
	}

	// Isolate CIPD assets.
	if _, ok := ISOLATE_ASSET_MAPPING[b.Name]; ok {
		b.isolateCIPDAsset(b.Name)
		return
	}

	// RecreateSKPs.
	if strings.Contains(b.Name, "RecreateSKPs") {
		b.recreateSKPs()
		return
	}

	// Update Go Dependencies.
	if strings.Contains(b.Name, "UpdateGoDeps") {
		b.updateGoDeps()
		return
	}

	// Create docker image.
	if b.extraConfig("CreateDockerImage") {
		if b.extraConfig("WASM") {
			b.createDockerImage("skia-wasm-release", path.Join("docker", "skia-wasm-release"))
			return
		} else {
			b.createDockerImage("skia-release", path.Join("docker", "skia-release"))
			return
		}
	}

	// Push apps from docker image.
	if b.extraConfig("PushAppsFromSkiaDockerImage") {
		b.createPushAppsFromSkiaDockerImage()
		return
	} else if b.extraConfig("PushAppsFromWASMDockerImage") {
		b.createPushAppsFromWASMDockerImage()
		return
	} else if b.extraConfig("PushAppsFromSkiaWASMDockerImages") {
		b.createPushAppsFromSkiaWASMDockerImages()
		return
	}

	// Infra tests.
	if b.extraConfig("InfraTests") {
		b.infra()
		return
	}

	// Housekeepers.
	if b.Name == "Housekeeper-PerCommit" {
		b.housekeeper()
		return
	}
	if b.Name == "Housekeeper-PerCommit-CheckGeneratedFiles" {
		b.checkGeneratedFiles()
		return
	}
	if b.Name == "Housekeeper-OnDemand-Presubmit" {
		b.Spec.Priority = 1
		b.presubmit()
		return
	}

	// Compile bots.
	if b.role("Build") {
		if b.extraConfig("Android") && b.extraConfig("Framework") {
			// Android Framework compile tasks use a different recipe.
			b.androidFrameworkCompile()
			return
		} else if b.extraConfig("G3") && b.extraConfig("Framework") {
			// G3 compile tasks use a different recipe.
			b.g3FrameworkCompile()
			return
		} else {
			b.compile(b.Name)
			return
		}
	}

	// Most remaining tasks need a compile task.
	compileTaskName := b.deriveCompileTaskName()

	// These bots do not need a compile task.
	if !b.role("Build") &&
		b.Name != "Housekeeper-PerCommit-BundleRecipes" &&
		!strings.Contains(b.Name, "Housekeeper-PerCommit-InfraTests") &&
		b.Name != "Housekeeper-PerCommit-CheckGeneratedFiles" &&
		b.Name != "Housekeeper-Nightly-UpdateGoDeps" &&
		b.Name != "Housekeeper-OnDemand-Presubmit" &&
		b.Name != "Housekeeper-PerCommit" &&
		b.Name != BUILD_TASK_DRIVERS_NAME &&
		!strings.Contains(b.Name, "CreateDockerImage") &&
		!strings.Contains(b.Name, "PushAppsFrom") &&
		!strings.Contains(b.Name, "Android_Framework") &&
		!strings.Contains(b.Name, "G3_Framework") &&
		!strings.Contains(b.Name, "RecreateSKPs") &&
		!strings.Contains(b.Name, "Housekeeper-PerCommit-Isolate") &&
		!strings.Contains(b.Name, "SkottieWASM") &&
		!strings.Contains(b.Name, "LottieWeb") {
		b.compile(compileTaskName)
	}

	// BuildStats bots. This computes things like binary size.
	if b.role("BuildStats") {
		b.buildstats(compileTaskName)
		return
	}

	// Common assets needed the Test and Perf tasks.
	pkgs := []*specs.CipdPackage{}
	if cipd := getIsolatedCIPDDeps(b.parts); len(cipd) == 0 {
		// for desktop machines
		pkgs = []*specs.CipdPackage{
			b.MustGetCipdPackageFromAsset("skimage"),
			b.MustGetCipdPackageFromAsset("skp"),
			b.MustGetCipdPackageFromAsset("svg"),
		}
	}

	if b.isLinux() {
		if b.matchExtraConfig("SAN") {
			pkgs = append(pkgs, b.MustGetCipdPackageFromAsset("clang_linux"))
		}
		if b.extraConfig("Vulkan") {
			pkgs = append(pkgs, b.MustGetCipdPackageFromAsset("linux_vulkan_sdk"))
		}
		if b.matchGpu("Intel") {
			pkgs = append(pkgs, b.MustGetCipdPackageFromAsset("mesa_intel_driver_linux"))
		}
		if b.extraConfig("OpenCL") {
			pkgs = append(pkgs,
				b.MustGetCipdPackageFromAsset("opencl_ocl_icd_linux"),
				b.MustGetCipdPackageFromAsset("opencl_intel_neo_linux"),
			)
		}
	}
	if b.extraConfig("ProcDump") {
		pkgs = append(pkgs, b.MustGetCipdPackageFromAsset("procdump_win"))
	}
	if b.extraConfig("CanvasKit", "PathKit") || (b.role("Test") && b.extraConfig("LottieWeb")) {
		// Docker-based tests that don't need the standard CIPD assets
		pkgs = []*specs.CipdPackage{}
	}

	// Valgrind runs at a low priority so that it doesn't occupy all the bots.
	if b.extraConfig("Valgrind") {
		// Priority of 0.085 should result in Valgrind tasks with a blamelist of ~10 commits having the
		// same score as other tasks with a blamelist of 1 commit, when we have insufficient bot
		// capacity to run more frequently.
		b.Spec.Priority = 0.085
	}

	// Test bots.
	if b.role("Test") {
		b.test(compileTaskName, pkgs)
		return
	}

	// Perf bots.
	if b.role("Perf") {
		b.perf(compileTaskName, pkgs)
		return
	}
}

func (b *jobBuilder) finish() {
	// Add the Job spec.
	b.Spec.Trigger = specs.TRIGGER_ANY_BRANCH
	if strings.Contains(b.Name, "-Nightly-") {
		b.Spec.Trigger = specs.TRIGGER_NIGHTLY
	} else if strings.Contains(b.Name, "-Weekly-") {
		b.Spec.Trigger = specs.TRIGGER_WEEKLY
	} else if strings.Contains(b.Name, "Flutter") || strings.Contains(b.Name, "CommandBuffer") {
		b.Spec.Trigger = specs.TRIGGER_MASTER_ONLY
	} else if strings.Contains(b.Name, "-OnDemand-") || strings.Contains(b.Name, "Android_Framework") || strings.Contains(b.Name, "G3_Framework") {
		b.Spec.Trigger = specs.TRIGGER_ON_DEMAND
	}
	b.MustAddJob(b.Name, b.Spec)
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
	defer func() {
		if err := f.Close(); err != nil {
			glog.Errorf("Failed to close %s: %s", jsonFile, err)
		}
	}()
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
