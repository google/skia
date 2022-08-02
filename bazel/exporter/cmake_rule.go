// Copyright 2022 Google LLC
//
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package exporter

import (
	"io"

	"go.skia.org/infra/go/skerr"
	"go.skia.org/infra/go/util"
	"go.skia.org/skia/bazel/exporter/build_proto/build"
)

// cmakeRule represents the CMake equivalent to a Bazel rule.
type cmakeRule struct {
	contents []byte      // Data to be written to CMake file.
	deps     []string    // Names of Bazel targets (not files) on which this rule directly depends.
	rule     *build.Rule // Holding pointer because struct contains a Mutex.
}

// newCMakeRule will create a new CMake rule object.
func newCMakeRule(r *build.Rule) *cmakeRule {
	return &cmakeRule{rule: r}
}

// Return the rule name.
func (r *cmakeRule) getName() string {
	return r.rule.GetName()
}

// Does this rule contain source dependencies?
func (r *cmakeRule) hasSrcs() bool {
	srcs, _ := getRuleStringArrayAttribute(r.rule, "srcs")
	return len(srcs) > 0
}

// Does this rule depend directly on a rule.
func (r *cmakeRule) hasDependency(ruleName string) bool {
	return util.In(ruleName, r.deps)
}

// Add a rule dependency.
func (r *cmakeRule) addDependency(ruleName string) error {
	if ruleName == "" {
		return skerr.Fmt("empty rule name")
	}
	if ruleName == r.getName() {
		return skerr.Fmt("rule cannot depend on self: %s", ruleName)
	}
	if r.hasDependency(ruleName) {
		return nil
	}
	// Trusting that circular dependencies are already fixed by Bazel.
	r.deps = append(r.deps, ruleName)
	return nil
}

// setContents will set the supplied chunk of a CMake project file to this
// rules contents.
func (r *cmakeRule) setContents(contents []byte) {
	r.contents = contents
}

// Write the contents for this rule using the supplied writer.
// Returns the number of bytes written and error.
func (r *cmakeRule) write(writer io.Writer) (int, error) {
	if len(r.contents) == 0 {
		return 0, skerr.Fmt("rule %s has no contents", r.getName())
	}
	return writer.Write(r.contents)
}
