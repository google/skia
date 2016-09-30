// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package main

/*
	Generate the tasks.json file.
*/

import (
	"bytes"
	"encoding/json"
	"fmt"
	"io/ioutil"
	"os"
	"path"
	"path/filepath"
	"sort"
	"strings"

	"github.com/skia-dev/glog"
	"go.skia.org/infra/go/common"
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
		"Build-Ubuntu-GCC-x86_64-Release-GN",
		"Perf-Ubuntu-GCC-GCE-CPU-AVX2-x86_64-Release-GN",
		"Test-Ubuntu-GCC-GCE-CPU-AVX2-x86_64-Release-GN",
	}

	// UPLOAD_DIMENSIONS are the Swarming dimensions for upload tasks.
	UPLOAD_DIMENSIONS = []string{
		"cpu:x86-64-avx2",
		"gpu:none",
		"os:Ubuntu",
		fmt.Sprintf("pool:%s", POOL_SKIA),
	}

	// Defines the structure of job names.
	jobNameSchema *JobNameSchema

	// Caches CIPD package info so that we don't have to re-read VERSION
	// files.
	cipdPackages = map[string]*specs.CipdPackage{}

	// Path to the infra/bots directory.
	infrabotsDir = ""
)

// deriveCompileTaskName returns the name of a compile task based on the given
// job name.
func deriveCompileTaskName(jobName string, parts map[string]string) string {
	if parts["role"] == "Housekeeper" {
		return "Build-Ubuntu-GCC-x86_64-Release-Shared"
	} else if parts["role"] == "Test" || parts["role"] == "Perf" {
		task_os := parts["os"]
		ec := parts["extra_config"]
		if task_os == "Android" {
			if ec == "Vulkan" {
				ec = "Android_Vulkan"
			} else if !strings.Contains(ec, "GN_Android") {
				ec = task_os
			}
			task_os = "Android"
		} else if task_os == "iOS" {
			ec = task_os
			task_os = "Mac"
		} else if strings.Contains(task_os, "Win") {
			task_os = "Win"
		}
		name, err := jobNameSchema.MakeJobName(map[string]string{
			"role":          "Build",
			"os":            task_os,
			"compiler":      parts["compiler"],
			"target_arch":   parts["arch"],
			"configuration": parts["configuration"],
			"extra_config":  ec,
		})
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
	if parts["extra_config"] == "SkiaCT" {
		return []string{
			"pool:SkiaCT",
		}
	}
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
			d["device_type"] = map[string]string{
				"AndroidOne":    "sprout",
				"GalaxyS3":      "m0", // "smdk4x12", Detected incorrectly by swarming?
				"GalaxyS4":      "",   // TODO(borenet,kjlubick)
				"GalaxyS7":      "heroqlteatt",
				"NVIDIA_Shield": "foster",
				"Nexus10":       "manta",
				"Nexus5":        "hammerhead",
				"Nexus6":        "shamu",
				"Nexus6p":       "angler",
				"Nexus7":        "grouper",
				"Nexus7v2":      "flo",
				"Nexus9":        "flounder",
				"NexusPlayer":   "fugu",
			}[parts["model"]]
		} else if strings.Contains(parts["os"], "iOS") {
			d["device"] = map[string]string{
				"iPad4": "iPad4,1",
			}[parts["model"]]
			// TODO(borenet): Replace this hack with something
			// better.
			d["os"] = "iOS-9.2"
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
				"GeForce320M": "10de:08a4",
				"GT610":       "10de:104a",
				"GTX550Ti":    "10de:1244",
				"GTX660":      "10de:11c0",
				"GTX960":      "10de:1401",
				"HD4000":      "8086:0a2e",
				"HD4600":      "8086:0412",
				"HD7770":      "1002:683d",
				"iHD530":      "8086:1912",
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

// getCipdPackage finds and returns the given CIPD package and version.
func getCipdPackage(assetName string) *specs.CipdPackage {
	if pkg, ok := cipdPackages[assetName]; ok {
		return pkg
	}
	versionFile := path.Join(infrabotsDir, "assets", assetName, "VERSION")
	contents, err := ioutil.ReadFile(versionFile)
	if err != nil {
		glog.Fatal(err)
	}
	version := strings.TrimSpace(string(contents))
	pkg := &specs.CipdPackage{
		Name:    fmt.Sprintf("skia/bots/%s", assetName),
		Path:    assetName,
		Version: fmt.Sprintf("version:%s", version),
	}
	if assetName == "win_toolchain" {
		pkg.Path = "t" // Workaround for path length limit on Windows.
	}
	cipdPackages[assetName] = pkg
	return pkg
}

// compile generates a compile task. Returns the name of the last task in the
// generated chain of tasks, which the Job should add as a dependency.
func compile(cfg *specs.TasksCfg, name string, parts map[string]string) string {
	// Collect the necessary CIPD packages.
	pkgs := []*specs.CipdPackage{}

	// Android bots require a toolchain.
	if strings.Contains(name, "Android") {
		pkgs = append(pkgs, getCipdPackage("android_sdk"))
		if strings.Contains(name, "Mac") {
			pkgs = append(pkgs, getCipdPackage("android_ndk_darwin"))
		} else {
			pkgs = append(pkgs, getCipdPackage("android_ndk_linux"))
		}
	}

	// Clang on Linux.
	if strings.Contains(name, "Ubuntu") && strings.Contains(name, "Clang") {
		pkgs = append(pkgs, getCipdPackage("clang_linux"))
	}

	// Windows toolchain.
	if strings.Contains(name, "Win") {
		pkgs = append(pkgs, getCipdPackage("win_toolchain"))
		if strings.Contains(name, "Vulkan") {
			pkgs = append(pkgs, getCipdPackage("win_vulkan_sdk"))
		}
	}

	// Add the task.
	cfg.Tasks[name] = &specs.TaskSpec{
		CipdPackages: pkgs,
		Dimensions:   swarmDimensions(parts),
		ExtraArgs: []string{
			"--workdir", "../../..", "swarm_compile",
			fmt.Sprintf("buildername=%s", name),
			"mastername=fake-master",
			"buildnumber=2",
			"slavename=fake-buildslave",
			fmt.Sprintf("swarm_out_dir=%s", specs.PLACEHOLDER_ISOLATED_OUTDIR),
			fmt.Sprintf("revision=%s", specs.PLACEHOLDER_REVISION),
		},
		Isolate:  "compile_skia.isolate",
		Priority: 0.8,
	}
	return name
}

// recreateSKPs generates a RecreateSKPs task. Returns the name of the last
// task in the generated chain of tasks, which the Job should add as a
// dependency.
func recreateSKPs(cfg *specs.TasksCfg, name string) string {
	// TODO
	return name
}

// ctSKPs generates a CT SKPs task. Returns the name of the last task in the
// generated chain of tasks, which the Job should add as a dependency.
func ctSKPs(cfg *specs.TasksCfg, name string) string {
	// TODO
	return name
}

// housekeeper generates a Housekeeper task. Returns the name of the last task
// in the generated chain of tasks, which the Job should add as a dependency.
func housekeeper(cfg *specs.TasksCfg, name, compileTaskName string) string {
	// TODO
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
func test(cfg *specs.TasksCfg, name string, parts map[string]string, compileTaskName string, pkgs []*specs.CipdPackage) string {
	cfg.Tasks[name] = &specs.TaskSpec{
		CipdPackages: pkgs,
		Dependencies: []string{compileTaskName},
		Dimensions:   swarmDimensions(parts),
		ExtraArgs: []string{
			"--workdir", "../../..", "swarm_test",
			fmt.Sprintf("buildername=%s", name),
			"mastername=fake-master",
			"buildnumber=2",
			"slavename=fake-buildslave",
			fmt.Sprintf("swarm_out_dir=%s", specs.PLACEHOLDER_ISOLATED_OUTDIR),
			fmt.Sprintf("revision=%s", specs.PLACEHOLDER_REVISION),
		},
		Isolate:  "test_skia.isolate",
		Priority: 0.8,
	}
	// Upload results if necessary.
	if doUpload(name) {
		uploadName := fmt.Sprintf("%s%s%s", PREFIX_UPLOAD, jobNameSchema.Sep, name)
		cfg.Tasks[uploadName] = &specs.TaskSpec{
			Dependencies: []string{name},
			Dimensions:   UPLOAD_DIMENSIONS,
			ExtraArgs: []string{
				"--workdir", "../../..", "upload_dm_results",
				fmt.Sprintf("buildername=%s", name),
				"mastername=fake-master",
				"buildnumber=2",
				"slavename=fake-buildslave",
				fmt.Sprintf("swarm_out_dir=%s", specs.PLACEHOLDER_ISOLATED_OUTDIR),
				fmt.Sprintf("revision=%s", specs.PLACEHOLDER_REVISION),
			},
			Isolate:  "upload_dm_results.isolate",
			Priority: 0.8,
		}
		return uploadName
	}
	return name
}

// perf generates a Perf task. Returns the name of the last task in the
// generated chain of tasks, which the Job should add as a dependency.
func perf(cfg *specs.TasksCfg, name string, parts map[string]string, compileTaskName string, pkgs []*specs.CipdPackage) string {
	cfg.Tasks[name] = &specs.TaskSpec{
		CipdPackages: pkgs,
		Dependencies: []string{compileTaskName},
		Dimensions:   swarmDimensions(parts),
		ExtraArgs: []string{
			"--workdir", "../../..", "swarm_perf",
			fmt.Sprintf("buildername=%s", name),
			"mastername=fake-master",
			"buildnumber=2",
			"slavename=fake-buildslave",
			fmt.Sprintf("swarm_out_dir=%s", specs.PLACEHOLDER_ISOLATED_OUTDIR),
			fmt.Sprintf("revision=%s", specs.PLACEHOLDER_REVISION),
		},
		Isolate:  "perf_skia.isolate",
		Priority: 0.8,
	}
	// Upload results if necessary.
	if strings.Contains(name, "Release") && doUpload(name) {
		uploadName := fmt.Sprintf("%s%s%s", PREFIX_UPLOAD, jobNameSchema.Sep, name)
		cfg.Tasks[uploadName] = &specs.TaskSpec{
			Dependencies: []string{name},
			Dimensions:   UPLOAD_DIMENSIONS,
			ExtraArgs: []string{
				"--workdir", "../../..", "upload_nano_results",
				fmt.Sprintf("buildername=%s", name),
				"mastername=fake-master",
				"buildnumber=2",
				"slavename=fake-buildslave",
				fmt.Sprintf("swarm_out_dir=%s", specs.PLACEHOLDER_ISOLATED_OUTDIR),
				fmt.Sprintf("revision=%s", specs.PLACEHOLDER_REVISION),
			},
			Isolate:  "upload_nano_results.isolate",
			Priority: 0.8,
		}
		return uploadName
	}
	return name
}

// process generates tasks and jobs for the given job name.
func process(cfg *specs.TasksCfg, name string) {
	if _, ok := cfg.Jobs[name]; ok {
		glog.Fatalf("Duplicate job %q", name)
	}
	deps := []string{}

	parts, err := jobNameSchema.ParseJobName(name)
	if err != nil {
		glog.Fatal(err)
	}

	// RecreateSKPs.
	if strings.Contains(name, "RecreateSKPs") {
		deps = append(deps, recreateSKPs(cfg, name))
	}

	// CT bots.
	if strings.Contains(name, "-CT_") {
		deps = append(deps, ctSKPs(cfg, name))
	}

	// Compile bots.
	if parts["role"] == "Build" {
		deps = append(deps, compile(cfg, name, parts))
	}

	// Any remaining bots need a compile task.
	compileTaskName := deriveCompileTaskName(name, parts)

	// Housekeeper.
	if parts["role"] == "Housekeeper" {
		deps = append(deps, housekeeper(cfg, name, compileTaskName))
	}

	// Common assets needed by the remaining bots.
	pkgs := []*specs.CipdPackage{
		getCipdPackage("skimage"),
		getCipdPackage("skp"),
		getCipdPackage("svg"),
	}

	// Test bots.
	if parts["role"] == "Test" {
		deps = append(deps, test(cfg, name, parts, compileTaskName, pkgs))
	}

	// Perf bots.
	if parts["role"] == "Perf" {
		deps = append(deps, perf(cfg, name, parts, compileTaskName, pkgs))
	}

	// Add the Job spec.
	cfg.Jobs[name] = &specs.JobSpec{
		Priority:  0.8,
		TaskSpecs: deps,
	}
}

// getCheckoutRoot returns the path of the root of the Skia checkout, or an
// error if it cannot be found.
func getCheckoutRoot() string {
	cwd, err := os.Getwd()
	if err != nil {
		glog.Fatal(err)
	}
	for {
		if _, err := os.Stat(cwd); err != nil {
			glog.Fatal(err)
		}
		s, err := os.Stat(path.Join(cwd, ".git"))
		if err == nil && s.IsDir() {
			// TODO(borenet): Should we verify that this is a Skia
			// checkout and not something else?
			return cwd
		}
		cwd = filepath.Clean(path.Join(cwd, ".."))
	}
}

// Regenerate the tasks.json file.
func main() {
	common.Init()
	defer common.LogPanic()

	// Where are we?
	root := getCheckoutRoot()
	infrabotsDir = path.Join(root, "infra", "bots")

	// Create the JobNameSchema.
	schema, err := NewJobNameSchema(path.Join(infrabotsDir, "recipe_modules", "builder_name_schema", "builder_name_schema.json"))
	if err != nil {
		glog.Fatal(err)
	}
	jobNameSchema = schema

	// Create the config.
	cfg := &specs.TasksCfg{
		Jobs:  map[string]*specs.JobSpec{},
		Tasks: map[string]*specs.TaskSpec{},
	}

	// Create Tasks and Jobs.
	for _, j := range JOBS {
		process(cfg, j)
	}

	// Validate the config.
	if err := cfg.Validate(); err != nil {
		glog.Fatal(err)
	}

	// Write the tasks.json file.
	outFile := path.Join(root, specs.TASKS_CFG_FILE)
	b, err := json.MarshalIndent(cfg, "", "  ")
	if err != nil {
		glog.Fatal(err)
	}
	// The json package escapes HTML characters, which makes our output
	// much less readable. Replace the escape characters with the real
	// character.
	b = bytes.Replace(b, []byte("\\u003c"), []byte("<"), -1)
	if err := ioutil.WriteFile(outFile, b, os.ModePerm); err != nil {
		glog.Fatal(err)
	}
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
