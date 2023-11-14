// Copyright 2022 Google LLC
//
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// This executable runs a Bazel(isk) build command for a single label using the provided
// config (which is assumed to be in //bazel/buildrc) and any provided Bazel args.
// This handles any setup needed to run Bazel on our CI machines before running the task, like
// setting up logs and the Bazel cache.
package main

import (
	"context"
	"flag"
	"fmt"
	"path/filepath"
	"strings"

	sk_exec "go.skia.org/infra/go/exec"
	"go.skia.org/infra/task_driver/go/lib/bazel"
	"go.skia.org/infra/task_driver/go/lib/os_steps"
	"go.skia.org/infra/task_driver/go/td"
	"go.skia.org/skia/infra/bots/task_drivers/common"
)

var (
	// Required properties for this task.
	// We want the cache to be on a bigger disk than default. The root disk, where the home
	// directory (and default Bazel cache) lives, is only 15 GB on our GCE VMs.
	gitPath   = flag.String("git_path", "", "Location of git binary to use for diffs.")
	projectId = flag.String("project_id", "", "ID of the Google Cloud project.")
	taskId    = flag.String("task_id", "", "ID of this task.")
	taskName  = flag.String("task_name", "", "Name of the task.")
	workdir   = flag.String("workdir", ".", "Working directory, the root directory of a full Skia checkout")
	// Optional flags.
	local  = flag.Bool("local", false, "True if running locally (as opposed to on the CI/CQ)")
	output = flag.String("o", "", "If provided, dump a JSON blob of step data to the given file. Prints to stdout if '-' is given.")
)

func main() {
	bazelFlags := common.MakeBazelFlags(common.MakeBazelFlagsOpts{
		AdditionalArgs: true,
	})

	// StartRun calls flag.Parse()
	ctx := td.StartRun(projectId, taskId, taskName, output, local)
	defer td.EndRun(ctx)

	bazelFlags.Validate(ctx)

	if *gitPath == "" {
		td.Fatal(ctx, fmt.Errorf("--git_path is required"))
	}

	wd, err := os_steps.Abs(ctx, *workdir)
	if err != nil {
		td.Fatal(ctx, err)
	}
	absGit, err := os_steps.Abs(ctx, *gitPath)
	if err != nil {
		td.Fatal(ctx, err)
	}
	if _, err := os_steps.Stat(ctx, absGit); err != nil {
		fmt.Printf("Cannot stat git binary %s\n", absGit)
		td.Fatal(ctx, err)
	}
	skiaPath := filepath.Join(wd, "skia")

	// When running on the CI, there is not a git checkout here, so we make a temp one.
	if !*local {
		if err := gitInit(ctx, absGit, skiaPath); err != nil {
			td.Fatal(ctx, err)
		}
	}

	opts := bazel.BazelOptions{
		CachePath: *bazelFlags.CacheDir,
	}
	if err := bazel.EnsureBazelRCFile(ctx, opts); err != nil {
		td.Fatal(ctx, err)
	}

	if err := bazelRun(ctx, skiaPath, "//bazel/deps_parser", *bazelFlags.AdditionalArgs...); err != nil {
		td.Fatal(ctx, err)
	}

	if err := bazelRun(ctx, skiaPath, "//tools/gpu/gl/interface:generate_gl_interfaces", *bazelFlags.AdditionalArgs...); err != nil {
		td.Fatal(ctx, err)
	}

	skslTests := []string{
		"compile_glsl_tests",
		"compile_glsl_nosettings_tests",
		"compile_metal_tests",
		"compile_skrp_tests",
		"compile_stage_tests",
		"compile_spirv_tests",
		"compile_wgsl_tests",
	}
	for _, label := range skslTests {
		if err := bazelRun(ctx, skiaPath, "//tools/skslc:"+label, *bazelFlags.AdditionalArgs...); err != nil {
			td.Fatal(ctx, err)
		}
	}

	if err := bazelRun(ctx, skiaPath, "//tools:generate_workarounds", *bazelFlags.AdditionalArgs...); err != nil {
		td.Fatal(ctx, err)
	}

	if err := bazelRun(ctx, skiaPath, "//tools/sksl-minify:minify_srcs", *bazelFlags.AdditionalArgs...); err != nil {
		td.Fatal(ctx, err)
	}

	if err := bazelRun(ctx, skiaPath, "//tools/sksl-minify:minify_tests", *bazelFlags.AdditionalArgs...); err != nil {
		td.Fatal(ctx, err)
	}

	if err := generateGNIFiles(ctx, skiaPath); err != nil {
		td.Fatal(ctx, err)
	}

	if err := checkGitDiff(ctx, absGit, skiaPath); err != nil {
		td.Fatal(ctx, err)
	}

	if !*local {
		if err := common.BazelCleanIfLowDiskSpace(ctx, *bazelFlags.CacheDir, skiaPath, "bazelisk"); err != nil {
			td.Fatal(ctx, err)
		}
	}
}

// bazelRun runs the given Bazel label from the Skia path with any given args using Bazelisk.
func bazelRun(ctx context.Context, skiaPath, label string, args ...string) error {
	return td.Do(ctx, td.Props("bazel run "+label), func(ctx context.Context) error {
		runCmd := &sk_exec.Command{
			Name:       "bazelisk",
			Args:       append([]string{"run", label}, args...),
			InheritEnv: true, // Need to make sure bazelisk is on the path
			Dir:        skiaPath,
			LogStdout:  true,
			LogStderr:  true,
		}
		_, err := sk_exec.RunCommand(ctx, runCmd)
		if err != nil {
			return err
		}
		return nil
	})
}

// gitInit creates a temporary git repository with all files in the Skia path. This allows the later
// git diff call to work properly. This is necessary because our current Swarming setup does not
// include the .git folder when copying down files.
func gitInit(ctx context.Context, gitPath, skiaPath string) error {
	step := fmt.Sprintf("Setting git baseline in %s", skiaPath)
	err := td.Do(ctx, td.Props(step), func(ctx context.Context) error {
		initCmd := &sk_exec.Command{
			Name:       gitPath,
			Args:       []string{"init"},
			InheritEnv: false,
			Dir:        skiaPath,
			LogStdout:  true,
			LogStderr:  true,
		}
		if _, err := sk_exec.RunCommand(ctx, initCmd); err != nil {
			return err
		}
		addCmd := &sk_exec.Command{
			Name:       gitPath,
			Args:       []string{"add", "."},
			InheritEnv: false,
			Dir:        skiaPath,
			LogStdout:  true,
			LogStderr:  true,
		}
		if _, err := sk_exec.RunCommand(ctx, addCmd); err != nil {
			return err
		}
		commitCmd := &sk_exec.Command{
			Name:       gitPath,
			Args:       []string{"commit", "-m", "baseline commit"},
			InheritEnv: false,
			Dir:        skiaPath,
			LogStdout:  true,
			LogStderr:  true,
		}
		if _, err := sk_exec.RunCommand(ctx, commitCmd); err != nil {
			return err
		}
		return nil
	})
	return err
}

// generateGNIFiles re-generates the .gni files from BUILD.bazel files, allowing interopt between
// the new system (Bazel) and the old one GN.
func generateGNIFiles(ctx context.Context, skiaPath string) error {
	return td.Do(ctx, td.Props("Generate GNI files from BUILD.bazel ones"), func(ctx context.Context) error {
		// Note: This is not done with bazel run ... because the exporter_tool calls Bazel, causing
		// an apparent deadlock because there can only be one running bazel task at a time.
		runCmd := &sk_exec.Command{
			Name:       "make",
			Args:       []string{"-C", "bazel", "generate_gni_rbe"},
			InheritEnv: true, // Need to make sure bazelisk is on the path,
			Dir:        skiaPath,
			LogStdout:  true,
			LogStderr:  true,
		}
		if _, err := sk_exec.RunCommand(ctx, runCmd); err != nil {
			return err
		}
		return nil
	})
}

// checkGitDiff runs git diff and returns error if the diff is non-empty.
func checkGitDiff(ctx context.Context, gitPath, skiaPath string) error {
	step := fmt.Sprintf("git diff %s", skiaPath)
	return td.Do(ctx, td.Props(step), func(ctx context.Context) error {
		runCmd := &sk_exec.Command{
			Name:       gitPath,
			Args:       []string{"diff", "--no-ext-diff"},
			InheritEnv: false,
			Dir:        skiaPath,
			LogStdout:  true,
			LogStderr:  true,
		}
		rv, err := sk_exec.RunCommand(ctx, runCmd)
		if err != nil {
			return err
		}
		if strings.TrimSpace(rv) != "" {
			return fmt.Errorf("Non-empty diff:\n" + rv)
		}
		return nil
	})
}
