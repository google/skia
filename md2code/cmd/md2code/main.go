package main

// This program allows to connect simple functions to command line arguments
// to perform maintenance tasks on Cloud Datastore namespaces.

import (
	"flag"
	"fmt"
	"io/ioutil"
	"path/filepath"
	"strings"

	"go.skia.org/infra/go/fileutil"
	"go.skia.org/infra/go/skerr"

	"gopkg.in/russross/blackfriday.v2"

	"go.skia.org/infra/go/common"
	"go.skia.org/infra/go/sklog"
)

func main() {
	// Command line flags.
	var (
		input  = flag.String("in", "", "Input file.")
		outDir = flag.String("out", "./out", "Output directory.")
		envDir = flag.String("env", "./envs", "Env dir")
	)

	common.Init()

	content, err := ioutil.ReadFile(*input)
	if err != nil {
		sklog.Fatalf("Error reading file %q: %s", *input, err)
	}

	markDown := blackfriday.New(blackfriday.WithExtensions(blackfriday.CommonExtensions))
	node := markDown.Parse(content)
	sklog.Infof("Doc parsed")

	codeFragments, err := extractCode(node, *input)
	if err != nil {
		sklog.Fatalf("Error extracting code: %s", err)
	}
	sklog.Infof("Extracted: %d fragments", len(codeFragments))

	if err := writeCodeFragments(codeFragments, *envDir, *outDir); err != nil {
		sklog.Fatalf("Error writting code: %s", err)
	}
}

func parseCodeBlockInfo(cb *blackfriday.CodeBlockData) (string, string, error) {
	info := string(cb.Info)
	kvMap, err := parseKVPairs(info)
	if err != nil {
		return "", "", err
	}

	name, okName := kvMap["name"]
	codeEnv, okCodeEnv := kvMap["cenv"]
	if !(okName && okCodeEnv) {
		return "", "", skerr.Fmt("Code info needs to contain name and code env. Got %q", info)
	}
	return name, codeEnv, nil
}

func parseKVPairs(kvStr string) (map[string]string, error) {
	parts := strings.Split(kvStr, ",")
	ret := make(map[string]string, len(parts))
	for _, part := range parts {
		p := strings.TrimSpace(part)
		kv := strings.SplitN(p, "=", 2)
		if len(kv) != 2 {
			return nil, skerr.Fmt("Invalid key value pair: %q", p)
		}
		key := strings.TrimSpace(kv[0])
		if key == "" {
			return nil, skerr.Fmt("Key cannot be empty: %q", p)
		}
		ret[key] = strings.TrimSpace(kv[1])
	}
	return ret, nil
}

func extractCode(node *blackfriday.Node, inputFile string) ([]*CodeFragment, error) {
	ret := []*CodeFragment{}
	var err error = nil
	_, inputFile = filepath.Split(inputFile)
	visitor := func(node *blackfriday.Node, entering bool) blackfriday.WalkStatus {
		if entering && node.Type == blackfriday.CodeBlock {
			// Parse the info
			var name, codeEnv string
			name, codeEnv, err = parseCodeBlockInfo(&node.CodeBlockData)
			if err != nil {
				return blackfriday.Terminate
			}

			sklog.Infof("NODE: %s - %s", name, codeEnv)

			ret = append(ret, &CodeFragment{
				file:    inputFile,
				name:    name,
				codeEnv: codeEnv,
				code:    string(node.Literal),
				line:    node.FenceOffset,
			})
		}
		return blackfriday.GoToNext
	}

	node.Walk(visitor)
	if err != nil {
		return nil, err
	}
	return ret, nil
}

func writeCodeFragments(frags []*CodeFragment, envDir, outputDir string) error {
	allCodeEnvs := map[string]string{}
	for _, frag := range frags {
		codeEnv, ok := allCodeEnvs[frag.codeEnv]
		if !ok {
			envFile := filepath.Join(envDir, frag.codeEnv)
			sklog.Infof("envFile: %s", envFile)
			codeEnvBytes, err := ioutil.ReadFile(envFile)
			if err != nil {
				return err
			}
			codeEnv = string(codeEnvBytes)
			allCodeEnvs[frag.codeEnv] = string(codeEnv)
		}

		if err := frag.writeToDir(codeEnv, outputDir); err != nil {
			return err
		}
	}

	return nil
}

type CodeFragment struct {
	file    string
	line    int
	name    string
	codeEnv string
	code    string
}

func (c *CodeFragment) writeToDir(codeEnv string, outDir string) error {
	useDir, err := fileutil.EnsureDirExists(outDir)
	if err != nil {
		return err
	}

	fileName := c.file + "_" + c.name + ".cpp"
	outFile := filepath.Join(useDir, fileName)
	sklog.Infof("Outfile: %s", outFile)
	contentLoc := fmt.Sprintf("File: %s (Line %d)", c.file, c.line)
	fileContent := fmt.Sprintf("%s \n// %s\n %s\n\n", codeEnv, contentLoc, c.code)
	return ioutil.WriteFile(outFile, []byte(fileContent), 0444)
}
