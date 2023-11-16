// Copyright 2023 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// This executable for a given ICU implementation (from a hardcoded set)
// collects performance, memory and actual data for comparison in a simple internal format

package main

import (
	"flag"
	"fmt"
	"os"
	"path/filepath"
	"strconv"
	"strings"

	"go.skia.org/skia/tools/unicode_comparison/go/bridge"
	"go.skia.org/skia/tools/unicode_comparison/go/helpers"
)

func createEmptyFile(name string) error {
	d := []byte("")
	return os.WriteFile(name, d, 0644)
}

func walkRecursively(input string, output string) error {
	err := filepath.Walk(input,
		func(inputPath string, info os.FileInfo, err error) error {
			if err != nil {
				fmt.Println(err)
				return err
			}
			if info.IsDir() {
				outputPath := strings.Replace(inputPath, input, output, 1)
				fmt.Printf("%s -> %s\n", inputPath, outputPath)
				err := os.MkdirAll(outputPath, os.ModePerm)
				helpers.Check(err)
			} else {
				outputPath := strings.Replace(inputPath, input, output, 1)
				if err := writeInfo(inputPath, outputPath); err != nil {
					fmt.Println(err)
					return err
				}
			}
			return nil
		})
	return err
}

func writeInfo(inputPath string, outputPath string) error {

	// Read file
	fileContent, err := os.ReadFile(inputPath)
	helpers.Check(err)

	// Convert []byte to string
	text := string(fileContent)

	// Initialize file
	time := bridge.PerfComputeCodeunitFlags(text)

	outputFile, err := os.Create(outputPath)
	helpers.Check(err)

	// Collect info
	line := fmt.Sprintf("%.2f", time)
	_, err = outputFile.WriteString(line + "\n0.0\n")
	helpers.Check(err)

	graphemes := ""
	softBreaks := ""
	hardBreaks := ""
	whitespaces := ""
	words := ""
	controls := ""

	graphemesCount := 0
	softBreaksCount := 0
	hardBreaksCount := 0
	whitespacesCount := 0
	wordsCount := 0
	controlsCount := 0
	for i := 0; i <= len(fileContent); i++ {
		flags := bridge.GetFlags(i)
		str := bridge.FlagsToString(flags)
		pos := strconv.Itoa(i)
		if strings.Contains(str, "G") {
			graphemes += " "
			graphemes += pos
			graphemesCount += 1
		}
		if strings.Contains(str, "S") {
			softBreaks += " "
			softBreaks += pos
			softBreaksCount += 1
		}
		if strings.Contains(str, "H") {
			hardBreaks += " "
			hardBreaks += pos
			hardBreaksCount += 1
		}
		if strings.Contains(str, "W") {
			whitespaces += " "
			whitespaces += pos
			whitespacesCount += 1
		}
		if strings.Contains(str, "D") {
			words += " "
			words += pos
			wordsCount += 1
		}
		if strings.Contains(str, "C") {
			controls += " "
			controls += pos
			controlsCount += 1
		}
	}

	_, err = outputFile.WriteString(strconv.Itoa(graphemesCount) + graphemes + "\n")
	helpers.Check(err)

	_, err = outputFile.WriteString(strconv.Itoa(softBreaksCount) + softBreaks + "\n")
	helpers.Check(err)

	_, err = outputFile.WriteString(strconv.Itoa(hardBreaksCount) + hardBreaks + "\n")
	helpers.Check(err)

	_, err = outputFile.WriteString(strconv.Itoa(whitespacesCount) + whitespaces + "\n")
	helpers.Check(err)

	_, err = outputFile.WriteString(strconv.Itoa(wordsCount) + words + "\n")
	helpers.Check(err)

	_, err = outputFile.WriteString(strconv.Itoa(controlsCount) + controls + "\n")
	helpers.Check(err)

	outputFile.Close()

	return nil
}

func main() {
	var (
		root = flag.String("root", "~/datasets", "Folder (pages will be under <Folder>/input")
		impl = flag.String("impl", "", "Unicode Implementation")
	)
	flag.Parse()

	*root = helpers.ExpandPath(*root)
	input := filepath.Join(*root, "input")
	output := filepath.Join(*root, "output", *impl)
	if *root == "" {
		fmt.Println("Must set --root")
		flag.PrintDefaults()
	} else if *impl == "" {
		fmt.Println("Must set --impl")
		flag.PrintDefaults()
	}
	if !bridge.InitUnicode(*impl) {
		return
	}
	err := walkRecursively(input, output)
	defer bridge.CleanupUnicode()

	if err != nil {
		fmt.Println(err)
	}
}
