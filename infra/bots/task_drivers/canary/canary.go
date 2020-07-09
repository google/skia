// Copyright 2020 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package main

import (
	"flag"
	"fmt"
	"time"

	"cloud.google.com/go/datastore"
	"cloud.google.com/go/pubsub"

	"go.skia.org/infra/autoroll/go/manual"
	"go.skia.org/infra/go/auth"
	"go.skia.org/infra/go/firestore"
	"go.skia.org/infra/go/skerr"
	"go.skia.org/infra/go/sklog"
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

	// Create token source with scope for datastore.
	// ts, err := auth_steps.Init(ctx, *local, auth.SCOPE_USERINFO_EMAIL, datastore.ScopeDatastore, "https://www.googleapis.com/auth/cloud-platform", "https://www.googleapis.com/auth/firebase")
	// ts, err := auth_steps.Init(ctx, *local, auth.SCOPE_USERINFO_EMAIL, datastore.ScopeDatastore, auth.SCOPE_FULL_CONTROL)
	ts, err := auth_steps.Init(ctx, *local, auth.SCOPE_USERINFO_EMAIL, auth.SCOPE_FULL_CONTROL, datastore.ScopeDatastore, auth.SCOPE_GERRIT, pubsub.ScopePubSub)
	if err != nil {
		td.Fatal(ctx, skerr.Wrap(err))
	}

	manualRollDB, err := manual.NewDBWithParams(ctx, firestore.FIRESTORE_PROJECT, "production", ts)
	if err != nil {
		td.Fatal(ctx, skerr.Wrap(err))
	}
	fmt.Println(manualRollDB)

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
	// if err := manualRollDB.Put(&req); err != nil {
	// 	td.Fatal(ctx, skerr.Wrap(err))
	// }
	req.Id = "rkpfHcYSLNpFGGDg5PCJ"

	for {
		roll, err := manualRollDB.Get(ctx, req.Id)
		if err != nil {
			// fmt.Println(err)
			td.Fatal(ctx, skerr.Wrap(err))
			// td.FailStep(ctx, err)
		}
		cl := roll.Url
		// TODO(rmistry): Figure out how to display the CL number in task driver.

		if roll.Status == manual.STATUS_COMPLETE {
			if roll.Result == manual.RESULT_SUCCESS {
				return
			} else if roll.Result == manual.RESULT_FAILURE {
				td.FailStep(ctx, fmt.Errorf("%s failed", cl))
			} else if roll.Result == manual.RESULT_UNKNOWN {
				td.FailStep(ctx, fmt.Errorf("%s completed with an unknown result", cl))
			}
		}
		if cl == "" {
			sklog.Infof("Canary roll has status %s", roll.Status)
		} else {
			sklog.Infof("Canary roll [ %s ] has status %s", roll.Url, roll.Status)
		}
		time.Sleep(10 * time.Second)
	}
}
