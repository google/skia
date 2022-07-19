// Copyright 2022 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package main

import (
	"encoding/base64"
	"encoding/json"
	"errors"
	"fmt"
	"io"
	"io/ioutil"
	"net"
	"net/http"
	"os"
	"os/signal"
	"path"
	"path/filepath"
	"strconv"
	"syscall"
)

const (
	envPortFileBaseName = "port"
)

func main() {
	envDir, envReadyFile := mustGetEnvironmentVariables()

	port, listener := mustGetUnusedNetworkPort()

	beginTestManagementLogic(listener)

	mustPrepareTestEnvironment(envDir, port)

	setupTerminationLogic()

	mustSignalTestsCanBegin(envReadyFile)

	select {} // Block until the termination handler calls os.Exit
}

// mustGetEnvironmentVariables returns two file paths: a directory that can be used to communicate
// between this binary and the test binaries, and the file that needs to be created when this
// binary has finished setting things up. It panics if it cannot read the values from the
// set environment variables.
func mustGetEnvironmentVariables() (string, string) {
	// Read in build paths to the ready and port files.
	envDir := os.Getenv("ENV_DIR")
	if envDir == "" {
		panic("required environment variable ENV_DIR is unset")
	}
	envReadyFile := os.Getenv("ENV_READY_FILE")
	if envReadyFile == "" {
		panic("required environment variable ENV_READY_FILE is unset")
	}
	return envDir, envReadyFile
}

// mustGetUnusedNetworkPort returns a network port chosen by the OS (and assumed to be previously
// unused) and a listener for that port. We choose a non-deterministic port instead of a fixed port
// because multiple tests may be running in parallel.
func mustGetUnusedNetworkPort() (int, net.Listener) {
	// Listen on an unused port chosen by the OS.
	listener, err := net.Listen("tcp", ":0")
	if err != nil {
		panic(err)
	}
	port := listener.Addr().(*net.TCPAddr).Port
	fmt.Printf("Environment is ready to go!\nListening on port %d.\n", port)
	return port, listener
}

// beginTestManagementLogic sets up the server endpoints which allow the JS gm() tests to exfiltrate
// their PNG images by means of a POST request.
func beginTestManagementLogic(listener net.Listener) {
	// The contents of this path go to //bazel-testlogs/path/to/test/test.outputs/ and are combined
	// into outputs.zip.
	// e.g. ls bazel-testlogs/modules/canvaskit/hello_world_test_with_env/test.outputs/
	//   test_001
	//   test_002
	//   outputs.zip   # contains test_001 and test_002
	// This environment var is documented in https://bazel.build/reference/test-encyclopedia
	// Note that Bazel expects a zip executable to be present on this machine in order to do this.
	// https://github.com/bazelbuild/bazel/blob/b9ffc16b94c1ee101031b0c010453847bdc532d1/tools/test/test-setup.sh#L425
	outPath := os.Getenv("TEST_UNDECLARED_OUTPUTS_DIR")
	if outPath == "" {
		panic("output directory was not configured")
	}

	http.HandleFunc("/healthz", func(w http.ResponseWriter, r *http.Request) {
		w.WriteHeader(http.StatusOK)
	})

	http.HandleFunc("/report", func(w http.ResponseWriter, r *http.Request) {
		payload, err := readPayload(r)
		if err != nil {
			http.Error(w, err.Error(), http.StatusBadRequest)
		}
		if payload.TestName == "" {
			http.Error(w, "Must specify test name", http.StatusBadRequest)
			return
		}
		// Write the data in the POST to the special Bazel output directory
		fileContents, err := base64.StdEncoding.DecodeString(payload.Base64Data)
		if err != nil {
			fmt.Printf("Invalid base64 data: %s\n", err.Error())
			http.Error(w, "Invalid base64 data "+err.Error(), http.StatusBadRequest)
			return
		}
		fileName := payload.TestName
		if payload.Config != "" {
			fileName += "." + payload.Config
		}

		fp := filepath.Join(outPath, fileName+".png")
		// Two newlines here makes the log stick out more.
		fmt.Printf("Writing test data to %s\n\n", fp)
		out, err := os.Create(fp)
		if err != nil {
			http.Error(w, err.Error(), http.StatusInternalServerError)
			panic(err)
		}
		if _, err := out.Write(fileContents); err != nil {
			http.Error(w, err.Error(), http.StatusInternalServerError)
			panic(err)
		}

		// Signal to the test that we have written the data to disk. Tests should be sure to wait
		// for this response before signaling they are done to avoid a race condition.
		w.WriteHeader(http.StatusCreated)
		// We are not worried about an XSS reflection attack here on a local server only up
		// when running tests.
		if _, err := fmt.Fprintln(w, "Accepted for test "+payload.TestName); err != nil {
			panic(err)
		}
	})
	go func() {
		serveForever(listener)
	}()
}

type testPayload struct {
	TestName   string `json:"name"`
	Base64Data string `json:"b64_data"`
	// Config, if set, will be added as a suffix before the .png in the file name
	// e.g. test_name.html_canvas.png. This will be parsed before uploading to Gold to be used
	// as the value for the "config" key. This allows us to have different variants of the same
	// test to compare and contrast.
	Config string `json:"config"`
}

// readPayload reads the body of the given request as JSON and parses it into a testPayload struct.
func readPayload(r *http.Request) (testPayload, error) {
	var payload testPayload
	if r.Body == nil {
		return payload, errors.New("no body received")
	}
	b, err := io.ReadAll(r.Body)
	if err != nil {
		return payload, err
	}
	_ = r.Body.Close()
	if err := json.Unmarshal(b, &payload); err != nil {
		return payload, errors.New("invalid JSON")
	}
	return payload, nil
}

// serveForever serves the given listener and blocks. If it could not start serving, it will panic.
func serveForever(listener net.Listener) {
	// If http.Serve returns, it is an error.
	if err := http.Serve(listener, nil); err != nil {
		panic(fmt.Sprintf("Finished serving due to error: %s\n", err))
	}
}

// mustPrepareTestEnvironment writes any files to the temporary test directory. This is just a file
// that indicates which port the gold tests should make POST requests to. It panics if there are
// any errors.
func mustPrepareTestEnvironment(dirTestsCanRead string, port int) {
	envPortFile := path.Join(dirTestsCanRead, envPortFileBaseName)
	if err := ioutil.WriteFile(envPortFile, []byte(strconv.Itoa(port)), 0644); err != nil {
		panic(err)
	}
}

// setupTerminationLogic creates a handler for SIGTERM which is what test_on_env will send the
// environment when the tests complete. There is currently nothing to do other than exit.
func setupTerminationLogic() {
	c := make(chan os.Signal, 1)
	go func() {
		<-c
		os.Exit(0)
	}()
	signal.Notify(c, syscall.SIGTERM)
}

// mustSignalTestsCanBegin creates the agreed upon ENV_READY_FILE which signals the test binary can
// be executed by Bazel. See test_on_env.bzl for more. It panics if the file cannot be created.
func mustSignalTestsCanBegin(envReadyFile string) {
	if err := ioutil.WriteFile(envReadyFile, []byte{}, 0644); err != nil {
		panic(err)
	}
}
