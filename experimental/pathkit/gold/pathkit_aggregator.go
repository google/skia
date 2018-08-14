// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package main

// This server runs along side the karma tests and listens for POST requests
// when any test case reports it has output for Gold. See testReporter.js
// for the browser side part.

import (
	"bytes"
	"crypto/md5"
	"encoding/base64"
	"encoding/json"
	"flag"
	"fmt"
	"io/ioutil"
	"log"
	"net/http"
	"os"
	"path"
	"strings"
)

// These files allow us to use upload_dm_results.py out of the box
const JSON_FILENAME = "dm.json"
const LOG_FILENAME = "verbose.log"

var (
	outDir = flag.String("out_dir", "/OUT/", "location to dump the Gold JSON and pngs")
	port   = flag.String("port", "8081", "Port to listen on.")

	botId            = flag.String("bot_id", "", "swarming bot id")
	browser          = flag.String("browser", "Chrome", "Browser Key")
	buildBucketID    = flag.Int64("buildbucket_build_id", 0, "Buildbucket build id key")
	builder          = flag.String("builder", "", "Builder, like 'Test-Debian9-EMCC-GCE-CPU-AVX2-wasm-Debug-All-PathKit'")
	compiledLanguage = flag.String("compiled_language", "wasm", "wasm or asm.js")
	config           = flag.String("config", "Release", "Configuration (e.g. Debug/Release) key")
	gitHash          = flag.String("git_hash", "-", "The git commit hash of the version being tested")
	hostOS           = flag.String("host_os", "Debian9", "OS Key")
	issue            = flag.Int64("issue", 0, "issue (if tryjob)")
	patch_storage    = flag.String("patch_storage", "", "patch storage (if tryjob)")
	patchset         = flag.Int64("patchset", 0, "patchset (if tryjob)")
	taskId           = flag.String("task_id", "", "swarming task id")
)

// Received from the JS side.
type reportBody struct {
	// e.g. "canvas" or "svg"
	OutputType string `json:"output_type"`
	// a base64 encoded PNG image.
	Data string `json:"data"`
	// a name describing the test. Should be unique enough to allow use of grep.
	TestName string `json:"test_name"`
}

// The keys to be used at the top level for all Results.
var defaultKeys map[string]string

// contains all the results reported in through report_gold_data
var results []*Result

// Copied from $INFRA_ROOT/golden/go/goldingestion/common.go
// Copied to simplify the Dockerfile and runtime dependencies

type Result struct {
	Key     map[string]string `json:"key"`
	Options map[string]string `json:"options"`
	Digest  string            `json:"md5"`
}

type DMResults struct {
	Builder        string            `json:"builder"`
	GitHash        string            `json:"gitHash"`
	Key            map[string]string `json:"key"`
	Issue          int64             `json:"issue,string"`
	Patchset       int64             `json:"patchset,string"`
	Results        []*Result         `json:"results"`
	PatchStorage   string            `json:"patch_storage"`
	SwarmingBotID  string            `json:"swarming_bot_id"`
	SwarmingTaskID string            `json:"swarming_task_id"`
	BuildBucketID  int64             `json:"buildbucket_build_id,string"`

	// name is the name/path of the file where this came from.
	name string
}

func main() {
	flag.Parse()

	defaultKeys = map[string]string{
		"arch":              "WASM",
		"browser":           *browser,
		"compiled_language": *compiledLanguage,
		"compiler":          "emsdk",
		"configuration":     *config,
		"cpu_or_gpu":        "CPU",
		"cpu_or_gpu_value":  "Browser",
		"os":                *hostOS,
		"source_type":       "pathkit",
	}

	results = []*Result{}

	http.HandleFunc("/report_gold_data", reporter)
	http.HandleFunc("/dump_json", dumpJSON)

	fmt.Printf("Waiting for gold ingestion on port %s\n", *port)

	log.Fatal(http.ListenAndServe(":"+*port, nil))
}

// reporter handles when the client reports a test has Gold output.
// It writes the corresponding PNG to disk and appends a Result, assuming
// no errors.
func reporter(w http.ResponseWriter, r *http.Request) {
	if r.Method != "POST" {
		http.Error(w, "Only POST accepted", 400)
		return
	}
	defer r.Body.Close()

	body, err := ioutil.ReadAll(r.Body)
	if err != nil {
		http.Error(w, "Malformed body", 400)
		return
	}

	testOutput := reportBody{}
	if err := json.Unmarshal(body, &testOutput); err != nil {
		fmt.Println(err)
		http.Error(w, "Could not unmarshal JSON", 400)
		return
	}

	hash := ""
	if hash, err = writeBase64EncodedPNG(testOutput.Data); err != nil {
		fmt.Println(err)
		http.Error(w, "Could not write image to disk", 500)
		return
	}

	if _, err := w.Write([]byte("Accepted")); err != nil {
		fmt.Printf("Could not write response: %s\n", err)
		return
	}

	results = append(results, &Result{
		Digest: hash,
		Key: map[string]string{
			"name":   testOutput.TestName,
			"config": testOutput.OutputType,
		},
		Options: map[string]string{
			"ext": "png",
		},
	})
}

// createOutputFile creates a file and set permissions correctly.
func createOutputFile(p string) (*os.File, error) {
	outputFile, err := os.Create(p)
	if err != nil {
		return nil, fmt.Errorf("Could not open file %s on disk: %s", p, err)
	}
	// Make this accessible (and deletable) by all users
	if err = outputFile.Chmod(0666); err != nil {
		return nil, fmt.Errorf("Could not change permissions of file %s: %s", p, err)
	}
	return outputFile, nil
}

// dumpJSON writes out a JSON file with all the results, typically at the end of
// all the tests.
func dumpJSON(w http.ResponseWriter, r *http.Request) {
	if r.Method != "POST" {
		http.Error(w, "Only POST accepted", 400)
		return
	}

	p := path.Join(*outDir, JSON_FILENAME)
	outputFile, err := createOutputFile(p)
	defer outputFile.Close()
	if err != nil {
		fmt.Println(err)
		http.Error(w, "Could not open json file on disk", 500)
		return
	}

	results := DMResults{
		BuildBucketID:  *buildBucketID,
		Builder:        *builder,
		GitHash:        *gitHash,
		Issue:          *issue,
		Key:            defaultKeys,
		PatchStorage:   *patch_storage,
		Patchset:       *patchset,
		Results:        results,
		SwarmingBotID:  *botId,
		SwarmingTaskID: *taskId,
	}

	enc := json.NewEncoder(outputFile)
	enc.SetIndent("", "  ") // Make it human readable.
	if err := enc.Encode(&results); err != nil {
		fmt.Println(err)
		http.Error(w, "Could not write json to disk", 500)
		return
	}
	fmt.Println("JSON Written")

	// Create verbose.log with a few characters in it.
	p = path.Join(*outDir, LOG_FILENAME)
	logFile, err := createOutputFile(p)
	if err != nil {
		fmt.Println(err)
		http.Error(w, "Could not open log file on disk", 500)
		return
	}
	if _, err := logFile.WriteString("done"); err != nil {
		fmt.Println(err)
		http.Error(w, "Could not write to logfile on disk", 500)
		return
	}
}

// writeBase64EncodedPNG writes a PNG to disk and returns the md5 of the
// decoded PNG bytes and any error. This hash is what will be used as
// the gold digest and the file name.
func writeBase64EncodedPNG(data string) (string, error) {
	// data starts with something like data:image/png;base64,[data]
	// https://en.wikipedia.org/wiki/Data_URI_scheme
	start := strings.Index(data, ",")
	bytes := bytes.NewBufferString(data[start+1:])
	pngReader := base64.NewDecoder(base64.StdEncoding, bytes)

	pngBytes, err := ioutil.ReadAll(pngReader)
	if err != nil {
		return "", fmt.Errorf("Could not decode base 64 encoding %s", err)
	}
	hash := fmt.Sprintf("%x", md5.Sum(pngBytes))

	p := path.Join(*outDir, hash+".png")
	outputFile, err := createOutputFile(p)
	defer outputFile.Close()
	if err != nil {
		return "", fmt.Errorf("Could not create png file %s: %s", p, err)
	}
	if _, err = outputFile.Write(pngBytes); err != nil {
		return "", fmt.Errorf("Could not write to file %s: %s", p, err)
	}
	return hash, nil
}
