// Copyright 2026 Google LLC
//
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package main

import (
	"context"
	"encoding/json"
	"flag"
	"fmt"
	"io"
	"net/http"
	"net/http/httptest"
	"net/url"
	"path/filepath"
	"strconv"
	"time"

	_ "embed"

	"github.com/chromedp/chromedp"
	"go.skia.org/infra/go/httputils"
	"go.skia.org/infra/go/skerr"
	"go.skia.org/infra/go/sklog"
	"go.skia.org/infra/task_driver/go/lib/os_steps"
	"go.skia.org/infra/task_driver/go/td"
)

//go:embed harness.html
var harnessHtml []byte

func main() {
	var (
		// Required flags for this task regardless of where it's running.
		dmFlagsStr = flag.String("dm_flags", "",
			"JSON-encoded array of flags to pass to dm invocation")
		dmPropertiesStr = flag.String("dm_properties", "",
			"JSON-encoded array of properties to pass to dm invocation")
		chromeCache = flag.String("chrome_cache", "",
			"The directory to store the Chrome binary.")
		buildPath = flag.String("build", "",
			"The directory where the built wasm/js code is.")

		// Required flags for running on CQ
		projectID     = flag.String("project_id", "", "ID of the Google Cloud project.")
		taskID        = flag.String("task_id", "", "ID of the task")
		taskName      = flag.String("task_name", "", "Name of the task.")
		goldHashesURL = flag.String("gold_hashes_url", "",
			"URL from which to download pre-existing hashes")

		// Debugging flags.
		local = flag.Bool("local", false,
			"True if running locally (as opposed to on the bots)")
		outputSteps = flag.String("o", "", "If provided, dump a JSON blob of step data to the "+
			"given file. Prints to stdout if '-' is given.")
	)

	ctx := td.StartRun(projectID, taskID, taskName, outputSteps, local)
	defer td.EndRun(ctx)

	cd, err := newChromeDriver(
		ctx,
		td.MustGetAbsolutePathOfFlag(ctx, *chromeCache, "chrome_cache"),
		*local,
	)
	if err != nil {
		td.Fatal(ctx, err)
	}
	defer cd.shutdown()

	knownHashes := mustFetchFile(ctx, *goldHashesURL)
	server, err := startLocalServer(
		ctx,
		knownHashes,
		td.MustGetAbsolutePathOfFlag(ctx, *buildPath, "build"),
	)
	if err != nil {
		td.Fatal(ctx, err)
	}
	defer server.Close()

	argv := dmArgv{
		properties: mustUnmarshalFlag[map[string]string](ctx, *dmPropertiesStr, "dm_properties"),
		extraFlags: mustUnmarshalFlag[[]string](ctx, *dmFlagsStr, "dm_flags"),
	}
	if err := runDm(ctx, cd, *local, server.URL, &argv); err != nil {
		td.Fatal(ctx, err)
	}
}

type dmArgv struct {
	match      string
	properties map[string]string
	extraFlags []string
}

func (a dmArgv) ToJson() ([]byte, error) {
	argv := []string{}
	if len(a.match) > 0 {
		argv = append(argv, "-m", a.match)
	}
	if len(a.properties) > 0 {
		argv = append(argv, "--properties")
		for k, v := range a.properties {
			argv = append(argv, k, v)
		}
	}
	argv = append(argv, a.extraFlags...)
	return json.Marshal(argv)
}

func runDm(ctx context.Context, cd *chromeDriver, local bool, url string, argv *dmArgv) error {
	ctx = td.StartStep(ctx, td.Props("run dm"))
	defer td.EndStep(ctx)

	argvJson, err := argv.ToJson()
	if err != nil {
		return td.FailStep(ctx, err)
	}

	// On CQ we don't get normal sklogging, so redirect to a task driver log stream.
	if !local {
		logs := td.NewLogStream(ctx, "dm", td.SeverityInfo)
		cancel := cd.AttachConsole(logs)
		defer cancel()
	}

	// Load harness page
	resp, err := chromedp.Run(cd.ctx, chromedp.NavigateResponse(url))
	if err != nil {
		return td.FailStep(ctx, err)
	} else if resp.Status != http.StatusOK {
		return td.FailStep(ctx,
			skerr.Fmt("failed to navigate to %s: %d %s", url, resp.Status, resp.StatusText))
	}

	// Wait for our entrypoint to exist
	loaded, err := chromedp.Run(cd.ctx, chromedp.Poll[bool](
		`typeof window.run === 'function'`,
		chromedp.WithPollingTimeout(5*time.Second),
	))
	if err != nil {
		return td.FailStep(ctx, err)
	} else if !loaded {
		return td.FailStep(ctx, skerr.Fmt("js entrypoint not found"))
	}

	// Run
	sklog.Info("page loaded; starting dm run")
	rc, err := chromedp.Run(cd.ctx,
		chromedp.Evaluate[int](fmt.Sprintf(`window.run(%s)`, argvJson), chromedp.EvalAwaitPromise),
	)
	if err != nil {
		return td.FailStep(ctx, err)
	}

	td.StepText(ctx, "dm exit code", strconv.Itoa(rc))
	if rc != 0 {
		return td.FailStep(ctx, skerr.Fmt("dm exited with code %d", rc))
	}

	return nil
}

func startLocalServer(
	ctx context.Context,
	knownHashes []byte,
	buildPath string,
) (*httptest.Server, error) {
	ctx = td.StartStep(ctx, td.Props("start localhost server").Infra())
	defer td.EndStep(ctx)

	dmJs := filepath.Join(buildPath, "dm.js")
	if _, err := os_steps.Stat(ctx, dmJs); err != nil {
		return nil, td.FailStep(
			ctx,
			skerr.Wrapf(err, "can't find %s, did you provide the right build path?", dmJs),
		)
	}

	harnessPath := filepath.Join(buildPath, "index.html")
	if err := os_steps.WriteFile(ctx, harnessPath, harnessHtml, 0o644); err != nil {
		return nil, td.FailStep(ctx, err)
	}

	knownHashesPath := filepath.Join(buildPath, "known-hashes.txt")
	if err := os_steps.WriteFile(ctx, knownHashesPath, knownHashes, 0o644); err != nil {
		return nil, td.FailStep(ctx, err)
	}

	mux := http.NewServeMux()
	mux.Handle("/", http.FileServer(http.Dir(buildPath)))

	// Suppress the misleading 404 for favicon, which we don't care about.
	mux.HandleFunc("/favicon.ico", func(w http.ResponseWriter, r *http.Request) {
		w.WriteHeader(http.StatusNoContent)
	})

	// WASM pthreads require these headers.
	addHeaders := func(next http.Handler) http.Handler {
		return http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
			w.Header().Set("Cross-Origin-Opener-Policy", "same-origin")
			w.Header().Set("Cross-Origin-Embedder-Policy", "require-corp")
			next.ServeHTTP(w, r)
		})
	}

	return httptest.NewServer(addHeaders(mux)), nil
}

func mustFetchFile(ctx context.Context, urlStr string) []byte {
	ctx = td.StartStep(ctx, td.Props(fmt.Sprintf("fetch remote file %s", urlStr)).Infra())
	defer td.EndStep(ctx)

	u, err := url.Parse(urlStr)
	if err != nil {
		td.Fatal(ctx, skerr.Wrapf(err, "bad url: '%s'", urlStr))
	}

	client := td.HttpClient(ctx, httputils.DefaultClientConfig().With2xxOnly().Client())
	resp, err := client.Get(u.String())
	if err != nil {
		td.Fatal(ctx, skerr.Wrapf(err, "GET failed: '%s'", u.String()))
	}

	defer resp.Body.Close()
	data, err := io.ReadAll(resp.Body)
	if err != nil {
		td.Fatal(ctx, skerr.Wrapf(err, "read failed: '%s'", u.String()))
	}

	return data
}

func mustUnmarshalFlag[T any](ctx context.Context, data string, message string) T {
	var value T
	if err := json.Unmarshal([]byte(data), &value); err != nil {
		td.Fatal(ctx, skerr.Wrapf(err, "unmarshalling required flag: %s", message))
	}
	return value
}
