// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
package main

import (
	"bufio"
	"bytes"
	"flag"
	"fmt"
	"log"
	"math/rand"
	"os"
	"os/exec"
	"path/filepath"
	"runtime"
	"strings"
	"time"
)

// Set default process count to NumCPU() or 8, whichever's smaller.  I picked 8
// heuristically as not too slow for CPU bound work, which can generally scale
// all the way up to NumCPU(), and not too slow for GPU bound work, which will
// start to thrash your GPU if too high, even hang your machine.
func defaultProcessLimit() int {
	limit := 8
	if n := runtime.NumCPU(); n < limit {
		return n
	}
	return limit
}

var script = flag.String("script", "", "A file with jobs to run, one per line.")
var random = flag.Bool("random", true, "Assign sources into job batches randomly?")
var quiet = flag.Bool("quiet", false, "Print only failures?")
var exact = flag.Bool("exact", false, "Match GM names only exactly.")
var processLimit = flag.Int("processLimit", defaultProcessLimit(),
	"Maximum number of concurrent processes, 0 -> NumCPU.")

func init() {
	flag.StringVar(script, "s", *script, "Alias for --script.")
	flag.BoolVar(random, "r", *random, "Alias for --random.")
	flag.BoolVar(quiet, "q", *quiet, "Alias for --quiet.")
	flag.BoolVar(exact, "e", *exact, "Alias for --exact.")
	flag.IntVar(processLimit, "j", *processLimit, "Alias for --processLimit.")
}

func listAllGMs(fm string) (gms []string, err error) {
	// Query fm binary for list of all available GMs by running with no arguments.
	cmd := exec.Command(fm)
	stdout, err := cmd.Output()
	if err != nil {
		return
	}
	// GM names are listed line-by-line.
	scanner := bufio.NewScanner(bytes.NewReader(stdout))
	for scanner.Scan() {
		gms = append(gms, scanner.Text())
	}
	err = scanner.Err()
	return
}

func callFM(fm string, sources []string, flags []string) bool {
	start := time.Now()

	args := flags[:]
	args = append(args, "-s")
	args = append(args, sources...)

	cmd := exec.Command(fm, args...)
	output, err := cmd.CombinedOutput()

	if err != nil {
		log.Printf("\n%v #failed (%v):\n%s\n", strings.Join(cmd.Args, " "), err, output)
		return false
	} else if !*quiet {
		log.Printf("\n%v #done in %v:\n%s", strings.Join(cmd.Args, " "), time.Since(start), output)
	}
	return true
}

func sourcesAndFlags(args []string, gms []string) ([]string, []string, error) {
	sources := []string{}
	flags := []string{}
	for _, arg := range args {
		// Treat "gm" or "gms" as a shortcut for all known GMs.
		if arg == "gm" || arg == "gms" {
			sources = append(sources, gms...)
			continue
		}

		// Is this an option to pass through to fm?
		if parts := strings.Split(arg, "="); len(parts) == 2 {
			f := "-"
			if len(parts[0]) > 1 {
				f += "-"
			}
			f += parts[0]

			flags = append(flags, f, parts[1])
			continue
		}

		// Is this argument naming a GM?
		matchedAnyGM := false
		for _, gm := range gms {
			if (*exact && gm == arg) || (!*exact && strings.Contains(gm, arg)) {
				sources = append(sources, gm)
				matchedAnyGM = true
			}
		}
		if matchedAnyGM {
			continue
		}

		// Anything left ought to be on the file system: a file, a directory, or a glob.
		// Not all shells expand globs, so we'll do it here just in case.
		matches, err := filepath.Glob(arg)
		if err != nil {
			return nil, nil, err
		}
		if len(matches) == 0 {
			return nil, nil, fmt.Errorf("Don't understand '%s'.", arg)
		}

		for _, match := range matches {
			err := filepath.Walk(match, func(path string, info os.FileInfo, err error) error {
				if !info.IsDir() {
					sources = append(sources, path)
				}
				return err
			})
			if err != nil {
				return nil, nil, err
			}
		}
	}
	return sources, flags, nil
}

func main() {
	flag.Parse()

	if *processLimit == 0 {
		*processLimit = runtime.NumCPU()
	}

	if flag.NArg() < 1 {
		log.Fatal("Please pass an fm binary as the first argument.")
	}
	fm := flag.Args()[0]

	gms, err := listAllGMs(fm)
	if err != nil {
		log.Fatalln("Could not query", fm, "for GMs:", err)
	}

	// One job can comes right on the command line,
	// and any number can come one per line from -script.
	jobs := [][]string{flag.Args()[1:]}
	if *script != "" {
		file, err := os.Open(*script)
		if err != nil {
			log.Fatal(err)
		}
		defer file.Close()

		scanner := bufio.NewScanner(file)
		for scanner.Scan() {
			jobs = append(jobs, strings.Fields(scanner.Text()))
		}
		if err = scanner.Err(); err != nil {
			log.Fatal(err)
		}
	}

	// The buffer size of main->worker queue channel isn't
	// super important... presumably we'll have many hungry
	// goroutines snapping work off it as quick as they can,
	// and if things get backed up, no real reason for main
	// to do anything but block.
	queue := make(chan struct {
		Sources []string
		Flags   []string
	}, *processLimit)

	// The buffer size of this worker->main results channel
	// is much more sensitive.  Since it's a many -> one
	// funnel, it's easy for the workers to produce lots of
	// results that main can't keep up with.
	//
	// This needlessly throttles our work, and in the worst
	// case, if the buffer fills up before main has finished
	// enqueueing all the work, we can deadlock.
	//
	// So we set the buffer size here large enough to hold
	// one result for every item we might possibly enqueue.
	results := make(chan bool, *processLimit*len(jobs))

	for i := 0; i < *processLimit; i++ {
		go func() {
			for sf := range queue {
				results <- callFM(fm, sf.Sources, sf.Flags)
			}
		}()
	}

	sent := 0
	for _, job := range jobs {
		// Skip blank lines, empty command lines.
		if len(job) == 0 {
			continue
		}
		sources, flags, err := sourcesAndFlags(job, gms)
		if err != nil {
			log.Fatal(err)
		}

		if *random {
			rand.Shuffle(len(sources), func(i, j int) {
				sources[i], sources[j] = sources[j], sources[i]
			})
		}

		// Round up so there's at least one source per batch.
		// This math also helps guarantee that sent stays <= cap(results).
		sourcesPerBatch := (len(sources) + *processLimit - 1) / *processLimit

		for i := 0; i < len(sources); i += sourcesPerBatch {
			batch := sources[i : i+sourcesPerBatch]

			queue <- struct {
				Sources []string
				Flags   []string
			}{batch, flags}

			sent += 1
		}
	}
	close(queue)

	if sent > cap(results) {
		log.Fatalf("Oops, we sent %d but cap(results) is only %d.  "+
			"This could lead to deadlock and is a bug.", sent, cap(results))
	}

	failures := 0
	for i := 0; i < sent; i++ {
		if !<-results {
			failures += 1
		}
	}
	if failures > 0 {
		log.Fatalln(failures, "invocations of", fm, "failed")
	}
}
