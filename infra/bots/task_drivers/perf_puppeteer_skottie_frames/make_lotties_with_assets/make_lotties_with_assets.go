// Copyright 2021 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This executable is handy for transforming the structure of lottie-samples into
// the one used by perf_puppeteer_skottie_frames.go. It is stored to the
// skia/internal/lotties_with_assets CIPD package. If any lotties need assets, those should
// be copied in to the subfolders and appropriately named.
// A new version can be updated with
// cipd create -name skia/internal/lotties_with_assets -in ./lotties/ -tag version:NN
// where NN is the version number.
package main

import (
	"flag"
	"fmt"
	"os"
	"path/filepath"
	"strings"
)

func main() {
	inputDir := flag.String("input", "", "The input directory of lottie files")
	outputDir := flag.String("output", "", "The output directory which will have the correct structure")
	flag.Parse()

	xf, err := os.ReadDir(*inputDir)
	if err != nil {
		fmt.Printf("Could not get lotties from %s: %s\n", *inputDir, err)
		os.Exit(1)
	}
	for _, entry := range xf {
		inputJSON := entry.Name()
		if !strings.HasSuffix(inputJSON, ".json") {
			continue
		}
		newName := strings.TrimPrefix(inputJSON, "lottiefiles.com - ")
		newName = strings.TrimSuffix(newName, ".json")
		newName = strings.ReplaceAll(newName, " ", "_")

		subDir := filepath.Join(*outputDir, newName)
		if err := os.MkdirAll(subDir, 0755); err != nil {
			fmt.Printf("Could not make %s: %s\n", subDir, err)
			os.Exit(1)
		}

		outFile, err := os.Create(filepath.Join(subDir, "data.json"))
		if err != nil {
			fmt.Printf("Could not make output file in %s: %s\n", subDir, err)
			os.Exit(1)
		}

		inputBytes, err := os.ReadFile(filepath.Join(*inputDir, inputJSON))
		if err != nil {
			fmt.Printf("Could not read input file %s: %s\n", inputJSON, err)
			os.Exit(1)
		}
		if _, err := outFile.Write(inputBytes); err != nil {
			fmt.Printf("Could not copy bytes to %s: %s\n", subDir, err)
			os.Exit(1)
		}
	}
}
