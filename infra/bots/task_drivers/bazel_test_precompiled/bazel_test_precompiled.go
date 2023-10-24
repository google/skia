// Copyright 2023 Google LLC
//
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// This task driver runs an arbitary command specified via command-line arguments. Its purpose is
// to run prebuilt Bazel tests on devices where we cannot (or don't want to) run Bazel, such as
// Raspberry Pis.

package main

import (
	"context"
	"flag"
	"fmt"
	"path/filepath"

	sk_common "go.skia.org/infra/go/common"
	sk_exec "go.skia.org/infra/go/exec"
	"go.skia.org/infra/go/skerr"
	"go.skia.org/infra/task_driver/go/lib/os_steps"
	"go.skia.org/infra/task_driver/go/td"
	"go.skia.org/skia/infra/bots/task_drivers/common"
)

var (
	// Required properties for this task.
	projectId      = flag.String("project_id", "", "ID of the Google Cloud project.")
	taskId         = flag.String("task_id", "", "ID of this task.")
	taskName       = flag.String("task_name", "", "Name of the task.")
	workdir        = flag.String("workdir", ".", "Working directory, the root directory of a full Skia checkout")
	command        = flag.String("command", "", "Path to the command to run (e.g. a shell script in a directory with CAS inputs).")
	commandWorkDir = flag.String("command_workdir", "", "Path to the working directory of the command to run (e.g. a directory with CAS inputs).")
	kind           = flag.String("kind", "", `Test kind ("benchmark", "gm" or "unit"). Required.`)

	// Flags required for GM tests.
	label            = flag.String("bazel_label", "", "The label of the Bazel target to test (only used for GM tests)")
	goldctlPath      = flag.String("goldctl_path", "", "The path to the golctl binary on disk.")
	gitCommit        = flag.String("git_commit", "", "The git hash to which the data should be associated. This will be used when changelist_id and patchset_order are not set to report data to Gold that belongs on the primary branch.")
	changelistID     = flag.String("changelist_id", "", "Should be non-empty only when run on the CQ.")
	patchsetOrderStr = flag.String("patchset_order", "", "Should be non-zero only when run on the CQ.")
	tryjobID         = flag.String("tryjob_id", "", "Should be non-zero only when run on the CQ.")

	// Optional flags.
	commandArgs = sk_common.NewMultiStringFlag("command_args", nil, "Any arguments to pass to the command to run. Optional.")
	local       = flag.Bool("local", false, "True if running locally (as opposed to on the CI/CQ)")
	output      = flag.String("o", "", "If provided, dump a JSON blob of step data to the given file. Prints to stdout if '-' is given.")
)

func main() {
	// StartRun calls flag.Parse()
	ctx := td.StartRun(projectId, taskId, taskName, output, local)
	defer td.EndRun(ctx)

	if *kind == "" {
		td.Fatal(ctx, skerr.Fmt("Flag --kind is required."))
	}

	var testKind testKind
	if *kind == "benchmark" {
		testKind = benchmarkTest
	} else if *kind == "gm" {
		testKind = gmTest
	} else if *kind == "unit" {
		testKind = unitTest
	} else {
		td.Fatal(ctx, skerr.Fmt("Unknown flag --kind value: %q", *kind))
	}

	wd, err := os_steps.Abs(ctx, *workdir)
	if err != nil {
		td.Fatal(ctx, err)
	}

	tdArgs := taskDriverArgs{
		UploadToGoldArgs: common.UploadToGoldArgs{
			BazelLabel:    *label,
			GoldctlPath:   filepath.Join(wd, *goldctlPath),
			GitCommit:     *gitCommit,
			ChangelistID:  *changelistID,
			PatchsetOrder: *patchsetOrderStr,
			TryjobID:      *tryjobID,
		},

		testKind:       testKind,
		commandPath:    filepath.Join(wd, *command),
		commandArgs:    *commandArgs,
		commandWorkDir: filepath.Join(wd, *commandWorkDir),
	}
	if testKind == gmTest {
		tdArgs.undeclaredOutputsDir, err = os_steps.TempDir(ctx, "", "test-undeclared-outputs-dir-*")
		if err != nil {
			td.Fatal(ctx, err)
		}
	}

	if err := run(ctx, tdArgs); err != nil {
		td.Fatal(ctx, err)
	}
}

type testKind int

const (
	benchmarkTest testKind = iota
	gmTest
	unitTest
)

// taskDriverArgs gathers the inputs to this task driver, and decouples the task driver's
// entry-point function from the command line flags, which facilitates writing unit tests.
type taskDriverArgs struct {
	common.UploadToGoldArgs

	commandPath          string
	commandArgs          []string
	commandWorkDir       string
	testKind             testKind
	undeclaredOutputsDir string
}

// run is the entrypoint of this task driver.
func run(ctx context.Context, tdArgs taskDriverArgs) error {
	// GM tests require an output directory in which to store any PNGs produced.
	var env []string
	if tdArgs.testKind == gmTest {
		env = append(env, fmt.Sprintf("TEST_UNDECLARED_OUTPUTS_DIR=%s", tdArgs.undeclaredOutputsDir))
	}

	runCmd := &sk_exec.Command{
		Name:       tdArgs.commandPath,
		Args:       tdArgs.commandArgs,
		Env:        env,
		InheritEnv: true, // Makes sure any CIPD-downloaded tools are available on $PATH.
		Dir:        tdArgs.commandWorkDir,
		LogStdout:  true,
		LogStderr:  true,
	}
	_, err := sk_exec.RunCommand(ctx, runCmd)
	if err != nil {
		return skerr.Wrap(err)
	}

	if tdArgs.testKind == gmTest {
		return skerr.Wrap(common.UploadToGold(ctx, tdArgs.UploadToGoldArgs, tdArgs.undeclaredOutputsDir))
	}

	return nil
}
