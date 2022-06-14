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

	"go.skia.org/infra/go/common"
	sk_exec "go.skia.org/infra/go/exec"
	"go.skia.org/infra/task_driver/go/lib/bazel"
	"go.skia.org/infra/task_driver/go/lib/os_steps"
	"go.skia.org/infra/task_driver/go/td"
)

var (
	// Required properties for this task.
	bazelArgs    = common.NewMultiStringFlag("bazel_arg", nil, "Additional arguments that should be forwarded directly to the Bazel invocation.")
	cross        = flag.String("cross", "", "An identifier specifying the target platform that Bazel should build for. If empty, Bazel builds for the host platform (the machine on which this executable is run).")
	config       = flag.String("config", "", "A custom configuration specified in //bazel/buildrc. This configuration potentially encapsulates many features and options.")
	expungeCache = flag.Bool("expunge_cache", false, "If set, the Bazel cache will be cleaned with --expunge before execution. We should only have to set this rarely, if something gets messed up.")
	projectId    = flag.String("project_id", "", "ID of the Google Cloud project.")
	label        = flag.String("label", "", "An absolute label to the target that should be built.")
	taskId       = flag.String("task_id", "", "ID of this task.")
	taskName     = flag.String("task_name", "", "Name of the task.")
	workdir      = flag.String("workdir", ".", "Working directory, the root directory of a full Skia checkout")
	// Optional flags.
	local  = flag.Bool("local", false, "True if running locally (as opposed to on the CI/CQ)")
	output = flag.String("o", "", "If provided, dump a JSON blob of step data to the given file. Prints to stdout if '-' is given.")
)

func main() {
	// StartRun calls flag.Parse()
	ctx := td.StartRun(projectId, taskId, taskName, output, local)
	defer td.EndRun(ctx)

	if *label == "" || *config == "" {
		td.Fatal(ctx, fmt.Errorf("--label and --config are required"))
	}

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

	if *cross != "" {
		// See https://bazel.build/concepts/platforms-intro and https://bazel.build/docs/platforms
		// when ready to support this.
		td.Fatal(ctx, fmt.Errorf("cross compilation not yet supported"))
	}

	if *expungeCache {
		if err := bazelClean(ctx, skiaDir); err != nil {
			td.Fatal(ctx, err)
		}
	}

	if err := bazelBuild(ctx, skiaDir, *label, *config, *bazelArgs...); err != nil {
		td.Fatal(ctx, err)
	}
}

// bazelBuild builds the target referenced by the given absolute label passing the provided
// config and any additional args to the build command. Instead of calling Bazel directly, we use
// Bazelisk to make sure we use the right version of Bazel, as defined in the .bazelversion file
// at the Skia root.
func bazelBuild(ctx context.Context, checkoutDir, label, config string, args ...string) error {
	step := fmt.Sprintf("Build %s with config %s and %d extra flags", label, config, len(args))
	return td.Do(ctx, td.Props(step), func(ctx context.Context) error {
		runCmd := &sk_exec.Command{
			Name: "bazelisk",
			Args: append([]string{"build",
				label,
				"--config=" + config, // Should be defined in //bazel/buildrc
			}, args...),
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

// bazelClean cleans the bazel cache and the external directory via the --expunge flag.
func bazelClean(ctx context.Context, checkoutDir string) error {
	return td.Do(ctx, td.Props("Cleaning cache with --expunge"), func(ctx context.Context) error {
		runCmd := &sk_exec.Command{
			Name:       "bazelisk",
			Args:       append([]string{"clean", "--expunge"}),
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
