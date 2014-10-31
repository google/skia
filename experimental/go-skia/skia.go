package skia

/*
#cgo CFLAGS: -I../../include/c
#cgo LDFLAGS: -L ../../out/Debug/
#cgo LDFLAGS: -lskia_core
#cgo LDFLAGS: -lskia_effects
#cgo LDFLAGS: -lskia_images
#cgo LDFLAGS: -lskia_opts
#cgo LDFLAGS: -lskia_ports
#cgo LDFLAGS: -lskia_sfnt
#cgo LDFLAGS: -lskia_utils
#cgo LDFLAGS: -lskia_opts_ssse3
#cgo LDFLAGS: -lskia_opts_sse4
#cgo LDFLAGS: -lm
#cgo LDFLAGS: -lstdc++
#cgo LDFLAGS: -lGL
#cgo LDFLAGS: -lGLU
#include "sk_surface.h"
*/
import "C"

func dummyFunction() {
	testPaint := C.sk_paint_new()
	defer func() {
		sk_paint_delete(testPaint)
	}()
}

// TODO: replace this with an idiomatic interface to Skia.
