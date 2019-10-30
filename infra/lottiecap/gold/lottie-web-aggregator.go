// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package main

// This server runs alongside lottiecap.js and istens for POST requests
// when any test case reports it has output for Gold.

// TODO(kjlubick): Deduplicate with pathkit-aggregator
// TODO(kjlubick): Handle uninteresting_hash.txt if needed.

import (
	"bytes"
	"crypto/md5"
	"encoding/base64"
	"encoding/json"
	"flag"
	"fmt"
	"image"
	"image/png"
	"io/ioutil"
	"log"
	"net/http"
	"os"
	"path"
	"strings"

	"go.skia.org/infra/golden/go/jsonio"
	"go.skia.org/infra/golden/go/types"
)

// This allows us to use upload_dm_results.py out of the box
const JSON_FILENAME = "dm.json"

var (
	outDir = flag.String("out_dir", "/OUT/", "location to dump the Gold JSON and pngs")
	port   = flag.String("port", "8081", "Port to listen on.")

	browser       = flag.String("browser", "Chrome", "Browser Key")
	buildBucketID = flag.String("buildbucket_build_id", "", "Buildbucket build id key")
	builder       = flag.String("builder", "", "Builder, like 'Test-Debian9-EMCC-GCE-CPU-AVX2-wasm-Debug-All-PathKit'")
	renderer      = flag.String("renderer", "lottie-web", "e.g. lottie-web or skottie")
	config        = flag.String("config", "Release", "Configuration (e.g. Debug/Release) key")
	gitHash       = flag.String("git_hash", "-", "The git commit hash of the version being tested")
	hostOS        = flag.String("host_os", "Debian9", "OS Key")
	issue         = flag.String("issue", "", "ChangeListID (if tryjob)")
	patchset      = flag.Int("patchset", 0, "patchset (if tryjob)")
	taskId        = flag.String("task_id", "", "Skia task id")
)

// reportBody is the JSON recieved from the JS side. It represents
// exactly one unique Gold image/test.
type reportBody struct {
	// a base64 encoded PNG image.
	Data string `json:"data"`
	// a name describing the test. Should be unique enough to allow use of grep.
	TestName string `json:"test_name"`
}

// The keys to be used at the top level for all Results.
var defaultKeys map[string]string

// contains all the results reported in through report_gold_data
var results []*jsonio.Result

func main() {
	flag.Parse()

	defaultKeys = map[string]string{
		"browser":          *browser,
		"renderer":         *renderer,
		"configuration":    *config,
		"cpu_or_gpu":       "CPU",
		"cpu_or_gpu_value": "Browser",
		"os":               *hostOS,
		"source_type":      "lottie",
	}

	results = []*jsonio.Result{}

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

	results = append(results, &jsonio.Result{
		Digest: types.Digest(hash),
		Key: map[string]string{
			"name": testOutput.TestName,
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

	results := jsonio.GoldResults{
		Builder:                     *builder,
		ChangeListID:                *issue,
		GitHash:                     *gitHash,
		CodeReviewSystem:            "gerrit",
		Key:                         defaultKeys,
		PatchSetOrder:               *patchset,
		Results:                     results,
		TaskID:                      *taskId,
		TryJobID:                    *buildBucketID,
		ContinuousIntegrationSystem: "buildbucket",
	}

	enc := json.NewEncoder(outputFile)
	enc.SetIndent("", "  ") // Make it human readable.
	if err := enc.Encode(&results); err != nil {
		fmt.Println(err)
		http.Error(w, "Could not write json to disk", 500)
		return
	}
	fmt.Println("JSON Written")
}

// writeBase64EncodedPNG writes a PNG to disk and returns the md5 of the
// decoded PNG bytes and any error. This hash is what will be used as
// the gold digest and the file name.
func writeBase64EncodedPNG(data string) (string, error) {
	// data starts with something like data:image/png;base64,[data]
	// https://en.wikipedia.org/wiki/Data_URI_scheme
	start := strings.Index(data, ",")
	b := bytes.NewBufferString(data[start+1:])
	pngReader := base64.NewDecoder(base64.StdEncoding, b)

	pngBytes, err := ioutil.ReadAll(pngReader)
	if err != nil {
		return "", fmt.Errorf("Could not decode base 64 encoding %s", err)
	}

	// compute the hash of the pixel values, like DM does
	img, err := png.Decode(bytes.NewBuffer(pngBytes))
	if err != nil {
		return "", fmt.Errorf("Not a valid png: %s", err)
	}
	hash := ""
	switch img.(type) {
	case *image.NRGBA:
		i := img.(*image.NRGBA)
		hash = fmt.Sprintf("%x", md5.Sum(i.Pix))
	case *image.RGBA:
		i := img.(*image.RGBA)
		hash = fmt.Sprintf("%x", md5.Sum(i.Pix))
	case *image.RGBA64:
		i := img.(*image.RGBA64)
		hash = fmt.Sprintf("%x", md5.Sum(i.Pix))
	default:
		return "", fmt.Errorf("Unknown type of image")
	}

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
