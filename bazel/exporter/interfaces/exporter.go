// Copyright 2022 Google LLC
//
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package interfaces

import "io"

// Writer is an interface that groups io.StringWriter and io.Writer to
// enable simpler writing of the exported Bazel output text.
type Writer interface {
	io.StringWriter
	io.Writer
}

// Exporter defines an interface for exporting the Bazel workspace
// rules to a different project format.
type Exporter interface {
	// Export will write the converted Bazel cquery response data to
	// a new project format.
	Export(qcmd QueryCommand) error
}
