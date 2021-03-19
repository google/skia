// Ports over all the content in //site into //site2 adding page metadata, and
// possibly moving files to new locations.
package main

import (
	"encoding/json"
	"fmt"
	"io"
	"io/ioutil"
	"log"
	"os"
	"path/filepath"
	"strings"
	"text/template"

	"go.skia.org/infra/go/util"
)

/*
{
  "dirOrder": [
    "contrib",
    "tools",
    "testing",
    "design",
    "present",
    "runtime",
    "chrome",
    "gardening"
  ],
  "fileOrder": []
}
*/
type Metadata struct {
	DirOrder  []string `json:"dirOrder"`
	FileOrder []string `json:"fileOrder"`
}

type Context struct {
	Title  string
	Body   string
	Weight int
}

const prefix = `
---
title: "{{ .Title }}"
linkTitle: "{{ .Title }}"
{{ if (gt .Weight 0) }}
weight: {{ .Weight }}
{{ end }}
---

{{ .Body }}
`

func main() {
	tmpl := template.Must(template.New("prefix").Parse(prefix))
	metadataByDir := map[string]Metadata{}
	// Copy files into ../site2/docs.
	const targetDir = "../site2/docs"
	err := filepath.Walk(".", func(path string, info os.FileInfo, err error) error {
		if err != nil {
			log.Fatal(err)
			return err
		}
		if info.IsDir() {
			// Don't do anything with dirs.
			return nil
		}
		ext := filepath.Ext(path)
		filename := filepath.Base(path)
		filenameWithoutExtension := strings.Split(filename, ".")[0]
		dir := filepath.Dir(path)
		fmt.Printf("visited file: %q %q %q\n", path, ext, filename)
		if ext == "" {
			fmt.Printf("Found METADATA for %q\n", dir)
			util.WithReadFile(path, func(r io.Reader) error {
				var m Metadata
				err := json.NewDecoder(r).Decode(&m)
				if err != nil {
					return err
				}
				metadataByDir[dir] = m
				fmt.Printf("Metadata: %#v\n", metadataByDir)
				return nil
			})

			return nil
		}
		weight := 0
		for index, filename := range metadataByDir[dir].FileOrder {
			if filename == filenameWithoutExtension {
				weight = (index + 1) * 10
			}
		}
		if filename == "index.md" {
			fmt.Printf("Re-write as _index.md.\n")
			filename = "_index.md"
			// Calculate weight differently.
			// First pop dir from end of path.
			parentDir := filepath.Dir(dir)
			currentDirName := filepath.Base(dir)
			if parentDir == "" {
				parentDir = "."
			}
			fmt.Println("Weight for: ", parentDir, currentDirName, metadataByDir[parentDir].FileOrder)
			for index, filename := range metadataByDir[parentDir].DirOrder {
				if filename == currentDirName {
					weight = (index + 1)
				}
			}
		}

		if ext == ".md" {
			fmt.Printf("Inject metadata header.\n")
			b, err := ioutil.ReadFile(path)
			if err != nil {
				log.Fatal(err)
			}
			body := string(b)
			// We need to handle both kinds of titles, ones prefixed with # and other where the next line is ====.
			parts := strings.SplitN(body, "\n", 3)
			header := parts[0]
			secondLine := parts[1]
			remainder := parts[2]
			// Strip the # off
			if strings.HasPrefix(header, "#") {
				header = header[1:]
			}
			if strings.HasPrefix(secondLine, "=") {
				secondLine = ""
			} else {
				remainder = secondLine + "\n" + remainder
			}
			title := strings.TrimSpace(header)
			targetFilename := filepath.Join(targetDir, dir, filename)
			util.WithWriteFile(targetFilename, func(w io.Writer) error {
				context := Context{
					Title:  title,
					Body:   remainder,
					Weight: weight,
				}
				return tmpl.Execute(w, context)
			})
		}
		return nil
	})
	if err != nil {
		log.Fatal(err)
	}
}
