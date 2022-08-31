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

var gniExportDescs = []exporter.GNIExportDesc{
	{GNI: "gn/core.gni", Vars: []exporter.GNIFileListExportDesc{
		{Var: "skia_core_public",
			Rules: []string{"//include/core:public_hdrs"}},
		{Var: "skia_core_sources",
			Rules: []string{
				"//include/private:private_hdrs",
				"//include/private/chromium:private_hdrs",
				"//src/core:core_hdrs",
				"//src/core:core_srcs",
				"//src/core:sksl_hdrs",
				"//src/core:sksl_srcs",
				"//src/image:core_hdrs",
				"//src/image:core_srcs",
				"//src/lazy:lazy_hdrs",
				"//src/lazy:lazy_srcs",
				"//src/opts:private_hdrs",
				"//src/shaders:shader_hdrs",
				"//src/shaders:shader_srcs",
				"//src/text:text_hdrs",
				"//src/text:text_srcs",
			}},
		{Var: "skia_pathops_public",
			Rules: []string{"//include/pathops:public_hdrs"}},
		{Var: "skia_pathops_sources",
			Rules: []string{
				"//src/pathops:pathops_hdrs",
				"//src/pathops:pathops_srcs",
			}},
		{Var: "skia_precompile_public",
			Rules: []string{"//include/core:precompile_public_hdrs"}},
		{Var: "skia_precompile_sources",
			Rules: []string{
				"//src/core:precompile_hdrs",
				"//src/core:precompile_srcs",
			}},
		{Var: "skia_skpicture_public",
			Rules: []string{"//include/core:skpicture_public_hdrs"}},
		{Var: "skia_skpicture_sources",
			Rules: []string{
				"//src/core:skpicture_hdrs",
				"//src/core:skpicture_srcs",
				"//src/shaders:skpicture_srcs",
			}},
		{Var: "src_images_srcs",
			Rules: []string{"//src/images:srcs"}}},
	},
	{GNI: "gn/effects.gni", Vars: []exporter.GNIFileListExportDesc{
		{Var: "skia_effects_public",
			Rules: []string{
				"//include/effects:public_hdrs",
			}},
		{Var: "skia_effects_sources",
			Rules: []string{
				"//src/effects:effects_hdrs",
				"//src/effects:effects_srcs",
				"//src/shaders/gradients:gradient_hdrs",
				"//src/shaders/gradients:gradient_srcs",
			}}},
	},
	{GNI: "gn/effects_imagefilters.gni", Vars: []exporter.GNIFileListExportDesc{
		{Var: "skia_effects_imagefilter_public",
			Rules: []string{
				"//include/effects:public_imagefilters_hdrs",
			}},
		{Var: "skia_effects_imagefilter_sources",
			Rules: []string{
				"//src/effects/imagefilters:imagefilters_srcs",
			}}},
	},
	{GNI: "gn/pdf.gni", Vars: []exporter.GNIFileListExportDesc{
		{Var: "skia_pdf_public",
			Rules: []string{"//include/docs:public_hdrs"}},
		{Var: "skia_pdf_sources",
			Rules: []string{
				"//src/pdf:pdf_hdrs",
				"//src/pdf:pdf_srcs",
			}}},
	},
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
		ExportDescs:  gniExportDescs,
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
