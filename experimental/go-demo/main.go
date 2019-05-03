/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
package main

/*
	This is a translation of the example code in
	"experimental/c-api-example/skia-c-example.c" to test the
	go-skia package.
*/

import (
	"log"
	"os"

	skia "go.skia.org/skia/experimental/go-skia"
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

	// // Get a skia image from the surface.
	skImg := surface.Image()

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
