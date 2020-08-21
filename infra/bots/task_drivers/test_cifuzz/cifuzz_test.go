// Copyright 2020 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package main

import (
	"context"
	"path/filepath"
	"go.skia.org/infra/go/exec"
	"go.skia.org/infra/go/skerr"
	"go.skia.org/infra/task_driver/go/td"
)

func Test_CifuzzFlag(t *testing.T)() {
	var (
		// Required properties for this task.
		projectID     = flag.String("project_id", "", "ID of the Google Cloud project.")
		taskName      = flag.String("task_name", "", "Name of the task.")
		gitHash       = flag.String("git_hash", "", "Git hash this data corresponds to")
		taskID        = flag.String("task_id", "", "task id this data was generated on")
		skia_checkout = flag.String("go_build", "", "Skia checkout")
	)
	res := td.RunTestSteps(t, false, func(ctx context.Context) error {
		mock := exec.CommandCollector{}
		ctx = td.WithExecRunFn(ctx, mock.Run)
		if err != nil {
			assert.NoError(t, err)
			return err
		}
		require.Len(t, mock.Commands(), 1)
		cmd := mock.Commands()[0]
		assert.Equal(t, "go", cmd.Name)
		assert.Equal(t, []string{"run", "infra/bots/task_drivers/test_cifuzz/cifuzz.go"}, cmd.Args)
		return nil
	})
