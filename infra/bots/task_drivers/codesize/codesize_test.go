// Copyright 2022 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package main

import (
	"context"
	"os"
	"path/filepath"
	"runtime"
	"testing"
	"time"

	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/mock"
	"github.com/stretchr/testify/require"
	"go.skia.org/infra/bazel/go/bazel"

	"go.skia.org/infra/go/exec"
	"go.skia.org/infra/go/gcs"
	"go.skia.org/infra/go/gcs/test_gcsclient"
	"go.skia.org/infra/go/gerrit"
	gerrit_testutils "go.skia.org/infra/go/gerrit/testutils"
	"go.skia.org/infra/go/git"
	"go.skia.org/infra/go/git/git_common"
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
	// The revision is assigned deterministically by the GitBuilder in test().
	const (
		expectedBloatyFileGCSPath       = "2022/01/31/01/693abc06538769c662ca1871d347323b133a5d3c/Build-Debian10-Clang-x86_64-Release/dm.tsv"
		expectedBloatyDiffFileGCSPath   = "2022/01/31/01/693abc06538769c662ca1871d347323b133a5d3c/Build-Debian10-Clang-x86_64-Release/dm.diff.txt"
		expectedJSONMetadataFileGCSPath = "2022/01/31/01/693abc06538769c662ca1871d347323b133a5d3c/Build-Debian10-Clang-x86_64-Release/dm.json"

		expectedPerfFileGCSPath = "nano-json-v1/2022/01/31/01/693abc06538769c662ca1871d347323b133a5d3c/Build-Debian10-Clang-x86_64-Release/codesize_CkPp9ElAaEXyYWNHpXHU.json"
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
  "compile_task_name_no_patch": "Build-Debian10-Clang-x86_64-Release-NoPatch",
  "binary_name": "dm",
  "bloaty_cipd_version": "1",
  "bloaty_args": [
    "build/dm_stripped",
    "-d",
    "compileunits,symbols",
    "-n",
    "0",
    "--tsv",
    "--debug-file=build/dm"
  ],
  "bloaty_diff_args": [
    "build/dm_stripped",
    "--debug-file=build/dm",
    "-d",
    "symbols",
    "-n",
    "0",
    "-s",
    "file",
    "--",
    "build_nopatch/dm_stripped",
    "--debug-file=build_nopatch/dm"
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
	const expectedPerfContents = `{
  "version": 1,
  "git_hash": "693abc06538769c662ca1871d347323b133a5d3c",
  "key": {
    "binary": "dm",
    "compile_task_name": "Build-Debian10-Clang-x86_64-Release"
  },
  "results": [
    {
      "key": {
        "measurement": "stripped_binary_bytes"
      },
      "measurement": 17
    },
    {
      "key": {
        "measurement": "stripped_diff_bytes"
      },
      "measurement": -6
    }
  ],
  "links": {
    "full_data": "https://task-driver.skia.org/td/CkPp9ElAaEXyYWNHpXHU"
  }
}`

	const expectedBloatyFileContents = "I'm a fake Bloaty output!"
	const expectedBloatyDiffFileContents = "Fake Bloaty diff output over here!"

	// Make sure we use UTC instead of the system timezone.
	fakeNow := time.Date(2022, time.January, 31, 2, 2, 3, 0, time.FixedZone("UTC+1", 60*60))

	repoState := types.RepoState{
		Repo: "https://skia.googlesource.com/skia.git",
	}
	mockGerrit, mockGitiles, repoState := setupMockGit(t, repoState)

	commandCollector := exec.CommandCollector{}
	// Mock "bloaty" invocations to output the appropriate contents to the fake stdout.
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

	mockCodeSizeGCS := mockGCSClient(codesizeGCSBucketName)
	expectUpload(t, mockCodeSizeGCS, expectedBloatyFileGCSPath, expectedBloatyFileContents)
	expectUpload(t, mockCodeSizeGCS, expectedBloatyDiffFileGCSPath, expectedBloatyDiffFileContents)
	expectUpload(t, mockCodeSizeGCS, expectedJSONMetadataFileGCSPath, expectedJSONMetadataFileContents)

	mockPerfGCS := mockGCSClient(perfGCSBucketName)
	expectUpload(t, mockPerfGCS, expectedPerfFileGCSPath, expectedPerfContents)

	// Realistic but arbitrary arguments.
	args := runStepsArgs{
		repoState:              repoState,
		gerrit:                 mockGerrit.Gerrit,
		gitilesRepo:            mockGitiles,
		codesizeGCS:            mockCodeSizeGCS,
		perfGCS:                mockPerfGCS,
		swarmingTaskID:         "58dccb0d6a3f0411",
		swarmingServer:         "https://chromium-swarm.appspot.com",
		taskID:                 "CkPp9ElAaEXyYWNHpXHU",
		taskName:               "CodeSize-dm-Debian10-Clang-x86_64-Release",
		compileTaskName:        "Build-Debian10-Clang-x86_64-Release",
		compileTaskNameNoPatch: "Build-Debian10-Clang-x86_64-Release-NoPatch",
		binaryName:             "dm",
		bloatyCIPDVersion:      "1",
		bloatyPath:             "/path/to/bloaty",
		stripPath:              "/path/to/strip",
	}

	res := td.RunTestSteps(t, false, func(ctx context.Context) error {
		ctx = now.TimeTravelingContext(fakeNow).WithContext(ctx)
		ctx = td.WithExecRunFn(ctx, commandCollector.Run)
		// Be in a temporary directory
		require.NoError(t, os.Chdir(t.TempDir()))
		// Create a file to simulate the result of copying and stripping the binary
		createTestFile(t, filepath.Join("build", "dm_stripped"), "This has 17 bytes")
		createTestFile(t, filepath.Join("build_nopatch", "dm_stripped"), "This has 23 bytes total")

		err := runSteps(ctx, args)
		assert.NoError(t, err)
		return err
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

	// We expect the following sequence of commands: "cp", "strip", "ls", "bloaty",
	//                                               "cp", "strip", "ls", "bloaty".
	require.Len(t, commands, 8)

	// We copy the binary and strip the debug symbols from the copy.
	assertCommandEqual(t, commands[0], "cp", "build/dm", "build/dm_stripped")
	assertCommandEqual(t, commands[1], "/path/to/strip", "build/dm_stripped")
	// listing the contents of the directory with the binaries is useful for debugging.
	assertCommandEqual(t, commands[2], "ls", "-al", "build")

	// Assert that Bloaty was invoked on the binary with the right arguments
	assertCommandEqual(t, commands[3], "/path/to/bloaty",
		"build/dm_stripped", "-d", "compileunits,symbols", "-n", "0", "--tsv", "--debug-file=build/dm")

	assertCommandEqual(t, commands[4], "cp", "build_nopatch/dm", "build_nopatch/dm_stripped")
	assertCommandEqual(t, commands[5], "/path/to/strip", "build_nopatch/dm_stripped")
	// Assert that "ls build_nopatch" was executed to list the contents of the directory with the
	// binaries built by the compile task at tip-of-tree, for debugging purposes.
	assertCommandEqual(t, commands[6], "ls", "-al", "build_nopatch")
	// We perform a diff between the two binaries (the -- is how bloaty does that).
	assertCommandEqual(t, commands[7], "/path/to/bloaty",
		"build/dm_stripped", "--debug-file=build/dm",
		"-d", "symbols", "-n", "0", "-s", "file",
		"--", "build_nopatch/dm_stripped", "--debug-file=build_nopatch/dm")

	// Assert that the .json and .tsv files were uploaded to GCS.
	mockCodeSizeGCS.AssertExpectations(t)
}

func createTestFile(t *testing.T, path, contents string) {
	require.NoError(t, os.MkdirAll(filepath.Dir(path), 0755))
	require.NoError(t, os.WriteFile(path, []byte(contents), 0644))
}

func TestRunSteps_Tryjob_Success(t *testing.T) {
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
  "compile_task_name_no_patch": "Build-Debian10-Clang-x86_64-Release-NoPatch",
  "binary_name": "dm",
  "bloaty_cipd_version": "1",
  "bloaty_args": [
    "build/dm_stripped",
    "-d",
    "compileunits,symbols",
    "-n",
    "0",
    "--tsv",
    "--debug-file=build/dm"
  ],
  "bloaty_diff_args": [
    "build/dm_stripped",
    "--debug-file=build/dm",
    "-d",
    "symbols",
    "-n",
    "0",
    "-s",
    "file",
    "--",
    "build_nopatch/dm_stripped",
    "--debug-file=build_nopatch/dm"
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
	const expectedBloatyFileContents = "I'm a fake Bloaty output!"
	const expectedBloatyDiffFileContents = "Fake Bloaty diff output over here!"

	// Make sure we use UTC instead of the system timezone.
	fakeNow := time.Date(2022, time.January, 31, 2, 2, 3, 0, time.FixedZone("UTC+1", 60*60))

	repoState := types.RepoState{
		Patch: types.Patch{
			Issue:     "12345",
			PatchRepo: "https://skia.googlesource.com/skia.git",
			Patchset:  "3",
			Server:    "https://skia-review.googlesource.com",
		},
		Repo: "https://skia.googlesource.com/skia.git",
	}
	mockGerrit, mockGitiles, repoState := setupMockGit(t, repoState)

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

	mockCodeSizeGCS := mockGCSClient(codesizeGCSBucketName)
	expectUpload(t, mockCodeSizeGCS, expectedBloatyFileGCSPath, expectedBloatyFileContents)
	expectUpload(t, mockCodeSizeGCS, expectedBloatyDiffFileGCSPath, expectedBloatyDiffFileContents)
	expectUpload(t, mockCodeSizeGCS, expectedJSONMetadataFileGCSPath, expectedJSONMetadataFileContents)

	// Realistic but arbitrary arguments.
	args := runStepsArgs{
		repoState:              repoState,
		gerrit:                 mockGerrit.Gerrit,
		gitilesRepo:            mockGitiles,
		codesizeGCS:            mockCodeSizeGCS,
		swarmingTaskID:         "58dccb0d6a3f0411",
		swarmingServer:         "https://chromium-swarm.appspot.com",
		taskID:                 "CkPp9ElAaEXyYWNHpXHU",
		taskName:               "CodeSize-dm-Debian10-Clang-x86_64-Release",
		compileTaskName:        "Build-Debian10-Clang-x86_64-Release",
		compileTaskNameNoPatch: "Build-Debian10-Clang-x86_64-Release-NoPatch",
		binaryName:             "dm",
		bloatyCIPDVersion:      "1",
		bloatyPath:             "/path/to/bloaty",
		stripPath:              "/path/to/strip",
	}

	res := td.RunTestSteps(t, false, func(ctx context.Context) error {
		ctx = now.TimeTravelingContext(fakeNow).WithContext(ctx)
		ctx = td.WithExecRunFn(ctx, commandCollector.Run)
		// Be in a temporary directory
		require.NoError(t, os.Chdir(t.TempDir()))
		// Create a file to simulate the result of copying and stripping the binary
		createTestFile(t, filepath.Join("build", "dm_stripped"), "This has 17 bytes")
		createTestFile(t, filepath.Join("build_nopatch", "dm_stripped"), "This has 23 bytes total")

		err := runSteps(ctx, args)
		assert.NoError(t, err)
		return err
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

	// We expect the following sequence of commands: "cp", "strip", "ls", "bloaty",
	//                                               "cp", "strip", "ls", "bloaty".
	require.Len(t, commands, 8)

	// We copy the binary and strip the debug symbols from the copy.
	assertCommandEqual(t, commands[0], "cp", "build/dm", "build/dm_stripped")
	assertCommandEqual(t, commands[1], "/path/to/strip", "build/dm_stripped")
	// listing the contents of the directory with the binaries is useful for debugging.
	assertCommandEqual(t, commands[2], "ls", "-al", "build")

	// Assert that Bloaty was invoked on the binary with the right arguments
	assertCommandEqual(t, commands[3], "/path/to/bloaty",
		"build/dm_stripped", "-d", "compileunits,symbols", "-n", "0", "--tsv", "--debug-file=build/dm")

	assertCommandEqual(t, commands[4], "cp", "build_nopatch/dm", "build_nopatch/dm_stripped")
	assertCommandEqual(t, commands[5], "/path/to/strip", "build_nopatch/dm_stripped")
	// Assert that "ls build_nopatch" was executed to list the contents of the directory with the
	// binaries built by the compile task at tip-of-tree, for debugging purposes.
	assertCommandEqual(t, commands[6], "ls", "-al", "build_nopatch")
	// We perform a diff between the two binaries (the -- is how bloaty does that).
	assertCommandEqual(t, commands[7], "/path/to/bloaty",
		"build/dm_stripped", "--debug-file=build/dm",
		"-d", "symbols", "-n", "0", "-s", "file",
		"--", "build_nopatch/dm_stripped", "--debug-file=build_nopatch/dm")

	// Assert that the .json, .tsv and .diff.txt files were uploaded to GCS.
	mockCodeSizeGCS.AssertExpectations(t)
}

func mockGCSClient(name string) *test_gcsclient.GCSClient {
	m := test_gcsclient.NewMockClient()
	m.On("Bucket").Return(name).Maybe()
	return m
}

func expectUpload(t *testing.T, client *test_gcsclient.GCSClient, path, contents string) {
	client.On("SetFileContents", testutils.AnyContext, path, gcs.FILE_WRITE_OPTS_TEXT, mock.Anything).Run(func(args mock.Arguments) {
		fileContents := string(args.Get(3).([]byte))
		assert.Equal(t, contents, fileContents)
	}).Return(nil)
}

func assertCommandEqual(t *testing.T, actualCmd *exec.Command, expectedCmd string, expectedArgs ...string) {
	assert.Equal(t, expectedCmd, actualCmd.Name)
	assert.Equal(t, expectedArgs, actualCmd.Args)
}

func setupMockGit(t *testing.T, repoState types.RepoState) (*gerrit_testutils.MockGerrit, *gitiles.Repo, types.RepoState) {
	commitTimestamp := time.Date(2022, time.January, 30, 23, 59, 0, 0, time.UTC)

	// Seed a fake Git repository.
	ctx := context.Background()
	if bazel.InBazelTest() && runtime.GOOS == "linux" {
		ctx = git_common.WithGitFinder(context.Background(), func() (string, error) {
			return filepath.Join(bazel.RunfilesDir(), "external", "git_linux_amd64", "bin", "git"), nil
		})
	}
	gitBuilder := git_testutils.GitInit(t, ctx)
	t.Cleanup(func() {
		gitBuilder.Cleanup()
	})
	gitBuilder.Add(ctx, "README.md", "I'm a fake repository.")
	repoState.Revision = gitBuilder.CommitMsgAt(ctx, "Fake commit subject", commitTimestamp)

	// Mock a Gerrit client.
	mockGerrit := gerrit_testutils.NewGerrit(t)
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
	mockRepo.MockGetCommit(ctx, repoState.Revision)
	mockGitiles := gitiles.NewRepo(gitBuilder.RepoUrl(), urlMock.Client())
	return mockGerrit, mockGitiles, repoState
}

func TestParseBloatyDiffOutput(t *testing.T) {
	tests := []struct {
		desc             string
		bloatyDiff       string
		expectedVMDiff   string
		expectedFileDiff string
	}{
		{
			desc:             "empty diff",
			bloatyDiff:       "",
			expectedVMDiff:   "",
			expectedFileDiff: "",
		},
		{
			desc:             "well-formed diff",
			bloatyDiff:       "test\n\test\n+0.0% +832 TOTAL +848 +0.0%\n\n",
			expectedVMDiff:   "+832",
			expectedFileDiff: "+848",
		},
		{
			desc:             "malformed diff",
			bloatyDiff:       "test\n\test\ntest\n",
			expectedVMDiff:   "",
			expectedFileDiff: "",
		},
	}

	for _, test := range tests {
		actualVMDiff, actualFileDiff := parseBloatyDiffOutput(test.bloatyDiff)
		assert.Equal(t, test.expectedVMDiff, actualVMDiff, test.desc)
		assert.Equal(t, test.expectedFileDiff, actualFileDiff, test.desc)
	}
}
