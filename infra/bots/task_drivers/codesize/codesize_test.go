// Copyright 2022 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package main

import (
	"context"
	"io/ioutil"
	"path/filepath"
	"testing"
	"time"

	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/mock"
	"github.com/stretchr/testify/require"

	"go.skia.org/infra/go/exec"
	"go.skia.org/infra/go/gcs"
	"go.skia.org/infra/go/gcs/test_gcsclient"
	"go.skia.org/infra/go/gerrit"
	gerrit_testutils "go.skia.org/infra/go/gerrit/testutils"
	"go.skia.org/infra/go/git"
	git_testutils "go.skia.org/infra/go/git/testutils"
	"go.skia.org/infra/go/gitiles"
	gitiles_testutils "go.skia.org/infra/go/gitiles/testutils"
	"go.skia.org/infra/go/mockhttpclient"
	"go.skia.org/infra/go/now"
	"go.skia.org/infra/go/testutils"
	"go.skia.org/infra/go/util"
	"go.skia.org/infra/task_driver/go/td"
	"go.skia.org/infra/task_scheduler/go/types"
)

func TestRunSteps_PostSubmit_Success(t *testing.T) {
	// An empty inputPatch indicates this is a post-submit task.
	inputPatch := types.Patch{}

	// The revision is assigned deterministically by the GitBuilder in test().
	const (
		expectedBloatyFileGCSPath       = "2022/01/31/01/693abc06538769c662ca1871d347323b133a5d3c/Build-Debian10-Clang-x86_64-Release/dm.tsv"
		expectedJSONMetadataFileGCSPath = "2022/01/31/01/693abc06538769c662ca1871d347323b133a5d3c/Build-Debian10-Clang-x86_64-Release/dm.json"
	)

	// The revision and author are assigned deterministically by the GitBuilder in test().
	const expectedJSONMetadataFileContents = `{
  "version": 1,
  "timestamp": "2022-01-31T01:02:03Z",
  "swarming_task_id": "58dccb0d6a3f0411",
  "swarming_server": "https://chromium-swarm.appspot.com",
  "task_id": "CkPp9ElAaEXyYWNHpXHU",
  "task_name": "CodeSize-dm-Debian10-Clang-x86_64-Release",
  "compile_task_name": "Build-Debian10-Clang-x86_64-Release",
  "binary_name": "dm",
  "bloaty_cipd_version": "1",
  "bloaty_args": [
    "build/dm",
    "-d",
    "compileunits,symbols",
    "-n",
    "0",
    "--tsv"
  ],
  "patch_issue": "",
  "patch_server": "",
  "patch_set": "",
  "repo": "https://skia.googlesource.com/skia.git",
  "revision": "693abc06538769c662ca1871d347323b133a5d3c",
  "commit_timestamp": "2022-01-30T23:59:00Z",
  "author": "test (test@google.com)",
  "subject": "Fake commit subject"
}`

	test(t, inputPatch, expectedBloatyFileGCSPath, "" /* =expectedBloatyDiffFileGCSPath */, expectedJSONMetadataFileGCSPath, expectedJSONMetadataFileContents)
}

func TestRunSteps_Tryjob_Success(t *testing.T) {
	inputPatch := types.Patch{
		Issue:     "12345",
		PatchRepo: "https://skia.googlesource.com/skia.git",
		Patchset:  "3",
		Server:    "https://skia-review.googlesource.com",
	}

	const (
		expectedBloatyFileGCSPath       = "2022/01/31/01/tryjob/12345/3/CkPp9ElAaEXyYWNHpXHU/Build-Debian10-Clang-x86_64-Release/dm.tsv"
		expectedBloatyDiffFileGCSPath   = "2022/01/31/01/tryjob/12345/3/CkPp9ElAaEXyYWNHpXHU/Build-Debian10-Clang-x86_64-Release/dm.diff.txt"
		expectedJSONMetadataFileGCSPath = "2022/01/31/01/tryjob/12345/3/CkPp9ElAaEXyYWNHpXHU/Build-Debian10-Clang-x86_64-Release/dm.json"
	)

	// The revision and author are assigned deterministically by the GitBuilder in test().
	const expectedJSONMetadataFileContents = `{
  "version": 1,
  "timestamp": "2022-01-31T01:02:03Z",
  "swarming_task_id": "58dccb0d6a3f0411",
  "swarming_server": "https://chromium-swarm.appspot.com",
  "task_id": "CkPp9ElAaEXyYWNHpXHU",
  "task_name": "CodeSize-dm-Debian10-Clang-x86_64-Release",
  "compile_task_name": "Build-Debian10-Clang-x86_64-Release",
  "binary_name": "dm",
  "bloaty_cipd_version": "1",
  "bloaty_args": [
    "build/dm",
    "-d",
    "compileunits,symbols",
    "-n",
    "0",
    "--tsv"
  ],
  "bloaty_diff_args": [
    "build/dm",
    "--",
    "build_nopatch/dm"
  ],
  "patch_issue": "12345",
  "patch_server": "https://skia-review.googlesource.com",
  "patch_set": "3",
  "repo": "https://skia.googlesource.com/skia.git",
  "revision": "693abc06538769c662ca1871d347323b133a5d3c",
  "commit_timestamp": "2022-01-30T23:59:00Z",
  "author": "test (test@google.com)",
  "subject": "Fake commit subject"
}`

	test(t, inputPatch, expectedBloatyFileGCSPath, expectedBloatyDiffFileGCSPath, expectedJSONMetadataFileGCSPath, expectedJSONMetadataFileContents)
}

// test assumes a post-submit task when expectedBloatyDiffFileGCSPath is empty, or a tryjob when
// set.
func test(t *testing.T, patch types.Patch, expectedBloatyFileGCSPath, expectedBloatyDiffFileGCSPath, expectedJSONMetadataFileGCSPath, expectedJSONMetadataFileContents string) {
	isTryjob := expectedBloatyDiffFileGCSPath != ""

	const expectedBloatyFileContents = "I'm a fake Bloaty output!"
	const expectedBloatyDiffFileContents = "Fake Bloaty diff output over here!"

	// Make sure we use UTC instead of the system timezone.
	fakeNow := time.Date(2022, time.January, 31, 2, 2, 3, 0, time.FixedZone("UTC+1", 60*60))
	commitTimestamp := time.Date(2022, time.January, 30, 23, 59, 0, 0, time.UTC)

	repoState := types.RepoState{
		Patch: patch,
		Repo:  "https://skia.googlesource.com/skia.git",
	}

	// Seed a fake Git repository.
	gitBuilder := git_testutils.GitInit(t, context.Background())
	defer gitBuilder.Cleanup()
	gitBuilder.Add(context.Background(), "README.md", "I'm a fake repository.")
	repoState.Revision = gitBuilder.CommitMsgAt(context.Background(), "Fake commit subject", commitTimestamp)

	// Mock a Gerrit client.
	tmp, err := ioutil.TempDir("", "")
	require.NoError(t, err)
	defer testutils.RemoveAll(t, tmp)
	mockGerrit := gerrit_testutils.NewGerrit(t, tmp)
	mockGerrit.MockGetIssueProperties(&gerrit.ChangeInfo{
		Issue: 12345,
		Owner: &gerrit.Person{
			Name:  "test",
			Email: "test@google.com",
		},
		Subject: "Fake commit subject",
		// We ignore the patchset commit hashes, their values do not matter.
		Revisions: map[string]*gerrit.Revision{
			"commit hash for patchset 1": {
				Number:        1,
				CreatedString: commitTimestamp.Add(-2 * time.Hour).Format(time.RFC3339),
			},
			"commit hash for patchset 2": {
				Number:        2,
				CreatedString: commitTimestamp.Add(-time.Hour).Format(time.RFC3339),
			},
			"commit hash for patchset 3": {
				Number:        3,
				CreatedString: commitTimestamp.Format(time.RFC3339),
			},
			"commit hash for patchset 4": {
				Number:        4,
				CreatedString: commitTimestamp.Add(time.Hour).Format(time.RFC3339),
			},
		},
	})

	// Mock a Gitiles client.
	urlMock := mockhttpclient.NewURLMock()
	mockRepo := gitiles_testutils.NewMockRepo(t, gitBuilder.RepoUrl(), git.GitDir(gitBuilder.Dir()), urlMock)
	mockRepo.MockGetCommit(context.Background(), repoState.Revision)
	mockGitiles := gitiles.NewRepo(gitBuilder.RepoUrl(), urlMock.Client())

	// Mock "bloaty" invocations.
	commandCollector := exec.CommandCollector{}
	commandCollector.SetDelegateRun(func(ctx context.Context, cmd *exec.Command) error {
		if filepath.Base(cmd.Name) == "bloaty" {
			// This argument indicates it's a binary diff invocation, see
			// https://github.com/google/bloaty/blob/f01ea59bdda11708d74a3826c23d6e2db6c996f0/doc/using.md#size-diffs.
			if util.In("--", cmd.Args) {
				cmd.CombinedOutput.Write([]byte(expectedBloatyDiffFileContents))
			} else {
				cmd.CombinedOutput.Write([]byte(expectedBloatyFileContents))
			}
			return nil
		}
		// "ls" and any other commands directly executed by the task driver produce no mock outputs.
		return nil
	})

	mockGCSClient := test_gcsclient.NewMockClient()
	defer mockGCSClient.AssertExpectations(t)

	// Mock the GCS client call to upload the Bloaty output.
	mockGCSClient.On(
		"SetFileContents",
		testutils.AnyContext,
		expectedBloatyFileGCSPath,
		gcs.FILE_WRITE_OPTS_TEXT,
		mock.Anything,
	).Run(func(args mock.Arguments) {
		fileContents := string(args.Get(3).([]byte))
		assert.Equal(t, expectedBloatyFileContents, fileContents)
	}).Return(nil)

	if isTryjob {
		// Mock the GCS client call to upload the Bloaty diff output.
		mockGCSClient.On(
			"SetFileContents",
			testutils.AnyContext,
			expectedBloatyDiffFileGCSPath,
			gcs.FILE_WRITE_OPTS_TEXT,
			mock.Anything,
		).Run(func(args mock.Arguments) {
			fileContents := string(args.Get(3).([]byte))
			assert.Equal(t, expectedBloatyDiffFileContents, fileContents)
		}).Return(nil)
	}

	// Mock the GCS client call to upload the JSON metadata file.
	mockGCSClient.On(
		"SetFileContents",
		testutils.AnyContext,
		expectedJSONMetadataFileGCSPath,
		gcs.FILE_WRITE_OPTS_TEXT,
		mock.Anything,
	).Run(func(args mock.Arguments) {
		fileContents := string(args.Get(3).([]byte))
		assert.Equal(t, expectedJSONMetadataFileContents, fileContents)
	}).Return(nil)

	// Realistic but arbitrary arguments.
	args := runStepsArgs{
		repoState:         repoState,
		gerrit:            mockGerrit.Gerrit,
		gitilesRepo:       mockGitiles,
		gcsClient:         mockGCSClient,
		swarmingTaskID:    "58dccb0d6a3f0411",
		swarmingServer:    "https://chromium-swarm.appspot.com",
		taskID:            "CkPp9ElAaEXyYWNHpXHU",
		taskName:          "CodeSize-dm-Debian10-Clang-x86_64-Release",
		compileTaskName:   "Build-Debian10-Clang-x86_64-Release",
		binaryName:        "dm",
		bloatyCIPDVersion: "1",
	}

	res := td.RunTestSteps(t, false, func(ctx context.Context) error {
		ctx = now.TimeTravelingContext(fakeNow).WithContext(ctx)
		ctx = td.WithExecRunFn(ctx, commandCollector.Run)

		return runSteps(ctx, args)
	})

	require.Empty(t, res.Errors)
	require.Empty(t, res.Exceptions)

	// Filter out all Git commands.
	var commands []*exec.Command
	for _, c := range commandCollector.Commands() {
		if filepath.Base(c.Name) != "git" {
			commands = append(commands, c)
		}
	}

	if isTryjob {
		// We expect the following sequence of commands: "ls", "bloaty", "ls", "bloaty".
		require.Len(t, commands, 4)
	} else {
		// We expect the following sequence of commands: "ls", "bloaty".
		require.Len(t, commands, 2)
	}

	// Assert that "ls build" was executed to list the contents of the directory with the binaries
	// built by the compile task, for debugging purposes.
	lsCmd := commands[0]
	assert.Equal(t, "ls", lsCmd.Name)
	assert.Equal(t, []string{"build"}, lsCmd.Args)

	// Assert that Bloaty was invoked with the expected arguments.
	bloatyCmd := commands[1]
	assert.Equal(t, "bloaty/bloaty", bloatyCmd.Name)
	assert.Equal(t, []string{"build/dm", "-d", "compileunits,symbols", "-n", "0", "--tsv"}, bloatyCmd.Args)

	if isTryjob {
		// Assert that "ls build_nopatch" was executed to list the contents of the directory with the
		// binaries built by the compile task at tip-of-tree, for debugging purposes.
		lsCmd = commands[2]
		assert.Equal(t, "ls", lsCmd.Name)
		assert.Equal(t, []string{"build_nopatch"}, lsCmd.Args)

		// Assert that Bloaty was invoked with the expected arguments.
		bloatyCmd = commands[3]
		assert.Equal(t, "bloaty/bloaty", bloatyCmd.Name)
		assert.Equal(t, []string{"build/dm", "--", "build_nopatch/dm"}, bloatyCmd.Args)
	}

	if isTryjob {
		// Assert that the .json, .tsv and .diff.txt files were uploaded to GCS.
		mockGCSClient.AssertNumberOfCalls(t, "SetFileContents", 3)
	} else {
		// Assert that the .json and .tsv files were uploaded to GCS.
		mockGCSClient.AssertNumberOfCalls(t, "SetFileContents", 2)
	}
}
