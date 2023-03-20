// Copyright 2022 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This task driver takes a binary (e.g. "dm") built by a Build-* task (e.g.
// "Build-Debian10-Clang-x86_64-Release"), runs Bloaty against the binary, and uploads the resulting
// code size statistics to the GCS bucket belonging to the https://codesize.skia.org service.
//
// When running as a tryjob, this task driver performs a size diff of said binary built at the
// tryjob's changelist/patchset vs. built at tip-of-tree. The binary built at tip-of-tree is
// produced by a *-NoPatch task (e.g. "Build-Debian10-Clang-x86_64-Release-NoPatch"), whereas the
// binary built at the tryjob's changelist/patchset is produced by a task of the same name except
// without the "-NoPatch" suffix (e.g. "Build-Debian10-Clang-x86_64-Release"). The size diff is
// calculated using Bloaty, see
// https://github.com/google/bloaty/blob/f01ea59bdda11708d74a3826c23d6e2db6c996f0/doc/using.md#size-diffs.
// The resulting diff is uploaded to the GCS bucket belonging to the https://codesize.skia.org
// service.
package main

import (
	"context"
	"encoding/json"
	"flag"
	"fmt"
	"os"
	"path/filepath"
	"strconv"
	"strings"
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
	"go.skia.org/infra/perf/go/ingest/format"
	"go.skia.org/infra/task_driver/go/lib/auth_steps"
	"go.skia.org/infra/task_driver/go/lib/checkout"
	"go.skia.org/infra/task_driver/go/lib/os_steps"
	"go.skia.org/infra/task_driver/go/td"
	"go.skia.org/infra/task_scheduler/go/types"
)

const (
	codesizeGCSBucketName = "skia-codesize"
	perfGCSBucketName     = "skia-perf"
	taskdriverURL         = "https://task-driver.skia.org/td/"
)

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
	// CompileTaskNameNoPatch should only be set for tryjobs.
	CompileTaskNameNoPatch string `json:"compile_task_name_no_patch,omitempty"`
	BinaryName             string `json:"binary_name"`

	BloatyCipdVersion string   `json:"bloaty_cipd_version"`
	BloatyArgs        []string `json:"bloaty_args"`
	// BloatyDiffArgs should only be set for tryjobs.
	BloatyDiffArgs []string `json:"bloaty_diff_args,omitempty"`

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
		projectID              = flag.String("project_id", "", "ID of the Google Cloud project.")
		taskID                 = flag.String("task_id", "", "ID of this task.")
		taskName               = flag.String("task_name", "", "Name of the task.")
		compileTaskName        = flag.String("compile_task_name", "", "Name of the compile task that produced the binary to analyze.")
		compileTaskNameNoPatch = flag.String("compile_task_name_no_patch", "", "Name of the *-NoPatch compile task that produced the binary to diff against (ignored when the task is not a tryjob).")
		binaryName             = flag.String("binary_name", "", "Name of the binary to analyze (e.g. \"dm\").")
		bloatyCIPDVersion      = flag.String("bloaty_cipd_version", "", "Version of the \"bloaty\" CIPD package used.")
		bloatyBinary           = flag.String("bloaty_binary", "", "Path to the bloaty binary.")
		stripBinary            = flag.String("strip_binary", "", "Path to the strip binary (part of binutils).")
		output                 = flag.String("o", "", "If provided, dump a JSON blob of step data to the given file. Prints to stdout if '-' is given.")
		local                  = flag.Bool("local", true, "True if running locally (as opposed to on the bots).")

		checkoutFlags = checkout.SetupFlags(nil)
	)
	ctx := td.StartRun(projectID, taskID, taskName, output, local)
	defer td.EndRun(ctx)

	if *bloatyBinary == "" || *stripBinary == "" {
		td.Fatal(ctx, skerr.Fmt("Must specify --bloaty_binary and --strip_binary"))
	}

	// The repository state contains the commit hash and patch/patchset if available.
	repoState, err := checkout.GetRepoState(checkoutFlags)
	if err != nil {
		td.Fatal(ctx, skerr.Wrap(err))
	}

	// Make an HTTP client with the required permissions to hit GCS, Gerrit and Gitiles.
	httpClient, _, err := auth_steps.InitHttpClient(ctx, *local, auth.ScopeReadWrite, gerrit.AuthScope, auth.ScopeUserinfoEmail)
	if err != nil {
		td.Fatal(ctx, skerr.Wrap(err))
	}

	// Make a GCS client with the required permissions to upload to the codesize.skia.org GCS bucket.
	store, err := storage.NewClient(ctx, option.WithHTTPClient(httpClient))
	if err != nil {
		td.Fatal(ctx, skerr.Wrap(err))
	}
	codesizeGCS := gcsclient.New(store, codesizeGCSBucketName)
	perfGCS := gcsclient.New(store, perfGCSBucketName)

	// Make a Gerrit client.
	gerritClient, err := gerrit.NewGerrit(repoState.Server, httpClient)
	if err != nil {
		td.Fatal(ctx, skerr.Wrap(err))
	}

	// Make a Gitiles client.
	gitilesRepo := gitiles.NewRepo(repoState.Repo, httpClient)

	args := runStepsArgs{
		repoState:              repoState,
		gerrit:                 gerritClient,
		gitilesRepo:            gitilesRepo,
		codesizeGCS:            codesizeGCS,
		perfGCS:                perfGCS,
		swarmingTaskID:         os.Getenv("SWARMING_TASK_ID"),
		swarmingServer:         os.Getenv("SWARMING_SERVER"),
		taskID:                 *taskID,
		taskName:               *taskName,
		compileTaskName:        *compileTaskName,
		compileTaskNameNoPatch: *compileTaskNameNoPatch,
		binaryName:             *binaryName,
		bloatyPath:             *bloatyBinary,
		bloatyCIPDVersion:      *bloatyCIPDVersion,
		stripPath:              *stripBinary,
	}

	if err := runSteps(ctx, args); err != nil {
		td.Fatal(ctx, skerr.Wrap(err))
	}
}

// runStepsArgs contains the input arguments to the runSteps function.
type runStepsArgs struct {
	repoState              types.RepoState
	gerrit                 *gerrit.Gerrit
	gitilesRepo            gitiles.GitilesRepo
	codesizeGCS            gcs.GCSClient
	perfGCS                gcs.GCSClient
	swarmingTaskID         string
	swarmingServer         string
	taskID                 string
	taskName               string
	compileTaskName        string
	compileTaskNameNoPatch string
	binaryName             string
	bloatyCIPDVersion      string
	bloatyPath             string
	stripPath              string
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
	bloatyOutput, bloatyArgs, err := runBloaty(ctx, args.stripPath, args.bloatyPath, args.binaryName)
	if err != nil {
		return skerr.Wrap(err)
	}

	// Build metadata structure.
	metadata := &BloatyOutputMetadata{
		Version:           1,
		Timestamp:         now.Now(ctx).UTC().Format(time.RFC3339),
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

	var bloatyDiffOutput string
	// Diff the binary built at the current changelist/patchset vs. at tip-of-tree.
	bloatyDiffOutput, metadata.BloatyDiffArgs, err = runBloatyDiff(ctx, args.stripPath, args.bloatyPath, args.binaryName)
	if err != nil {
		return skerr.Wrap(err)
	}
	metadata.CompileTaskNameNoPatch = args.compileTaskNameNoPatch

	gcsDir := computeTargetGCSDirectory(ctx, args.repoState, args.taskID, args.compileTaskName)

	// Upload pretty-printed JSON metadata file to GCS.
	jsonMetadata, err := json.MarshalIndent(metadata, "", "  ")
	if err != nil {
		return skerr.Wrap(err)
	}
	if err = uploadFileToGCS(ctx, args.codesizeGCS, fmt.Sprintf("%s/%s.json", gcsDir, args.binaryName), jsonMetadata); err != nil {
		return skerr.Wrap(err)
	}

	// Upload Bloaty diff output plain-text file to GCS.
	if err = uploadFileToGCS(ctx, args.codesizeGCS, fmt.Sprintf("%s/%s.diff.txt", gcsDir, args.binaryName), []byte(bloatyDiffOutput)); err != nil {
		return skerr.Wrap(err)
	}

	// Upload Bloaty output .tsv file to GCS.
	//
	// It is important that we upload the .tsv file last because the codesizeserver binary will
	// only start processing the .json and .diff.txt files once it receives the Pub/Sub
	// notification that a .tsv file has been uploaded. Pub/Sub notifications are pretty quick, so
	// by uploading files in this order we avoid a race condition.
	if err = uploadFileToGCS(ctx, args.codesizeGCS, fmt.Sprintf("%s/%s.tsv", gcsDir, args.binaryName), []byte(bloatyOutput)); err != nil {
		return skerr.Wrap(err)
	}
	if args.repoState.IsTryJob() {
		// Add VM and file diff results to the step data. This is consumed by the codesize plugin
		// to display results on the Gerrit CL for tryjob runs.
		vmDiff, fileDiff := parseBloatyDiffOutput(bloatyDiffOutput)
		if vmDiff != "" && fileDiff != "" {
			td.StepText(ctx, "VM Diff", vmDiff)
			td.StepText(ctx, "File Diff", fileDiff)
		}

		// TODO(rmistry): Remove the below "Diff Bytes" section after the above
		// works and is integrated with the codesize plugin.
		s, err := os_steps.Stat(ctx, filepath.Join("build", args.binaryName+"_stripped"))
		if err != nil {
			return err
		}
		totalBytes := s.Size()

		s, err = os_steps.Stat(ctx, filepath.Join("build_nopatch", args.binaryName+"_stripped"))
		if err != nil {
			return err
		}
		beforeBytes := s.Size()

		diffBytes := totalBytes - beforeBytes
		td.StepText(ctx, "Diff Bytes", strconv.FormatInt(diffBytes, 10))
	} else {
		// Upload perf data for non-tryjob runs on status.skia.org.
		perfData := format.Format{
			Version: 1,
			GitHash: args.repoState.Revision,
			Key: map[string]string{
				"binary":            args.binaryName,
				"compile_task_name": args.compileTaskName,
			},
			Links: map[string]string{
				"full_data": taskdriverURL + args.taskID,
			},
		}
		if err = uploadPerfData(ctx, args.perfGCS, gcsDir, args.binaryName, args.taskID, perfData); err != nil {
			return skerr.Wrap(err)
		}
	}

	return nil
}

// parseBloatyDiffOutput parses bloaty output and returns the VM diff
// and the file diff strings.
// Example: for "...\n...\n+0.0% +832 TOTAL +848Ki +0.0%\n\n" we return
// (+832, +848Ki).
// If the output is not in expected format then we return empty strings.
func parseBloatyDiffOutput(bloatyDiffOutput string) (string, string) {
	tokens := strings.Split(strings.Trim(bloatyDiffOutput, "\n"), "\n")
	if len(tokens) > 0 {
		// Final line in bloaty output is the line with the results.
		outputLine := tokens[len(tokens)-1]
		words := strings.Fields(outputLine)
		// Format is expected to look like this:
		// +0.0% +832 TOTAL +848 +0.0%
		if len(words) == 5 {
			return words[1], words[3]
		}
	}
	return "", ""
}

// runBloaty runs Bloaty against the given binary and returns the Bloaty output in TSV format and
// the Bloaty command-line arguments used. It uses the strip command to strip out debug symbols,
// so they do not inflate the file size numbers.
func runBloaty(ctx context.Context, stripPath, bloatyPath, binaryName string) (string, []string, error) {
	binaryWithSymbols := filepath.Join("build", binaryName)
	binaryNoSymbols := filepath.Join("build", binaryName+"_stripped")
	err := td.Do(ctx, td.Props("Create stripped version of binary"), func(ctx context.Context) error {
		runCmd := &exec.Command{
			Name:       "cp",
			Args:       []string{binaryWithSymbols, binaryNoSymbols},
			InheritEnv: true,
			LogStdout:  true,
			LogStderr:  true,
		}
		_, err := exec.RunCommand(ctx, runCmd)
		if err != nil {
			return skerr.Wrap(err)
		}
		runCmd = &exec.Command{
			Name:       stripPath,
			Args:       []string{binaryNoSymbols},
			InheritEnv: true,
			LogStdout:  true,
			LogStderr:  true,
		}
		_, err = exec.RunCommand(ctx, runCmd)
		if err != nil {
			return skerr.Wrap(err)
		}
		runCmd = &exec.Command{
			Name:       "ls",
			Args:       []string{"-al", "build"},
			InheritEnv: true,
			LogStdout:  true,
			LogStderr:  true,
		}
		_, err = exec.RunCommand(ctx, runCmd)
		if err != nil {
			return skerr.Wrap(err)
		}

		return nil
	})
	if err != nil {
		return "", nil, skerr.Wrap(err)
	}

	runCmd := &exec.Command{
		Name: bloatyPath,
		Args: []string{
			binaryNoSymbols,
			"-d",
			"compileunits,symbols",
			"-n",
			"0",
			"--tsv",
			"--debug-file=" + binaryWithSymbols,
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
		return "", nil, skerr.Wrap(err)
	}

	return bloatyOutput, runCmd.Args, nil
}

// runBloatyDiff invokes Bloaty to diff the given binary built at the current changelist/patchset
// vs. at tip of tree, and returns the plain-text Bloaty output and the command-line arguments
// used. Like before, it strips the debug symbols out before computing that diff.
func runBloatyDiff(ctx context.Context, stripPath, bloatyPath, binaryName string) (string, []string, error) {
	// These were created from the runBloaty step
	binaryWithPatchWithSymbols := filepath.Join("build", binaryName)
	binaryWithPatchWithNoSymbols := filepath.Join("build", binaryName+"_stripped")
	// These will be created next
	binaryWithNoPatchWithSymbols := filepath.Join("build_nopatch", binaryName)
	binaryWithNoPatchWithNoSymbols := filepath.Join("build_nopatch", binaryName+"_stripped")
	err := td.Do(ctx, td.Props("Create stripped version of no_patch binary"), func(ctx context.Context) error {
		runCmd := &exec.Command{
			Name:       "cp",
			Args:       []string{binaryWithNoPatchWithSymbols, binaryWithNoPatchWithNoSymbols},
			InheritEnv: true,
			LogStdout:  true,
			LogStderr:  true,
		}
		_, err := exec.RunCommand(ctx, runCmd)
		if err != nil {
			return skerr.Wrap(err)
		}
		runCmd = &exec.Command{
			Name:       stripPath,
			Args:       []string{binaryWithNoPatchWithNoSymbols},
			InheritEnv: true,
			LogStdout:  true,
			LogStderr:  true,
		}
		_, err = exec.RunCommand(ctx, runCmd)
		if err != nil {
			return skerr.Wrap(err)
		}
		runCmd = &exec.Command{
			Name:       "ls",
			Args:       []string{"-al", "build_nopatch"},
			InheritEnv: true,
			LogStdout:  true,
			LogStderr:  true,
		}
		_, err = exec.RunCommand(ctx, runCmd)
		return err
	})
	if err != nil {
		return "", nil, skerr.Wrap(err)
	}

	runCmd := &exec.Command{
		Name: bloatyPath,
		Args: []string{
			binaryWithPatchWithNoSymbols,
			"--debug-file=" + binaryWithPatchWithSymbols,
			"-d", "symbols", "-n", "0", "-s", "file",
			"--",
			binaryWithNoPatchWithNoSymbols,
			"--debug-file=" + binaryWithNoPatchWithSymbols,
		},
		InheritEnv: true,
		LogStdout:  true,
		LogStderr:  true,
	}

	var bloatyOutput string
	if err := td.Do(ctx, td.Props(fmt.Sprintf("Run Bloaty diff against binary %q", binaryName)), func(ctx context.Context) error {
		bloatyOutput, err = exec.RunCommand(ctx, runCmd)
		return err
	}); err != nil {
		return "", nil, skerr.Wrap(err)
	}

	return bloatyOutput, runCmd.Args, nil
}

// computeTargetGCSDirectory computes the target GCS directory where to upload the Bloaty output file
// and JSON metadata file.
func computeTargetGCSDirectory(ctx context.Context, repoState types.RepoState, taskID, compileTaskName string) string {
	timePrefix := now.Now(ctx).UTC().Format("2006/01/02/15") // YYYY/MM/DD/HH.
	if repoState.IsTryJob() {
		// Example: 2022/01/31/01/tryjob/12345/3/CkPp9ElAaEXyYWNHpXHU/Build-Debian10-Clang-x86_64-Release
		return fmt.Sprintf("%s/tryjob/%s/%s/%s/%s", timePrefix, repoState.Patch.Issue, repoState.Patch.Patchset, taskID, compileTaskName)
	} else {
		// Example: 2022/01/31/01/033ccea12c0949d0f712471bfcb4ed6daf69aaff/Build-Debian10-Clang-x86_64-Release
		return fmt.Sprintf("%s/%s/%s", timePrefix, repoState.Revision, compileTaskName)
	}
}

// uploadPerfData gets the file size of the stripped binary (i.e. without debug symbols), formats
// the JSON how Perf expects it, and uploads it to Perf's GCS bucket.
func uploadPerfData(ctx context.Context, perfGCS gcs.GCSClient, gcsPathPrefix, binaryName, taskID string, perfData format.Format) error {
	// Use the taskID to guarantee unique file ids
	gcsPath := "nano-json-v1/" + gcsPathPrefix + "/codesize_" + taskID + ".json"

	err := td.Do(ctx, td.Props("Upload total stripped binary size to Perf"), func(ctx context.Context) error {
		s, err := os_steps.Stat(ctx, filepath.Join("build", binaryName+"_stripped"))
		if err != nil {
			return err
		}
		totalBytes := s.Size()

		s, err = os_steps.Stat(ctx, filepath.Join("build_nopatch", binaryName+"_stripped"))
		if err != nil {
			return err
		}
		beforeBytes := s.Size()

		perfData.Results = []format.Result{{
			Key:         map[string]string{"measurement": "stripped_binary_bytes"},
			Measurement: float32(totalBytes),
		}, {
			Key:         map[string]string{"measurement": "stripped_diff_bytes"},
			Measurement: float32(totalBytes - beforeBytes),
		}}

		perfJSON, err := json.MarshalIndent(perfData, "", "  ")
		if err != nil {
			return err
		}
		return uploadFileToGCS(ctx, perfGCS, gcsPath, perfJSON)
	})
	if err != nil {
		return skerr.Wrap(err)
	}
	return nil
}

// uploadFileToGCS uploads a file to the given GCS bucket.
func uploadFileToGCS(ctx context.Context, gcsClient gcs.GCSClient, path string, contents []byte) error {
	gcsURL := fmt.Sprintf("gs://%s/%s", gcsClient.Bucket(), path)
	return td.Do(ctx, td.Props(fmt.Sprintf("Upload %s", gcsURL)), func(ctx context.Context) error {
		if err := gcsClient.SetFileContents(ctx, path, gcs.FILE_WRITE_OPTS_TEXT, contents); err != nil {
			return skerr.Wrapf(err, "Could not write task to %s", gcsURL)
		}
		return nil
	})
}
