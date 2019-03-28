package main

import (
	"bufio"
	"bytes"
	"flag"
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

// Set default job count to NumCPU() or 8, whichever's smaller.  I picked 8
// heuristically as not too slow for CPU bound work, which can generally scale
// all the way up to NumCPU(), and not too slow for GPU bound work, which will
// start to thrash your GPU if too high, even hang your machine.
func defaultJobs() int {
	limit := 8
	if n := runtime.NumCPU(); n < limit {
		return n
	}
	return limit
}

var jobs = flag.Int("j", defaultJobs(), "Maximum number of concurrent jobs, 0 -> NumCPU.")
var shuffle = flag.Bool("s", false, "Assign sources into job batches randomly?")
var verbose = flag.Bool("v", false, "Log sucessful job timing and output?")

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

func callFM(fm string, flags []string, job []string) {
	start := time.Now()

	args := flags[:]
	args = append(args, "-v", "-s")
	args = append(args, job...)

	cmd := exec.Command(fm, args...)

	output, err := cmd.CombinedOutput()
	if err != nil {
		log.Printf("Job %v failed (%v):\n%s\n", job, err, output)
	} else if *verbose {
		log.Printf("Job done in %v:\n%s", time.Since(start), output)
	}
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

	srcs := []string{}
	flags := []string{}

	for _, arg := range flag.Args()[1:] {
		// Treat "gm" or "gms" as a shortcut for all known GMs.
		if arg == "gm" || arg == "gms" {
			srcs = append(srcs, gms...)
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
			srcs = append(srcs, arg)
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
			log.Fatal(err)
		}
		if len(matches) == 0 {
			log.Fatalf("Don't understand '%s'.", arg)
		}

		for _, match := range matches {
			err := filepath.Walk(match, func(path string, info os.FileInfo, err error) error {
				if !info.IsDir() {
					srcs = append(srcs, path)
				}
				return err
			})
			if err != nil {
				log.Fatal(err)
			}
		}
	}

	if *shuffle {
		rand.Shuffle(len(srcs), func(i, j int) {
			srcs[i], srcs[j] = srcs[j], srcs[i]
		})
	}

	// Round job sizes up, making sure to run at least one per job.
	if *jobs == 0 {
		*jobs = runtime.NumCPU()
	}
	srcsPerJob := (len(srcs) + *jobs - 1) / *jobs

	wg := &sync.WaitGroup{}
	for i := 0; i < len(srcs); i += srcsPerJob {
		job := srcs[i : i+srcsPerJob]

		wg.Add(1)
		go func() {
			callFM(fm, flags, job)
			wg.Done()
		}()
	}
	wg.Wait()
}
