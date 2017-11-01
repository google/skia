/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
package main

import (
	"errors"
	"image"
	"image/draw"
	"image/png"
	"io/ioutil"
	"log"
	"os"
	"path"
	"sync"
)

const (
	dot_png = ".png"
	min_png = "min.png"
	max_png = "max.png"
)

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

func process_directory(input_directory string, output_directory string) error {
	var img_max image.NRGBA
	var img_min image.NRGBA
	files, err := ioutil.ReadDir(input_directory)
	if err != nil {
		return err
	}
	for _, file := range files {
		file, err := os.Open(path.Join(input_directory, file.Name()))
		if err != nil {
			return err
		}
		img, err := png.Decode(file)
		file.Close()
		if err != nil {
			continue // Not an error
		}
		if img_max.Rect.Max.X == 0 {
			img_max = to_nrgba(img)
			img_min = copy_nrgba(img_max)
			continue
		}
		w := img.Bounds().Max.X - img.Bounds().Min.X
		h := img.Bounds().Max.Y - img.Bounds().Min.Y
		if img_max.Rect.Max.X != w || img_max.Rect.Max.Y != h {
			return errors.New("size mismatch")
		}
		img_copy := to_nrgba(img)
		for i, value := range img_copy.Pix {
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
	err = os.Mkdir(output_directory, os.ModePerm)
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

func main() {
	if len(os.Args) != 3 {
		os.Exit(1)
	}
	input := os.Args[1]
	output := os.Args[2]
	err := os.Mkdir(output, os.ModePerm)
	if err != nil && !os.IsExist(err) {
		log.Fatal(err)
	}
	files, err := ioutil.ReadDir(input)
	if err != nil {
		log.Fatal(err)
	}

	var wg sync.WaitGroup
	for _, file := range files {
		wg.Add(1)
		go func() {
			defer wg.Done()
			x := path.Join(input, file.Name())
			y := path.Join(output, file.Name())
			err := process_directory(x, y)
			if err != nil {
				log.Fatal(err)
			}

		}()
	}
	wg.Wait()
}
