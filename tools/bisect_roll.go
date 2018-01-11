// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package main

/*
   Tool for bisecting failed rolls.
*/

import (
	"bufio"
	"context"
	"flag"
	"fmt"
	"os"
	"os/exec"
	"os/user"
	"path"
	"strings"
	"time"

	"go.skia.org/infra/autoroll/go/repo_manager"
	"go.skia.org/infra/go/autoroll"
	"go.skia.org/infra/go/common"
	"go.skia.org/infra/go/gerrit"
	"go.skia.org/infra/go/util"
)

var (
	// Flags.
	autoRollerAccount = flag.String("autoroller_account", "skia-deps-roller@chromium.org", "Email address of the autoroller.")
	childPath         = flag.String("childPath", "src/third_party/skia", "Path within parent repo of the project to roll.")
	gerritUrl         = flag.String("gerrit", "https://chromium-review.googlesource.com", "URL of the Gerrit server.")
	parentRepoUrl     = flag.String("parent_repo_url", common.REPO_CHROMIUM, "URL of the parent repo (the child repo rolls into this repo).")
	workdir           = flag.String("workdir", path.Join(os.TempDir(), "autoroll_bisect"), "Working directory.")
)

func log(tmpl string, a ...interface{}) {
	fmt.Println(fmt.Sprintf(tmpl, a...))
}

func bail(a ...interface{}) {
	fmt.Fprintln(os.Stderr, a...)
	os.Exit(1)
}

func main() {
	// Setup.
	common.Init()
	ctx := context.Background()

	log("Updating repos and finding roll attempts; this can take a few minutes...")

	// Create the working directory if necessary.
	if err := os.MkdirAll(*workdir, os.ModePerm); err != nil {
		bail(err)
	}

	// Create the RepoManager.
	gclient, err := exec.LookPath("gclient")
	if err != nil {
		bail(err)
	}
	depotTools := path.Dir(gclient)
	user, err := user.Current()
	if err != nil {
		bail(err)
	}
	gitcookiesPath := path.Join(user.HomeDir, ".gitcookies")
	g, err := gerrit.NewGerrit(*gerritUrl, gitcookiesPath, nil)
	if err != nil {
		bail("Failed to create Gerrit client:", err)
	}
	g.TurnOnAuthenticatedGets()
	childBranch := "master"
	strat, err := repo_manager.GetNextRollStrategy(repo_manager.ROLL_STRATEGY_BATCH, childBranch, "")
	if err != nil {
		bail(err)
	}
	rm, err := repo_manager.NewDEPSRepoManager(ctx, *workdir, *parentRepoUrl, "master", *childPath, childBranch, depotTools, g, strat, nil, true, nil, "(local run)")
	if err != nil {
		bail(err)
	}

	// Determine the set of not-yet-rolled commits.
	lastRoll := rm.LastRollRev()
	nextRoll := rm.NextRollRev()
	commits, err := rm.ChildRevList(ctx, fmt.Sprintf("%s..%s", lastRoll, nextRoll))
	if err != nil {
		bail(err)
	}
	if len(commits) == 0 {
		log("Repo is up-to-date.")
		os.Exit(0)
	} else if len(commits) == 1 {
		log("Recommend reverting commit %s", commits[0])
		os.Exit(0)
	}

	// Next, find any failed roll CLs.
	// TODO(borenet): Use the timestamp of the last-rolled commit.
	lastRollTime := time.Now().Add(-24 * time.Hour)
	modAfter := gerrit.SearchModifiedAfter(lastRollTime)
	cls, err := g.Search(500, modAfter, gerrit.SearchOwner(*autoRollerAccount))
	if err != nil {
		bail(err)
	}
	cls2, err := g.Search(500, modAfter, gerrit.SearchOwner("self"))
	if err != nil {
		bail(err)
	}
	cls = append(cls, cls2...)

	// Filter out CLs which don't look like rolls, de-duplicate CLs which
	// roll to the same commit, taking the most recent.
	rollCls := make(map[string]*autoroll.AutoRollIssue, len(cls))
	fullHashFn := func(hash string) (string, error) {
		return rm.FullChildHash(ctx, hash)
	}
	for _, cl := range cls {
		issue, err := autoroll.FromGerritChangeInfo(cl, fullHashFn, false)
		if err == nil {
			if old, ok := rollCls[issue.RollingTo]; !ok || ok && issue.Modified.After(old.Modified) {
				rollCls[issue.RollingTo] = issue
			}
		}
	}

	// Report the summary of the not-rolled commits and their associated
	// roll results to the user.
	log("%d commits have not yet rolled:", len(commits))
	earliestFail := -1
	latestFail := -1
	latestSuccess := -1 // eg. dry runs.
	for idx, commit := range commits {
		if cl, ok := rollCls[commit]; ok {
			log("%s roll %s", commit[:12], cl.Result)
			if util.In(cl.Result, autoroll.FAILURE_RESULTS) {
				earliestFail = idx
				if latestFail == -1 {
					latestFail = idx
				}
			} else if util.In(cl.Result, autoroll.SUCCESS_RESULTS) && latestSuccess == -1 {
				latestSuccess = idx
			}
		} else {
			log(commit[:12])
		}
	}

	// Suggest a commit to try rolling. The user may choose a different one.
	suggestedCommit := ""
	if latestSuccess != -1 {
		suggestedCommit = commits[latestSuccess]
		log("Recommend landing successful roll %s/%d", *gerritUrl, rollCls[suggestedCommit].Issue)
	} else if latestFail != 0 {
		suggestedCommit = commits[0]
		if issue, ok := rollCls[suggestedCommit]; ok && issue.Result == autoroll.ROLL_RESULT_IN_PROGRESS {
			log("Recommend waiting for the current in-progress roll to finish: %s/%d", *gerritUrl, issue.Issue)
			suggestedCommit = ""
		} else {
			log("Recommend trying a roll at %s which has not yet been tried.", suggestedCommit)
		}
	} else if earliestFail == 0 {
		log("Recommend reverting commit %s", commits[earliestFail])
	} else {
		// Bisect the commits which have not yet failed.
		remaining := commits[earliestFail+1:]
		idx := len(remaining) / 2
		suggestedCommit = remaining[idx]
		log("Recommend trying a roll at %s", suggestedCommit)
	}

	// Ask the user what commit to roll.
	msg := "Type a commit hash to roll"
	if suggestedCommit != "" {
		msg += fmt.Sprintf(" (press enter to roll at suggested commit %s)", suggestedCommit[:12])
	}
	log("%s:", msg)
	reader := bufio.NewReader(os.Stdin)
	text, err := reader.ReadString('\n')
	if err != nil {
		bail(err)
	}
	text = strings.TrimSpace(text)
	if text == "" && suggestedCommit != "" {
		text = suggestedCommit
	}
	if text == "" {
		bail("You must enter a commit hash.")
	}
	log("Attempting a roll at %q", text)
	rollTo, err := rm.FullChildHash(ctx, text)
	if err != nil {
		bail(text, "is not a valid commit hash:", text, err)
	}

	// Upload a roll.
	email, err := g.GetUserEmail()
	if err != nil {
		bail(err)
	}
	issue, err := rm.CreateNewRoll(ctx, lastRoll, rollTo, []string{email}, "", false)
	if err != nil {
		bail(err)
	}
	log("Uploaded %s/%d", *gerritUrl, issue)
}
