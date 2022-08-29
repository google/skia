// Copyright 2022 Google LLC
//
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package exporter

import (
	"bufio"
	"bytes"
	"fmt"
	"os"
	"path/filepath"
	"regexp"
	"sort"
	"strings"
	"sync"

	"go.skia.org/infra/go/skerr"
	"go.skia.org/infra/go/util"
	"go.skia.org/skia/bazel/exporter/build_proto/analysis_v2"
	"go.skia.org/skia/bazel/exporter/build_proto/build"
	"go.skia.org/skia/bazel/exporter/interfaces"
	"google.golang.org/protobuf/proto"
)

// gniFileChunk represents a valid chunk of a GNI file.
type gniFileChunk []byte

// gniFileList represents a valid chunk of a GNI file which represents a
// file list. This is of the form:
//
//	file_sources = [
//	  "$_include/core/SkPicture.cpp",
//	  "$_include/core/SkPicture.h",
//	]
type gniFileList struct {
	gniVarName string       // The file list variable name in the GNI file.
	data       gniFileChunk // GNI file list data written as text data.
}

type gniFileContents struct {
	wsGNIPath   string        // Workspace relative path to *.gni file.
	hasSrcs     bool          // Has at least one file in $_src/ dir?
	hasIncludes bool          // Has at least one file in $_include/ dir?
	hasModules  bool          // Has at least one file in $_module/ dir?
	data        []gniFileList // Unordered list of GNI chunks to be written to |wsGNIPath|.
}

type GNIExporterParams struct {
	WorkspaceDir string // The Bazel workspace directory path.

	// Map workspace relative gni path to list of required vars. This map is used when
	// exporting (Export()) as well as verification (CheckCurrent()).
	//
	// When exporting the list of required variables determines the order in which the
	// GNI variables are written. Any file list not specified in this list is written
	// after those specified, and in an undetermined order. This ordering is done to
	// ensure the file contents are written in a stable ordering providing for easy
	// comparison with the prior versions being replaced.
	//
	// When verifying files this dictionary determines what *.gni files are verified
	// and what variables they *must* contain.
	GNIFileVars map[string][]string // Map ws relative gni path to list of required vars.
}

// GNIExporter is an object responsible for exporting rules defined in a
// Bazel workspace as file lists in GNI format (GN's *.gni files). This exporter
// is very tightly coupled to the Skia Bazel rules and GNI configuration.
type GNIExporter struct {
	workspaceDir    string                      // The Bazel workspace path.
	gniFileVars     map[string][]string         // Map ws relative gni path to list of required vars.
	fs              interfaces.FileSystem       // For filesystem interactions.
	gniFileContents map[string]*gniFileContents // Map *.gni filenames to mutable file data object pointers.
	convertedRules  []string                    // All converted Bazel rules (bazel rule name).
	convertMutex    sync.Mutex                  // Ensure only one rule conversion occurring at a time.
}

// Represents an association between a workspace relative path (identified b
// a regular expression) and the output GNI file.
type gniPattern struct {
	reg *regexp.Regexp // reg to match workspace relative directory path.
	gni string         // Workspace relative *.gni file path.
}

// This list is processed in order to map directory paths to *.gni files.
var dirToGniMapper = []gniPattern{
	{reg: regexp.MustCompile(`^include/core$`), gni: "gn/core.gni"},
	{reg: regexp.MustCompile(`^include/docs$`), gni: "gn/pdf.gni"},
	{reg: regexp.MustCompile(`^include/effects$`), gni: "gn/effects.gni"},
	{reg: regexp.MustCompile(`^include/pathops$`), gni: "gn/core.gni"},
	{reg: regexp.MustCompile(`^include/private$`), gni: "gn/core.gni"},
	{reg: regexp.MustCompile(`^include/private/chromium$`), gni: "gn/core.gni"},
	{reg: regexp.MustCompile(`^include/private/gpu.*`), gni: "gn/gpu.gni"},
	{reg: regexp.MustCompile(`^include/utils$`), gni: "gn/utils.gni"},
	{reg: regexp.MustCompile(`^src/core$`), gni: "gn/core.gni"},
	{reg: regexp.MustCompile(`^src/effects/imagefilters$`), gni: "gn/effects_imagefilters.gni"},
	{reg: regexp.MustCompile(`^src/lazy$`), gni: "gn/core.gni"},
	{reg: regexp.MustCompile(`^src/opts$`), gni: "gn/core.gni"},
	{reg: regexp.MustCompile(`^src/pathops$`), gni: "gn/core.gni"},
	{reg: regexp.MustCompile(`^src/pdf$`), gni: "gn/pdf.gni"},
	{reg: regexp.MustCompile(`^src/shaders/gradients$`), gni: "gn/effects.gni"},
	{reg: regexp.MustCompile(`^src/shaders$`), gni: "gn/core.gni"},
	{reg: regexp.MustCompile(`^src/text/gpu$`), gni: "gn/gpu.gni"},
	{reg: regexp.MustCompile(`^src/text$`), gni: "gn/core.gni"},
	{reg: regexp.MustCompile(`^src/utils$`), gni: "gn/utils.gni"},
	{reg: regexp.MustCompile(`^src/effects.*`), gni: "gn/effects.gni"},
	{reg: regexp.MustCompile(`^src/gpu.*`), gni: "gn/gpu.gni"},
	{reg: regexp.MustCompile(`^src/ports.*`), gni: "gn/sksl.gni"},
	{reg: regexp.MustCompile(`^src/image.*`), gni: "gn/core.gni"},
	{reg: regexp.MustCompile(`^include/sksl.*`), gni: "gn/sksl.gni"},
	{reg: regexp.MustCompile(`^include/gpu.*`), gni: "gn/gpu.gni"},
	{reg: regexp.MustCompile(`^src/sksl.*`), gni: "gn/sksl.gni"},
	{reg: regexp.MustCompile(`.*skcms.*`), gni: "modules/skcms/skcms.gni"},
}

// A map of rules which should be merged into a target rule.
// Note: the function processing this list will not look for multi-level
// merges. In other words, this will not work:
//
//	"A" → "B"
//	"B" → "C"
var mergeMap = map[string]string{
	"//include/private:private_hdrs":          "//src/core:core_srcs",
	"//include/private:sksl_private_hdrs":     "//src/sksl:core_srcs",
	"//include/private/chromium:private_hdrs": "//src/core:core_srcs",
	"//include/sksl:public_hdrs":              "//src/sksl:core_srcs",
	"//src/c:effects_srcs":                    "//src/effects:effects_srcs",
	"//src/c:srcs":                            "//src/core:core_srcs",
	"//src/core:core_hdrs":                    "//src/core:core_srcs",
	"//src/core:precompile_hdrs":              "//src/core:precompile_srcs",
	"//src/core:skpicture_hdrs":               "//src/core:skpicture_srcs",
	"//src/core:sksl_hdrs":                    "//src/core:core_srcs",
	"//src/core:sksl_srcs":                    "//src/core:core_srcs",
	"//src/effects:effects_hdrs":              "//src/effects:effects_srcs",
	"//src/image:core_hdrs":                   "//src/core:core_srcs",
	"//src/image:core_srcs":                   "//src/core:core_srcs",
	"//src/lazy:lazy_hdrs":                    "//src/core:core_srcs",
	"//src/lazy:lazy_srcs":                    "//src/core:core_srcs",
	"//src/opts:private_hdrs":                 "//src/core:core_srcs",
	"//src/pathops:pathops_hdrs":              "//src/pathops:pathops_srcs",
	"//src/pdf:pdf_hdrs":                      "//src/pdf:pdf_srcs",
	"//src/shaders:shader_hdrs":               "//src/core:core_srcs",
	"//src/shaders:shader_srcs":               "//src/core:core_srcs",
	"//src/shaders:skpicture_hdrs":            "//src/shaders:skpicture_srcs",
	"//src/shaders:skpicture_srcs":            "//src/core:skpicture_srcs",
	"//src/shaders/gradients:gradient_hdrs":   "//src/effects:effects_srcs",
	"//src/shaders/gradients:gradient_srcs":   "//src/effects:effects_srcs",
	"//src/sksl:core_hdrs":                    "//src/sksl:core_srcs",
	"//src/sksl/analysis:analysis_hdrs":       "//src/sksl:core_srcs",
	"//src/sksl/analysis:analysis_srcs":       "//src/sksl:core_srcs",
	"//src/sksl/codegen:core_srcs":            "//src/sksl:core_srcs",
	"//src/sksl/codegen:gpu_hdrs":             "//src/sksl/codegen:gpu_srcs",
	"//src/sksl/codegen:srcs":                 "//src/sksl:core_srcs",
	"//src/sksl/dsl:srcs":                     "//src/sksl:core_srcs",
	"//src/sksl/dsl/priv:srcs":                "//src/sksl:core_srcs",
	"//src/sksl/ir:ir_hdrs":                   "//src/sksl:core_srcs",
	"//src/sksl/ir:ir_srcs":                   "//src/sksl:core_srcs",
	"//src/sksl/tracing:srcs":                 "//src/sksl:core_srcs",
	"//src/sksl/transform:transform_srcs":     "//src/sksl:core_srcs",
	"//src/text:text_hdrs":                    "//src/core:core_srcs",
	"//src/text:text_srcs":                    "//src/core:core_srcs",
	"//src/utils:core_hdrs":                   "//src/utils:core_srcs",
	"//src/utils:json_hdrs":                   "//src/utils:core_srcs",
	"//src/utils:json_srcs":                   "//src/utils:core_srcs",
}

// Override the generated rule (technically GNI file set) name. This is
// to match the current hardcoded file set names.
var ruleNameMap = map[string]string{
	"include_core_precompile_public_hdrs":        "skia_precompile_public",
	"include_core_public_hdrs":                   "skia_core_public",
	"include_core_skpicture_public_hdrs":         "skia_skpicture_public",
	"include_docs_public_hdrs":                   "skia_pdf_public",
	"include_effects_public_hdrs":                "skia_effects_public",
	"include_pathops_public_hdrs":                "skia_pathops_public",
	"include_utils_public_hdrs":                  "skia_utils_public",
	"modules_skcms_skcms":                        "skcms_sources",
	"src_core_core_srcs":                         "skia_core_sources",
	"src_core_precompile_srcs":                   "skia_precompile_sources",
	"src_core_skpicture_srcs":                    "skia_skpicture_sources",
	"src_effects_effects_srcs":                   "skia_effects_sources",
	"src_effects_imagefilters_imagefilters_hdrs": "skia_effects_imagefilter_public",
	"src_effects_imagefilters_imagefilters_srcs": "skia_effects_imagefilter_sources",
	"src_pathops_pathops_srcs":                   "skia_pathops_sources",
	"src_pdf_pdf_srcs":                           "skia_pdf_sources",
	"src_sksl_codegen_gpu_srcs":                  "skia_sksl_gpu_sources",
	"src_sksl_core_srcs":                         "skia_sksl_sources",
	"src_utils_core_srcs":                        "skia_utils_sources",
}

var deprecatedFiles = []string{
	"include/core/SkDrawLooper.h",
	"include/effects/SkBlurDrawLooper.h",
	"include/effects/SkLayerDrawLooper.h",
}

// TODO(skbug.com/12345): Remove when development is complete.
// Any rule not mapped to a specific file is put into this file. This
// makes it easy to identify orphaned rules during development.
const defaultGNI = "gn/XXX.gni"

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
		workspaceDir:    params.WorkspaceDir,
		gniFileVars:     params.GNIFileVars,
		fs:              filesystem,
		gniFileContents: make(map[string]*gniFileContents),
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

// Given a Bazel rule, return the GNI file list variable name in which
// to contain all files from the given rule.
func getRuleGNIVariableName(r *build.Rule) (string, error) {
	n, err := getRuleSimpleName(r.GetName())
	if err != nil {
		return "", skerr.Wrap(err)
	}
	// Check if there are specific overrides.
	if v, ok := ruleNameMap[n]; ok {
		return v, nil
	}
	return n, nil
}

// Is the file path a C++ header?
func isHeaderFile(path string) bool {
	ext := strings.ToLower(filepath.Ext(path))
	return ext == ".h" || ext == ".hpp"
}

// Is the file path portion of the given |target| a C/C++ header file?
func isTargetCppHeaderFile(target string) bool {
	_, _, t, err := parseRule(target)
	if err != nil {
		return false
	}
	return isHeaderFile(t)
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

// Should the given rule be skipped during the export process?
// The general rule is that:
//
//  1. Package visibility is skipped - unless it contains a "srcs"
//     attribute which is entirely made up of headers.
//  2. Contains the word "public" in the rule name.
//  3. Matches a few hardcoded rule names to wither skip or not.
func shouldSkipRule(r *build.Rule) (bool, error) {
	if r.GetName() == "//src/opts:srcs" {
		// This has a hardcoded footer in //gn/opts.gni.
		return true, nil
	}
	if strings.Contains(r.GetName(), "public") {
		return false, nil
	}
	if r.GetName() == "//include/private:private_hdrs" ||
		r.GetName() == "//src/opts:private_hdrs" {
		return false, nil
	}
	vis, err := getRuleStringArrayAttribute(r, "visibility")
	if err != nil {
		fmt.Fprintf(os.Stderr, "Rule %s bad visibility: %v", r.GetName(), err)
		return false, skerr.Wrap(err)
	}
	if len(vis) != 1 || !strings.HasSuffix(vis[0], ":__pkg__") {
		// It's possible to have multiple visibility values. Only checking
		// for a list of one because this implementation is simplistic, but
		// adequate for Skia's current Bazel rules.
		return false, nil
	}
	// Iterate over srcs because rules may put headers in srcs. Only if srcs
	// is made up solely of headers will it be skipped.
	srcs, err := getRuleStringArrayAttribute(r, "srcs")
	if err != nil {
		return false, skerr.Wrap(err)
	}
	if len(srcs) == 0 {
		return true, nil
	}
	for _, item := range srcs {
		if isFileTarget(item) && !isTargetCppHeaderFile(item) {
			return false, nil
		}
	}
	return true, nil
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

// Print a warning, to stdout, of all files in the list of
// files which are duplicated entries.
//
// Note: |files| must be sorted.
func warnDuplicates(files []string) {
	for i, e := range files {
		if i == len(files)-1 {
			continue
		}
		if e == files[i+1] {
			fmt.Printf("Warning, duplicate file: %s\n", e)
		}
	}
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

// Get the file list (|gniFileList|) object for the given file list variable name.
func (fd *gniFileContents) getFileList(fileListVariableName string) (gniFileList, bool) {
	for i := 0; i < len(fd.data); i++ {
		if fd.data[i].gniVarName == fileListVariableName {
			return fd.data[i], true
		}
	}
	return gniFileList{}, false
}

// Given an absolute path to a file/dir within a workspace,
// return a workspace relative path.
func (e *GNIExporter) absToWorkspacePath(absPath string) string {
	if !filepath.IsAbs(absPath) {
		panic("filepath not absolute")
	}
	return strings.TrimPrefix(absPath[len(e.workspaceDir):], "/")
}

// Given a workspace relative path return an absolute path.
func (e *GNIExporter) workspaceToAbsPath(wsPath string) string {
	if filepath.IsAbs(wsPath) {
		panic("filepath already absolute")
	}
	return filepath.Join(e.workspaceDir, wsPath)
}

// Retrieve (or create) a gniFileContents object for the given *.gni relative file path.
func (e *GNIExporter) getFileData(gniPath string) *gniFileContents {
	if d, ok := e.gniFileContents[gniPath]; ok {
		return d
	}
	d := &gniFileContents{wsGNIPath: gniPath}
	e.gniFileContents[gniPath] = d
	return d
}

// Should the workspace relative path to a GNI file be exported?
// TODO(skbug.com/12345): Delete this function once all *.gni's are supported.
func (e *GNIExporter) shouldExportFile(wsFilePath string) bool {
	_, ok := e.gniFileVars[wsFilePath]
	return ok
}

// Map a rule location directory to the desired output *.gni file path.
// Returns a workspace relative path - not absolute.
func (e *GNIExporter) getLocationGNFilePath(absRuleLocationDir string) string {
	wsRelativePath := e.absToWorkspacePath(absRuleLocationDir)
	for _, entry := range dirToGniMapper {
		if entry.reg.Match([]byte(wsRelativePath)) {
			if !e.shouldExportFile(entry.gni) {
				return defaultGNI
			}
			return entry.gni
		}
	}
	// TODO(skbug.com/12345): Make this an error when development is complete.
	return defaultGNI
}

// Get the desired output *.gni file path for the given rule.
// Returns a workspace relative path - not absolute.
func (e *GNIExporter) getRuleGNFilePath(r *build.Rule) (string, error) {
	// TODO(skbug.com/12345): Un-hardcode this. There are a few cases where not
	// all files in a directory map to the same GNI file.
	if r.GetName() == "//src/c:effects_srcs" {
		return "gn/effects.gni", nil
	}
	if strings.Contains(r.GetName(), ":gpu_") {
		return "gn/gpu.gni", nil
	}
	dir, err := getLocationDir(r.GetLocation())
	if err != nil {
		return "", skerr.Wrap(err)
	}
	return e.getLocationGNFilePath(dir), nil
}

// Convert the given rule into a chunk of valid GNI configuration text. The
// converted project data is saved in GNIExporter instance for later writing.
func (e *GNIExporter) convertRule(r *build.Rule, qr *analysis_v2.CqueryResult) error {
	if util.In(r.GetName(), e.convertedRules) {
		return nil
	}
	e.convertedRules = append(e.convertedRules, r.GetName())
	skip, err := shouldSkipRule(r)
	if err != nil {
		return skerr.Wrap(err)
	}
	if skip {
		return nil
	}
	if _, ok := mergeMap[r.GetName()]; ok {
		// Will find and write this rule later when the target rule is exported.
		return nil
	}
	targets, err := getSrcsAndHdrs(r)
	if err != nil {
		return skerr.Wrap(err)
	}

	// If there are any rules to be merged into this rule, collect the
	// data from those rules and merge into this rules data.
	for otherName, ruleVariableName := range mergeMap {
		if ruleVariableName != r.GetName() {
			continue
		}
		otherRule := findRule(qr, otherName)
		if otherRule == nil {
			continue
		}
		e.convertedRules = append(e.convertedRules, otherRule.GetName())

		otherFiles, err := getSrcsAndHdrs(otherRule)
		if err != nil {
			return skerr.Wrap(err)
		}
		targets = append(targets, otherFiles...)
	}

	if len(targets) == 0 {
		return nil
	}

	files, err := convertTargetsToFilePaths(targets)
	if err != nil {
		return skerr.Wrap(err)
	}

	files = filterDeprecatedFiles(files)

	files, err = addGNIVariablesToWorkspacePaths(files)
	if err != nil {
		return skerr.Wrap(err)
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
	warnDuplicates(files)

	ruleVariableName, err := getRuleGNIVariableName(r)
	if err != nil {
		return skerr.Wrap(err)
	}

	wsGNIPath, err := e.getRuleGNFilePath(r)
	if err != nil {
		return skerr.Wrap(err)
	}
	if !e.shouldExportFile(wsGNIPath) {
		return nil
	}
	gniFileContents := e.getFileData(wsGNIPath)

	for i := range files {
		if strings.HasPrefix(files[i], "$_src/") {
			gniFileContents.hasSrcs = true
		} else if strings.HasPrefix(files[i], "$_include/") {
			gniFileContents.hasIncludes = true
		} else if strings.HasPrefix(files[i], "$_modules/") {
			gniFileContents.hasModules = true
		}
	}
	var contents bytes.Buffer
	fmt.Fprintf(&contents, "# %s\n", r.GetName())
	fmt.Fprintf(&contents, "%s = [\n", ruleVariableName)

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

	// Save and write later.
	gniFileContents.data = append(gniFileContents.data,
		gniFileList{gniVarName: ruleVariableName, data: contents.Bytes()})

	return nil
}

func (e *GNIExporter) writeData(gniFileContents *gniFileContents, writer interfaces.Writer) error {
	writeGNFileHeader(writer, gniFileContents)

	writtenVars := []string{}

	// Write out the GNI data.
	if variableWriteOrder, ok := e.gniFileVars[gniFileContents.wsGNIPath]; ok {
		for i := 0; i < len(variableWriteOrder); i++ {
			variableName := variableWriteOrder[i]
			gniFileList, ok := gniFileContents.getFileList(variableName)
			if !ok {
				// TODO(skbug.com/12345): Make this an error when all file sets are matched to rules.
				fmt.Printf("cannot find %s variable data\n", variableName)
				continue
			}
			writer.WriteString("\n")
			_, err := writer.Write(gniFileList.data)
			if err != nil {
				return skerr.Wrap(err)
			}
			writtenVars = append(writtenVars, variableName)
		}
	}

	for i := 0; i < len(gniFileContents.data); i++ {
		r := gniFileContents.data[i]
		if util.In(r.gniVarName, writtenVars) {
			continue
		}
		writer.WriteString("\n")
		_, err := writer.Write(r.data)
		if err != nil {
			return skerr.Wrap(err)
		}
		writtenVars = append(writtenVars, r.gniVarName)
	}

	// Append the hardcoded file footers.
	for sfx, footer := range footerMap {
		if strings.HasSuffix(gniFileContents.wsGNIPath, sfx) {
			writer.WriteString("\n")
			fmt.Fprintln(writer, footer)
			break
		}
	}
	return nil
}

// Write all GNI file data to the appropriate files.
func (e *GNIExporter) writeFileData(gniFileContents *gniFileContents) error {
	writer, err := e.fs.OpenFile(e.workspaceToAbsPath(gniFileContents.wsGNIPath))
	if err != nil {
		return skerr.Wrap(err)
	}

	return e.writeData(gniFileContents, writer)
}

// Convert all Bazel query results (i.e. rules) into equivalent GNI file lists.
func (e *GNIExporter) convertQueryResults(qcmd interfaces.QueryCommand) error {
	e.convertMutex.Lock()
	defer e.convertMutex.Unlock()
	e.convertedRules = nil

	in, err := qcmd.Read()
	if err != nil {
		return skerr.Wrapf(err, "error reading bazel cquery data")
	}
	qr := &analysis_v2.CqueryResult{}
	if err := proto.Unmarshal(in, qr); err != nil {
		return skerr.Wrapf(err, "failed to unmarshal cquery result")
	}
	// cquery results are "dependency-ordered", but for a guaranteed
	// stable ordering we sort alphabetically.
	sortedResults := qr.Results
	sort.Slice(sortedResults, func(i, j int) bool {
		return sortedResults[i].Target.Rule.GetName() < sortedResults[j].Target.Rule.GetName()
	})
	for _, result := range sortedResults {
		r := result.Target.Rule
		err := e.convertRule(r, qr)
		if err != nil {
			return skerr.Wrapf(err, `error exporting rule %q`, r.GetName())
		}
	}
	return nil
}

// Export will convert all rules defined in the Bazel query
// command result to *.gni files within the workspace.
func (e *GNIExporter) Export(qcmd interfaces.QueryCommand) error {
	err := e.convertQueryResults(qcmd)
	if err != nil {
		return skerr.Wrap(err)
	}
	for _, f := range e.gniFileContents {
		err = e.writeFileData(f)
		if err != nil {
			return skerr.Wrap(err)
		}
	}
	return nil
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

// Check that all required GNI variables are present in the GNI file identified by
// |filepath|. Error results will be written using the |writer|.
func (e *GNIExporter) checkGNIFileVariables(filepath string, writer interfaces.Writer) (ok bool, err error) {
	wsRelPath := e.absToWorkspacePath(filepath)
	expectedVars, ok := e.gniFileVars[wsRelPath]
	if !ok {
		return false, skerr.Fmt("%s is not a generated file", filepath)
	}
	actual, err := e.getFileGNIVariables(filepath)
	if err != nil {
		return false, skerr.Wrap(err)
	}
	ok = true
	for _, e := range expectedVars {
		if !util.In(e, actual) {
			fmt.Fprintf(writer, "Error: Expected variable %s not found in %s\n", e, filepath)
			ok = false
		}
	}
	return ok, nil
}

// Ensure the proper variables are defined in all generated GNI files
// This ensures that the GNI files distributed with Skia contain the
// file lists needed to build, and for backward compatibility.
func (e *GNIExporter) checkAllVariables(writer interfaces.Writer) (numFileErrors int, err error) {
	for fname, _ := range e.gniFileVars {
		ok, err := e.checkGNIFileVariables(e.workspaceToAbsPath(fname), writer)
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
