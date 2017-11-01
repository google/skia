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
	"sync"
)

const (
	min_png = "min.png"
	max_png = "max.png"
)

type ParamSet struct {
	Config      []string `json:"config"`
	SourceType  []string `json:"source_type"`
	ExtraConfig []string `json:"extra_config"`
}

type SRDigest struct {
	Status string   `json:"status"`
	Params ParamSet `json:"paramset"`
	URL    string   `json:"URL"`
}

type TestRecord struct {
	TestName string     `json:"testName"`
	Digests  []SRDigest `json:"digests"`
}

func in(v string, a []string) bool {
	for _, u := range a {
		if u == v {
			return true
		}
	}
	return false
}

// TODO(halcanary): clean up this blacklist.
var blacklist = [...]string{
	"circular-clips",
	"colorcomposefilter_wacky",
	"coloremoji_blendmodes",
	"colormatrix",
	"complexclip_bw",
	"complexclip_bw_invert",
	"complexclip_bw_layer",
	"complexclip_bw_layer_invert",
	"convex-lineonly-paths-stroke-and-fill",
	"dftext",
	"downsamplebitmap_image_high_mandrill_512.png",
	"downsamplebitmap_image_medium_mandrill_512.png",
	"filterbitmap_image_mandrill_16.png",
	"filterbitmap_image_mandrill_64.png",
	"filterbitmap_image_mandrill_64.png_g8",
	"gradients_degenerate_2pt",
	"gradients_degenerate_2pt_nodither",
	"gradients_local_perspective",
	"gradients_local_perspective_nodither",
	"imagefilterstransformed",
	"image_scale_aligned",
	"lattice",
	"linear_gradient",
	"mipmap_srgb",
	"mixedtextblobs",
	"OverStroke",
	"simple-offsetimagefilter",
	"strokerect",
	"textblobmixedsizes",
	"textblobmixedsizes_df"}

func process_test(testName string, imgUrls []string, output string) error {
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
			img_max = to_nrgba(img)
			img_min = copy_nrgba(img_max)
			continue
		}
		w := img.Bounds().Max.X - img.Bounds().Min.X
		h := img.Bounds().Max.Y - img.Bounds().Min.Y
		if img_max.Rect.Max.X != w || img_max.Rect.Max.Y != h {
			return errors.New("size mismatch")
		}
		img_nrgba := to_nrgba(img)
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
	err := os.Mkdir(output_directory, os.ModePerm)
	if err != nil && !os.IsExist(err) {
		return err
	}
	err = write_png_to_file(path.Join(output_directory, min_png), &img_min)
	if err != nil {
		return err
	}
	err = write_png_to_file(path.Join(output_directory, max_png), &img_max)
	if err != nil {
		return err
	}
	return nil

}
func readMetaJsonFile(filename string) ([]TestRecord, error) {
	file, err := os.Open(filename)
	if err != nil {
		return nil, err
	}
	dec := json.NewDecoder(file)
	var records []TestRecord
	err = dec.Decode(&records)
	return records, err
}

func write_png_to_file(path string, img image.Image) error {
	file, err := os.Create(path)
	if err != nil {
		return err
	}
	err = png.Encode(file, img)
	file.Close()
	if err != nil {
		return err
	}
	return nil
}

// to_nrgb() may return a shallow copy of img if it's already NRGBA.
func to_nrgba(img image.Image) image.NRGBA {
	switch v := img.(type) {
	case *image.NRGBA:
		return *v
	}
	nimg := *image.NewNRGBA(img.Bounds())
	draw.Draw(&nimg, img.Bounds(), img, image.Point{0, 0}, draw.Src)
	return nimg
}

func copy_nrgba(src image.NRGBA) image.NRGBA {
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
	err := os.Mkdir(output, os.ModePerm)
	if err != nil && !os.IsExist(err) {
		log.Fatal(err)
	}

	records, err := readMetaJsonFile(input)
	if err != nil {
		log.Fatal(err)
	}

	var wg sync.WaitGroup
	for _, record := range records {
		if in(record.TestName, blacklist[:]) {
			continue
		}
		var goodUrls []string
		for _, digest := range record.Digests {
			if (in("vk", digest.Params.Config) ||
				in("gles", digest.Params.Config)) &&
				digest.Status == "positive" {
				goodUrls = append(goodUrls, digest.URL)
			}
		}
		wg.Add(1)
		go func(testName string, imgUrls []string, output string) {
			defer wg.Done()
			err := process_test(testName, imgUrls, output)
			if err != nil {
				log.Fatal(err)
			}
			fmt.Printf("\r%-60s", testName)
		}(record.TestName, goodUrls, output)
	}
	wg.Wait()
	fmt.Printf("\r%60s\n", "")
}
