/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
package main

/*
	This is a translation of the example code in src/c/sk_surface.cpp
	to idomatic Go.
*/

import (
	"image/png"
	"log"
	"os"

	"skia.googlesource.com/skia/experimental/go-skia"
)

func main() {
	imgInfo := skia.NewImageInfo(100, 100)
	imgInfo.SetColorType(skia.BGRA_8888_COLORTYPE)
	imgInfo.SetAlphaType(skia.PREMUL_ALPHATYPE)

	surface := skia.NewRasterSurface(imgInfo)
	canvas := surface.Canvas()

	paint := skia.NewPaint(0xFFFF0000, true)
	rect := skia.NewRect(5, 5, 95, 95)
	canvas.DrawOval(rect, paint)

	rect.Left += 25
	rect.Top += 25
	rect.Right -= 25
	rect.Bottom -= 25

	paint.SetColor(0xFF00FF00)
	canvas.DrawRect(rect, paint)

	path := skia.NewPath()
	path.MoveTo(50, 50)
	path.LineTo(100, 100)
	path.LineTo(50, 100)
	path.Close()
	canvas.DrawPath(path, paint)

	// // Get a skia image from the surface.
	skImg := surface.ImageSnapshot()

	// Get it as a native Go image and save it as a PNG.
	img := skImg.NRGBA()

	// Write new image to file if we have one.
	if img != nil {
		out, err := os.Create("testimage.png")
		if err != nil {
			log.Fatal(err)
		}
		defer out.Close()

		if err = png.Encode(out, img); err != nil {
			log.Fatal(err)
		}

	}
}
