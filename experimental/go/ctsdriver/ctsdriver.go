package ctsdriver

import (
	"bytes"
	"fmt"
	"image"
	"os"
	"path/filepath"
	"strings"

	"go.skia.org/infra/go/fileutil"

	"go.skia.org/infra/go/upload"
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

// Load the test suite from disk.
func Load(testSuiteZipPath string, oDir string) error {
	var err error = nil
	suite, err = tsuite.Load(testSuiteZipPath)
	if err != nil {
		return err
	}

	outputDir = oDir
	return nil
}

func TestNames() string {
	return strings.Join(suite.TestNames(), "|")
}

func RecordTestResult(testName string, img *Image) (string, error) {
	// Convert the image to an NRGBA images.
	// var resultImg *image.NRGBA = nil

	classifiers := suite.GetClassifiers()
	var retBuf bytes.Buffer

	for testName, classifier := range classifiers {
		cf := classifier.(*tsuite.Memorizer)
		imgs := cf.GetImages()
		for digest, img := range imgs {
			fName := filepath.Join(outputDir, testName, digest+".png")
			if err := fileutil.EnsureDirPathExists(fName); err != nil {
				return retBuf.String(), err
			}

			outFile, err := os.Create(fName)
			if err != nil {
				return retBuf.String(), err
			}

			err = func() error {
				defer outFile.Close()
				return diff.WritePNG(outFile, img)
			}()

			if err != nil {
				return retBuf.String(), err
			}
			_, _ = fmt.Fprintf(&retBuf, "Wrote %s\n", fName)

			md5Hash, err := upload.UploadFile(fName, "http://104.154.66.138/upload")
			if err != nil {
				return retBuf.String(), fmt.Errorf("Unable to upload file %s. Got error: %s", fName, err)
			}
			_, _ = fmt.Fprintf(&retBuf, "Uploaded (%s): %s\n", md5Hash, fName)
		}
	}

	return retBuf.String(), nil
}

// Evaluate a test. digest is the MD5 hash of the internal pixel buffer
// to speed up evaluation. The returned value is the probability of a pass.
func evaluate(testName, digest string, img *image.NRGBA) float32 {
	ret, _ := suite.Evaluate(testName, digest, img)
	return ret
}
