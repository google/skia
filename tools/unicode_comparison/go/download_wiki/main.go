// Copyright 2023 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// This executable downloads some amount of wikipedia pages by given locale(s),
// breaks them into smaller parts by sections, then by sentences and
// writes them down into separate files

package main

import (
	"flag"
	"fmt"
	gowiki "github.com/trietmn/go-wiki"
	"go.skia.org/skia/tools/unicode_comparison/go/bridge"
	"go.skia.org/skia/tools/unicode_comparison/go/helpers"
	"os"
	"path/filepath"
	"strconv"
	"strings"
)

func downloadLocalPagesBySections(searchResult []string, localInput string, lastCount, fileLimit, textLimit int) int {
	countSentences := 0
	for _ /*index*/, element := range searchResult {
		// Get the page
		page, err := gowiki.GetPage(element, -1, false, true)
		if err != nil {
			fmt.Println(err)
			continue
		}

		sections, err := page.GetSectionList()
		if err != nil {
			fmt.Println(err)
			continue
		}
		for si, section := range sections {
			if si == len(sections)-1 {
				// It looks like this library breaks on the last section
				break
			}
			if len(section) == 0 {
				fmt.Println("Empty section!")
				continue
			}

			content, err := page.GetSection(section)
			if err != nil {
				fmt.Println(err)
				continue
			}

			trimmed := strings.TrimSpace(content)
			if len(trimmed) == 0 {
				continue
			}

			// We generate broked by sentences texts from the same section
			sentences := bridge.GetSentences(trimmed)
			start := 0
			for _ /*i*/, end := range sentences {
				smallFileName := localInput + "/page." + strconv.Itoa(lastCount+countSentences+1) // + "." + strconv.Itoa(index+1) + "." + strconv.Itoa(si+1) + "." + strconv.Itoa(i+1)
				smallText := strings.TrimSpace(trimmed[start:end])
				if len(smallText) == 0 {
					continue
				} else if len(smallText) > textLimit {
					trim := 0
					if bridge.TrimSentence(smallText, &trim, textLimit) {
						smallText = smallText[:trim]
					}
				}
				helpers.WriteTextFile(smallFileName, smallText)
				start = int(end)
				countSentences += 1
				if lastCount+countSentences >= fileLimit {
					return countSentences
				}
			}
		}
	}
	return countSentences
}

func main() {
	var (
		root      = flag.String("root", "~/datasets", "Folder (pages will be downloaded under <Folder>/input")
		locale    = flag.String("locale", "*", "Locale")
		pattern   = flag.String("pattern", "*", "Pattern for search")
		fileLimit = flag.Int("fileLimit", 10, "Number of text files to download")
		pageLimit = flag.Int("pageLimit", 5, "Number of pages to download in one attempt")
		textLimit = flag.Int("textLimit", 1000, "Max length of a single text")
		verbose   = flag.Bool("verbose", true, "Print more details about the process")
	)
	flag.Parse()
	if *root == "" {
		fmt.Println("Must set --root")
		flag.PrintDefaults()
	}

	if !bridge.InitUnicode("icu") {
		return
	}

	*root = helpers.ExpandPath(*root)
	input := filepath.Join(*root, "input")

	if *verbose {
		fmt.Printf("Downloading wiki pages:")
		fmt.Printf("root=%v\n", *root)
		fmt.Printf("locale=%v\n", *locale)
		fmt.Printf("pattern=%v\n", *pattern)
		fmt.Printf("fileLimit=%v\n", *fileLimit)
		fmt.Printf("pageLimit=%v\n", *pageLimit)
		fmt.Printf("textLimit=%v\n", *textLimit)
	}

	locales := []string{}
	if *locale != "*" {
		locales = strings.Split(*locale, ",")
	} else {
		// Sorted down by number of wiki pages
		locales = []string{"en", "ru", "it", "de", "ro", "uk", "fa", "he", "fi", "fr", "zh", "ar", "id", "tr", "th", "vi", "lv", "lt", "hr", "az", "el", "ms", "bn", "te", "ur"}
		// "ka", "pt" do not get downloaded properly
	}

	for _, loc := range locales {
		localInput := filepath.Join(input, loc)

		err := os.MkdirAll(localInput, os.ModePerm)
		helpers.Check(err)

		gowiki.SetLanguage(loc)
		fileCount := 0
		attempt := *fileLimit * 10
		for fileCount < *fileLimit && attempt > 0 {
			files := 0
			if *pattern == "*" {
				searchResult, err := gowiki.GetRandom(*pageLimit)
				if err != nil {
					attempt -= 1
					fmt.Printf("Cannot download %d random pages for locale %s:\n%s\n", *pageLimit, loc, err)
					continue
				}
				files = downloadLocalPagesBySections(searchResult, localInput, fileCount, *fileLimit, *textLimit)
			} else {
				searchResult, _, err := gowiki.Search(*pattern, *pageLimit, true)
				helpers.Check(err)
				files = downloadLocalPagesBySections(searchResult, localInput, fileCount, *fileLimit, *textLimit)
			}
			if files == 0 {
				attempt -= 1
			} else {
				fileCount += files
			}
		}
		if *verbose {
			if fileCount >= *fileLimit {
				fmt.Printf("Locale %s (%v files)\n", loc, fileCount)
			} else if fileCount == 0 {
				fmt.Printf("Locale %s does not containt text on %v attempts to download\n", loc, *fileLimit)
			} else {
				fmt.Printf("Locale %s containt less texts than %v on %v attempts to download\n", loc, *fileLimit, *fileLimit)
			}
		}
	}

	bridge.CleanupUnicode()
}
