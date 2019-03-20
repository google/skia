// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package main

import (
	"flag"
	"fmt"
	"io/ioutil"
	"os"
	"path/filepath"
	"strings"

	"github.com/flynn/json5"
)

var outDir = flag.String("out_dir", "../../src/gpu/gl", "Where to output the GrGlAssembleInterface_* and GrGlInterface.cpp files")
var inTable = flag.String("in_table", "./interface.json5", "The JSON5 table to read in")

const CORE_FEATURE = "<core>"
const TAB = "    "

type FeatureSet struct {
	GLReqs    []Requirement `json:"GL"`
	GLESReqs  []Requirement `json:"GLES"`
	WebGLReqs []Requirement `json:"WebGL"`

	Functions []string `json:"functions"`
	Required  bool     `json:"required"`
}

type Requirement struct {
	Extension  string     `json:"ext"`
	MinVersion *GLVersion `json:"min_version"`
}

type GLVersion [2]int

func generateAssembleInterface(features []FeatureSet) {
	gles := fillAssembleTemplate(ASSEMBLE_INTERFACE_GL_ES, features, func(fs FeatureSet) []Requirement {
		return fs.GLESReqs
	})
	writeToFile(*outDir, "GrGLAssembleInterface_gles_gen.cpp", gles)
	gl := fillAssembleTemplate(ASSEMBLE_INTERFACE_GL, features, func(fs FeatureSet) []Requirement {
		return fs.GLReqs
	})
	writeToFile(*outDir, "GrGLAssembleInterface_gl_gen.cpp", gl)
}

func fillAssembleTemplate(template string, features []FeatureSet, getReqs func(FeatureSet) []Requirement) string {
	content := ""
	for _, feature := range features {
		reqs := getReqs(feature)
		for idx, req := range reqs {
			ifStart := makeIfBlock(req, idx+1, len(reqs))

			if ifStart != "" {
				if idx == 0 {
					content = addLine(content, ifStart)
				} else {
					content = content + ifStart + "\n"
				}
			}

			for _, function := range feature.Functions {
				if ifStart != "" {
					// extra tab
					content += TAB
				}
				suffix := deriveSuffix(req.Extension)
				// Some ARB extensions don't have ARB suffixes because they were written for backwards
				// compatibility simultaneous to adding them as required in a new GL version.
				if req.Extension == CORE_FEATURE || suffix == "ARB" {
					content = addLine(content, fmt.Sprintf("GET_PROC(%s);", function))
				} else {
					content = addLine(content, fmt.Sprintf("GET_PROC_SUFFIX(%s, %s);", function, suffix))
				}
			}

			ifEnd := "}"
			if idx == len(reqs)-1 {
				if ifStart != "" {
					content = addLine(content, ifEnd)
				}
				// Add an extra newline between blocks for easier readability
				content += "\n"
			} else {
				content = content + TAB + ifEnd + " else "
			}
		}
	}

	return strings.Replace(template, "[[content]]", content, 1)
}

func makeIfBlock(req Requirement, curBlock, totalBlocks int) string {
	mv := req.MinVersion
	if req.Extension == CORE_FEATURE && mv == nil {
		if totalBlocks != 1 {
			abort("Core feature with no Min Version, but wasn't only requirement")
		}
		return ""
	}
	if req.Extension == CORE_FEATURE {
		if curBlock < totalBlocks {
			return fmt.Sprintf("if (version >= GR_GL_VER(%d,%d)) {", mv[0], mv[1])
		}
	}
	// We know it has an extension
	if mv == nil {
		return fmt.Sprintf("if (extensions.has(%q)) {", req.Extension)
	} else {
		return fmt.Sprintf("if (version >= GR_GL_VER(%d,%d) && extensions.has(%q)) {", mv[0], mv[1], req.Extension)
	}
}

func deriveSuffix(ext string) string {
	ext = strings.TrimPrefix(ext, "GL_")
	return strings.Split(ext, "_")[0]
}

func addLine(str, line string) string {
	return str + TAB + line + "\n"
}

func writeToFile(parent, file, content string) {
	p := filepath.Join(parent, file)
	f, err := os.Create(p)
	if err != nil {
		abort(fmt.Sprintf("Could not open file %s for writing %s", p, err))
	}
	defer f.Close()
	if _, err = f.WriteString(content); err != nil {
		abort(fmt.Sprintf("Error while writing to file %s: %s", p, err))
	}
}

func generateValidateInterface(features []FeatureSet) {
	content := ""
	for _, feature := range features {
		tabDepth := 1
		// generate standard check && extension check - skip if required
		if feature.Required {
			indent := strings.Repeat(TAB, tabDepth)
			content += indent + "if ("
			for idx, function := range feature.Functions {
				if idx != 0 {
					// Add an indent and some spacing to make it line up
					content += indent + "    "
				}
				content += "!fFunctions." + function
				if idx != len(feature.Functions)-1 {
					content += " ||\n"
				} else {
					content += ") {\n"
				}
			}
			content += indent + TAB + "RETURN_FALSE_INTERFACE;" + "\n"
			content += indent + "}\n"
		}
		//
	}
	fmt.Println(content)
}

func main() {
	flag.Parse()
	b, err := ioutil.ReadFile(*inTable)
	if err != nil {
		abort(fmt.Sprintf("Could not read file %s", err))
	}

	dir, err := os.Open(*outDir)
	if err != nil {
		abort(fmt.Sprintf("Could not write to output dir %s", err))
	}
	defer dir.Close()
	if fi, err := dir.Stat(); err != nil {
		abort(fmt.Sprintf("Error getting info about %s: %s", *outDir, err))
	} else if !fi.IsDir() {
		abort(fmt.Sprintf("%s must be a directory", *outDir))
	}

	features := []FeatureSet{}

	err = json5.Unmarshal(b, &features)

	//generateAssembleInterface(features)
	generateValidateInterface(features)
}

func abort(msg string) {
	fmt.Printf("%s\n", msg)
	os.Exit(1)
}
