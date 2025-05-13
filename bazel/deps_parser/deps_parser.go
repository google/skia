// Copyright 2022 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package main

import (
	"flag"
	"fmt"
	"io"
	"os"
	"os/exec"
	"path/filepath"
	"regexp"
	"strings"
)

type depConfig struct {
	bazelNameOverride string // Bazel style uses underscores not dashes, so we fix those if needed.
	needsBazelFile    bool
	patches           []string
	patchCmds         []string
	patchCmdsWin      []string
}

// These are all C++ deps or Rust deps (with a compatible C++ FFI) used by the Bazel build.
// They are a subset of those listed in DEPS.
// The key is the name of the repo as specified in DEPS.
var deps = map[string]depConfig{
	"abseil-cpp":  {bazelNameOverride: "abseil_cpp"},
	"brotli":      {},
	"highway":     {},
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
			`"rm source/i18n/BUILD.bazel"`,
			`"rm source/common/BUILD.bazel"`,
			`"rm source/stubdata/BUILD.bazel"`,
		},
		patchCmdsWin: []string{
			`"del source/i18n/BUILD.bazel"`,
			`"del source/common/BUILD.bazel"`,
			`"del source/stubdata/BUILD.bazel"`,
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
		depsFile   = flag.String("deps_file", "DEPS", "The location of the DEPS file. Usually at the root of the repository")
		genBzlFile = flag.String("gen_bzl_file", "bazel/deps.bzl", "The location of the .bzl file that has the generated Bazel repository rules.")
		// https://bazel.build/docs/user-manual#running-executables
		repoDir        = flag.String("repo_dir", os.Getenv("BUILD_WORKSPACE_DIRECTORY"), "The root directory of the repo. Default set by BUILD_WORKSPACE_DIRECTORY env variable.")
		buildifierPath = flag.String("buildifier", "", "Where to find buildifier. Defaults to Bazel's location")
	)
	flag.Parse()

	if *repoDir == "" {
		fmt.Println(`Must set --repo_dir
This is done automatically via:
    bazel run //bazel/deps_parser`)
		os.Exit(1)
	}

	buildifier := *buildifierPath
	if buildifier == "" {
		// We don't know if this will be buildifier_linux_x64, buildifier_macos_arm64, etc
		bp, err := filepath.Glob("../buildifier*/file/buildifier")
		if err != nil || len(bp) != 1 {
			fmt.Printf("Could not find exactly one buildifier executable %s %v\n", err, bp)
			os.Exit(1)
		}
		buildifier = bp[0]
	}
	buildifier, err := filepath.Abs(buildifier)
	if err != nil {
		fmt.Printf("Abs path error %s\n", err)
		os.Exit(1)
	}

	fmt.Println(os.Environ())

	if *depsFile == "" || *genBzlFile == "" {
		fmt.Println("Must set --deps_file and --gen_bzl_file")
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
	if err := exec.Command(buildifier, "--mode=fix", "--lint=fix", outputFile).Run(); err != nil {
		fmt.Printf("Buildifier error on %s %s\n", outputFile, err)
		os.Exit(1)
	}
	if err := moveWithCopyBackup(outputFile, *genBzlFile); err != nil {
		fmt.Printf("Could not write to generated .bzl file: %s\n", err)
		os.Exit(1)
	}
	fmt.Printf("Wrote %d deps\n", count)
}

func parseDEPSFile(contents []string) (string, int, error) {
	depsLine := regexp.MustCompile(`externals/(\S+)".+"(https.+)@([a-f0-9]+)"`)
	outputFile, err := os.CreateTemp("", "genbzl")
	if err != nil {
		return "", 0, fmt.Errorf("Could not create output file: %s\n", err)
	}
	defer outputFile.Close()

	if _, err := outputFile.WriteString(header); err != nil {
		return "", 0, fmt.Errorf("Could not write header to output file %s: %s\n", outputFile.Name(), err)
	}

	var nativeRepos []string
	var providedRepos []string

	count := 0
	for _, line := range contents {
		if match := depsLine.FindStringSubmatch(line); len(match) > 0 {
			id := match[1]
			repo := match[2]
			rev := match[3]

			cfg, ok := deps[id]
			if !ok {
				continue
			}
			if cfg.bazelNameOverride != "" {
				id = cfg.bazelNameOverride
			}
			if cfg.needsBazelFile {
				if err := writeNewGitRepositoryRule(outputFile, id, repo, rev, cfg.patches, cfg.patchCmds, cfg.patchCmdsWin); err != nil {
					return "", 0, fmt.Errorf("Could not write to output file %s: %s\n", outputFile.Name(), err)
				}
				workspaceLine := fmt.Sprintf("# @%s - //bazel/external/%s:BUILD.bazel", id, id)
				providedRepos = append(providedRepos, workspaceLine)
			} else {
				if err := writeGitRepositoryRule(outputFile, id, repo, rev); err != nil {
					return "", 0, fmt.Errorf("Could not write to output file %s: %s\n", outputFile.Name(), err)
				}
				workspaceLine := fmt.Sprintf("# @%s - %s", id, repo)
				nativeRepos = append(nativeRepos, workspaceLine)
			}
			count++
		}
	}
	if count != len(deps) {
		return "", 0, fmt.Errorf("Not enough deps written. Maybe the deps dictionary needs a bazelNameOverride or an old dep needs to be removed?")
	}

	if _, err := outputFile.WriteString(footer); err != nil {
		return "", 0, fmt.Errorf("Could not write footer to output file %s: %s\n", outputFile.Name(), err)
	}

	return outputFile.Name(), count, nil
}

const (
	startListString = `#### START GENERATED LIST OF THIRD_PARTY DEPS`
	endListString   = `#### END GENERATED LIST OF THIRD_PARTY DEPS`
)

const header = `"""
This file is auto-generated from //bazel/deps_parser
DO NOT MODIFY BY HAND.
Instead, do:
    bazel run //bazel/deps_parser
"""

load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository", "new_git_repository")

def _c_plus_plus_modules_impl(ctx):
    """A list of native Bazel git rules to download third party git repositories

       These are in the order they appear in //DEPS.
        https://bazel.build/rules/lib/repo/git

    Args:
      ctx: https://bazel.build/rules/lib/builtins/module_ctx
    """`

// If necessary, we can make a new map for bazel deps
const footer = `

c_plus_plus_modules = module_extension(
    implementation = _c_plus_plus_modules_impl,
)
`

func writeNewGitRepositoryRule(w io.StringWriter, bazelName, repo, rev string, patches, patchCmds, patchCmdsWin []string) error {
	_, err := w.WriteString(fmt.Sprintf(`
    new_git_repository(
        name = "%s",
        build_file = "//bazel/external/%s:BUILD.bazel",
        commit = "%s",
        remote = "%s",
        `, bazelName, bazelName, rev, repo))
	if err != nil {
		return err
	}
	if len(patches) > 0 {
		patch_files := `["` + strings.Join(patches, `",\n"`) + `"]`
		_, err := w.WriteString(fmt.Sprintf("patches = %s,\n", patch_files))
		if err != nil {
			return err
		}
	}
	if len(patchCmds) > 0 {
		patches_unix := "[" + strings.Join(patchCmds, ",\n") + "]"
		_, err := w.WriteString(fmt.Sprintf("patch_cmds = %s,\n", patches_unix))
		if err != nil {
			return err
		}
	}
	if len(patchCmdsWin) > 0 {
		patches_win := "[" + strings.Join(patchCmdsWin, ",\n") + "]"
		_, err := w.WriteString(fmt.Sprintf("patch_cmds_win = %s,\n", patches_win))
		if err != nil {
			return err
		}
	}
	_, err = w.WriteString(")\n")
	return err
}

func writeGitRepositoryRule(w io.StringWriter, bazelName, repo, rev string) error {
	_, err := w.WriteString(fmt.Sprintf(`
    git_repository(
        name = "%s",
        commit = "%s",
        remote = "%s",
    )
`, bazelName, rev, repo))
	return err
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
