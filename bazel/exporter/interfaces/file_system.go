// Copyright 2022 Google LLC
//
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
package interfaces

// FileSystem defines an interface for interacting with the underlying OS
// filesystem.
type FileSystem interface {
	// OpenFile defines a function responsible for opening a file with
	// write access identified by the absolute path.
	OpenFile(path string) (Writer, error)

	// ReadFile defines a function responsible for reading the entire
	// contents of a file from disk.
	ReadFile(filename string) ([]byte, error)
}
