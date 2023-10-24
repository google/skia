// Copyright 2023 Google LLC
//
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package common

import (
	"context"
	"runtime"
)

type computeGoldAndPerfKeyValuePairsContextKeyType = string

// ComputeGoldAndPerfKeyValuePairsContextKey is a context key that can be used from tests to
// override the set of key/value pairs returned by ComputeGoldAndPerfKeyValuePairs.
const ComputeGoldAndPerfKeyValuePairsContextKey = computeGoldAndPerfKeyValuePairsContextKeyType("overwriteComputeGoldAndPerfKeyValuePairs")

// WithGoldAndPerfKeyValuePairsContext overrides the key/value pairs returned by
// ComputeGoldAndPerfKeyValuePairs.
func WithGoldAndPerfKeyValuePairsContext(ctx context.Context, keyValuePairs map[string]string) context.Context {
	return context.WithValue(ctx, ComputeGoldAndPerfKeyValuePairsContextKey, keyValuePairs)
}

// ComputeGoldAndPerfKeyValuePairs returns the set of Gold and Perf key/value pairs that should be
// determined by the task driver.
//
// TODO(lovisolo): Infer these key/value pairs from the Bazel config, host, etc.
func ComputeGoldAndPerfKeyValuePairs(ctx context.Context) map[string]string {
	if ctxValue := ctx.Value(ComputeGoldAndPerfKeyValuePairsContextKey); ctxValue != nil {
		keyValuePairs, ok := ctxValue.(map[string]string)
		if !ok {
			panic("context value associated with ComputeGoldAndPerfKeyValuePairsContextKey is not a map[string]string")
		}
		return keyValuePairs
	}

	// The "os" key produced by DM can have values like these:
	//
	// - Android
	// - ChromeOS
	// - Debian10
	// - Mac10.15.7
	// - Mac11
	// - Ubuntu18
	// - Win10
	// - Win2019
	// - iOS
	//
	// TODO(lovisolo): Determine the "os" key in a fashion similar to DM.
	if runtime.GOOS != "linux" {
		panic("only linux is supported at this time")
	}
	os := "linux"

	// TODO(lovisolo): Delete this temporary hack.
	if runtime.GOARCH == "arm" || runtime.GOARCH == "arm64" {
		// As a temporary hack to be able to generate diferent traces for the same GM on Linux vs.
		// Android, we assume that if the task driver is running on an ARM machine, then it's a
		// Raspberry Pi connected to an Android phone. This is only for use while we experiment with
		// Bazel-built GM and benchmark tests.
		//
		// Moving forward, we should try to derive the "os", "model" and "arch" keys from the
		// BazelTest-* task's "host" component. A potential approach could be to use hosts such as
		// "NUC9i7QN_Debian11". In this example, we can derive the "model" and "arch" keys from the
		// "NUC9i7QN" part, and the "os" key would match the "Debian11" part.
		os = "android"
	}

	// TODO(lovisolo): "arch" key ("arm", "arm64", "x86", "x86_64", etc.).
	// TODO(lovisolo): "configuration" key ("Debug", "Release", "OptimizeForSize", etc.).
	// TODO(lovisolo): "model" key ("MacBook10.1", "Pixel5", "iPadPro", "iPhone11", etc.).

	return map[string]string{
		"os": os,
	}
}
