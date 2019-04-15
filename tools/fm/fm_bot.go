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
	"sync/atomic"
	"time"
)

// Too many GPU processes and we'll start to overwhelm your GPU,
// even hanging your machine in the worst case.  Here's a reasonable default.
func defaultGpuLimit() int {
	limit := 8
	if n := runtime.NumCPU(); n < limit {
		return n
	}
	return limit
}

var script = flag.String("script", "", "A file with jobs to run, one per line. - for stdin.")
var random = flag.Bool("random", true, "Assign sources into job batches randomly?")
var quiet = flag.Bool("quiet", false, "Print only failures?")
var exact = flag.Bool("exact", false, "Match GM names only exactly.")
var cpuLimit = flag.Int("cpuLimit", runtime.NumCPU(),
	"Maximum number of concurrent processes for CPU-bound work.")
var gpuLimit = flag.Int("gpuLimit", defaultGpuLimit(),
	"Maximum number of concurrent processes for GPU-bound work.")

func init() {
	flag.StringVar(script, "s", *script, "Alias for --script.")
	flag.BoolVar(random, "r", *random, "Alias for --random.")
	flag.BoolVar(quiet, "q", *quiet, "Alias for --quiet.")
	flag.BoolVar(exact, "e", *exact, "Alias for --exact.")
	flag.IntVar(cpuLimit, "c", *cpuLimit, "Alias for --cpuLimit.")
	flag.IntVar(gpuLimit, "g", *gpuLimit, "Alias for --gpuLimit.")
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
		if !*quiet || len(sources) == 1 {
			log.Printf("\n%v #failed (%v):\n%s\n", strings.Join(cmd.Args, " "), err, output)
		}
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
		// I wish we could parse flags here too, but it's too late.
		if strings.HasPrefix(arg, "-") {
			msg := "Is '%s' an fm flag? If so please pass it using flag=value syntax."
			if flag.Lookup(arg[1:]) != nil {
				msg = "Please pass fm_bot flags like '%s' on the command line before the FM binary."
			}
			return nil, nil, fmt.Errorf(msg, arg)
		}

		// Everything after a # is a comment.
		if strings.HasPrefix(arg, "#") {
			break
		}

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

type work struct {
	Sources []string
	Flags   []string
}

func main() {
	flag.Parse()

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
		file := os.Stdin
		if *script != "-" {
			file, err = os.Open(*script)
			if err != nil {
				log.Fatal(err)
			}
			defer file.Close()
		}

		scanner := bufio.NewScanner(file)
		for scanner.Scan() {
			jobs = append(jobs, strings.Fields(scanner.Text()))
		}
		if err = scanner.Err(); err != nil {
			log.Fatal(err)
		}
	}


	wg := &sync.WaitGroup{}
	var failures int32 = 0

	worker := func(queue chan work) {
		for w := range queue {
			if !callFM(fm, w.Sources, w.Flags) {
				if len(w.Sources) == 1 {
					// If a source ran alone and failed, that's just a failure.
					atomic.AddInt32(&failures, 1)
				} else {
					// If a batch of sources ran and failed, split them up and try again.
					for _, source := range w.Sources {
						wg.Add(1)
						queue <- work{[]string{source}, w.Flags}
					}
				}
			}
			wg.Done()
		}
	}

	cpu := make(chan work, *cpuLimit)
	for i := 0; i < *cpuLimit; i++ {
		go worker(cpu)
	}

	gpu := make(chan work, *gpuLimit)
	for i := 0; i < *gpuLimit; i++ {
		go worker(gpu)
	}

	for _, job := range jobs {
		// Skip blank lines, empty command lines.
		if len(job) == 0 {
			continue
		}
		sources, flags, err := sourcesAndFlags(job, gms)
		if err != nil {
			log.Fatal(err)
		}

		// Determine if this is CPU-bound or GPU-bound work, conservatively assuming GPU.
		queue, limit := gpu, *gpuLimit
		backend := ""
		for i, flag := range flags {
			if flag == "-b" || flag == "--backend" {
				backend = flags[i+1]
			}
		}
		whitelisted := map[string]bool{
			"cpu": true,
			"skp": true,
			"pdf": true,
		}
		if whitelisted[backend] {
			queue, limit = cpu, *cpuLimit
		}

		if *random {
			rand.Shuffle(len(sources), func(i, j int) {
				sources[i], sources[j] = sources[j], sources[i]
			})
		}

		// Round up so there's at least one source per batch.
		sourcesPerBatch := (len(sources) + limit - 1) / limit

		for i := 0; i < len(sources); i += sourcesPerBatch {
			end := i + sourcesPerBatch
			if end > len(sources) {
				end = len(sources)
			}
			batch := sources[i:end]

			wg.Add(1)
			queue <- work{batch, flags}
		}
	}

	wg.Wait()

	if failures > 0 {
		log.Fatalln(failures, "failures after retries")
	}
}
