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
	"image"
	"image/png"
	"io/ioutil"
	"log"
	"net/http"
	"os"
	"os/exec"
	"path"
	"strconv"
	"strings"
	"sync"
)

const keysFileName = "keys.json"

var (
	workDir     = flag.String("work_dir", "/tmp/workdir", "Temporary location to dump the Gold JSON and pngs")
	port        = flag.String("port", "8081", "Port to listen on.")
	goldctlPath = flag.String("goldctl_path", "/opt/goldctl", "location of the goldctl binary")

	browser          = flag.String("browser", "Chrome", "Browser Key")
	buildBucketID    = flag.Int64("buildbucket_build_id", 0, "Buildbucket build id key")
	builder          = flag.String("builder", "", "Builder, like 'Test-Debian9-EMCC-GCE-CPU-AVX2-wasm-Debug-All-PathKit'")
	compiledLanguage = flag.String("compiled_language", "wasm", "wasm or asm.js")
	config           = flag.String("config", "Release", "Configuration (e.g. Debug/Release) key")
	gitHash          = flag.String("git_hash", "-", "The git commit hash of the version being tested")
	hostOS           = flag.String("host_os", "Debian9", "OS Key")
	instanceID       = flag.String("instance_id", "skia-legacy", "Gold instance to upload to")
	issue            = flag.Int64("issue", 0, "issue (if tryjob)")
	patchset         = flag.Int64("patchset", 0, "patchset (if tryjob)")
	sourceType       = flag.String("source_type", "pathkit", "Gold Source type, like pathkit,canvaskit")

	// auth related flags
	useLuci            = flag.Bool("use_luci", false, "If goldctl should be authenticated with the environment variable LUCI_CONTEXT. Used on the bots.")
	serviceAccountPath = flag.String("service_account_path", "", "The path to a service account to authenticate goldctl. Used for local testing.")
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

// goldctl is not thread-safe
var goldctlMutex sync.Mutex

func main() {
	flag.Parse()

	// check that goldctl is available
	if _, err := os.Open(*goldctlPath); err != nil {
		log.Fatalf("Cannot locate goldctl - did you specify it? %s\n%s", *goldctlPath, err)
		return
	}

	if !*useLuci && *serviceAccountPath == "" {
		log.Fatalf("You must set either --use_luci or --service_account_path")
		return
	}

	if err := writeInitialKeys(); err != nil {
		log.Fatalf("could not initialize goldctl - initial JSON error: %s", err)
		return
	}

	if err := authAndInitGoldCtl(); err != nil {
		log.Fatalf("could not initialize goldctl: %s", err)
		return
	}

	http.HandleFunc("/report_gold_data", reporter)
	http.HandleFunc("/finalize", finalize)

	fmt.Printf("Waiting for gold ingestion on port %s\n", *port)

	log.Fatal(http.ListenAndServe(":"+*port, nil))
}

func authAndInitGoldCtl() error {
	goldctlMutex.Lock()
	defer goldctlMutex.Unlock()
	var auth *exec.Cmd
	if *useLuci {
		return fmt.Errorf("not impl")
	} else {
		auth = exec.Command(*goldctlPath, "auth", "--work-dir", *workDir, "--service-account", *serviceAccountPath)
	}
	// reminder: CombinedOutput runs the command to completion
	if output, err := auth.CombinedOutput(); err != nil {
		return fmt.Errorf("Problem setting up auth: %s\n%s", err, string(output))
	} else {
		log.Printf("Output from goldctl auth: %s", string(output))
	}
	args := []string{
		"imgtest", "init", "--work-dir", *workDir, "--commit", *gitHash,
		"--keys-file", path.Join(*workDir, keysFileName), "--instance", *instanceID,
		"--upload-only",
		// The following will be 0 or empty for non-tryjobs, and thus, ignored
		"--jobid", ItoA(*buildBucketID), "--issue", ItoA(*issue), "--patchset", ItoA(*patchset),
	}
	init := exec.Command(*goldctlPath, args...)
	log.Printf("goldctl %s", strings.Join(args, " "))
	if output, err := init.CombinedOutput(); err != nil {

		return fmt.Errorf("Problem running imgtest init: %s\n%s", err, string(output))
	} else {
		log.Printf("Output from goldctl imgtest init: %s\n", string(output))
	}
	return nil
}

func ItoA(i int64) string {
	return strconv.FormatInt(i, 10)
}

func writeInitialKeys() error {
	p := path.Join(*workDir, keysFileName)
	outputFile, err := createOutputFile(p)
	defer outputFile.Close()
	if err != nil {
		return fmt.Errorf("Could not open json file on disk: %s", err)
	}

	cpuGPU := "CPU"
	if strings.Index(*builder, "-GPU-") != -1 {
		cpuGPU = "GPU"
	}
	// The keys to be used at the top level for all Results.
	defaultKeys := map[string]string{
		"arch":              "WASM",
		"browser":           *browser,
		"compiled_language": *compiledLanguage,
		"compiler":          "emsdk",
		"configuration":     *config,
		"cpu_or_gpu":        cpuGPU,
		"cpu_or_gpu_value":  "Browser",
		"os":                *hostOS,
		"source_type":       *sourceType,
	}

	enc := json.NewEncoder(outputFile)
	enc.SetIndent("", "  ") // Make it human readable.
	if err := enc.Encode(defaultKeys); err != nil {
		return fmt.Errorf("Could not write json to disk: %s", err)
	}
	return nil
}

// reporter handles when the client reports a test has Gold output.
// It writes the corresponding PNG to disk and appends a Result, assuming
// no errors.
func reporter(w http.ResponseWriter, r *http.Request) {
	if r.Method != "POST" {
		http.Error(w, "Only POST accepted", http.StatusBadRequest)
		return
	}
	defer r.Body.Close()

	body, err := ioutil.ReadAll(r.Body)
	if err != nil {
		http.Error(w, "Malformed body", http.StatusBadRequest)
		return
	}

	testOutput := reportBody{}
	if err := json.Unmarshal(body, &testOutput); err != nil {
		fmt.Println(err)
		http.Error(w, "Could not unmarshal JSON", http.StatusBadRequest)
		return
	}

	pngPath := ""
	if pngPath, err = writeBase64EncodedPNG(testOutput.Data); err != nil {
		fmt.Println(err)
		http.Error(w, "Could not write image to disk", http.StatusInternalServerError)
		return
	}

	goldctlMutex.Lock()
	defer goldctlMutex.Unlock()
	args := []string{
		"imgtest", "add", "--work-dir", *workDir,
		"--dryrun",
		"--add-test-key", "config:" + testOutput.OutputType,
		"--test-name", testOutput.TestName,
		"--png-file", pngPath,
	}
	init := exec.Command(*goldctlPath, args...)
	log.Printf("goldctl %s", strings.Join(args, " "))
	if output, err := init.CombinedOutput(); err != nil {
		log.Printf("Problem running imgtest add: %s\n%s", err, string(output))
		http.Error(w, "Could not report image", http.StatusInternalServerError)
		return
	} else {
		log.Printf("Output from goldctl imgtest add: %s\n", string(output))
	}

	if _, err := w.Write([]byte("Accepted")); err != nil {
		fmt.Printf("Could not write response: %s\n", err)
	}
}

// createOutputFile creates a file and set permissions correctly.
func createOutputFile(p string) (*os.File, error) {
	outputFile, err := os.Create(p)
	if err != nil {
		return nil, fmt.Errorf("Could not open file %s on disk, does its parent directory exist?: %s", p, err)
	}
	// Make this accessible (and deletable) by all users
	if err = outputFile.Chmod(0666); err != nil {
		return nil, fmt.Errorf("Could not change permissions of file %s: %s", p, err)
	}
	return outputFile, nil
}

// finalize calls goldctl imgtest finalize, which uploads the JSON file
// with all the tests in it.
func finalize(w http.ResponseWriter, r *http.Request) {
	if r.Method != "POST" {
		http.Error(w, "Only POST accepted", http.StatusBadRequest)
		return
	}
	goldctlMutex.Lock()
	defer goldctlMutex.Unlock()
	args := []string{
		"imgtest", "finalize", "--work-dir", *workDir,
		"--dryrun",
	}
	init := exec.Command(*goldctlPath, args...)
	log.Printf("goldctl %s", strings.Join(args, " "))
	if output, err := init.CombinedOutput(); err != nil {
		log.Printf("Problem running imgtest finalize: %s\n%s", err, string(output))
		http.Error(w, "Could not report JSON", http.StatusInternalServerError)
		return
	} else {
		log.Printf("Output from goldctl imgtest finalize: %s\n", string(output))
	}

	if _, err := w.Write([]byte("Accepted")); err != nil {
		fmt.Printf("Could not write response: %s\n", err)
	}
}

// writeBase64EncodedPNG writes a PNG to disk and the path to the written file and any error.
// The filename is based on the md5 hash of the pixels. goldctl will re-hash it for the "official"
// hash for use in Gold, which may not be the same as this one.
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

	p := path.Join(*workDir, hash+".png")
	outputFile, err := createOutputFile(p)
	defer outputFile.Close()
	if err != nil {
		return "", fmt.Errorf("Could not create png file %s: %s", p, err)
	}
	if _, err = outputFile.Write(pngBytes); err != nil {
		return "", fmt.Errorf("Could not write to file %s: %s", p, err)
	}
	return p, nil
}
