/*
 * Copyright 2025 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

package main

/*
	Reformat the jobs.json file, sorting the jobs by name.
*/

import (
	"encoding/json"
	"os"
	"path/filepath"
	"regexp"
	"runtime"
	"sort"

	"go.skia.org/infra/go/skerr"
	"go.skia.org/infra/task_scheduler/go/specs"
)

func main() {
	jobsJsonPath := findJobsJSON()
	b, err := os.ReadFile(jobsJsonPath)
	if err != nil {
		panic(err)
	}
	b, err = updateJobsJSON(b)
	if err != nil {
		panic(err)
	}
	if err := os.WriteFile(jobsJsonPath, b, 0644); err != nil {
		panic(err)
	}
}

// findJobsJSON attempts to find the jobs.json path based on the location of
// this file.  If that fails, fall back to assuming that this is being run from
// the root of the checkout.
func findJobsJSON() string {
	_, filename, _, ok := runtime.Caller(0)
	if ok {
		return filepath.Join(filepath.Dir(filepath.Dir(filename)), "jobs.json")
	}
	return filepath.Join("infra", "bots", "jobs.json")
}

// job represents a single entry in jobs.json.
type job struct {
	Name     string                      `json:"name"`
	CqConfig *specs.CommitQueueJobConfig `json:"cq_config"`
}

// jobSlice implements sort.Interface to sort jobs by name.
type jobSlice ([]*job)

func (s jobSlice) Less(i, j int) bool {
	return s[i].Name < s[j].Name
}

func (s jobSlice) Len() int {
	return len(s)
}

func (s jobSlice) Swap(i, j int) {
	s[i], s[j] = s[j], s[i]
}

func updateJobsJSON(oldJobsContents []byte) ([]byte, error) {
	var jobs []*job
	if err := json.Unmarshal(oldJobsContents, &jobs); err != nil {
		return nil, skerr.Wrapf(err, "failed to decode jobs.json")
	}

	sort.Sort(jobSlice(jobs))

	newJobsContents, err := json.MarshalIndent(jobs, "", "  ")
	if err != nil {
		return nil, skerr.Wrapf(err, "failed to encode jobs.json")
	}

	// Replace instances of `"cq_config": null`; these are cluttery and
	// unnecessary, but we can't use omitempty because that causes the Marshaler
	// to also omit `"cq_config": {}` which indicates that a job *should* be on
	// the CQ.  We also omit some whitespace to keep things relatively compact.
	jobsJSONReplaceRegex := regexp.MustCompile(`(?m)\{\n\s*"name":\s*"(\S+)",\n\s*"cq_config":\s*null\n\s*}`)
	jobsJSONReplaceContents := []byte(`{"name": "$1"}`)
	return jobsJSONReplaceRegex.ReplaceAll(newJobsContents, jobsJSONReplaceContents), nil
}
