// Copyright (c) 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/*
	Run a Skia app to completion, piping the log to stdout.
*/

package main

import (
	"flag"
	"fmt"
	"io"
	"os"
	"os/exec"
	"os/signal"
	"strconv"
	"strings"
	"time"

	"github.com/skia-dev/glog"
	"go.skia.org/infra/go/common"
)

const (
	COM_SKIA = "com.skia"
)

var (
	adbPath = flag.String("adb", "", "Path to the ADB executable.")
	app     = flag.String("app", "VisualBench", "Name of the app to run.")
	serial  = flag.String("s", "", "Serial number for the Android device to use.")
)

// Struct used for running ADB commands.
type ADB struct {
	path   string
	serial string
}

// ADBFromFlags returns an ADB instance based on flags passed to the program.
func ADBFromFlags() *ADB {
	rv := &ADB{
		path:   *adbPath,
		serial: *serial,
	}
	if rv.path == "" {
		rv.path = "adb"
	}
	return rv
}

// Cmd creates an exec.Cmd object for the given ADB command.
func (adb *ADB) Cmd(log bool, args ...string) *exec.Cmd {
	cmd := []string{}
	if adb.serial != "" {
		cmd = append(cmd, "-s", adb.serial)
	}
	cmd = append(cmd, args...)
	if log {
		glog.Infof("Exec `%s %s`", adb.path, strings.Join(cmd, " "))
	}
	return exec.Command(adb.path, cmd...)
}

// KillSkia kills any running process.
func (adb *ADB) Kill(proc string) error {
	return adb.Cmd(false, "shell", "am", "force-stop", proc).Run()
}

// PollSkia checks to see if the given process is running. If so, return the pid, otherwise return -1.
func (adb *ADB) Poll(proc string) (int64, error) {
	output, err := adb.Cmd(false, "shell", "ps").Output()
	if err != nil {
		return -1, fmt.Errorf("Failed to check the running processes on the device: %v", err)
	}
	for _, line := range strings.Split(string(output), "\n") {
		if strings.Contains(line, proc) {
			fields := strings.Fields(line)
			pid, err := strconv.ParseInt(fields[1], 10, 32)
			if err != nil {
				return -1, fmt.Errorf("Failed to parse \"%s\" to an integer: %v", fields[1], err)
			}
			return pid, nil
		}
	}
	return -1, nil
}

// ReadLinesFromPipe reads from the given pipe and logs its output.
func ReadLinesFromPipe(pipe io.Reader, lines chan string) {
	buf := []byte{}

	// writeLine recovers from a panic when writing to the channel.
	writeLine := func(s string) {
		defer func() {
			if r := recover(); r != nil {
				glog.Warningf("Panic writing to channel... are we exiting?")
			}
		}()
		lines <- s
	}

	// readLines reads output lines from the buffer and pushes them into the channel.
	readlines := func() {
		readIdx := 0
		// Read lines from buf.
		for i, b := range buf {
			if b == '\n' {
				s := string(buf[readIdx:i])
				writeLine(s)
				readIdx = i + 1
			}
		}
		// Remove the lines we read.
		buf = buf[readIdx:]
	}

	// Loop forever, reading bytes from the pipe.
	readbuf := make([]byte, 4096)
	for {
		read, err := pipe.Read(readbuf)
		if read > 0 {
			buf = append(buf, readbuf[:read]...)

			// Read lines from the buffers.
			readlines()
		}
		if err != nil {
			if err == io.EOF {
				break
			} else {
				glog.Error(err)
			}
		} else if read == 0 {
			time.Sleep(20 * time.Millisecond)
		}
	}
	// Read any remaining lines from the buffers.
	readlines()
	// "Flush" any incomplete lines from the buffers.
	writeLine(string(buf))
}

// RunApp launches the given app on the device, pipes its log output to stdout,
// and returns when the app finishes running, with an error if the app did not
// complete successfully.
func RunApp(adb *ADB, appName string, args []string) error {
	// Kill any running instances of the app.
	if err := adb.Kill(COM_SKIA); err != nil {
		return fmt.Errorf("Failed to kill app: %v", err)
	}

	// Clear the ADB log.
	if err := adb.Cmd(false, "logcat", "-c").Run(); err != nil {
		return fmt.Errorf("Failed to clear ADB log: %v", err)
	}

	// Prepare to read the subprocess output.
	logProc := adb.Cmd(false, "logcat")
	defer func() {
		// Cleanup.
		if err := logProc.Process.Kill(); err != nil {
			glog.Errorf("Failed to kill logcat process: %v", err)
		}
	}()

	stdout, err := logProc.StdoutPipe()
	if err != nil {
		return fmt.Errorf("Failed to obtain stdout pipe: %v", err)
	}
	stdoutLines := make(chan string)
	stderr, err := logProc.StderrPipe()
	if err != nil {
		return fmt.Errorf("Failed to obtain stderr pipe: %v", err)
	}
	stderrLines := make(chan string)

	go ReadLinesFromPipe(stdout, stdoutLines)
	go ReadLinesFromPipe(stderr, stderrLines)
	if err := logProc.Start(); err != nil {
		return fmt.Errorf("Failed to start logcat process.")
	}

	// Launch the app.
	stop := make(chan bool, 1)
	activity := fmt.Sprintf("%s.%s/%s.%sActivity", COM_SKIA, strings.ToLower(appName), COM_SKIA, appName)
	flags := strings.Join(args, " ")
	cmd := []string{"shell", "am", "start", "-S", "-n", activity, "--es", "cmdLineFlags", flags}
	output, err := adb.Cmd(true, cmd...).Output()
	if err != nil {
		return fmt.Errorf("Failed to launch app: %v", err)
	}
	glog.Info(string(output))
	// Make a handler for SIGINT so that we can kill the app if this script is interrupted.
	go func() {
		interrupt := make(chan os.Signal, 1)
		signal.Notify(interrupt, os.Interrupt)
		for _ = range interrupt {
			glog.Infof("Received SIGINT; killing app.")
			stop <- true
			close(stdoutLines)
			close(stderrLines)
			if err := adb.Kill(COM_SKIA); err != nil {
				glog.Errorf("Failed to kill app: %v", err)
			}
			if err := logProc.Process.Kill(); err != nil {
				glog.Errorf("Failed to kill logcat process: %v", err)
			}
		}
	}()

	// Loop until the app finishes or we're interrupted, writing output as appropriate.
	// TODO(borenet): VisualBench should print its exit code. This script should exit
	// with that code, or a non-zero code if the process ended without printing any code.
	second := time.Tick(time.Second)
PollLoop:
	for {
		select {
		case <-second:
			// Poll the Skia process every second to make sure it's still running.
			pid, err := adb.Poll(COM_SKIA)
			if err != nil {
				glog.Errorf("Failed to poll Skia process: %v", err)
			} else if pid < 0 {
				glog.Infof("Skia process is no longer running!")
				break PollLoop
			}
		case <-stop:
			break PollLoop
		case line := <-stdoutLines:
			glog.Info(line)
		case line := <-stderrLines:
			glog.Error(line)
		}
	}

	return nil
}

func main() {
	common.Init()
	args := flag.Args()
	if err := RunApp(ADBFromFlags(), *app, args); err != nil {
		glog.Fatal(err)
	}
}
