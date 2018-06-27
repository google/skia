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


typedef struct sk_wstream_managedstream_t sk_wstream_managedstream_t;


typedef bool   (*sk_managedwstream_write_delegate)        (sk_wstream_managedstream_t* cmanagedStream, const void* buffer, size_t size);
typedef void   (*sk_managedwstream_flush_delegate)        (sk_wstream_managedstream_t* cmanagedStream);
typedef size_t (*sk_managedwstream_bytesWritten_delegate) (const sk_wstream_managedstream_t* cmanagedStream);
typedef void   (*sk_managedwstream_destroy_delegate)      (size_t cmanagedStream);


SK_X_API sk_wstream_managedstream_t* sk_managedwstream_new (void);
SK_X_API void sk_managedwstream_destroy (sk_wstream_managedstream_t*);

SK_X_API void sk_managedwstream_set_delegates (const sk_managedwstream_write_delegate pWrite,
                                               const sk_managedwstream_flush_delegate pFlush,
                                               const sk_managedwstream_bytesWritten_delegate pBytesWritten,
                                               const sk_managedwstream_destroy_delegate pDestroy);


typedef struct sk_stream_managedstream_t sk_stream_managedstream_t;


typedef size_t                     (*sk_managedstream_read_delegate)         (sk_stream_managedstream_t* cmanagedStream, void* buffer, size_t size);
typedef size_t                     (*sk_managedstream_peek_delegate)         (sk_stream_managedstream_t* cmanagedStream, void* buffer, size_t size);
typedef bool                       (*sk_managedstream_isAtEnd_delegate)      (const sk_stream_managedstream_t* cmanagedStream);
typedef bool                       (*sk_managedstream_hasPosition_delegate)  (const sk_stream_managedstream_t* cmanagedStream);
typedef bool                       (*sk_managedstream_hasLength_delegate)    (const sk_stream_managedstream_t* cmanagedStream);
typedef bool                       (*sk_managedstream_rewind_delegate)       (sk_stream_managedstream_t* cmanagedStream);
typedef size_t                     (*sk_managedstream_getPosition_delegate)  (const sk_stream_managedstream_t* cmanagedStream);
typedef bool                       (*sk_managedstream_seek_delegate)         (sk_stream_managedstream_t* cmanagedStream, size_t position);
typedef bool                       (*sk_managedstream_move_delegate)         (sk_stream_managedstream_t* cmanagedStream, long offset);
typedef size_t                     (*sk_managedstream_getLength_delegate)    (const sk_stream_managedstream_t* cmanagedStream);
typedef sk_stream_managedstream_t* (*sk_managedstream_createNew_delegate)    (const sk_stream_managedstream_t* cmanagedStream);
typedef void                       (*sk_managedstream_destroy_delegate)      (size_t cmanagedStream);


// c API
SK_X_API sk_stream_managedstream_t* sk_managedstream_new (void);
SK_X_API void sk_managedstream_destroy (sk_stream_managedstream_t*);

SK_X_API void sk_managedstream_set_delegates (const sk_managedstream_read_delegate pRead,
                                              const sk_managedstream_peek_delegate pPeek,
                                              const sk_managedstream_isAtEnd_delegate pIsAtEnd,
                                              const sk_managedstream_hasPosition_delegate pHasPosition,
                                              const sk_managedstream_hasLength_delegate pHasLength,
                                              const sk_managedstream_rewind_delegate pRewind,
                                              const sk_managedstream_getPosition_delegate pGetPosition,
                                              const sk_managedstream_seek_delegate pSeek,
                                              const sk_managedstream_move_delegate pMove,
                                              const sk_managedstream_getLength_delegate pGetLength,
                                              const sk_managedstream_createNew_delegate pCreateNew,
                                              const sk_managedstream_destroy_delegate pDestroy);


SK_C_PLUS_PLUS_END_GUARD

#endif
