package main

import (
	"bytes"
	"context"
	"encoding/json"
	"flag"
	"fmt"
	"net/http"
	"os"
	"os/exec"
	"strings"
	"syscall"
	"time"

	"cloud.google.com/go/storage"
	"google.golang.org/api/option"
	gstorage "google.golang.org/api/storage/v1"

	"go.skia.org/infra/go/auth"
	"go.skia.org/infra/go/common"
	"go.skia.org/infra/go/sklog"
	"go.skia.org/infra/go/util"
	"go.skia.org/infra/golden/go/ctseval"
	"go.skia.org/infra/golden/go/tsuite"
)

type DevRec struct {
	ID       string
	Name     string
	Versions []string
}

// Mapping from Device ids to name and version information.
// Generated from running:
//
//     gcloud firebase test android models list --format json > all-files.json
//
// var TEST_DEV_VERSIONS = map[string]*DevRec{
// 	"A0001":       &DevRec{ID: "A0001", Name: "OnePlus One", Versions: []string{"22"}},
// 	"D6503":       &DevRec{ID: "D6503", Name: "Xperia Z2", Versions: []string{"21"}},
// 	"D6603":       &DevRec{ID: "D6603", Name: "Xperia Z3", Versions: []string{"21"}},
// 	"E5803":       &DevRec{ID: "E5803", Name: "Xperia Z5 Compact", Versions: []string{"22"}},
// 	"F5121":       &DevRec{ID: "F5121", Name: "Sony Xperia X", Versions: []string{"23"}},
// 	"SH-04H":      &DevRec{ID: "SH-04H", Name: "SH-04H", Versions: []string{"23"}},
// 	"athene":      &DevRec{ID: "athene", Name: "Moto G4 Plus", Versions: []string{"23"}},
// 	"athene_f":    &DevRec{ID: "athene_f", Name: "Moto G4", Versions: []string{"23"}},
// 	"condor_umts": &DevRec{ID: "condor_umts", Name: "Moto E", Versions: []string{"19"}},
// 	"falcon_umts": &DevRec{ID: "falcon_umts", Name: "Moto G (1st Gen)", Versions: []string{"19"}},
// 	"flo":         &DevRec{ID: "flo", Name: "Nexus 7 (2013)", Versions: []string{"19", "21"}},
// 	"flounder":    &DevRec{ID: "flounder", Name: "Nexus 9", Versions: []string{"21"}},
// 	"g3":          &DevRec{ID: "g3", Name: "LG G3", Versions: []string{"19"}},
// 	"hammerhead":  &DevRec{ID: "hammerhead", Name: "Nexus 5", Versions: []string{"19", "21", "22", "23"}},
// 	"hero2lte":    &DevRec{ID: "hero2lte", Name: "Galaxy S7 edge", Versions: []string{"23"}},
// 	"herolte":     &DevRec{ID: "herolte", Name: "Galaxy S7", Versions: []string{"23", "24"}},
// 	"hlte":        &DevRec{ID: "hlte", Name: "Galaxy Note 3 Duos", Versions: []string{"19"}},
// 	"htc_m8":      &DevRec{ID: "htc_m8", Name: "HTC One (M8)", Versions: []string{"19"}},
// 	"j1acevelte":  &DevRec{ID: "j1acevelte", Name: "Galaxy J1 ace SM-J111M", Versions: []string{"22"}},
// 	"j5lte":       &DevRec{ID: "j5lte", Name: "Galaxy J5", Versions: []string{"23"}},
// 	"j7xelte":     &DevRec{ID: "j7xelte", Name: "Galaxy J7 (SM-J710MN)", Versions: []string{"23"}},
// 	"lt02wifi":    &DevRec{ID: "lt02wifi", Name: "Galaxy Tab 3", Versions: []string{"19"}},
// 	"m0":          &DevRec{ID: "m0", Name: "Samsung Galaxy S3", Versions: []string{"18"}},
// 	"mako":        &DevRec{ID: "mako", Name: "Nexus 4", Versions: []string{"19", "22"}},
// 	"osprey_umts": &DevRec{ID: "osprey_umts", Name: "Moto G (3rd Gen)", Versions: []string{"22"}},
// 	"p1":          &DevRec{ID: "p1", Name: "LG G4", Versions: []string{"22"}},
// 	"sailfish":    &DevRec{ID: "sailfish", Name: "Pixel", Versions: []string{"25", "26"}},
// 	"serranolte":  &DevRec{ID: "serranolte", Name: "Galaxy S4 mini", Versions: []string{"19"}},
// 	"shamu":       &DevRec{ID: "shamu", Name: "Nexus 6", Versions: []string{"21", "22", "23"}},
// 	"t03g":        &DevRec{ID: "t03g", Name: "Galaxy Note 2", Versions: []string{"19"}},
// 	"titan_umts":  &DevRec{ID: "titan_umts", Name: "Moto G (2nd Gen)", Versions: []string{"19"}},
// 	"trelte":      &DevRec{ID: "trelte", Name: "Galaxy Note 4", Versions: []string{"22"}},
// 	"victara":     &DevRec{ID: "victara", Name: "Moto X", Versions: []string{"19"}},
// 	"zeroflte":    &DevRec{ID: "zeroflte", Name: "Galaxy S6", Versions: []string{"22"}},
// 	"zerolte":     &DevRec{ID: "zerolte", Name: "Galaxy S6 Edge", Versions: []string{"22"}},
// }

// var TEST_DEV_VERSIONS = map[string]*DevRec{
// 	"hammerhead": &DevRec{ID: "hammerhead", Name: "Nexus 5", Versions: []string{"19", "21", "22", "23"}},
// }

// Command line flags.
var (
	serviceAccountFile = flag.String("service_account_file", "", "Credentials file for service account.")
)

const (
	RUN_TESTS_TEMPLATE = `gcloud beta firebase test android run
	--type=game-loop
	--app=%s
	--results-bucket=%s
	--results-dir=%s
	--directories-to-pull=/sdcard/Android/data/org.skia.cts18
	%s
`
	MODEL_VERSION_TMPL   = "--device model=%s,version=%s,orientation=portrait"
	RESULT_BUCKET        = "skia-firebase-test-lab"
	RESULT_DIR_TMPL      = "testruns/%s/%s"
	RUN_ID_TMPL          = "testrun-%d"
	CMD_AVAILABE_DEVICES = "gcloud firebase test android models list --format json"
)

var (
	BLACKLIST_DEV_IDS = []string{
		"lt02wifi", // ignore Galaxy Tab 3 which crashes.
		// "A0001",
		// "D6503",
		// "D6603",
		// "E5803",
		// "F5121",
		// "SH-04H",
		// "athene",
		// "athene_f",
		// "condor_umts",
		// "falcon_umts",
		// "flo",
		// "flounder",
		// "g3",
		// // 		"hammerhead",
		// "hero2lte",
		// "herolte",
		// "hlte",
		// "htc_m8",
		// "j1acevelte",
		// "j5lte",
		// "j7xelte",
		// "lt02wifi",
		// "m0",
		// "mako",
		// "osprey_umts",
		// "p1",
		// "sailfish",
		// "serranolte",
		// "shamu",
		// "t03g",
		// "titan_umts",
		// "trelte",
		// "victara",
		// "zeroflte",
		// "zerolte",
	}
)

func main() {
	common.Init()

	args := flag.Args()
	apk_path := args[0]

	// Make sure we can get the service account client.
	client, err := auth.NewJWTServiceAccountClient("", *serviceAccountFile, nil, gstorage.CloudPlatformScope, "https://www.googleapis.com/auth/userinfo.email")
	if err != nil {
		sklog.Fatalf("Failed to authenticate service account: %s", err)
	}

	devices, ignoredDevices, err := getAvailableDevices(BLACKLIST_DEV_IDS)
	if err != nil {
		sklog.Fatalf("Unable to retrieve available devices: %s", err)
	}

	sklog.Infof("Found %d devices.", len(devices))
	for _, dev := range devices {
		sklog.Infof("%-15s %-30s %v", dev.ID, dev.Name, dev.VersionIDs)
	}

	if err := runTests(apk_path, devices, ignoredDevices, client); err != nil {
		sklog.Fatalf("Error triggering tests on Firebase: %s", err)
	}
}

func parseCommand(cmdStr string) *exec.Cmd {
	cmdArgs := strings.Split(strings.TrimSpace(cmdStr), " ")
	for idx := range cmdArgs {
		cmdArgs[idx] = strings.TrimSpace(cmdArgs[idx])
	}
	return exec.Command(cmdArgs[0], cmdArgs[1:]...)
}

func getAvailableDevices(ignoreIDs []string) ([]*tsuite.FirebaseDevice, []*tsuite.FirebaseDevice, error) {
	var buf bytes.Buffer
	cmd := parseCommand(CMD_AVAILABE_DEVICES)
	cmd.Stdout = &buf
	cmd.Stderr = os.Stdout
	if err := cmd.Run(); err != nil {
		return nil, nil, err
	}

	foundDevices := []*tsuite.FirebaseDevice{}
	if err := json.Unmarshal(buf.Bytes(), &foundDevices); err != nil {
		return nil, nil, err
	}

	// Filter out all the virtual devices.
	ret := make([]*tsuite.FirebaseDevice, 0, len(foundDevices))
	ignored := make([]*tsuite.FirebaseDevice, 0, len(foundDevices))
	ignoreMap := util.NewStringSet(ignoreIDs)
	for _, dev := range foundDevices {
		if (dev.Form == "PHYSICAL") && !ignoreMap[dev.ID] {
			ret = append(ret, dev)
		} else {
			ignored = append(ignored, dev)
		}
	}

	return ret, ignored, nil
}

func runTests(apk_path string, devices, ignoredDevices []*tsuite.FirebaseDevice, client *http.Client) error {
	// Get the model-version we want to test. Assume on average each model has 5 supported versions.
	modelSelectors := make([]string, 0, len(devices)*5)
	for _, devRec := range devices {
		for _, version := range devRec.VersionIDs {
			modelSelectors = append(modelSelectors, fmt.Sprintf(MODEL_VERSION_TMPL, devRec.ID, version))
		}
	}

	now := time.Now().Add(time.Hour * 8)
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

	if err := cmd.Run(); err != nil {
		// Get the exit code.
		if exitError, ok := err.(*exec.ExitError); ok {
			ws := exitError.Sys().(syscall.WaitStatus)
			exitCode = ws.ExitStatus()
		}
		sklog.Errorf("Error running tests: %s", err)
		sklog.Errorf("Exit code: %d", exitCode)

		// Exit code 10 means some devices failed.
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

	writeMetaData(meta, RESULT_BUCKET, resultsDir+"/"+ctseval.MetaDataFileName, client)
	return nil
}

func writeMetaData(metaData *tsuite.TestRunMeta, bucket, path string, client *http.Client) error {
	storageClient, err := storage.NewClient(context.Background(), option.WithHTTPClient(client))
	if err != nil {
		return err
	}

	w := storageClient.Bucket(bucket).Object(path).NewWriter(context.Background())
	if err := json.NewEncoder(w).Encode(metaData); err != nil {
		return err
	}
	defer util.Close(w)

	sklog.Infof("Sucess: Meta data written to %s/%s", bucket, path)
	return nil
}
