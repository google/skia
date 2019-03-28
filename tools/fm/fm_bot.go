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
	"sync"
	"time"
)

var jobs = flag.Int("j", runtime.NumCPU(), "Maximum number of concurrent jobs.")
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

func callFM(fm string, job []string) {
	start := time.Now()

	// TODO(mtklein): allow passing flags through to fm
	args := []string{"-b", "cpu", "-v", "-s"}
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
	// We'll treat "gm", "gms", or an empty argument list as a shortcut for all known GMs.
	if flag.NArg() == 1 {
		srcs = append(srcs, gms...)
	}
	for _, arg := range flag.Args()[1:] {
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
	srcsPerJob := (len(srcs) + *jobs - 1) / *jobs

	wg := &sync.WaitGroup{}
	for i := 0; i < len(srcs); i += srcsPerJob {
		job := srcs[i : i+srcsPerJob]

		wg.Add(1)
		go func() {
			callFM(fm, job)
			wg.Done()
		}()
	}
	wg.Wait()
}
