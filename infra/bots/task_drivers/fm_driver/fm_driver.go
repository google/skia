// Copyright 2020 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package main

import (
	"bufio"
	"flag"
	"math/rand"
	"runtime"
	"strings"
	"sync"

	"go.skia.org/infra/go/exec"
	"go.skia.org/infra/task_driver/go/td"
)

func main() {
	var (
		projectId = flag.String("project_id", "", "ID of the Google Cloud project.")
		taskId    = flag.String("task_id", "", "ID of this task.")
		taskName  = flag.String("task_name", "", "Name of the task.")
		local     = flag.Bool("local", true, "True if running locally (as opposed to on the bots)")
		output    = flag.String("o", "", "If provided, dump a JSON blob of step data to the given file. Prints to stdout if '-' is given.")

		workdir   = flag.String("workdir", ".", "Working directory")
		fm        = flag.String("fm", "build/fm", "FM binary to run, relative to -workdir")
		resources = flag.String("resources", "resources", "Passed to fm -i, relative to -workdir")
	)
	ctx := td.StartRun(projectId, taskId, taskName, output, local)
	defer td.EndRun(ctx)

	// Run fm --listTests to find the names of all linked GMs.
	stdout, err := exec.RunCwd(ctx, *workdir, *fm, "--listGMs", "-i", *resources)
	if err != nil {
		td.FailStep(ctx, td.InfraError(err))
		return
	}

	gms := []string{}
	scanner := bufio.NewScanner(strings.NewReader(stdout))
	for scanner.Scan() {
		gms = append(gms, scanner.Text())
	}
	if err := scanner.Err(); err != nil {
		td.FailStep(ctx, td.InfraError(err))
		return
	}

	// Shuffle the GMs randomly as a cheap way to approximate evenly expensive batches.
	rand.Shuffle(len(gms), func(i, j int) {
		gms[i], gms[j] = gms[j], gms[i]
	})

	// Round up so there's at least one GM per batch.
	limit := runtime.NumCPU()
	batchSize := (len(gms) + limit - 1) / limit

	wg := &sync.WaitGroup{}
	for i := 0; i < len(gms); i += batchSize {
		end := i + batchSize
		if end > len(gms) {
			end = len(gms)
		}
		batch := gms[i:end]

		cmd := []string{}
		cmd = append(cmd, *fm)
		cmd = append(cmd, "-i", *resources)
		cmd = append(cmd, "-b", "cpu")
		cmd = append(cmd, "-s")
		cmd = append(cmd, batch...)

		wg.Add(1)
		go func(cmd []string) {
			if _, err := exec.RunCwd(ctx, *workdir, cmd...); err != nil {
				td.FailStep(ctx, err)
			}
			wg.Done()
		}(cmd)
	}
	wg.Wait()
}
