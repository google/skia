// Copyright 2026 Google LLC
//
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package main

import (
	"flag"

	"go.skia.org/infra/task_driver/go/lib/os_steps"
	"go.skia.org/infra/task_driver/go/td"
)

func main() {

	var (
		// Required flags for this task regardless of where it's running.
		workPath = flag.String("work_path", "work", "The directory to use to store temporary files.")

		// Required flags for running on CQ
		projectID = flag.String("project_id", "", "ID of the Google Cloud project.")
		taskID    = flag.String("task_id", "", "ID of the task")
		taskName  = flag.String("task_name", "", "Name of the task.")

		// Debugging flags.
		local       = flag.Bool("local", false, "True if running locally (as opposed to on the bots)")
		outputSteps = flag.String("o", "", "If provided, dump a JSON blob of step data to the given file. Prints to stdout if '-' is given.")
	)

	ctx := td.StartRun(projectID, taskID, taskName, outputSteps, local)
	defer td.EndRun(ctx)

	if err := os_steps.MkdirAll(ctx, *workPath); err != nil {
		td.Fatal(ctx, err)
	}
}
