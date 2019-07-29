/*
 * Copyright 2014 Google Inc.
 * Copyright 2015 Xamarin Inc.
 * Copyright 2017 Microsoft Corporation. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef sk_stream_DEFINED
#define sk_stream_DEFINED

#include "sk_types.h"

SK_C_PLUS_PLUS_BEGIN_GUARD

SK_C_API void sk_stream_asset_destroy(sk_stream_asset_t* cstream);

////////////////////////////////////////////////////////////////////////////////

SK_C_API sk_stream_filestream_t* sk_filestream_new(const char* path);
SK_C_API void sk_filestream_destroy(sk_stream_filestream_t* cstream);
SK_C_API bool sk_filestream_is_valid(sk_stream_filestream_t* cstream);

////////////////////////////////////////////////////////////////////////////////

SK_C_API sk_stream_memorystream_t* sk_memorystream_new(void);
SK_C_API sk_stream_memorystream_t* sk_memorystream_new_with_length(size_t length);
SK_C_API sk_stream_memorystream_t* sk_memorystream_new_with_data(const void* data, size_t length, bool copyData);
SK_C_API sk_stream_memorystream_t* sk_memorystream_new_with_skdata(sk_data_t* data);
SK_C_API void sk_memorystream_set_memory(sk_stream_memorystream_t* cmemorystream, const void* data, size_t length, bool copyData);
SK_C_API void sk_memorystream_destroy(sk_stream_memorystream_t* cstream);

////////////////////////////////////////////////////////////////////////////////

SK_C_API size_t sk_stream_read(sk_stream_t* cstream, void* buffer, size_t size);
SK_C_API size_t sk_stream_peek(sk_stream_t* cstream, void* buffer, size_t size);
SK_C_API size_t sk_stream_skip(sk_stream_t* cstream, size_t size);
SK_C_API bool sk_stream_is_at_end(sk_stream_t* cstream);
SK_C_API bool sk_stream_read_s8(sk_stream_t* cstream, int8_t* buffer);
SK_C_API bool sk_stream_read_s16(sk_stream_t* cstream, int16_t* buffer);
SK_C_API bool sk_stream_read_s32(sk_stream_t* cstream, int32_t* buffer);
SK_C_API bool sk_stream_read_u8(sk_stream_t* cstream, uint8_t* buffer);
SK_C_API bool sk_stream_read_u16(sk_stream_t* cstream, uint16_t* buffer);
SK_C_API bool sk_stream_read_u32(sk_stream_t* cstream, uint32_t* buffer);
SK_C_API bool sk_stream_read_bool(sk_stream_t* cstream, bool* buffer);
SK_C_API bool sk_stream_rewind(sk_stream_t* cstream);
SK_C_API bool sk_stream_has_position(sk_stream_t* cstream);
SK_C_API size_t sk_stream_get_position(sk_stream_t* cstream);
SK_C_API bool sk_stream_seek(sk_stream_t* cstream, size_t position);
SK_C_API bool sk_stream_move(sk_stream_t* cstream, long offset);
SK_C_API bool sk_stream_has_length(sk_stream_t* cstream);
SK_C_API size_t sk_stream_get_length(sk_stream_t* cstream);
SK_C_API const void* sk_stream_get_memory_base(sk_stream_t* cstream);
SK_C_API sk_stream_t* sk_stream_fork(sk_stream_t* cstream);
SK_C_API sk_stream_t* sk_stream_duplicate(sk_stream_t* cstream);
SK_C_API void sk_stream_destroy(sk_stream_t* cstream);

////////////////////////////////////////////////////////////////////////////////

SK_C_API sk_wstream_filestream_t* sk_filewstream_new(const char* path);
SK_C_API void sk_filewstream_destroy(sk_wstream_filestream_t* cstream);
SK_C_API bool sk_filewstream_is_valid(sk_wstream_filestream_t* cstream);

SK_C_API sk_wstream_dynamicmemorystream_t* sk_dynamicmemorywstream_new(void);
SK_C_API sk_stream_asset_t* sk_dynamicmemorywstream_detach_as_stream(sk_wstream_dynamicmemorystream_t* cstream);
SK_C_API sk_data_t* sk_dynamicmemorywstream_detach_as_data(sk_wstream_dynamicmemorystream_t* cstream);
SK_C_API void sk_dynamicmemorywstream_copy_to(sk_wstream_dynamicmemorystream_t* cstream, void* data);
SK_C_API bool sk_dynamicmemorywstream_write_to_stream(sk_wstream_dynamicmemorystream_t* cstream, sk_wstream_t* dst);
SK_C_API void sk_dynamicmemorywstream_destroy(sk_wstream_dynamicmemorystream_t* cstream);

////////////////////////////////////////////////////////////////////////////////

SK_C_API bool sk_wstream_write(sk_wstream_t* cstream, const void* buffer, size_t size);
SK_C_API bool sk_wstream_newline(sk_wstream_t* cstream);
SK_C_API void sk_wstream_flush(sk_wstream_t* cstream);
SK_C_API size_t sk_wstream_bytes_written(sk_wstream_t* cstream);
SK_C_API bool sk_wstream_write_8(sk_wstream_t* cstream, uint8_t value);
SK_C_API bool sk_wstream_write_16(sk_wstream_t* cstream, uint16_t value);
SK_C_API bool sk_wstream_write_32(sk_wstream_t* cstream, uint32_t value);
SK_C_API bool sk_wstream_write_text(sk_wstream_t* cstream, const char* value);
SK_C_API bool sk_wstream_write_dec_as_text(sk_wstream_t* cstream, int32_t value);
SK_C_API bool sk_wstream_write_bigdec_as_text(sk_wstream_t* cstream, int64_t value, int minDigits);
SK_C_API bool sk_wstream_write_hex_as_text(sk_wstream_t* cstream, uint32_t value, int minDigits);
SK_C_API bool sk_wstream_write_scalar_as_text(sk_wstream_t* cstream, float value);
SK_C_API bool sk_wstream_write_bool(sk_wstream_t* cstream, bool value);
SK_C_API bool sk_wstream_write_scalar(sk_wstream_t* cstream, float value);
SK_C_API bool sk_wstream_write_packed_uint(sk_wstream_t* cstream, size_t value);
SK_C_API bool sk_wstream_write_stream(sk_wstream_t* cstream, sk_stream_t* input, size_t length);
SK_C_API int sk_wstream_get_size_of_packed_uint(size_t value);

SK_C_PLUS_PLUS_END_GUARD

#endif
