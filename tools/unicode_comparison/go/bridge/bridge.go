// Copyright 2023 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package bridge

// #include <stdio.h>
// #include <stdlib.h>
// #include "../../cpp/bridge.h"
// #cgo LDFLAGS: -L../../../../out/Release/ -lbridge
import "C"

import (
	"unsafe"
)

type CodeUnitFlags int

const (
	kNoCodeUnitFlag        CodeUnitFlags = 0x00
	kPartOfWhiteSpaceBreak               = 0x01
	kGraphemeStart                       = 0x02
	kSoftLineBreakBefore                 = 0x04
	kHardLineBreakBefore                 = 0x08
	kPartOfIntraWordBreak                = 0x10
	kControl                             = 0x20
	kTabulation                          = 0x40
	kGlyphClusterStart                   = 0x80
	kIdeographic                         = 0x100
	kEmoji                               = 0x200
	kWordBreak                           = 0x400
	kSentenceBreak                       = 0x800
)

type SkString struct {
	ptr unsafe.Pointer
}

type IntPtr struct {
	ptr unsafe.Pointer
}

func PerfComputeCodeunitFlags(text string) float64 {
	cs := C.CString(text)
	defer C.free(unsafe.Pointer(cs))
	return float64(C.perf_compute_codeunit_flags(cs))
}

func GetFlags(index int) int {
	return int(C.getFlags(C.int(index)))
}

func GetSentences(text string) []uint {
	cs := C.CString(text)
	defer C.free(unsafe.Pointer(cs))
	var length C.int
	var p = C.getSentences(cs, &length)
	var result = make([]uint, length)
	copy(result, unsafe.Slice((*uint)(unsafe.Pointer(p)), int(length)))
	return result
}

func TrimSentence(text string, length *int, limit int) bool {
	cs := C.CString(text)
	defer C.free(unsafe.Pointer(cs))
	var num C.int
	result := C.trimSentence(cs, &num, C.int(limit))
	*length = int(num)
	return bool(result)
}

func FlagsToString(flags int) string {
	result := ""
	if (flags & kGraphemeStart) != 0 {
		result += "G"
	}
	if (flags & kSoftLineBreakBefore) != 0 {
		result += "S"
	}
	if (flags & kHardLineBreakBefore) != 0 {
		result += "H"
	}
	if (flags & kPartOfWhiteSpaceBreak) != 0 {
		result += "W"
	}
	if (flags & kWordBreak) != 0 {
		result += "D"
	}
	if (flags & kControl) != 0 {
		result += "C"
	}
	return result
}

func ToUpper(str string) SkString {
	cs := C.CString(str)
	defer C.free(unsafe.Pointer(cs))
	return SkString{
		ptr: C.toUpper(cs),
	}
}

func InitUnicode(impl string) bool {
	cs := C.CString(impl)
	defer C.free(unsafe.Pointer(cs))
	result := C.init_skunicode_impl(cs)
	return bool(result)
}

func CleanupUnicode() {
	C.cleanup_unicode_impl()
}
