// Copyright 2020 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package main

import (
	"flag"
	"fmt"
	"os"
	"path/filepath"

	"go.skia.org/infra/go/exec"
	"go.skia.org/infra/go/skerr"
	"go.skia.org/infra/task_driver/go/td"
)

func main() {
	var (
		projectId = flag.String("project_id", "", "ID of the Google Cloud project.")
		taskId    = flag.String("task_id", "", "ID of this task.")
		taskName  = flag.String("task_name", "", "Name of the task.")
		output    = flag.String("o", "", "If provided, dump a JSON blob of step data to the given file. Prints to stdout if '-' is given.")
		local     = flag.Bool("local", true, "True if running locally (as opposed to on the bots)")

		skiaCheckoutRoot = flag.String("skia_checkout_root", "./skia", "Path to Skia's checkout.")
	)
	ctx := td.StartRun(projectId, taskId, taskName, output, local)
	defer td.EndRun(ctx)

	skiaCheckoutAbsPath := td.MustGetAbsolutePathOfFlag(ctx, *skiaCheckoutRoot, "skia_checkout_root")
	binPath := filepath.Join(skiaCheckoutAbsPath, "bin")

	// Fetch GN before running gn_to_bp.py
	if _, err := exec.RunCwd(ctx, skiaCheckoutAbsPath, filepath.Join(binPath, "fetch-gn")); err != nil {
		td.Fatal(ctx, skerr.Wrap(err))
	}

	// Run gn_to_bp.py
	gnEnv := []string{fmt.Sprintf("PATH=%s/:%s", binPath, os.Getenv("PATH"))}
	if _, gnToBpErr := exec.RunCommand(ctx, &exec.Command{
		Env:  gnEnv,
		Dir:  skiaCheckoutAbsPath,
		Name: "python",
		Args: []string{"-c", "from gn import gn_to_bp"},
	}); gnToBpErr != nil {
		td.Fatal(ctx, fmt.Errorf("Failed to run gn_to_bp: %s", gnToBpErr))
	}
	return
}
