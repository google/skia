// Copyright 2022 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package main

import (
	"encoding/json"
	"flag"
	"fmt"
	"os"
	"regexp"
	"strings"
)

type depConfig struct {
	bazelNameOverride string // Bazel style uses underscores not dashes, so we fix those if needed.
	needsBazelFile    bool
	patches           []string
	patchCmds         []string
	patchCmdsWin      []string
	isIndirect        bool // if True, it's used by another dependency, not by Skia directly
}

// These are all C++ deps or Rust deps (with a compatible C++ FFI) used by the Bazel build.
// They are a subset of those listed in DEPS.
// The key is the name of the repo as specified in DEPS.
var depsOverrides = map[string]depConfig{
	"abseil-cpp":  {bazelNameOverride: "abseil_cpp", isIndirect: true},
	"brotli":      {isIndirect: true},
	"highway":     {isIndirect: true},
	"spirv-tools": {bazelNameOverride: "spirv_tools"},
	// This name is important because spirv_tools expects @spirv_headers to exist by that name.
	"spirv-headers": {bazelNameOverride: "spirv_headers"},

	"dawn":           {needsBazelFile: true},
	"delaunator-cpp": {bazelNameOverride: "delaunator", needsBazelFile: true},
	"dng_sdk":        {needsBazelFile: true},
	"expat": {
		needsBazelFile: true,
		patches:        []string{"//bazel/external/expat:config_files.patch"},
	},
	"freetype": {
		needsBazelFile: true,
		patches:        []string{"//bazel/external/freetype:config_files.patch"},
	},
	"harfbuzz": {
		needsBazelFile: true,
		patches:        []string{"//bazel/external/harfbuzz:config_files.patch"},
	},
	"icu": {
		needsBazelFile: true,
		patches:        []string{"//bazel/external/icu:icu_utils.patch"},
		patchCmds: []string{
			"rm source/i18n/BUILD.bazel",
			"rm source/common/BUILD.bazel",
			"rm source/stubdata/BUILD.bazel",
		},
		patchCmdsWin: []string{
			"del source/i18n/BUILD.bazel",
			"del source/common/BUILD.bazel",
			"del source/stubdata/BUILD.bazel",
		},
	},
	"icu4x":                    {needsBazelFile: true},
	"imgui":                    {needsBazelFile: true},
	"libavif":                  {needsBazelFile: true},
	"libgav1":                  {needsBazelFile: true},
	"libjpeg-turbo":            {bazelNameOverride: "libjpeg_turbo", needsBazelFile: true},
	"libjxl":                   {needsBazelFile: true},
	"libpng":                   {needsBazelFile: true},
	"libwebp":                  {needsBazelFile: true},
	"libyuv":                   {needsBazelFile: true},
	"spirv-cross":              {bazelNameOverride: "spirv_cross", needsBazelFile: true},
	"perfetto":                 {needsBazelFile: true},
	"piex":                     {needsBazelFile: true},
	"vello":                    {needsBazelFile: true},
	"vulkan-headers":           {bazelNameOverride: "vulkan_headers", needsBazelFile: true},
	"vulkan-tools":             {bazelNameOverride: "vulkan_tools", needsBazelFile: true},
	"vulkan-utility-libraries": {bazelNameOverride: "vulkan_utility_libraries", needsBazelFile: true},
	"vulkanmemoryallocator":    {needsBazelFile: true},
	"wuffs":                    {needsBazelFile: true},
	"zlib":                     {needsBazelFile: true},
}

func main() {
	var (
		depsFile    = flag.String("deps_file", "DEPS", "The location of the DEPS file. Usually at the root of the repository")
		genJSONFile = flag.String("gen_json_file", "bazel/deps.json", "The location of the .json file that has the generated data used to make Bazel git_repository rules.")
		// https://bazel.build/docs/user-manual#running-executables
		repoDir = flag.String("repo_dir", os.Getenv("BUILD_WORKSPACE_DIRECTORY"), "The root directory of the repo. Default set by BUILD_WORKSPACE_DIRECTORY env variable.")
	)
	flag.Parse()

	if *repoDir == "" {
		fmt.Println(`Must set --repo_dir
This is done automatically via:
    bazel run //bazel/deps_parser`)
		os.Exit(1)
	}

	fmt.Println(os.Environ())

	if *depsFile == "" || *genJSONFile == "" {
		fmt.Println("Must set --deps_file and --gen_json_file")
		flag.PrintDefaults()
	}

	if err := os.Chdir(*repoDir); err != nil {
		fmt.Printf("Could not cd to %s\n", *repoDir)
		os.Exit(1)
	}

	b, err := os.ReadFile(*depsFile)
	if err != nil {
		fmt.Printf("Could not open %s: %s\n", *depsFile, err)
		os.Exit(1)
	}
	contents := strings.Split(string(b), "\n")

	outputFile, count, err := parseDEPSFile(contents)
	if err != nil {
		fmt.Printf("Parsing error %s\n", err)
		os.Exit(1)
	}
	if err := moveWithCopyBackup(outputFile, *genJSONFile); err != nil {
		fmt.Printf("Could not write to generated .bzl file: %s\n", err)
		os.Exit(1)
	}
	fmt.Printf("Wrote %d deps\n", count)
}

type repoConfig struct {
	// https://bazel.build/rules/lib/repo/git
	BuildFile    string   `json:"build_file,omitempty"`
	Commit       string   `json:"commit"`
	Name         string   `json:"name"`
	PatchCmds    []string `json:"patch_cmds,omitempty"`
	PatchCmdsWin []string `json:"patch_cmds_win,omitempty"`
	Patches      []string `json:"patches,omitempty"`
	Remote       string   `json:"remote"`
}

type deps struct {
	Warning  string       `json:"!note"`
	Direct   []repoConfig `json:"direct"`
	Indirect []repoConfig `json:"indirect"`
}

func parseDEPSFile(contents []string) (string, int, error) {
	depsLine := regexp.MustCompile(`externals/(\S+)".+"(https.+)@([a-f0-9]+)"`)
	outputFile, err := os.CreateTemp("", "genbzl")
	if err != nil {
		return "", 0, fmt.Errorf("Could not create output file: %s\n", err)
	}
	defer outputFile.Close()

	var d deps
	d.Warning = "DO NOT MODIFY BY HAND. Instead, bazelisk run //bazel/deps_parser"
	for _, line := range contents {
		if match := depsLine.FindStringSubmatch(line); len(match) > 0 {
			rc := repoConfig{Name: match[1], Remote: match[2], Commit: match[3]}

			cfg, ok := depsOverrides[rc.Name]
			if !ok {
				continue
			}
			if cfg.bazelNameOverride != "" {
				rc.Name = cfg.bazelNameOverride
			}
			rc.Patches = cfg.patches
			rc.PatchCmds = cfg.patchCmds
			rc.PatchCmdsWin = cfg.patchCmdsWin
			if cfg.needsBazelFile {
				rc.BuildFile = fmt.Sprintf("//bazel/external/%s:BUILD.bazel", rc.Name)
			}
			if cfg.isIndirect {
				d.Indirect = append(d.Indirect, rc)
			} else {
				d.Direct = append(d.Direct, rc)
			}

		}
	}

	je := json.NewEncoder(outputFile)
	je.SetIndent("", "  ")
	if err := je.Encode(d); err != nil {
		return "", 0, fmt.Errorf("Could not encode json file: %s\n", err)
	}

	return outputFile.Name(), len(d.Direct) + len(d.Indirect), nil
}

func moveWithCopyBackup(src, dst string) error {
	// Atomically rename temp file to workspace. This should minimize the chance of corruption
	// or writing a partial file if there is an error or the program is interrupted.
	if err := os.Rename(src, dst); err != nil {
		// Errors can happen if the temporary file is on a different partition than the Skia
		// codebase. In that case, do a manual read/write to copy the data. See
		// https://github.com/jenkins-x/jx/issues/449 for a similar issue
		if strings.Contains(err.Error(), "invalid cross-device link") {
			bytes, err := os.ReadFile(src)
			if err != nil {
				return fmt.Errorf("Could not do backup read from %s: %s\n", src, err)
			}
			if err := os.WriteFile(dst, bytes, 0644); err != nil {
				return fmt.Errorf("Could not do backup write of %d bytes to %s: %s\n", len(bytes), dst, err)
			}
			// Backup "move" successful
			return nil
		}
		return fmt.Errorf("Could not write %s -> %s: %s\n", src, dst, err)
	}
	return nil
}
