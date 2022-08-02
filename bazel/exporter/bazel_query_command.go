// Copyright 2022 Google LLC
//
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package exporter

import (
	"fmt"
	"os"
	"os/exec"

	"go.skia.org/infra/go/skerr"
	"go.skia.org/skia/bazel/exporter/interfaces"
)

// BazelQueryCommand implements the QueryCommand interface. It will
// execute the bazel executable to return all rules defined in
// a protocol buffer.
type BazelQueryCommand struct {
	ruleNames []string
	workspace string
}

// NewBazelQueryCommand will create a new BazelQueryCommand instance which will,
// when Read() is called, invoke the bazel executable to execute a cquery
// command in the provided workspace for the supplied rules.
func NewBazelQueryCommand(ruleNames []string, workspace string) *BazelQueryCommand {
	return &BazelQueryCommand{ruleNames: ruleNames, workspace: workspace}
}

// Stop the Bazel server if running.
func shutdownBazelServer() error {
	cmd := exec.Command("bazel", "shutdown")
	_, err := cmd.Output()
	return err
}

// Read will execute the Bazel cquery command, supplied to NewBazelQueryCommand(),
// and return the results.
func (c *BazelQueryCommand) Read() ([]byte, error) {
	if len(c.ruleNames) == 0 {
		return nil, skerr.Fmt("no query rules")
	}

	pwd, err := os.Getwd()
	if err != nil {
		return nil, skerr.Wrapf(err, `can't get working directory`)
	}
	err = os.Chdir(c.workspace)
	if err != nil {
		return nil, skerr.Wrapf(err, `can't set working directory to %q`, c.workspace)
	}
	// Shutdown the Bazel server to workaround a known issue with cquery:
	// See "Non-deterministic output" in https://bazel.build/docs/cquery#known-issues
	err = shutdownBazelServer()
	if err != nil {
		return nil, skerr.Wrap(err)
	}
	ruleArg := `kind("rule", `
	for i, r := range c.ruleNames {
		if i > 0 {
			ruleArg = ruleArg + " + "
		}
		ruleArg = ruleArg + fmt.Sprintf("deps(%s)", r)
	}
	ruleArg = ruleArg + ")"
	args := []string{"cquery", "--noimplicit_deps", ruleArg, "--output", "proto"}
	cmd := exec.Command("bazel", args...)
	_ = os.Chdir(pwd)
	data, err := cmd.Output()
	if err != nil {
		return nil, skerr.Wrapf(err, `error running %v`, cmd)
	}
	return data, nil
}

// Make sure BazelQueryCommand fulfills the QueryCommand interface.
var _ interfaces.QueryCommand = (*BazelQueryCommand)(nil)
