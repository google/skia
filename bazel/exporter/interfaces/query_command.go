// Copyright 2022 Google LLC
//
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
package interfaces

// QueryCommand defines an interface for returning the response of a
// Bazel query or cquery call.
type QueryCommand interface {
	// Read will return the response data for a `bazel [cquery|query] ...`
	// invocation.
	Read() ([]byte, error)
}
