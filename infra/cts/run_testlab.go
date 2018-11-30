/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

package main

import (
	"bytes"
	"context"
	"encoding/json"
	"flag"
	"fmt"
	"io"
	"io/ioutil"
	"net/http"
	"os"
	"os/exec"
	"sort"
	"strconv"
	"strings"
	"syscall"
	"time"

	"go.skia.org/infra/go/gcs"

	"cloud.google.com/go/storage"
	"google.golang.org/api/option"
	gstorage "google.golang.org/api/storage/v1"

	"go.skia.org/infra/go/auth"
	"go.skia.org/infra/go/common"
	"go.skia.org/infra/go/sklog"
	"go.skia.org/infra/go/util"
<<<<<<< HEAD   (ac7f23 SkQP: refatctor C++ bits.)
=======
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
>>>>>>> BRANCH (3e3428 SkQP: Remove tests that use too much RAM)
)

const (
	META_DATA_FILENAME = "meta.json"
)

// Command line flags.
var (
	devicesFile        = flag.String("devices", "", "JSON file that maps device ids to versions to run on. Same format as produced by the dump_devices flag.")
	dryRun             = flag.Bool("dryrun", false, "Print out the command and quit without triggering tests.")
<<<<<<< HEAD   (ac7f23 SkQP: refatctor C++ bits.)
	dumpDevFile        = flag.String("dump_devices", "", "Creates a JSON file with all physical devices that are not deprecated.")
	minAPIVersion      = flag.Int("min_api", 0, "Minimum API version required by device.")
	maxAPIVersion      = flag.Int("max_api", 99, "Maximum API version required by device.")
	properties         = flag.String("properties", "", "Custom meta data to be added to the uploaded APK. Comma separated list of key=value pairs, i.e. 'k1=v1,k2=v2,k3=v3.")
	serviceAccountFile = flag.String("service_account_file", "", "Credentials file for service account.")
	uploadGCSPath      = flag.String("upload_path", "", "GCS path (bucket/path) to where the APK should be uploaded to. It's assume to a full path (not a directory).")
=======
	minAPIVersion      = flag.Int("min_api", 22, "Minimum API version required by device.")
	maxAPIVersion      = flag.Int("max_api", 23, "Maximum API version required by device.")
>>>>>>> BRANCH (3e3428 SkQP: Remove tests that use too much RAM)
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
	MODEL_VERSION_TMPL    = "--device model=%s,version=%s,orientation=portrait"
	RESULT_BUCKET         = "skia-firebase-test-lab"
	RESULT_DIR_TMPL       = "testruns/%s/%s"
	RUN_ID_TMPL           = "testrun-%d"
	CMD_AVAILABLE_DEVICES = "gcloud firebase test android models list --format json"
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
		sklog.Fatalf("Error retrieving available devices: %s", err)
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

<<<<<<< HEAD   (ac7f23 SkQP: refatctor C++ bits.)
	// Filter the devices according the white list and other parameters.
	devices, ignoredDevices := filterDevices(fbDevices, whiteList, *minAPIVersion, *maxAPIVersion)
	sklog.Infof("---\nSelected devices:")
=======
	// Get list of all available devices.
	devices, ignoredDevices, err := getAvailableDevices(WHITELIST_DEV_IDS, *minAPIVersion, *maxAPIVersion)
	if err != nil {
		sklog.Fatalf("Unable to retrieve available devices: %s", err)
	}
	sklog.Infof("---")
	sklog.Infof("Selected devices:")
>>>>>>> BRANCH (3e3428 SkQP: Remove tests that use too much RAM)
	logDevices(devices)

	if len(devices) == 0 {
		sklog.Errorf("No devices selected. Not running tests.")
		os.Exit(1)
	}

	if err := runTests(apkPath, devices, ignoredDevices, client, *dryRun); err != nil {
		sklog.Fatalf("Error running tests on Firebase: %s", err)
	}

	if !*dryRun && (*uploadGCSPath != "") && (*properties != "") {
		if err := uploadAPK(apkPath, *uploadGCSPath, *properties, client); err != nil {
			sklog.Fatalf("Error uploading APK to '%s': %s", *uploadGCSPath, err)
		}
	}
}

<<<<<<< HEAD   (ac7f23 SkQP: refatctor C++ bits.)
// getAvailableDevices queries Firebase Testlab for all physical devices that
// are not deprecated. It returns two lists with the same information.
// The first contains all device information as returned by Firebase while
// the second contains the information necessary to use in a whitelist.
func getAvailableDevices() ([]*DeviceVersions, DeviceList, error) {
=======
// getAvailableDevices is given a whitelist. It queries Firebase Testlab for all
// available devices and then returns a list of devices to be tested and the list
// of ignored devices.
func getAvailableDevices(whiteList map[string][]string, minAPIVersion, maxAPIVersion int) ([]*tsuite.DeviceVersions, []*tsuite.DeviceVersions, error) {
>>>>>>> BRANCH (3e3428 SkQP: Remove tests that use too much RAM)
	// Get the list of all devices in JSON format from Firebase testlab.
	var buf bytes.Buffer
	var errBuf bytes.Buffer
	cmd := parseCommand(CMD_AVAILABLE_DEVICES)
	cmd.Stdout = &buf
	cmd.Stderr = io.MultiWriter(os.Stdout, &errBuf)
	if err := cmd.Run(); err != nil {
		return nil, nil, sklog.FmtErrorf("Error running: %s\nError:%s\nStdErr:%s", CMD_AVAILABLE_DEVICES, err, errBuf)
	}

	// Unmarshal the result.
	foundDevices := []*DeviceVersions{}
	bufBytes := buf.Bytes()
	if err := json.Unmarshal(bufBytes, &foundDevices); err != nil {
		return nil, nil, sklog.FmtErrorf("Unmarshal of device information failed: %s \nJSON Input: %s\n", err, string(bufBytes))
	}

<<<<<<< HEAD   (ac7f23 SkQP: refatctor C++ bits.)
	// Filter the devices and copy them to device list.
	devList := DeviceList{}
	ret := make([]*DeviceVersions, 0, len(foundDevices))
	for _, foundDev := range foundDevices {
		// Only consider physical devices and devices that are not deprecated.
		if (foundDev.Form == "PHYSICAL") && !util.In("deprecated", foundDev.Tags) {
			ret = append(ret, foundDev)
			devList = append(devList, &DevInfo{
				ID:          foundDev.ID,
				Name:        foundDev.Name,
				RunVersions: foundDev.VersionIDs,
			})
=======
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
>>>>>>> BRANCH (3e3428 SkQP: Remove tests that use too much RAM)
		}
	}
	return foundDevices, devList, nil
}

// filterDevices filters the given devices by ensuring that they are in the white list
// and within the given api version range.
// It returns two lists: (accepted_devices, ignored_devices)
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
			if len(ignoredVersions) > 0 {
				ignored = append(ignored, &DeviceVersions{FirebaseDevice: dev.FirebaseDevice, RunVersions: ignoredVersions})
			}
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
	var errBuf bytes.Buffer
	cmd := parseCommand(cmdStr)
	cmd.Stdout = os.Stdout
	cmd.Stderr = io.MultiWriter(os.Stdout, &errBuf)
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
			return sklog.FmtErrorf("Error running: %s\nError:%s\nStdErr:%s", cmdStr, err, errBuf)
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

	targetPath := fmt.Sprintf("%s/%s/%s", RESULT_BUCKET, resultsDir, META_DATA_FILENAME)
	if err := meta.writeToGCS(targetPath, client); err != nil {
		return err
	}
	sklog.Infof("Meta data written to gs://%s", targetPath)
	return nil
}

// uploadAPK uploads the APK at the given path to the bucket/path in gcsPath.
// The key-value pairs in propStr are set as custom meta data of the APK.
func uploadAPK(apkPath, gcsPath, propStr string, client *http.Client) error {
	properties, err := splitProperties(propStr)
	if err != nil {
		return err
	}
	apkFile, err := os.Open(apkPath)
	if err != nil {
		return err
	}
	defer util.Close(apkFile)

	if err := copyReaderToGCS(gcsPath, apkFile, client, "application/vnd.android.package-archive", properties, true, false); err != nil {
		return err
	}

	sklog.Infof("APK uploaded to gs://%s", gcsPath)
	return nil
}

// splitProperties receives a comma separated list of 'key=value' pairs and
// returnes them as a map.
func splitProperties(propStr string) (map[string]string, error) {
	splitProps := strings.Split(propStr, ",")
	properties := make(map[string]string, len(splitProps))
	for _, oneProp := range splitProps {
		kv := strings.Split(oneProp, "=")
		if len(kv) != 2 {
			return nil, sklog.FmtErrorf("Inavlid porperties format. Unable to parse '%s'", propStr)
		}
		properties[strings.TrimSpace(kv[0])] = strings.TrimSpace(kv[1])
	}
	return properties, nil
}

// logDevices logs the given list of devices.
func logDevices(devices []*DeviceVersions) {
	sklog.Infof("Found %d devices.", len(devices))
	for _, dev := range devices {
		fbDev := dev.FirebaseDevice
		sklog.Infof("%-15s %-30s %v / %v", fbDev.ID, fbDev.Name, fbDev.VersionIDs, dev.RunVersions)
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

// DeviceList is a simple list of devices, primarily used to define the
// whitelist of devices we want to run on.
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

// FirebaseDevice contains the information and JSON tags for device information
// returned by firebase.
type FirebaseDevice struct {
	Brand        string   `json:"brand"`
	Form         string   `json:"form"`
	ID           string   `json:"id"`
	Manufacturer string   `json:"manufacturer"`
	Name         string   `json:"name"`
	VersionIDs   []string `json:"supportedVersionIds"`
	Tags         []string `json:"tags"`
}

// DeviceVersions combines device information from Firebase Testlab with
// a selected list of versions. This is used to define a subset of versions
// used by a devices.
type DeviceVersions struct {
	*FirebaseDevice

	// RunVersions contains the version ids of interest contained in Device.
	RunVersions []string
}

// TestRunMeta contains the meta data of a complete testrun on firebase.
type TestRunMeta struct {
	ID             string            `json:"id"`
	TS             int64             `json:"timeStamp"`
	Devices        []*DeviceVersions `json:"devices"`
	IgnoredDevices []*DeviceVersions `json:"ignoredDevices"`
	ExitCode       int               `json:"exitCode"`
}

// writeToGCS writes the meta data as JSON to the given bucket and path in
// GCS. It assumes that the provided client has permissions to write to the
// specified location in GCS.
func (t *TestRunMeta) writeToGCS(gcsPath string, client *http.Client) error {
	jsonBytes, err := json.Marshal(t)
	if err != nil {
		return err
	}
	return copyReaderToGCS(gcsPath, bytes.NewReader(jsonBytes), client, "", nil, false, true)
}

// TODO(stephana): Merge copyReaderToGCS into the go/gcs in
// the infra repository.

// copyReaderToGCS reads all available content from the given reader and writes
// it to the given path in GCS.
func copyReaderToGCS(gcsPath string, reader io.Reader, client *http.Client, contentType string, metaData map[string]string, public bool, gzip bool) error {
	storageClient, err := storage.NewClient(context.Background(), option.WithHTTPClient(client))
	if err != nil {
		return err
	}
	bucket, path := gcs.SplitGSPath(gcsPath)
	w := storageClient.Bucket(bucket).Object(path).NewWriter(context.Background())

	// Set the content if requested.
	if contentType != "" {
		w.ObjectAttrs.ContentType = contentType
	}

	// Set the meta data if requested
	if metaData != nil {
		w.Metadata = metaData
	}

	// Make the object public if requested.
	if public {
		w.ACL = []storage.ACLRule{{Entity: storage.AllUsers, Role: storage.RoleReader}}
	}

	// Write the everything the reader can provide to the GCS object. Either
	// gzip'ed or plain.
	if gzip {
		w.ObjectAttrs.ContentEncoding = "gzip"
		err = util.WithGzipWriter(w, func(w io.Writer) error {
			_, err := io.Copy(w, reader)
			return err
		})
	} else {
		_, err = io.Copy(w, reader)
	}

	// Make sure we return an error when we close the remote object.
	if err != nil {
		_ = w.CloseWithError(err)
		return err
	}
	return w.Close()
}
