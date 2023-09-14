// Copyright 2023 Google LLC
//
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package common

import (
	"context"
	"testing"

	exec_testutils "go.skia.org/infra/go/exec/testutils"

	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/require"
	"go.skia.org/infra/go/exec"
	"go.skia.org/infra/task_driver/go/td"
	"go.skia.org/skia/infra/bots/task_drivers/testutils"
)

func TestBazelCleanIfLowDiskSpace_EnoughDiskSpace_BazelCachePreserved(t *testing.T) {
	commandCollector := exec.CommandCollector{}
	res := td.RunTestSteps(t, false, func(ctx context.Context) error {
		ctx = td.WithExecRunFn(ctx, commandCollector.Run)

		ctx = context.WithValue(ctx, BazelCleanIfLowDiskSpaceContextKey, BazelCleanIfLowDiskSpaceContextValue{
			GetPartitionMountpoints: func() ([]string, error) {
				// Note that some of these mountpoints are prefixes of the actual mountpoint ("/mnt/pd0").
				// This test checks that BazelCleanIfLowDiskSpace correctly identifies the mountpoint.
				return []string{"/", "/boot", "/mnt", "/mnt/pd0", "/var"}, nil
			},
			FreeBytesOnPartition: func(mountpoint string) (uint64, error) {
				require.Equal(t, "/mnt/pd0", mountpoint)
				return uint64(20_000_000_000), nil
			},
		})

		err := BazelCleanIfLowDiskSpace(ctx, "/mnt/pd0/bazel_cache", "/path/to/checkout", "/path/to/bazel")

		assert.NoError(t, err)
		return err
	})

	require.Empty(t, res.Errors)
	require.Empty(t, res.Exceptions)
	testutils.AssertStepNames(t, res,
		"Clean Bazel cache if disk space is too low",
		"No need to clear the Bazel cache: free space on partition /mnt/pd0 is 20000000000 bytes, which is above the threshold of 15000000000 bytes",
	)

	assert.Empty(t, commandCollector.Commands())
}

func TestBazelCleanIfLowDiskSpace_LowDiskSpace_BazelCacheDeleted(t *testing.T) {
	commandCollector := exec.CommandCollector{}
	res := td.RunTestSteps(t, false, func(ctx context.Context) error {
		ctx = td.WithExecRunFn(ctx, commandCollector.Run)

		ctx = context.WithValue(ctx, BazelCleanIfLowDiskSpaceContextKey, BazelCleanIfLowDiskSpaceContextValue{
			GetPartitionMountpoints: func() ([]string, error) {
				// Note that some of these mountpoints are prefixes of the actual mountpoint ("/mnt/pd0").
				// This test checks that BazelCleanIfLowDiskSpace correctly identifies the mountpoint.
				return []string{"/", "/boot", "/mnt/pd0", "/var"}, nil
			},
			FreeBytesOnPartition: func(mountpoint string) (uint64, error) {
				require.Equal(t, "/mnt/pd0", mountpoint)
				return 0, nil
			},
		})

		err := BazelCleanIfLowDiskSpace(ctx, "/mnt/pd0", "/path/to/checkout", "/path/to/bazel")

		assert.NoError(t, err)
		return err
	})

	require.Empty(t, res.Errors)
	require.Empty(t, res.Exceptions)
	testutils.AssertStepNames(t, res,
		"Clean Bazel cache if disk space is too low",
		"Free space on partition /mnt/pd0 is 0 bytes, which is below the threshold of 15000000000 bytes",
		"/path/to/bazel clean",
	)

	require.Len(t, commandCollector.Commands(), 1)
	exec_testutils.AssertCommandsMatch(t, [][]string{
		{
			"/path/to/bazel",
			"clean",
		},
	}, commandCollector.Commands())
	assert.Equal(t, "/path/to/checkout", commandCollector.Commands()[0].Dir)
}
