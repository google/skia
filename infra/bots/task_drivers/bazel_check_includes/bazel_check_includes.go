// Copyright 2022 Google LLC
//
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// This executable checks that we can build certain Bazel targets using --features skia_enforce_iwyu
// which validates that a subset of files are properly declaring all includes that they use.
package main

import (
	"context"
	"flag"
	"fmt"
	"path/filepath"
	"strconv"

	sk_exec "go.skia.org/infra/go/exec"
	"go.skia.org/infra/task_driver/go/lib/bazel"
	"go.skia.org/infra/task_driver/go/lib/os_steps"
	"go.skia.org/infra/task_driver/go/td"
)

// This value is arbitrarily selected. It is smaller than our maximum RBE pool size.
const rbeJobs = 100

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
	ctx := td.StartRun(projectId, taskId, taskName, output, local)
	defer td.EndRun(ctx)

	wd, err := os_steps.Abs(ctx, *workdir)
	if err != nil {
		td.Fatal(ctx, err)
	}
	skiaDir := filepath.Join(wd, "skia")

	opts := bazel.BazelOptions{
		// We want the cache to be on a bigger disk than default. The root disk, where the home
		// directory (and default Bazel cache) lives, is only 15 GB on our GCE VMs.
		CachePath: "/mnt/pd0/bazel_cache",
	}
	if err := bazel.EnsureBazelRCFile(ctx, opts); err != nil {
		td.Fatal(ctx, err)
	}

	targets := []string{"//example:hello_world_gl", "//example:hello_world_vulkan",
		"//example:hello_world_dawn", "//example:vulkan_basic", "//:skia_core", "//src/svg/...",
		"//src/utils/...",
	}
	for _, t := range targets {
		if err := bazelCheckIncludes(ctx, skiaDir, t); err != nil {
			td.Fatal(ctx, err)
		}
	}

	// Here are other configurations, e.g. with a GPU backend. These extra configurations
	// will make sure IWYU is happy with other #ifdef settings
	if err := bazelCheckIncludes(ctx, skiaDir, "//src/svg/...",
		"--gpu_backend=gl_backend", "--include_decoder=jpeg_decode_codec"); err != nil {
		td.Fatal(ctx, err)
	}

	if err := bazelCheckIncludes(ctx, skiaDir, "//tools/debugger", "--gpu_backend=gl_backend"); err != nil {
		td.Fatal(ctx, err)
	}

	if err := bazelCheckIncludes(ctx, skiaDir, "//src/utils/...", "--gpu_backend=gl_backend"); err != nil {
		td.Fatal(ctx, err)
	}
}

// bazelCheckIncludes builds the given label with the feature enabled that treats improper include
// declarations as a compilation error.
func bazelCheckIncludes(ctx context.Context, checkoutDir, label string, opts ...string) error {
	step := fmt.Sprintf("Build and check includes for %s with %d extra flags", label, len(opts))
	return td.Do(ctx, td.Props(step), func(ctx context.Context) error {
		runCmd := &sk_exec.Command{
			Name: "bazelisk",
			Args: append([]string{"build",
				"--config=linux-rbe", // Compile using RBE
				"--features=skia_enforce_iwyu",
				"--jobs=" + strconv.Itoa(rbeJobs),
				"--keep_going",              // Don't stop after first error
				"--remote_download_minimal", // Don't bother downloading the outputs
				label,
			}, opts...),
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
