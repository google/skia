/*
 * Copyright 2016 Xamarin Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkStream.h"

#include "sk_stream.h"

#include "sk_types_priv.h"

void sk_stream_asset_destroy(sk_stream_asset_t* cstream)
{
    delete AsStreamAsset(cstream);
}

void sk_filestream_destroy(sk_stream_filestream_t* cstream)
{
    delete AsFileStream(cstream);
}

void sk_memorystream_destroy(sk_stream_memorystream_t* cstream)
{
    delete AsMemoryStream(cstream);
}

sk_stream_filestream_t* sk_filestream_new (const char* path)
{
    return ToFileStream(new SkFILEStream(path));
}

sk_stream_memorystream_t* sk_memorystream_new ()
{
    return ToMemoryStream(new SkMemoryStream());
}
sk_stream_memorystream_t* sk_memorystream_new_with_length (size_t length)
{
    return ToMemoryStream(new SkMemoryStream(length));
}
sk_stream_memorystream_t* sk_memorystream_new_with_data (const void* data, size_t length, bool copyData)
{
    return ToMemoryStream(new SkMemoryStream(data, length, copyData));
}
sk_stream_memorystream_t* sk_memorystream_new_with_skdata (sk_data_t* data)
{
    return ToMemoryStream(new SkMemoryStream(AsData(data)));
}
void sk_memorystream_set_memory (sk_stream_memorystream_t* cmemorystream, const void* data, size_t length, bool copyData)
{
    AsMemoryStream(cmemorystream)->setMemory(data, length, copyData);
}

size_t sk_stream_read (sk_stream_t* cstream, void* buffer, size_t size)
{
    return AsStream(cstream)->read(buffer, size);
}

size_t sk_stream_skip (sk_stream_t* cstream, size_t size)
{
    return AsStream(cstream)->skip(size);
}

bool sk_stream_is_at_end (sk_stream_t* cstream)
{
    return AsStream(cstream)->isAtEnd();
}
int8_t sk_stream_read_s8 (sk_stream_t* cstream)
{
    return AsStream(cstream)->readS8();
}
int16_t sk_stream_read_s16 (sk_stream_t* cstream)
{
    return AsStream(cstream)->readS16();
}
int32_t sk_stream_read_s32 (sk_stream_t* cstream)
{
    return AsStream(cstream)->readS32();
}
uint8_t sk_stream_read_u8 (sk_stream_t* cstream)
{
    return AsStream(cstream)->readU8();
}
uint16_t sk_stream_read_u16 (sk_stream_t* cstream)
{
    return AsStream(cstream)->readU16();
}
uint32_t sk_stream_read_u32 (sk_stream_t* cstream)
{
    return AsStream(cstream)->readU32();
}
bool sk_stream_read_bool (sk_stream_t* cstream)
{
    return AsStream(cstream)->readBool();
}

bool sk_stream_rewind (sk_stream_t* cstream)
{
    return AsStream(cstream)->rewind();
}

bool sk_stream_has_position (sk_stream_t* cstream)
{
    return AsStream(cstream)->hasPosition();
}

size_t sk_stream_get_position (sk_stream_t* cstream)
{
    return AsStream(cstream)->getPosition();
}

bool sk_stream_seek (sk_stream_t* cstream, size_t position)
{
    return AsStream(cstream)->seek(position);
}

bool sk_stream_move (sk_stream_t* cstream, long offset)
{
    return AsStream(cstream)->move(offset);
}

bool sk_stream_has_length (sk_stream_t* cstream)
{
    return AsStream(cstream)->hasLength();
}

size_t sk_stream_get_length (sk_stream_t* cstream)
{
    return AsStream(cstream)->getLength();
}

void sk_filewstream_destroy(sk_wstream_filestream_t* cstream)
{
    delete AsFileWStream(cstream);
}

void sk_dynamicmemorywstream_destroy(sk_wstream_dynamicmemorystream_t* cstream)
{
    delete AsDynamicMemoryWStream(cstream);
}

sk_wstream_filestream_t* sk_filewstream_new(const char* path)
{
    return ToFileWStream(new SkFILEWStream(path));
}

sk_wstream_dynamicmemorystream_t* sk_dynamicmemorywstream_new()
{
    return ToDynamicMemoryWStream(new SkDynamicMemoryWStream());
}

sk_data_t* sk_dynamicmemorywstream_copy_to_data(sk_wstream_dynamicmemorystream_t* cstream)
{
    return ToData(AsDynamicMemoryWStream(cstream)->copyToData());
}

sk_stream_asset_t* sk_dynamicmemorywstream_detach_as_stream(sk_wstream_dynamicmemorystream_t* cstream)
{
    return ToStreamAsset(AsDynamicMemoryWStream(cstream)->detachAsStream());
}

bool sk_wstream_write(sk_wstream_t* cstream, const void* buffer, size_t size)
{
    return AsWStream(cstream)->write(buffer, size);
}

void sk_wstream_newline(sk_wstream_t* cstream)
{
    return AsWStream(cstream)->newline();
}

void sk_wstream_flush(sk_wstream_t* cstream)
{
    return AsWStream(cstream)->flush();
}

size_t sk_wstream_bytes_written(sk_wstream_t* cstream)
{
    return AsWStream(cstream)->bytesWritten();
}

bool sk_wstream_write_8(sk_wstream_t* cstream, uint8_t value)
{
    return AsWStream(cstream)->write8(value);
}

bool sk_wstream_write_16(sk_wstream_t* cstream, uint16_t value)
{
    return AsWStream(cstream)->write16(value);
}

bool sk_wstream_write_32(sk_wstream_t* cstream, uint32_t value)
{
    return AsWStream(cstream)->write32(value);
}

bool sk_wstream_write_text(sk_wstream_t* cstream, const char* value)
{
    return AsWStream(cstream)->writeText(value);
}

bool sk_wstream_write_dec_as_text(sk_wstream_t* cstream, int32_t value)
{
    return AsWStream(cstream)->writeDecAsText(value);
}

bool sk_wstream_write_bigdec_as_text(sk_wstream_t* cstream, int64_t value, int minDigits)
{
    return AsWStream(cstream)->writeBigDecAsText(value, minDigits);
}

bool sk_wstream_write_hex_as_text(sk_wstream_t* cstream, uint32_t value, int minDigits)
{
    return AsWStream(cstream)->writeHexAsText(value, minDigits);
}

bool sk_wstream_write_scalar_as_text(sk_wstream_t* cstream, float value)
{
    return AsWStream(cstream)->writeScalarAsText(value);
}

bool sk_wstream_write_bool(sk_wstream_t* cstream, bool value)
{
    return AsWStream(cstream)->writeBool(value);
}

bool sk_wstream_write_scalar(sk_wstream_t* cstream, float value)
{
    return AsWStream(cstream)->writeScalar(value);
}

bool sk_wstream_write_packed_uint(sk_wstream_t* cstream, size_t value)
{
    return AsWStream(cstream)->writePackedUInt(value);
}

bool sk_wstream_write_stream(sk_wstream_t* cstream, sk_stream_t* input, size_t length)
{
    return AsWStream(cstream)->writeStream(AsStream(input), length);
}

int sk_wstream_get_size_of_packed_uint(size_t value)
{
    return SkWStream::SizeOfPackedUInt(value);
}
