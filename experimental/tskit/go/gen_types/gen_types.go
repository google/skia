// Copyright 2021 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package main

import (
	"fmt"
	"regexp"
	"sort"
	"strings"
)

func main() {
	fmt.Println("Not implemented yet")
}

var (
	privateExportLine    = regexp.MustCompile(`TS_PRIVATE_EXPORT\("(?P<export>.+)"\)`)
	publicExportLine     = regexp.MustCompile(`TS_EXPORT\("(?P<export>.+)"\)`)
	classDefinitionStart = regexp.MustCompile(`class_<.+>\("(?P<name>.+)"\)`)
	classDefinitionEnd   = regexp.MustCompile(`\.(function|constructor).+;`)
)

func generateAmbientNamespace(namespace, contents string) (string, error) {
	lines := strings.Split(contents, "\n")
	var privateModuleFunctions []string
	var publicModuleFunctions []string

	currentClass := ""
	for _, line := range lines {
		if match := classDefinitionStart.FindStringSubmatch(line); match != nil {
			currentClass = match[1]
		}
		if classDefinitionEnd.MatchString(line) {
			currentClass = ""
		}
		if match := privateExportLine.FindStringSubmatch(line); match != nil {
			if currentClass == "" {
				privateModuleFunctions = append(privateModuleFunctions, match[1])
			}
		}
		if match := publicExportLine.FindStringSubmatch(line); match != nil {
			if currentClass == "" {
				publicModuleFunctions = append(publicModuleFunctions, match[1])
			}
		}
	}

	sort.Strings(privateModuleFunctions)
	sort.Strings(publicModuleFunctions)

	output := fmt.Sprintf(`/// <reference path="embind.d.ts" />
declare namespace %s {
	export interface Bindings {
`, namespace)

	for _, export := range privateModuleFunctions {
		output += "\t\t"
		output += export
		if !strings.HasSuffix(export, ";") {
			output += ";"
		}
		output += "\n"
	}
	output += "\n"

	for _, export := range publicModuleFunctions {
		output += "\t\t"
		output += export
		if !strings.HasSuffix(export, ";") {
			output += ";"
		}
		output += "\n"
	}
	output += "\n"
	// TODO(kjlubick) constructors, constants

	output += "\t}"
	// TODO(kjlubick) interfaces

	output += "\n}\n"
	return output, nil
}
