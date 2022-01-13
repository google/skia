// Copyright 2022 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This executable downloads, verifies, and uploads a given file to the Skia infra Bazel mirror.
// Users should have gsutil installed, on the PATH and authenticated.
// There are two modes of use:
//   - Specify a single file via --url and --sha256.
//   - Copy a JSON array of objects (or Starlark list of dictionaries) via standard in.
// This should only need to be called when we add new dependencies or update existing ones. Calling
// it with already archived files should be fine - the mirror is a CAS, so the update should be a
// no-op. The files will be uploaded to the mirror with some metadata about where they came from.
package main

import (
	"crypto/sha256"
	"encoding/hex"
	"flag"
	"fmt"
	"io"
	"net/http"
	"os"
	"os/exec"
	"path/filepath"
	"strings"

	"github.com/flynn/json5"

	"go.skia.org/infra/go/skerr"
)

const (
	gcsBucketAndPrefix = "gs://skia-world-readable/bazel/"
)

func main() {
	var (
		url           = flag.String("url", "", "The single url to mirror. --sha256 must be set.")
		sha256Hash    = flag.String("sha256", "", "The sha256sum of the url to mirror. --url must also be set.")
		jsonFromStdin = flag.Bool("json", false, "If set, read JSON from stdin that consists of a list of objects.")
	)
	flag.Parse()

	if (*url != "" && *sha256Hash == "") || (*url == "" && *sha256Hash != "") {
		flag.Usage()
		fatalf("Must set both of or non of --url and --sha256")
	} else if !*jsonFromStdin {
		fatalf("Must specify --url and --sha256 or --json")
	}

	workDir, err := os.MkdirTemp("", "bazel_gcs")
	if err != nil {
		fatalf("Could not make temp directory: %s", err)
	}

	if *jsonFromStdin {
		fmt.Println("Waiting for input on std in. Use Ctrl+D (EOF) when done copying and pasting the array.")
		b, err := io.ReadAll(os.Stdin)
		if err != nil {
			fatalf("Error while reading from stdin: %s", err)
		}
		if err := processJSON(workDir, b); err != nil {
			fatalf("Could not process data from stdin: %s", err)
		}
	} else {
		if err := processOne(workDir, *url, *sha256Hash); err != nil {
			fatalf("Error while processing entry: %s", err)
		}
	}
}

type urlEntry struct {
	SHA256 string `json:"sha256"`
	URL    string `json:"url"`
}

func processJSON(workDir string, b []byte) error {
	// We generally will be copying a list from Bazel files, written with Starlark (i.e. Pythonish).
	// As a result, we need to turn the almost valid JSON array of objects into actually valid JSON.
	// It is easier to just do string replacing rather than going line by line to remove the
	// troublesome comments.
	cleaned := fixStarlarkComments(b)
	var entries []urlEntry
	if err := json5.Unmarshal([]byte(cleaned), &entries); err != nil {
		return skerr.Wrapf(err, "unmarshalling JSON")
	}
	for _, entry := range entries {
		if err := processOne(workDir, entry.URL, entry.SHA256); err != nil {
			return skerr.Wrapf(err, "while processing entry: %+v", entry)
		}
	}
	return nil
}

// fixStarlarkComments replaces the Starlark comment symbol (#) with a JSON comment symbol (//).
func fixStarlarkComments(b []byte) string {
	return strings.ReplaceAll(string(b), "#", "//")
}

func processOne(workDir, url, hash string) error {
	suf := getSuffix(url)
	if suf == "" {
		return skerr.Fmt("%s is not a supported file type", url)
	}
	fmt.Printf("Downloading and verifying %s...\n", url)
	res, err := http.Get(url)
	if err != nil {
		return skerr.Wrapf(err, "downloading %s", url)
	}
	contents, err := io.ReadAll(res.Body)
	if err != nil {
		return skerr.Wrapf(err, "reading %s", url)
	}
	if err := res.Body.Close(); err != nil {
		return skerr.Wrapf(err, "after reading %s", url)
	}
	// Verify
	h := sha256.Sum256(contents)
	if actual := hex.EncodeToString(h[:]); actual != hash {
		return skerr.Fmt("Invalid hash of %s. %s != %s", url, actual, hash)
	}
	fmt.Printf("Uploading %s to GCS...\n", url)
	// Write to disk so gsutil can access it
	tmpFile := filepath.Join(workDir, hash+suf)
	if err := os.WriteFile(tmpFile, contents, 0644); err != nil {
		return skerr.Wrapf(err, "writing %d bytes to %s", len(contents), tmpFile)
	}
	// Upload using gsutil (which is assumed to be properly authed)
	cmd := exec.Command("gsutil",
		// Add custom metadata so we can figure out what the unrecognizable file name was created
		// from. Custom metadata values must start with x-goog-meta-
		"-h", "x-goog-meta-original-url:"+url,
		"cp", tmpFile, gcsBucketAndPrefix+hash+suf)
	cmd.Stdout = os.Stdout
	cmd.Stderr = os.Stderr
	return skerr.Wrapf(cmd.Run(), "uploading %s to GCS", tmpFile)
}

var supportedSuffixes = []string{".tar.gz", ".tar.xz", ".deb"}

// getSuffix returns the filetype suffix of the file if it is in the list of supported suffixes.
// Otherwise, it returns empty string.
func getSuffix(url string) string {
	for _, suf := range supportedSuffixes {
		if strings.HasSuffix(url, suf) {
			return suf
		}
	}
	return ""
}

func fatalf(format string, args ...interface{}) {
	// Ensure there is a newline at the end of the fatal message.
	format = strings.TrimSuffix(format, "\n") + "\n"
	fmt.Printf(format, args...)
	os.Exit(1)
}
