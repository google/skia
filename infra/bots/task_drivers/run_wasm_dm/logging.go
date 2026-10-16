// Copyright 2026 Google LLC
//
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package main

import (
	"io"

	"github.com/jcgregorio/logger"
)

type logWriter struct{ w io.Writer }

// Static asserts that we implement the right interfaces
var _ io.Writer = &logWriter{}
var _ logger.SyncWriter = &logWriter{}

func (s *logWriter) Write(p []byte) (int, error) { return s.w.Write(p) }
func (*logWriter) Sync() error                   { return nil }

func newLogger(w io.Writer) *logger.Logger {
	return logger.NewFromOptions(&logger.Options{
		SyncWriter: &logWriter{w},
	})
}
