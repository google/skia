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
	"sync"
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
var random = flag.Bool("random", false, "Assign sources into job batches randomly?")
var verbose = flag.Bool("verbose", false, "Log sucessful job timing and output?")
var processLimit = flag.Int("processLimit", defaultProcessLimit(),
	"Maximum number of concurrent processes, 0 -> NumCPU.")

func init() {
	flag.StringVar(script, "s", *script, "Alias for --script.")
	flag.BoolVar(random, "r", *random, "Alias for --random.")
	flag.BoolVar(verbose, "v", *verbose, "Alias for --verbose.")
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
	return
}

func callFM(fm string, flags []string, sources []string) {
	start := time.Now()

	args := flags[:]
	args = append(args, "-v", "-s")
	args = append(args, sources...)

	cmd := exec.Command(fm, args...)

	output, err := cmd.CombinedOutput()
	if err != nil {
		log.Printf("%v failed (%v):\n%s\n", sources, err, output)
	} else if *verbose {
		log.Printf("Done in %v:\n%s", time.Since(start), output)
	}
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

		// Is this argument naming a specific GM?
		if func() bool {
			for _, gm := range gms {
				if arg == gm {
					return true
				}
			}
			return false
		}() {
			sources = append(sources, arg)
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

	// One job can go right on the command line.
	sources, flags, err := sourcesAndFlags(flag.Args()[1:], gms)
	if err != nil {
		log.Fatal(err)
	}

	if *random {
		rand.Shuffle(len(sources), func(i, j int) {
			sources[i], sources[j] = sources[j], sources[i]
		})
	}

	// Round batch sizes up, making sure to batch at least one per batch.
	sourcesPerBatch := (len(sources) + *processLimit - 1) / *processLimit

	wg := &sync.WaitGroup{}
	for i := 0; i < len(sources); i += sourcesPerBatch {
		batch := sources[i : i+sourcesPerBatch]

		wg.Add(1)
		go func() {
			callFM(fm, flags, batch)
			wg.Done()
		}()
	}
	wg.Wait()
}
