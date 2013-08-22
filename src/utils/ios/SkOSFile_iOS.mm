/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <Foundation/Foundation.h>
#include "SkOSFile.h"
#include "SkString.h"

struct SkFILE {
    NSData* fData;
    size_t  fOffset;
    size_t  fLength;
};

SkFILE* sk_fopen(const char cpath[], SkFILE_Flags flags) {
    if (flags & kWrite_SkFILE_Flag) {
        return NULL;
    }

    SkString cname, csuffix;

    const char* start = strrchr(cpath, '/');
    if (NULL == start) {
        start = cpath;
    } else {
        start += 1;
    }
    const char* stop = strrchr(cpath, '.');
    if (NULL == stop) {
        return NULL;
    } else {
        stop += 1;
    }

    cname.set(start, stop - start - 1);
    csuffix.set(stop);

    NSBundle* bundle = [NSBundle mainBundle];
    NSString* name = [NSString stringWithUTF8String:cname.c_str()];
    NSString* suffix = [NSString stringWithUTF8String:csuffix.c_str()];
    NSString* path = [bundle pathForResource:name ofType:suffix];
    NSData* data = [NSData dataWithContentsOfMappedFile:path];

    if (data) {
        [data retain];
        SkFILE* rec = new SkFILE;
        rec->fData = data;
        rec->fOffset = 0;
        rec->fLength = [data length];
        return reinterpret_cast<SkFILE*>(rec);
    }
    return NULL;
}

size_t sk_fgetsize(SkFILE* rec) {
    SkASSERT(rec);
    return rec->fLength;
}

bool sk_frewind(SkFILE* rec) {
    SkASSERT(rec);
    rec->fOffset = 0;
    return true;
}

size_t sk_fread(void* buffer, size_t byteCount, SkFILE* rec) {
    if (NULL == buffer) {
        return rec->fLength;
    } else {
        size_t remaining = rec->fLength - rec->fOffset;
        if (byteCount > remaining) {
            byteCount = remaining;
        }
        memcpy(buffer, (char*)[rec->fData bytes] + rec->fOffset, byteCount);
        rec->fOffset += byteCount;
        SkASSERT(rec->fOffset <= rec->fLength);
        return byteCount;
    }
}

size_t sk_fwrite(const void* buffer, size_t byteCount, SkFILE* f) {
    SkDEBUGFAIL("Not supported yet");
    return 0;
}

void sk_fflush(SkFILE* f) {
    SkDEBUGFAIL("Not supported yet");
}

void sk_fclose(SkFILE* rec) {
    SkASSERT(rec);
    [rec->fData release];
    delete rec;
}

