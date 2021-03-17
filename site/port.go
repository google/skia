// Ports over all the content in //site into //site2 adding page metadata, and
// possibly moving files to new locations.
package main

import (
	"fmt"
	"io"
	"io/ioutil"
	"log"
	"os"
	"path/filepath"
	"strings"

	"go.skia.org/infra/go/util"
)

const prefix = `
---
title: "%s"
linkTitle: "%s"
---
`

func main() {
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
		fmt.Printf("visited file: %q %q %q\n", path, ext, filename)
		if ext == "" {
			// Skip METADATA files.
			return nil
		}
		if filename == "index.md" {
			fmt.Printf("Re-write as _index.md.\n")
			// Re-write as _index.md.
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
			}
			title := strings.TrimSpace(header)
			newHeader := fmt.Sprintf(prefix, title, title)
			targetFilename := filepath.Join(targetDir, path)
			util.WithWriteFile(targetFilename, func(w io.Writer) error {
				_, err := w.Write([]byte(newHeader + remainder))
				return err
			})
		}
		return nil
	})
	if err != nil {
		log.Fatal(err)
	}
}
