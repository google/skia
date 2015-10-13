/*-
 * Copyright (c) 2003-2007 Tim Kientzle
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR(S) ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR(S) BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "archive_platform.h"

__FBSDID("$FreeBSD: head/lib/libarchive/archive_write_set_compression_gzip.c 201081 2009-12-28 02:04:42Z kientzle $");

#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif
#include <time.h>
#ifdef HAVE_ZLIB_H
#include <cm_zlib.h>
#endif

#include "archive.h"
#include "archive_private.h"
#include "archive_string.h"
#include "archive_write_private.h"

#if ARCHIVE_VERSION_NUMBER < 4000000
int
archive_write_set_compression_gzip(struct archive *a)
{
	__archive_write_filters_free(a);
	return (archive_write_add_filter_gzip(a));
}
#endif

/* Don't compile this if we don't have zlib. */

struct private_data {
	int		 compression_level;
	int		 timestamp;
#ifdef HAVE_ZLIB_H
	z_stream	 stream;
	int64_t		 total_in;
	unsigned char	*compressed;
	size_t		 compressed_buffer_size;
	unsigned long	 crc;
#else
	struct archive_write_program_data *pdata;
#endif
};

/*
 * Yuck.  zlib.h is not const-correct, so I need this one bit
 * of ugly hackery to convert a const * pointer to a non-const pointer.
 */
#define	SET_NEXT_IN(st,src)					\
	(st)->stream.next_in = (Bytef *)(uintptr_t)(const void *)(src)

static int archive_compressor_gzip_options(struct archive_write_filter *,
		    const char *, const char *);
static int archive_compressor_gzip_open(struct archive_write_filter *);
static int archive_compressor_gzip_write(struct archive_write_filter *,
		    const void *, size_t);
static int archive_compressor_gzip_close(struct archive_write_filter *);
static int archive_compressor_gzip_free(struct archive_write_filter *);
#ifdef HAVE_ZLIB_H
static int drive_compressor(struct archive_write_filter *,
		    struct private_data *, int finishing);
#endif


/*
 * Add a gzip compression filter to this write handle.
 */
int
archive_write_add_filter_gzip(struct archive *_a)
{
	struct archive_write *a = (struct archive_write *)_a;
	struct archive_write_filter *f = __archive_write_allocate_filter(_a);
	struct private_data *data;
	archive_check_magic(&a->archive, ARCHIVE_WRITE_MAGIC,
	    ARCHIVE_STATE_NEW, "archive_write_add_filter_gzip");

	data = calloc(1, sizeof(*data));
	if (data == NULL) {
		archive_set_error(&a->archive, ENOMEM, "Out of memory");
		return (ARCHIVE_FATAL);
	}
	f->data = data;
	f->open = &archive_compressor_gzip_open;
	f->options = &archive_compressor_gzip_options;
	f->close = &archive_compressor_gzip_close;
	f->free = &archive_compressor_gzip_free;
	f->code = ARCHIVE_FILTER_GZIP;
	f->name = "gzip";
#ifdef HAVE_ZLIB_H
	data->compression_level = Z_DEFAULT_COMPRESSION;
	return (ARCHIVE_OK);
#else
	data->pdata = __archive_write_program_allocate();
	if (data->pdata == NULL) {
		free(data);
		archive_set_error(&a->archive, ENOMEM, "Out of memory");
		return (ARCHIVE_FATAL);
	}
	data->compression_level = 0;
	archive_set_error(&a->archive, ARCHIVE_ERRNO_MISC,
	    "Using external gzip program");
	return (ARCHIVE_WARN);
#endif
}

static int
archive_compressor_gzip_free(struct archive_write_filter *f)
{
	struct private_data *data = (struct private_data *)f->data;

#ifdef HAVE_ZLIB_H
	free(data->compressed);
#else
	__archive_write_program_free(data->pdata);
#endif
	free(data);
	f->data = NULL;
	return (ARCHIVE_OK);
}

/*
 * Set write options.
 */
static int
archive_compressor_gzip_options(struct archive_write_filter *f, const char *key,
    const char *value)
{
	struct private_data *data = (struct private_data *)f->data;

	if (strcmp(key, "compression-level") == 0) {
		if (value == NULL || !(value[0] >= '0' && value[0] <= '9') ||
		    value[1] != '\0')
			return (ARCHIVE_WARN);
		data->compression_level = value[0] - '0';
		return (ARCHIVE_OK);
	}
	if (strcmp(key, "timestamp") == 0) {
		data->timestamp = (value == NULL)?-1:1;
		return (ARCHIVE_OK);
	}

	/* Note: The "warn" return is just to inform the options
	 * supervisor that we didn't handle it.  It will generate
	 * a suitable error if no one used this option. */
	return (ARCHIVE_WARN);
}

#ifdef HAVE_ZLIB_H
/*
 * Setup callback.
 */
static int
archive_compressor_gzip_open(struct archive_write_filter *f)
{
	struct private_data *data = (struct private_data *)f->data;
	int ret;

	ret = __archive_write_open_filter(f->next_filter);
	if (ret != ARCHIVE_OK)
		return (ret);

	if (data->compressed == NULL) {
		size_t bs = 65536, bpb;
		if (f->archive->magic == ARCHIVE_WRITE_MAGIC) {
			/* Buffer size should be a multiple number of
			 * the of bytes per block for performance. */
			bpb = archive_write_get_bytes_per_block(f->archive);
			if (bpb > bs)
				bs = bpb;
			else if (bpb != 0)
				bs -= bs % bpb;
		}
		data->compressed_buffer_size = bs;
		data->compressed
		    = (unsigned char *)malloc(data->compressed_buffer_size);
		if (data->compressed == NULL) {
			archive_set_error(f->archive, ENOMEM,
			    "Can't allocate data for compression buffer");
			return (ARCHIVE_FATAL);
		}
	}

	data->crc = crc32(0L, NULL, 0);
	data->stream.next_out = data->compressed;
	data->stream.avail_out = (uInt)data->compressed_buffer_size;

	/* Prime output buffer with a gzip header. */
	data->compressed[0] = 0x1f; /* GZip signature bytes */
	data->compressed[1] = 0x8b;
	data->compressed[2] = 0x08; /* "Deflate" compression */
	data->compressed[3] = 0; /* No options */
	if (data->timestamp >= 0) {
		time_t t = time(NULL);
		data->compressed[4] = (uint8_t)(t)&0xff;  /* Timestamp */
		data->compressed[5] = (uint8_t)(t>>8)&0xff;
		data->compressed[6] = (uint8_t)(t>>16)&0xff;
		data->compressed[7] = (uint8_t)(t>>24)&0xff;
	} else
		memset(&data->compressed[4], 0, 4);
	data->compressed[8] = 0; /* No deflate options */
	data->compressed[9] = 3; /* OS=Unix */
	data->stream.next_out += 10;
	data->stream.avail_out -= 10;

	f->write = archive_compressor_gzip_write;

	/* Initialize compression library. */
	ret = deflateInit2(&(data->stream),
	    data->compression_level,
	    Z_DEFLATED,
	    -15 /* < 0 to suppress zlib header */,
	    8,
	    Z_DEFAULT_STRATEGY);

	if (ret == Z_OK) {
		f->data = data;
		return (ARCHIVE_OK);
	}

	/* Library setup failed: clean up. */
	archive_set_error(f->archive, ARCHIVE_ERRNO_MISC, "Internal error "
	    "initializing compression library");

	/* Override the error message if we know what really went wrong. */
	switch (ret) {
	case Z_STREAM_ERROR:
		archive_set_error(f->archive, ARCHIVE_ERRNO_MISC,
		    "Internal error initializing "
		    "compression library: invalid setup parameter");
		break;
	case Z_MEM_ERROR:
		archive_set_error(f->archive, ENOMEM,
		    "Internal error initializing compression library");
		break;
	case Z_VERSION_ERROR:
		archive_set_error(f->archive, ARCHIVE_ERRNO_MISC,
		    "Internal error initializing "
		    "compression library: invalid library version");
		break;
	}

	return (ARCHIVE_FATAL);
}

/*
 * Write data to the compressed stream.
 */
static int
archive_compressor_gzip_write(struct archive_write_filter *f, const void *buff,
    size_t length)
{
	struct private_data *data = (struct private_data *)f->data;
	int ret;

	/* Update statistics */
	data->crc = crc32(data->crc, (const Bytef *)buff, (uInt)length);
	data->total_in += length;

	/* Compress input data to output buffer */
	SET_NEXT_IN(data, buff);
	data->stream.avail_in = (uInt)length;
	if ((ret = drive_compressor(f, data, 0)) != ARCHIVE_OK)
		return (ret);

	return (ARCHIVE_OK);
}

/*
 * Finish the compression...
 */
static int
archive_compressor_gzip_close(struct archive_write_filter *f)
{
	unsigned char trailer[8];
	struct private_data *data = (struct private_data *)f->data;
	int ret, r1;

	/* Finish compression cycle */
	ret = drive_compressor(f, data, 1);
	if (ret == ARCHIVE_OK) {
		/* Write the last compressed data. */
		ret = __archive_write_filter(f->next_filter,
		    data->compressed,
		    data->compressed_buffer_size - data->stream.avail_out);
	}
	if (ret == ARCHIVE_OK) {
		/* Build and write out 8-byte trailer. */
		trailer[0] = (uint8_t)(data->crc)&0xff;
		trailer[1] = (uint8_t)(data->crc >> 8)&0xff;
		trailer[2] = (uint8_t)(data->crc >> 16)&0xff;
		trailer[3] = (uint8_t)(data->crc >> 24)&0xff;
		trailer[4] = (uint8_t)(data->total_in)&0xff;
		trailer[5] = (uint8_t)(data->total_in >> 8)&0xff;
		trailer[6] = (uint8_t)(data->total_in >> 16)&0xff;
		trailer[7] = (uint8_t)(data->total_in >> 24)&0xff;
		ret = __archive_write_filter(f->next_filter, trailer, 8);
	}

	switch (deflateEnd(&(data->stream))) {
	case Z_OK:
		break;
	default:
		archive_set_error(f->archive, ARCHIVE_ERRNO_MISC,
		    "Failed to clean up compressor");
		ret = ARCHIVE_FATAL;
	}
	r1 = __archive_write_close_filter(f->next_filter);
	return (r1 < ret ? r1 : ret);
}

/*
 * Utility function to push input data through compressor,
 * writing full output blocks as necessary.
 *
 * Note that this handles both the regular write case (finishing ==
 * false) and the end-of-archive case (finishing == true).
 */
static int
drive_compressor(struct archive_write_filter *f,
    struct private_data *data, int finishing)
{
	int ret;

	for (;;) {
		if (data->stream.avail_out == 0) {
			ret = __archive_write_filter(f->next_filter,
			    data->compressed,
			    data->compressed_buffer_size);
			if (ret != ARCHIVE_OK)
				return (ARCHIVE_FATAL);
			data->stream.next_out = data->compressed;
			data->stream.avail_out =
			    (uInt)data->compressed_buffer_size;
		}

		/* If there's nothing to do, we're done. */
		if (!finishing && data->stream.avail_in == 0)
			return (ARCHIVE_OK);

		ret = deflate(&(data->stream),
		    finishing ? Z_FINISH : Z_NO_FLUSH );

		switch (ret) {
		case Z_OK:
			/* In non-finishing case, check if compressor
			 * consumed everything */
			if (!finishing && data->stream.avail_in == 0)
				return (ARCHIVE_OK);
			/* In finishing case, this return always means
			 * there's more work */
			break;
		case Z_STREAM_END:
			/* This return can only occur in finishing case. */
			return (ARCHIVE_OK);
		default:
			/* Any other return value indicates an error. */
			archive_set_error(f->archive, ARCHIVE_ERRNO_MISC,
			    "GZip compression failed:"
			    " deflate() call returned status %d",
			    ret);
			return (ARCHIVE_FATAL);
		}
	}
}

#else /* HAVE_ZLIB_H */

static int
archive_compressor_gzip_open(struct archive_write_filter *f)
{
	struct private_data *data = (struct private_data *)f->data;
	struct archive_string as;
	int r;

	archive_string_init(&as);
	archive_strcpy(&as, "gzip");

	/* Specify compression level. */
	if (data->compression_level > 0) {
		archive_strcat(&as, " -");
		archive_strappend_char(&as, '0' + data->compression_level);
	}
	if (data->timestamp < 0)
		/* Do not save timestamp. */
		archive_strcat(&as, " -n");
	else if (data->timestamp > 0)
		/* Save timestamp. */
		archive_strcat(&as, " -N");

	f->write = archive_compressor_gzip_write;
	r = __archive_write_program_open(f, data->pdata, as.s);
	archive_string_free(&as);
	return (r);
}

static int
archive_compressor_gzip_write(struct archive_write_filter *f, const void *buff,
    size_t length)
{
	struct private_data *data = (struct private_data *)f->data;

	return __archive_write_program_write(f, data->pdata, buff, length);
}

static int
archive_compressor_gzip_close(struct archive_write_filter *f)
{
	struct private_data *data = (struct private_data *)f->data;

	return __archive_write_program_close(f, data->pdata);
}

#endif /* HAVE_ZLIB_H */
