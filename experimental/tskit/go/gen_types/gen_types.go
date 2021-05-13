// Copyright 2021 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package main

import (
	"flag"
	"fmt"
	"io/ioutil"
	"path/filepath"
	"regexp"
	"sort"
	"strings"

	"go.skia.org/infra/go/skerr"
	"go.skia.org/infra/go/sklog"
)

func main() {
	inputCPPDir := flag.String("input_cpp_dir", "", "A folder containing .cpp binding files that make use of the TS_* macros")
	outputNamespaceDir := flag.String("output_namespace_dir", "", "The folder where the ambient namespace (.d.ts) files that correspond to the C++ API should be written")

	flag.Parse()
	if *inputCPPDir == "" || *outputNamespaceDir == "" {
		sklog.Fatalf("--input_cpp_dir and --output_namespace_dir must be specified")
	}

	cppInputs, err := ioutil.ReadDir(*inputCPPDir)
	if err != nil {
		sklog.Fatalf("Could not read directory %s: %s", *inputCPPDir, err)
	}
	for _, file := range cppInputs {
		name := file.Name()
		if strings.HasSuffix(name, ".cpp") {
			fp := filepath.Join(*inputCPPDir, name)
			contents, err := ioutil.ReadFile(fp)
			if err != nil {
				sklog.Fatalf("Could not read file %s: %s", fp, err)
			}
			namespace := strings.TrimSuffix(name, ".cpp")
			output, err := generateAmbientNamespace(namespace, string(contents))
			if err != nil {
				sklog.Fatalf("Could not generate ambient namespace from %s: %s", fp, err)
			}
			fp = filepath.Join(*outputNamespaceDir, namespace+".d.ts")
			if err := ioutil.WriteFile(fp, []byte(output), 0666); err != nil {
				sklog.Fatalf("Could not write to %s: %s", fp, err)
			}
		}
	}
}

var (
	privateExportLine    = regexp.MustCompile(`TS_PRIVATE_EXPORT\("(?P<export>.+)"\)`)
	publicExportLine     = regexp.MustCompile(`TS_EXPORT\("(?P<export>.+)"\)`)
	classDefinitionStart = regexp.MustCompile(`class_<.+>\("(?P<name>.+)"\)`)
	classDefinitionEnd   = regexp.MustCompile(`\.(function|constructor).+;`)
	typeAnnotation       = regexp.MustCompile(`@type\s+(?P<optional>@optional)?\s*(?P<type>\w+)`)
	valueObjectStart     = regexp.MustCompile(`value_object<.+>\("(?P<name>.+)"\)`)
	valueObjectField     = regexp.MustCompile(`.field\("(?P<name>.+?)",.+\);?`)
	constantDefinition   = regexp.MustCompile(`constant\("(?P<name>.+?)",.+\);`)
)

// generateAmbientNamespace reads through the given file contents line by line. It looks for
// explicit macro annotations (e.g. TS_EXPORT) and embind signatures (e.g. constant("foo", "bar").
// From these, it generates the ambient namespace declarations and returns it as a string.
// In general, it will put declarations in alphabetical order for deterministic ordering.
func generateAmbientNamespace(namespace, contents string) (string, error) {
	var privateModuleFunctions []string
	var publicModuleFunctions []string
	var wasmObjects []*wasmObject
	var valueObjects []*valueObject
	var constants []string

	var currentWasmObject *wasmObject
	var currentValueObject *valueObject
	var lastTypeAnnotation string
	var lastTypeOptional bool
	lines := strings.Split(contents, "\n")
	for i, line := range lines {
		if match := classDefinitionStart.FindStringSubmatch(line); match != nil {
			currentWasmObject = &wasmObject{
				name: match[1],
			}
			wasmObjects = append(wasmObjects, currentWasmObject)
		}
		if classDefinitionEnd.MatchString(line) {
			currentWasmObject = nil
		}
		if match := privateExportLine.FindStringSubmatch(line); match != nil {
			export := match[1]
			if currentWasmObject == nil {
				privateModuleFunctions = append(privateModuleFunctions, export)
			} else {
				currentWasmObject.privateMethods = append(currentWasmObject.privateMethods, export)
			}
		}
		if match := publicExportLine.FindStringSubmatch(line); match != nil {
			export := match[1]
			if currentWasmObject == nil {
				publicModuleFunctions = append(publicModuleFunctions, export)
			} else {
				if strings.HasPrefix(export, "new") {
					currentWasmObject.constructors = append(currentWasmObject.constructors, export)
				} else {
					currentWasmObject.publicMethods = append(currentWasmObject.publicMethods, export)
				}
			}
		}
		if match := typeAnnotation.FindStringSubmatch(line); match != nil {
			lastTypeOptional = match[1] != ""
			lastTypeAnnotation = match[2]
		}
		if match := valueObjectStart.FindStringSubmatch(line); match != nil {
			currentValueObject = &valueObject{
				name: match[1],
			}
			valueObjects = append(valueObjects, currentValueObject)
		}
		if match := valueObjectField.FindStringSubmatch(line); match != nil {
			if currentValueObject == nil {
				// Should never happen for valid code - embind wouldn't compile with this.
				return "", skerr.Fmt("Line %d: Found a .field outside a value object declaration", i+1)
			}
			if lastTypeAnnotation == "" {
				return "", skerr.Fmt("Line %d: field %q must be preceded by a @type annotation", i+1, match[1])
			}
			fieldWithType := fmt.Sprintf("%s: %s", match[1], lastTypeAnnotation)
			if lastTypeOptional {
				fieldWithType = fmt.Sprintf("%s?: %s", match[1], lastTypeAnnotation)
			}
			currentValueObject.fields = append(currentValueObject.fields, fieldWithType)
			lastTypeAnnotation = "" // It's been consumed, so reset it
			lastTypeOptional = false
			if strings.HasSuffix(match[0], ";") {
				currentValueObject = nil // we've gotten to the end of our value object
			}
		}
		if match := constantDefinition.FindStringSubmatch(line); match != nil {
			if lastTypeAnnotation == "" {
				return "", skerr.Fmt("Line %d: constant %q must be preceded by a @type annotation", i+1, match[1])
			}
			constWithType := fmt.Sprintf("readonly %s: %s", match[1], lastTypeAnnotation)
			if lastTypeOptional {
				constWithType = fmt.Sprintf("readonly %s?: %s", match[1], lastTypeAnnotation)
			}
			constants = append(constants, constWithType)
			lastTypeAnnotation = "" // It's been consumed, so reset it
			lastTypeOptional = false
		}
	}

	sort.Strings(privateModuleFunctions)
	sort.Strings(publicModuleFunctions)
	sort.Slice(wasmObjects, func(i, j int) bool {
		return wasmObjects[i].name < wasmObjects[j].name
	})
	sort.Slice(valueObjects, func(i, j int) bool {
		return valueObjects[i].name < valueObjects[j].name
	})
	sort.Strings(constants)

	output := fmt.Sprintf(`/// <reference path="embind.d.ts" />
/* This file is autogenerated using gen_types.go and make generate */
declare namespace %s {
	export interface Bindings {
`, namespace)

	for _, export := range privateModuleFunctions {
		output += fmt.Sprintf("\t\t%s\n", ensureSemicolon(export))
	}
	output += "\n"

	for _, export := range publicModuleFunctions {
		output += fmt.Sprintf("\t\t%s\n", ensureSemicolon(export))
	}
	output += "\n"

	for _, obj := range wasmObjects {
		output += fmt.Sprintf("\t\treadonly %s: %sConstructor;\n", obj.name, obj.name)
	}
	output += "\n"

	for _, c := range constants {
		output += fmt.Sprintf("\t\t%s;\n", c)
	}

	output += "\t}\n\n"

	// The constructors for all exposed objects.
	for _, obj := range wasmObjects {
		sort.Strings(obj.constructors)
		output += fmt.Sprintf("\texport interface %sConstructor {\n", obj.name)
		for _, c := range obj.constructors {
			output += fmt.Sprintf("\t\t%s\n", ensureSemicolon(c))
		}
		output += "\t}\n\n"
	}
	// The exposed objects themselves
	for _, obj := range wasmObjects {
		sort.Strings(obj.privateMethods)
		sort.Strings(obj.publicMethods)
		output += fmt.Sprintf("\texport interface %s extends embind.EmbindObject<%s> {\n", obj.name, obj.name)
		for _, m := range obj.privateMethods {
			output += fmt.Sprintf("\t\t%s\n", ensureSemicolon(m))
		}
		if len(obj.privateMethods) > 0 {
			output += "\n"
		}
		for _, m := range obj.publicMethods {
			output += fmt.Sprintf("\t\t%s\n", ensureSemicolon(m))
		}
		output += "\t}\n\n"
	}

	for _, obj := range valueObjects {
		sort.Strings(obj.fields)
		output += fmt.Sprintf("\texport interface %s {\n", obj.name)
		for _, f := range obj.fields {
			output += fmt.Sprintf("\t\t%s,\n", f)
		}
		output += "\t}\n\n"
	}

	output = strings.TrimSuffix(output, "\n")
	output += "}\n"
	return output, nil
}

func ensureSemicolon(js string) string {
	if !strings.HasSuffix(js, ";") {
		return js + ";"
	}
	return js
}

type wasmObject struct {
	name           string
	constructors   []string
	publicMethods  []string
	privateMethods []string
}

type valueObject struct {
	name   string
	fields []string
}
