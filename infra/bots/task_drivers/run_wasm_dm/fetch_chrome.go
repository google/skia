// Copyright 2026 Google LLC
//
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package main

import (
	"context"
	"encoding/json"
	"errors"
	"fmt"
	"io"
	"io/fs"
	"net/http"
	"net/url"
	"os"
	"path"
	"path/filepath"
	r "runtime"
	"time"

	"go.skia.org/infra/go/exec"
	"go.skia.org/infra/go/httputils"
	"go.skia.org/infra/go/skerr"
	"go.skia.org/infra/go/sklog"
	"go.skia.org/infra/task_driver/go/lib/os_steps"
	"go.skia.org/infra/task_driver/go/td"
)

const (
	chromeForTestingUrlBase = "https://googlechromelabs.github.io/chrome-for-testing"
	stampFileName           = "chrome-install.stamp"
)

type chromeDlSpec struct {
	platform string
	zipUrl   string
	exePath  string
}

type chromeVersions struct {
	Timestamp time.Time       `json:"timestamp"`
	Versions  []chromeVersion `json:"versions"`
}

type chromeVersion struct {
	Version   string           `json:"version"`
	Revision  string           `json:"revision"`
	Downloads versionDownloads `json:"downloads"`
}

type versionDownloads struct {
	Chrome []versionDownload `json:"chrome"`
}

type versionDownload struct {
	Platform string `json:"platform"`
	URL      string `json:"url"`
}

type chromeStamp struct {
	Version string `json:"version"`
	Bin     string `json:"bin"`
}

func installChrome(ctx context.Context, cacheDir string) (string, error) {
	chromeInstallDir := filepath.Join(cacheDir, "chrome")
	if err := os_steps.MkdirAll(ctx, chromeInstallDir); err != nil {
		return "", err
	}

	latestVersion, err := fetchChromeLatestVersionNum()
	if err != nil {
		return "", err
	}
	sklog.Infof("latest chrome version is %s", latestVersion)

	stampPath := filepath.Join(cacheDir, stampFileName)
	stamp, err := readStamp(ctx, stampPath)
	if err != nil {
		return "", err
	} else if stamp.Version == latestVersion {
		sklog.Infof("using cached chrome binary version %s at %s", stamp.Version, stamp.Bin)
		return stamp.Bin, nil
	}

	versions, err := fetchChromeVersions()
	if err != nil {
		return "", err
	}

	spec, err := determineChromeDownloadSpec(latestVersion, versions)
	if err != nil {
		return "", err
	}

	sklog.Infof("fetching chrome %s...", spec.zipUrl)
	zipfile := path.Join(cacheDir, "chrome.zip")
	if err := downloadFile(ctx, spec.zipUrl, zipfile); err != nil {
		return "", err
	} else if err := extractZip(ctx, zipfile, cacheDir); err != nil {
		return "", err
	}

	chromeBin := path.Join(cacheDir, spec.exePath)
	stamp = chromeStamp{
		Version: latestVersion,
		Bin:     chromeBin,
	}
	if err := writeStamp(ctx, stampPath, stamp); err != nil {
		sklog.Warningf("writing stamp file failed: %w", err)
	}
	sklog.Infof("using chrome version %s at %s", latestVersion, chromeBin)
	td.StepText(ctx, "chrome version", latestVersion)
	return chromeBin, nil
}

func writeStamp(ctx context.Context, path string, s chromeStamp) error {
	b, err := json.Marshal(s)
	if err != nil {
		return skerr.Wrap(err)
	}
	return os_steps.WriteFile(ctx, path, b, 0o644)
}

func readStamp(ctx context.Context, path string) (chromeStamp, error) {
	var stamp chromeStamp
	err := td.Do(
		ctx,
		td.Props(fmt.Sprintf("read stamp %s", path)).Infra(),
		func(ctx context.Context) error {
			b, err := os.ReadFile(path)
			if errors.Is(err, fs.ErrNotExist) {
				return nil
			} else if err != nil {
				return err
			}
			if err := json.Unmarshal(b, &stamp); err != nil {
				return skerr.Wrapf(err, "parsing stamp %s", path)
			}
			return nil
		},
	)
	return stamp, err
}

func fetchChromeLatestVersionNum() (string, error) {
	url := fmt.Sprintf("%s/LATEST_RELEASE_STABLE", chromeForTestingUrlBase)
	resp, err := http.Get(url)
	if err != nil {
		return "", fmt.Errorf("failed to fetch %s: %w", url, err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		return "", fmt.Errorf("failed to fetch %s: %s", url, resp.Status)
	}

	body, err := io.ReadAll(resp.Body)
	if err != nil {
		return "", fmt.Errorf("failed to read body: %w", err)
	}
	return string(body), nil
}

func fetchChromeVersions() (*chromeVersions, error) {
	url := fmt.Sprintf("%s/known-good-versions-with-downloads.json", chromeForTestingUrlBase)
	resp, err := http.Get(url)
	if err != nil {
		return nil, fmt.Errorf("failed to fetch %s: %w", url, err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		return nil, fmt.Errorf("failed to fetch %s: %s", url, resp.Status)
	}

	var versions chromeVersions
	if err := json.NewDecoder(resp.Body).Decode(&versions); err != nil {
		return nil, fmt.Errorf("failed to decode %s: %w", url, err)
	}

	return &versions, nil
}

func determineChromeDownloadSpec(
	latestVersion string,
	versions *chromeVersions,
) (chromeDlSpec, error) {
	platforms := map[string]map[string]string{
		"darwin":  {"amd64": "mac-x64", "arm64": "mac-arm64"},
		"linux":   {"amd64": "linux64"},
		"windows": {"386": "win32", "amd64": "win64"},
	}

	exes := map[string]string{
		"darwin":  "Google Chrome for Testing.app/Contents/MacOS/Google Chrome for Testing",
		"linux":   "chrome",
		"windows": "chrome.exe",
	}

	platform, ok := platforms[r.GOOS][r.GOARCH]
	if !ok {
		return chromeDlSpec{}, fmt.Errorf("no known chrome platform for %s/%s", r.GOOS, r.GOARCH)
	}

	zipUrl := ""
	for _, v := range versions.Versions {
		if v.Version == latestVersion {
			for _, p := range v.Downloads.Chrome {
				if p.Platform == platform {
					zipUrl = p.URL
					break
				}
			}
		}
		if len(zipUrl) > 0 {
			break
		}
	}
	if len(zipUrl) == 0 {
		return chromeDlSpec{}, fmt.Errorf(
			"failed to find zip url for chrome %s platform %s",
			latestVersion,
			platform,
		)
	}

	exePath := path.Join(fmt.Sprintf("chrome-%s", platform), exes[r.GOOS])
	return chromeDlSpec{
		platform: platform,
		zipUrl:   zipUrl,
		exePath:  exePath,
	}, nil
}

func downloadFile(ctx context.Context, urlStr, destPath string) error {
	u, err := url.Parse(urlStr)
	if err != nil {
		td.Fatal(ctx, skerr.Wrapf(err, "bad url: '%s'", urlStr))
	}

	f, err := os.Create(destPath)
	if err != nil {
		return fmt.Errorf("failed to create file: %w", err)
	}
	defer f.Close()

	client := td.HttpClient(ctx, httputils.DefaultClientConfig().With2xxOnly().Client())
	resp, err := client.Get(u.String())
	if err != nil {
		td.Fatal(ctx, skerr.Wrapf(err, "GET failed: '%s'", u.String()))
	}
	defer resp.Body.Close()

	if _, err := io.Copy(f, resp.Body); err != nil {
		return fmt.Errorf("failed to write: %w", err)
	}

	return nil
}

func extractZip(ctx context.Context, zipPath, targetDir string) error {
	runCmd := &exec.Command{
		Name:      "unzip",
		Args:      []string{"-oq", zipPath, "-d", targetDir},
		LogStderr: true,
	}
	_, err := exec.RunCommand(ctx, runCmd)
	if err != nil {
		return err
	}
	return nil
}
