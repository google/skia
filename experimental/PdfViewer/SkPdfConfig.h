/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPdfConfig_DEFINED
#define SkPdfConfig_DEFINED

#define PDF_TRACK_OBJECT_USAGE
//#define PDF_TRACK_STREAM_OFFSETS
//#define PDF_TRACE
//#define PDF_TRACE_READ_TOKEN
//#define PDF_TRACE_DRAWTEXT
//#define PDF_TRACE_DIFF_IN_PNG
//#define PDF_DEBUG_NO_CLIPING
//#define PDF_DEBUG_NO_PAGE_CLIPING
//#define PDF_DEBUG_3X


// TODO(edisonn): pass a flag to say how it was used? e.g. asked the type? Obtained value?
// Implement it when it will be needed the first time to fix some bug.
#ifdef PDF_TRACK_OBJECT_USAGE
#define SkPdfMarkObjectUsed() fUsed = true
#else
#define SkPdfMarkObjectUsed()
#endif   // PDF_TRACK_OBJECT_USAGE

#ifdef PDF_TRACK_OBJECT_USAGE
#define SkPdfMarkObjectUnused() fUsed = false
#else
#define SkPdfMarkObjectUnused()
#endif   // PDF_TRACK_OBJECT_USAGE

#endif  // SkPdfConfig_DEFINED
