/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
package main

import (
	"encoding/json"
	"errors"
	"fmt"
	"image"
	"image/draw"
	"image/png"
	"log"
	"net/http"
	"os"
	"path"
	"sort"
	"strings"
	"sync"

	"go.skia.org/infra/golden/go/search"
)

const (
	min_png = "min.png"
	max_png = "max.png"
)

type ExportTestRecordArray []search.ExportTestRecord

func (a ExportTestRecordArray) Len() int           { return len(a) }
func (a ExportTestRecordArray) Swap(i, j int)      { a[i], a[j] = a[j], a[i] }
func (a ExportTestRecordArray) Less(i, j int) bool { return a[i].TestName < a[j].TestName }

func in(v string, a []string) bool {
	for _, u := range a {
		if u == v {
			return true
		}
	}
	return false
}

func clampU8(v int) uint8 {
	if v < 0 {
		return 0
	} else if v > 255 {
		return 255
	}
	return uint8(v)
}

func processTest(testName string, imgUrls []string, output string) error {
	if strings.ContainsRune(testName, '/') {
		return nil
	}
	output_directory := path.Join(output, testName)
	var img_max image.NRGBA
	var img_min image.NRGBA
	for _, url := range imgUrls {
		resp, err := http.Get(url)
		if err != nil {
			return err
		}
		img, err := png.Decode(resp.Body)
		resp.Body.Close()
		if err != nil {
			return err
		}
		if img_max.Rect.Max.X == 0 {
			// N.B. img_max.Pix may alias img.Pix (if they're already NRGBA).
			img_max = toNrgba(img)
			img_min = copyNrgba(img_max)
			continue
		}
		w := img.Bounds().Max.X - img.Bounds().Min.X
		h := img.Bounds().Max.Y - img.Bounds().Min.Y
		if img_max.Rect.Max.X != w || img_max.Rect.Max.Y != h {
			return errors.New("size mismatch")
		}
		img_nrgba := toNrgba(img)
		for i, value := range img_nrgba.Pix {
			if value > img_max.Pix[i] {
				img_max.Pix[i] = value
			} else if value < img_min.Pix[i] {
				img_min.Pix[i] = value
			}
		}
	}
	if img_max.Rect.Max.X == 0 {
		return nil
	}

	if err := os.Mkdir(output_directory, os.ModePerm); err != nil && !os.IsExist(err) {
		return err
	}
	if err := writePngToFile(path.Join(output_directory, min_png), &img_min); err != nil {
		return err
	}
	if err := writePngToFile(path.Join(output_directory, max_png), &img_max); err != nil {
		return err
	}
	return nil

}

func readMetaJsonFile(filename string) ([]search.ExportTestRecord, error) {
	file, err := os.Open(filename)
	if err != nil {
		return nil, err
	}
	dec := json.NewDecoder(file)
	var records []search.ExportTestRecord
	err = dec.Decode(&records)
	return records, err
}

func writePngToFile(path string, img image.Image) error {
	file, err := os.Create(path)
	if err != nil {
		return err
	}
	defer file.Close()
	return png.Encode(file, img)
}

// to_nrgb() may return a shallow copy of img if it's already NRGBA.
func toNrgba(img image.Image) image.NRGBA {
	switch v := img.(type) {
	case *image.NRGBA:
		return *v
	}
	nimg := *image.NewNRGBA(img.Bounds())
	draw.Draw(&nimg, img.Bounds(), img, image.Point{0, 0}, draw.Src)
	return nimg
}

func copyNrgba(src image.NRGBA) image.NRGBA {
	dst := image.NRGBA{make([]uint8, len(src.Pix)), src.Stride, src.Rect}
	copy(dst.Pix, src.Pix)
	return dst
}

func main() {
	if len(os.Args) != 3 {
		log.Printf("Usage:\n  %s INPUT.json OUTPUT_DIRECTORY\n\n", os.Args[0])
		os.Exit(1)
	}
	input := os.Args[1]
	output := os.Args[2]
	// output is removed and replaced with a clean directory.
	if err := os.RemoveAll(output); err != nil && !os.IsNotExist(err) {
		log.Fatal(err)
	}
	if err := os.MkdirAll(output, os.ModePerm); err != nil && !os.IsExist(err) {
		log.Fatal(err)
	}

	records, err := readMetaJsonFile(input)
	if err != nil {
		log.Fatal(err)
	}
	sort.Sort(ExportTestRecordArray(records))

	var wg sync.WaitGroup
	for _, record := range records {
		var goodUrls []string
		for _, digest := range record.Digests {
			if (in("vk", digest.ParamSet["config"]) ||
				in("gles", digest.ParamSet["config"])) &&
				digest.Status == "positive" {
				goodUrls = append(goodUrls, digest.URL)
			}
		}
		wg.Add(1)
		go func(testName string, imgUrls []string, output string) {
			defer wg.Done()
			if err := processTest(testName, imgUrls, output); err != nil {
				log.Fatal(err)
			}
			fmt.Printf("\r%-60s", testName)
		}(record.TestName, goodUrls, output)
	}
	wg.Wait()
	fmt.Printf("\r%60s\n", "")
}
