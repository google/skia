
/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkTLS.h"
#include "SkTypes.h"
#include "SkError.h"
#include "SkErrorInternals.h"

#include <stdio.h>
#include <stdarg.h>

namespace {
    void *CreateThreadError() {
        return SkNEW_ARGS(SkError, (kNoError_SkError));
    }
    void DeleteThreadError(void* v) {
        SkDELETE(reinterpret_cast<SkError*>(v));
    }
    #define THREAD_ERROR \
        (*reinterpret_cast<SkError*>(SkTLS::Get(CreateThreadError, DeleteThreadError)))

    void *CreateThreadErrorCallback() {
        return SkNEW_ARGS(SkErrorCallbackFunction, (SkErrorInternals::DefaultErrorCallback));
    }
    void DeleteThreadErrorCallback(void* v) {
        SkDELETE(reinterpret_cast<SkErrorCallbackFunction *>(v));
    }

    #define THREAD_ERROR_CALLBACK                                                             \
        *(reinterpret_cast<SkErrorCallbackFunction *>(SkTLS::Get(CreateThreadErrorCallback,   \
                                                                 DeleteThreadErrorCallback)))

    void *CreateThreadErrorContext() {
        return SkNEW_ARGS(void **, (NULL));
    }
    void DeleteThreadErrorContext(void* v) {
        SkDELETE(reinterpret_cast<void **>(v));
    }
    #define THREAD_ERROR_CONTEXT \
        (*reinterpret_cast<void **>(SkTLS::Get(CreateThreadErrorContext, DeleteThreadErrorContext)))

    #define ERROR_STRING_LENGTH 2048

    void *CreateThreadErrorString() {
        return SkNEW_ARRAY(char, (ERROR_STRING_LENGTH));
    }
    void DeleteThreadErrorString(void* v) {
        SkDELETE_ARRAY(reinterpret_cast<char *>(v));
    }
    #define THREAD_ERROR_STRING \
        (reinterpret_cast<char *>(SkTLS::Get(CreateThreadErrorString, DeleteThreadErrorString)))
}

SkError SkGetLastError() {
    return SkErrorInternals::GetLastError();
}
void SkClearLastError() {
    SkErrorInternals::ClearError();
}
void SkSetErrorCallback(SkErrorCallbackFunction cb, void *context) {
    SkErrorInternals::SetErrorCallback(cb, context);
}
const char *SkGetLastErrorString() {
    return SkErrorInternals::GetLastErrorString();
}

// ------------ Private Error functions ---------

void SkErrorInternals::SetErrorCallback(SkErrorCallbackFunction cb, void *context) {
    if (cb == NULL) {
        THREAD_ERROR_CALLBACK = SkErrorInternals::DefaultErrorCallback;
    } else {
        THREAD_ERROR_CALLBACK = cb;
    }
    THREAD_ERROR_CONTEXT = context;
}

void SkErrorInternals::DefaultErrorCallback(SkError code, void *context) {
    SkDebugf("Skia Error: %s\n", SkGetLastErrorString());
}

void SkErrorInternals::ClearError() {
    SkErrorInternals::SetError( kNoError_SkError, "All is well" );
}

SkError SkErrorInternals::GetLastError() {
    return THREAD_ERROR;
}

const char *SkErrorInternals::GetLastErrorString() {
    return THREAD_ERROR_STRING;
}

void SkErrorInternals::SetError(SkError code, const char *fmt, ...) {
    THREAD_ERROR = code;
    va_list args;

    char *str = THREAD_ERROR_STRING;
    const char *error_name = NULL;
    switch( code ) {
        case kNoError_SkError:
            error_name = "No Error";
            break;
        case kInvalidArgument_SkError:
            error_name = "Invalid Argument";
            break;
        case kInvalidOperation_SkError:
            error_name = "Invalid Operation";
            break;
        case kInvalidHandle_SkError:
            error_name = "Invalid Handle";
            break;
        case kInvalidPaint_SkError:
            error_name = "Invalid Paint";
            break;
        case kOutOfMemory_SkError:
            error_name = "Out Of Memory";
            break;
        case kParseError_SkError:
            error_name = "Parse Error";
            break;
        default:
            error_name = "Unknown error";
            break;
    }

    sprintf( str, "%s: ", error_name );
    int string_left = SkToInt(ERROR_STRING_LENGTH - strlen(str));
    str += strlen(str);

    va_start( args, fmt );
    vsnprintf( str, string_left, fmt, args );
    va_end( args );
    SkErrorCallbackFunction fn = THREAD_ERROR_CALLBACK;
    if (fn && code != kNoError_SkError) {
        fn(code, THREAD_ERROR_CONTEXT);
    }
}
