// Copyright 2023 Google LLC
//
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This task driver runs a single Bazel test target representing one or more benchmarks, or a Bazel
// test suite consisting of multiple such targets, using the provided config (which is assumed to
// be in //bazel/buildrc). Benchmark results are uploaded to a GCS bucket for ingestion by Perf.
// This task driver handles any setup steps needed to run Bazel on our CI machines before running
// the task, such as setting up logs and the Bazel cache.

package main

import (
	"context"
	"flag"
	"fmt"
	"path/filepath"

	"cloud.google.com/go/storage"
	"go.skia.org/infra/go/auth"
	sk_exec "go.skia.org/infra/go/exec"
	"go.skia.org/infra/go/gcs"
	"go.skia.org/infra/go/gcs/gcsclient"
	"go.skia.org/infra/go/skerr"
	"go.skia.org/infra/task_driver/go/lib/auth_steps"
	"go.skia.org/infra/task_driver/go/lib/bazel"
	"go.skia.org/infra/task_driver/go/lib/os_steps"
	"go.skia.org/infra/task_driver/go/td"
	"go.skia.org/skia/infra/bots/task_drivers/common"
	"google.golang.org/api/option"
)

var (
	// Required properties for this task.
	//
	// We want the cache to be on a bigger disk than default. The root disk, where the home directory
	// (and default Bazel cache) lives, is only 15 GB on our GCE VMs.
	projectId = flag.String("project_id", "", "ID of the Google Cloud project.")
	taskID    = flag.String("task_id", "", "ID of this task.")
	taskName  = flag.String("task_name", "", "Name of the task.")
	workdir   = flag.String("workdir", ".", "Working directory.")

	gitCommit        = flag.String("git_commit", "", "The git hash to which the data should be associated.")
	changelistID     = flag.String("changelist_id", "", "Should be non-empty only when run on the CQ.")
	patchsetOrderStr = flag.String("patchset_order", "", "Should be non-zero only when run on the CQ.")

	// Optional flags.
	local  = flag.Bool("local", false, "True if running locally (as opposed to on the CI/CQ)")
	output = flag.String("o", "", "If provided, dump a JSON blob of step data to the given file. Prints to stdout if '-' is given.")
)

func main() {
	bazelFlags := common.MakeBazelFlags(common.MakeBazelFlagsOpts{
		Label:  true,
		Config: true,
	})

	// StartRun calls flag.Parse().
	ctx := td.StartRun(projectId, taskID, taskName, output, local)
	defer td.EndRun(ctx)

	bazelFlags.Validate(ctx)

	wd, err := os_steps.Abs(ctx, *workdir)
	if err != nil {
		td.Fatal(ctx, err)
	}

	opts := bazel.BazelOptions{
		CachePath: *bazelFlags.CacheDir,
	}
	if err := bazel.EnsureBazelRCFile(ctx, opts); err != nil {
		td.Fatal(ctx, err)
	}

	// Make an HTTP client with the required permissions to upload to the perf.skia.org GCS bucket.
	httpClient, _, err := auth_steps.InitHttpClient(ctx, *local, auth.ScopeReadWrite, auth.ScopeUserinfoEmail)
	if err != nil {
		td.Fatal(ctx, skerr.Wrap(err))
	}

	// Make a GCS client to to upload to the perf.skia.org GCS bucket.
	store, err := storage.NewClient(ctx, option.WithHTTPClient(httpClient))
	if err != nil {
		td.Fatal(ctx, skerr.Wrap(err))
	}
	gcsClient := gcsclient.New(store, common.PerfGCSBucketName)

	if err := run(ctx, *bazelFlags.CacheDir, taskDriverArgs{
		BenchmarkInfo: common.BenchmarkInfo{
			GitCommit:     *gitCommit,
			TaskName:      *taskName,
			TaskID:        *taskID,
			ChangelistID:  *changelistID,
			PatchsetOrder: *patchsetOrderStr,
		},
		checkoutDir: filepath.Join(wd, "skia"),
		bazelLabel:  *bazelFlags.Label,
		bazelConfig: *bazelFlags.Config,
	}, gcsClient); err != nil {
		td.Fatal(ctx, err)
	}
}

// taskDriverArgs gathers the inputs to this task driver, and decouples the task driver's
// entry-point function from the command line flags, which facilitates writing unit tests.
type taskDriverArgs struct {
	common.BenchmarkInfo
	checkoutDir string
	bazelLabel  string
	bazelConfig string
}

// run is the entrypoint of this task driver.
func run(ctx context.Context, bazelCacheDir string, tdArgs taskDriverArgs, gcsClient gcs.GCSClient) error {
	outputsZipPath, err := common.ValidateLabelAndReturnOutputsZipPath(tdArgs.checkoutDir, tdArgs.bazelLabel)
	if err != nil {
		return skerr.Wrap(err)
	}

	testArgs := common.ComputeBenchmarkTestRunnerCLIFlags(ctx, tdArgs.BenchmarkInfo)
	if err := bazelTest(ctx, tdArgs.checkoutDir, tdArgs.bazelLabel, tdArgs.bazelConfig, testArgs); err != nil {
		return skerr.Wrap(err)
	}

	if err := common.UploadToPerf(ctx, gcsClient, tdArgs.BenchmarkInfo, outputsZipPath); err != nil {
		return skerr.Wrap(err)
	}

	if !*local {
		if err := common.BazelCleanIfLowDiskSpace(ctx, bazelCacheDir, tdArgs.checkoutDir, "bazelisk"); err != nil {
			return skerr.Wrap(err)
		}
	}

	return nil
}

// bazelTest runs the test referenced by the given fully qualified Bazel label under the given
// config.
func bazelTest(ctx context.Context, checkoutDir, label, config string, testArgs []string) error {
	args := []string{"test",
		label,
		"--config=" + config, // Should be defined in //bazel/buildrc.
		"--test_output=errors",
		"--jobs=100",
		// Run the test locally. We want benchmarks to run on bare-metal machines that we control,
		// rather than on an RBE worker. Compilation can still happen on RBE.
		//
		// The --spawn_strategy flag[1] controls where and how commands are executed. For example:
		//
		//  - Flag --spawn_strategy=sandboxed executes commands inside a sandbox on the local system.
		//    This is the default Bazel behavior on systems that support sandboxing.
		//
		//  - Flag --spawn_strategy=local executes commands as regular, local subprocesses without any
		//    sandboxing.
		//
		//  - Flag --spawn_strategy=remote executes commands remotely, provided a remote executor has
		//    been configured. We use this strategy when running Bazel with --config=remote. See the
		//    //.bazelrc file[2].
		//
		// The --strategy flag[3] allows us to override --spawn_strategy on a per-mnemonic basis. In
		// our case, we set --strategy=TestRunner=local to force test actions to run as a local
		// subprocess. In combination with --config=remote (or any configuration that implies it, such
		// as --config=linux_rbe) this has the effect of running test actions locally, while build
		// actions and any other actions run on RBE.
		//
		// The "TestRunner" mnemonic for test actions is determined here[4].
		//
		// [1] https://bazel.build/docs/user-manual#spawn-strategy
		// [2] https://skia.googlesource.com/skia/+/e5c37860c792de6bba0c9465c3f5280cb13dbbb9/.bazelrc#128
		// [3] https://bazel.build/docs/user-manual#strategy
		// [4] https://github.com/bazelbuild/bazel/blob/f79ca0275e14d7c8fb478bd910ad7fb127440fd8/src/main/java/com/google/devtools/build/lib/analysis/test/TestRunnerAction.java#L107
		"--strategy=TestRunner=local",
	}
	for _, testArg := range testArgs {
		args = append(args, "--test_arg="+testArg)
	}

	return td.Do(ctx, td.Props(fmt.Sprintf("Test %s with config %s", label, config)), func(ctx context.Context) error {
		runCmd := &sk_exec.Command{
			Name:       "bazelisk",
			Args:       args,
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
