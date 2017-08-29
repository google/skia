package main

import (
	"flag"
	"fmt"
	"strings"
	"time"

	"go.skia.org/infra/go/common"
	"go.skia.org/infra/go/sklog"
)

type DevRec struct {
	ID       string
	Name     string
	Versions []string
}

// APK_PATH=$1
// TEST_RUN_ID=test-run-$()

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

var TEST_DEV_VERSIONS = map[string]*DevRec{
	"hammerhead": &DevRec{ID: "hammerhead", Name: "Nexus 5", Versions: []string{"19", "21", "22", "23"}},
}

const (
	// 	RUN_TESTS_TEMPLATE = `gcloud beta firebase test android run
	// --type=game-loop
	// --app=%s
	// %s`

	RUN_TESTS_TEMPLATE = `gcloud beta firebase test android run
	--type=game-loop
	--app=%s
	--results-bucket="skia-firebase-test-lab"
	--results-dir="%s"
	--directories-to-pull="/data/user/0/org.skia.cts18/files"
	%s
`

	MODEL_VERSION_TMPL = "--device model=%s,version=%s,orientation=portrait"
)

func main() {
	common.Init()

	args := flag.Args()
	apk_path := args[0]

	devices := make([]string, 0, len(TEST_DEV_VERSIONS))
	for devID, devRec := range TEST_DEV_VERSIONS {
		devices = append(devices, fmt.Sprintf(MODEL_VERSION_TMPL, devID, devRec.Versions[len(devRec.Versions)-1]))
	}

	results_dir := fmt.Sprintf("testrun-%d", time.Now().UnixNano()/int64(time.Millisecond))
	cmd := fmt.Sprintf(RUN_TESTS_TEMPLATE, apk_path, results_dir, strings.Join(devices, "\n"))
	sklog.Infof("CMD: %s\n\n", cmd)
	sklog.Infof("run: %s", strings.Replace(cmd, "\n", " ", -1))

	// # DEVICE_IDS maps device ids to OS versions.
	// declare -A DEVICE_IDS
	// DEVICE_IDS[]=
	// # BUILD_IDS[arm]=armeabi-v7a
	// # BUILD_IDS[x64]=x86_64
	// # BUILD_IDS[x86]=x86
	// # BUILD_IDS[mipsel]=mips
	// # BUILD_IDS[mips64el]=mips64

	// DEVICE_ID=Nexus5
	// DEVICE_VERSION=23

	// gcloud beta firebase test android run \
	//                       --type=game-loop \
	//                       --app=${APK_PATH} \
	//                       --results-bucket=skia-firebase-test-lab \
	//                       --results-dir=${TEST_RUN_ID}/${DEVICE_ID} \
	//                       --directories-to-pull=/data/user/0/org.skia.cts18/files \
	//                       --device model=${DEVICE_ID},version=${DEVICE_VERSION}
	//
}
