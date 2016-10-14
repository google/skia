/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
package skia

// First, build Skia this way:
//   ./gyp_skia -Dskia_shared_lib=1 && ninja -C out/Debug

/*
#cgo LDFLAGS: -lGL
#cgo LDFLAGS: -lGLU
#cgo LDFLAGS: -lX11
#cgo LDFLAGS: -ldl
#cgo LDFLAGS: -lfontconfig
#cgo LDFLAGS: -lfreetype
#cgo LDFLAGS: -lgif
#cgo LDFLAGS: -lm
#cgo LDFLAGS: -lpng
#cgo LDFLAGS: -lstdc++
#cgo LDFLAGS: -lz

#cgo LDFLAGS: -L ../../out/Debug/lib
#cgo LDFLAGS: -Wl,-rpath=../../out/Debug/lib
#cgo LDFLAGS: -lskia

#cgo CFLAGS: -I../../include/c
#include "sk_canvas.h"
#include "sk_image.h"
#include "sk_paint.h"
#include "sk_path.h"
#include "sk_surface.h"
*/
import "C"

import (
	"image"
	"runtime"
	"unsafe"
)

// ////////////////////////////////////////////
// Color types
// ////////////////////////////////////////////
type ColorType C.sk_colortype_t

var (
	UNKNOWN_COLORTYPE   ColorType = C.UNKNOWN_SK_COLORTYPE
	RGBA_8888_COLORTYPE ColorType = C.RGBA_8888_SK_COLORTYPE
	BGRA_8888_COLORTYPE ColorType = C.BGRA_8888_SK_COLORTYPE
	ALPHA_8_COLORTYPE   ColorType = C.ALPHA_8_SK_COLORTYPE
)

// ////////////////////////////////////////////
// Alpha types
// ////////////////////////////////////////////
type AlphaType C.sk_alphatype_t

var (
	OPAQUE_ALPHATYPE   AlphaType = C.OPAQUE_SK_ALPHATYPE
	PREMUL_ALPHATYPE   AlphaType = C.PREMUL_SK_ALPHATYPE
	UNPREMUL_ALPHATYPE AlphaType = C.UNPREMUL_SK_ALPHATYPE
)

// ////////////////////////////////////////////
// ImageInfo
// ////////////////////////////////////////////
type ImageInfo struct {
	skVal C.sk_imageinfo_t
}

func NewImageInfo(width, height int) *ImageInfo {
	result := &ImageInfo{}
	result.skVal.width = C.int32_t(width)
	result.skVal.height = C.int32_t(height)
	return result
}

// TODO: figure out colortype as int and whether
// we can expose the underlying struct directly.
// Add getter functions.
func (ii *ImageInfo) SetColorType(colorType ColorType) {
	ii.skVal.colorType = C.sk_colortype_t(colorType)
}

func (ii *ImageInfo) SetAlphaType(alphaType AlphaType) {
	ii.skVal.alphaType = C.sk_alphatype_t(alphaType)
}

// ////////////////////////////////////////////
// Canvas
// ////////////////////////////////////////////
type Canvas struct {
	skCanvasRef *C.sk_canvas_t
}

func newCanvasFromRef(ref *C.sk_canvas_t) *Canvas {
	ret := &Canvas{
		skCanvasRef: ref,
	}

	// No finalizer here, because canvases are deallocated by the surface.
	return ret
}

func (c *Canvas) DrawOval(rect *Rect, paint *Paint) *Canvas {
	C.sk_canvas_draw_oval(c.skCanvasRef, rect.skRectPtr(), paint.skPaintRef)
	return c
}

func (c *Canvas) DrawRect(rect *Rect, paint *Paint) *Canvas {
	C.sk_canvas_draw_rect(c.skCanvasRef, rect.skRectPtr(), paint.skPaintRef)
	return c
}

func (c *Canvas) DrawPath(path *Path, paint *Paint) *Canvas {
	C.sk_canvas_draw_path(c.skCanvasRef, path.skPathRef, paint.skPaintRef)
	return c
}

// ////////////////////////////////////////////
// Paint
// ////////////////////////////////////////////

type Paint struct {
	skPaintRef *C.sk_paint_t
}

func NewPaint(color int, antiAlias bool) *Paint {
	ret := &Paint{
		skPaintRef: C.sk_paint_new(),
	}

	// Wire up the finalize to release the pointer.
	runtime.SetFinalizer(ret, func(p *Paint) {
		C.sk_paint_delete(p.skPaintRef)
	})

	return ret
}

func (p *Paint) SetColor(color uint32) {
	C.sk_paint_set_color(p.skPaintRef, C.sk_color_t(color))
}

func (p *Paint) SetAntiAlias(antiAlias bool) {
	C.sk_paint_set_antialias(p.skPaintRef, C._Bool(antiAlias))
}

// ////////////////////////////////////////////
// Rect
// ////////////////////////////////////////////
type Rect struct {
	Left   float32
	Top    float32
	Right  float32
	Bottom float32
}

func NewRect(left, top, right, bottom float32) *Rect {
	return &Rect{
		Left:   left,
		Top:    top,
		Right:  right,
		Bottom: bottom,
	}
}

// Convert the Rect into a pointer to sk_rect_t. That means that the fields
// have to exactly align with the fields in sk_rect_t.
func (r *Rect) skRectPtr() *C.sk_rect_t {
	return (*C.sk_rect_t)(unsafe.Pointer(r))
}

// ////////////////////////////////////////////
// Path
// ////////////////////////////////////////////
type Path struct {
	skPathRef *C.sk_path_t
}

func NewPath() *Path {
	ret := &Path{
		skPathRef: C.sk_path_new(),
	}

	// Wire up the finalize to release the pointer.
	runtime.SetFinalizer(ret, func(p *Path) {
		C.sk_path_delete(p.skPathRef)
	})

	return ret
}

func (p *Path) MoveTo(x, y float32) {
	C.sk_path_move_to(p.skPathRef, C.float(x), C.float(y))
}

func (p *Path) LineTo(x, y float32) {
	C.sk_path_line_to(p.skPathRef, C.float(x), C.float(y))
}

func (p *Path) Close() {
	C.sk_path_close(p.skPathRef)
}

// ////////////////////////////////////////////
// Image
// ////////////////////////////////////////////
type Image struct {
	skImageRef *C.sk_image_t
}

func newImageFromRef(ref *C.sk_image_t) *Image {
	ret := &Image{
		skImageRef: ref,
	}

	runtime.SetFinalizer(ret, func(i *Image) {
		C.sk_image_unref(i.skImageRef)
	})

	return ret
}

// TODO (stephana): Add a function that returns a standard go image.
func (i *Image) NRGBA() *image.NRGBA {
	return nil
}

// ////////////////////////////////////////////
// Surface
// ////////////////////////////////////////////
type Surface struct {
	// Pointer to sk_surface_t
	skSurfaceRef *C.sk_surface_t
}

func NewRasterSurface(imgInfo *ImageInfo) *Surface {
	ret := &Surface{
		skSurfaceRef: C.sk_surface_new_raster(&imgInfo.skVal),
	}

	// Wire up the finalize to release the pointer.
	runtime.SetFinalizer(ret, func(s *Surface) {
		C.sk_surface_unref(s.skSurfaceRef)
	})

	return ret
}

func (s *Surface) Canvas() *Canvas {
	return newCanvasFromRef(C.sk_surface_get_canvas(s.skSurfaceRef))
}

func (s *Surface) ImageSnapshot() *Image {
	return newImageFromRef(C.sk_surface_new_image_snapshot(s.skSurfaceRef))
}
