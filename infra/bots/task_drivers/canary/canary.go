// Copyright 2020 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package main

import (
	"flag"
	"fmt"
	"time"

	"cloud.google.com/go/datastore"

	"go.skia.org/infra/autoroll/go/manual"
	"go.skia.org/infra/go/auth"
	"go.skia.org/infra/go/firestore"
	"go.skia.org/infra/task_driver/go/lib/auth_steps"
	"go.skia.org/infra/task_driver/go/lib/checkout"
	"go.skia.org/infra/task_driver/go/td"
)

type work struct {
	Sources []string
	Flags   []string
}

func main() {
	var (
		projectId = flag.String("project_id", "", "ID of the Google Cloud project.")
		taskId    = flag.String("task_id", "", "ID of this task.")
		taskName  = flag.String("task_name", "", "Name of the task.")
		local     = flag.Bool("local", true, "True if running locally (as opposed to on the bots)")
		output    = flag.String("o", "", "If provided, dump a JSON blob of step data to the given file. Prints to stdout if '-' is given.")

		checkoutFlags = checkout.SetupFlags(nil)

		// TODO(rmistry): This should be roller name instead.
		rollerName = flag.String("roller_name", "", "The framework we want to canary.")
	)
	ctx := td.StartRun(projectId, taskId, taskName, output, local)
	defer td.EndRun(ctx)
	if *rollerName == "" {
		td.Fatalf(ctx, "--roller_name must be specified")
	}

	rs, err := checkout.GetRepoState(checkoutFlags)
	if err != nil {
		td.Fatal(ctx, err)
	}
	if rs.Issue == "" || rs.Patchset == "" {
		td.Fatalf(ctx, "This task driver should be run only as a try bot")
	}

	// Create token source with scope for datastore.
	ts, err := auth_steps.Init(ctx, *local, auth.SCOPE_USERINFO_EMAIL, datastore.ScopeDatastore)
	if err != nil {
		td.Fatal(ctx, err)
	}

	manualRollDB, err := manual.NewDBWithParams(ctx, firestore.FIRESTORE_PROJECT, "production", ts)
	if err != nil {
		td.Fatal(ctx, err)
	}
	fmt.Println(manualRollDB)

	// Might have to use buildbucket to get the requester for this tryjob? asked Eric
	requester := "rmistry@google.com"

	req := manual.ManualRollRequest{
		Requester:  requester,
		RollerName: *rollerName,
		Status:     manual.STATUS_PENDING,
		Timestamp:  firestore.FixTimestamp(time.Now()),
		Revision:   rs.GetPatchRef(),

		DryRun:            true,
		NoEmail:           true,
		NoResolveRevision: true,
	}
	if err := manualRollDB.Put(&req); err != nil {
		td.Fatal(ctx, err)
	}
	fmt.Printf("%+v", req)
	fmt.Println("THIS IS THE ID::")
	fmt.Printf(req.Id)
	// req.Id = "pvAXXm9JmOOwckdaulYh"
	// ID = pvAXXm9JmOOwckdaulYh

	roll, err := manualRollDB.Get(ctx, req.Id)
	if err != nil {
		td.Fatal(ctx, err)
	}
	fmt.Println("THE ROLL IS BELOW!")
	fmt.Printf("%+v", roll)

	// Need to poll here. Similar to tree_status ?
}
