/*
 * Copyright 2015 Xamarin Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// EXPERIMENTAL EXPERIMENTAL EXPERIMENTAL EXPERIMENTAL
// DO NOT USE -- FOR INTERNAL TESTING ONLY

#ifndef sk_stream_DEFINED
#define sk_stream_DEFINED

#include "sk_types.h"

SK_C_PLUS_PLUS_BEGIN_GUARD

SK_API sk_stream_filestream_t* sk_filestream_new (const char* path);

SK_API sk_stream_memorystream_t* sk_memorystream_new ();
SK_API sk_stream_memorystream_t* sk_memorystream_new_with_length (size_t length);
SK_API sk_stream_memorystream_t* sk_memorystream_new_with_data (const void* data, size_t length, bool copyData);
SK_API sk_stream_memorystream_t* sk_memorystream_new_with_skdata (sk_data_t* data);
SK_API void sk_memorystream_set_memory (sk_stream_memorystream_t* cmemorystream, const void* data, size_t length, bool copyData);

SK_API bool sk_stream_is_at_end (sk_stream_t* cstream);
SK_API int8_t sk_stream_read_s8 (sk_stream_t* cstream);
SK_API int16_t sk_stream_read_s16 (sk_stream_t* cstream);
SK_API int32_t sk_stream_read_s32 (sk_stream_t* cstream);
SK_API uint8_t sk_stream_read_u8 (sk_stream_t* cstream);
SK_API uint16_t sk_stream_read_u16 (sk_stream_t* cstream);
SK_API uint32_t sk_stream_read_u32 (sk_stream_t* cstream);
SK_API bool sk_stream_read_bool (sk_stream_t* cstream);

SK_C_PLUS_PLUS_END_GUARD

#endif
