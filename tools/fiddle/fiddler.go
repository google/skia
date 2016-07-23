/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
package main

// Example use:
//   git clone https://skia.googlesource.com/skia.git
//   cd skia
//   SKIA=$PWD
//   cd tools/fiddle
//   go get github.com/skia-dev/glog
//   go get go.skia.org/infra/go/util
//   go build fiddler.go
//   # compile prerequisites
//   ./fiddler "$SKIA"
//   # compile and run a fiddle
//   ./fiddler "$SKIA" draw.cpp | ./parse-fiddle-output
//   # compile and run a different fiddle
//   ./fiddler "$SKIA" ANOTHER_FIDDLE.cpp | ./parse-fiddle-output

import (
	"bytes"
	"fmt"
	"io"
	"io/ioutil"
	"os"
	"os/exec"
	"path"
	"syscall"

	"github.com/skia-dev/glog"
	"go.skia.org/infra/go/util"
)

func fiddlePath() string {
	return path.Join("tools", "fiddle")
}
func setResourceLimits() error {
	const maximumTimeInSeconds = 5
	limit := syscall.Rlimit{maximumTimeInSeconds, maximumTimeInSeconds}
	if err := syscall.Setrlimit(syscall.RLIMIT_CPU, &limit); err != nil {
		return err
	}
	const maximumMemoryInBytes = 1 << 28
	limit = syscall.Rlimit{maximumMemoryInBytes, maximumMemoryInBytes}
	return syscall.Setrlimit(syscall.RLIMIT_AS, &limit)
}

// execCommand runs command and returns an error if it fails.  If there is no
// error, all output is discarded.
func execCommand(input io.Reader, dir string, name string, arg ...string) error {
	var buffer bytes.Buffer
	cmd := exec.Command(name, arg...)
	cmd.Dir = dir
	cmd.Stdout = &buffer
	cmd.Stderr = &buffer
	cmd.Stdin = input
	if err := cmd.Run(); err != nil {
		return fmt.Errorf("execution failed:\n\n%s\n[%v]", buffer.String(), err)
	}
	return nil
}

func compileArgs(skiaSrc string) string {
	return "@" + path.Join(skiaSrc, "cmake", "skia_compile_arguments.txt")
}

func linkArgs(skiaSrc string) string {
	return "@" + path.Join(skiaSrc, "cmake", "skia_link_arguments.txt")
}

// fiddler compiles the input, links against skia, and runs the executable.
// @param skiaSrc: the base directory of the Skia repository
// @param inputReader: C++ fiddle source
// @param output: stdout of executable sent here
// @param tempDir: where to place the compiled executable
func fiddler(skiaSrc string, inputReader io.Reader, output io.Writer, tempDir string) error {
	binarypath := path.Join(tempDir, "fiddle")
	fiddle_dir := path.Join(skiaSrc, fiddlePath())
	if err := execCommand(inputReader, fiddle_dir,
		"c++",
		compileArgs(skiaSrc),
		"-I", fiddle_dir,
		"-o", binarypath,
		"-x", "c++", "-", "-x", "none",
		"fiddle_main.o",
		"-lOSMesa",
		linkArgs(skiaSrc),
	); err != nil {
		return err
	}
	var buffer bytes.Buffer
	runCmd := exec.Cmd{Path: binarypath, Stdout: output, Stderr: &buffer}
	if err := runCmd.Run(); err != nil {
		return fmt.Errorf("execution failed:\n\n%s\n[%v]", buffer.String(), err)
	}
	return nil
}

// Compile Skia library and fiddle_main.cpp
// @param skiaSrc: the base directory of the Skia repository.
func fiddlerPrerequisites(skiaSrc string) error {
	cmakeDir := path.Join(skiaSrc, "cmake")
	if err := execCommand(nil, cmakeDir, "cmake", "-G", "Ninja", "."); err != nil {
		return err
	}
	if err := execCommand(nil, cmakeDir, "ninja", "skia"); err != nil {
		return err
	}
	fiddle_dir := path.Join(skiaSrc, fiddlePath())
	if err := execCommand(nil, fiddle_dir, "c++", compileArgs(skiaSrc),
		"fiddle_main.h"); err != nil {
		return err
	}
	return execCommand(nil, fiddle_dir, "c++", compileArgs(skiaSrc),
		"-c", "-o", "fiddle_main.o", "fiddle_main.cpp")
}

func main() {
	if len(os.Args) < 2 {
		glog.Fatalf("usage: %s SKIA_SRC_PATH [PATH_TO_DRAW.CPP]", os.Args[0])
	}
	skiaSrc := os.Args[1]
	if len(os.Args) < 3 {
		// execCommand(nil, skiaSrc, "git", "fetch")
		// execCommand(nil, skiaSrc, "git", "checkout", "origin/master")
		if err := fiddlerPrerequisites(skiaSrc); err != nil {
			glog.Fatal(err)
		}
	} else {
		if err := setResourceLimits(); err != nil {
			glog.Fatal(err)
		}
		tempDir, err := ioutil.TempDir("", "fiddle_")
		if err != nil {
			glog.Fatal(err)
		}
		defer func() {
			err = os.RemoveAll(tempDir)
			if err != nil {
				glog.Fatalf("os.RemoveAll(tempDir) failed: %v", err)
			}
		}()
		if os.Args[2] == "-" {
			if err := fiddler(skiaSrc, os.Stdin, os.Stdout, tempDir); err != nil {
				glog.Fatal(err)
			}
		} else {
			inputFile, err := os.Open(os.Args[2])
			if err != nil {
				glog.Fatalf("unable to open \"%s\": %v", os.Args[2], err)
			}
			defer util.Close(inputFile)
			if err = fiddler(skiaSrc, inputFile, os.Stdout, tempDir); err != nil {
				glog.Fatal(err)
			}
		}
	}
}
