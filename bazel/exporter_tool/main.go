// Copyright 2022 Google LLC
//
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package main

import (
	"flag"
	"fmt"
	"os"
	"runtime/pprof"

	"go.skia.org/infra/go/common"
	"go.skia.org/infra/go/skerr"
	"go.skia.org/skia/bazel/exporter"
	"go.skia.org/skia/bazel/exporter/interfaces"
)

// fileListWriteOrder specifies the order of file lists to be written to a GNI file.
// In a *.gni file a file list is like:
//
//	file_list = [
//	  ...
//	]
//
// Skia clients reference some file lists when building Skia using GN. It is important
// that these lists continue to exist so as to not break customer builds and maintain
// backward compatibility.
var fileListWriteOrder = map[string][]string{
	"gn/core.gni": {
		"skia_core_public",
		"skia_core_sources",
		"skia_pathops_public",
		"skia_pathops_sources",
		"skia_precompile_public",
		"skia_precompile_sources",
		"skia_skpicture_public",
		"skia_skpicture_sources"},
	"gn/effects.gni": {
		"skia_effects_public",
		"skia_effects_sources"},
	/*
		TODO(skbug.com/12345): Add support for all *.gni files containing file lists.

		"gn/effects_imagefilters.gni": {
			"skia_effects_imagefilter_public",
			"skia_effects_imagefilter_sources"},
		"gn/gpu.gni": {
			"skia_gpu_sources",
			"skia_gl_gpu_sources",
			"skia_null_gpu_sources",
			"skia_gpu_sources",
			"skia_vk_sources",
			"skia_direct3d_sources",
			"skia_dawn_sources",
			"skia_metal_sources",
			"skia_native_gpu_sources",
			"skia_shared_gpu_sources"},
		"gn/pdf.gni": {
			"skia_pdf_public",
			"skia_pdf_sources"},
		"gn/sksl.gni": {
			"skia_sksl_sources",
			"skia_sksl_tracing_sources",
			"skia_sksl_gpu_sources",
			"skslc_deps"},
		"gn/utils.gni": {
			"skia_utils_public",
			"skia_utils_sources"},
	*/
}

const (
	unknownErr    = 1
	invalidArgErr = 2
	exportErr     = 3
	verifyErr     = 4
	profilerErr   = 5
)

type fileSystem struct {
	workspaceDir string
	outFormat    string
	openFiles    []*os.File
}

func (fs *fileSystem) OpenFile(path string) (interfaces.Writer, error) {
	f, err := os.Create(path)
	if err != nil {
		return nil, skerr.Wrap(err)
	}
	fs.openFiles = append(fs.openFiles, f)
	return f, nil
}

func (fs *fileSystem) ReadFile(filename string) ([]byte, error) {
	return os.ReadFile(filename)
}

func (fs *fileSystem) Shutdown() {
	for _, f := range fs.openFiles {
		f.Close() // Ignore error.
	}
}

// Make sure fileSystem fulfills the FileSystem interface.
var _ interfaces.FileSystem = (*fileSystem)(nil)

func createExporter(projName, cmakeFileName string, fs *fileSystem) interfaces.Exporter {
	if fs.outFormat == "cmake" {
		return exporter.NewCMakeExporter(projName, fs.workspaceDir, cmakeFileName, fs)
	}
	params := exporter.GNIExporterParams{
		WorkspaceDir: fs.workspaceDir,
		GNIFileVars:  fileListWriteOrder,
	}
	return exporter.NewGNIExporter(params, fs)
}

func doExport(qr interfaces.QueryCommand, exp interfaces.Exporter, outFormat string) {
	err := exp.Export(qr)
	if err != nil {
		fmt.Fprintf(os.Stderr, "Error exporting to %s: %v\n", outFormat, err)
		os.Exit(exportErr)
	}
}

func doCheckCurrent(qr interfaces.QueryCommand, exp interfaces.Exporter, outFormat string) {
	numOutOfDate, err := exp.CheckCurrent(qr, os.Stdout)
	if err != nil {
		fmt.Fprintf(os.Stderr, "Error running %s currency check: %v\n", outFormat, err)
		os.Exit(verifyErr)
	}
	if numOutOfDate > 0 {
		fmt.Fprintf(os.Stdout, "%d files are out of date\n", numOutOfDate)
		os.Exit(verifyErr)
	}
}

func main() {
	var (
		queryRules    = common.NewMultiStringFlag("rule", nil, "Bazel rule (may be repeated).")
		outFormat     = flag.String("output_format", "", "Desired output format. One of cmake or gni.")
		cmakeFileName = flag.String("out", "CMakeLists.txt", "CMake output file")
		projName      = flag.String("proj_name", "", "CMake project name")
		checkCurrent  = flag.Bool("check_current", false, "Identify any out-of-date target files")
		cpuprofile    = flag.String("cpuprofile", "", "write cpu profile to file")
	)
	flag.Parse()
	if *outFormat != "cmake" && *outFormat != "gni" {
		if *outFormat == "" {
			fmt.Fprintln(os.Stderr, "Output format required")
		} else {
			fmt.Fprintf(os.Stderr, "Incorrect output format: \"%s\"\n", *outFormat)
		}
		fmt.Fprintf(os.Stderr, "Usage of %s:\n", os.Args[0])
		flag.PrintDefaults()
		os.Exit(invalidArgErr)
	}
	if *cmakeFileName == "" || *projName == "" {
		fmt.Fprintf(os.Stderr, "Usage of %s:\n", os.Args[0])
		flag.PrintDefaults()
		os.Exit(invalidArgErr)
	}
	workspaceDir, err := os.Getwd()
	if err != nil {
		fmt.Fprintf(os.Stderr, "Error getting pwd: %v", err)
		os.Exit(unknownErr)
	}
	if *checkCurrent && *outFormat != "gni" {
		fmt.Fprintf(os.Stderr, "-check_current is only supported for gni output")
		flag.PrintDefaults()
		os.Exit(invalidArgErr)
	}
	if *cpuprofile != "" {
		f, err := os.Create(*cpuprofile)
		if err != nil {
			fmt.Fprintf(os.Stderr, "unable to create %q: %v\n", *cpuprofile, err)
			os.Exit(profilerErr)
		}
		defer f.Close()
		if err = pprof.StartCPUProfile(f); err != nil {
			fmt.Fprintf(os.Stderr, "error starting CPU profile: %v\n", err)
			os.Exit(profilerErr)
		}
		defer pprof.StopCPUProfile()
	}
	qr := exporter.NewBazelQueryCommand(*queryRules, workspaceDir)
	fs := fileSystem{workspaceDir: workspaceDir, outFormat: *outFormat}
	defer fs.Shutdown()
	var exp interfaces.Exporter = createExporter(*projName, *cmakeFileName, &fs)
	if *checkCurrent {
		doCheckCurrent(qr, exp, *outFormat)
	} else {
		doExport(qr, exp, *outFormat)
	}
}
