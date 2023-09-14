// Copyright 2022 Google LLC
//
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// This executable tests the "layering_check" feature of our custom Bazel toolchain. It builds
// a target multiple times with different defines, which turn on a different "invalid" include
// and verifies that they fail.
package main

import (
	"context"
	"flag"
	"fmt"
	"path/filepath"

	sk_exec "go.skia.org/infra/go/exec"
	"go.skia.org/infra/task_driver/go/lib/bazel"
	"go.skia.org/infra/task_driver/go/lib/os_steps"
	"go.skia.org/infra/task_driver/go/td"
	"go.skia.org/skia/infra/bots/task_drivers/common"
)

var (
	// Required properties for this task.
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
		Label:  true,
		Config: true,
	})

	// StartRun calls flag.Parse()
	ctx := td.StartRun(projectId, taskId, taskName, output, local)
	defer td.EndRun(ctx)

	bazelFlags.Validate(ctx)

	wd, err := os_steps.Abs(ctx, *workdir)
	if err != nil {
		td.Fatal(ctx, err)
	}
	skiaDir := filepath.Join(wd, "skia")

	opts := bazel.BazelOptions{
		// We want the cache to be on a bigger disk than default. The root disk, where the home
		// directory (and default Bazel cache) lives, is only 15 GB on our GCE VMs.
		CachePath: *bazelFlags.CacheDir,
	}
	if err := bazel.EnsureBazelRCFile(ctx, opts); err != nil {
		td.Fatal(ctx, err)
	}

	// This build should succeed
	if err := bazelBuild(ctx, skiaDir, *bazelFlags.Label, *bazelFlags.Config); err != nil {
		td.Fatal(ctx, err)
	}

	// All of these should fail
	definesToIncludeExtraHeaders := []string{
		"HEADER_INCLUDES_TRANSITIVE_HEADER",
		"HEADER_INCLUDES_PRIVATE_HEADER",
		"SOURCE_INCLUDES_TRANSITIVE_HEADER",
		"SOURCE_INCLUDES_PRIVATE_HEADER",
	}
	for _, define := range definesToIncludeExtraHeaders {
		if err := expectFailure(ctx, skiaDir, *bazelFlags.Label, *bazelFlags.Config, define); err != nil {
			td.Fatal(ctx, err)
		}
	}

	if !*local {
		if err := common.BazelCleanIfLowDiskSpace(ctx, *bazelFlags.CacheDir, skiaDir, "bazelisk"); err != nil {
			td.Fatal(ctx, err)
		}
	}
}

// bazelBuild builds the target referenced by the given absolute label passing the provided
// config to Bazel command. Instead of calling Bazel directly, we use
func bazelBuild(ctx context.Context, checkoutDir, label, config string) error {
	step := fmt.Sprintf("Build %s", label)
	return td.Do(ctx, td.Props(step), func(ctx context.Context) error {
		runCmd := &sk_exec.Command{
			Name: "bazelisk",
			Args: append([]string{"build",
				label,
				"--config=" + config, // Should be defined in //bazel/buildrc
			}),
			InheritEnv: true, // Makes sure bazelisk is on PATH
			Dir:        checkoutDir,
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

// expectFailure builds the target with the given config and additional define. Note that this
// define affects *all* build commands, whereas setting a copt on the BUILD.bazel impacts that
// cc_library/cc_binary but not dependencies nor dependents (except headers from dependencies).
// It returns an error if the underlying build does *not* fail. We expect setting the define to
// include a file that is not allowed and our Bazel build should enforce that.
func expectFailure(ctx context.Context, checkoutDir, label, config, define string) error {
	step := fmt.Sprintf("Expect building %s with %s to fail", label, define)
	return td.Do(ctx, td.Props(step), func(ctx context.Context) error {
		runCmd := &sk_exec.Command{
			Name: "bazelisk",
			Args: append([]string{"build",
				label,
				"--config=" + config, // Should be defined in //bazel/buildrc
				"--copt=-D" + define + "=1",
			}),
			InheritEnv: true, // Makes sure bazelisk is on PATH
			Dir:        checkoutDir,
			LogStdout:  true,
			LogStderr:  true,
		}
		_, err := sk_exec.RunCommand(ctx, runCmd)
		if err == nil {
			return fmt.Errorf("Expected a failure but didn't get one.")
		}
		return nil
	})
}
