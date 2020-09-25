// Copyright 2020 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package main

import (
	"context"
	"io/ioutil"
	"os"
	"path/filepath"
	"testing"

	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/require"
	"go.skia.org/infra/task_driver/go/td"
)

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
		extractOutput(ctx, fakeWorkDir, fakeOutput)
		return nil
	})
	require.Empty(t, res.Errors)
	require.Empty(t, res.Exceptions)

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
