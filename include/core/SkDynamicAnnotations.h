/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkDynamicAnnotations_DEFINED
#define SkDynamicAnnotations_DEFINED

// This file contains macros used to send out-of-band signals to dynamic instrumentation systems,
// namely thread sanitizer.  This is a cut-down version of the full dynamic_annotations library with
// only the features used by Skia.

// We check the same define to know to enable the annotations, but prefix all our macros with SK_.
#if DYNAMIC_ANNOTATIONS_ENABLED

extern "C" {
// TSAN provides these hooks.
void AnnotateIgnoreReadsBegin(const char* file, int line);
void AnnotateIgnoreReadsEnd(const char* file, int line);
}  // extern "C"

// SK_ANNOTATE_UNPROTECTED_READ can wrap any variable read to tell TSAN to ignore that it appears to
// be a racy read.  This should be used only when we can make an external guarantee that though this
// particular read is racy, it is being used as part of a mechanism which is thread safe.  Examples:
//   - the first check in double-checked locking;
//   - checking if a ref count is equal to 1.
// Note that in both these cases, we must still add terrifyingly subtle memory barriers to provide
// that overall thread safety guarantee.  Using this macro to shut TSAN up without providing such an
// external guarantee is pretty much never correct.
template <typename T>
inline T SK_ANNOTATE_UNPROTECTED_READ(const volatile T& x) {
    AnnotateIgnoreReadsBegin(__FILE__, __LINE__);
    T read = x;
    AnnotateIgnoreReadsEnd(__FILE__, __LINE__);
    return read;
}

#else  // !DYNAMIC_ANNOTATIONS_ENABLED

#define SK_ANNOTATE_UNPROTECTED_READ(x) (x)

#endif

#endif//SkDynamicAnnotations_DEFINED
