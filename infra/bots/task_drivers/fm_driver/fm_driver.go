// Copyright 2020 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package main

import (
	"bufio"
	"bytes"
	"context"
	"flag"
	"fmt"
	"math/rand"
	"net/http"
	"os"
	"path/filepath"
	"runtime"
	"strconv"
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
		imgs      = flag.String("imgs", "", "Shorthand `directory` contents as 'imgs'.")
		skps      = flag.String("skps", "", "Shorthand `directory` contents as 'skps'.")
		svgs      = flag.String("svgs", "", "Shorthand `directory` contents as 'svgs'.")
		script    = flag.String("script", "", "File (or - for stdin) with one job per line.")
	)
	flag.Parse()

	ctx := context.Background()
	failStep := func(err error) { fmt.Fprintln(os.Stderr, err) }
	fatal := func(err error) {
		failStep(err)
		os.Exit(1)
	}

	if !*local {
		ctx = td.StartRun(projectId, taskId, bot, output, local)
		defer td.EndRun(ctx)
		failStep = func(err error) { td.FailStep(ctx, err) }
		fatal = func(err error) { td.Fatal(ctx, err) }
	}

	if flag.NArg() < 1 {
		fatal(fmt.Errorf("Please pass an fm binary."))
	}
	fm := flag.Arg(0)

	// Run `fm <flag>` to find the names of all linked GMs or tests.
	query := func(flag string) []string {
		stdout := &bytes.Buffer{}
		cmd := &exec.Command{Name: fm, Stdout: stdout}
		cmd.Args = append(cmd.Args, "-i", *resources)
		cmd.Args = append(cmd.Args, flag)
		if err := exec.Run(ctx, cmd); err != nil {
			fatal(err)
		}

		lines := []string{}
		scanner := bufio.NewScanner(stdout)
		for scanner.Scan() {
			lines = append(lines, scanner.Text())
		}
		if err := scanner.Err(); err != nil {
			fatal(err)
		}
		return lines
	}

	// Lowercase with leading '.' stripped.
	normalizedExt := func(s string) string {
		return strings.ToLower(filepath.Ext(s)[1:])
	}

	// Walk directory for files with given set of extensions.
	walk := func(dir string, exts map[string]bool) (files []string) {
		if dir != "" {
			err := filepath.Walk(dir, func(path string, info os.FileInfo, err error) error {
				if err != nil {
					return err
				}
				if !info.IsDir() && exts[normalizedExt(info.Name())] {
					files = append(files, path)
				}
				return nil
			})

			if err != nil {
				fatal(err)
			}
		}
		return
	}

	rawExts := map[string]bool{
		"arw": true,
		"cr2": true,
		"dng": true,
		"nef": true,
		"nrw": true,
		"orf": true,
		"pef": true,
		"raf": true,
		"rw2": true,
		"srw": true,
	}
	imgExts := map[string]bool{
		"astc": true,
		"bmp":  true,
		"gif":  true,
		"ico":  true,
		"jpeg": true,
		"jpg":  true,
		"ktx":  true,
		"png":  true,
		"wbmp": true,
		"webp": true,
	}
	for k, v := range rawExts {
		imgExts[k] = v
	}

	// We can use "gm" or "gms" as shorthand to refer to all GMs, and similar for the rest.
	shorthands := map[string][]string{
		"gm":   query("--listGMs"),
		"test": query("--listTests"),
		"img":  walk(*imgs, imgExts),
		"skp":  walk(*skps, map[string]bool{"skp": true}),
		"svg":  walk(*svgs, map[string]bool{"svg": true}),
	}
	for k, v := range shorthands {
		shorthands[k+"s"] = v
	}

	// Query Gold for all known hashes when running as a bot.
	known := map[string]bool{
		"0832f708a97acc6da385446384647a8f": true, // MD5 of passing unit test.
	}
	if *bot != "" {
		func() {
			url := "https://storage.googleapis.com/skia-infra-gm/hash_files/gold-prod-hashes.txt"
			resp, err := http.Get(url)
			if err != nil {
				fatal(err)
			}
			defer resp.Body.Close()

			scanner := bufio.NewScanner(resp.Body)
			for scanner.Scan() {
				known[scanner.Text()] = true
			}
			if err := scanner.Err(); err != nil {
				fatal(err)
			}

			fmt.Fprintf(os.Stdout, "Gold knew %v unique hashes.\n", len(known))
		}()
	}

	type Work struct {
		Sources []string // Passed to FM -s: names of gms/tests, paths to image files, .skps, etc.
		Flags   []string // Other flags to pass to FM: --ct 565, --msaa 16, etc.
	}

	queue := make(chan Work, 1<<20) // Arbitrarily huge buffer to avoid ever blocking.
	wg := &sync.WaitGroup{}
	var failures int32 = 0

	var worker func([]string, []string)
	worker = func(sources, flags []string) {
		defer wg.Done()

		stdout := &bytes.Buffer{}
		stderr := &bytes.Buffer{}
		cmd := &exec.Command{Name: fm, Stdout: stdout, Stderr: stderr}
		cmd.Args = append(cmd.Args, "-i", *resources)
		cmd.Args = append(cmd.Args, flags...)
		cmd.Args = append(cmd.Args, "-s")
		cmd.Args = append(cmd.Args, sources...)

		// Run our FM command.
		err := exec.Run(ctx, cmd)

		// On success, scan stdout for any unknown hashes.
		unknownHash := func() string {
			if err == nil && *bot != "" { // We only fetch known hashes when using -bot.
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
					fatal(err)
				}
			}
			return ""
		}()

		// If a batch failed or produced an unknown hash, isolate with individual reruns.
		if len(sources) > 1 && (err != nil || unknownHash != "") {
			wg.Add(len(sources))
			for i := range sources {
				worker(sources[i:i+1], flags)
			}
			return
		}

		// If an individual run failed, nothing more to do but fail.
		if err != nil {
			atomic.AddInt32(&failures, 1)

			lines := []string{}
			scanner := bufio.NewScanner(stderr)
			for scanner.Scan() {
				lines = append(lines, scanner.Text())
			}
			if err := scanner.Err(); err != nil {
				fatal(err)
			}

			failStep(fmt.Errorf("%v %v #failed:\n\t%v\n",
				cmd.Name,
				strings.Join(cmd.Args, " "),
				strings.Join(lines, "\n\t")))

			return
		}

		// If an individual run succeeded but produced an unknown hash, TODO upload .png to Gold.
		// For now just print out the command and the hash it produced.
		if unknownHash != "" {
			fmt.Fprintf(os.Stdout, "%v %v #%v\n",
				cmd.Name,
				strings.Join(cmd.Args, " "),
				unknownHash)
		}
	}

	for i := 0; i < runtime.NumCPU(); i++ {
		go func() {
			for w := range queue {
				worker(w.Sources, w.Flags)
			}
		}()
	}

	// Get some work going, first breaking it into batches to increase our parallelism.
	kickoff := func(sources, flags []string) {
		if len(sources) == 0 {
			return // A blank or commented job line from -script or the command line.
		}

		// Shuffle the sources randomly as a cheap way to approximate evenly expensive batches.
		// (Intentionally not rand.Seed()'d to stay deterministically reproducible.)
		rand.Shuffle(len(sources), func(i, j int) {
			sources[i], sources[j] = sources[j], sources[i]
		})

		nbatches := runtime.NumCPU()                      // Arbitrary, nice to scale ~= cores.
		batch := (len(sources) + nbatches - 1) / nbatches // Round up to avoid empty batches.
		util.ChunkIter(len(sources), batch, func(start, end int) error {
			wg.Add(1)
			queue <- Work{sources[start:end], flags}
			return nil
		})
	}

	// Parse a job like "gms b=cpu ct=8888" into sources and flags for kickoff().
	parse := func(job []string) (sources, flags []string) {
		for _, token := range job {
			// Everything after # is a comment.
			if strings.HasPrefix(token, "#") {
				break
			}

			// Expand "gm" or "gms"  to all known GMs, or same for tests, images, skps, svgs.
			if vals, ok := shorthands[token]; ok {
				sources = append(sources, vals...)
				continue
			}

			// Is this a flag to pass through to FM?
			if parts := strings.Split(token, "="); len(parts) == 2 {
				f := "-"
				if len(parts[0]) > 1 {
					f += "-"
				}
				f += parts[0]

				flags = append(flags, f, parts[1])
				continue
			}

			// Anything else must be the name of a source for FM to run.
			sources = append(sources, token)
		}
		return
	}

	// Parse one job from the command line, handy for ad hoc local runs.
	kickoff(parse(flag.Args()[1:]))

	// Any number of jobs can come from -script.
	if *script != "" {
		file := os.Stdin
		if *script != "-" {
			file, err := os.Open(*script)
			if err != nil {
				fatal(err)
			}
			defer file.Close()
		}
		scanner := bufio.NewScanner(file)
		for scanner.Scan() {
			kickoff(parse(strings.Fields(scanner.Text())))
		}
		if err := scanner.Err(); err != nil {
			fatal(err)
		}
	}

	// If we're a bot (or acting as if we are one), kick off its work.
	if *bot != "" {
		parts := strings.Split(*bot, "-")
		OS, model, CPU_or_GPU := parts[1], parts[3], parts[4]

		commonFlags := []string{
			"--nativeFonts",
			strconv.FormatBool(strings.Contains(*bot, "NativeFonts")),
		}

		run := func(sources []string, extraFlags string) {
			kickoff(sources, append(strings.Fields(extraFlags), commonFlags...))
		}

		gms := shorthands["gms"]
		imgs := shorthands["imgs"]
		svgs := shorthands["svgs"]
		skps := shorthands["skps"]
		tests := shorthands["tests"]

		filter := func(in []string, keep func(string) bool) (out []string) {
			for _, s := range in {
				if keep(s) {
					out = append(out, s)
				}
			}
			return
		}

		if strings.Contains(OS, "Win") {
			// We can't decode these formats on Windows.
			imgs = filter(imgs, func(s string) bool { return !rawExts[normalizedExt(s)] })
		}

		if CPU_or_GPU == "CPU" {
			commonFlags = append(commonFlags, "-b", "cpu")

			// FM's default flags are equivalent to --config srgb in DM.
			run(gms, "")
			run(imgs, "")
			run(svgs, "")
			run(skps, "")
			run(tests, "")

			if model == "GCE" {
				run(gms, "--ct g8 --legacy")                      // --config g8
				run(gms, "--ct 565 --legacy")                     // --config 565
				run(gms, "--ct 8888 --legacy")                    // --config 8888.
				run(gms, "--ct f16")                              // --config esrgb
				run(gms, "--ct f16 --tf linear")                  // --config f16
				run(gms, "--ct 8888 --gamut p3")                  // --config p3
				run(gms, "--ct 8888 --gamut narrow --tf 2.2")     // --config narrow
				run(gms, "--ct f16 --gamut rec2020 --tf rec2020") // --config erec2020

				run(gms, "--skvm")
				run(gms, "--skvm --ct f16")

				run(imgs, "--decodeToDst --ct f16 --gamut rec2020 --tf rec2020")
			}

			// TODO: pic-8888 equivalent?
			// TODO: serialize-8888 equivalent?
		}
	}

	wg.Wait()
	if failures > 0 {
		fatal(fmt.Errorf("%v runs of %v failed after retries.\n", failures, fm))
	}
}
