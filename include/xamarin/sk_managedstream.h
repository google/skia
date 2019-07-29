/*
 * Copyright 2015 Xamarin Inc.
 * Copyright 2017 Microsoft Corporation. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef sk_managedstream_DEFINED
#define sk_managedstream_DEFINED

#include "sk_xamarin.h"

#include "sk_types.h"

SK_C_PLUS_PLUS_BEGIN_GUARD

// WRITEABLE MANAGED STREAM

typedef struct sk_wstream_managedstream_t sk_wstream_managedstream_t;

typedef bool   (*sk_managedwstream_write_proc)        (      sk_wstream_managedstream_t* s, void* context, const void* buffer, size_t size);
typedef void   (*sk_managedwstream_flush_proc)        (      sk_wstream_managedstream_t* s, void* context);
typedef size_t (*sk_managedwstream_bytesWritten_proc) (const sk_wstream_managedstream_t* s, void* context);
typedef void   (*sk_managedwstream_destroy_proc)      (      sk_wstream_managedstream_t* s, void* context);

typedef struct {
    sk_managedwstream_write_proc fWrite;
    sk_managedwstream_flush_proc fFlush;
    sk_managedwstream_bytesWritten_proc fBytesWritten;
    sk_managedwstream_destroy_proc fDestroy;
} sk_managedwstream_procs_t;

SK_X_API void sk_managedwstream_set_procs(sk_managedwstream_procs_t procs);

SK_X_API sk_wstream_managedstream_t* sk_managedwstream_new(void* context);
SK_X_API void sk_managedwstream_destroy(sk_wstream_managedstream_t* s);


// READ-ONLY MANAGED STREAM

typedef struct sk_stream_managedstream_t sk_stream_managedstream_t;

typedef size_t                     (*sk_managedstream_read_proc)         (      sk_stream_managedstream_t* s, void* context, void* buffer, size_t size);
typedef size_t                     (*sk_managedstream_peek_proc)         (const sk_stream_managedstream_t* s, void* context, void* buffer, size_t size);
typedef bool                       (*sk_managedstream_isAtEnd_proc)      (const sk_stream_managedstream_t* s, void* context);
typedef bool                       (*sk_managedstream_hasPosition_proc)  (const sk_stream_managedstream_t* s, void* context);
typedef bool                       (*sk_managedstream_hasLength_proc)    (const sk_stream_managedstream_t* s, void* context);
typedef bool                       (*sk_managedstream_rewind_proc)       (      sk_stream_managedstream_t* s, void* context);
typedef size_t                     (*sk_managedstream_getPosition_proc)  (const sk_stream_managedstream_t* s, void* context);
typedef bool                       (*sk_managedstream_seek_proc)         (      sk_stream_managedstream_t* s, void* context, size_t position);
typedef bool                       (*sk_managedstream_move_proc)         (      sk_stream_managedstream_t* s, void* context, long offset);
typedef size_t                     (*sk_managedstream_getLength_proc)    (const sk_stream_managedstream_t* s, void* context);
typedef sk_stream_managedstream_t* (*sk_managedstream_duplicate_proc)    (const sk_stream_managedstream_t* s, void* context);
typedef sk_stream_managedstream_t* (*sk_managedstream_fork_proc)         (const sk_stream_managedstream_t* s, void* context);
typedef void                       (*sk_managedstream_destroy_proc)      (      sk_stream_managedstream_t* s, void* context);

typedef struct {
    sk_managedstream_read_proc fRead;
    sk_managedstream_peek_proc fPeek;
    sk_managedstream_isAtEnd_proc fIsAtEnd;
    sk_managedstream_hasPosition_proc fHasPosition;
    sk_managedstream_hasLength_proc fHasLength;
    sk_managedstream_rewind_proc fRewind;
    sk_managedstream_getPosition_proc fGetPosition;
    sk_managedstream_seek_proc fSeek;
    sk_managedstream_move_proc fMove;
    sk_managedstream_getLength_proc fGetLength;
    sk_managedstream_duplicate_proc fDuplicate;
    sk_managedstream_fork_proc fFork;
    sk_managedstream_destroy_proc fDestroy;
} sk_managedstream_procs_t;

SK_X_API void sk_managedstream_set_procs(sk_managedstream_procs_t procs);

SK_X_API sk_stream_managedstream_t* sk_managedstream_new(void* context);
SK_X_API void sk_managedstream_destroy(sk_stream_managedstream_t* s);

SK_C_PLUS_PLUS_END_GUARD

#endif
