// Copyright 2020 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package main

import (
	"bufio"
	"bytes"
	"flag"
	"fmt"
	"math/rand"
	"net/http"
	"os"
	"runtime"
	"strings"
	"sync"
	"sync/atomic"

	"go.skia.org/infra/go/exec"
	"go.skia.org/infra/go/util"
	"go.skia.org/infra/task_driver/go/td"
)

func main() {
	var (
		projectId = flag.String("project_id", "", "ID of the Google Cloud project.")
		taskId    = flag.String("task_id", "", "ID of this task.")
		bot       = flag.String("bot", "", "Name of the task.")
		output    = flag.String("o", "", "Dump JSON step data to the given file, or stdout if -.")
		local     = flag.Bool("local", true, "Running locally (else on the bots)?")

		resources = flag.String("resources", "resources", "Passed to fm -i.")
		script    = flag.String("script", "", "File (or - for stdin) with one job per line.")
	)
	ctx := td.StartRun(projectId, taskId, bot, output, local)
	defer td.EndRun(ctx)

	actualStdout := os.Stdout
	actualStderr := os.Stderr
	verbosity := exec.Info
	if *local {
		// Task Driver echoes every exec.Run() stdout and stderr to the console,
		// which makes it hard to find failures (especially stdout).  Send them to /dev/null.
		devnull, err := os.OpenFile(os.DevNull, os.O_WRONLY, 0)
		if err != nil {
			td.Fatal(ctx, err)
		}
		os.Stdout = devnull
		os.Stderr = devnull
		// Having stifled stderr/stdout, changing Command.Verbose won't have any visible effect,
		// but setting it to Silent will bypass a fair chunk of wasted formatting work.
		verbosity = exec.Silent
	}

	if flag.NArg() < 1 {
		td.Fatalf(ctx, "Please pass an fm binary.")
	}
	fm := flag.Arg(0)

	// Run `fm <flag>` to find the names of all linked GMs or tests.
	query := func(flag string) []string {
		stdout := &bytes.Buffer{}
		cmd := &exec.Command{Name: fm, Stdout: stdout, Verbose: verbosity}
		cmd.Args = append(cmd.Args, "-i", *resources)
		cmd.Args = append(cmd.Args, flag)
		if err := exec.Run(ctx, cmd); err != nil {
			td.Fatal(ctx, err)
		}

		lines := []string{}
		scanner := bufio.NewScanner(stdout)
		for scanner.Scan() {
			lines = append(lines, scanner.Text())
		}
		if err := scanner.Err(); err != nil {
			td.Fatal(ctx, err)
		}
		return lines
	}
	gms := query("--listGMs")
	tests := query("--listTests")

	// Query Gold for all known hashes when running as a bot.
	known := map[string]bool{
		"0832f708a97acc6da385446384647a8f": true, // MD5 of passing unit test.
	}
	if *bot != "" {
		func() {
			url := "https://storage.googleapis.com/skia-infra-gm/hash_files/gold-prod-hashes.txt"
			resp, err := http.Get(url)
			if err != nil {
				td.Fatal(ctx, err)
			}
			defer resp.Body.Close()

			scanner := bufio.NewScanner(resp.Body)
			for scanner.Scan() {
				known[scanner.Text()] = true
			}
			if err := scanner.Err(); err != nil {
				td.Fatal(ctx, err)
			}

			fmt.Fprintf(actualStdout, "Gold knew %v unique hashes.\n", len(known))
		}()
	}

	type Work struct {
		Sources []string // Passed to FM -s: names of gms/tests, paths to image files, .skps, etc.
		Flags   []string // Other flags to pass to FM: --ct 565, --msaa 16, etc.
	}

	// Parse a job like "gms b=cpu ct=8888" into Work{Sources=<all GMs>, Flags={-b,cpu,--ct,8888}}.
	parse := func(job []string) (w Work) {
		for _, token := range job {
			// Everything after # is a comment.
			if strings.HasPrefix(token, "#") {
				break
			}

			// Treat "gm" or "gms" as a shortcut for all known GMs.
			if token == "gm" || token == "gms" {
				w.Sources = append(w.Sources, gms...)
				continue
			}
			// Same for tests.
			if token == "test" || token == "tests" {
				w.Sources = append(w.Sources, tests...)
				continue
			}

			// Is this a flag to pass through to FM?
			if parts := strings.Split(token, "="); len(parts) == 2 {
				f := "-"
				if len(parts[0]) > 1 {
					f += "-"
				}
				f += parts[0]

				w.Flags = append(w.Flags, f, parts[1])
				continue
			}

			// Anything else must be the name of a source for FM to run.
			w.Sources = append(w.Sources, token)
		}
		return
	}

	// Parse one job from the command line, handy for ad hoc local runs.
	todo := []Work{parse(flag.Args()[1:])}

	// Any number of jobs can come from -script.
	if *script != "" {
		file := os.Stdin
		if *script != "-" {
			file, err := os.Open(*script)
			if err != nil {
				td.Fatal(ctx, err)
			}
			defer file.Close()
		}
		scanner := bufio.NewScanner(file)
		for scanner.Scan() {
			todo = append(todo, parse(strings.Fields(scanner.Text())))
		}
		if err := scanner.Err(); err != nil {
			td.Fatal(ctx, err)
		}
	}

	// If we're a bot (or acting as if we are one), add its work too.
	if *bot != "" {
		parts := strings.Split(*bot, "-")
		OS := parts[1]

		// For no reason but as a demo, skip GM aarectmodes and test GoodHash.
		filter := func(in []string, test func(string) bool) (out []string) {
			for _, s := range in {
				if test(s) {
					out = append(out, s)
				}
			}
			return
		}
		if OS == "Debian10" {
			gms = filter(gms, func(s string) bool { return s != "aarectmodes" })
			tests = filter(tests, func(s string) bool { return s != "GoodHash" })
		}

		// You could use parse() here if you like, but it's just as easy to make Work{} directly.
		work := func(sources []string, flags string) {
			todo = append(todo, Work{sources, strings.Fields(flags)})
		}
		work(tests, "-b cpu")
		work(gms, "-b cpu")
		work(gms, "-b cpu --skvm")
	}

	// We'll kick off worker goroutines to run batches of Work, and on failure,
	// crash, or unknown hash, we'll split that batch into individual reruns to
	// isolate those unusual results.
	var failures int32 = 0
	wg := &sync.WaitGroup{}

	var worker func([]string, []string)
	worker = func(sources []string, flags []string) {
		defer wg.Done()

		stdout := &bytes.Buffer{}
		stderr := &bytes.Buffer{}
		cmd := &exec.Command{Name: fm, Stdout: stdout, Stderr: stderr, Verbose: verbosity}
		cmd.Args = append(cmd.Args, "-i", *resources, "-s")
		cmd.Args = append(cmd.Args, sources...)
		cmd.Args = append(cmd.Args, flags...)

		// Run our FM command.
		err := exec.Run(ctx, cmd)

		// On success, scan stdout for any unknown hashes.
		unknownHash := func() string {
			if err == nil && *bot != "" { // The map of known hashes is only filled when using -bot.
				scanner := bufio.NewScanner(stdout)
				for scanner.Scan() {
					if parts := strings.Fields(scanner.Text()); len(parts) == 3 {
						md5 := parts[1]
						if !known[md5] {
							return md5
						}
					}
				}
				if err := scanner.Err(); err != nil {
					td.Fatal(ctx, err)
				}
			}
			return ""
		}()

		// If a batch failed or produced an unknown hash, kick off individual runs to isolate.
		if len(sources) > 1 && (err != nil || unknownHash != "") {
			for i := range sources {
				wg.Add(1)
				/*go*/ worker(sources[i:i+1], flags)
			}
			return
		}

		// If an individual run failed, nothing more to do but fail.
		if err != nil {
			atomic.AddInt32(&failures, 1)
			td.FailStep(ctx, err)
			if *local {
				lines := []string{}
				scanner := bufio.NewScanner(stderr)
				for scanner.Scan() {
					lines = append(lines, scanner.Text())
				}
				if err := scanner.Err(); err != nil {
					td.Fatal(ctx, err)
				}
				fmt.Fprintf(actualStderr, "%v %v #failed:\n\t%v\n",
					cmd.Name,
					strings.Join(cmd.Args, " "),
					strings.Join(lines, "\n\t"))
			}
			return
		}

		// If an individual run succeeded but produced an unknown hash, TODO upload .png to Gold.
		if unknownHash != "" {
			fmt.Fprintf(actualStdout, "%v %v #%v\n",
				cmd.Name,
				strings.Join(cmd.Args, " "),
				unknownHash)
		}
	}

	for _, w := range todo {
		if len(w.Sources) == 0 {
			continue // A blank or commented job line from -script or the command line.
		}

		// Shuffle the sources randomly as a cheap way to approximate evenly expensive batches.
		// (Intentionally not rand.Seed()'d to stay deterministically reproducible.)
		rand.Shuffle(len(w.Sources), func(i, j int) {
			w.Sources[i], w.Sources[j] = w.Sources[j], w.Sources[i]
		})

		// Round batch sizes up so there's at least one source per batch.
		// Batch size is arbitrary, but nice to scale with the machine like this.
		batches := runtime.NumCPU()
		batch := (len(w.Sources) + batches - 1) / batches
		util.ChunkIter(len(w.Sources), batch, func(start, end int) error {
			wg.Add(1)
			go worker(w.Sources[start:end], w.Flags)
			return nil
		})
	}
	wg.Wait()

	if failures > 0 {
		if *local {
			// td.Fatalf() would work fine, but barfs up a panic that we don't need to see.
			fmt.Fprintf(actualStderr, "%v runs of %v failed after retries.\n", failures, fm)
			os.Exit(1)
		} else {
			td.Fatalf(ctx, "%v runs of %v failed after retries.", failures, fm)
		}
	}
}
