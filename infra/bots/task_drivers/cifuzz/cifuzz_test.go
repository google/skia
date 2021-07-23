// Copyright 2020 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package main

import (
	"context"
	"io/ioutil"
	"os"
	"os/exec"
	"path/filepath"
	"strings"
	"testing"

	"go.skia.org/infra/go/testutils/unittest"

	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/require"
	"go.skia.org/infra/task_driver/go/td"
)

func TestSetupCIFuzzRepoAndDocker_Success(t *testing.T) {
	unittest.LinuxOnlyTest(t)
	base := t.TempDir()
	fakeWorkDir := filepath.Join(base, "work")
	gb, err := exec.Command("which", "git").Output()
	require.NoError(t, err)
	gitExe := strings.TrimSpace(string(gb))
	res := td.RunTestSteps(t, false, func(ctx context.Context) error {
		return setupCIFuzzRepoAndDocker(ctx, fakeWorkDir, gitExe)
	})
	require.Empty(t, res.Errors)
	require.Empty(t, res.Exceptions)
	require.Len(t, res.Steps, 1)
	step := res.Steps[0]
	assert.Equal(t, td.StepResultSuccess, step.Result)
	assert.Empty(t, step.Errors)
	assert.Empty(t, step.Exceptions)
	// Make sure the output directory is created
	assert.DirExists(t, filepath.Join(fakeWorkDir, "out"))
}

func TestExtractOutput_OnlyCopyFuzzOutputs(t *testing.T) {
	base := t.TempDir()
	fakeWorkDir := filepath.Join(base, "work")
	fakeCIFuzzOut := filepath.Join(fakeWorkDir, "out")
	fakeOutput := filepath.Join(base, "swarming_out")

	err := os.MkdirAll(fakeCIFuzzOut, 0777)
	require.NoError(t, err)

	touch(t, filepath.Join(fakeCIFuzzOut, "crash-1234"))
	touch(t, filepath.Join(fakeCIFuzzOut, "oom-1234"))
	touch(t, filepath.Join(fakeCIFuzzOut, "timeout-1234"))
	touch(t, filepath.Join(fakeCIFuzzOut, "a_fuzz_executable_that_should_not_be_copied"))

	_, err = os.Stat(fakeOutput)
	require.Error(t, err)

	res := td.RunTestSteps(t, false, func(ctx context.Context) error {
		return extractOutput(ctx, fakeWorkDir, fakeOutput)
	})
	require.Empty(t, res.Errors)
	require.Empty(t, res.Exceptions)
	require.Len(t, res.Steps, 1)
	step := res.Steps[0]
	assert.Equal(t, td.StepResultSuccess, step.Result)
	assert.Empty(t, step.Errors)
	assert.Empty(t, step.Exceptions)

	copiedFiles, err := ioutil.ReadDir(fakeOutput)
	require.NoError(t, err)
	fileNames := make([]string, 0, len(copiedFiles))
	for _, f := range copiedFiles {
		fileNames = append(fileNames, f.Name())
	}
	assert.ElementsMatch(t, []string{"crash-1234", "oom-1234", "timeout-1234"}, fileNames)
}

func touch(t *testing.T, file string) {
	err := ioutil.WriteFile(file, []byte("Whatever"), 0666)
	require.NoError(t, err)
}
