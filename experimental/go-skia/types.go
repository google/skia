/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// Created by cgo -godefs. Enum fields in structs were fixed by hand.
// command: go tool cgo -godefs ctypes.go > types.go
//
// The purpose of this file is to have Go structs with the same memory
// layout as their C counterparts. For enums we want the underlying primitive
// types to match.
//
// TODO(stephan): Add tests that allow to detect failure on platforms other
// than Linux and changes in the underlying C types.

package skia

type Color uint32

type ColorType uint32

const (
	UNKNOWN_COLORTYPE   ColorType = 0x0
	RGBA_8888_COLORTYPE ColorType = 0x1
	BGRA_8888_COLORTYPE ColorType = 0x2
	ALPHA_8_COLORTYPE   ColorType = 0x3
)

type AlphaType uint32

const (
	OPAQUE_ALPHATYPE   AlphaType = 0x0
	PREMUL_ALPHATYPE   AlphaType = 0x1
	UNPREMUL_ALPHATYPE AlphaType = 0x2
)

type PixelGeometry uint32

const (
	UNKNOWN_SK_PIXELGEOMETRY PixelGeometry = 0x0
	RGB_H_SK_PIXELGEOMETRY   PixelGeometry = 0x1
	BGR_H_SK_PIXELGEOMETRY   PixelGeometry = 0x2
	RGB_V_SK_PIXELGEOMETRY   PixelGeometry = 0x3
	BGR_V_SK_PIXELGEOMETRY   PixelGeometry = 0x4
)

type ImageInfo struct {
	Width     int32
	Height    int32
	ColorType ColorType
	AlphaType AlphaType
}

type SurfaceProps struct {
	PixelGeometry PixelGeometry
}

type Rect struct {
	Left   float32
	Top    float32
	Right  float32
	Bottom float32
}
