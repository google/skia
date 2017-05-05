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
	"log"
	"os"

	skia "skia.googlesource.com/skia/experimental/go-skia"
)

func main() {
	imgInfo := &skia.ImageInfo{
		Width:     640,
		Height:    480,
		ColorType: skia.GetDefaultColortype(),
		AlphaType: skia.PREMUL_ALPHATYPE,
	}

	surface, err := skia.NewRasterSurface(imgInfo)
	if err != nil {
		log.Fatalln(err)
	}

	canvas := surface.Canvas()

	fillPaint := skia.NewPaint()
	fillPaint.SetColor(0xFF0000FF)
	canvas.DrawPaint(fillPaint)
	fillPaint.SetColor(0xFF00FFFF)

	rect := skia.NewRect(100, 100, 540, 380)
	canvas.DrawRect(rect, fillPaint)

	strokePaint := skia.NewPaint()
	strokePaint.SetColor(0xFFFF0000)
	strokePaint.SetAntiAlias(true)
	strokePaint.SetStroke(true)
	strokePaint.SetStrokeWidth(5.0)

	path := skia.NewPath()
	path.MoveTo(50, 50)
	path.LineTo(590, 50)
	path.CubicTo(-490, 50, 1130, 430, 50, 430)
	path.LineTo(590, 430)
	canvas.DrawPath(path, strokePaint)

	fillPaint.SetColor(0x8000FF00)
	canvas.DrawOval(skia.NewRect(120, 120, 520, 360), fillPaint)

	// rect.Left += 25
	// rect.Top += 25
	// rect.Right -= 25
	// rect.Bottom -= 25

	// paint.SetColor(0xFF00FF00)
	// canvas.DrawRect(rect, paint)

	// path := skia.NewPath()
	// path.MoveTo(50, 50)
	// path.LineTo(100, 100)
	// path.LineTo(50, 100)
	// path.Close()
	// canvas.DrawPath(path, paint)

	// // Get a skia image from the surface.
	skImg := surface.Image()

	// // Get it as a native Go image and save it as a PNG.
	// img := skImg.NRGBA()

	// Write new image to file if we have one.
	if skImg != nil {
		out, err := os.Create("testimage.png")
		if err != nil {
			log.Fatal(err)
		}
		defer out.Close()

		if err := skImg.WritePNG(out); err != nil {
			log.Fatalf("Unable to write png: %s", err)
		}
	}
}
