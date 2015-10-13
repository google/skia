/*-
 * Copyright (c) 2008 Anselm Strauss
 * Copyright (c) 2009 Joerg Sonnenberger
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

/*
 * Development supported by Google Summer of Code 2008.
 */

#include "archive_platform.h"
__FBSDID("$FreeBSD: head/lib/libarchive/archive_write_set_format_zip.c 201168 2009-12-29 06:15:32Z kientzle $");

#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif
#ifdef HAVE_LANGINFO_H
#include <langinfo.h>
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif
#ifdef HAVE_ZLIB_H
#include <cm_zlib.h>
#endif

#include "archive.h"
#include "archive_endian.h"
#include "archive_entry.h"
#include "archive_entry_locale.h"
#include "archive_private.h"
#include "archive_write_private.h"

#ifndef HAVE_ZLIB_H
#include "archive_crc32.h"
#endif

#define ZIP_ENTRY_FLAG_LENGTH_AT_END	(1<<3)
#define ZIP_ENTRY_FLAG_UTF8_NAME	(1 << 11)


enum compression {
	COMPRESSION_UNSPECIFIED = -1,
	COMPRESSION_STORE = 0,
	COMPRESSION_DEFLATE = 8
};

#ifdef HAVE_ZLIB_H
#define COMPRESSION_DEFAULT	COMPRESSION_DEFLATE
#else
#define COMPRESSION_DEFAULT	COMPRESSION_STORE
#endif

struct cd_segment {
	struct cd_segment *next;
	size_t buff_size;
	unsigned char *buff;
	unsigned char *p;
};

/* Bits used to enable/disable certain experimental features. */
#define	EXPERIMENT_LA 1
#define	EXPERIMENTS_ALL 0xffff

struct zip {

	int64_t entry_offset;
	int64_t entry_compressed_size;
	int64_t entry_uncompressed_size;
	int64_t entry_compressed_written;
	int64_t entry_uncompressed_written;
	int64_t entry_uncompressed_limit;
	struct archive_entry *entry;
	uint32_t entry_crc32;
	enum compression entry_compression;
	int entry_flags;
	int entry_uses_zip64;
	int experiments;

	unsigned char *file_header;
	size_t file_header_extra_offset;
	unsigned long (*crc32func)(unsigned long crc, const void *buff, size_t len);

	struct cd_segment *central_directory;
	struct cd_segment *central_directory_last;
	size_t central_directory_bytes;
	size_t central_directory_entries;

	int64_t written_bytes; /* Overall position in file. */

	struct archive_string_conv *opt_sconv;
	struct archive_string_conv *sconv_default;
	enum compression requested_compression;
	int init_default_conversion;

#define ZIP_FLAG_AVOID_ZIP64 1
#define ZIP_FLAG_FORCE_ZIP64 2
#define ZIP_FLAG_EXPERIMENT_EL 4
	int flags;

#ifdef HAVE_ZLIB_H
	z_stream stream;
	size_t len_buf;
	unsigned char *buf;
#endif
};

/* Don't call this min or MIN, since those are already defined
   on lots of platforms (but not all). */
#define zipmin(a, b) ((a) > (b) ? (b) : (a))

static ssize_t archive_write_zip_data(struct archive_write *,
		   const void *buff, size_t s);
static int archive_write_zip_close(struct archive_write *);
static int archive_write_zip_free(struct archive_write *);
static int archive_write_zip_finish_entry(struct archive_write *);
static int archive_write_zip_header(struct archive_write *,
	      struct archive_entry *);
static int archive_write_zip_options(struct archive_write *,
	      const char *, const char *);
static unsigned int dos_time(const time_t);
static size_t path_length(struct archive_entry *);
static int write_path(struct archive_entry *, struct archive_write *);
static void copy_path(struct archive_entry *, unsigned char *);
static struct archive_string_conv *get_sconv(struct archive_write *, struct zip *);

static unsigned char *
cd_alloc(struct zip *zip, size_t length)
{
	unsigned char *p;

	if (zip->central_directory == NULL
	    || (zip->central_directory_last->p + length
		> zip->central_directory_last->buff + zip->central_directory_last->buff_size)) {
		struct cd_segment *segment = calloc(1, sizeof(*segment));
		if (segment == NULL)
			return NULL;
		segment->buff_size = 64 * 1024;
		segment->buff = malloc(segment->buff_size);
		if (segment->buff == NULL) {
			free(segment);
			return NULL;
		}
		segment->p = segment->buff;

		if (zip->central_directory == NULL) {
			zip->central_directory
			    = zip->central_directory_last
			    = segment;
		} else {
			zip->central_directory_last->next = segment;
			zip->central_directory_last = segment;
		}
	}

	p = zip->central_directory_last->p;
	zip->central_directory_last->p += length;
	zip->central_directory_bytes += length;
	return (p);
}

static unsigned long
real_crc32(unsigned long crc, const void *buff, size_t len)
{
	return crc32(crc, buff, len);
}

static unsigned long
fake_crc32(unsigned long crc, const void *buff, size_t len)
{
	(void)crc; /* UNUSED */
	(void)buff; /* UNUSED */
	(void)len; /* UNUSED */
	return 0;
}

static int
archive_write_zip_options(struct archive_write *a, const char *key,
    const char *val)
{
	struct zip *zip = a->format_data;
	int ret = ARCHIVE_FAILED;

	if (strcmp(key, "compression") == 0) {
		/*
		 * Set compression to use on all future entries.
		 * This only affects regular files.
		 */
		if (val == NULL || val[0] == 0) {
			archive_set_error(&a->archive, ARCHIVE_ERRNO_MISC,
			    "%s: compression option needs a compression name",
			    a->format_name);
		} else if (strcmp(val, "deflate") == 0) {
#ifdef HAVE_ZLIB_H
			zip->requested_compression = COMPRESSION_DEFLATE;
			ret = ARCHIVE_OK;
#else
			archive_set_error(&a->archive, ARCHIVE_ERRNO_MISC,
			    "deflate compression not supported");
#endif
		} else if (strcmp(val, "store") == 0) {
			zip->requested_compression = COMPRESSION_STORE;
			ret = ARCHIVE_OK;
		}
		return (ret);
	} else if (strcmp(key, "experimental") == 0) {
		if (val == NULL || val[0] == 0) {
			zip->flags &= ~ ZIP_FLAG_EXPERIMENT_EL;
		} else {
			zip->flags |= ZIP_FLAG_EXPERIMENT_EL;
		}
		return (ARCHIVE_OK);
	} else if (strcmp(key, "fakecrc32") == 0) {
		/*
		 * FOR TESTING ONLY:  disable CRC calculation to speed up
		 * certain complex tests.
		 */
		if (val == NULL || val[0] == 0) {
			zip->crc32func = real_crc32;
		} else {
			zip->crc32func = fake_crc32;
		}
		return (ARCHIVE_OK);
	} else if (strcmp(key, "hdrcharset")  == 0) {
		/*
		 * Set the character set used in translating filenames.
		 */
		if (val == NULL || val[0] == 0) {
			archive_set_error(&a->archive, ARCHIVE_ERRNO_MISC,
			    "%s: hdrcharset option needs a character-set name",
			    a->format_name);
		} else {
			zip->opt_sconv = archive_string_conversion_to_charset(
			    &a->archive, val, 0);
			if (zip->opt_sconv != NULL)
				ret = ARCHIVE_OK;
			else
				ret = ARCHIVE_FATAL;
		}
		return (ret);
	} else if (strcmp(key, "zip64") == 0) {
		/*
		 * Bias decisions about Zip64: force them to be
		 * generated in certain cases where they are not
		 * forbidden or avoid them in certain cases where they
		 * are not strictly required.
		 */
		if (val != NULL && *val != '\0') {
			zip->flags |= ZIP_FLAG_FORCE_ZIP64;
			zip->flags &= ~ZIP_FLAG_AVOID_ZIP64;
		} else {
			zip->flags &= ~ZIP_FLAG_FORCE_ZIP64;
			zip->flags |= ZIP_FLAG_AVOID_ZIP64;
		}
		return (ARCHIVE_OK);
	}

	/* Note: The "warn" return is just to inform the options
	 * supervisor that we didn't handle it.  It will generate
	 * a suitable error if no one used this option. */
	return (ARCHIVE_WARN);
}

int
archive_write_zip_set_compression_deflate(struct archive *_a)
{
	struct archive_write *a = (struct archive_write *)_a;
	int ret = ARCHIVE_FAILED;

	archive_check_magic(_a, ARCHIVE_WRITE_MAGIC,
		ARCHIVE_STATE_NEW | ARCHIVE_STATE_HEADER | ARCHIVE_STATE_DATA,
		"archive_write_zip_set_compression_deflate");
	if (a->archive.archive_format != ARCHIVE_FORMAT_ZIP) {
		archive_set_error(&a->archive, ARCHIVE_ERRNO_MISC,
		"Can only use archive_write_zip_set_compression_deflate"
		" with zip format");
		ret = ARCHIVE_FATAL;
	} else {
#ifdef HAVE_ZLIB_H
		struct zip *zip = a->format_data;
		zip->requested_compression = COMPRESSION_DEFLATE;
		ret = ARCHIVE_OK;
#else
		archive_set_error(&a->archive, ARCHIVE_ERRNO_MISC,
			"deflate compression not supported");
		ret = ARCHIVE_FAILED;
#endif
	}
	return (ret);
}

int
archive_write_zip_set_compression_store(struct archive *_a)
{
	struct archive_write *a = (struct archive_write *)_a;
	struct zip *zip = a->format_data;
	int ret = ARCHIVE_FAILED;

	archive_check_magic(_a, ARCHIVE_WRITE_MAGIC,
		ARCHIVE_STATE_NEW | ARCHIVE_STATE_HEADER | ARCHIVE_STATE_DATA,
		"archive_write_zip_set_compression_deflate");
	if (a->archive.archive_format != ARCHIVE_FORMAT_ZIP) {
		archive_set_error(&a->archive, ARCHIVE_ERRNO_MISC,
			"Can only use archive_write_zip_set_compression_store"
			" with zip format");
		ret = ARCHIVE_FATAL;
	} else {
		zip->requested_compression = COMPRESSION_STORE;
		ret = ARCHIVE_OK;
	}
	return (ret);
}

int
archive_write_set_format_zip(struct archive *_a)
{
	struct archive_write *a = (struct archive_write *)_a;
	struct zip *zip;

	archive_check_magic(_a, ARCHIVE_WRITE_MAGIC,
	    ARCHIVE_STATE_NEW, "archive_write_set_format_zip");

	/* If another format was already registered, unregister it. */
	if (a->format_free != NULL)
		(a->format_free)(a);

	zip = (struct zip *) calloc(1, sizeof(*zip));
	if (zip == NULL) {
		archive_set_error(&a->archive, ENOMEM,
		    "Can't allocate zip data");
		return (ARCHIVE_FATAL);
	}

	/* "Unspecified" lets us choose the appropriate compression. */
	zip->requested_compression = COMPRESSION_UNSPECIFIED;
	zip->crc32func = real_crc32;

#ifdef HAVE_ZLIB_H
	zip->len_buf = 65536;
	zip->buf = malloc(zip->len_buf);
	if (zip->buf == NULL) {
		free(zip);
		archive_set_error(&a->archive, ENOMEM,
		    "Can't allocate compression buffer");
		return (ARCHIVE_FATAL);
	}
#endif

	a->format_data = zip;
	a->format_name = "zip";
	a->format_options = archive_write_zip_options;
	a->format_write_header = archive_write_zip_header;
	a->format_write_data = archive_write_zip_data;
	a->format_finish_entry = archive_write_zip_finish_entry;
	a->format_close = archive_write_zip_close;
	a->format_free = archive_write_zip_free;
	a->archive.archive_format = ARCHIVE_FORMAT_ZIP;
	a->archive.archive_format_name = "ZIP";

	return (ARCHIVE_OK);
}

static int
is_all_ascii(const char *p)
{
	const unsigned char *pp = (const unsigned char *)p;

	while (*pp) {
		if (*pp++ > 127)
			return (0);
	}
	return (1);
}

static int
archive_write_zip_header(struct archive_write *a, struct archive_entry *entry)
{
	unsigned char local_header[32];
	unsigned char local_extra[128];
	struct zip *zip = a->format_data;
	unsigned char *e;
	unsigned char *cd_extra;
	size_t filename_length;
	const char *slink = NULL;
	size_t slink_size = 0;
	struct archive_string_conv *sconv = get_sconv(a, zip);
	int ret, ret2 = ARCHIVE_OK;
	int64_t size;
	mode_t type;
	int version_needed = 10;

	/* Ignore types of entries that we don't support. */
	type = archive_entry_filetype(entry);
	if (type != AE_IFREG && type != AE_IFDIR && type != AE_IFLNK) {
		archive_set_error(&a->archive, ARCHIVE_ERRNO_MISC,
		    "Filetype not supported");
		return ARCHIVE_FAILED;
	};

	/* If we're not using Zip64, reject large files. */
	if (zip->flags & ZIP_FLAG_AVOID_ZIP64) {
		/* Reject entries over 4GB. */
		if (archive_entry_size_is_set(entry)
		    && (archive_entry_size(entry) > 0xffffffff)) {
			archive_set_error(&a->archive, ARCHIVE_ERRNO_MISC,
			    "Files > 4GB require Zip64 extensions");
			return ARCHIVE_FAILED;
		}
		/* Reject entries if archive is > 4GB. */
		if (zip->written_bytes > 0xffffffff) {
			archive_set_error(&a->archive, ARCHIVE_ERRNO_MISC,
			    "Archives > 4GB require Zip64 extensions");
			return ARCHIVE_FAILED;
		}
	}

	/* Only regular files can have size > 0. */
	if (type != AE_IFREG)
		archive_entry_set_size(entry, 0);


	/* Reset information from last entry. */
	zip->entry_offset = zip->written_bytes;
	zip->entry_uncompressed_limit = INT64_MAX;
	zip->entry_compressed_size = 0;
	zip->entry_uncompressed_size = 0;
	zip->entry_compressed_written = 0;
	zip->entry_uncompressed_written = 0;
	zip->entry_flags = 0;
	zip->entry_uses_zip64 = 0;
	zip->entry_crc32 = zip->crc32func(0, NULL, 0);
	if (zip->entry != NULL) {
		archive_entry_free(zip->entry);
		zip->entry = NULL;
	}

#if defined(_WIN32) && !defined(__CYGWIN__)
	/* Make sure the path separators in pahtname, hardlink and symlink
	 * are all slash '/', not the Windows path separator '\'. */
	zip->entry = __la_win_entry_in_posix_pathseparator(entry);
	if (zip->entry == entry)
		zip->entry = archive_entry_clone(entry);
#else
	zip->entry = archive_entry_clone(entry);
#endif
	if (zip->entry == NULL) {
		archive_set_error(&a->archive, ENOMEM,
		    "Can't allocate zip header data");
		return (ARCHIVE_FATAL);
	}

	if (sconv != NULL) {
		const char *p;
		size_t len;

		if (archive_entry_pathname_l(entry, &p, &len, sconv) != 0) {
			if (errno == ENOMEM) {
				archive_set_error(&a->archive, ENOMEM,
				    "Can't allocate memory for Pathname");
				return (ARCHIVE_FATAL);
			}
			archive_set_error(&a->archive,
			    ARCHIVE_ERRNO_FILE_FORMAT,
			    "Can't translate Pathname '%s' to %s",
			    archive_entry_pathname(entry),
			    archive_string_conversion_charset_name(sconv));
			ret2 = ARCHIVE_WARN;
		}
		if (len > 0)
			archive_entry_set_pathname(zip->entry, p);

		/*
		 * There is no standard for symlink handling; we convert
		 * it using the same character-set translation that we use
		 * for filename.
		 */
		if (type == AE_IFLNK) {
			if (archive_entry_symlink_l(entry, &p, &len, sconv)) {
				if (errno == ENOMEM) {
					archive_set_error(&a->archive, ENOMEM,
					    "Can't allocate memory "
					    " for Symlink");
					return (ARCHIVE_FATAL);
				}
				/* No error if we can't convert. */
			} else if (len > 0)
				archive_entry_set_symlink(zip->entry, p);
		}
	}

	/* If filename isn't ASCII and we can use UTF-8, set the UTF-8 flag. */
	if (!is_all_ascii(archive_entry_pathname(zip->entry))) {
		if (zip->opt_sconv != NULL) {
			if (strcmp(archive_string_conversion_charset_name(
					zip->opt_sconv), "UTF-8") == 0)
				zip->entry_flags |= ZIP_ENTRY_FLAG_UTF8_NAME;
#if HAVE_NL_LANGINFO
		} else if (strcmp(nl_langinfo(CODESET), "UTF-8") == 0) {
			zip->entry_flags |= ZIP_ENTRY_FLAG_UTF8_NAME;
#endif
		}
	}
	filename_length = path_length(zip->entry);

	/* Determine appropriate compression and size for this entry. */
	if (type == AE_IFLNK) {
		slink = archive_entry_symlink(zip->entry);
		if (slink != NULL)
			slink_size = strlen(slink);
		else
			slink_size = 0;
		zip->entry_uncompressed_limit = slink_size;
		zip->entry_compressed_size = slink_size;
		zip->entry_uncompressed_size = slink_size;
		zip->entry_crc32 = zip->crc32func(zip->entry_crc32,
		    (const unsigned char *)slink, slink_size);
		zip->entry_compression = COMPRESSION_STORE;
		version_needed = 20;
	} else if (type != AE_IFREG) {
		zip->entry_compression = COMPRESSION_STORE;
		zip->entry_uncompressed_limit = 0;
		size = 0;
		version_needed = 20;
	} else if (archive_entry_size_is_set(zip->entry)) {
		size = archive_entry_size(zip->entry);
		zip->entry_uncompressed_limit = size;
		zip->entry_compression = zip->requested_compression;
		if (zip->entry_compression == COMPRESSION_UNSPECIFIED) {
			zip->entry_compression = COMPRESSION_DEFAULT;
		}
		if (zip->entry_compression == COMPRESSION_STORE) {
			zip->entry_compressed_size = size;
			zip->entry_uncompressed_size = size;
			version_needed = 10;
		} else {
			zip->entry_uncompressed_size = size;
			version_needed = 20;
		}
		if ((zip->flags & ZIP_FLAG_FORCE_ZIP64) /* User asked. */
		    || (zip->entry_uncompressed_size > ARCHIVE_LITERAL_LL(0xffffffff))) { /* Large entry. */
			zip->entry_uses_zip64 = 1;
			version_needed = 45;
		}

		/* We may know the size, but never the CRC. */
		zip->entry_flags |= ZIP_ENTRY_FLAG_LENGTH_AT_END;
	} else {
		/* Prefer deflate if it's available, because deflate
		 * has a clear end-of-data marker that makes
		 * length-at-end more reliable. */
		zip->entry_compression = COMPRESSION_DEFAULT;
		zip->entry_flags |= ZIP_ENTRY_FLAG_LENGTH_AT_END;
		if ((zip->flags & ZIP_FLAG_AVOID_ZIP64) == 0) {
			zip->entry_uses_zip64 = 1;
			version_needed = 45;
		} else if (zip->entry_compression == COMPRESSION_STORE) {
			version_needed = 10;
		} else {
			version_needed = 20;
		}
	}

	/* Format the local header. */
	memset(local_header, 0, sizeof(local_header));
	memcpy(local_header, "PK\003\004", 4);
	archive_le16enc(local_header + 4, version_needed);
	archive_le16enc(local_header + 6, zip->entry_flags);
	archive_le16enc(local_header + 8, zip->entry_compression);
	archive_le32enc(local_header + 10, dos_time(archive_entry_mtime(zip->entry)));
	archive_le32enc(local_header + 14, zip->entry_crc32);
	if (zip->entry_uses_zip64) {
		/* Zip64 data in the local header "must" include both
		 * compressed and uncompressed sizes AND those fields
		 * are included only if these are 0xffffffff;
		 * THEREFORE these must be set this way, even if we
		 * know one of them is smaller. */
		archive_le32enc(local_header + 18, ARCHIVE_LITERAL_LL(0xffffffff));
		archive_le32enc(local_header + 22, ARCHIVE_LITERAL_LL(0xffffffff));
	} else {
		archive_le32enc(local_header + 18, zip->entry_compressed_size);
		archive_le32enc(local_header + 22, zip->entry_uncompressed_size);
	}
	archive_le16enc(local_header + 26, filename_length);

	/* Format as much of central directory file header as we can: */
	zip->file_header = cd_alloc(zip, 46);
	/* If (zip->file_header == NULL) XXXX */
	++zip->central_directory_entries;
	memset(zip->file_header, 0, 46);
	memcpy(zip->file_header, "PK\001\002", 4);
	/* "Made by PKZip 2.0 on Unix." */
	archive_le16enc(zip->file_header + 4, 3 * 256 + version_needed);
	archive_le16enc(zip->file_header + 6, version_needed);
	archive_le16enc(zip->file_header + 8, zip->entry_flags);
	archive_le16enc(zip->file_header + 10, zip->entry_compression);
	archive_le32enc(zip->file_header + 12, dos_time(archive_entry_mtime(zip->entry)));
	archive_le16enc(zip->file_header + 28, filename_length);
	/* Following Info-Zip, store mode in the "external attributes" field. */
	archive_le32enc(zip->file_header + 38,
	    ((uint32_t)archive_entry_mode(zip->entry)) << 16);
	e = cd_alloc(zip, filename_length);
	/* If (e == NULL) XXXX */
	copy_path(zip->entry, e);

	/* Format extra data. */
	memset(local_extra, 0, sizeof(local_extra));
	e = local_extra;

	/* First, extra blocks that are the same between
	 * the local file header and the central directory.
	 * We format them once and then duplicate them. */

	/* UT timestamp, length depends on what timestamps are set. */
	memcpy(e, "UT", 2);
	archive_le16enc(e + 2,
	    1
	    + (archive_entry_mtime_is_set(entry) ? 4 : 0)
	    + (archive_entry_atime_is_set(entry) ? 4 : 0)
	    + (archive_entry_ctime_is_set(entry) ? 4 : 0));
	e += 4;
	*e++ =
	    (archive_entry_mtime_is_set(entry) ? 1 : 0)
	    | (archive_entry_atime_is_set(entry) ? 2 : 0)
	    | (archive_entry_ctime_is_set(entry) ? 4 : 0);
	if (archive_entry_mtime_is_set(entry)) {
		archive_le32enc(e, (uint32_t)archive_entry_mtime(entry));
		e += 4;
	}
	if (archive_entry_atime_is_set(entry)) {
		archive_le32enc(e, (uint32_t)archive_entry_atime(entry));
		e += 4;
	}
	if (archive_entry_ctime_is_set(entry)) {
		archive_le32enc(e, (uint32_t)archive_entry_ctime(entry));
		e += 4;
	}

	/* ux Unix extra data, length 11, version 1 */
	/* TODO: If uid < 64k, use 2 bytes, ditto for gid. */
	memcpy(e, "ux\013\000\001", 5);
	e += 5;
	*e++ = 4; /* Length of following UID */
	archive_le32enc(e, (uint32_t)archive_entry_uid(entry));
	e += 4;
	*e++ = 4; /* Length of following GID */
	archive_le32enc(e, (uint32_t)archive_entry_gid(entry));
	e += 4;

	/* Copy UT and ux into central directory as well. */
	zip->file_header_extra_offset = zip->central_directory_bytes;
	cd_extra = cd_alloc(zip, e - local_extra);
	memcpy(cd_extra, local_extra, e - local_extra);

	/*
	 * Following extra blocks vary between local header and
	 * central directory. These are the local header versions.
	 * Central directory versions get formatted in
	 * archive_write_zip_finish_entry() below.
	 */

	/* "[Zip64 entry] in the local header MUST include BOTH
	 * original [uncompressed] and compressed size fields." */
	if (zip->entry_uses_zip64) {
		unsigned char *zip64_start = e;
		memcpy(e, "\001\000\020\000", 4);
		e += 4;
		archive_le64enc(e, zip->entry_uncompressed_size);
		e += 8;
		archive_le64enc(e, zip->entry_compressed_size);
		e += 8;
		archive_le16enc(zip64_start + 2, e - (zip64_start + 4));
	}

	if (zip->flags & ZIP_FLAG_EXPERIMENT_EL) {
		/* Experimental 'el' extension to improve streaming. */
		unsigned char *external_info = e;
		int included = 7;
		memcpy(e, "el\000\000", 4); // 0x6c65 + 2-byte length
		e += 4;
		e[0] = included; /* bitmap of included fields */
		e += 1;
		if (included & 1) {
			archive_le16enc(e, /* "Version created by" */
			    3 * 256 + version_needed);
			e += 2;
		}
		if (included & 2) {
			archive_le16enc(e, 0); /* internal file attributes */
			e += 2;
		}
		if (included & 4) {
			archive_le32enc(e,  /* external file attributes */
			    ((uint32_t)archive_entry_mode(zip->entry)) << 16);
			e += 4;
		}
		if (included & 8) {
			// Libarchive does not currently support file comments.
		}
		archive_le16enc(external_info + 2, e - (external_info + 4));
	}

	/* Update local header with size of extra data and write it all out: */
	archive_le16enc(local_header + 28, e - local_extra);

	ret = __archive_write_output(a, local_header, 30);
	if (ret != ARCHIVE_OK)
		return (ARCHIVE_FATAL);
	zip->written_bytes += 30;

	ret = write_path(zip->entry, a);
	if (ret <= ARCHIVE_OK)
		return (ARCHIVE_FATAL);
	zip->written_bytes += ret;

	ret = __archive_write_output(a, local_extra, e - local_extra);
	if (ret != ARCHIVE_OK)
		return (ARCHIVE_FATAL);
	zip->written_bytes += e - local_extra;

	/* For symlinks, write the body now. */
	if (slink != NULL) {
		ret = __archive_write_output(a, slink, slink_size);
		if (ret != ARCHIVE_OK)
			return (ARCHIVE_FATAL);
		zip->entry_compressed_written += slink_size;
		zip->entry_uncompressed_written += slink_size;
		zip->written_bytes += slink_size;
	}

#ifdef HAVE_ZLIB_H
	if (zip->entry_compression == COMPRESSION_DEFLATE) {
		zip->stream.zalloc = Z_NULL;
		zip->stream.zfree = Z_NULL;
		zip->stream.opaque = Z_NULL;
		zip->stream.next_out = zip->buf;
		zip->stream.avail_out = (uInt)zip->len_buf;
		if (deflateInit2(&zip->stream, Z_DEFAULT_COMPRESSION,
		    Z_DEFLATED, -15, 8, Z_DEFAULT_STRATEGY) != Z_OK) {
			archive_set_error(&a->archive, ENOMEM,
			    "Can't init deflate compressor");
			return (ARCHIVE_FATAL);
		}
	}
#endif

	return (ret2);
}

static ssize_t
archive_write_zip_data(struct archive_write *a, const void *buff, size_t s)
{
	int ret;
	struct zip *zip = a->format_data;

	if ((int64_t)s > zip->entry_uncompressed_limit)
		s = (size_t)zip->entry_uncompressed_limit;
	zip->entry_uncompressed_written += s;

	if (s == 0) return 0;

	switch (zip->entry_compression) {
	case COMPRESSION_STORE:
		ret = __archive_write_output(a, buff, s);
		if (ret != ARCHIVE_OK)
			return (ret);
		zip->written_bytes += s;
		zip->entry_compressed_written += s;
		break;
#if HAVE_ZLIB_H
	case COMPRESSION_DEFLATE:
		zip->stream.next_in = (unsigned char*)(uintptr_t)buff;
		zip->stream.avail_in = (uInt)s;
		do {
			ret = deflate(&zip->stream, Z_NO_FLUSH);
			if (ret == Z_STREAM_ERROR)
				return (ARCHIVE_FATAL);
			if (zip->stream.avail_out == 0) {
				ret = __archive_write_output(a, zip->buf,
					zip->len_buf);
				if (ret != ARCHIVE_OK)
					return (ret);
				zip->entry_compressed_written += zip->len_buf;
				zip->written_bytes += zip->len_buf;
				zip->stream.next_out = zip->buf;
				zip->stream.avail_out = (uInt)zip->len_buf;
			}
		} while (zip->stream.avail_in != 0);
		break;
#endif

	default:
		archive_set_error(&a->archive, ARCHIVE_ERRNO_MISC,
		    "Invalid ZIP compression type");
		return ARCHIVE_FATAL;
	}

	zip->entry_uncompressed_limit -= s;
	zip->entry_crc32 = zip->crc32func(zip->entry_crc32, buff, (unsigned)s);
	return (s);

}

static int
archive_write_zip_finish_entry(struct archive_write *a)
{
	struct zip *zip = a->format_data;
	int ret;

#if HAVE_ZLIB_H
	if (zip->entry_compression == COMPRESSION_DEFLATE) {
		for (;;) {
			size_t remainder;
			ret = deflate(&zip->stream, Z_FINISH);
			if (ret == Z_STREAM_ERROR)
				return (ARCHIVE_FATAL);
			remainder = zip->len_buf - zip->stream.avail_out;
			ret = __archive_write_output(a, zip->buf, remainder);
			if (ret != ARCHIVE_OK)
				return (ret);
			zip->entry_compressed_written += remainder;
			zip->written_bytes += remainder;
			zip->stream.next_out = zip->buf;
			if (zip->stream.avail_out != 0)
				break;
			zip->stream.avail_out = (uInt)zip->len_buf;
		}
		deflateEnd(&zip->stream);
	}
#endif

	/* Write trailing data descriptor. */
	if ((zip->entry_flags & ZIP_ENTRY_FLAG_LENGTH_AT_END) != 0) {
		char d[24];
		memcpy(d, "PK\007\010", 4);
		archive_le32enc(d + 4, zip->entry_crc32);
		if (zip->entry_uses_zip64) {
			archive_le64enc(d + 8, (uint64_t)zip->entry_compressed_written);
			archive_le64enc(d + 16, (uint64_t)zip->entry_uncompressed_written);
			ret = __archive_write_output(a, d, 24);
			zip->written_bytes += 24;
		} else {
			archive_le32enc(d + 8, (uint32_t)zip->entry_compressed_written);
			archive_le32enc(d + 12, (uint32_t)zip->entry_uncompressed_written);
			ret = __archive_write_output(a, d, 16);
			zip->written_bytes += 16;
		}
		if (ret != ARCHIVE_OK)
			return (ARCHIVE_FATAL);
	}

	/* Append Zip64 extra data to central directory information. */
	if (zip->entry_compressed_written > ARCHIVE_LITERAL_LL(0xffffffff)
	    || zip->entry_uncompressed_written > ARCHIVE_LITERAL_LL(0xffffffff)
	    || zip->entry_offset > ARCHIVE_LITERAL_LL(0xffffffff)) {
		unsigned char zip64[32];
		unsigned char *z = zip64, *zd;
		memcpy(z, "\001\000\000\000", 4);
		z += 4;
		if (zip->entry_uncompressed_written >= ARCHIVE_LITERAL_LL(0xffffffff)) {
			archive_le64enc(z, zip->entry_uncompressed_written);
			z += 8;
		}
		if (zip->entry_compressed_written >= ARCHIVE_LITERAL_LL(0xffffffff)) {
			archive_le64enc(z, zip->entry_compressed_written);
			z += 8;
		}
		if (zip->entry_offset >= ARCHIVE_LITERAL_LL(0xffffffff)) {
			archive_le64enc(z, zip->entry_offset);
			z += 8;
		}
		archive_le16enc(zip64 + 2, z - (zip64 + 4));
		zd = cd_alloc(zip, z - zip64);
		if (zd == NULL) {
			archive_set_error(&a->archive, ENOMEM,
				"Can't allocate zip data");
			return (ARCHIVE_FATAL);
		}
		memcpy(zd, zip64, z - zip64);
		/* Zip64 means version needs to be set to at least 4.5 */
		if (archive_le16dec(zip->file_header + 6) < 45)
			archive_le16enc(zip->file_header + 6, 45);
	}

	/* Fix up central directory file header. */
	archive_le32enc(zip->file_header + 16, zip->entry_crc32);
	archive_le32enc(zip->file_header + 20,
	    zipmin(zip->entry_compressed_written, ARCHIVE_LITERAL_LL(0xffffffff)));
	archive_le32enc(zip->file_header + 24,
	    zipmin(zip->entry_uncompressed_written, ARCHIVE_LITERAL_LL(0xffffffff)));
	archive_le16enc(zip->file_header + 30,
	    zip->central_directory_bytes - zip->file_header_extra_offset);
	archive_le32enc(zip->file_header + 42,
	    zipmin(zip->entry_offset, ARCHIVE_LITERAL_LL(0xffffffff)));

	return (ARCHIVE_OK);
}

static int
archive_write_zip_close(struct archive_write *a)
{
	uint8_t buff[64];
	int64_t offset_start, offset_end;
	struct zip *zip = a->format_data;
	struct cd_segment *segment;
	int ret;

	offset_start = zip->written_bytes;
	segment = zip->central_directory;
	while (segment != NULL) {
		ret = __archive_write_output(a,
		    segment->buff, segment->p - segment->buff);
		if (ret != ARCHIVE_OK)
			return (ARCHIVE_FATAL);
		zip->written_bytes += segment->p - segment->buff;
		segment = segment->next;
	}
	offset_end = zip->written_bytes;

	/* If central dir info is too large, write Zip64 end-of-cd */
	if (offset_end - offset_start > ARCHIVE_LITERAL_LL(0xffffffff)
	    || offset_start > ARCHIVE_LITERAL_LL(0xffffffff)
	    || zip->central_directory_entries > 0xffffUL
	    || (zip->flags & ZIP_FLAG_FORCE_ZIP64)) {
	  /* Zip64 end-of-cd record */
	  memset(buff, 0, 56);
	  memcpy(buff, "PK\006\006", 4);
	  archive_le64enc(buff + 4, 44);
	  archive_le16enc(buff + 12, 45);
	  archive_le16enc(buff + 14, 45);
	  /* This is disk 0 of 0. */
	  archive_le64enc(buff + 24, zip->central_directory_entries);
	  archive_le64enc(buff + 32, zip->central_directory_entries);
	  archive_le64enc(buff + 40, offset_end - offset_start);
	  archive_le64enc(buff + 48, offset_start);
	  ret = __archive_write_output(a, buff, 56);
	  if (ret != ARCHIVE_OK)
		  return (ARCHIVE_FATAL);
	  zip->written_bytes += 56;

	  /* Zip64 end-of-cd locator record. */
	  memset(buff, 0, 20);
	  memcpy(buff, "PK\006\007", 4);
	  archive_le32enc(buff + 4, 0);
	  archive_le64enc(buff + 8, offset_end);
	  archive_le32enc(buff + 16, 1);
	  ret = __archive_write_output(a, buff, 20);
	  if (ret != ARCHIVE_OK)
		  return (ARCHIVE_FATAL);
	  zip->written_bytes += 20;

	}

	/* Format and write end of central directory. */
	memset(buff, 0, sizeof(buff));
	memcpy(buff, "PK\005\006", 4);
	archive_le16enc(buff + 8, zipmin(0xffffU, zip->central_directory_entries));
	archive_le16enc(buff + 10, zipmin(0xffffU, zip->central_directory_entries));
	archive_le32enc(buff + 12, (uint32_t)zipmin(ARCHIVE_LITERAL_LL(0xffffffff), (offset_end - offset_start)));
	archive_le32enc(buff + 16, (uint32_t)zipmin(ARCHIVE_LITERAL_LL(0xffffffff), offset_start));
	ret = __archive_write_output(a, buff, 22);
	if (ret != ARCHIVE_OK)
		return (ARCHIVE_FATAL);
	zip->written_bytes += 22;
	return (ARCHIVE_OK);
}

static int
archive_write_zip_free(struct archive_write *a)
{
	struct zip *zip;
	struct cd_segment *segment;

	zip = a->format_data;
	while (zip->central_directory != NULL) {
		segment = zip->central_directory;
		zip->central_directory = segment->next;
		free(segment->buff);
		free(segment);
	}
#ifdef HAVE_ZLIB_H
	free(zip->buf);
#endif
	archive_entry_free(zip->entry);
	/* TODO: Free opt_sconv, sconv_default */

	free(zip);
	a->format_data = NULL;
	return (ARCHIVE_OK);
}

/* Convert into MSDOS-style date/time. */
static unsigned int
dos_time(const time_t unix_time)
{
	struct tm *t;
	unsigned int dt;

	/* This will not preserve time when creating/extracting the archive
	 * on two systems with different time zones. */
	t = localtime(&unix_time);

	/* MSDOS-style date/time is only between 1980-01-01 and 2107-12-31 */
	if (t->tm_year < 1980 - 1900)
		/* Set minimum date/time '1980-01-01 00:00:00'. */
		dt = 0x00210000U;
	else if (t->tm_year > 2107 - 1900)
		/* Set maximum date/time '2107-12-31 23:59:58'. */
		dt = 0xff9fbf7dU;
	else {
		dt = 0;
		dt += ((t->tm_year - 80) & 0x7f) << 9;
		dt += ((t->tm_mon + 1) & 0x0f) << 5;
		dt += (t->tm_mday & 0x1f);
		dt <<= 16;
		dt += (t->tm_hour & 0x1f) << 11;
		dt += (t->tm_min & 0x3f) << 5;
		dt += (t->tm_sec & 0x3e) >> 1; /* Only counting every 2 seconds. */
	}
	return dt;
}

static size_t
path_length(struct archive_entry *entry)
{
	mode_t type;
	const char *path;

	type = archive_entry_filetype(entry);
	path = archive_entry_pathname(entry);

	if (path == NULL)
		return (0);
	if (type == AE_IFDIR &&
	    (path[0] == '\0' || path[strlen(path) - 1] != '/')) {
		return strlen(path) + 1;
	} else {
		return strlen(path);
	}
}

static int
write_path(struct archive_entry *entry, struct archive_write *archive)
{
	int ret;
	const char *path;
	mode_t type;
	size_t written_bytes;

	path = archive_entry_pathname(entry);
	type = archive_entry_filetype(entry);
	written_bytes = 0;

	ret = __archive_write_output(archive, path, strlen(path));
	if (ret != ARCHIVE_OK)
		return (ARCHIVE_FATAL);
	written_bytes += strlen(path);

	/* Folders are recognized by a trailing slash. */
	if ((type == AE_IFDIR) & (path[strlen(path) - 1] != '/')) {
		ret = __archive_write_output(archive, "/", 1);
		if (ret != ARCHIVE_OK)
			return (ARCHIVE_FATAL);
		written_bytes += 1;
	}

	return ((int)written_bytes);
}

static void
copy_path(struct archive_entry *entry, unsigned char *p)
{
	const char *path;
	size_t pathlen;
	mode_t type;

	path = archive_entry_pathname(entry);
	pathlen = strlen(path);
	type = archive_entry_filetype(entry);

	memcpy(p, path, pathlen);

	/* Folders are recognized by a trailing slash. */
	if ((type == AE_IFDIR) & (path[pathlen - 1] != '/')) {
		p[pathlen] = '/';
		p[pathlen + 1] = '\0';
	}
}


static struct archive_string_conv *
get_sconv(struct archive_write *a, struct zip *zip)
{
	if (zip->opt_sconv != NULL)
		return (zip->opt_sconv);

	if (!zip->init_default_conversion) {
		zip->sconv_default =
		    archive_string_default_conversion_for_write(&(a->archive));
		zip->init_default_conversion = 1;
	}
	return (zip->sconv_default);
}
