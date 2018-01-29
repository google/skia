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

// `slack` is an amount to add to the max and subtract from the minimim channel
// value for each pixel for the given test.  By default, slack is 0.
var slack = map[string]int{
	"addarc":                                       1,
	"animated-image-blurs":                         1,
	"anisotropic_hq":                               1,
	"bitmapfilters":                                1,
	"bleed":                                        1,
	"bleed_alpha_bmp":                              1,
	"bleed_alpha_bmp_shader":                       1,
	"bleed_alpha_image":                            1,
	"bleed_alpha_image_shader":                     1,
	"bleed_image":                                  1,
	"blur2rectsnonninepatch":                       1,
	"blurcircles2":                                 1,
	"blurimagevmask":                               1,
	"blurs":                                        1,
	"bmp_filter_quality_repeat":                    1,
	"circular_arcs_stroke_butt":                    1,
	"circular_arcs_stroke_square":                  1,
	"cliperror":                                    1,
	"colormatrix":                                  1,
	"complexclip_aa_invert":                        1,
	"complexclip_aa_layer_invert":                  1,
	"concavepaths":                                 1,
	"const_color_processor":                        1,
	"convex-polygon-inset":                         1,
	"dashcircle":                                   1,
	"dash_line_zero_off_interval":                  1,
	"downsamplebitmap_image_high":                  1,
	"downsamplebitmap_image_low":                   1,
	"downsamplebitmap_image_medium":                1,
	"downsamplebitmap_text_high_72.00pt":           1,
	"downsamplebitmap_text_low_72.00pt":            1,
	"downsamplebitmap_text_medium_72.00pt":         1,
	"drawregionmodes":                              1,
	"dstreadshuffle":                               1,
	"extractalpha":                                 1,
	"fillcircle":                                   1,
	"filterbitmap_checkerboard_192_192":            1,
	"filterbitmap_checkerboard_32_2":               1,
	"filterbitmap_checkerboard_32_32":              1,
	"filterbitmap_checkerboard_32_32_g8":           1,
	"filterbitmap_checkerboard_32_8":               1,
	"filterbitmap_checkerboard_4_4":                1,
	"filterbitmap_image_color_wheel.png":           1,
	"filterbitmap_image_mandrill_16.png":           1,
	"filterbitmap_image_mandrill_32.png":           1,
	"filterbitmap_image_mandrill_64.png":           1,
	"filterbitmap_image_mandrill_64.png_g8":        1,
	"filterbitmap_text_10.00pt":                    1,
	"filterbitmap_text_3.00pt":                     1,
	"filterbitmap_text_7.00pt":                     1,
	"fontmgr_bounds":                               1,
	"fontmgr_bounds_1_-0.25":                       1,
	"glyph_pos_h_s":                                1,
	"gradients_2pt_conical_edge":                   1,
	"gradients_many":                               1,
	"gradients_no_texture":                         1,
	"gradients_no_texture_nodither":                1,
	"imageblur":                                    1,
	"imageblurclampmode":                           1,
	"imageblurrepeatmode":                          1,
	"imagefiltersbase":                             1,
	"imagefiltersclipped":                          1,
	"imagefilterscropexpand":                       1,
	"imagefilterscropped":                          1,
	"imagefiltersscaled":                           1,
	"imagefiltersstroked":                          1,
	"linear_gradient":                              1,
	"manycircles":                                  1,
	"nested_aa":                                    1,
	"nested_flipY_aa":                              1,
	"ninepatch-stretch":                            1,
	"nonclosedpaths":                               1,
	"parsedpaths":                                  1,
	"pathfill":                                     1,
	"polygons":                                     1,
	"rects":                                        1,
	"savelayer_with_backdrop":                      1,
	"scaled_tilemodes_npot":                        1,
	"shadertext2":                                  1,
	"shadertext3":                                  1,
	"shadow_utils_occl":                            1,
	"smallpaths":                                   1,
	"spritebitmap":                                 1,
	"strokecircle":                                 1,
	"stroke-fill":                                  1,
	"strokerects":                                  1,
	"strokes3":                                     1,
	"strokes_round":                                1,
	"tall_stretched_bitmaps":                       1,
	"thinconcavepaths":                             1,
	"tilemodes_npot":                               1,
	"varied_text_clipped_no_lcd":                   1,
	"varied_text_ignorable_clip_no_lcd":            1,
	"xfermodes":                                    1,
	"xfermodes2":                                   1,
	"xfermodes3":                                   1,
	"yuv_to_rgb_effect":                            1,
	"zeroPath":                                     1,
}

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
	if delta, ok := slack[testName]; ok {
		for i, v := range img_min.Pix {
			img_min.Pix[i] = clampU8(int(v) - delta)
		}
		for i, v := range img_max.Pix {
			img_max.Pix[i] = clampU8(int(v) + delta)
		}
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
