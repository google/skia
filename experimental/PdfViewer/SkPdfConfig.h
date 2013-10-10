/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPdfConfig_DEFINED
#define SkPdfConfig_DEFINED

#include "stddef.h"
class SkPdfNativeObject;

// shows what objects have not been used in rendering. can be used to track what features we might
// have not implemented, or where we implemented only the default behaivour
//#define PDF_TRACK_OBJECT_USAGE

// tracks the position in the stream, it can be used to show where exactly the errors happened
//#define PDF_TRACK_STREAM_OFFSETS

// reports issues, warning, NYI, errors, ...
// enable PDF_TRACK_STREAM_OFFSETS to also have the offset in the stream where the error happened
//#define PDF_REPORT

// At various points in code we show the value of important variables with this flag
//#define PDF_TRACE

// displays the result of each read token, individual result
//#define PDF_TRACE_READ_TOKEN

// Every drawtext draws before a rectangle, in this way we see the one that might have failed
//#define PDF_TRACE_DRAWTEXT

// For each render operations, it will dump the canvas in a png
//#define PDF_TRACE_DIFF_IN_PNG

// Does not clip at all, can be used in debugging issues
//#define PDF_DEBUG_NO_CLIPING

// Does not click the page, use is with 3x
//#define PDF_DEBUG_NO_PAGE_CLIPING

// render the page 3X bigger (with content in center) - used to make sure we don't mess up
// positioning
// like a tick tac toe board, only the center one has content, all the rest of them have to be clean
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

#ifdef PDF_TRACK_STREAM_OFFSETS
#define TRACK_OBJECT_SRC(a)
#define STORE_TRACK_PARAMETERS(obj) (obj)->fStreamId = streamId;\
                                    (obj)->fOffsetStart = offsetStart;\
                                    (obj)->fOffsetEnd = offsetEnd;
#define STORE_TRACK_PARAMETER_OFFSET_END(obj,offsetEnd) (obj)->fOffsetEnd = (offsetEnd)-streamStart;
#else
#define TRACK_OBJECT_SRC(a)
#define STORE_TRACK_PARAMETERS(obj)
#define STORE_TRACK_PARAMETER_OFFSET_END(obj,offsetEnd)
#endif   //PDF_TRACK_STREAM_OFFSETS

// TODO(edisonn): move it somewhere else?
struct SkPdfInputStream {
#ifdef PDF_TRACK_STREAM_OFFSETS
    // no parent object -> original file to be rendered
    // no parent file -> stream object
    // both -> external stream object
    int fParentFileID;
    const SkPdfNativeObject* fParentObject;

    size_t fDelta;  // delta in parent stream
    const unsigned char* fStart;
#endif  //  PDF_TRACK_STREAM_OFFSETS

    const unsigned char* fEnd;
};

struct SkPdfInputStreamLocation {
    SkPdfInputStream fInputStream;
    const unsigned char* fNow;
};

#ifdef PDF_TRACK_STREAM_OFFSETS
struct SkPdfInputStreamRange {
    SkPdfInputStream fInputStream;
    const unsigned char* fRangeStart;
    const unsigned char* fRangeEnd;
};
#endif  //  PDF_TRACK_STREAM_OFFSETS


#endif  // SkPdfConfig_DEFINED
