/*
 * Copyright 2015 Xamarin Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkStream.h"

#include "sk_stream.h"
#include "sk_types_priv.h"

sk_stream_filestream_t* sk_filestream_new (const char* path)
{
	return (sk_stream_filestream_t*)new SkFILEStream(path);
}

sk_stream_memorystream_t* sk_memorystream_new ()
{
	return (sk_stream_memorystream_t*)new SkMemoryStream();
}
sk_stream_memorystream_t* sk_memorystream_new_with_length (size_t length)
{
	return (sk_stream_memorystream_t*)new SkMemoryStream(length);
}
sk_stream_memorystream_t* sk_memorystream_new_with_data (const void* data, size_t length, bool copyData)
{
	return (sk_stream_memorystream_t*)new SkMemoryStream(data, length, copyData);
}
sk_stream_memorystream_t* sk_memorystream_new_with_skdata (sk_data_t* data)
{
	return (sk_stream_memorystream_t*)new SkMemoryStream(AsData(data));
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
