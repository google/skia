//go:build tools
// +build tools

/*
 * Copyright 2025 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

package main

// This file exists to ensure that tool dependencies (eg. go mod) which are
// not directly imported in our code actually get included in the go.mod file.
// For more information see the discussion on:
// https://github.com/golang/go/issues/25922

import (
	// We build @org_skia_go_infra//infra/bots/task_drivers/canary/canary.go (in
	// build_task_drivers.sh) but that dependency is not noticed by go and we cannot directly
	// depend on that package (because it's a main package), so we add this other package
	// that also depends on cloud.google.com/go/datastore to include that transitive dependency.
	_ "go.skia.org/infra/go/ds"
	_ "go.skia.org/infra/go/firestore"

	_ "go.chromium.org/luci"

	_ "github.com/vektra/mockery/v2"
)
