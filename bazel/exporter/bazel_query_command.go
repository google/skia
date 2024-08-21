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
	queryType string // "query" or "cquery"
}

// A list of all Skia Bazel build flags which enable the building and/or
// exposure of all source files.
var allSkiaFlags = []string{
	"--ck_enable_canvas_polyfill",
	"--ck_enable_embedded_font",
	"--ck_enable_fonts",
	"--ck_enable_matrix_js",
	"--ck_enable_runtime_effect",
	"--ck_enable_skottie",
	"--ck_enable_skp_serialization",
}

// NewBazelQueryCommand will create a new BazelQueryCommand instance which will,
// when Read() is called, invoke the bazel executable to execute a cquery
// command in the provided workspace for the supplied rules.
func NewBazelCMakeQueryCommand(ruleNames []string, workspace string) *BazelQueryCommand {
	return &BazelQueryCommand{ruleNames: ruleNames, workspace: workspace, queryType: "cquery"}
}

// NewBazelQueryCommand will create a new BazelQueryCommand instance which will,
// when Read() is called, invoke the bazel executable to execute a query
// command in the provided workspace for the supplied rules.
func NewBazelGNIQueryCommand(ruleNames []string, workspace string) *BazelQueryCommand {
	return &BazelQueryCommand{ruleNames: ruleNames, workspace: workspace, queryType: "query"}
}

// Stop the Bazel server if running.
func shutdownBazelServer() error {
	cmd := exec.Command("bazelisk", "shutdown")
	_, err := cmd.Output()
	return err
}

// Read will execute the Bazel query/cquery command, supplied to NewBazel*QueryCommand(),
// and return the binary protobuf results.
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
	if c.queryType == "cquery" {
		// Shutdown the Bazel server to workaround a known issue with cquery:
		// See "Non-deterministic output" in https://bazel.build/docs/cquery#known-issues
		err = shutdownBazelServer()
		if err != nil {
			return nil, skerr.Wrap(err)
		}
	}
	ruleArg := `kind("rule", `
	for i, r := range c.ruleNames {
		if i > 0 {
			ruleArg = ruleArg + " + "
		}
		ruleArg = ruleArg + fmt.Sprintf("deps(%s)", r)
	}
	ruleArg = ruleArg + ")"
	args := []string{c.queryType, "--noimplicit_deps", ruleArg, "--output", "proto"}
	if c.queryType == "cquery" {
		args = append(args, allSkiaFlags...)
	}
	cmd := exec.Command("bazelisk", args...)
	_ = os.Chdir(pwd)
	data, err := cmd.Output()
	if err != nil {
		if exiterr, ok := err.(*exec.ExitError); ok {
			fmt.Printf("Stderr: %s\n", exiterr.Stderr)
		}
		return nil, skerr.Wrapf(err, `error running %v`, cmd)
	}
	return data, nil
}

// Make sure BazelQueryCommand fulfills the QueryCommand interface.
var _ interfaces.QueryCommand = (*BazelQueryCommand)(nil)
