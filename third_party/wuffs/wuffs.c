/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// The Wuffs library ships as a single file - a .h file - which GN does not
// recognize as something to be compiled, because it ends in .h and not .c or
// .cpp. Instead, this trivial file is a placeholder .c file that is a BUILD.gn
// target for the third party Wuffs library.
//
// Copy/pasting from the Wuffs .h file's comments:
//
// ----
//
// Wuffs ships as a "single file C library" or "header file library" as per
// https://github.com/nothings/stb/blob/master/docs/stb_howto.txt
//
// To use that single file as a "foo.c"-like implementation, instead of a
// "foo.h"-like header, #define WUFFS_IMPLEMENTATION before #include'ing or
// compiling it.
//
// ----
#define WUFFS_IMPLEMENTATION

// Defining the WUFFS_CONFIG__MODULE* macros are optional, but it lets users of
// Wuffs' .h file whitelist which parts of Wuffs to build. That file contains
// the entire Wuffs standard library, implementing a variety of codecs and file
// formats. Without this macro definition, an optimizing compiler or linker may
// very well discard Wuffs code for unused codecs, but listing the Wuffs
// modules we use makes that process explicit. Preprocessing means that such
// code simply isn't compiled.
//
// For Skia, we're only interested in particular image codes (e.g. GIF) and
// their dependencies (e.g. BASE, LZW).
#define WUFFS_CONFIG__MODULES
#define WUFFS_CONFIG__MODULE__BASE
#define WUFFS_CONFIG__MODULE__GIF
#define WUFFS_CONFIG__MODULE__LZW

#include "wuffs-v0.2.h"
