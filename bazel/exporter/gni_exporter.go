// Copyright 2022 Google LLC
//
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package exporter

import (
	"bytes"
	"fmt"
	"path/filepath"
	"regexp"
	"sort"
	"strings"

	"go.skia.org/infra/go/skerr"
	"go.skia.org/skia/bazel/exporter/build_proto/build"
	"go.skia.org/skia/bazel/exporter/interfaces"
	"google.golang.org/protobuf/proto"
)

// The contents (or partial contents) of a GNI file.
type gniFileContents struct {
	hasSrcs     bool            // Has at least one file in $_src/ dir?
	hasIncludes bool            // Has at least one file in $_include/ dir?
	hasModules  bool            // Has at least one file in $_module/ dir?
	bazelFiles  map[string]bool // Set of Bazel files generating GNI contents.
	data        []byte          // The file contents to be written.
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

// The footer written to gn/core.gni.
const coreGNIFooter = `skia_core_sources += skia_pathops_sources

skia_core_public += skia_pathops_public
`

// The footer written to gn/sksl_tests.gni.
const skslTestsFooter = `sksl_glsl_tests_sources =
    sksl_error_tests + sksl_glsl_tests + sksl_inliner_tests +
    sksl_folding_tests + sksl_shared_tests

sksl_glsl_settings_tests_sources = sksl_blend_tests + sksl_settings_tests

sksl_metal_tests_sources =
    sksl_blend_tests + sksl_compute_tests + sksl_metal_tests + sksl_shared_tests

sksl_hlsl_tests_sources = sksl_blend_tests + sksl_shared_tests

sksl_wgsl_tests_sources =
    sksl_blend_tests + sksl_compute_tests + sksl_folding_tests +
    sksl_shared_tests + sksl_wgsl_tests

sksl_spirv_tests_sources =
    sksl_blend_tests + sksl_compute_tests + sksl_shared_tests + sksl_spirv_tests

sksl_skrp_tests_sources = sksl_folding_tests + sksl_rte_tests + sksl_shared_tests

sksl_stage_tests_sources =
    sksl_rte_tests + sksl_mesh_tests + sksl_mesh_error_tests

sksl_minify_tests_sources = sksl_folding_tests + sksl_mesh_tests + sksl_rte_tests`

// The footer written to modules/skshaper/skshaper.gni.
const skshaperFooter = `
declare_args() {
  skia_enable_skshaper = true
}
declare_args() {
  skia_enable_skshaper_tests = skia_enable_skshaper
}`

// Map of GNI file names to footer text to be appended to the end of the file.
var footerMap = map[string]string{
	"gn/core.gni":                   coreGNIFooter,
	"gn/sksl_tests.gni":             skslTestsFooter,
	"modules/skshaper/skshaper.gni": skshaperFooter,
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

func makeGniFileContents() gniFileContents {
	return gniFileContents{
		bazelFiles: make(map[string]bool),
	}
}

// Given a Bazel rule name find that rule from within the
// query results. Returns nil if the given rule is not present.
func findQueryResultRule(qr *build.QueryResult, name string) *build.Rule {
	for _, target := range qr.GetTarget() {
		r := target.GetRule()
		if r.GetName() == name {
			return r
		}
	}
	return nil
}

// Given a relative path to a file return the relative path to the
// top directory (in our case the workspace). For example:
//
//	getPathToTopDir("path/to/file.h") -> "../.."
//
// The paths are to be delimited by forward slashes ('/') - even on
// Windows.
func getPathToTopDir(path string) string {
	if filepath.IsAbs(path) {
		return ""
	}
	d, _ := filepath.Split(path)
	if d == "" {
		return "."
	}
	d = strings.TrimSuffix(d, "/")
	items := strings.Split(d, "/")
	var sb = strings.Builder{}
	for i := 0; i < len(items); i++ {
		if i > 0 {
			sb.WriteString("/")
		}
		sb.WriteString("..")
	}
	return sb.String()
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
	// These sksl tests are purposely listed as a relative path underneath resources/sksl because
	// that relative path is re-used by the GN logic to put stuff under //tests/sksl as well.
	if strings.HasPrefix(path, "resources/sksl/") {
		return strings.TrimPrefix(path, "resources/sksl/"), nil
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
func writeGNFileHeader(writer interfaces.Writer, gniFile *gniFileContents, pathToWorkspace string) {
	_, _ = fmt.Fprintln(writer, "# DO NOT EDIT: This is a generated file.")
	_, _ = fmt.Fprintln(writer, "# See //bazel/exporter_tool/README.md for more information.")

	_, _ = fmt.Fprintln(writer, "#")
	if len(gniFile.bazelFiles) > 1 {
		keys := make([]string, 0, len(gniFile.bazelFiles))
		_, _ = fmt.Fprintln(writer, "# The sources of truth are:")
		for bazelPath := range gniFile.bazelFiles {
			keys = append(keys, bazelPath)
		}
		sort.Strings(keys)
		for _, wsPath := range keys {
			_, _ = fmt.Fprintf(writer, "#   //%s\n", wsPath)
		}
	} else {
		for bazelPath := range gniFile.bazelFiles {
			_, _ = fmt.Fprintf(writer, "# The source of truth is //%s\n", bazelPath)
		}
	}

	_, _ = writer.WriteString("\n")
	_, _ = fmt.Fprintln(writer, "# To update this file, run make -C bazel generate_gni")

	_, _ = writer.WriteString("\n")
	if gniFile.hasSrcs {
		_, _ = fmt.Fprintf(writer, "_src = get_path_info(\"%s/src\", \"abspath\")\n", pathToWorkspace)
	}
	if gniFile.hasIncludes {
		_, _ = fmt.Fprintf(writer, "_include = get_path_info(\"%s/include\", \"abspath\")\n", pathToWorkspace)
	}
	if gniFile.hasModules {
		_, _ = fmt.Fprintf(writer, "_modules = get_path_info(\"%s/modules\", \"abspath\")\n", pathToWorkspace)
	}
}

// removeDuplicates returns the list of files after it has been sorted and
// all duplicate values have been removed.
func removeDuplicates(files []string) []string {
	if len(files) <= 1 {
		return files
	}
	sort.Strings(files)
	rv := make([]string, 0, len(files))
	rv = append(rv, files[0])
	for _, f := range files {
		if rv[len(rv)-1] != f {
			rv = append(rv, f)
		}
	}
	return rv
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

// Given an absolute path return a workspace relative path.
func (e *GNIExporter) absToWorkspacePath(absPath string) (string, error) {
	if !filepath.IsAbs(absPath) {
		return "", skerr.Fmt(`"%s" is not an absolute path`, absPath)
	}
	if absPath == e.workspaceDir {
		return "", nil
	}
	wsDir := e.workspaceDir + "/"
	if !strings.HasPrefix(absPath, wsDir) {
		return "", skerr.Fmt(`"%s" is not in the workspace "%s"`, absPath, wsDir)
	}
	return strings.TrimPrefix(absPath, wsDir), nil
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
	for path := range other.bazelFiles {
		c.bazelFiles[path] = true
	}
	c.data = append(c.data, other.data...)
}

// Convert all rules that go into a GNI file list.
func (e *GNIExporter) convertGNIFileList(desc GNIFileListExportDesc, qr *build.QueryResult) (gniFileContents, error) {
	var rules []string
	fileContents := makeGniFileContents()
	var targets []string
	for _, ruleName := range desc.Rules {
		r := findQueryResultRule(qr, ruleName)
		if r == nil {
			return gniFileContents{}, skerr.Fmt("Cannot find rule %s", ruleName)
		}
		absBazelPath, _, _, err := parseLocation(*r.Location)
		if err != nil {
			return gniFileContents{}, skerr.Wrap(err)
		}
		wsBazelpath, err := e.absToWorkspacePath(absBazelPath)
		if err != nil {
			return gniFileContents{}, skerr.Wrap(err)
		}
		fileContents.bazelFiles[wsBazelpath] = true
		t, err := getSrcsAndHdrs(r)
		if err != nil {
			return gniFileContents{}, skerr.Wrap(err)
		}
		if len(t) == 0 {
			return gniFileContents{}, skerr.Fmt("No files to export in rule %s", ruleName)
		}
		targets = append(targets, t...)
		rules = append(rules, ruleName)
	}

	files, err := convertTargetsToFilePaths(targets)
	if err != nil {
		return gniFileContents{}, skerr.Wrap(err)
	}

	files, err = addGNIVariablesToWorkspacePaths(files)
	if err != nil {
		return gniFileContents{}, skerr.Wrap(err)
	}

	files = removeDuplicates(files)

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

	if len(rules) > 1 {
		_, _ = fmt.Fprintln(&contents, "# List generated by Bazel rules:")
		for _, bazelFile := range rules {
			_, _ = fmt.Fprintf(&contents, "#  %s\n", bazelFile)
		}
	} else if len(rules) > 0 {
		_, _ = fmt.Fprintf(&contents, "# Generated by Bazel rule %s\n", rules[0])
	}
	_, _ = fmt.Fprintf(&contents, "%s = [\n", desc.Var)

	for _, target := range files {
		_, _ = fmt.Fprintf(&contents, "  %q,\n", target)
	}
	_, _ = fmt.Fprintln(&contents, "]")
	_, _ = fmt.Fprintln(&contents)
	fileContents.data = contents.Bytes()

	return fileContents, nil
}

// Export all Bazel rules to a single *.gni file.
func (e *GNIExporter) exportGNIFile(gniExportDesc GNIExportDesc, qr *build.QueryResult) error {
	// Keep the contents of each file list in memory before writing to disk.
	// This is done so that we know what variables to define for each of the
	// file lists. i.e. $_src, $_include, etc.
	gniFileContents := makeGniFileContents()
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

	pathToWorkspace := getPathToTopDir(gniExportDesc.GNI)
	writeGNFileHeader(writer, &gniFileContents, pathToWorkspace)
	_, _ = writer.WriteString("\n")

	_, err = writer.Write(gniFileContents.data)
	if err != nil {
		return skerr.Wrap(err)
	}

	for gniPath, footer := range footerMap {
		if gniExportDesc.GNI == gniPath {
			_, _ = fmt.Fprintln(writer, footer)
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
	qr := &build.QueryResult{}
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

// Make sure GNIExporter fulfills the Exporter interface.
var _ interfaces.Exporter = (*GNIExporter)(nil)
