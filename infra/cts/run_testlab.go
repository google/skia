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
	"io/ioutil"
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
)

// TODO(stephana): Convert the hard coded whitelist to a command line flag that
// loads a file with the whitelisted devices and versions. Make sure to include
// human readable names for the devices.

// WHITELIST_DEV_IDS contains a mapping from the device id to the list of
// Android API versions that we should run agains. Usually this will be the
// latest version. To see available devices and version run with
// --dryrun flag or run '$ gcloud firebase test android models list'

const (
	META_DATA_FILENAME = "meta.json"
)

// Command line flags.
var (
	serviceAccountFile = flag.String("service_account_file", "", "Credentials file for service account.")
	dryRun             = flag.Bool("dryrun", false, "Print out the command and quit without triggering tests.")
	minAPIVersion      = flag.Int("min_api", 0, "Minimum API version required by device.")
	maxAPIVersion      = flag.Int("max_api", 99, "Maximum API version required by device.")
	devicesFile        = flag.String("devices", "", "JSON file that maps device ids to versions to run on. Same format as produced by the dump_devices flag.")
	dumpDevFile        = flag.String("dump_devices", "", "Creates a JSON file with all physical devices that are not deprecated.")
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

	// Get the path to the APK. It can be empty if we are dumping the device list.
	apkPath := flag.Arg(0)
	if *dumpDevFile == "" && apkPath == "" {
		sklog.Errorf("Missing APK. The APK file needs to be passed as the positional argument.")
		os.Exit(1)
	}

	// Get the available devices.
	fbDevices, deviceList, err := getAvailableDevices()
	if err != nil {
		sklog.Fatalf("Error retrieving devices: %s", err)
	}

	// Dump the device list and exit.
	if *dumpDevFile != "" {
		if err := writeDeviceList(*dumpDevFile, deviceList); err != nil {
			sklog.Fatalf("Unable to write devices: %s", err)
		}
		return
	}

	// If no devices are explicitly listed. Use all of them.
	whiteList := deviceList
	if *devicesFile != "" {
		whiteList, err = readDeviceList(*devicesFile)
		if err != nil {
			sklog.Fatalf("Error reading device file: %s", err)
		}
	}

	// Make sure we can authenticate locally and in the cloud.
	client, err := auth.NewJWTServiceAccountClient("", *serviceAccountFile, nil, gstorage.CloudPlatformScope, "https://www.googleapis.com/auth/userinfo.email")
	if err != nil {
		sklog.Fatalf("Failed to authenticate service account: %s. Run 'get_service_account' to obtain a service account file.", err)
	}

	// Filter the devices according the white list and other parameters.
	devices, ignoredDevices := filterDevices(fbDevices, whiteList, *minAPIVersion, *maxAPIVersion)
	if err != nil {
		sklog.Fatalf("Unable to retrieve available devices: %s", err)
	}
	sklog.Infof("---")
	sklog.Infof("Selected devices:")
	logDevices(devices)

	if len(devices) == 0 {
		sklog.Errorf("No devices selected. Not running tests.")
		os.Exit(1)
	}

	if err := runTests(apkPath, devices, ignoredDevices, client, *dryRun); err != nil {
		sklog.Fatalf("Error triggering tests on Firebase: %s", err)
	}
}

// getAvailableDevices is given a whitelist. It queries Firebase Testlab for all
// available devices and then returns a list of devices to be tested and the list
// of ignored devices.
func getAvailableDevices() ([]*DeviceVersions, DeviceList, error) {
	// Get the list of all devices in JSON format from Firebase testlab.
	var buf bytes.Buffer
	cmd := parseCommand(CMD_AVAILABE_DEVICES)
	cmd.Stdout = &buf
	cmd.Stderr = os.Stdout
	if err := cmd.Run(); err != nil {
		return nil, nil, err
	}

	// Unmarshal the result.
	foundDevices := []*DeviceVersions{}
	bufBytes := buf.Bytes()
	if err := json.Unmarshal(bufBytes, &foundDevices); err != nil {
		return nil, nil, sklog.FmtErrorf("Unmarshal of device information failed: %s \nJSON Input: %s\n", err, string(bufBytes))
	}

	// Filter the devices and copy them to device list.
	devList := DeviceList{}
	for i := 0; i < len(foundDevices); {
		// Only consider physical devices and devices that are not deprecated.
		if (foundDevices[i].Form != "PHYSICAL") || util.In("deprecated", foundDevices[i].Tags) {
			foundDevices = append(foundDevices[:i], foundDevices[i+1:]...)
		} else {
			devList = append(devList, &DevInfo{ID: foundDevices[i].ID,
				Name:        foundDevices[i].Name,
				RunVersions: foundDevices[i].VersionIDs})
			// devList[foundDevices[i].ID] = append(devList[foundDevices[i].ID], foundDevices[i].VersionIDs...)
			i++
		}
	}
	return foundDevices, devList, nil
}

func filterDevices(foundDevices []*DeviceVersions, whiteList DeviceList, minAPIVersion, maxAPIVersion int) ([]*DeviceVersions, []*DeviceVersions) {
	// iterate over the available devices and partition them.
	allDevices := make([]*DeviceVersions, 0, len(foundDevices))
	ret := make([]*DeviceVersions, 0, len(foundDevices))
	ignored := make([]*DeviceVersions, 0, len(foundDevices))
	for _, dev := range foundDevices {
		// Only include devices that are on the whitelist and have versions defined.
		if targetDev := whiteList.find(dev.ID); targetDev != nil && (len(targetDev.RunVersions) > 0) {
			versionSet := util.NewStringSet(dev.VersionIDs)
			reqVersions := util.NewStringSet(filterVersions(targetDev.RunVersions, minAPIVersion, maxAPIVersion))
			whiteListVersions := versionSet.Intersect(reqVersions).Keys()
			ignoredVersions := versionSet.Complement(reqVersions).Keys()
			sort.Strings(whiteListVersions)
			sort.Strings(ignoredVersions)
			if len(whiteListVersions) > 0 {
				ret = append(ret, &DeviceVersions{FirebaseDevice: dev.FirebaseDevice, RunVersions: whiteListVersions})
			}
			ignored = append(ignored, &DeviceVersions{FirebaseDevice: dev.FirebaseDevice, RunVersions: ignoredVersions})
		} else {
			ignored = append(ignored, &DeviceVersions{FirebaseDevice: dev.FirebaseDevice, RunVersions: dev.VersionIDs})
		}
		allDevices = append(allDevices, &DeviceVersions{FirebaseDevice: dev.FirebaseDevice, RunVersions: dev.VersionIDs})
	}

	sklog.Infof("All devices:")
	logDevices(allDevices)

	return ret, ignored
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
func runTests(apk_path string, devices, ignoredDevices []*DeviceVersions, client *http.Client, dryRun bool) error {
	// Get the model-version we want to test. Assume on average each model has 5 supported versions.
	modelSelectors := make([]string, 0, len(devices)*5)
	for _, devRec := range devices {
		for _, version := range devRec.RunVersions {
			modelSelectors = append(modelSelectors, fmt.Sprintf(MODEL_VERSION_TMPL, devRec.FirebaseDevice.ID, version))
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
	meta := &TestRunMeta{
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
func logDevices(devices []*DeviceVersions) {
	devMap := map[string][]string{}
	sklog.Infof("Found %d devices.", len(devices))
	for _, dev := range devices {
		fbDev := dev.FirebaseDevice
		sklog.Infof("%-15s %-30s %v / %v", fbDev.ID, fbDev.Name, fbDev.VersionIDs, dev.RunVersions)
		devMap[fbDev.ID] = append(devMap[fbDev.ID], fbDev.VersionIDs...)
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

// type DeviceList map[string][]string

type DeviceList []*DevInfo
type DevInfo struct {
	ID          string   `json:"id"`
	Name        string   `json:"name"`
	RunVersions []string `json:"runVersions"`
}

func (d DeviceList) find(id string) *DevInfo {
	for _, devInfo := range d {
		if devInfo.ID == id {
			return devInfo
		}
	}
	return nil
}

func writeDeviceList(fileName string, devList DeviceList) error {
	jsonBytes, err := json.MarshalIndent(devList, "", "  ")
	if err != nil {
		return sklog.FmtErrorf("Unable to encode JSON: %s", err)
	}

	if err := ioutil.WriteFile(fileName, jsonBytes, 0644); err != nil {
		sklog.FmtErrorf("Unable to write file '%s': %s", fileName, err)
	}
	return nil
}

func readDeviceList(fileName string) (DeviceList, error) {
	inFile, err := os.Open(fileName)
	if err != nil {
		return nil, sklog.FmtErrorf("Unable to open file '%s': %s", fileName, err)
	}
	defer util.Close(inFile)

	var devList DeviceList
	if err := json.NewDecoder(inFile).Decode(&devList); err != nil {
		return nil, sklog.FmtErrorf("Unable to decode JSON from '%s': %s", fileName, err)
	}
	return devList, nil
}
