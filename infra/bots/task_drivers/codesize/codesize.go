// Copyright 2022 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This task driver takes a binary (e.g. "dm") built by a Build-* task (e.g.
// "Build-Debian10-Clang-x86_64-Release"), runs Bloaty against the binary, and uploads the resulting
// code size statistics to the GCS bucket belonging to the https://codesize.skia.org service.
package main

import (
	"context"
	"encoding/json"
	"flag"
	"fmt"
	"os"
	"strconv"
	"time"

	"cloud.google.com/go/storage"
	"google.golang.org/api/option"

	"go.skia.org/infra/go/auth"
	"go.skia.org/infra/go/exec"
	"go.skia.org/infra/go/gcs"
	"go.skia.org/infra/go/gcs/gcsclient"
	"go.skia.org/infra/go/gerrit"
	"go.skia.org/infra/go/gitiles"
	"go.skia.org/infra/go/now"
	"go.skia.org/infra/go/skerr"
	"go.skia.org/infra/task_driver/go/lib/auth_steps"
	"go.skia.org/infra/task_driver/go/lib/checkout"
	"go.skia.org/infra/task_driver/go/td"
	"go.skia.org/infra/task_scheduler/go/types"
)

const gcsBucketName = "skia-codesize"

// BloatyOutputMetadata contains the Bloaty version and command-line arguments used, and metadata
// about the task where Bloaty was invoked. This struct is serialized into a JSON file that is
// uploaded to GCS alongside the Bloaty output file.
//
// TODO(lovisolo): Move this struct to the buildbot repository.
type BloatyOutputMetadata struct {
	Version   int    `json:"version"` // Schema version of this file, starting at 1.
	Timestamp string `json:"timestamp"`

	SwarmingTaskID string `json:"swarming_task_id"`
	SwarmingServer string `json:"swarming_server"`

	TaskID          string `json:"task_id"`
	TaskName        string `json:"task_name"`
	CompileTaskName string `json:"compile_task_name"`
	BinaryName      string `json:"binary_name"`

	BloatyCipdVersion string   `json:"bloaty_cipd_version"`
	BloatyArgs        []string `json:"bloaty_args"`

	PatchIssue  string `json:"patch_issue"`
	PatchServer string `json:"patch_server"`
	PatchSet    string `json:"patch_set"`
	Repo        string `json:"repo"`
	Revision    string `json:"revision"`

	CommitTimestamp string `json:"commit_timestamp"`
	Author          string `json:"author"`
	Subject         string `json:"subject"`
}

func main() {
	var (
		projectID         = flag.String("project_id", "", "ID of the Google Cloud project.")
		taskID            = flag.String("task_id", "", "ID of this task.")
		taskName          = flag.String("task_name", "", "Name of the task.")
		compileTaskName   = flag.String("compile_task_name", "", "Name of the compile task that produced the binary to analyze.")
		binaryName        = flag.String("binary_name", "", "Name of the binary to analyze (e.g. \"dm\").")
		bloatyCIPDVersion = flag.String("bloaty_cipd_version", "", "Version of the \"bloaty\" CIPD package used.")
		output            = flag.String("o", "", "If provided, dump a JSON blob of step data to the given file. Prints to stdout if '-' is given.")
		local             = flag.Bool("local", true, "True if running locally (as opposed to on the bots).")

		checkoutFlags = checkout.SetupFlags(nil)
	)
	ctx := td.StartRun(projectID, taskID, taskName, output, local)
	defer td.EndRun(ctx)

	// The repository state contains the commit hash and patch/patchset if available.
	repoState, err := checkout.GetRepoState(checkoutFlags)
	if err != nil {
		td.Fatal(ctx, skerr.Wrap(err))
	}

	// Make an HTTP client with the required permissions to hit GCS, Gerrit and Gitiles.
	httpClient, err := auth_steps.InitHttpClient(ctx, *local, auth.ScopeReadWrite, gerrit.AuthScope, auth.ScopeUserinfoEmail)
	if err != nil {
		td.Fatal(ctx, skerr.Wrap(err))
	}

	// Make a GCS client with the required permissions to upload to the codesize.skia.org GCS bucket.
	store, err := storage.NewClient(ctx, option.WithHTTPClient(httpClient))
	if err != nil {
		td.Fatal(ctx, skerr.Wrap(err))
	}
	gcsClient := gcsclient.New(store, gcsBucketName)

	// Make a Gerrit client.
	gerrit, err := gerrit.NewGerrit(repoState.Server, httpClient)
	if err != nil {
		td.Fatal(ctx, skerr.Wrap(err))
	}

	// Make a Gitiles client.
	gitilesRepo := gitiles.NewRepo(repoState.Repo, httpClient)

	args := runStepsArgs{
		repoState:         repoState,
		gerrit:            gerrit,
		gitilesRepo:       gitilesRepo,
		gcsClient:         gcsClient,
		swarmingTaskID:    os.Getenv("SWARMING_TASK_ID"),
		swarmingServer:    os.Getenv("SWARMING_SERVER"),
		taskID:            *taskID,
		taskName:          *taskName,
		compileTaskName:   *compileTaskName,
		binaryName:        *binaryName,
		bloatyCIPDVersion: *bloatyCIPDVersion,
	}

	if err := runSteps(ctx, args); err != nil {
		td.Fatal(ctx, skerr.Wrap(err))
	}
}

// runStepsArgs contains the input arguments to the runSteps function.
type runStepsArgs struct {
	repoState         types.RepoState
	gerrit            *gerrit.Gerrit
	gitilesRepo       gitiles.GitilesRepo
	gcsClient         gcs.GCSClient
	swarmingTaskID    string
	swarmingServer    string
	taskID            string
	taskName          string
	compileTaskName   string
	binaryName        string
	bloatyCIPDVersion string
}

// runSteps runs the main steps of this task driver.
func runSteps(ctx context.Context, args runStepsArgs) error {
	var (
		author          string
		subject         string
		commitTimestamp string
	)

	// Read the CL subject, author and timestamp. We talk to Gerrit when running as a tryjob, or to
	// Gitiles when running as a post-submit task.
	if args.repoState.IsTryJob() {
		issue, err := strconv.ParseInt(args.repoState.Issue, 10, 64)
		if err != nil {
			return skerr.Wrap(err)
		}
		patchset, err := strconv.ParseInt(args.repoState.Patchset, 10, 64)
		if err != nil {
			return skerr.Wrap(err)
		}
		changeInfo, err := args.gerrit.GetIssueProperties(ctx, issue)
		if err != nil {
			return skerr.Wrap(err)
		}
		// This matches the format of the author field returned by Gitiles.
		author = fmt.Sprintf("%s (%s)", changeInfo.Owner.Name, changeInfo.Owner.Email)
		subject = changeInfo.Subject
		for _, revision := range changeInfo.Revisions {
			if revision.Number == patchset {
				commitTimestamp = revision.CreatedString
				break
			}
		}
	} else {
		longCommit, err := args.gitilesRepo.Details(ctx, args.repoState.Revision)
		if err != nil {
			return skerr.Wrap(err)
		}
		author = longCommit.Author
		subject = longCommit.Subject
		commitTimestamp = longCommit.Timestamp.Format(time.RFC3339)
	}

	// Run Bloaty and capture its output.
	bloatyOutput, bloatyArgs, err := runBloaty(ctx, args.binaryName)
	if err != nil {
		return skerr.Wrap(err)
	}

	// Build metadata structure.
	metadata := &BloatyOutputMetadata{
		Version:           1,
		Timestamp:         now.Now(ctx).Format(time.RFC3339),
		SwarmingTaskID:    args.swarmingTaskID,
		SwarmingServer:    args.swarmingServer,
		TaskID:            args.taskID,
		TaskName:          args.taskName,
		CompileTaskName:   args.compileTaskName,
		BinaryName:        args.binaryName,
		BloatyCipdVersion: args.bloatyCIPDVersion,
		BloatyArgs:        bloatyArgs,
		PatchIssue:        args.repoState.Issue,
		PatchServer:       args.repoState.Server,
		PatchSet:          args.repoState.Patchset,
		Repo:              args.repoState.Repo,
		Revision:          args.repoState.Revision,
		CommitTimestamp:   commitTimestamp,
		Author:            author,
		Subject:           subject,
	}

	gcsDir := computeTargetGCSDirectory(ctx, args.repoState, args.taskID, args.compileTaskName)

	// Upload Bloaty output TSV file to GCS.
	if err = uploadFileToGCS(ctx, args.gcsClient, fmt.Sprintf("%s/%s.tsv", gcsDir, args.binaryName), []byte(bloatyOutput)); err != nil {
		return skerr.Wrap(err)
	}

	// Upload pretty-printed JSON metadata file to GCS.
	jsonMetadata, err := json.MarshalIndent(metadata, "", "  ")
	if err != nil {
		return skerr.Wrap(err)
	}
	if err = uploadFileToGCS(ctx, args.gcsClient, fmt.Sprintf("%s/%s.json", gcsDir, args.binaryName), jsonMetadata); err != nil {
		return skerr.Wrap(err)
	}

	return nil
}

// runBloaty runs Bloaty against the given binary and returns the Bloaty output in TSV format and
// the Bloaty command-line arguments used.
func runBloaty(ctx context.Context, binaryName string) (string, []string, error) {
	err := td.Do(ctx, td.Props("List files under $PWD/build (for debugging purposes)"), func(ctx context.Context) error {
		runCmd := &exec.Command{
			Name:       "ls",
			Args:       []string{"build"},
			InheritEnv: true,
			LogStdout:  true,
			LogStderr:  true,
		}
		_, err := exec.RunCommand(ctx, runCmd)
		return err
	})
	if err != nil {
		return "", []string{}, skerr.Wrap(err)
	}

	runCmd := &exec.Command{
		Name: "bloaty/bloaty",
		Args: []string{
			"build/" + binaryName,
			"-d",
			"compileunits,symbols",
			"-n",
			"0",
			"--tsv",
		},
		InheritEnv: true,
		LogStdout:  true,
		LogStderr:  true,
	}

	var bloatyOutput string

	if err := td.Do(ctx, td.Props(fmt.Sprintf("Run Bloaty against binary %q", binaryName)), func(ctx context.Context) error {
		bloatyOutput, err = exec.RunCommand(ctx, runCmd)
		return err
	}); err != nil {
		return "", []string{}, skerr.Wrap(err)
	}

	return bloatyOutput, runCmd.Args, nil
}

// computeTargetGCSDirectory computs the target GCS directory where to upload the Bloaty output file
// and JSON metadata file.
func computeTargetGCSDirectory(ctx context.Context, repoState types.RepoState, taskID, compileTaskName string) string {
	yearMonthDate := now.Now(ctx).Format("2006/01/02") // YYYY/MM/DD.
	if repoState.IsTryJob() {
		// Example: 2022/01/31/tryjob/12345/3/CkPp9ElAaEXyYWNHpXHU/Build-Debian10-Clang-x86_64-Release
		return fmt.Sprintf("%s/tryjob/%s/%s/%s/%s", yearMonthDate, repoState.Patch.Issue, repoState.Patch.Patchset, taskID, compileTaskName)
	} else {
		// Example: 2022/01/31/033ccea12c0949d0f712471bfcb4ed6daf69aaff/Build-Debian10-Clang-x86_64-Release
		return fmt.Sprintf("%s/%s/%s", yearMonthDate, repoState.Revision, compileTaskName)
	}
}

// uploadFileToGCS uploads a file to the codesize.skia.org GCS bucket.
func uploadFileToGCS(ctx context.Context, gcsClient gcs.GCSClient, path string, contents []byte) error {
	gcsURL := fmt.Sprintf("gs://%s/%s", gcsBucketName, path)
	return td.Do(ctx, td.Props(fmt.Sprintf("Upload %s", gcsURL)), func(ctx context.Context) error {
		if err := gcsClient.SetFileContents(ctx, path, gcs.FILE_WRITE_OPTS_TEXT, contents); err != nil {
			return fmt.Errorf("Could not write task to %s: %s", gcsURL, err)
		}
		return nil
	})
}
