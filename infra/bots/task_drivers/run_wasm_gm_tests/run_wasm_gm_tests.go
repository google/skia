// Copyright 2020 The Chromium Authors. All rights reserved.
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
		// Required properties for this task.
		builtPath   = flag.String("built_path", "", "The directory where the built wasm/js code will be.")
		goldCtlPath = flag.String("gold_ctl_path", "", "Path to the goldctl binary")
		projectID   = flag.String("project_id", "", "ID of the Google Cloud project.")
		skiaPath    = flag.String("skia_path", "", "Path to skia repo root.")
		taskID      = flag.String("task_id", "", "task id this data was generated on")
		taskName    = flag.String("task_name", "", "Name of the task.")
		workPath    = flag.String("work_path", "", "The directory to use to store temporary files (e.g. docker build)")

		// Debugging flags.
		local       = flag.Bool("local", false, "True if running locally (as opposed to on the bots)")
		outputSteps = flag.String("o", "", "If provided, dump a JSON blob of step data to the given file. Prints to stdout if '-' is given.")
	)

	// Setup.
	ctx := td.StartRun(projectID, taskID, taskName, outputSteps, local)
	defer td.EndRun(ctx)

	builtAbsPath := getAbsoluteOfRequiredFlag(ctx, *builtPath, "built_path")
	goldCtlAbsPath := getAbsoluteOfRequiredFlag(ctx, *goldCtlPath, "gold_ctl_path")
	skiaAbsPath := getAbsoluteOfRequiredFlag(ctx, *skiaPath, "skia_path")
	workAbsPath := getAbsoluteOfRequiredFlag(ctx, *workPath, "work_path")

	if err := os_steps.MkdirAll(ctx, workAbsPath); err != nil {
		td.Fatal(ctx, err)
	}

	// initialize goldctl

	// Start webserver to receive gold images and data

	// Eventually read in https://gold.skia.org/json/v1/hashes See skbug.com/10824

	// Run puppeteer tests. HTML will post to the server we indicate. They will only finish
	// when the last post succeeds, so we are sure the server has handled everything.

	// stop webserver.

	// call goldctl finalize to upload stuff.

}
