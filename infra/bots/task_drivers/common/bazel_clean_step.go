// Copyright 2023 Google LLC
//
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package common

import (
	"context"
	"fmt"
	"strings"

	sk_exec "go.skia.org/infra/go/exec"

	"github.com/shirou/gopsutil/disk"
	"go.skia.org/infra/go/skerr"
	"go.skia.org/infra/task_driver/go/td"
)

// The DiskSpaceLow alert triggers at 10GB, so we set this threshold to a slightly higher value.
// For reference, Swarming seems to quarantine machines when they go below 3GB.
const bazelCachePartitionMinRequiredFreeSpaceBytes = uint64(15_000_000_000)

type bazelCleanIfLowDiskSpaceContextKeyType = string

// BazelCleanIfLowDiskSpaceContextKey is a context key that can be used from tests to override the
// functions used by BazelCleanIfLowDiskSpace to compute the free space on the partition where the
// Bazel cache lives. Values associated to this context key should be of type
// BazelCleanIfLowDiskSpaceContextValue.
const BazelCleanIfLowDiskSpaceContextKey = bazelCleanIfLowDiskSpaceContextKeyType("overwriteBazelCleanIfLowDiskSpaceDiskFns")

// BazelCleanIfLowDiskSpaceContextValue is the type of the value associated with the
// BazelCleanIfLowDiskSpaceContextKey context key.
type BazelCleanIfLowDiskSpaceContextValue = struct {
	GetPartitionMountpoints func() ([]string, error)
	FreeBytesOnPartition    func(string) (uint64, error)
}

// WithEnoughSpaceOnBazelCachePartitionTestOnlyContext returns a context that makes
// common.BazelCleanIfLowDiskSpace() think there is enough space on the partition where the Bazel
// cache is found. It also returns a path within said partition where the Bazel cache is assumed to
// live, which should be passed to the code under test that invokes
// common.BazelCleanIfLowDiskSpace().
//
// This function is placed here rather than in the testutils Go package to avoid an import cycle.
func WithEnoughSpaceOnBazelCachePartitionTestOnlyContext(ctx context.Context) (context.Context, string) {
	const (
		bazelCacheDir                 = "/mnt/pd0/bazel_cache"
		bazelCachePartitionMountpoint = "/mnt/pd0"
	)

	ctx = context.WithValue(ctx, BazelCleanIfLowDiskSpaceContextKey, BazelCleanIfLowDiskSpaceContextValue{
		GetPartitionMountpoints: func() ([]string, error) {
			// For the purposes of satisfying common.BazelCleanIfLowDiskSpace(), it suffices to only return
			// the mountpoint for the partition where the Bazel cache directory lives.
			return []string{bazelCachePartitionMountpoint}, nil
		},
		FreeBytesOnPartition: func(mountpoint string) (uint64, error) {
			if mountpoint != bazelCachePartitionMountpoint {
				panic(fmt.Sprintf("mountpoint %q does not equal %q; this is a bug", mountpoint, bazelCachePartitionMountpoint))
			}
			return uint64(20_000_000_000), nil
		},
	})

	return ctx, bazelCacheDir
}

// BazelCleanIfLowDiskSpace runs "bazel clean" as a task driver step if disk space is too low. This
// step should be added at the end of any task driver that shells out to Bazel in order to prevent
// DiskSpaceLow alerts due to the Bazel cache (usually at /mnt/pd0/bazel_cache) growing too large.
//
// Ideally, we would like to tell Bazel to prevent the cache from growing above a certain size, but
// there is currently no way to do this. See discussion in the below links:
//
// - https://github.com/bazelbuild/bazel/issues/1035
// - https://github.com/bazelbuild/bazel/issues/5139
//
// Testing: Set the BazelCleanIfLowDiskSpaceContextKey context key to override the functions that
// compute the free space (measured in bytes) on the partition where the Bazel cache lives.
func BazelCleanIfLowDiskSpace(ctx context.Context, bazelCacheDir, bazelWorkspaceDir, pathToBazel string) error {
	return skerr.Wrap(td.Do(ctx, td.Props("Clean Bazel cache if disk space is too low"), func(ctx context.Context) error {
		// Are any of the disk-related functions mocked?
		getPartitionMountpointsFn := getPartitionMountpoints
		freeBytesOnPartitionFn := freeBytesOnPartition
		if ctxValue := ctx.Value(BazelCleanIfLowDiskSpaceContextKey); ctxValue != nil {
			typedCtxValue, ok := ctxValue.(BazelCleanIfLowDiskSpaceContextValue)
			if !ok {
				panic("context value associated with BazelCleanIfLowDiskSpaceContextKey is not a BazelCleanIfLowDiskSpaceContextValue")
			}
			if typedCtxValue.FreeBytesOnPartition != nil {
				freeBytesOnPartitionFn = typedCtxValue.FreeBytesOnPartition
			}
			if typedCtxValue.GetPartitionMountpoints != nil {
				getPartitionMountpointsFn = typedCtxValue.GetPartitionMountpoints
			}
		}

		// Find the partition where the Bazel cache lives.
		mountpoints, err := getPartitionMountpointsFn()
		if err != nil {
			return skerr.Wrap(err)
		}
		var mountpointCandidates []string // Any mountpoints that are prefixes of bazelCacheDir.
		for _, mountpoint := range mountpoints {
			if strings.HasPrefix(bazelCacheDir, mountpoint) {
				mountpointCandidates = append(mountpointCandidates, mountpoint)
			}
		}
		bazelCachePartitionMountpoint := ""
		for _, candidate := range mountpointCandidates {
			// The longest candidate wins. For example, if the Bazel cache directory is
			// "/mnt/pd0/bazel_cache" and the candidates are "/mnt", "/mnt/pd0" and "/", then "/mnt/pd0"
			// is selected.
			if len(candidate) > len(bazelCachePartitionMountpoint) {
				bazelCachePartitionMountpoint = candidate
			}
		}
		if bazelCachePartitionMountpoint == "" {
			return skerr.Fmt("could not find partition for Bazel cache directory at %q", bazelCacheDir)
		}

		// Find out how much free space is left on that partition.
		freeSpace, err := freeBytesOnPartitionFn(bazelCachePartitionMountpoint)
		if err != nil {
			return skerr.Wrap(err)
		}

		// Run "bazel clean" if free space on that partition is too low.
		if freeSpace < bazelCachePartitionMinRequiredFreeSpaceBytes {
			msg := fmt.Sprintf("Free space on partition %s is %d bytes, which is below the threshold of %d bytes", bazelCachePartitionMountpoint, freeSpace, bazelCachePartitionMinRequiredFreeSpaceBytes)
			if err := td.Do(ctx, td.Props(msg), func(ctx context.Context) error { return nil }); err != nil {
				return skerr.Wrap(err)
			}

			cmd := &sk_exec.Command{
				Name:       pathToBazel,
				Dir:        bazelWorkspaceDir,
				Args:       []string{"clean"},
				InheritEnv: true, // Make sure "bazelisk" is on PATH.
				LogStdout:  true,
				LogStderr:  true,
			}
			_, err := sk_exec.RunCommand(ctx, cmd)
			return skerr.Wrap(err)
		}

		msg := fmt.Sprintf("No need to clear the Bazel cache: free space on partition %s is %d bytes, which is above the threshold of %d bytes", bazelCachePartitionMountpoint, freeSpace, bazelCachePartitionMinRequiredFreeSpaceBytes)
		return skerr.Wrap(td.Do(ctx, td.Props(msg), func(ctx context.Context) error { return nil }))
	}))
}

// getPartitionMountpoints returns the mountpoints for all mounted partitions.
func getPartitionMountpoints() ([]string, error) {
	partitionStats, err := disk.Partitions(true /* =all */)
	if err != nil {
		return nil, skerr.Wrap(err)
	}
	var mountpoints []string
	for _, stat := range partitionStats {
		mountpoints = append(mountpoints, stat.Mountpoint)
	}
	return mountpoints, nil
}

// freeBytesOnPartition returns the free space measured in bytes for the partition mounted at the
// given mountpoint
func freeBytesOnPartition(mountpoint string) (uint64, error) {
	usage, err := disk.Usage(mountpoint)
	if err != nil {
		return 0, skerr.Wrap(err)
	}
	return usage.Free, nil
}
