/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

package main

import (
	"bytes"
	"encoding/json"
	"flag"
	"fmt"
	"net/http"
	"os"
	"os/exec"
	"sort"
	"strconv"
	"strings"
	"syscall"
	"time"

	gstorage "google.golang.org/api/storage/v1"

	"go.skia.org/infra/go/auth"
	"go.skia.org/infra/go/common"
	"go.skia.org/infra/go/sklog"
	"go.skia.org/infra/go/util"
	"go.skia.org/infra/golden/go/tsuite"
)

// TODO(stephana): Convert the hard coded whitelist to a command line flag that
// loads a file with the whitelisted devices and versions. Make sure to include
// human readable names for the devices.

var (
	// WHITELIST_DEV_IDS contains a mapping from the device id to the list of
	// Android API versions that we should run agains. Usually this will be the
	// latest version. To see available devices and version run with
	// --dryrun flag or run '$ gcloud firebase test android models list'

	WHITELIST_DEV_IDS = map[string][]string{
		"A0001": {"22"},
		// "E5803":       {"22"},    deprecated
		// "F5121":       {"23"},    deprecated
		"G8142":      {"25"},
		"HWMHA":      {"24"},
		"SH-04H":     {"23"},
		"athene":     {"23"},
		"athene_f":   {"23"},
		"hammerhead": {"23"},
		"harpia":     {"23"},
		"hero2lte":   {"23"},
		"herolte":    {"24"},
		"j1acevelte": {"22"},
		"j5lte":      {"23"},
		"j7xelte":    {"23"},
		"lucye":      {"24"},
		// "mako":        {"22"},   deprecated
		"osprey_umts": {"22"},
		// "p1":          {"22"},   deprecated
		"sailfish": {"26"},
		"shamu":    {"23"},
		"trelte":   {"22"},
		"zeroflte": {"22"},
		"zerolte":  {"22"},
	}
)

const (
	META_DATA_FILENAME = "meta.json"
)

// Command line flags.
var (
	serviceAccountFile = flag.String("service_account_file", "", "Credentials file for service account.")
	dryRun             = flag.Bool("dryrun", false, "Print out the command and quit without triggering tests.")
	minAPIVersion      = flag.Int("min_api", 22, "Minimum API version required by device.")
	maxAPIVersion      = flag.Int("max_api", 23, "Maximum API version required by device.")
)

const (
	RUN_TESTS_TEMPLATE = `gcloud beta firebase test android run
	--type=game-loop
	--app=%s
	--results-bucket=%s
	--results-dir=%s
	--directories-to-pull=/sdcard/Android/data/org.skia.skqp
	--timeout 30m
	%s
`
	MODEL_VERSION_TMPL   = "--device model=%s,version=%s,orientation=portrait"
	RESULT_BUCKET        = "skia-firebase-test-lab"
	RESULT_DIR_TMPL      = "testruns/%s/%s"
	RUN_ID_TMPL          = "testrun-%d"
	CMD_AVAILABE_DEVICES = "gcloud firebase test android models list --format json"
)

func main() {
	common.Init()

	// Get the apk.
	args := flag.Args()
	apk_path := args[0]

	// Make sure we can get the service account client.
	client, err := auth.NewJWTServiceAccountClient("", *serviceAccountFile, nil, gstorage.CloudPlatformScope, "https://www.googleapis.com/auth/userinfo.email")
	if err != nil {
		sklog.Fatalf("Failed to authenticate service account: %s. Run 'get_service_account' to obtain a service account file.", err)
	}

	// Get list of all available devices.
	devices, ignoredDevices, err := getAvailableDevices(WHITELIST_DEV_IDS, *minAPIVersion, *maxAPIVersion)
	if err != nil {
		sklog.Fatalf("Unable to retrieve available devices: %s", err)
	}
	sklog.Infof("---")
	sklog.Infof("Selected devices:")
	logDevices(devices)

	if err := runTests(apk_path, devices, ignoredDevices, client, *dryRun); err != nil {
		sklog.Fatalf("Error triggering tests on Firebase: %s", err)
	}
}

// getAvailableDevices is given a whitelist. It queries Firebase Testlab for all
// available devices and then returns a list of devices to be tested and the list
// of ignored devices.
func getAvailableDevices(whiteList map[string][]string, minAPIVersion, maxAPIVersion int) ([]*tsuite.DeviceVersions, []*tsuite.DeviceVersions, error) {
	// Get the list of all devices in JSON format from Firebase testlab.
	var buf bytes.Buffer
	cmd := parseCommand(CMD_AVAILABE_DEVICES)
	cmd.Stdout = &buf
	cmd.Stderr = os.Stdout
	if err := cmd.Run(); err != nil {
		return nil, nil, err
	}

	// Unmarshal the result.
	foundDevices := []*tsuite.FirebaseDevice{}
	bufBytes := buf.Bytes()
	if err := json.Unmarshal(bufBytes, &foundDevices); err != nil {
		return nil, nil, sklog.FmtErrorf("Unmarshal of device information failed: %s \nJSON Input: %s\n", err, string(bufBytes))
	}

	// iterate over the available devices and partition them.
	allDevices := make([]*tsuite.DeviceVersions, 0, len(foundDevices))
	ret := make([]*tsuite.DeviceVersions, 0, len(foundDevices))
	ignored := make([]*tsuite.DeviceVersions, 0, len(foundDevices))
	for _, dev := range foundDevices {
		// Filter out all the virtual devices.
		if dev.Form == "PHYSICAL" {
			// Only include devices that are on the whitelist and have versions defined.
			if foundVersions, ok := whiteList[dev.ID]; ok && (len(foundVersions) > 0) {
				versionSet := util.NewStringSet(dev.VersionIDs)
				reqVersions := util.NewStringSet(filterVersions(foundVersions, minAPIVersion, maxAPIVersion))
				whiteListVersions := versionSet.Intersect(reqVersions).Keys()
				ignoredVersions := versionSet.Complement(reqVersions).Keys()
				sort.Strings(whiteListVersions)
				sort.Strings(ignoredVersions)
				ret = append(ret, &tsuite.DeviceVersions{Device: dev, Versions: whiteListVersions})
				ignored = append(ignored, &tsuite.DeviceVersions{Device: dev, Versions: ignoredVersions})
			} else {
				ignored = append(ignored, &tsuite.DeviceVersions{Device: dev, Versions: dev.VersionIDs})
			}
			allDevices = append(allDevices, &tsuite.DeviceVersions{Device: dev, Versions: dev.VersionIDs})
		}
	}

	sklog.Infof("All devices:")
	logDevices(allDevices)

	return ret, ignored, nil
}

// filterVersions returns the elements in versionIDs where minVersion <= element <= maxVersion.
func filterVersions(versionIDs []string, minVersion, maxVersion int) []string {
	ret := make([]string, 0, len(versionIDs))
	for _, versionID := range versionIDs {
		id, err := strconv.Atoi(versionID)
		if err != nil {
			sklog.Fatalf("Error parsing version id '%s': %s", versionID, err)
		}
		if (id >= minVersion) && (id <= maxVersion) {
			ret = append(ret, versionID)
		}
	}
	return ret
}

// runTests runs the given apk on the given list of devices.
func runTests(apk_path string, devices, ignoredDevices []*tsuite.DeviceVersions, client *http.Client, dryRun bool) error {
	// Get the model-version we want to test. Assume on average each model has 5 supported versions.
	modelSelectors := make([]string, 0, len(devices)*5)
	for _, devRec := range devices {
		for _, version := range devRec.Versions {
			modelSelectors = append(modelSelectors, fmt.Sprintf(MODEL_VERSION_TMPL, devRec.Device.ID, version))
		}
	}

	now := time.Now()
	nowMs := now.UnixNano() / int64(time.Millisecond)
	runID := fmt.Sprintf(RUN_ID_TMPL, nowMs)
	resultsDir := fmt.Sprintf(RESULT_DIR_TMPL, now.Format("2006/01/02/15"), runID)
	cmdStr := fmt.Sprintf(RUN_TESTS_TEMPLATE, apk_path, RESULT_BUCKET, resultsDir, strings.Join(modelSelectors, "\n"))
	cmdStr = strings.TrimSpace(strings.Replace(cmdStr, "\n", " ", -1))

	// Run the command.
	cmd := parseCommand(cmdStr)
	cmd.Stdout = os.Stdout
	cmd.Stderr = os.Stdout
	exitCode := 0

	if dryRun {
		fmt.Printf("[dry run]: Would have run this command: %s\n", cmdStr)
		return nil
	}

	if err := cmd.Run(); err != nil {
		// Get the exit code.
		if exitError, ok := err.(*exec.ExitError); ok {
			ws := exitError.Sys().(syscall.WaitStatus)
			exitCode = ws.ExitStatus()
		}
		sklog.Errorf("Error running tests: %s", err)
		sklog.Errorf("Exit code: %d", exitCode)

		// Exit code 10 means triggering on Testlab succeeded, but but some of the
		// runs on devices failed. We consider it a success for this script.
		if exitCode != 10 {
			return err
		}
	}

	// Store the result in a meta json file.
	meta := &tsuite.TestRunMeta{
		ID:             runID,
		TS:             nowMs,
		Devices:        devices,
		IgnoredDevices: ignoredDevices,
		ExitCode:       exitCode,
	}

	meta.WriteToGCS(RESULT_BUCKET, resultsDir+"/"+META_DATA_FILENAME, client)
	return nil
}

// logDevices logs the given list of devices.
func logDevices(devices []*tsuite.DeviceVersions) {
	sklog.Infof("Found %d devices.", len(devices))
	for _, dev := range devices {
		sklog.Infof("%-15s %-30s %v / %v", dev.Device.ID, dev.Device.Name, dev.Device.VersionIDs, dev.Versions)
	}
}

// parseCommad parses a command line and wraps it in an exec.Command instance.
func parseCommand(cmdStr string) *exec.Cmd {
	cmdArgs := strings.Split(strings.TrimSpace(cmdStr), " ")
	for idx := range cmdArgs {
		cmdArgs[idx] = strings.TrimSpace(cmdArgs[idx])
	}
	return exec.Command(cmdArgs[0], cmdArgs[1:]...)
}
