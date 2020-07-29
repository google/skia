// Copyright 2020 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package main

import (
	"context"
	"flag"
	"fmt"
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

		local = flag.Bool("local", true, "True if running locally (as opposed to on the bots)")
		// workdir = flag.String("workdir", ".", "Working directory")

		// checkoutFlags = checkout.SetupFlags(nil)

		gnToBpPath = flag.String("gn_to_bp_path", "", "Path to location of gn_to_bp.py")
	)
	ctx := td.StartRun(projectId, taskId, taskName, output, local)
	defer td.EndRun(ctx)

	// rs, err := checkout.GetRepoState(checkoutFlags)
	// if err != nil {
	// 	td.Fatal(ctx, skerr.Wrap(err))
	// }
	if *gnToBpPath == "" {
		td.Fatalf(ctx, "--gn_to_bp_path must be specified")
	}
	gnToBpAbsPath, err := filepath.Abs(*gnToBpPath)
	if err != nil {
		td.Fatal(ctx, skerr.Wrap(err))
	}

	// wd, err := os_steps.Abs(ctx, *workdir)
	// if err != nil {
	// 	td.Fatal(ctx, err)
	// }

	// // Check out the code.
	// co, err := checkout.EnsureGitCheckout(ctx, path.Join(wd, "repo"), rs)
	// if err != nil {
	// 	td.Fatal(ctx, err)
	// }

	// Run gn_to_bp.py after syncing.
	err = td.Do(ctx, td.Props("Run ./bin/sync"), func(ctx context.Context) error {
		if _, err := exec.RunCwd(ctx, "", "./bin/sync"); err != nil {
			return fmt.Errorf("Error in running ./bin/sync: %s", err)
		}
		return nil
	})
	if err != nil {
		td.Fatal(ctx, skerr.Wrap(err))
	}
	err = td.Do(ctx, td.Props(fmt.Sprintf("Run %s", gnToBpAbsPath)), func(ctx context.Context) error {
		if _, err := exec.RunCwd(ctx, "", fmt.Sprintf("vpython %s", gnToBpAbsPath)); err != nil {
			return fmt.Errorf("Error in running %s: %s", gnToBpAbsPath, err)
		}
		return nil
	})
	if err != nil {
		td.Fatal(ctx, skerr.Wrap(err))
	}

	return
}
