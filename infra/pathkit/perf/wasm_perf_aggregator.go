// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package main

// This server runs along side the karma tests and listens for POST requests
// when any test case reports it has output for Perf. See perfReporter.js
// for the browser side part.

// Unlike the gold ingester, the perf ingester allows multiple reports
// of the same benchmark and will output the average of these results
// on a call to dump

import (
	"encoding/json"
	"flag"
	"fmt"
	"io/ioutil"
	"log"
	"net/http"
	"os"
	"path"
	"strconv"
	"strings"
	"sync"

	"github.com/google/uuid"
	"go.skia.org/infra/perf/go/ingestcommon"
)

// upload_nano_results looks for anything*.json
// We add the random UUID to avoid name clashes when uploading to
// the perf bucket (which uploads to folders based on Month/Day/Hour, which can
// easily have duplication if multiple perf tasks run in an hour.)
var JSON_FILENAME = fmt.Sprintf("%s_browser_bench.json", uuid.New().String())

var (
	outDir = flag.String("out_dir", "/OUT/", "location to dump the Perf JSON")
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
	sourceType       = flag.String("source_type", "pathkit", "Gold Source type, like pathkit,canvaskit")
)

// Received from the JS side.
type reportBody struct {
	// a name describing the benchmark. Should be unique enough to allow use of grep.
	BenchName string `json:"bench_name"`
	// The number of microseconds of the task.
	TimeMicroSeconds float64 `json:"time_us"`
}

// The keys to be used at the top level for all Results.
var defaultKeys map[string]string

// contains all the results reported in through report_perf_data
var results map[string][]reportBody
var resultsMutex sync.Mutex

type BenchData struct {
	Hash         string                               `json:"gitHash"`
	Issue        string                               `json:"issue"`
	PatchSet     string                               `json:"patchset"`
	Key          map[string]string                    `json:"key"`
	Options      map[string]string                    `json:"options,omitempty"`
	Results      map[string]ingestcommon.BenchResults `json:"results"`
	PatchStorage string                               `json:"patch_storage,omitempty"`

	SwarmingTaskID string `json:"swarming_task_id,omitempty"`
	SwarmingBotID  string `json:"swarming_bot_id,omitempty"`
}

func main() {
	flag.Parse()

	cpuGPU := "CPU"
	if strings.Index(*builder, "-GPU-") != -1 {
		cpuGPU = "GPU"
	}
	defaultKeys = map[string]string{
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

	results = make(map[string][]reportBody)

	http.HandleFunc("/report_perf_data", reporter)
	http.HandleFunc("/dump_json", dumpJSON)

	fmt.Printf("Waiting for perf ingestion on port %s\n", *port)

	log.Fatal(http.ListenAndServe(":"+*port, nil))
}

// reporter handles when the client reports a test has a benchmark.
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

	benchOutput := reportBody{}
	if err := json.Unmarshal(body, &benchOutput); err != nil {
		fmt.Println(err)
		http.Error(w, "Could not unmarshal JSON", 400)
		return
	}

	if _, err := w.Write([]byte("Accepted")); err != nil {
		fmt.Printf("Could not write response: %s\n", err)
		return
	}
	resultsMutex.Lock()
 	defer resultsMutex.Unlock()
	results[benchOutput.BenchName] = append(results[benchOutput.BenchName], benchOutput)
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
// all the tests. If there is more than one result per benchmark, we report the average.
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

	benchData := BenchData{
		Hash:           *gitHash,
		Issue:          strconv.FormatInt(*issue, 10),
		PatchStorage:   *patch_storage,
		PatchSet:       strconv.FormatInt(*patchset, 10),
		Key:            defaultKeys,
		SwarmingBotID:  *botId,
		SwarmingTaskID: *taskId,
	}

	allResults := make(map[string]ingestcommon.BenchResults)
	for name, benches := range results {
		samples := []float64{}
		total := float64(0)
		for _, t := range benches {
			samples = append(samples, t.TimeMicroSeconds)
			total += t.TimeMicroSeconds
		}
		allResults[name] = map[string]ingestcommon.BenchResult{
			"default": map[string]interface{}{
				"average_us": total / float64(len(benches)),
				"samples":    samples,
			},
		}
	}
	benchData.Results = allResults

	enc := json.NewEncoder(outputFile)
	enc.SetIndent("", "  ") // Make it human readable.
	if err := enc.Encode(&benchData); err != nil {
		fmt.Println(err)
		http.Error(w, "Could not write json to disk", 500)
		return
	}
	fmt.Println("JSON Written")
}
