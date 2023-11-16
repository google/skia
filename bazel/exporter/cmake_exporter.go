// Copyright 2022 Google LLC
//
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package exporter

import (
	"bytes"
	"fmt"
	"path/filepath"
	"strings"

	"go.skia.org/infra/go/skerr"
	"go.skia.org/skia/bazel/exporter/build_proto/analysis_v2"
	"go.skia.org/skia/bazel/exporter/build_proto/build"
	"go.skia.org/skia/bazel/exporter/interfaces"
	"google.golang.org/protobuf/proto"
)

type CMakeExporter struct {
	projName     string
	workspace    cmakeWorkspace
	workspaceDir string // Absolute path to Bazel workspace directory.
	cmakeFile    string // Absolute path to CMake output file.
	fs           interfaces.FileSystem
}

// NewCMakeExporter creates an exporter that will export a Bazel project
// query from a project in the workspaceDir to a CMake project file identified
// by cmakeFile.
//
// Note: cmakeFile must be an absolute path.
func NewCMakeExporter(projName, workspaceDir, cmakeFile string, fs interfaces.FileSystem) *CMakeExporter {
	return &CMakeExporter{
		workspace:    *newCMakeWorkspace(),
		projName:     projName,
		workspaceDir: workspaceDir,
		cmakeFile:    cmakeFile,
		fs:           fs,
	}
}

// Return the default copts (COMPILE_FLAGS in CMake) for the macOS toolchain.
func getMacPlatformRuleCopts() []string {
	// TODO(crbug.com/skia/13586): Retrieve these values from Bazel.
	// These values must match those values defined in mac_toolchain_config.bzl
	return []string{
		// These items are from _make_default_flags().
		"-std=c++17",
		"-Wno-psabi",

		// From _make_target_specific_flags.
		"--target=arm64-apple-macos11",
	}
}

// Return the default copts (COMPILE_FLAGS in CMake) for the Linux toolchain.
func getLinuxPlatformRuleCopts() []string {
	// TODO(crbug.com/skia/13586): Retrieve these values from Bazel.
	return []string{
		// These items are from _make_default_flags().
		"-std=c++17",
		"-Wno-psabi",

		// Added to avoid compile warning.
		"-Wno-attributes",
	}
}

// Write the CMake project config to set the COMPILE_FLAGS
// variables for all platforms.
func writePlatformCompileFlags(writer interfaces.Writer) {
	val := strings.Join(getMacPlatformRuleCopts(), " ")
	_, _ = fmt.Fprintf(writer, "set(DEFAULT_COMPILE_FLAGS_MACOS %q)\n", val)

	val = strings.Join(getLinuxPlatformRuleCopts(), " ")
	_, _ = fmt.Fprintf(writer, "set(DEFAULT_COMPILE_FLAGS_LINUX %q)\n", val)
	_, _ = writer.WriteString("\n")
	_, _ = fmt.Fprintln(writer, `if (APPLE)`)
	_, _ = fmt.Fprintln(writer, `  set(DEFAULT_COMPILE_FLAGS "${DEFAULT_COMPILE_FLAGS_MACOS}")`)
	_, _ = fmt.Fprintln(writer, `else()`)
	_, _ = fmt.Fprintln(writer, `  set(DEFAULT_COMPILE_FLAGS "${DEFAULT_COMPILE_FLAGS_LINUX}")`)
	_, _ = fmt.Fprintln(writer, `endif()`)
}

// Return the copts rule attribute for the given rule.
func getRuleCopts(r *build.Rule) ([]string, error) {
	ruleOpts, err := getRuleStringArrayAttribute(r, "copts")
	if err != nil {
		return nil, skerr.Wrap(err)
	}
	copts := []string{"${DEFAULT_COMPILE_FLAGS}"}
	return appendUnique(copts, ruleOpts...), nil
}

// Return the include paths for the supplied rule and all rules on which
// this rule depends.
//
// Note: All rules are absolute paths.
func getRuleIncludes(r *build.Rule, qr *analysis_v2.CqueryResult) ([]string, error) {
	deps, err := getRuleStringArrayAttribute(r, "deps")
	if err != nil {
		return nil, skerr.Wrap(err)
	}
	includes, err := getRuleStringArrayAttribute(r, "includes")
	if err != nil {
		return nil, skerr.Wrap(err)
	}
	ruleDir, err := getLocationDir(r.GetLocation())
	if err != nil {
		return nil, skerr.Wrap(err)
	}
	for idx, inc := range includes {
		if inc == "." {
			includes[idx] = ruleDir
		}
	}
	for _, d := range deps {
		dr := findRule(qr, d)
		if dr == nil {
			return nil, skerr.Fmt("cannot find rule %s", d)
		}
		if isExternalRule(dr.GetName()) {
			continue
		}
		incs, err := getRuleIncludes(dr, qr)
		if err != nil {
			return nil, skerr.Wrap(err)
		}
		includes = appendUnique(includes, incs...)
	}
	return includes, nil
}

// Return the deps for the supplied rule and all rules on which
// this rule depends.
func getRuleDefines(r *build.Rule, qr *analysis_v2.CqueryResult) ([]string, error) {
	deps, err := getRuleStringArrayAttribute(r, "deps")
	if err != nil {
		return nil, skerr.Wrap(err)
	}
	defines, err := getRuleStringArrayAttribute(r, "defines")
	if err != nil {
		return nil, skerr.Wrap(err)
	}
	for _, d := range deps {
		dr := findRule(qr, d)
		if dr == nil {
			return nil, skerr.Fmt("cannot find rule %s", d)
		}
		defs, err := getRuleDefines(dr, qr)
		if err != nil {
			return nil, skerr.Wrap(err)
		}
		defines = appendUnique(defines, defs...)
	}
	return defines, nil
}

// Convert an absolute path to a file *within the workspace* to a
// workspace relative path. All paths start with ${CMAKE_SOURCE_DIR}.
func (e *CMakeExporter) absToWorkspaceRelativePath(absPath string) string {
	if absPath == e.workspaceDir {
		return "${CMAKE_SOURCE_DIR}"
	}
	return fmt.Sprintf("${CMAKE_SOURCE_DIR}/%s", absPath[len(e.workspaceDir)+1:])
}

// Write the list of items (which may be rules or files) to the supplied buffer.
func (e *CMakeExporter) writeItems(r *cmakeRule, projectDir string, items []string, buffer *bytes.Buffer) error {
	for _, item := range items {
		if isFileTarget(item) {
			_, _, target, err := parseRule(item)
			if err != nil {
				return skerr.Wrap(err)
			}
			absPath := filepath.Join(projectDir, target)
			_, _ = fmt.Fprintf(buffer, "    %q\n", e.absToWorkspaceRelativePath(absPath))
		} else {
			cmakeName, err := getRuleSimpleName(item)
			if err != nil {
				return skerr.Wrap(err)
			}
			_, _ = fmt.Fprintf(buffer, "    ${%s}\n", cmakeName)
			err = r.addDependency(item)
			if err != nil {
				return skerr.Wrap(err)
			}
		}
	}
	return nil
}

// Write the "srcs" and "hdrs" rule attributes to the supplied buffer.
func (e *CMakeExporter) writeSrcsAndHdrs(rule *cmakeRule, buffer *bytes.Buffer, r *build.Rule) error {
	ruleDir, err := getLocationDir(r.GetLocation())
	if err != nil {
		return skerr.Wrap(err)
	}
	for _, attrib := range r.Attribute {
		if attrib.GetName() == "srcs" {
			if attrib.GetType() != build.Attribute_LABEL_LIST {
				return skerr.Fmt(`srcs in rule %q is not a list`, r.GetName())
			}
			fmt.Fprintln(buffer, "    # Sources:")
			err := e.writeItems(rule, ruleDir, attrib.GetStringListValue(), buffer)
			if err != nil {
				return skerr.Wrap(err)
			}
		}
		if attrib.GetName() == "hdrs" {
			if attrib.GetType() != build.Attribute_LABEL_LIST {
				return skerr.Fmt(`hdrs in rule %q is not a list`, r.GetName())
			}
			fmt.Fprintln(buffer, "    # Headers:")
			err := e.writeItems(rule, ruleDir, attrib.GetStringListValue(), buffer)
			if err != nil {
				return skerr.Wrap(err)
			}
		}
	}
	return nil
}

// Write the target COMPILE_FLAGS property to the supplied buffer (if there are any copts).
func (e *CMakeExporter) writeCompileFlags(r *build.Rule, buffer *bytes.Buffer) error {
	copts, err := getRuleCopts(r)
	if err != nil {
		return skerr.Wrap(err)
	}
	if len(copts) == 0 {
		// No error, just nothing to write.
		return nil
	}
	str := strings.Join(copts, " ")
	cmakeName, err := getRuleSimpleName(r.GetName())
	if err != nil {
		return skerr.Wrap(err)
	}
	_, err = fmt.Fprintf(buffer, "set_target_properties(%s PROPERTIES COMPILE_FLAGS\n  %q\n)\n",
		cmakeName, str)
	return err
}

// Write the target COMPILE_DEFINITIONS property to the supplied buffer (if there are any defines).
func (e *CMakeExporter) writeCompileDefinitions(r *build.Rule, qr *analysis_v2.CqueryResult, buffer *bytes.Buffer) error {
	defines, err := getRuleDefines(r, qr)
	if err != nil {
		return skerr.Wrap(err)
	}
	if len(defines) == 0 {
		// No error, just nothing to write.
		return nil
	}
	str := strings.Join(defines, ";")
	cmakeName, err := getRuleSimpleName(r.GetName())
	if err != nil {
		return skerr.Wrap(err)
	}
	_, err = fmt.Fprintf(buffer, "set_target_properties(%s PROPERTIES COMPILE_DEFINITIONS\n  %q\n)\n", cmakeName, str)
	return err
}

// Write the target INCLUDE_DIRECTORIES property to the supplied buffer (if there are any).
func (e *CMakeExporter) writeIncludeDirectories(r *build.Rule, qr *analysis_v2.CqueryResult, buffer *bytes.Buffer) error {
	includes, err := getRuleIncludes(r, qr)
	if err != nil {
		return skerr.Wrap(err)
	}
	includes = appendUnique(includes, e.workspaceDir)
	for i, path := range includes {
		includes[i] = e.absToWorkspaceRelativePath(path)
	}
	str := strings.Join(includes, ";")
	cmakeName, err := getRuleSimpleName(r.GetName())
	if err != nil {
		return skerr.Wrap(err)
	}
	_, err = fmt.Fprintf(buffer, "set_target_properties(%s PROPERTIES INCLUDE_DIRECTORIES\n  %q\n)\n", cmakeName, str)
	return err
}

// Write the target LINK_FLAGS property to the supplied buffer (if there are any linkopts).
func (e *CMakeExporter) writeLinkFlags(r *build.Rule, buffer *bytes.Buffer) error {
	defines, err := getRuleStringArrayAttribute(r, "linkopts")
	if err != nil {
		return skerr.Wrap(err)
	}
	if len(defines) == 0 {
		// No error, just nothing to write.
		return nil
	}
	str := strings.Join(defines, " ")
	cmakeName, err := getRuleSimpleName(r.GetName())
	if err != nil {
		return skerr.Wrap(err)
	}
	_, err = fmt.Fprintf(buffer, "set_target_properties(%s PROPERTIES LINK_FLAGS\n  %q\n)\n", cmakeName, str)
	return err
}

// Write all target properties to the supplied buffer.
func (e *CMakeExporter) writeProperties(r *build.Rule, qr *analysis_v2.CqueryResult, buffer *bytes.Buffer) error {
	err := e.writeCompileFlags(r, buffer)
	if err != nil {
		return skerr.Wrap(err)
	}
	err = e.writeLinkFlags(r, buffer)
	if err != nil {
		return skerr.Wrap(err)
	}
	err = e.writeCompileDefinitions(r, qr, buffer)
	if err != nil {
		return skerr.Wrap(err)
	}
	err = e.writeIncludeDirectories(r, qr, buffer)
	if err != nil {
		return skerr.Wrap(err)
	}
	return nil
}

// Convert the filegroup rule to the CMake equivalent.
func (e *CMakeExporter) convertFilegroupRule(r *build.Rule) error {

	rule := e.workspace.createRule(r)

	var contents bytes.Buffer

	targetName := r.GetName()
	variableName, err := getRuleSimpleName(r.GetName())
	if err != nil {
		return skerr.Wrap(err)
	}
	_, _ = fmt.Fprintf(&contents, "# %s\n", targetName)
	_, _ = fmt.Fprintf(&contents, "list(APPEND %s\n", variableName)

	err = e.writeSrcsAndHdrs(rule, &contents, r)
	if err != nil {
		return skerr.Wrap(err)
	}
	fmt.Fprintln(&contents, ")")
	rule.setContents(contents.Bytes())

	return nil
}

// Convert the cc_binary rule to the CMake equivalent.
func (e *CMakeExporter) convertCCBinaryRule(r *build.Rule, qr *analysis_v2.CqueryResult) error {

	rule := e.workspace.createRule(r)

	targetName := r.GetName()
	var contents bytes.Buffer
	_, _ = fmt.Fprintf(&contents, "# %s\n", targetName)
	cmakeName, err := getRuleSimpleName(r.GetName())
	if err != nil {
		return skerr.Wrap(err)
	}
	_, _ = fmt.Fprintf(&contents, "add_executable(%s \"\")\n", cmakeName)
	_, _ = fmt.Fprintf(&contents, "target_sources(%s\n", cmakeName)
	fmt.Fprintln(&contents, "  PRIVATE")

	err = e.writeSrcsAndHdrs(rule, &contents, r)
	if err != nil {
		return skerr.Wrap(err)
	}

	fmt.Fprintln(&contents, ")")
	err = e.writeProperties(r, qr, &contents)
	if err != nil {
		return skerr.Wrap(err)
	}
	rule.setContents(contents.Bytes())

	return nil
}

// Convert the cc_library rule to the CMake equivalent.
func (e *CMakeExporter) convertCCLibraryRule(r *build.Rule, qr *analysis_v2.CqueryResult) error {

	rule := e.workspace.createRule(r)

	targetName := r.GetName()
	cmakeName, err := getRuleSimpleName(r.GetName())
	if err != nil {
		return skerr.Wrap(err)
	}
	var contents bytes.Buffer
	_, _ = fmt.Fprintf(&contents, "# %s\n", targetName)
	_, _ = fmt.Fprintf(&contents, "add_library(%s \"\")\n", cmakeName)
	_, _ = fmt.Fprintf(&contents, "target_sources(%s\n", cmakeName)
	fmt.Fprintln(&contents, "  PRIVATE")

	err = e.writeSrcsAndHdrs(rule, &contents, r)
	if err != nil {
		return skerr.Wrap(err)
	}
	fmt.Fprintln(&contents, ")")
	err = e.writeProperties(r, qr, &contents)
	if err != nil {
		return skerr.Wrap(err)
	}

	rule.setContents(contents.Bytes())

	return nil
}

// Export will convert the input Bazel cquery output, provided by the
// supplied QueryCommand parameter, to CMake. The equivalent
// CMake project definition will be written using the writer provided
// to the constructor method.
func (e *CMakeExporter) Export(qcmd interfaces.QueryCommand) error {

	in, err := qcmd.Read()
	if err != nil {
		return skerr.Wrapf(err, "error reading Bazel cquery data")
	}
	qr := analysis_v2.CqueryResult{}
	if err := proto.Unmarshal(in, &qr); err != nil {
		return skerr.Wrapf(err, "failed to unmarshal Bazel cquery result")
	}

	writer, err := e.fs.OpenFile(e.cmakeFile)
	if err != nil {
		return skerr.Wrap(err)
	}
	_, _ = fmt.Fprintln(writer, "# DO NOT EDIT: This file is auto-generated.")
	_, _ = fmt.Fprintln(writer, "cmake_minimum_required(VERSION 3.13)")
	_, _ = writer.WriteString("\n")
	_, _ = fmt.Fprintf(writer, "project(%s LANGUAGES C CXX)\n", e.projName)
	_, _ = writer.WriteString("\n")

	writePlatformCompileFlags(writer)
	_, _ = writer.WriteString("\n")

	for _, result := range qr.GetResults() {
		t := result.GetTarget()
		r := t.GetRule()
		if isExternalRule(r.GetName()) {
			continue
		}
		var err error = nil
		switch {
		case r.GetRuleClass() == "cc_binary":
			err = e.convertCCBinaryRule(r, &qr)
		case r.GetRuleClass() == "cc_library":
			err = e.convertCCLibraryRule(r, &qr)
		case r.GetRuleClass() == "filegroup":
			err = e.convertFilegroupRule(r)
		}
		if err != nil {
			return skerr.Wrapf(err, "failed to convert %s", r.GetRuleClass())
		}
	}

	_, err = e.workspace.write(writer)
	if err != nil {
		return skerr.Wrap(err)
	}

	return nil
}

// Make sure CMakeExporter fulfills the Exporter interface.
var _ interfaces.Exporter = (*CMakeExporter)(nil)
