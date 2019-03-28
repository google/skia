// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package main

// gen_interface creates the assemble/validate cpp files given the
// interface.json5 file.
// See README for more details.

import (
	"flag"
	"fmt"
	"io/ioutil"
	"os"
	"path/filepath"
	"sort"
	"strings"

	"github.com/flynn/json5"
)

var (
	outDir  = flag.String("out_dir", "../../src/gpu/gl", "Where to output the GrGlAssembleInterface_* and GrGlInterface.cpp files")
	inTable = flag.String("in_table", "./interface.json5", "The JSON5 table to read in")
	dryRun  = flag.Bool("dryrun", false, "Print the outputs, don't write to file")
)

const (
	CORE_FEATURE        = "<core>"
	SPACER              = "    "
	GLES_FILE_NAME      = "GrGLAssembleGLESInterfaceAutogen.cpp"
	GL_FILE_NAME        = "GrGLAssembleGLInterfaceAutogen.cpp"
	INTERFACE_FILE_NAME = "GrGLInterfaceAutogen.cpp"
)

// FeatureSet represents one set of requirements for each of the GL "standards" that
// Skia supports.  This is currently OpenGL, OpenGL ES and WebGL.
// OpenGL is typically abbreviated as just "GL".
// https://www.khronos.org/registry/OpenGL/index_gl.php
// https://www.khronos.org/opengles/
// https://www.khronos.org/registry/webgl/specs/1.0/
type FeatureSet struct {
	GLReqs    []Requirement `json:"GL"`
	GLESReqs  []Requirement `json:"GLES"`
	WebGLReqs []Requirement `json:"WebGL"`

	Functions         []string           `json:"functions"`
	HardCodeFunctions []HardCodeFunction `json:"hardcode_functions"`
	OptionalFunctions []string           `json:"optional"` // not checked in validate

	// only assembled/validated when testing
	TestOnlyFunctions []string `json:"test_functions"`

	Required bool `json:"required"`
	EGLProc  bool `json:"egl_proc"`
}

// Requirement lists how we know if a function exists. Extension is the
// GL extension (or the string CORE_FEATURE if it's part of the core functionality).
// MinVersion optionally indicates the minimum version of a standard
// that has the function.
// SuffixOverride allows the extension suffix to be manually specified instead
// of automatically derived from the extension name.
// (for example, if an NV extension specifies some EXT extensions)
type Requirement struct {
	Extension      string     `json:"ext"` // required
	MinVersion     *GLVersion `json:"min_version"`
	SuffixOverride *string    `json:"suffix"`
}

// HardCodeFunction indicates to not use the C++ macro and just directly
// adds a given function ptr to the struct.
type HardCodeFunction struct {
	PtrName  string `json:"ptr_name"`
	CastName string `json:"cast_name"`
	GetName  string `json:"get_name"`
}

var CORE_REQUIREMENT = Requirement{Extension: CORE_FEATURE, MinVersion: nil}

type GLVersion [2]int

// RequirementGetter functions allows us to "iterate" over the requirements
// of the different standards which are stored as keys in FeatureSet and
// normally not easily iterable.
type RequirementGetter func(FeatureSet) []Requirement

func glRequirements(fs FeatureSet) []Requirement {
	return fs.GLReqs
}

func glesRequirements(fs FeatureSet) []Requirement {
	return fs.GLESReqs
}

func webglRequirements(fs FeatureSet) []Requirement {
	return fs.WebGLReqs
}

// generateAssembleInterface creates one GrGLAssembleInterface_[type]_gen.cpp
// for each of the standards
func generateAssembleInterface(features []FeatureSet) {
	gl := fillAssembleTemplate(ASSEMBLE_INTERFACE_GL, features, glRequirements)
	writeToFile(*outDir, GL_FILE_NAME, gl)
	gles := fillAssembleTemplate(ASSEMBLE_INTERFACE_GL_ES, features, glesRequirements)
	writeToFile(*outDir, GLES_FILE_NAME, gles)
	// uncomment this when ready to implement WebGL
	// webgl := fillAssembleTemplate(ASSEMBLE_INTERFACE_WEB_GL, features, webglRequirements)
	// writeToFile(*outDir, "GrGLAssembleInterface_webgl_gen.cpp", webgl)
}

// fillAssembleTemplate returns a generated file given a template (for a single standard)
// to fill out and a slice of features with which to fill it.  getReqs is used to select
// the requirements for the standard we are working on.
func fillAssembleTemplate(template string, features []FeatureSet, getReqs RequirementGetter) string {
	content := ""
	for _, feature := range features {
		// For each feature set, we are going to create a series of
		// if statements, which check for the requirements (e.g. extensions, version)
		// and inside those if branches, write the code to load the
		// correct function pointer to the interface. GET_PROC and
		// GET_PROC_SUFFIX are macros defined in C++ part of the template
		// to accomplish this (for a core feature and extensions, respectively).
		reqs := getReqs(feature)
		if len(reqs) == 0 {
			continue
		}
		isEGL := feature.EGLProc
		// blocks holds all the if blocks generated - it will be joined with else
		// after and appended to content
		blocks := []string{}
		for i, req := range reqs {
			block := ""
			ifExpr := requirementIfExpression(req, true)

			if ifExpr != "" {
				if strings.HasPrefix(ifExpr, "(") {
					ifExpr = "if " + ifExpr + " {"
				} else {
					ifExpr = "if (" + ifExpr + ") {"
				}
				// Indent the first if statement
				if i == 0 {
					block = addLine(block, ifExpr)
				} else {
					block += ifExpr + "\n"
				}
			}
			// sort for determinism
			sort.Strings(feature.Functions)
			for _, function := range feature.Functions {
				block = assembleFunction(block, ifExpr, function, isEGL, req)
			}
			sort.Strings(feature.TestOnlyFunctions)
			if len(feature.TestOnlyFunctions) > 0 {
				block += "#if GR_TEST_UTILS\n"
				for _, function := range feature.TestOnlyFunctions {
					block = assembleFunction(block, ifExpr, function, isEGL, req)
				}
				block += "#endif\n"
			}

			// a hard code function does not use the C++ macro
			for _, hcf := range feature.HardCodeFunctions {
				if ifExpr != "" {
					// extra tab for being in an if statement
					block += SPACER
				}
				line := fmt.Sprintf(`functions->%s =(%s*)get(ctx, "%s");`, hcf.PtrName, hcf.CastName, hcf.GetName)
				block = addLine(block, line)
			}
			if ifExpr != "" {
				block += SPACER + "}"
			}
			blocks = append(blocks, block)
		}
		content += strings.Join(blocks, " else ")

		if feature.Required && reqs[0] != CORE_REQUIREMENT {
			content += ` else {
        SkASSERT(false); // Required feature
        return nullptr;
    }`
		}

		if !strings.HasSuffix(content, "\n") {
			content += "\n"
		}
		// Add an extra space between blocks for easier readability
		content += "\n"

	}

	return strings.Replace(template, "[[content]]", content, 1)
}

// assembleFunction is a little helper that will add a function to the interface
// using an appropriate macro (e.g. if the function is added)
// with an extension.
func assembleFunction(block, ifExpr, function string, isEGL bool, req Requirement) string {
	if ifExpr != "" {
		// extra tab for being in an if statement
		block += SPACER
	}
	suffix := deriveSuffix(req.Extension)
	// Some ARB extensions don't have ARB suffixes because they were written
	// for backwards compatibility simultaneous to adding them as required
	// in a new GL version.
	if suffix == "ARB" {
		suffix = ""
	}
	if req.SuffixOverride != nil {
		suffix = *req.SuffixOverride
	}
	if isEGL {
		block = addLine(block, fmt.Sprintf("GET_EGL_PROC_SUFFIX(%s, %s);", function, suffix))
	} else if req.Extension == CORE_FEATURE || suffix == "" {
		block = addLine(block, fmt.Sprintf("GET_PROC(%s);", function))
	} else if req.Extension != "" {
		block = addLine(block, fmt.Sprintf("GET_PROC_SUFFIX(%s, %s);", function, suffix))
	}
	return block
}

// requirementIfExpression returns a string that is an if expression
// Notably, there is no if expression if the function is a "core" function
// on all supported versions.
// The expressions are wrapped in parentheses so they can be safely
// joined together with && or ||.
func requirementIfExpression(req Requirement, isLocal bool) string {
	mv := req.MinVersion
	if req == CORE_REQUIREMENT {
		return ""
	}
	if req.Extension == CORE_FEATURE && mv != nil {
		return fmt.Sprintf("(glVer >= GR_GL_VER(%d,%d))", mv[0], mv[1])
	}
	extVar := "fExtensions"
	if isLocal {
		extVar = "extensions"
	}
	// We know it has an extension
	if req.Extension != "" {
		if mv == nil {
			return fmt.Sprintf("%s.has(%q)", extVar, req.Extension)
		} else {
			return fmt.Sprintf("(glVer >= GR_GL_VER(%d,%d) && %s.has(%q))", mv[0], mv[1], extVar, req.Extension)
		}
	}
	abort("ERROR: requirement must have ext\n")
	return "ERROR"
}

// driveSuffix returns the suffix of the function associated with the given
// extension.
func deriveSuffix(ext string) string {
	// Some extensions begin with GL_ or EGL_ and then have the actual
	// extension like KHR, EXT etc.
	ext = strings.TrimPrefix(ext, "GL_")
	ext = strings.TrimPrefix(ext, "EGL_")
	return strings.Split(ext, "_")[0]
}

// addLine is a little helper function which handles the newline and tab
func addLine(str, line string) string {
	return str + SPACER + line + "\n"
}

func writeToFile(parent, file, content string) {
	p := filepath.Join(parent, file)
	if *dryRun {
		fmt.Printf("Writing to %s\n", p)
		fmt.Println(content)
	} else {
		if err := ioutil.WriteFile(p, []byte(content), 0644); err != nil {
			abort("Error while writing to file %s: %s", p, err)
		}
	}
}

// validationEntry is a helper struct that contains anything
// necessary to make validation code for a given standard.
type validationEntry struct {
	StandardCheck string
	GetReqs       RequirementGetter
}

func generateValidateInterface(features []FeatureSet) {
	standards := []validationEntry{
		{
			StandardCheck: "GR_IS_GR_GL(fStandard)",
			GetReqs:       glRequirements,
		}, {
			StandardCheck: "GR_IS_GR_GL_ES(fStandard)",
			GetReqs:       glesRequirements,
		}, /*{ Disable until ready to add WebGL support
			StandardCheck: "GR_IS_GR_WEB_GL(fStandard)",
			GetReqs: webglRequirements
		},*/
	}
	content := ""
	// For each feature, we are going to generate a series of
	// boolean expressions which check that the functions we thought
	// were gathered during the assemble phase actually were applied to
	// the interface (functionCheck). This check will be guarded
	// another set of if statements (one per standard) based
	// on the same requirements (standardChecks) that were used when
	// assembling the interface.
	for _, feature := range features {
		if allReqsAreCore(feature) {
			content += functionCheck(feature, 1)
		} else {
			content += SPACER
			standardChecks := []string{}
			for _, std := range standards {
				reqs := std.GetReqs(feature)
				if reqs == nil || len(reqs) == 0 {
					continue
				}
				expr := []string{}
				for _, r := range reqs {
					e := requirementIfExpression(r, false)
					if e != "" {
						expr = append(expr, e)
					}
				}
				check := ""
				if len(expr) == 0 {
					check = fmt.Sprintf("%s", std.StandardCheck)
				} else {
					lineBreak := "\n" + SPACER + "      "
					check = fmt.Sprintf("(%s && (%s%s))", std.StandardCheck, lineBreak, strings.Join(expr, " ||"+lineBreak))
				}
				standardChecks = append(standardChecks, check)
			}
			content += fmt.Sprintf("if (%s) {\n", strings.Join(standardChecks, " ||\n"+SPACER+"   "))
			content += functionCheck(feature, 2)

			content += SPACER + "}\n"
		}
		// add additional line between each block
		content += "\n"
	}
	content = strings.Replace(VALIDATE_INTERFACE, "[[content]]", content, 1)
	writeToFile(*outDir, INTERFACE_FILE_NAME, content)
}

// functionCheck returns an if statement that checks that all functions
// in the passed in slice are on the interface (that is, they are truthy
// on the fFunctions struct)
func functionCheck(feature FeatureSet, indentLevel int) string {
	// sort for determinism
	sort.Strings(feature.Functions)
	indent := strings.Repeat(SPACER, indentLevel)

	checks := []string{}
	for _, function := range feature.Functions {
		if in(function, feature.OptionalFunctions) {
			continue
		}
		if feature.EGLProc {
			checks = append(checks, "!fFunctions.fEGL"+function)
		} else {
			checks = append(checks, "!fFunctions.f"+function)
		}
	}
	testOnly := []string{}
	for _, function := range feature.TestOnlyFunctions {
		if in(function, feature.OptionalFunctions) {
			continue
		}
		if feature.EGLProc {
			testOnly = append(testOnly, "!fFunctions.fEGL"+function)
		} else {
			testOnly = append(testOnly, "!fFunctions.f"+function)
		}
	}
	for _, hcf := range feature.HardCodeFunctions {
		checks = append(checks, "!fFunctions."+hcf.PtrName)
	}
	preCheck := ""
	if len(testOnly) != 0 {
		preCheck = fmt.Sprintf(`#if GR_TEST_UTILS
%sif (%s) {
%s%sRETURN_FALSE_INTERFACE;
%s}
#endif
`, indent, strings.Join(testOnly, " ||\n"+indent+"    "), indent, SPACER, indent)
	}

	if len(checks) == 0 {
		return preCheck + strings.Repeat(SPACER, indentLevel) + "// all functions were marked optional or test_only\n"
	}

	return preCheck + fmt.Sprintf(`%sif (%s) {
%s%sRETURN_FALSE_INTERFACE;
%s}
`, indent, strings.Join(checks, " ||\n"+indent+"    "), indent, SPACER, indent)
}

// allReqsAreCore returns true iff the FeatureSet is part of "core" for
// all standards
func allReqsAreCore(feature FeatureSet) bool {
	if feature.GLReqs == nil || feature.GLESReqs == nil {
		return false
	}
	return feature.GLReqs[0] == CORE_REQUIREMENT && feature.GLESReqs[0] == CORE_REQUIREMENT
	// uncomment below when adding WebGL support
	// && feature.WebGLReqs[0] == CORE_REQUIREMENT
}

func validateFeatures(features []FeatureSet) {
	seen := map[string]bool{}
	for _, feature := range features {
		for _, fn := range feature.Functions {
			if seen[fn] {
				abort("ERROR: Duplicate function %s\n", fn)
			}
			seen[fn] = true
		}
		for _, fn := range feature.TestOnlyFunctions {
			if seen[fn] {
				abort("ERROR: Duplicate function %s\n", fn)
			}
			seen[fn] = true
		}
	}
}

// in returns true if |s| is *in* |a| slice.
func in(s string, a []string) bool {
	for _, x := range a {
		if x == s {
			return true
		}
	}
	return false
}

func abort(fmtStr string, inputs ...interface{}) {
	fmt.Printf(fmtStr+"\n", inputs...)
	os.Exit(1)
}

func main() {
	flag.Parse()
	b, err := ioutil.ReadFile(*inTable)
	if err != nil {
		abort("Could not read file %s", err)
	}

	dir, err := os.Open(*outDir)
	if err != nil {
		abort("Could not write to output dir %s", err)
	}
	defer dir.Close()
	if fi, err := dir.Stat(); err != nil {
		abort("Error getting info about %s: %s", *outDir, err)
	} else if !fi.IsDir() {
		abort("%s must be a directory", *outDir)
	}

	features := []FeatureSet{}

	err = json5.Unmarshal(b, &features)
	if err != nil {
		abort("Invalid JSON: %s", err)
	}

	validateFeatures(features)

	generateAssembleInterface(features)
	generateValidateInterface(features)
}
