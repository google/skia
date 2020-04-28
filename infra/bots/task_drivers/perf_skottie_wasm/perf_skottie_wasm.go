// Copyright 2020 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package main

import (
	"flag"

	"go.skia.org/infra/go/exec"
	"go.skia.org/infra/task_driver/go/td"
)

func main() {
	var (
		// Required properties for this task.
		projectId = flag.String("project_id", "", "ID of the Google Cloud project.")
		taskId    = flag.String("task_id", "", "ID of this task.")
		taskName  = flag.String("task_name", "", "Name of the task.")
		workdir   = flag.String("workdir", ".", "Working directory")

		// Optional flags.
		local  = flag.Bool("local", false, "True if running locally (as opposed to on the bots)")
		output = flag.String("o", "", "If provided, dump a JSON blob of step data to the given file. Prints to stdout if '-' is given.")
	)

	// Setup.
	ctx := td.StartRun(projectId, taskId, taskName, output, local)
	defer td.EndRun(ctx)

	// Run the infra tests.
	// TODO(kjlubick) finish this implementation.
	if _, err := exec.RunCwd(ctx, *workdir, "ls", "-ahl"); err != nil {
		td.Fatal(ctx, err)
	}
}
