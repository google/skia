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

#if SK_DYNAMIC_ANNOTATIONS_ENABLED

extern "C" {
// TSAN provides these hooks.
void AnnotateIgnoreReadsBegin(const char* file, int line);
void AnnotateIgnoreReadsEnd(const char* file, int line);
void AnnotateIgnoreWritesBegin(const char* file, int line);
void AnnotateIgnoreWritesEnd(const char* file, int line);
void AnnotateBenignRaceSized(const char* file, int line,
                             const volatile void* addr, long size, const char* desc);
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

// Like SK_ANNOTATE_UNPROTECTED_READ, but for writes.
template <typename T>
inline void SK_ANNOTATE_UNPROTECTED_WRITE(T* ptr, const volatile T& val) {
    AnnotateIgnoreWritesBegin(__FILE__, __LINE__);
    *ptr = val;
    AnnotateIgnoreWritesEnd(__FILE__, __LINE__);
}

// Ignore racy reads and racy writes to this pointer, indefinitely.
// If at all possible, use the more precise SK_ANNOTATE_UNPROTECTED_READ.
template <typename T>
void SK_ANNOTATE_BENIGN_RACE(T* ptr) {
    AnnotateBenignRaceSized(__FILE__, __LINE__, ptr, sizeof(*ptr), "SK_ANNOTATE_BENIGN_RACE");
}

#else  // !SK_DYNAMIC_ANNOTATIONS_ENABLED

#define SK_ANNOTATE_UNPROTECTED_READ(x) (x)
#define SK_ANNOTATE_UNPROTECTED_WRITE(ptr, val) *(ptr) = (val)
#define SK_ANNOTATE_BENIGN_RACE(ptr)

#endif

// Can be used to wrap values that are intentionally racy, usually small mutable cached values, e.g.
//   - SkMatrix type mask
//   - SkPixelRef genIDs
template <typename T>
class SkTRacy {
public:
    operator const T() const {
        return SK_ANNOTATE_UNPROTECTED_READ(fVal);
    }

    SkTRacy& operator=(const T& val) {
        SK_ANNOTATE_UNPROTECTED_WRITE(&fVal, val);
        return *this;
    }

private:
    T fVal;
};

// This is like SkTRacy, but allows you to return the value by reference.
// TSAN is better at suppressing SkTRacy than SkTRacyReffable, so use SkTRacy when possible.
//
// We use this for SkPathRef bounds, which is an SkRect we pass around by reference publically.
template <typename T>
class SkTRacyReffable {
public:
    SkTRacyReffable() { SK_ANNOTATE_BENIGN_RACE(&fVal); }

    operator const T&() const {
        return fVal;
    }

    SkTRacyReffable& operator=(const T& val) {
        fVal = val;
        return *this;
    }

    const T* get() const { return &fVal; }
          T* get()       { return &fVal; }

    const T* operator->() const { return &fVal; }
          T* operator->()       { return &fVal; }

private:
    T fVal;
};

#endif//SkDynamicAnnotations_DEFINED
