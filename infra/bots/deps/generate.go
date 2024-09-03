// Copyright 2024 Google LLC
//
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

//go:build ignore
// +build ignore

package main

import (
	"go.skia.org/infra/go/depot_tools/generator"
)

func main() {
	generator.MustGenerate("../../../DEPS")
}
