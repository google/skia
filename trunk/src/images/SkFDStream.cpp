
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkStream.h"
#include <unistd.h>

//#define TRACE_FDSTREAM

SkFDStream::SkFDStream(int fileDesc, bool closeWhenDone)
    : fFD(fileDesc), fCloseWhenDone(closeWhenDone) {
}

SkFDStream::~SkFDStream() {
    if (fFD >= 0 && fCloseWhenDone) {
        ::close(fFD);
    }
}

bool SkFDStream::rewind() {
    if (fFD >= 0) {
        off_t value = ::lseek(fFD, 0, SEEK_SET);
#ifdef TRACE_FDSTREAM
        if (value) {
            SkDebugf("xxxxxxxxxxxxxx rewind failed %d\n", value);
        }
#endif
        return value == 0;
    }
    return false;
}

size_t SkFDStream::read(void* buffer, size_t size) {
    if (fFD >= 0) {
        if (buffer == NULL && size == 0) {  // request total size
            off_t curr = ::lseek(fFD, 0, SEEK_CUR);
            if (curr < 0) {
#ifdef TRACE_FDSTREAM
                SkDebugf("xxxxxxxxxxxxx lseek failed 0 CURR\n");
#endif
                return 0;   // error
            }
            off_t size = ::lseek(fFD, 0, SEEK_END);
            if (size < 0) {
#ifdef TRACE_FDSTREAM
                SkDebugf("xxxxxxxxxxxxx lseek failed 0 END\n");
#endif
                size = 0;   // error
            }
            if (::lseek(fFD, curr, SEEK_SET) != curr) {
                // can't restore, error
#ifdef TRACE_FDSTREAM
                SkDebugf("xxxxxxxxxxxxx lseek failed %d SET\n", curr);
#endif
                return 0;
            }
            return (size_t) size;
        } else if (NULL == buffer) {        // skip
            off_t oldCurr = ::lseek(fFD, 0, SEEK_CUR);
            if (oldCurr < 0) {
#ifdef TRACE_FDSTREAM
                SkDebugf("xxxxxxxxxxxxx lseek1 failed %d CUR\n", oldCurr);
#endif
                return 0;   // error;
            }
            off_t newCurr = ::lseek(fFD, size, SEEK_CUR);
            if (newCurr < 0) {
#ifdef TRACE_FDSTREAM
                SkDebugf("xxxxxxxxxxxxx lseek2 failed %d CUR\n", newCurr);
#endif
                return 0;   // error;
            }
            // return the actual amount we skipped
            return (size_t) (newCurr - oldCurr);
        } else {                            // read
            ssize_t actual = ::read(fFD, buffer, size);
            // our API can't return an error, so we return 0
            if (actual < 0) {
#ifdef TRACE_FDSTREAM
                SkDebugf("xxxxxxxxxxxxx read failed %d actual %d\n", size, actual);
#endif
                actual = 0;
            }
            return actual;
        }
    }
    return 0;
}

