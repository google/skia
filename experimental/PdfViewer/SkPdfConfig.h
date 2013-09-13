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

//#define PDF_TRACK_OBJECT_USAGE
//#define PDF_TRACK_STREAM_OFFSETS
//#define PDF_REPORT
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

#ifdef PDF_TRACK_STREAM_OFFSETS

// TODO(edisonn): track source files
#define SkPdfTrackSrourceFile(foo) -2

#define GET_TRACK_STREAM , int streamId, const unsigned char* streamStart
#define PUT_TRACK_STREAM_ARGS , streamId, streamStart
#define PUT_TRACK_STREAM(start,end) , streamId, start-streamStart, end-streamStart
#define PUT_TRACK_STREAM_ARGS_EXPL(a,b,c) , a, b, c
#define PUT_TRACK_STREAM_ARGS_EXPL2(a,b) , a, b


#define PUT_TRACK_PARAMETERS , streamId, offsetStart, offsetEnd
#define PUT_TRACK_PARAMETERS_OBJ(obj) , (obj)->streamId(), (obj)->offsetStart(), (obj)->offsetEnd()
#define PUT_TRACK_PARAMETERS_OBJ2(obj,objEnd) , (obj)->streamId(), (obj)->offsetStart(), (objEnd)->offsetEnd()
#define PUT_TRACK_PARAMETERS_SRC , SkPdfTrackSrourceFile(__FILE__), __LINE__, __LINE__
#define PUT_TRACK_PARAMETERS_SRC0  SkPdfTrackSrourceFile(__FILE__), __LINE__, __LINE__
#define GET_TRACK_PARAMETERS , int streamId, int offsetStart, int offsetEnd
#define GET_TRACK_PARAMETERS0  int streamId, int offsetStart, int offsetEnd
#define STORE_TRACK_PARAMETERS(obj) (obj)->fStreamId = streamId; (obj)->fOffsetStart = offsetStart; (obj)->fOffsetEnd = offsetEnd;
#define STORE_TRACK_PARAMETER_OFFSET_END(obj,offsetEnd) (obj)->fOffsetEnd = (offsetEnd)-streamStart;
#else
#define GET_TRACK_STREAM
#define PUT_TRACK_STREAM_ARGS
#define PUT_TRACK_STREAM(start,end)
#define PUT_TRACK_STREAM_ARGS_EXPL(a,b,c)
#define PUT_TRACK_STREAM_ARGS_EXPL2(a,b)


#define PUT_TRACK_PARAMETERS
#define PUT_TRACK_PARAMETERS_OBJ(obj)
#define PUT_TRACK_PARAMETERS_OBJ2(obj,objEnd)
#define PUT_TRACK_PARAMETERS_SRC
#define PUT_TRACK_PARAMETERS_SRC0
#define GET_TRACK_PARAMETERS
#define GET_TRACK_PARAMETERS0
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
