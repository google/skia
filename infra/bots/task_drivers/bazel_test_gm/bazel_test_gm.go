// Copyright 2023 Google LLC
//
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This task driver runs a single Bazel test target representing one or more GMs, or a Bazel test
// suite consisting of multiple such targets, using the provided config (which is assumed to be in
// //bazel/buildrc). GM results are uploaded to Gold via goldctl. This task driver handles any
// setup steps needed to run Bazel on our CI machines before running the task, such as setting up
// logs and the Bazel cache.

package main

import (
	"context"
	"flag"
	"fmt"
	"path/filepath"

	sk_exec "go.skia.org/infra/go/exec"
	"go.skia.org/infra/go/skerr"
	"go.skia.org/infra/task_driver/go/lib/bazel"
	"go.skia.org/infra/task_driver/go/lib/os_steps"
	"go.skia.org/infra/task_driver/go/td"
	"go.skia.org/skia/infra/bots/task_drivers/common"
)

var (
	// Required properties for this task.
	//
	// We want the cache to be on a bigger disk than default. The root disk, where the home directory
	// (and default Bazel cache) lives, is only 15 GB on our GCE VMs.
	cachePath = flag.String("cache_path", "/mnt/pd0/bazel_cache", "The path where the Bazel cache should live. This should be able to hold tens of GB at least.")
	cross     = flag.String("cross", "", "An identifier specifying the target platform that Bazel should build for. If empty, Bazel builds for the host platform (the machine on which this executable is run).")
	label     = flag.String("test_label", "", "The label of the Bazel target to test.")
	config    = flag.String("test_config", "", "A custom configuration specified in //bazel/buildrc. This configuration potentially encapsulates many features and options.")
	projectId = flag.String("project_id", "", "ID of the Google Cloud project.")
	taskId    = flag.String("task_id", "", "ID of this task.")
	taskName  = flag.String("task_name", "", "Name of the task.")
	workdir   = flag.String("workdir", ".", "Working directory.")

	// goldctl data.
	goldctlPath      = flag.String("goldctl_path", "", "The path to the golctl binary on disk.")
	gitCommit        = flag.String("git_commit", "", "The git hash to which the data should be associated. This will be used when changelist_id and patchset_order are not set to report data to Gold that belongs on the primary branch.")
	changelistID     = flag.String("changelist_id", "", "Should be non-empty only when run on the CQ.")
	patchsetOrderStr = flag.String("patchset_order", "", "Should be non-zero only when run on the CQ.")
	tryjobID         = flag.String("tryjob_id", "", "Should be non-zero only when run on the CQ.")

	// Optional flags.
	local  = flag.Bool("local", false, "True if running locally (as opposed to on the CI/CQ)")
	output = flag.String("o", "", "If provided, dump a JSON blob of step data to the given file. Prints to stdout if '-' is given.")
)

func main() {
	// StartRun calls flag.Parse().
	ctx := td.StartRun(projectId, taskId, taskName, output, local)
	defer td.EndRun(ctx)

	if *label == "" {
		td.Fatal(ctx, fmt.Errorf("--test_label is required"))
	}

	if *config == "" {
		td.Fatal(ctx, fmt.Errorf("--test_config is required"))
	}

	wd, err := os_steps.Abs(ctx, *workdir)
	if err != nil {
		td.Fatal(ctx, err)
	}

	opts := bazel.BazelOptions{
		CachePath: *cachePath,
	}
	if err := bazel.EnsureBazelRCFile(ctx, opts); err != nil {
		td.Fatal(ctx, err)
	}

	if *cross != "" {
		// See https://bazel.build/concepts/platforms-intro and https://bazel.build/docs/platforms when
		// ready to support this.
		td.Fatal(ctx, fmt.Errorf("cross compilation not yet supported"))
	}

	if err := run(ctx, taskDriverArgs{
		UploadToGoldArgs: common.UploadToGoldArgs{
			BazelLabel:    *label,
			GoldctlPath:   filepath.Join(wd, *goldctlPath),
			GitCommit:     *gitCommit,
			ChangelistID:  *changelistID,
			PatchsetOrder: *patchsetOrderStr,
			TryjobID:      *tryjobID,
		},
		checkoutDir: filepath.Join(wd, "skia"),
		bazelConfig: *config,
	}); err != nil {
		td.Fatal(ctx, err)
	}
}

// taskDriverArgs gathers the inputs to this task driver, and decouples the task driver's
// entry-point function from the command line flags, which facilitates writing unit tests.
type taskDriverArgs struct {
	common.UploadToGoldArgs

	checkoutDir string
	bazelConfig string
}

// run is the entrypoint of this task driver.
func run(ctx context.Context, tdArgs taskDriverArgs) error {
	outputsZipPath, err := common.ValidateLabelAndReturnOutputsZipPath(tdArgs.checkoutDir, tdArgs.BazelLabel)
	if err != nil {
		return skerr.Wrap(err)
	}

	if err := bazelTest(ctx, tdArgs.checkoutDir, tdArgs.BazelLabel, tdArgs.bazelConfig); err != nil {
		return skerr.Wrap(err)
	}

	if err := common.UploadToGold(ctx, tdArgs.UploadToGoldArgs, outputsZipPath); err != nil {
		return skerr.Wrap(err)
	}

	return nil
}

// bazelTest runs the test referenced by the given fully qualified Bazel label under the given
// config.
func bazelTest(ctx context.Context, checkoutDir, label, config string) error {
	return td.Do(ctx, td.Props(fmt.Sprintf("Test %s with config %s", label, config)), func(ctx context.Context) error {
		runCmd := &sk_exec.Command{
			Name: "bazelisk",
			Args: []string{"test",
				label,
				"--config=" + config, // Should be defined in //bazel/buildrc.
				"--test_output=errors",
				"--jobs=100",
			},
			InheritEnv: true, // Makes sure bazelisk is on PATH.
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
