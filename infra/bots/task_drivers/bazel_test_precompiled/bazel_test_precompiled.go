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
	"flag"
	"path/filepath"

	"go.skia.org/infra/go/common"
	sk_exec "go.skia.org/infra/go/exec"
	"go.skia.org/infra/task_driver/go/lib/os_steps"
	"go.skia.org/infra/task_driver/go/td"
)

var (
	// Required properties for this task.
	projectId      = flag.String("project_id", "", "ID of the Google Cloud project.")
	taskId         = flag.String("task_id", "", "ID of this task.")
	taskName       = flag.String("task_name", "", "Name of the task.")
	workdir        = flag.String("workdir", ".", "Working directory, the root directory of a full Skia checkout")
	command        = flag.String("command", "", "Path to the command to run (e.g. a shell script in a directory with CAS inputs).")
	commandWorkDir = flag.String("command_workdir", "", "Path to the working directory of the command to run (e.g. a directory with CAS inputs).")

	// Optional flags.
	commandArgs = common.NewMultiStringFlag("command_args", nil, "Any arguments to pass to the command to run. Optional.")
	local       = flag.Bool("local", false, "True if running locally (as opposed to on the CI/CQ)")
	output      = flag.String("o", "", "If provided, dump a JSON blob of step data to the given file. Prints to stdout if '-' is given.")
)

func main() {
	// StartRun calls flag.Parse()
	ctx := td.StartRun(projectId, taskId, taskName, output, local)
	defer td.EndRun(ctx)

	wd, err := os_steps.Abs(ctx, *workdir)
	if err != nil {
		td.Fatal(ctx, err)
	}

	commandAbs := filepath.Join(wd, *command)
	commandWorkDirAbs := filepath.Join(wd, *commandWorkDir)

	runCmd := &sk_exec.Command{
		Name:       commandAbs,
		Args:       *commandArgs,
		InheritEnv: true, // Makes sure any CIPD-downloaded tools are available on $PATH.
		Dir:        commandWorkDirAbs,
		LogStdout:  true,
		LogStderr:  true,
	}
	_, err = sk_exec.RunCommand(ctx, runCmd)
	if err != nil {
		td.Fatal(ctx, err)
	}
}
