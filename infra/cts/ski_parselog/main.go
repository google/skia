package main

import (
	"bufio"
	"flag"
	"fmt"
	"os"
	"regexp"
	"strings"

	"go.skia.org/infra/go/common"
	"go.skia.org/infra/go/sklog"
	"go.skia.org/infra/go/util"
)

const (
	START_INDICATOR = "Running: skqp"
	MAX_LINES       = 200
)

var (
	SUCCESS_INDICATOR = regexp.MustCompile(".*Finished running all tests.*")
	END_INDICATOR     = regexp.MustCompile("Force finishing.*org.skia.skqp.*SkQPActivity")
)

// Command line flags.
var (
	dryRun = flag.Bool("dryrun", false, "Print out the command and quit without triggering tests.")
)

func main() {
	common.Init()
	args := flag.Args()

	fileName := args[0]

	inFile := openFile(fileName)
	defer util.Close(inFile)

	scanner := bufio.NewScanner(inFile)
	allLines := []string{}
	indicatorIdx := -1
	success := false
	for scanner.Scan() {
		line := scanner.Text()
		if strings.Contains(line, START_INDICATOR) {
			indicatorIdx = len(allLines)
		} else if SUCCESS_INDICATOR.Match([]byte(line)) {
			success = true
			break
		}
		allLines = append(allLines, line)
	}

	if err := scanner.Err(); err != nil {
		sklog.Fatal()
	}

	if success {
		sklog.Infof("Success detected. No error extracted.")
		return
	}

	if indicatorIdx == -1 {
		sklog.Fatalf("Start indicator string '%s' not found", START_INDICATOR)
	}

	blockEnd := indicatorIdx
	fmt.Printf("%d     %d \n", blockEnd, len(allLines))
	lineCount := 1
	for blockEnd < len(allLines) {
		if END_INDICATOR.Match([]byte(allLines[blockEnd])) || (lineCount >= MAX_LINES) {
			break
		}
		lineCount++
		blockEnd++
	}
	targetLines := allLines[indicatorIdx : blockEnd+1]

	sklog.Info("Found %d lines starting at %d:", len(targetLines), indicatorIdx)
	for _, line := range targetLines {
		fmt.Println(line)
	}
}

func openFile(fileName string) *os.File {
	file, err := os.Open(fileName)
	if err != nil {
		sklog.Fatalf("Unable to open file %s: %s", fileName, err)
	}
	return file
}
