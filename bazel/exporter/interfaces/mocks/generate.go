// Copyright 2022 Google LLC
//
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package mocks

//go:generate bazelisk run //infra:mockery -- --name QueryCommand --srcpkg=go.skia.org/skia/bazel/exporter/interfaces --output ${PWD}
//go:generate bazelisk run //infra:mockery -- --name FileSystem --srcpkg=go.skia.org/skia/bazel/exporter/interfaces --output ${PWD}
