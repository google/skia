package ctsdriver

import (
	"testing"

	assert "github.com/stretchr/testify/require"

	"go.skia.org/infra/go/fileutil"
)

func TestRunTests(t *testing.T) {
	outputDir, err := fileutil.EnsureDirExists("./testdata")
	assert.NoError(t, err)

	assert.NoError(t, Load("./knowledge.zip", outputDir))

	// lines, err := RecordTestResult(t)
	// assert.NoError(t, err)

	// fmt.Println("Output: \n\n")
	// for _, line := range lines {
	// 	fmt.Println(line)
	// }
}
