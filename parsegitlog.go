// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package main

import (
	"flag"
	"fmt"
	"log"
	"os/exec"
	"regexp"
	"strings"
)

const (
	// We use double-newline between commits, since  asingle commit may have a
	// newline due to multiple reviewers.
	kGitLogFormat = `--pretty=%ce%n%ae%n%(trailers:key=Reviewed-by,valueonly)%n%n`
)

var (
	exclude_pattern = flag.String("exclude_pattern", `autoroll|create-skp`,
		"Regex for commiter/author/reviewer that will be"
		" bucketed separately if no google address is present.")
	git_since = flag.String("git_since", `1 year ago`,
		"string date or relative time passed to git log --since")

	reviewer_re = regexp.MustCompile(`<(.*)>`)
)

func getAccountSet(commit string) map[string]bool {
	accounts := make(map[string]bool)
	for _, account := range strings.Split(commit, "\n") {
		// Reviewers lines include full name, strip to email for consistency.
		stripped_account_name := reviewer_re.FindStringSubmatch(account)
		if len(stripped_account_name) > 0 {
			account = stripped_account_name[1]
		}
		accounts[account] = true
	}
	return accounts
}

func main() {
	flag.Parse()
	cmd := exec.Command("git", "--no-pager", "log", "--since", fmt.Sprintf(`"%s"`, *git_since), kGitLogFormat)
	fmt.Println(cmd.String())
	out, err := cmd.Output()
	if err != nil {
		log.Fatal(err)
	}
	// Set of commiter/author/reviewer combos that are problematic.
	bad_commits := make(map[string]int)
	// Count of googlers associated with each commit.
	googler_counts := make(map[int]int)
	// Number of times each non-google account is associated with a commit that doesn't include a googler.
	non_google_account_freq := make(map[string]int)
	exclude_re := regexp.MustCompile(*exclude_pattern)
	for _, s := range strings.Split(string(out), "\n\n") {
		if s == "" {
			continue
		}
		googlers := strings.Count(s, "google.com")
		if googlers == 0 && exclude_re.MatchString(s) && *exclude_pattern != "" {
			googler_counts[-1]++
			continue
		}
		googler_counts[googlers]++
		if googlers == 0 {
			bad_commits[strings.ReplaceAll(s, "\n", "|")]++
			for acc, _ := range getAccountSet(s) {
				non_google_account_freq[acc]++
			}
		}
	}

	fmt.Println("\nCommits missing google.com commiter/author/reviewer:")
	for account_set, count := range bad_commits {
		fmt.Printf("%d times - %s\n", count, account_set)
	}
	fmt.Printf("\n\nCount of googlers per commit (-1 is exclude_pattern matching commits):\n%v\n", googler_counts)

	fmt.Println("\nAccounts on commits lacking googlers:")
	for acc, count := range non_google_account_freq {
		fmt.Printf("%-40v%d\n", acc, count)
	}
}
