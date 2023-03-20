// Copyright 2022 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package main

import (
	"flag"
	"fmt"
	"io"
	"os"
	"regexp"
	"sort"
	"strings"
)

type depConfig struct {
	bazelNameOverride string // Bazel style uses underscores not dashes, so we fix those if needed.
	needsBazelFile    bool
}

// These are all deps used by the Bazel build. They are a subset of those listed in DEPS.
// The key is the name of the repo as specified in DEPS.
var deps = map[string]depConfig{
	"abseil-cpp":  {bazelNameOverride: "abseil_cpp"},
	"brotli":      {},
	"highway":     {},
	"spirv-tools": {bazelNameOverride: "spirv_tools"},
	// This name is important because spirv_tools expects @spirv_headers to exist by that name.
	"spirv-headers": {bazelNameOverride: "spirv_headers"},

	"dawn":                  {needsBazelFile: true},
	"dng_sdk":               {needsBazelFile: true},
	"expat":                 {needsBazelFile: true},
	"freetype":              {needsBazelFile: true},
	"harfbuzz":              {needsBazelFile: true},
	"icu":                   {needsBazelFile: true},
	"libavif":               {needsBazelFile: true},
	"libgav1":               {needsBazelFile: true},
	"libjpeg-turbo":         {bazelNameOverride: "libjpeg_turbo", needsBazelFile: true},
	"libjxl":                {needsBazelFile: true},
	"libpng":                {needsBazelFile: true},
	"libwebp":               {needsBazelFile: true},
	"libyuv":                {needsBazelFile: true},
	"spirv-cross":           {bazelNameOverride: "spirv_cross", needsBazelFile: true},
	"perfetto":              {needsBazelFile: true},
	"piex":                  {needsBazelFile: true},
	"vulkan-headers":        {bazelNameOverride: "vulkan_headers", needsBazelFile: true},
	"vulkan-tools":          {bazelNameOverride: "vulkan_tools", needsBazelFile: true},
	"vulkanmemoryallocator": {needsBazelFile: true},
	"wuffs":                 {needsBazelFile: true},
	// Some other dependency downloads zlib but with their own rules
	"zlib": {bazelNameOverride: "zlib_skia", needsBazelFile: true},
}

func main() {
	var (
		depsFile      = flag.String("deps_file", "DEPS", "The location of the DEPS file. Usually at the root of the repository")
		genBzlFile    = flag.String("gen_bzl_file", "bazel/deps.bzl", "The location of the .bzl file that has the generated Bazel repository rules.")
		workspaceFile = flag.String("workspace_file", "WORKSPACE.bazel", "The location of the WORKSPACE file that should be updated with dep names.")
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

	outputFile, count, err := parseDEPSFile(contents, *workspaceFile)
	if err := os.Rename(outputFile, *genBzlFile); err != nil {
		fmt.Printf("Could not write from %s to %s: %s\n", outputFile, *depsFile, err)
		os.Exit(1)
	}
	fmt.Printf("Wrote %d deps\n", count)
}

func parseDEPSFile(contents []string, workspaceFile string) (string, int, error) {
	depsLine := regexp.MustCompile(`externals/(\S+)".+"(https.+)@([a-f0-9]+)"`)
	outputFile, err := os.CreateTemp("", "genbzl")
	if err != nil {
		return "", 0, fmt.Errorf("Could not create output file: %s\n", err)
	}
	defer outputFile.Close()

	if _, err := outputFile.WriteString(header); err != nil {
		return "", 0, fmt.Errorf("Could not write to output file %s: %s\n", outputFile.Name(), err)
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
				if err := writeNewGitRepositoryRule(outputFile, id, repo, rev); err != nil {
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

	if newWorkspaceFile, err := writeCommentsToWorkspace(workspaceFile, nativeRepos, providedRepos); err != nil {
		fmt.Printf("Could not parse workspace file %s: %s\n", workspaceFile, err)
		os.Exit(1)
	} else {
		// Atomically rename temp file to workspace. This should minimize the chance of corruption
		// or writing a partial file if there is an error or the program is interrupted.
		if err := os.Rename(newWorkspaceFile, workspaceFile); err != nil {
			fmt.Printf("Could not write comments in workspace file %s -> %s: %s\n", newWorkspaceFile, workspaceFile, err)
			os.Exit(1)
		}
	}
	return outputFile.Name(), count, nil
}

func writeCommentsToWorkspace(workspaceFile string, nativeRepos, providedRepos []string) (string, error) {
	b, err := os.ReadFile(workspaceFile)
	if err != nil {
		return "", fmt.Errorf("Could not open %s: %s\n", workspaceFile, err)
	}
	newWorkspace, err := os.CreateTemp("", "workspace")
	if err != nil {
		return "", fmt.Errorf("Could not make tempfile: %s\n", err)
	}
	defer newWorkspace.Close()

	workspaceContents := strings.Split(string(b), "\n")

	sort.Strings(nativeRepos)
	sort.Strings(providedRepos)
	for _, line := range workspaceContents {
		if _, err := newWorkspace.WriteString(line + "\n"); err != nil {
			return "", err
		}
		if line == startListString {
			break
		}
	}
	for _, repoLine := range nativeRepos {
		if _, err := newWorkspace.WriteString(repoLine + "\n"); err != nil {
			return "", err
		}
	}
	if _, err := newWorkspace.WriteString("#\n"); err != nil {
		return "", err
	}
	for _, repoLine := range providedRepos {
		if _, err := newWorkspace.WriteString(repoLine + "\n"); err != nil {
			return "", err
		}
	}
	if _, err := newWorkspace.WriteString(endListString + "\n"); err != nil {
		return "", err
	}

	pastEnd := false
	// Skip the last line, which is blank. We don't want to end with two empty newlines.
	for _, line := range workspaceContents[:len(workspaceContents)-1] {
		if line == endListString {
			pastEnd = true
			continue
		}
		if !pastEnd {
			continue
		}
		if _, err := newWorkspace.WriteString(line + "\n"); err != nil {
			return "", err
		}
	}

	return newWorkspace.Name(), nil
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

def git_repos_from_deps():
    """A list of native Bazel git rules to download third party git repositories

       These are in the order they appear in //DEPS.
        https://bazel.build/rules/lib/repo/git
    """`

func writeNewGitRepositoryRule(w io.StringWriter, bazelName, repo, rev string) error {
	// TODO(kjlubick) In a newer version of Bazel, new_git_repository can be replaced with just
	// git_repository
	_, err := w.WriteString(fmt.Sprintf(`
    new_git_repository(
        name = "%s",
        build_file = "@//bazel/external/%s:BUILD.bazel",
        commit = "%s",
        remote = "%s",
    )
`, bazelName, bazelName, rev, repo))
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
