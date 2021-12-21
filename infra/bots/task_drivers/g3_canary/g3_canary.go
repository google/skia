// Copyright 2020 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package main

import (
	"context"
	"encoding/json"
	"errors"
	"flag"
	"fmt"
	"net/http"
	"strconv"
	"time"

	"cloud.google.com/go/storage"
	"google.golang.org/api/option"

	"go.skia.org/infra/go/auth"
	"go.skia.org/infra/go/gcs"
	"go.skia.org/infra/go/gcs/gcsclient"
	"go.skia.org/infra/go/httputils"
	"go.skia.org/infra/go/skerr"
	"go.skia.org/infra/go/sklog"
	"go.skia.org/infra/promk/go/pushgateway"
	"go.skia.org/infra/task_driver/go/lib/auth_steps"
	"go.skia.org/infra/task_driver/go/lib/checkout"
	"go.skia.org/infra/task_driver/go/td"
)

const (
	g3CanaryBucketName = "g3-compile-tasks"

	InfraFailureErrorMsg    = "Your run failed due to unknown infrastructure failures. Ask the Infra Gardener to investigate (or directly ping rmistry@)."
	MissingApprovalErrorMsg = "To run the G3 tryjob, changes must be either owned and authored by Googlers or approved (Code-Review+1) by Googlers."
	MergeConflictErrorMsg   = "G3 tryjob failed because the change is causing a merge conflict when applying it to the Skia hash in G3."

	PatchingInformation = "Tip: If needed, could try patching in the CL into a local G3 client with \"g4 patch\" and then hacking on it."

	// Metric constants for pushgateway.
	jobName                    = "g3-canary"
	metricName                 = "g3_canary_infra_failure"
	metricValue_NoInfraFailure = "0"
	metricValue_InfraFailure   = "1"
)

type CanaryStatusType string

const (
	ExceptionStatus       CanaryStatusType = "exception"
	MissingApprovalStatus CanaryStatusType = "missing_approval"
	MergeConflictStatus   CanaryStatusType = "merge_conflict"
	FailureStatus         CanaryStatusType = "failure"
	SuccessStatus         CanaryStatusType = "success"
)

type G3CanaryTask struct {
	Issue    int              `json:"issue"`
	Patchset int              `json:"patchset"`
	Status   CanaryStatusType `json:"status"`
	Result   string           `json:"result"`
	Error    string           `json:"error"`
	CL       int              `json:"cl"`
}

func main() {
	var (
		projectId = flag.String("project_id", "", "ID of the Google Cloud project.")
		taskId    = flag.String("task_id", "", "ID of this task.")
		taskName  = flag.String("task_name", "", "Name of the task.")
		output    = flag.String("o", "", "If provided, dump a JSON blob of step data to the given file. Prints to stdout if '-' is given.")
		local     = flag.Bool("local", true, "True if running locally (as opposed to on the bots)")

		checkoutFlags = checkout.SetupFlags(nil)
	)
	ctx := td.StartRun(projectId, taskId, taskName, output, local)
	defer td.EndRun(ctx)

	rs, err := checkout.GetRepoState(checkoutFlags)
	if err != nil {
		td.Fatal(ctx, skerr.Wrap(err))
	}
	if rs.Issue == "" || rs.Patchset == "" {
		td.Fatalf(ctx, "This task driver should be run only as a try bot")
	}

	// Create token source with scope for GCS access.
	ts, err := auth_steps.Init(ctx, *local, auth.ScopeUserinfoEmail, auth.ScopeFullControl)
	if err != nil {
		td.Fatal(ctx, skerr.Wrap(err))
	}
	client := httputils.DefaultClientConfig().WithTokenSource(ts).Client()
	store, err := storage.NewClient(ctx, option.WithHTTPClient(client))
	if err != nil {
		td.Fatalf(ctx, "Failed to create storage service client: %s", err)
	}
	gcsClient := gcsclient.New(store, g3CanaryBucketName)

	taskFileName := fmt.Sprintf("%s-%s.json", rs.Issue, rs.Patchset)
	taskStoragePath := fmt.Sprintf("gs://%s/%s", g3CanaryBucketName, taskFileName)

	err = td.Do(ctx, td.Props("Trigger new task if not already running"), func(ctx context.Context) error {
		if _, err := gcsClient.GetFileContents(ctx, taskFileName); err != nil {
			if err == storage.ErrObjectNotExist {
				// The task is not already running. Create a new file to trigger a new run.
				if err := triggerCanaryRoll(ctx, rs.Issue, rs.Patchset, taskFileName, taskStoragePath, gcsClient); err != nil {
					td.Fatal(ctx, fmt.Errorf("Could not trigger canary roll for %s/%s: %s", rs.Issue, rs.Patchset, err))
				}
			} else {
				return fmt.Errorf("Could not read %s: %s", taskStoragePath, err)
			}
		} else {
			fmt.Printf("G3 canary task for %s/%s already exists\n", rs.Issue, rs.Patchset)
		}
		return nil
	})
	if err != nil {
		td.Fatal(ctx, skerr.Wrap(err))
	}

	defer func() {
		// Cleanup the storage file after the task finishes.
		if err := gcsClient.DeleteFile(ctx, taskFileName); err != nil {
			sklog.Errorf("Could not delete %s: %s", taskStoragePath, err)
		}
	}()

	// Add documentation link for canary rolls.
	td.StepText(ctx, "Canary roll doc", "https://goto.google.com/autoroller-canary-bots")

	// Wait for the canary roll to finish.
	if err := waitForCanaryRoll(ctx, taskFileName, taskStoragePath, client, gcsClient); err != nil {
		td.Fatal(ctx, skerr.Wrap(err))
	}
}

func triggerCanaryRoll(ctx context.Context, issue, patchset, taskFileName, taskStoragePath string, gcsClient gcs.GCSClient) error {
	ctx = td.StartStep(ctx, td.Props("Trigger canary roll"))
	defer td.EndStep(ctx)

	i, err := strconv.Atoi(issue)
	if err != nil {
		return fmt.Errorf("Could not convert %s to int: %s", issue, err)
	}
	p, err := strconv.Atoi(patchset)
	if err != nil {
		return fmt.Errorf("Could not convert %s to int: %s", patchset, err)
	}
	newTask := G3CanaryTask{
		Issue:    i,
		Patchset: p,
	}
	taskJson, err := json.Marshal(newTask)
	if err != nil {
		return fmt.Errorf("Could not encode task to JSON: %s", err)
	}
	if err := gcsClient.SetFileContents(ctx, taskFileName, gcs.FILE_WRITE_OPTS_TEXT, taskJson); err != nil {
		return fmt.Errorf("Could not write task to %s: %s", taskStoragePath, err)
	}
	fmt.Printf("G3 canary task for %s/%s has been successfully added to %s\n", issue, patchset, taskStoragePath)
	return nil
}

func waitForCanaryRoll(parentCtx context.Context, taskFileName, taskStoragePath string, httpClient *http.Client, gcsClient gcs.GCSClient) error {
	ctx := td.StartStep(parentCtx, td.Props("Wait for canary roll"))
	defer td.EndStep(ctx)

	// For updating g3_canary_infra_failure metric after run completes.
	pg := pushgateway.New(httpClient, jobName, pushgateway.DefaultPushgatewayURL)

	// For writing to the step's log stream.
	stdout := td.NewLogStream(ctx, "stdout", td.SeverityInfo)
	// Lets add the roll link only once to step data.
	addedRollLinkStepData := false
	for {
		// Read task status from storage.
		contents, err := gcsClient.GetFileContents(ctx, taskFileName)
		if err != nil {
			return td.FailStep(ctx, fmt.Errorf("Could not read contents of %s: %s", taskStoragePath, err))
		}
		var task G3CanaryTask
		if err := json.Unmarshal(contents, &task); err != nil {
			return td.FailStep(ctx, fmt.Errorf("Could not unmarshal %s: %s", taskStoragePath, err))
		}

		var rollStatus string
		if task.CL == 0 {
			rollStatus = "Waiting for Canary roll to start"
		} else {
			clLink := fmt.Sprintf("http://cl/%d", task.CL)
			if !addedRollLinkStepData {
				// Add the roll link to both the current step and it's parent.
				td.StepText(ctx, "Canary roll CL", clLink)
				td.StepText(parentCtx, "Canary roll CL", clLink)
				addedRollLinkStepData = true
			}
			rollStatus = fmt.Sprintf("Canary roll [ %s ] has status %s", clLink, task.Result)
		}
		if _, err := stdout.Write([]byte(rollStatus)); err != nil {
			return td.FailStep(ctx, fmt.Errorf("Could not write to stdout: %s", err))
		}

		switch task.Status {
		case "":
			// Still waiting for the task.
			time.Sleep(30 * time.Second)
			continue
		case ExceptionStatus:
			if task.Error != "" {
				sklog.Errorf("Run failed with: %s", task.Error)
			}
			pg.Push(ctx, metricName, metricValue_InfraFailure)
			// Use a general purpose error message.
			return td.FailStep(ctx, errors.New(InfraFailureErrorMsg))
		case MissingApprovalStatus:
			pg.Push(ctx, metricName, metricValue_NoInfraFailure)
			return td.FailStep(ctx, errors.New(MissingApprovalErrorMsg))
		case MergeConflictStatus:
			pg.Push(ctx, metricName, metricValue_NoInfraFailure)
			return td.FailStep(ctx, errors.New(MergeConflictErrorMsg))
		case FailureStatus:
			pg.Push(ctx, metricName, metricValue_NoInfraFailure)
			return td.FailStep(ctx, fmt.Errorf("Run failed G3 TAP.\n%s", PatchingInformation))
		case SuccessStatus:
			// Run passed G3 TAP.
			pg.Push(ctx, metricName, metricValue_NoInfraFailure)
			return nil
		}
	}
}
