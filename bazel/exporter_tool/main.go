// Copyright 2022 Google LLC
//
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package main

import (
	"flag"
	"fmt"
	"os"

	"go.skia.org/infra/go/common"
	"go.skia.org/skia/bazel/exporter"
)

func main() {
	var (
		myFlags       = common.NewMultiStringFlag("rule", nil, "Bazel rule (may be repeated).")
		cmakeFileName = flag.String("out", "CMakeLists.txt", "CMake output file")
		projName      = flag.String("proj_name", "", "CMake project name")
	)
	flag.Parse()
	if *cmakeFileName == "" || *projName == "" {
		fmt.Fprintf(os.Stderr, "Usage of %s:\n", os.Args[0])
		flag.PrintDefaults()
		os.Exit(1)
	}
	workspaceDir, err := os.Getwd()
	if err != nil {
		fmt.Fprintf(os.Stderr, "Error getting pwd: %v", err)
		os.Exit(2)
	}
	qr := exporter.NewBazelQueryCommand(*myFlags, workspaceDir)
	exporter := exporter.NewCMakeExporter(workspaceDir)
	cmakeFile, err := os.Create(*cmakeFileName)
	if err != nil {
		fmt.Fprintf(os.Stderr, "Error opening %s: %v", *cmakeFileName, err)
		os.Exit(3)
	}
	defer cmakeFile.Close()
	err = exporter.Export(qr, cmakeFile, *projName)
	if err != nil {
		fmt.Fprintf(os.Stderr, "Error exporting to CMake: %v\n", err)
		os.Exit(4)
	}
}
