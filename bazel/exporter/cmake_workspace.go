// Copyright 2022 Google LLC
//
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package exporter

import (
	"fmt"
	"io"
	"sort"

	"go.skia.org/infra/go/skerr"
	"go.skia.org/infra/go/util"
	"go.skia.org/skia/bazel/exporter/build_proto/build"
)

// cmakeWorkspace represents the entire state of a CMake project.
type cmakeWorkspace struct {
	// Map Bazel rule names to cmakeRule instances. Holding pointer
	// values as the rules are mutable.
	rules map[string]*cmakeRule
}

// writeState tracks the state of an in-progress write of the workspace.
type writeState struct {
	writtenRules []string // All cmakeRule's written during write.
}

// newCMakeWorkspace will create a new CMake workspace object.
func newCMakeWorkspace() *cmakeWorkspace {
	return &cmakeWorkspace{rules: map[string]*cmakeRule{}}
}

// Determine if a rule has been written.
func (s *writeState) isRuleWritten(ruleName string) bool {
	return util.In(ruleName, s.writtenRules)
}

// setRuleWritten will mark the rule as having been written.
func (s *writeState) setRuleWritten(ruleName string) {
	s.writtenRules = append(s.writtenRules, ruleName)
}

// getRule will return the CMake wrapper for a Bazel rule given the
// rule name. Will return nil if there is no corresponding rule.
func (w *cmakeWorkspace) getRule(name string) *cmakeRule {
	return w.rules[name]
}

// createRule will create (if necessary) a new CMake rule object
// for the given rule name.
func (w *cmakeWorkspace) createRule(rule *build.Rule) *cmakeRule {
	if r := w.getRule(rule.GetName()); r != nil {
		return r
	}
	r := newCMakeRule(rule)
	w.rules[rule.GetName()] = r
	return r
}

// writeRule will write the given rule to the writer.
// It will first write all dependent rules so that they appear
// in the CMake project file before the rule that depends on them.
func (w *cmakeWorkspace) writeRule(writer io.Writer, r *cmakeRule, state *writeState) (int, error) {
	nb := 0
	if !r.hasSrcs() {
		return 0, nil
	}
	// First write all dependencies because CMake does not support forward references.
	for _, name := range r.deps {
		dep := w.getRule(name)
		if dep == nil {
			return 0, skerr.Fmt(`cannot find rule %q`, name)
		}
		n, err := w.writeRule(writer, dep, state)
		if err != nil {
			return nb, skerr.Wrap(err)
		}
		nb += n
	}
	if state.isRuleWritten(r.getName()) {
		return nb, nil
	}
	num, err := fmt.Fprintln(writer)
	if err != nil {
		return nb, skerr.Wrap(err)
	}
	nb += num
	num, err = r.write(writer)
	if err != nil {
		return nb, skerr.Wrap(err)
	}
	state.setRuleWritten(r.getName())
	nb += num
	return nb, nil
}

// Write this workspace using the given writer.
func (w *cmakeWorkspace) write(writer io.Writer) (int, error) {
	// Sort rule names to ensure a deterministic output.
	var sortedRuleNames []string
	for name := range w.rules {
		sortedRuleNames = append(sortedRuleNames, name)
	}
	sort.Strings(sortedRuleNames)

	var state writeState
	nb := 0
	for _, name := range sortedRuleNames {
		r := w.rules[name]
		num, err := w.writeRule(writer, r, &state)
		if err != nil {
			return nb, skerr.Wrap(err)
		}
		nb += num
	}
	return nb, nil
}
