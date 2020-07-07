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
		framework = flag.String("framework", "framework", "The framework we want to canary.")
	)
	ctx := td.StartRun(projectId, taskId, taskName, output, local)
	defer td.EndRun(ctx)

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

	req := manual.ManualRollRequest{
		Requester:  "HOW DO I DO THIS?",
		RollerName: "FROM FLAGS",
		Status:     manual.STATUS_PENDING,
		Timestamp:  firestore.FixTimestamp(time.Now()),
	}
	if err := manualRollDB.Put(&req); err != nil {
		td.Fatal(ctx, err)
	}
}
