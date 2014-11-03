package main

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
#include "sk_surface.h"
*/
import "C"

import (
	"fmt"
)

func main() {
	p := C.sk_paint_new()
	defer C.sk_paint_delete(p)
	fmt.Println("OK!")
}

// TODO: replace this with an idiomatic interface to Skia.
