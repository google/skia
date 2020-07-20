// Copyright 2020 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package main

import (
	"context"
	"errors"
	"flag"
	"fmt"
	"time"

	"cloud.google.com/go/datastore"

	"go.skia.org/infra/autoroll/go/manual"
	"go.skia.org/infra/go/auth"
	"go.skia.org/infra/go/firestore"
	"go.skia.org/infra/go/skerr"
	"go.skia.org/infra/task_driver/go/lib/auth_steps"
	"go.skia.org/infra/task_driver/go/lib/checkout"
	"go.skia.org/infra/task_driver/go/td"
)

func main() {
	var (
		projectId = flag.String("project_id", "", "ID of the Google Cloud project.")
		taskId    = flag.String("task_id", "", "ID of this task.")
		taskName  = flag.String("task_name", "", "Name of the task.")
		output    = flag.String("o", "", "If provided, dump a JSON blob of step data to the given file. Prints to stdout if '-' is given.")
		local     = flag.Bool("local", true, "True if running locally (as opposed to on the bots)")

		checkoutFlags = checkout.SetupFlags(nil)

		rollerName = flag.String("roller_name", "", "The roller we will use to create the canary with.")
	)
	ctx := td.StartRun(projectId, taskId, taskName, output, local)
	defer td.EndRun(ctx)
	if *rollerName == "" {
		td.Fatalf(ctx, "--roller_name must be specified")
	}

	rs, err := checkout.GetRepoState(checkoutFlags)
	if err != nil {
		td.Fatal(ctx, skerr.Wrap(err))
	}
	if rs.Issue == "" || rs.Patchset == "" {
		td.Fatalf(ctx, "This task driver should be run only as a try bot")
	}

	// Create token source with scope for datastore access.
	ts, err := auth_steps.Init(ctx, *local, auth.SCOPE_USERINFO_EMAIL, datastore.ScopeDatastore)
	if err != nil {
		td.Fatal(ctx, skerr.Wrap(err))
	}

	manualRollDB, err := manual.NewDBWithParams(ctx, firestore.FIRESTORE_PROJECT, "production", ts)
	if err != nil {
		td.Fatal(ctx, skerr.Wrap(err))
	}

	req := manual.ManualRollRequest{
		Requester:  *rollerName,
		RollerName: *rollerName,
		Status:     manual.STATUS_PENDING,
		Timestamp:  firestore.FixTimestamp(time.Now()),
		Revision:   rs.GetPatchRef(),

		DryRun:            true,
		NoEmail:           true,
		NoResolveRevision: true,
	}
	if err := td.Do(ctx, td.Props("Trigger canary roll").Infra(), func(ctx context.Context) error {
		return manualRollDB.Put(&req)
	}); err != nil {
		td.Fatal(ctx, skerr.Wrap(err))
	}

	// Add documentation link for canary rolls.
	td.StepText(ctx, "Canary roll doc", "https://goto.google.com/autoroller-canary-bots")

	if err := waitForCanaryRoll(ctx, manualRollDB, req.Id); err != nil {
		td.Fatal(ctx, skerr.Wrap(err))
	}
}

func waitForCanaryRoll(parentCtx context.Context, manualRollDB manual.DB, rollId string) error {
	ctx := td.StartStep(parentCtx, td.Props("Wait for canary roll"))
	defer td.EndStep(ctx)

	// For writing to the step's log stream.
	stdout := td.NewLogStream(ctx, "stdout", td.Info)
	// Lets add the roll link only once to step data.
	addedRollLinkStepData := false
	for {
		roll, err := manualRollDB.Get(ctx, rollId)
		if err != nil {
			return td.FailStep(ctx, fmt.Errorf("Could not find canary roll with ID: %s", rollId))
		}
		cl := roll.Url
		var rollStatus string
		if cl == "" {
			rollStatus = fmt.Sprintf("Canary roll has status %s", roll.Status)
		} else {
			if !addedRollLinkStepData {
				// Add the roll link to both the current step and it's parent.
				td.StepText(ctx, "Canary roll CL", cl)
				td.StepText(parentCtx, "Canary roll CL", cl)
				addedRollLinkStepData = true
			}
			rollStatus = fmt.Sprintf("Canary roll [ %s ] has status %s", roll.Url, roll.Status)
		}
		if _, err := stdout.Write([]byte(rollStatus)); err != nil {
			return td.FailStep(ctx, fmt.Errorf("Could not write to stdout: %s", err))
		}

		if roll.Status == manual.STATUS_COMPLETE {
			if roll.Result == manual.RESULT_SUCCESS {
				return nil
			} else if roll.Result == manual.RESULT_FAILURE {
				if cl == "" {
					return td.FailStep(ctx, errors.New("Canary roll could not be created. Ask the trooper to investigate (or directly ping rmistry@)."))
				}
				return td.FailStep(ctx, fmt.Errorf("Canary roll [ %s ] failed", cl))
			} else if roll.Result == manual.RESULT_UNKNOWN {
				return td.FailStep(ctx, fmt.Errorf("Canary roll [ %s ] completed with an unknown result", cl))
			}
		}
		time.Sleep(30 * time.Second)
	}
}
