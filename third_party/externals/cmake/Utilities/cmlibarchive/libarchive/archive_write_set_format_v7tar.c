/*-
 * Copyright (c) 2003-2007 Tim Kientzle
 * Copyright (c) 2011-2012 Michihiro NAKAJIMA
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
__FBSDID("$FreeBSD$");


#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif
#include <stdio.h>
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif

#include "archive.h"
#include "archive_entry.h"
#include "archive_entry_locale.h"
#include "archive_private.h"
#include "archive_write_private.h"

struct v7tar {
	uint64_t	entry_bytes_remaining;
	uint64_t	entry_padding;

	struct archive_string_conv *opt_sconv;
	struct archive_string_conv *sconv_default;
	int	init_default_conversion;
};

/*
 * Define structure of POSIX 'v7tar' tar header.
 */
#define	V7TAR_name_offset 0
#define	V7TAR_name_size 100
#define	V7TAR_mode_offset 100
#define	V7TAR_mode_size 6
#define	V7TAR_mode_max_size 8
#define	V7TAR_uid_offset 108
#define	V7TAR_uid_size 6
#define	V7TAR_uid_max_size 8
#define	V7TAR_gid_offset 116
#define	V7TAR_gid_size 6
#define	V7TAR_gid_max_size 8
#define	V7TAR_size_offset 124
#define	V7TAR_size_size 11
#define	V7TAR_size_max_size 12
#define	V7TAR_mtime_offset 136
#define	V7TAR_mtime_size 11
#define	V7TAR_mtime_max_size 12
#define	V7TAR_checksum_offset 148
#define	V7TAR_checksum_size 8
#define	V7TAR_typeflag_offset 156
#define	V7TAR_typeflag_size 1
#define	V7TAR_linkname_offset 157
#define	V7TAR_linkname_size 100
#define	V7TAR_padding_offset 257
#define	V7TAR_padding_size 255

/*
 * A filled-in copy of the header for initialization.
 */
static const char template_header[] = {
	/* name: 100 bytes */
	0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
	0,0,0,0,
	/* Mode, space-null termination: 8 bytes */
	'0','0','0','0','0','0', ' ','\0',
	/* uid, space-null termination: 8 bytes */
	'0','0','0','0','0','0', ' ','\0',
	/* gid, space-null termination: 8 bytes */
	'0','0','0','0','0','0', ' ','\0',
	/* size, space termation: 12 bytes */
	'0','0','0','0','0','0','0','0','0','0','0', ' ',
	/* mtime, space termation: 12 bytes */
	'0','0','0','0','0','0','0','0','0','0','0', ' ',
	/* Initial checksum value: 8 spaces */
	' ',' ',' ',' ',' ',' ',' ',' ',
	/* Typeflag: 1 byte */
	0,
	/* Linkname: 100 bytes */
	0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
	0,0,0,0,
	/* Padding: 255 bytes */
	0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0
};

static ssize_t	archive_write_v7tar_data(struct archive_write *a, const void *buff,
		    size_t s);
static int	archive_write_v7tar_free(struct archive_write *);
static int	archive_write_v7tar_close(struct archive_write *);
static int	archive_write_v7tar_finish_entry(struct archive_write *);
static int	archive_write_v7tar_header(struct archive_write *,
		    struct archive_entry *entry);
static int	archive_write_v7tar_options(struct archive_write *,
		    const char *, const char *);
static int	format_256(int64_t, char *, int);
static int	format_number(int64_t, char *, int size, int max, int strict);
static int	format_octal(int64_t, char *, int);
static int	format_header_v7tar(struct archive_write *, char h[512],
		    struct archive_entry *, int, struct archive_string_conv *);

/*
 * Set output format to 'v7tar' format.
 */
int
archive_write_set_format_v7tar(struct archive *_a)
{
	struct archive_write *a = (struct archive_write *)_a;
	struct v7tar *v7tar;

	archive_check_magic(_a, ARCHIVE_WRITE_MAGIC,
	    ARCHIVE_STATE_NEW, "archive_write_set_format_v7tar");

	/* If someone else was already registered, unregister them. */
	if (a->format_free != NULL)
		(a->format_free)(a);

	/* Basic internal sanity test. */
	if (sizeof(template_header) != 512) {
		archive_set_error(&a->archive, ARCHIVE_ERRNO_MISC,
		    "Internal: template_header wrong size: %zu should be 512",
		    sizeof(template_header));
		return (ARCHIVE_FATAL);
	}

	v7tar = (struct v7tar *)malloc(sizeof(*v7tar));
	if (v7tar == NULL) {
		archive_set_error(&a->archive, ENOMEM,
		    "Can't allocate v7tar data");
		return (ARCHIVE_FATAL);
	}
	memset(v7tar, 0, sizeof(*v7tar));
	a->format_data = v7tar;
	a->format_name = "tar (non-POSIX)";
	a->format_options = archive_write_v7tar_options;
	a->format_write_header = archive_write_v7tar_header;
	a->format_write_data = archive_write_v7tar_data;
	a->format_close = archive_write_v7tar_close;
	a->format_free = archive_write_v7tar_free;
	a->format_finish_entry = archive_write_v7tar_finish_entry;
	a->archive.archive_format = ARCHIVE_FORMAT_TAR;
	a->archive.archive_format_name = "tar (non-POSIX)";
	return (ARCHIVE_OK);
}

static int
archive_write_v7tar_options(struct archive_write *a, const char *key,
    const char *val)
{
	struct v7tar *v7tar = (struct v7tar *)a->format_data;
	int ret = ARCHIVE_FAILED;

	if (strcmp(key, "hdrcharset")  == 0) {
		if (val == NULL || val[0] == 0)
			archive_set_error(&a->archive, ARCHIVE_ERRNO_MISC,
			    "%s: hdrcharset option needs a character-set name",
			    a->format_name);
		else {
			v7tar->opt_sconv = archive_string_conversion_to_charset(
			    &a->archive, val, 0);
			if (v7tar->opt_sconv != NULL)
				ret = ARCHIVE_OK;
			else
				ret = ARCHIVE_FATAL;
		}
		return (ret);
	}

	/* Note: The "warn" return is just to inform the options
	 * supervisor that we didn't handle it.  It will generate
	 * a suitable error if no one used this option. */
	return (ARCHIVE_WARN);
}

static int
archive_write_v7tar_header(struct archive_write *a, struct archive_entry *entry)
{
	char buff[512];
	int ret, ret2;
	struct v7tar *v7tar;
	struct archive_entry *entry_main;
	struct archive_string_conv *sconv;

	v7tar = (struct v7tar *)a->format_data;

	/* Setup default string conversion. */
	if (v7tar->opt_sconv == NULL) {
		if (!v7tar->init_default_conversion) {
			v7tar->sconv_default =
			    archive_string_default_conversion_for_write(
				&(a->archive));
			v7tar->init_default_conversion = 1;
		}
		sconv = v7tar->sconv_default;
	} else
		sconv = v7tar->opt_sconv;

	/* Sanity check. */
	if (archive_entry_pathname(entry) == NULL) {
		archive_set_error(&a->archive, ARCHIVE_ERRNO_MISC,
		    "Can't record entry in tar file without pathname");
		return (ARCHIVE_FAILED);
	}

	/* Only regular files (not hardlinks) have data. */
	if (archive_entry_hardlink(entry) != NULL ||
	    archive_entry_symlink(entry) != NULL ||
	    !(archive_entry_filetype(entry) == AE_IFREG))
		archive_entry_set_size(entry, 0);

	if (AE_IFDIR == archive_entry_filetype(entry)) {
		const char *p;
		size_t path_length;
		/*
		 * Ensure a trailing '/'.  Modify the entry so
		 * the client sees the change.
		 */
#if defined(_WIN32) && !defined(__CYGWIN__)
		const wchar_t *wp;

		wp = archive_entry_pathname_w(entry);
		if (wp != NULL && wp[wcslen(wp) -1] != L'/') {
			struct archive_wstring ws;

			archive_string_init(&ws);
			path_length = wcslen(wp);
			if (archive_wstring_ensure(&ws,
			    path_length + 2) == NULL) {
				archive_set_error(&a->archive, ENOMEM,
				    "Can't allocate v7tar data");
				archive_wstring_free(&ws);
				return(ARCHIVE_FATAL);
			}
			/* Should we keep '\' ? */
			if (wp[path_length -1] == L'\\')
				path_length--;
			archive_wstrncpy(&ws, wp, path_length);
			archive_wstrappend_wchar(&ws, L'/');
			archive_entry_copy_pathname_w(entry, ws.s);
			archive_wstring_free(&ws);
			p = NULL;
		} else
#endif
			p = archive_entry_pathname(entry);
		/*
		 * On Windows, this is a backup operation just in
		 * case getting WCS failed. On POSIX, this is a
		 * normal operation.
		 */
		if (p != NULL && p[strlen(p) - 1] != '/') {
			struct archive_string as;

			archive_string_init(&as);
			path_length = strlen(p);
			if (archive_string_ensure(&as,
			    path_length + 2) == NULL) {
				archive_set_error(&a->archive, ENOMEM,
				    "Can't allocate v7tar data");
				archive_string_free(&as);
				return(ARCHIVE_FATAL);
			}
#if defined(_WIN32) && !defined(__CYGWIN__)
			/* NOTE: This might break the pathname
			 * if the current code page is CP932 and
			 * the pathname includes a character '\'
			 * as a part of its multibyte pathname. */
			if (p[strlen(p) -1] == '\\')
				path_length--;
			else
#endif
			archive_strncpy(&as, p, path_length);
			archive_strappend_char(&as, '/');
			archive_entry_copy_pathname(entry, as.s);
			archive_string_free(&as);
		}
	}

#if defined(_WIN32) && !defined(__CYGWIN__)
	/* Make sure the path separators in pahtname, hardlink and symlink
	 * are all slash '/', not the Windows path separator '\'. */
	entry_main = __la_win_entry_in_posix_pathseparator(entry);
	if (entry_main == NULL) {
		archive_set_error(&a->archive, ENOMEM,
		    "Can't allocate v7tar data");
		return(ARCHIVE_FATAL);
	}
	if (entry != entry_main)
		entry = entry_main;
	else
		entry_main = NULL;
#else
	entry_main = NULL;
#endif
	ret = format_header_v7tar(a, buff, entry, 1, sconv);
	if (ret < ARCHIVE_WARN) {
		if (entry_main)
			archive_entry_free(entry_main);
		return (ret);
	}
	ret2 = __archive_write_output(a, buff, 512);
	if (ret2 < ARCHIVE_WARN) {
		if (entry_main)
			archive_entry_free(entry_main);
		return (ret2);
	}
	if (ret2 < ret)
		ret = ret2;

	v7tar->entry_bytes_remaining = archive_entry_size(entry);
	v7tar->entry_padding = 0x1ff & (-(int64_t)v7tar->entry_bytes_remaining);
	if (entry_main)
		archive_entry_free(entry_main);
	return (ret);
}

/*
 * Format a basic 512-byte "v7tar" header.
 *
 * Returns -1 if format failed (due to field overflow).
 * Note that this always formats as much of the header as possible.
 * If "strict" is set to zero, it will extend numeric fields as
 * necessary (overwriting terminators or using base-256 extensions).
 *
 */
static int
format_header_v7tar(struct archive_write *a, char h[512],
    struct archive_entry *entry, int strict,
    struct archive_string_conv *sconv)
{
	unsigned int checksum;
	int i, r, ret;
	size_t copy_length;
	const char *p, *pp;
	int mytartype;

	ret = 0;
	mytartype = -1;
	/*
	 * The "template header" already includes the "v7tar"
	 * signature, various end-of-field markers and other required
	 * elements.
	 */
	memcpy(h, &template_header, 512);

	/*
	 * Because the block is already null-filled, and strings
	 * are allowed to exactly fill their destination (without null),
	 * I use memcpy(dest, src, strlen()) here a lot to copy strings.
	 */
	r = archive_entry_pathname_l(entry, &pp, &copy_length, sconv);
	if (r != 0) {
		if (errno == ENOMEM) {
			archive_set_error(&a->archive, ENOMEM,
			    "Can't allocate memory for Pathname");
			return (ARCHIVE_FATAL);
		}
		archive_set_error(&a->archive, ARCHIVE_ERRNO_FILE_FORMAT,
		    "Can't translate pathname '%s' to %s",
		    pp, archive_string_conversion_charset_name(sconv));
		ret = ARCHIVE_WARN;
	}
	if (strict && copy_length < V7TAR_name_size)
		memcpy(h + V7TAR_name_offset, pp, copy_length);
	else if (!strict && copy_length <= V7TAR_name_size)
		memcpy(h + V7TAR_name_offset, pp, copy_length);
	else {
		/* Prefix is too long. */
		archive_set_error(&a->archive, ENAMETOOLONG,
		    "Pathname too long");
		ret = ARCHIVE_FAILED;
	}

	r = archive_entry_hardlink_l(entry, &p, &copy_length, sconv);
	if (r != 0) {
		if (errno == ENOMEM) {
			archive_set_error(&a->archive, ENOMEM,
			    "Can't allocate memory for Linkname");
			return (ARCHIVE_FATAL);
		}
		archive_set_error(&a->archive,
		    ARCHIVE_ERRNO_FILE_FORMAT,
		    "Can't translate linkname '%s' to %s",
		    p, archive_string_conversion_charset_name(sconv));
		ret = ARCHIVE_WARN;
	}
	if (copy_length > 0)
		mytartype = '1';
	else {
		r = archive_entry_symlink_l(entry, &p, &copy_length, sconv);
		if (r != 0) {
			if (errno == ENOMEM) {
				archive_set_error(&a->archive, ENOMEM,
				    "Can't allocate memory for Linkname");
				return (ARCHIVE_FATAL);
			}
			archive_set_error(&a->archive,
			    ARCHIVE_ERRNO_FILE_FORMAT,
			    "Can't translate linkname '%s' to %s",
			    p, archive_string_conversion_charset_name(sconv));
			ret = ARCHIVE_WARN;
		}
	}
	if (copy_length > 0) {
		if (copy_length >= V7TAR_linkname_size) {
			archive_set_error(&a->archive, ENAMETOOLONG,
			    "Link contents too long");
			ret = ARCHIVE_FAILED;
			copy_length = V7TAR_linkname_size;
		}
		memcpy(h + V7TAR_linkname_offset, p, copy_length);
	}

	if (format_number(archive_entry_mode(entry) & 07777,
	    h + V7TAR_mode_offset, V7TAR_mode_size,
	    V7TAR_mode_max_size, strict)) {
		archive_set_error(&a->archive, ERANGE,
		    "Numeric mode too large");
		ret = ARCHIVE_FAILED;
	}

	if (format_number(archive_entry_uid(entry),
	    h + V7TAR_uid_offset, V7TAR_uid_size, V7TAR_uid_max_size, strict)) {
		archive_set_error(&a->archive, ERANGE,
		    "Numeric user ID too large");
		ret = ARCHIVE_FAILED;
	}

	if (format_number(archive_entry_gid(entry),
	    h + V7TAR_gid_offset, V7TAR_gid_size, V7TAR_gid_max_size, strict)) {
		archive_set_error(&a->archive, ERANGE,
		    "Numeric group ID too large");
		ret = ARCHIVE_FAILED;
	}

	if (format_number(archive_entry_size(entry),
	    h + V7TAR_size_offset, V7TAR_size_size,
	    V7TAR_size_max_size, strict)) {
		archive_set_error(&a->archive, ERANGE,
		    "File size out of range");
		ret = ARCHIVE_FAILED;
	}

	if (format_number(archive_entry_mtime(entry),
	    h + V7TAR_mtime_offset, V7TAR_mtime_size,
	    V7TAR_mtime_max_size, strict)) {
		archive_set_error(&a->archive, ERANGE,
		    "File modification time too large");
		ret = ARCHIVE_FAILED;
	}

	if (mytartype >= 0) {
		h[V7TAR_typeflag_offset] = mytartype;
	} else {
		switch (archive_entry_filetype(entry)) {
		case AE_IFREG: case AE_IFDIR:
			break;
		case AE_IFLNK:
			h[V7TAR_typeflag_offset] = '2';
			break;
		case AE_IFCHR:
			archive_set_error(&a->archive,
			    ARCHIVE_ERRNO_FILE_FORMAT,
			    "tar format cannot archive character device");
			return (ARCHIVE_FAILED);
		case AE_IFBLK:
			archive_set_error(&a->archive,
			    ARCHIVE_ERRNO_FILE_FORMAT,
			    "tar format cannot archive block device");
			return (ARCHIVE_FAILED);
		case AE_IFIFO:
			archive_set_error(&a->archive,
			    ARCHIVE_ERRNO_FILE_FORMAT,
			    "tar format cannot archive fifo");
			return (ARCHIVE_FAILED);
		case AE_IFSOCK:
			archive_set_error(&a->archive,
			    ARCHIVE_ERRNO_FILE_FORMAT,
			    "tar format cannot archive socket");
			return (ARCHIVE_FAILED);
		default:
			archive_set_error(&a->archive,
			    ARCHIVE_ERRNO_FILE_FORMAT,
			    "tar format cannot archive this (mode=0%lo)",
			    (unsigned long)archive_entry_mode(entry));
			ret = ARCHIVE_FAILED;
		}
	}

	checksum = 0;
	for (i = 0; i < 512; i++)
		checksum += 255 & (unsigned int)h[i];
	format_octal(checksum, h + V7TAR_checksum_offset, 6);
	/* Can't be pre-set in the template. */
	h[V7TAR_checksum_offset + 6] = '\0';
	return (ret);
}

/*
 * Format a number into a field, with some intelligence.
 */
static int
format_number(int64_t v, char *p, int s, int maxsize, int strict)
{
	int64_t limit;

	limit = ((int64_t)1 << (s*3));

	/* "Strict" only permits octal values with proper termination. */
	if (strict)
		return (format_octal(v, p, s));

	/*
	 * In non-strict mode, we allow the number to overwrite one or
	 * more bytes of the field termination.  Even old tar
	 * implementations should be able to handle this with no
	 * problem.
	 */
	if (v >= 0) {
		while (s <= maxsize) {
			if (v < limit)
				return (format_octal(v, p, s));
			s++;
			limit <<= 3;
		}
	}

	/* Base-256 can handle any number, positive or negative. */
	return (format_256(v, p, maxsize));
}

/*
 * Format a number into the specified field using base-256.
 */
static int
format_256(int64_t v, char *p, int s)
{
	p += s;
	while (s-- > 0) {
		*--p = (char)(v & 0xff);
		v >>= 8;
	}
	*p |= 0x80; /* Set the base-256 marker bit. */
	return (0);
}

/*
 * Format a number into the specified field.
 */
static int
format_octal(int64_t v, char *p, int s)
{
	int len;

	len = s;

	/* Octal values can't be negative, so use 0. */
	if (v < 0) {
		while (len-- > 0)
			*p++ = '0';
		return (-1);
	}

	p += s;		/* Start at the end and work backwards. */
	while (s-- > 0) {
		*--p = (char)('0' + (v & 7));
		v >>= 3;
	}

	if (v == 0)
		return (0);

	/* If it overflowed, fill field with max value. */
	while (len-- > 0)
		*p++ = '7';

	return (-1);
}

static int
archive_write_v7tar_close(struct archive_write *a)
{
	return (__archive_write_nulls(a, 512*2));
}

static int
archive_write_v7tar_free(struct archive_write *a)
{
	struct v7tar *v7tar;

	v7tar = (struct v7tar *)a->format_data;
	free(v7tar);
	a->format_data = NULL;
	return (ARCHIVE_OK);
}

static int
archive_write_v7tar_finish_entry(struct archive_write *a)
{
	struct v7tar *v7tar;
	int ret;

	v7tar = (struct v7tar *)a->format_data;
	ret = __archive_write_nulls(a,
	    (size_t)(v7tar->entry_bytes_remaining + v7tar->entry_padding));
	v7tar->entry_bytes_remaining = v7tar->entry_padding = 0;
	return (ret);
}

static ssize_t
archive_write_v7tar_data(struct archive_write *a, const void *buff, size_t s)
{
	struct v7tar *v7tar;
	int ret;

	v7tar = (struct v7tar *)a->format_data;
	if (s > v7tar->entry_bytes_remaining)
		s = (size_t)v7tar->entry_bytes_remaining;
	ret = __archive_write_output(a, buff, s);
	v7tar->entry_bytes_remaining -= s;
	if (ret != ARCHIVE_OK)
		return (ret);
	return (s);
}
