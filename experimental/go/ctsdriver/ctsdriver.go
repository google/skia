package ctsdriver

import (
	"fmt"
	"image"
	"os"
	"path/filepath"
	"strings"

	"go.skia.org/infra/go/fileutil"
	"go.skia.org/infra/golden/go/ctseval"
	"go.skia.org/infra/golden/go/diff"
	"go.skia.org/infra/golden/go/tsuite"
)

// ByteOrder constants.
const (
	NRGBA = iota
	RGBA
	ARGB
	RGB
	BGR
)

type Image struct {
	Pix []uint8

	Width int

	Height int

	ByteOrder int
}

// Module level variables.

// suite is the test suite that processes tests results.
var suite *tsuite.CompatTestSuite = nil

// outputDir is the output directory.
var outputDir string = ""
var suiteResult = &tsuite.SuiteResult{}

// Load the test suite from disk and set the output path.
// If the output string is empty the results are going to be
// uploaded.
func Load(testSuiteZipPath string, oDir string) error {
	var err error = nil
	suite, err = tsuite.LoadFromFile(testSuiteZipPath)
	if err != nil {
		return err
	}

	outputDir = oDir
	return nil
}

// Finished marks the end. This causes a meta data file to be written.
func Finished() string {
	fName := filepath.Join(outputDir, ctseval.ResultFileName)
	if err := suiteResult.WriteToFile(fName); err != nil {
		return err.Error()
	}
	return ""
}

func TestNames() string {
	return strings.Join(suite.TestNames(), "|")
}

func RecordTestResult(testName string, img *Image, errMsg string) {
	// If there is an error message then we record the result.
	if errMsg != "" {
		suiteResult.Add(testName, tsuite.ERR, errMsg)
		return
	}

	// Convert the image to an NRGBA images.
	// var resultImg *image.NRGBA = nil
	nrgbaImg := image.NewNRGBA(image.Rect(0, 0, img.Width, img.Height))
	copy(nrgbaImg.Pix, img.Pix)

	fName := filepath.Join(outputDir, testName+".png")
	if err := fileutil.EnsureDirPathExists(fName); err != nil {
		suiteResult.Add(testName, tsuite.SAVE_FAILED, fmt.Sprintf("Unable to create output dir: %s", err))
		return
	}

	outFile, err := os.Create(fName)
	if err != nil {
		suiteResult.Add(testName, tsuite.SAVE_FAILED, fmt.Sprintf("Unable to create output file: %s", err))
		return
	}

	err = func() error {
		defer outFile.Close()
		return diff.WritePNG(outFile, nrgbaImg)
	}()

	if err != nil {
		suiteResult.Add(testName, tsuite.SAVE_FAILED, fmt.Sprintf("Unable to write png: %s", err))
		return
	}
	suiteResult.Add(testName, tsuite.OK, "")
}
