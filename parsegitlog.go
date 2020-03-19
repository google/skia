// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package main

import (
	"fmt"
	"strings"
	"regexp"
	"log"
	"os/exec"
)

const (
	kGitLogFormat = `--pretty=%ce|||%ae|||%(trailers:key=Reviewed-by,valueonly)%n%n`
)

type Commit struct {
	committer string
	author string
	reviewers []string
}


func main() {
	out, err := exec.Command("git", "--no-pager", "log", "-5000", kGitLogFormat).Output()
	if err != nil {
		log.Fatal(err)
	}
	var commits []string;
	for _, s := range strings.Split(string(out), "\n\n") {
		if s != "" {
			commits = append(commits, s)
		}
	}
	fmt.Printf("OUT:\n%s", out)
	fmt.Printf("SPLIT:\n%+q", commits)
	fmt.Printf("length of slice:%d\n\n", len(commits))
	counts := make(map[int]int)
	var bad_commits []string
	bots_re := regexp.MustCompile(`autoroll|create-skp`)
	for _, s := range commits {
		googlers := strings.Count(s, "google.com")
		if (googlers == 0 && bots_re.MatchString(s) ) {
			counts[-1]++;
			continue
		}
		counts[googlers]++
		if googlers == 0 {
			bad_commits = append(bad_commits, s)
		}
	}
	fmt.Printf("Bad Commits:\n")
	for _, s := range bad_commits {
		fmt.Printf("%s\n", s)
	}
	fmt.Printf("\nGooglers Count: %v", counts)
}
