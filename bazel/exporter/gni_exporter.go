// Copyright 2022 Google LLC
//
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package exporter

import (
	"bufio"
	"bytes"
	"fmt"
	"path/filepath"
	"regexp"
	"sort"
	"strings"

	"go.skia.org/infra/go/skerr"
	"go.skia.org/infra/go/util"
	"go.skia.org/skia/bazel/exporter/build_proto/analysis_v2"
	"go.skia.org/skia/bazel/exporter/build_proto/build"
	"go.skia.org/skia/bazel/exporter/interfaces"
	"google.golang.org/protobuf/proto"
)

// The contents (or partial contents) of a GNI file.
type gniFileContents struct {
	hasSrcs     bool   // Has at least one file in $_src/ dir?
	hasIncludes bool   // Has at least one file in $_include/ dir?
	hasModules  bool   // Has at least one file in $_module/ dir?
	data        []byte // The file contents to be written.
}

// GNIFileListExportDesc contains a description of the data that
// will comprise a GN file list variable when written to a *.gni file.
type GNIFileListExportDesc struct {
	// The file list variable name to use in the exported *.gni file.
	// In the *.gni file this will look like:
	//   var_name = [ ... ]
	Var string
	// The Bazel rule name(s) to export into the file list.
	Rules []string
}

// GNIExportDesc defines a GNI file to be exported, the rules to be
// exported, and the file list variable names in which to list the
// rule files.
type GNIExportDesc struct {
	GNI  string                  // The export destination *.gni file path (relative to workspace).
	Vars []GNIFileListExportDesc // List of GNI file list variable rules.
}

// GNIExporterParams contains the construction parameters when
// creating a new GNIExporter via NewGNIExporter().
type GNIExporterParams struct {
	WorkspaceDir string          // The Bazel workspace directory path.
	ExportDescs  []GNIExportDesc // The Bazel rules to export.
}

// GNIExporter is an object responsible for exporting rules defined in a
// Bazel workspace to file lists in GNI format (GN's *.gni files). This exporter
// is tightly coupled to the Skia Bazel rules and GNI configuration.
type GNIExporter struct {
	workspaceDir   string                // The Bazel workspace path.
	fs             interfaces.FileSystem // For filesystem interactions.
	exportGNIDescs []GNIExportDesc       // The rules to export.
}

// Skia source files which are deprecated. These are omitted from
// *.gni files during export because the skia.h file is a generated
// file and it cannot include deprecated files without breaking
// clients that include it.
var deprecatedFiles = []string{
	"include/core/SkDrawLooper.h",
	"include/effects/SkBlurDrawLooper.h",
	"include/effects/SkLayerDrawLooper.h",
}

// The footer written to core.gni.
const coreGNIFooter = `skia_core_sources += skia_pathops_sources
skia_core_sources += skia_skpicture_sources

skia_core_public += skia_pathops_public
skia_core_public += skia_skpicture_public`

// Map of GNI file names to footer text to be appended to the end of the file.
var footerMap = map[string]string{
	"gn/core.gni": coreGNIFooter,
}

// Match variable definition of a list in a *.gni file. For example:
//
//	foo = []
//
// will match "foo"
var gniVariableDefReg = regexp.MustCompile(`^(\w+)\s?=\s?\[`)

// NewGNIExporter creates an exporter that will export to GN's (*.gni) files.
func NewGNIExporter(params GNIExporterParams, filesystem interfaces.FileSystem) *GNIExporter {
	e := &GNIExporter{
		workspaceDir:   params.WorkspaceDir,
		fs:             filesystem,
		exportGNIDescs: params.ExportDescs,
	}
	return e
}

// Retrieve all rule attributes which are internal file targets.
func getRuleFiles(r *build.Rule, attrName string) ([]string, error) {
	items, err := getRuleStringArrayAttribute(r, attrName)
	if err != nil {
		return nil, skerr.Wrap(err)
	}

	var files []string
	for _, item := range items {
		if !isExternalRule(item) && isFileTarget(item) {
			files = append(files, item)
		}
	}
	return files, nil
}

// Convert a file path into a workspace relative path using variables to
// specify the base folder. The variables are one of $_src, $_include, or $_modules.
func makeRelativeFilePathForGNI(path string) (string, error) {
	if strings.HasPrefix(path, "src/") {
		return "$_src/" + strings.TrimPrefix(path, "src/"), nil
	}
	if strings.HasPrefix(path, "include/") {
		return "$_include/" + strings.TrimPrefix(path, "include/"), nil
	}
	if strings.HasPrefix(path, "modules/") {
		return "$_modules/" + strings.TrimPrefix(path, "modules/"), nil
	}

	return "", skerr.Fmt("can't find path for %q\n", path)
}

// Convert a slice of workspace relative paths into a new slice containing
// GNI variables ($_src, $_include, etc.). *All* paths in the supplied
// slice must be a supported top-level directory.
func addGNIVariablesToWorkspacePaths(paths []string) ([]string, error) {
	vars := make([]string, 0, len(paths))
	for _, path := range paths {
		withVar, err := makeRelativeFilePathForGNI(path)
		if err != nil {
			return nil, skerr.Wrap(err)
		}
		vars = append(vars, withVar)
	}
	return vars, nil
}

// Is the file path a C++ header?
func isHeaderFile(path string) bool {
	ext := strings.ToLower(filepath.Ext(path))
	return ext == ".h" || ext == ".hpp"
}

// Does the list of file paths contain only header files?
func fileListContainsOnlyCppHeaderFiles(files []string) bool {
	for _, f := range files {
		if !isHeaderFile(f) {
			return false
		}
	}
	return len(files) > 0 // Empty list is false, else all are headers.
}

// Write the *.gni file header.
func writeGNFileHeader(writer interfaces.Writer, gniFile *gniFileContents) {
	fmt.Fprintln(writer, "# DO NOT EDIT: This is a generated file.")
	writer.WriteString("\n")
	if gniFile.hasSrcs {
		fmt.Fprintln(writer, `_src = get_path_info("../src", "abspath")`)
	}
	if gniFile.hasIncludes {
		fmt.Fprintln(writer, `_include = get_path_info("../include", "abspath")`)
	}
	if gniFile.hasModules {
		fmt.Fprintln(writer, `_modules = get_path_info("../modules", "abspath")`)
	}
}

// Find the first duplicated file in a sorted list of file paths.
// The file paths are case insensitive.
func findDuplicate(files []string) (path string, hasDuplicate bool) {
	for i, e := range files {
		if i == len(files)-1 {
			continue
		}
		if strings.EqualFold(e, files[i+1]) {
			return e, true
		}
	}
	return "", false
}

// Retrieve all sources ("srcs" attribute) and headers ("hdrs" attribute)
// and return as a single slice of target names. Slice entries will be
// something like:
//
//	"//src/core/file.cpp".
func getSrcsAndHdrs(r *build.Rule) ([]string, error) {
	srcs, err := getRuleFiles(r, "srcs")
	if err != nil {
		return nil, skerr.Wrap(err)
	}

	hdrs, err := getRuleFiles(r, "hdrs")
	if err != nil {
		return nil, skerr.Wrap(err)
	}
	return append(srcs, hdrs...), nil
}

// Convert a slice of file path targets to workspace relative file paths.
// i.e. convert each element like:
//
//	"//src/core/file.cpp"
//
// into:
//
//	"src/core/file.cpp"
func convertTargetsToFilePaths(targets []string) ([]string, error) {
	paths := make([]string, 0, len(targets))
	for _, target := range targets {
		path, err := getFilePathFromFileTarget(target)
		if err != nil {
			return nil, skerr.Wrap(err)
		}
		paths = append(paths, path)
	}
	return paths, nil
}

// Is the source file deprecated? i.e. should the file be exported to projects
// generated by this package?
func isSourceFileDeprecated(workspacePath string) bool {
	return util.In(workspacePath, deprecatedFiles)
}

// Filter all deprecated files from the |files| slice, returning a new slice
// containing no deprecated files. All paths in |files| must be workspace-relative
// paths.
func filterDeprecatedFiles(files []string) []string {
	filtered := make([]string, 0, len(files))
	for _, path := range files {
		if !isSourceFileDeprecated(path) {
			filtered = append(filtered, path)
		}
	}
	return filtered
}

// Return the top-level component (directory or file) of a relative file path.
// The paths are assumed to be delimited by forward slash (/) characters (even on Windows).
// An empty string is returned if no top level folder can be found.
//
// Example:
//
//	"foo/bar/baz.txt" returns "foo"
func extractTopLevelFolder(path string) string {
	parts := strings.Split(path, "/")
	if len(parts) > 0 {
		return parts[0]
	}
	return ""
}

// Extract the name of a variable assignment from a line of text from a GNI file.
// So, a line like:
//
//	"foo = [...]"
//
// will return:
//
//	"foo"
func getGNILineVariable(line string) string {
	if matches := gniVariableDefReg.FindStringSubmatch(line); matches != nil {
		return matches[1]
	}
	return ""
}

// Given a workspace relative path return an absolute path.
func (e *GNIExporter) workspaceToAbsPath(wsPath string) string {
	if filepath.IsAbs(wsPath) {
		panic("filepath already absolute")
	}
	return filepath.Join(e.workspaceDir, wsPath)
}

// Merge the another file contents object into this one.
func (c *gniFileContents) merge(other gniFileContents) {
	if other.hasIncludes {
		c.hasIncludes = true
	}
	if other.hasModules {
		c.hasModules = true
	}
	if other.hasSrcs {
		c.hasSrcs = true
	}
	c.data = append(c.data, other.data...)
}

// Convert all rules that go into a GNI file list.
func (e *GNIExporter) convertGNIFileList(desc GNIFileListExportDesc, qr *analysis_v2.CqueryResult) (gniFileContents, error) {
	var targets []string
	for _, ruleName := range desc.Rules {
		r := findRule(qr, ruleName)
		if r == nil {
			return gniFileContents{}, skerr.Fmt("Cannot find rule %s", ruleName)
		}
		t, err := getSrcsAndHdrs(r)
		if err != nil {
			return gniFileContents{}, skerr.Wrap(err)
		}
		if len(t) == 0 {
			return gniFileContents{}, skerr.Fmt("No files to export in rule %s", ruleName)
		}
		targets = append(targets, t...)
	}

	files, err := convertTargetsToFilePaths(targets)
	if err != nil {
		return gniFileContents{}, skerr.Wrap(err)
	}

	files = filterDeprecatedFiles(files)

	files, err = addGNIVariablesToWorkspacePaths(files)
	if err != nil {
		return gniFileContents{}, skerr.Wrap(err)
	}

	sort.Slice(files, func(i, j int) bool {
		// Generally sort alphabetically, but make $_include/ after $_src.
		isfx := extractTopLevelFolder(files[i])
		jsfx := extractTopLevelFolder(files[j])
		if isfx == jsfx {
			return files[i] < files[j]
		}
		return isfx >= jsfx // Make $_include come after $_src.
	})
	if dup, hasDup := findDuplicate(files); hasDup {
		return gniFileContents{}, skerr.Fmt("%q is included in two or more rules.", dup)
	}

	fileContents := gniFileContents{}
	for i := range files {
		if strings.HasPrefix(files[i], "$_src/") {
			fileContents.hasSrcs = true
		} else if strings.HasPrefix(files[i], "$_include/") {
			fileContents.hasIncludes = true
		} else if strings.HasPrefix(files[i], "$_modules/") {
			fileContents.hasModules = true
		}
	}

	var contents bytes.Buffer
	fmt.Fprintf(&contents, "%s = [\n", desc.Var)

	printedIncludeComment := false
	onlyHeaders := fileListContainsOnlyCppHeaderFiles(files)
	for _, target := range files {
		if !onlyHeaders && !printedIncludeComment && strings.HasPrefix(target, "$_include") {
			fmt.Fprintf(&contents, "\n  # Includes\n")
			printedIncludeComment = true
		}
		fmt.Fprintf(&contents, "  %q,\n", target)
	}
	fmt.Fprintln(&contents, "]")
	fmt.Fprintln(&contents)
	fileContents.data = contents.Bytes()

	return fileContents, nil
}

// Export all Bazel rules to a single *.gni file.
func (e *GNIExporter) exportGNIFile(gniExportDesc GNIExportDesc, qr *analysis_v2.CqueryResult) error {
	// Keep the contents of each file list in memory before writing to disk.
	// This is done so that we know what variables to define for each of the
	// file lists. i.e. $_src, $_include, etc.
	gniFileContents := gniFileContents{}
	for _, varDesc := range gniExportDesc.Vars {
		fileListContents, err := e.convertGNIFileList(varDesc, qr)
		if err != nil {
			return skerr.Wrap(err)
		}
		gniFileContents.merge(fileListContents)
	}

	writer, err := e.fs.OpenFile(e.workspaceToAbsPath(gniExportDesc.GNI))
	if err != nil {
		return skerr.Wrap(err)
	}

	writeGNFileHeader(writer, &gniFileContents)
	writer.WriteString("\n")

	_, err = writer.Write(gniFileContents.data)
	if err != nil {
		return skerr.Wrap(err)
	}

	for gniPath, footer := range footerMap {
		if gniExportDesc.GNI == gniPath {
			fmt.Fprintln(writer, footer)
			break
		}
	}

	return nil
}

// Export the contents of a Bazel query response to one or more GNI
// files.
//
// The Bazel data to export, and the destination GNI files are defined
// by the configuration data supplied to NewGNIExporter().
func (e *GNIExporter) Export(qcmd interfaces.QueryCommand) error {
	in, err := qcmd.Read()
	if err != nil {
		return skerr.Wrapf(err, "error reading bazel cquery data")
	}
	qr := &analysis_v2.CqueryResult{}
	if err := proto.Unmarshal(in, qr); err != nil {
		return skerr.Wrapf(err, "failed to unmarshal cquery result")
	}
	for _, desc := range e.exportGNIDescs {
		err = e.exportGNIFile(desc, qr)
		if err != nil {
			return skerr.Wrap(err)
		}
	}
	return nil
}

// Retrieve all variable names from a GNI file identified by |filepath|.
func (e *GNIExporter) getFileGNIVariables(filepath string) ([]string, error) {
	fileBytes, err := e.fs.ReadFile(filepath)
	if err != nil {
		return nil, skerr.Wrap(err)
	}
	reader := bytes.NewReader(fileBytes)
	scanner := bufio.NewScanner(reader)
	var variables []string
	for scanner.Scan() {
		if v := getGNILineVariable(scanner.Text()); v != "" {
			variables = append(variables, v)
		}
	}
	return variables, nil
}

// Check that all required GNI variables are present in the GNI file described by |gniExportDesc|.
func (e *GNIExporter) checkGNIFileVariables(gniExportDesc GNIExportDesc, writer interfaces.Writer) (ok bool, err error) {
	var expectedVars []string
	for _, varDesc := range gniExportDesc.Vars {
		expectedVars = append(expectedVars, varDesc.Var)
	}
	absPath := e.workspaceToAbsPath(gniExportDesc.GNI)
	actual, err := e.getFileGNIVariables(absPath)
	if err != nil {
		return false, skerr.Wrap(err)
	}
	ok = true
	for _, e := range expectedVars {
		if !util.In(e, actual) {
			fmt.Fprintf(writer, "Error: Expected variable %s not found in %s\n", e, absPath)
			ok = false
		}
	}
	return ok, nil
}

// Ensure the proper variables are defined in all generated GNI files.
// This ensures that the GNI files distributed with Skia contain the
// file lists needed to build, and for backward compatibility.
func (e *GNIExporter) checkAllVariables(writer interfaces.Writer) (numFileErrors int, err error) {
	for _, gniDesc := range e.exportGNIDescs {
		ok, err := e.checkGNIFileVariables(gniDesc, writer)
		if err != nil {
			return 0, skerr.Wrap(err)
		}
		if !ok {
			numFileErrors += 1
		}
	}
	return numFileErrors, nil
}

// CheckCurrent will determine if each on-disk GNI file is current. In other words,
// do the file contents exactly match what would be produced if Export were
// run?
func (e *GNIExporter) CheckCurrent(qcmd interfaces.QueryCommand, errWriter interfaces.Writer) (numFileErrors int, err error) {
	return e.checkAllVariables(errWriter)
}

// Make sure GNIExporter fulfills the Exporter interface.
var _ interfaces.Exporter = (*GNIExporter)(nil)
