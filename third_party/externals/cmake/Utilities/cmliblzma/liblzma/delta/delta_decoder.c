///////////////////////////////////////////////////////////////////////////////
//
/// \file       delta_decoder.c
/// \brief      Delta filter decoder
//
//  Author:     Lasse Collin
//
//  This file has been put into the public domain.
//  You can do whatever you want with this file.
//
///////////////////////////////////////////////////////////////////////////////

#include "delta_decoder.h"
#include "delta_private.h"


static void
decode_buffer(lzma_coder *coder, uint8_t *buffer, size_t size)
{
	size_t i;
	const size_t distance = coder->distance;

	for (i = 0; i < size; ++i) {
		buffer[i] += coder->history[(distance + coder->pos) & 0xFF];
		coder->history[coder->pos-- & 0xFF] = buffer[i];
	}
}


static lzma_ret
delta_decode(lzma_coder *coder, lzma_allocator *allocator,
		const uint8_t *LZMA_RESTRICT in, size_t *LZMA_RESTRICT in_pos,
		size_t in_size, uint8_t *LZMA_RESTRICT out,
		size_t *LZMA_RESTRICT out_pos, size_t out_size, lzma_action action)
{
	const size_t out_start = *out_pos;
	lzma_ret ret;

	assert(coder->next.code != NULL);

	ret = coder->next.code(coder->next.coder, allocator,
			in, in_pos, in_size, out, out_pos, out_size,
			action);

	decode_buffer(coder, out + out_start, *out_pos - out_start);

	return ret;
}


extern lzma_ret
lzma_delta_decoder_init(lzma_next_coder *next, lzma_allocator *allocator,
		const lzma_filter_info *filters)
{
	next->code = &delta_decode;
	return lzma_delta_coder_init(next, allocator, filters);
}


extern lzma_ret
lzma_delta_props_decode(void **options, lzma_allocator *allocator,
		const uint8_t *props, size_t props_size)
{
	lzma_options_delta *opt;

	if (props_size != 1)
		return LZMA_OPTIONS_ERROR;

	opt = lzma_alloc(sizeof(lzma_options_delta), allocator);
	if (opt == NULL)
		return LZMA_MEM_ERROR;

	opt->type = LZMA_DELTA_TYPE_BYTE;
	opt->dist = props[0] + 1;

	*options = opt;

	return LZMA_OK;
}
