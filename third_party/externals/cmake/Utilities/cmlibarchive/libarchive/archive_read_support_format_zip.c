/*-
 * Copyright (c) 2004-2013 Tim Kientzle
 * Copyright (c) 2011-2012 Michihiro NAKAJIMA
 * Copyright (c) 2013 Konrad Kleine
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
__FBSDID("$FreeBSD: head/lib/libarchive/archive_read_support_format_zip.c 201102 2009-12-28 03:11:36Z kientzle $");

/*
 * The definitive documentation of the Zip file format is:
 *   http://www.pkware.com/documents/casestudies/APPNOTE.TXT
 *
 * The Info-Zip project has pioneered various extensions to better
 * support Zip on Unix, including the 0x5455 "UT", 0x5855 "UX", 0x7855
 * "Ux", and 0x7875 "ux" extensions for time and ownership
 * information.
 *
 * History of this code: The streaming Zip reader was first added to
 * libarchive in January 2005.  Support for seekable input sources was
 * added in Nov 2011.
 */

#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_ZLIB_H
#include <cm_zlib.h>
#endif

#include "archive.h"
#include "archive_endian.h"
#include "archive_entry.h"
#include "archive_entry_locale.h"
#include "archive_private.h"
#include "archive_rb.h"
#include "archive_read_private.h"

#ifndef HAVE_ZLIB_H
#include "archive_crc32.h"
#endif

#if defined(_WIN32) && !defined(__CYGWIN__)
# define snprintf _snprintf
#endif

struct zip_entry {
	struct archive_rb_node	node;
	struct zip_entry	*next;
	int64_t			local_header_offset;
	int64_t			compressed_size;
	int64_t			uncompressed_size;
	int64_t			gid;
	int64_t			uid;
	struct archive_string	rsrcname;
	time_t			mtime;
	time_t			atime;
	time_t			ctime;
	uint32_t		crc32;
	uint16_t		mode;
	uint16_t		zip_flags; /* From GP Flags Field */
	unsigned char		compression;
	unsigned char		system; /* From "version written by" */
	unsigned char		flags; /* Our extra markers. */
};

/* Bits used in zip_flags. */
#define ZIP_ENCRYPTED	(1 << 0)
#define ZIP_LENGTH_AT_END	(1 << 3)
#define ZIP_STRONG_ENCRYPTED	(1 << 6)
#define ZIP_UTF8_NAME	(1 << 11)
/* See "7.2 Single Password Symmetric Encryption Method"
   in http://www.pkware.com/documents/casestudies/APPNOTE.TXT */
#define ZIP_CENTRAL_DIRECTORY_ENCRYPTED	(1 << 13)

/* Bits used in flags. */
#define LA_USED_ZIP64	(1 << 0)
#define LA_FROM_CENTRAL_DIRECTORY (1 << 1)

struct zip {
	/* Structural information about the archive. */
	char			format_name[64];
	int64_t			central_directory_offset;
	size_t			central_directory_entries_total;
	size_t			central_directory_entries_on_this_disk;
	int			has_encrypted_entries;

	/* List of entries (seekable Zip only) */
	struct zip_entry	*zip_entries;
	struct archive_rb_tree	tree;
	struct archive_rb_tree	tree_rsrc;

	/* Bytes read but not yet consumed via __archive_read_consume() */
	size_t			unconsumed;

	/* Information about entry we're currently reading. */
	struct zip_entry	*entry;
	int64_t			entry_bytes_remaining;

	/* These count the number of bytes actually read for the entry. */
	int64_t			entry_compressed_bytes_read;
	int64_t			entry_uncompressed_bytes_read;

	/* Running CRC32 of the decompressed data */
	unsigned long		entry_crc32;
	unsigned long		(*crc32func)(unsigned long, const void *, size_t);
	char			ignore_crc32;

	/* Flags to mark progress of decompression. */
	char			decompress_init;
	char			end_of_entry;

#ifdef HAVE_ZLIB_H
	unsigned char 		*uncompressed_buffer;
	size_t 			uncompressed_buffer_size;
	z_stream		stream;
	char			stream_valid;
#endif

	struct archive_string_conv *sconv;
	struct archive_string_conv *sconv_default;
	struct archive_string_conv *sconv_utf8;
	int			init_default_conversion;
	int			process_mac_extensions;
};

/* Many systems define min or MIN, but not all. */
#define	zipmin(a,b) ((a) < (b) ? (a) : (b))

/* ------------------------------------------------------------------------ */

/*
 * Common code for streaming or seeking modes.
 *
 * Includes code to read local file headers, decompress data
 * from entry bodies, and common API.
 */

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

static struct {
	int id;
	const char * name;
} compression_methods[] = {
	{0, "uncompressed"}, /* The file is stored (no compression) */
	{1, "shrinking"}, /* The file is Shrunk */
	{2, "reduced-1"}, /* The file is Reduced with compression factor 1 */
	{3, "reduced-2"}, /* The file is Reduced with compression factor 2 */
	{4, "reduced-3"}, /* The file is Reduced with compression factor 3 */
	{5, "reduced-4"}, /* The file is Reduced with compression factor 4 */
	{6, "imploded"}, /* The file is Imploded */
	{7, "reserved"}, /* Reserved for Tokenizing compression algorithm */
	{8, "deflation"}, /* The file is Deflated */
	{9, "deflation-64-bit"}, /* Enhanced Deflating using Deflate64(tm) */
	{10, "ibm-terse"}, /* PKWARE Data Compression Library Imploding (old IBM TERSE) */
	{11, "reserved"}, /* Reserved by PKWARE */
	{12, "bzip"}, /* File is compressed using BZIP2 algorithm */
	{13, "reserved"}, /* Reserved by PKWARE */
	{14, "lzma"}, /* LZMA (EFS) */
	{15, "reserved"}, /* Reserved by PKWARE */
	{16, "reserved"}, /* Reserved by PKWARE */
	{17, "reserved"}, /* Reserved by PKWARE */
	{18, "ibm-terse-new"}, /* File is compressed using IBM TERSE (new) */
	{19, "ibm-lz777"}, /* IBM LZ77 z Architecture (PFS) */
	{97, "wav-pack"}, /* WavPack compressed data */
	{98, "ppmd-1"} /* PPMd version I, Rev 1 */
};

static const char *
compression_name(const int compression)
{
	static const int num_compression_methods = sizeof(compression_methods)/sizeof(compression_methods[0]);
	int i=0;
	while(compression >= 0 && i < num_compression_methods) {
		if (compression_methods[i].id == compression) {
			return compression_methods[i].name;
		}
		i++;
	}
	return "??";
}

/* Convert an MSDOS-style date/time into Unix-style time. */
static time_t
zip_time(const char *p)
{
	int msTime, msDate;
	struct tm ts;

	msTime = (0xff & (unsigned)p[0]) + 256 * (0xff & (unsigned)p[1]);
	msDate = (0xff & (unsigned)p[2]) + 256 * (0xff & (unsigned)p[3]);

	memset(&ts, 0, sizeof(ts));
	ts.tm_year = ((msDate >> 9) & 0x7f) + 80; /* Years since 1900. */
	ts.tm_mon = ((msDate >> 5) & 0x0f) - 1; /* Month number. */
	ts.tm_mday = msDate & 0x1f; /* Day of month. */
	ts.tm_hour = (msTime >> 11) & 0x1f;
	ts.tm_min = (msTime >> 5) & 0x3f;
	ts.tm_sec = (msTime << 1) & 0x3e;
	ts.tm_isdst = -1;
	return mktime(&ts);
}

/*
 * The extra data is stored as a list of
 *	id1+size1+data1 + id2+size2+data2 ...
 *  triplets.  id and size are 2 bytes each.
 */
static void
process_extra(const char *p, size_t extra_length, struct zip_entry* zip_entry)
{
	unsigned offset = 0;

	while (offset < extra_length - 4)
	{
		unsigned short headerid = archive_le16dec(p + offset);
		unsigned short datasize = archive_le16dec(p + offset + 2);
		offset += 4;
		if (offset + datasize > extra_length)
			break;
#ifdef DEBUG
		fprintf(stderr, "Header id 0x%x, length %d\n",
		    headerid, datasize);
#endif
		switch (headerid) {
		case 0x0001:
			/* Zip64 extended information extra field. */
			zip_entry->flags |= LA_USED_ZIP64;
			if (zip_entry->uncompressed_size == 0xffffffff) {
				if (datasize < 8)
					break;
				zip_entry->uncompressed_size =
				    archive_le64dec(p + offset);
				offset += 8;
				datasize -= 8;
			}
			if (zip_entry->compressed_size == 0xffffffff) {
				if (datasize < 8)
					break;
				zip_entry->compressed_size =
				    archive_le64dec(p + offset);
				offset += 8;
				datasize -= 8;
			}
			if (zip_entry->local_header_offset == 0xffffffff) {
				if (datasize < 8)
					break;
				zip_entry->local_header_offset =
				    archive_le64dec(p + offset);
				offset += 8;
				datasize -= 8;
			}
			/* archive_le32dec(p + offset) gives disk
			 * on which file starts, but we don't handle
			 * multi-volume Zip files. */
			break;
		case 0x5455:
		{
			/* Extended time field "UT". */
			int flags = p[offset];
			offset++;
			datasize--;
			/* Flag bits indicate which dates are present. */
			if (flags & 0x01)
			{
#ifdef DEBUG
				fprintf(stderr, "mtime: %lld -> %d\n",
				    (long long)zip_entry->mtime,
				    archive_le32dec(p + offset));
#endif
				if (datasize < 4)
					break;
				zip_entry->mtime = archive_le32dec(p + offset);
				offset += 4;
				datasize -= 4;
			}
			if (flags & 0x02)
			{
				if (datasize < 4)
					break;
				zip_entry->atime = archive_le32dec(p + offset);
				offset += 4;
				datasize -= 4;
			}
			if (flags & 0x04)
			{
				if (datasize < 4)
					break;
				zip_entry->ctime = archive_le32dec(p + offset);
				offset += 4;
				datasize -= 4;
			}
			break;
		}
		case 0x5855:
		{
			/* Info-ZIP Unix Extra Field (old version) "UX". */
			if (datasize >= 8) {
				zip_entry->atime = archive_le32dec(p + offset);
				zip_entry->mtime =
				    archive_le32dec(p + offset + 4);
			}
			if (datasize >= 12) {
				zip_entry->uid =
				    archive_le16dec(p + offset + 8);
				zip_entry->gid =
				    archive_le16dec(p + offset + 10);
			}
			break;
		}
		case 0x6c65:
		{
			/* Experimental 'el' field */
			/*
			 * Introduced Dec 2013 to provide a way to
			 * include external file attributes in local file
			 * header.  This provides file type and permission
			 * information necessary to support full streaming
			 * extraction.  Currently being discussed with
			 * other Zip developers... subject to change.
			 */
			int bitmap, bitmap_last;

			if (datasize < 1)
				break;
			bitmap_last = bitmap = 0xff & p[offset];
			offset += 1;
			datasize -= 1;

			/* We only support first 7 bits of bitmap; skip rest. */
			while ((bitmap_last & 0x80) != 0
			    && datasize >= 1) {
				bitmap_last = p[offset];
				offset += 1;
				datasize -= 1;
			}

			if (bitmap & 1) {
				// 2 byte "version made by"
				if (datasize < 2)
					break;
				zip_entry->system
				    = archive_le16dec(p + offset) >> 8;
				offset += 2;
				datasize -= 2;
			}
			if (bitmap & 2) {
				// 2 byte "internal file attributes"
				uint32_t internal_attributes;
				if (datasize < 2)
					break;
				internal_attributes
				    = archive_le16dec(p + offset);
				// Not used by libarchive at present.
				(void)internal_attributes; /* UNUSED */
				offset += 2;
				datasize -= 2;
			}
			if (bitmap & 4) {
				// 4 byte "external file attributes"
				uint32_t external_attributes;
				if (datasize < 4)
					break;
				external_attributes
				    = archive_le32dec(p + offset);
				if (zip_entry->system == 3) {
					zip_entry->mode
					    = external_attributes >> 16;
				}
				offset += 4;
				datasize -= 4;
			}
			if (bitmap & 8) {
				// 2 byte comment length + comment
				uint32_t comment_length;
				if (datasize < 2)
					break;
				comment_length
				    = archive_le16dec(p + offset);
				offset += 2;
				datasize -= 2;

				if (datasize < comment_length)
					break;
				// Comment is not supported by libarchive
				offset += comment_length;
				datasize -= comment_length;
			}
			break;
		}
		case 0x7855:
			/* Info-ZIP Unix Extra Field (type 2) "Ux". */
#ifdef DEBUG
			fprintf(stderr, "uid %d gid %d\n",
			    archive_le16dec(p + offset),
			    archive_le16dec(p + offset + 2));
#endif
			if (datasize >= 2)
				zip_entry->uid = archive_le16dec(p + offset);
			if (datasize >= 4)
				zip_entry->gid =
				    archive_le16dec(p + offset + 2);
			break;
		case 0x7875:
		{
			/* Info-Zip Unix Extra Field (type 3) "ux". */
			int uidsize = 0, gidsize = 0;

			/* TODO: support arbitrary uidsize/gidsize. */
			if (datasize >= 1 && p[offset] == 1) {/* version=1 */
				if (datasize >= 4) {
					/* get a uid size. */
					uidsize = p[offset+1];
					if (uidsize == 2)
						zip_entry->uid =
						    archive_le16dec(
						        p + offset + 2);
					else if (uidsize == 4 && datasize >= 6)
						zip_entry->uid =
						    archive_le32dec(
						        p + offset + 2);
				}
				if (datasize >= (2 + uidsize + 3)) {
					/* get a gid size. */
					gidsize = p[offset+2+uidsize];
					if (gidsize == 2)
						zip_entry->gid =
						    archive_le16dec(
						        p+offset+2+uidsize+1);
					else if (gidsize == 4 &&
					    datasize >= (2 + uidsize + 5))
						zip_entry->gid =
						    archive_le32dec(
						        p+offset+2+uidsize+1);
				}
			}
			break;
		}
		default:
			break;
		}
		offset += datasize;
	}
#ifdef DEBUG
	if (offset != extra_length)
	{
		fprintf(stderr,
		    "Extra data field contents do not match reported size!\n");
	}
#endif
}

/*
 * Assumes file pointer is at beginning of local file header.
 */
static int
zip_read_local_file_header(struct archive_read *a, struct archive_entry *entry,
    struct zip *zip)
{
	const char *p;
	const void *h;
	const wchar_t *wp;
	const char *cp;
	size_t len, filename_length, extra_length;
	struct archive_string_conv *sconv;
	struct zip_entry *zip_entry = zip->entry;
	struct zip_entry zip_entry_central_dir;
	int ret = ARCHIVE_OK;
	char version;

	/* Save a copy of the original for consistency checks. */
	zip_entry_central_dir = *zip_entry;

	zip->decompress_init = 0;
	zip->end_of_entry = 0;
	zip->entry_uncompressed_bytes_read = 0;
	zip->entry_compressed_bytes_read = 0;
	zip->entry_crc32 = zip->crc32func(0, NULL, 0);

	/* Setup default conversion. */
	if (zip->sconv == NULL && !zip->init_default_conversion) {
		zip->sconv_default =
		    archive_string_default_conversion_for_read(&(a->archive));
		zip->init_default_conversion = 1;
	}

	if ((p = __archive_read_ahead(a, 30, NULL)) == NULL) {
		archive_set_error(&a->archive, ARCHIVE_ERRNO_FILE_FORMAT,
		    "Truncated ZIP file header");
		return (ARCHIVE_FATAL);
	}

	if (memcmp(p, "PK\003\004", 4) != 0) {
		archive_set_error(&a->archive, -1, "Damaged Zip archive");
		return ARCHIVE_FATAL;
	}
	version = p[4];
	zip_entry->system = p[5];
	zip_entry->zip_flags = archive_le16dec(p + 6);
	if (zip_entry->zip_flags & (ZIP_ENCRYPTED | ZIP_STRONG_ENCRYPTED)) {
		zip->has_encrypted_entries = 1;
		archive_entry_set_is_data_encrypted(entry, 1);
		if (zip_entry->zip_flags & ZIP_CENTRAL_DIRECTORY_ENCRYPTED &&
			zip_entry->zip_flags & ZIP_ENCRYPTED &&
			zip_entry->zip_flags & ZIP_STRONG_ENCRYPTED) {
			archive_entry_set_is_metadata_encrypted(entry, 1);
			return ARCHIVE_FATAL;
		}
	}
	zip_entry->compression = (char)archive_le16dec(p + 8);
	zip_entry->mtime = zip_time(p + 10);
	zip_entry->crc32 = archive_le32dec(p + 14);
	zip_entry->compressed_size = archive_le32dec(p + 18);
	zip_entry->uncompressed_size = archive_le32dec(p + 22);
	filename_length = archive_le16dec(p + 26);
	extra_length = archive_le16dec(p + 28);

	__archive_read_consume(a, 30);

	/* Read the filename. */
	if ((h = __archive_read_ahead(a, filename_length, NULL)) == NULL) {
		archive_set_error(&a->archive, ARCHIVE_ERRNO_FILE_FORMAT,
		    "Truncated ZIP file header");
		return (ARCHIVE_FATAL);
	}
	if (zip_entry->zip_flags & ZIP_UTF8_NAME) {
		/* The filename is stored to be UTF-8. */
		if (zip->sconv_utf8 == NULL) {
			zip->sconv_utf8 =
			    archive_string_conversion_from_charset(
				&a->archive, "UTF-8", 1);
			if (zip->sconv_utf8 == NULL)
				return (ARCHIVE_FATAL);
		}
		sconv = zip->sconv_utf8;
	} else if (zip->sconv != NULL)
		sconv = zip->sconv;
	else
		sconv = zip->sconv_default;

	if (archive_entry_copy_pathname_l(entry,
	    h, filename_length, sconv) != 0) {
		if (errno == ENOMEM) {
			archive_set_error(&a->archive, ENOMEM,
			    "Can't allocate memory for Pathname");
			return (ARCHIVE_FATAL);
		}
		archive_set_error(&a->archive,
		    ARCHIVE_ERRNO_FILE_FORMAT,
		    "Pathname cannot be converted "
		    "from %s to current locale.",
		    archive_string_conversion_charset_name(sconv));
		ret = ARCHIVE_WARN;
	}
	__archive_read_consume(a, filename_length);

	/* Work around a bug in Info-Zip: When reading from a pipe, it
	 * stats the pipe instead of synthesizing a file entry. */
	if ((zip_entry->mode & AE_IFMT) == AE_IFIFO) {
		zip_entry->mode &= ~ AE_IFMT;
		zip_entry->mode |= AE_IFREG;
	}

	if ((zip_entry->mode & AE_IFMT) == 0) {
		/* Especially in streaming mode, we can end up
		   here without having seen proper mode information.
		   Guess from the filename. */
		wp = archive_entry_pathname_w(entry);
		if (wp != NULL) {
			len = wcslen(wp);
			if (len > 0 && wp[len - 1] == L'/')
				zip_entry->mode |= AE_IFDIR;
			else
				zip_entry->mode |= AE_IFREG;
		} else {
			cp = archive_entry_pathname(entry);
			len = (cp != NULL)?strlen(cp):0;
			if (len > 0 && cp[len - 1] == '/')
				zip_entry->mode |= AE_IFDIR;
			else
				zip_entry->mode |= AE_IFREG;
		}
		if (zip_entry->mode == AE_IFDIR) {
			zip_entry->mode |= 0775;
		} else if (zip_entry->mode == AE_IFREG) {
			zip_entry->mode |= 0664;
		}
	}

	/* Read the extra data. */
	if ((h = __archive_read_ahead(a, extra_length, NULL)) == NULL) {
		archive_set_error(&a->archive, ARCHIVE_ERRNO_FILE_FORMAT,
		    "Truncated ZIP file header");
		return (ARCHIVE_FATAL);
	}

	process_extra(h, extra_length, zip_entry);
	__archive_read_consume(a, extra_length);

	if (zip_entry->flags & LA_FROM_CENTRAL_DIRECTORY) {
		/* If this came from the central dir, it's size info
		 * is definitive, so ignore the length-at-end flag. */
		zip_entry->zip_flags &= ~ZIP_LENGTH_AT_END;
		/* If local header is missing a value, use the one from
		   the central directory.  If both have it, warn about
		   mismatches. */
		if (zip_entry->crc32 == 0) {
			zip_entry->crc32 = zip_entry_central_dir.crc32;
		} else if (!zip->ignore_crc32
		    && zip_entry->crc32 != zip_entry_central_dir.crc32) {
			archive_set_error(&a->archive,
			    ARCHIVE_ERRNO_FILE_FORMAT,
			    "Inconsistent CRC32 values");
			ret = ARCHIVE_WARN;
		}
		if (zip_entry->compressed_size == 0) {
			zip_entry->compressed_size
			    = zip_entry_central_dir.compressed_size;
		} else if (zip_entry->compressed_size
		    != zip_entry_central_dir.compressed_size) {
			archive_set_error(&a->archive,
			    ARCHIVE_ERRNO_FILE_FORMAT,
			    "Inconsistent compressed size: "
			    "%jd in central directory, %jd in local header",
			    (intmax_t)zip_entry_central_dir.compressed_size,
			    (intmax_t)zip_entry->compressed_size);
			ret = ARCHIVE_WARN;
		}
		if (zip_entry->uncompressed_size == 0) {
			zip_entry->uncompressed_size
			    = zip_entry_central_dir.uncompressed_size;
		} else if (zip_entry->uncompressed_size
		    != zip_entry_central_dir.uncompressed_size) {
			archive_set_error(&a->archive,
			    ARCHIVE_ERRNO_FILE_FORMAT,
			    "Inconsistent uncompressed size: "
			    "%jd in central directory, %jd in local header",
			    (intmax_t)zip_entry_central_dir.uncompressed_size,
			    (intmax_t)zip_entry->uncompressed_size);
			ret = ARCHIVE_WARN;
		}
	}

	/* Populate some additional entry fields: */
	archive_entry_set_mode(entry, zip_entry->mode);
	archive_entry_set_uid(entry, zip_entry->uid);
	archive_entry_set_gid(entry, zip_entry->gid);
	archive_entry_set_mtime(entry, zip_entry->mtime, 0);
	archive_entry_set_ctime(entry, zip_entry->ctime, 0);
	archive_entry_set_atime(entry, zip_entry->atime, 0);

	if ((zip->entry->mode & AE_IFMT) == AE_IFLNK) {
		size_t linkname_length = zip_entry->compressed_size;

		archive_entry_set_size(entry, 0);
		p = __archive_read_ahead(a, linkname_length, NULL);
		if (p == NULL) {
			archive_set_error(&a->archive, ARCHIVE_ERRNO_MISC,
			    "Truncated Zip file");
			return ARCHIVE_FATAL;
		}
		if (__archive_read_consume(a, linkname_length) < 0) {
			archive_set_error(&a->archive, ARCHIVE_ERRNO_MISC,
			    "Read error skipping symlink target name");
			return ARCHIVE_FATAL;
		}

		sconv = zip->sconv;
		if (sconv == NULL && (zip->entry->zip_flags & ZIP_UTF8_NAME))
			sconv = zip->sconv_utf8;
		if (sconv == NULL)
			sconv = zip->sconv_default;
		if (archive_entry_copy_symlink_l(entry, p, linkname_length,
		    sconv) != 0) {
			if (errno != ENOMEM && sconv == zip->sconv_utf8 &&
			    (zip->entry->zip_flags & ZIP_UTF8_NAME))
			    archive_entry_copy_symlink_l(entry, p,
				linkname_length, NULL);
			if (errno == ENOMEM) {
				archive_set_error(&a->archive, ENOMEM,
				    "Can't allocate memory for Symlink");
				return (ARCHIVE_FATAL);
			}
			/*
			 * Since there is no character-set regulation for
			 * symlink name, do not report the conversion error
			 * in an automatic conversion.
			 */
			if (sconv != zip->sconv_utf8 ||
			    (zip->entry->zip_flags & ZIP_UTF8_NAME) == 0) {
				archive_set_error(&a->archive,
				    ARCHIVE_ERRNO_FILE_FORMAT,
				    "Symlink cannot be converted "
				    "from %s to current locale.",
				    archive_string_conversion_charset_name(
					sconv));
				ret = ARCHIVE_WARN;
			}
		}
		zip_entry->uncompressed_size = zip_entry->compressed_size = 0;
	} else if (0 == (zip_entry->zip_flags & ZIP_LENGTH_AT_END)
	    || zip_entry->uncompressed_size > 0) {
		/* Set the size only if it's meaningful. */
		archive_entry_set_size(entry, zip_entry->uncompressed_size);
	}
	zip->entry_bytes_remaining = zip_entry->compressed_size;

	/* If there's no body, force read_data() to return EOF immediately. */
	if (0 == (zip_entry->zip_flags & ZIP_LENGTH_AT_END)
	    && zip->entry_bytes_remaining < 1)
		zip->end_of_entry = 1;

	/* Set up a more descriptive format name. */
	snprintf(zip->format_name, sizeof(zip->format_name), "ZIP %d.%d (%s)",
	    version / 10, version % 10,
	    compression_name(zip->entry->compression));
	a->archive.archive_format_name = zip->format_name;

	return (ret);
}

/*
 * Read "uncompressed" data.  There are three cases:
 *  1) We know the size of the data.  This is always true for the
 * seeking reader (we've examined the Central Directory already).
 *  2) ZIP_LENGTH_AT_END was set, but only the CRC was deferred.
 * Info-ZIP seems to do this; we know the size but have to grab
 * the CRC from the data descriptor afterwards.
 *  3) We're streaming and ZIP_LENGTH_AT_END was specified and
 * we have no size information.  In this case, we can do pretty
 * well by watching for the data descriptor record.  The data
 * descriptor is 16 bytes and includes a computed CRC that should
 * provide a strong check.
 *
 * TODO: Technically, the PK\007\010 signature is optional.
 * In the original spec, the data descriptor contained CRC
 * and size fields but had no leading signature.  In practice,
 * newer writers seem to provide the signature pretty consistently.
 *
 * For uncompressed data, the PK\007\010 marker seems essential
 * to be sure we've actually seen the end of the entry.
 *
 * Returns ARCHIVE_OK if successful, ARCHIVE_FATAL otherwise, sets
 * zip->end_of_entry if it consumes all of the data.
 */
static int
zip_read_data_none(struct archive_read *a, const void **_buff,
    size_t *size, int64_t *offset)
{
	struct zip *zip;
	const char *buff;
	ssize_t bytes_avail;

	(void)offset; /* UNUSED */

	zip = (struct zip *)(a->format->data);

	if (zip->entry->zip_flags & ZIP_LENGTH_AT_END) {
		const char *p;

		/* Grab at least 24 bytes. */
		buff = __archive_read_ahead(a, 24, &bytes_avail);
		if (bytes_avail < 24) {
			/* Zip archives have end-of-archive markers
			   that are longer than this, so a failure to get at
			   least 24 bytes really does indicate a truncated
			   file. */
			archive_set_error(&a->archive,
			    ARCHIVE_ERRNO_FILE_FORMAT,
			    "Truncated ZIP file data");
			return (ARCHIVE_FATAL);
		}
		/* Check for a complete PK\007\010 signature, followed
		 * by the correct 4-byte CRC. */
		p = buff;
		if (p[0] == 'P' && p[1] == 'K'
		    && p[2] == '\007' && p[3] == '\010'
		    && (archive_le32dec(p + 4) == zip->entry_crc32
			|| zip->ignore_crc32)) {
			if (zip->entry->flags & LA_USED_ZIP64) {
				zip->entry->crc32 = archive_le32dec(p + 4);
				zip->entry->compressed_size = archive_le64dec(p + 8);
				zip->entry->uncompressed_size = archive_le64dec(p + 16);
				zip->unconsumed = 24;
			} else {
				zip->entry->crc32 = archive_le32dec(p + 4);
				zip->entry->compressed_size = archive_le32dec(p + 8);
				zip->entry->uncompressed_size = archive_le32dec(p + 12);
				zip->unconsumed = 16;
			}
			zip->end_of_entry = 1;
			return (ARCHIVE_OK);
		}
		/* If not at EOF, ensure we consume at least one byte. */
		++p;

		/* Scan forward until we see where a PK\007\010 signature
		 * might be. */
		/* Return bytes up until that point.  On the next call,
		 * the code above will verify the data descriptor. */
		while (p < buff + bytes_avail - 4) {
			if (p[3] == 'P') { p += 3; }
			else if (p[3] == 'K') { p += 2; }
			else if (p[3] == '\007') { p += 1; }
			else if (p[3] == '\010' && p[2] == '\007'
			    && p[1] == 'K' && p[0] == 'P') {
				break;
			} else { p += 4; }
		}
		bytes_avail = p - buff;
	} else {
		if (zip->entry_bytes_remaining == 0) {
			zip->end_of_entry = 1;
			return (ARCHIVE_OK);
		}
		/* Grab a bunch of bytes. */
		buff = __archive_read_ahead(a, 1, &bytes_avail);
		if (bytes_avail <= 0) {
			archive_set_error(&a->archive,
			    ARCHIVE_ERRNO_FILE_FORMAT,
			    "Truncated ZIP file data");
			return (ARCHIVE_FATAL);
		}
		if (bytes_avail > zip->entry_bytes_remaining)
			bytes_avail = (ssize_t)zip->entry_bytes_remaining;
	}
	*size = bytes_avail;
	zip->entry_bytes_remaining -= bytes_avail;
	zip->entry_uncompressed_bytes_read += bytes_avail;
	zip->entry_compressed_bytes_read += bytes_avail;
	zip->unconsumed += bytes_avail;
	*_buff = buff;
	return (ARCHIVE_OK);
}

#ifdef HAVE_ZLIB_H
static int
zip_deflate_init(struct archive_read *a, struct zip *zip)
{
	int r;

	/* If we haven't yet read any data, initialize the decompressor. */
	if (!zip->decompress_init) {
		if (zip->stream_valid)
			r = inflateReset(&zip->stream);
		else
			r = inflateInit2(&zip->stream,
			    -15 /* Don't check for zlib header */);
		if (r != Z_OK) {
			archive_set_error(&a->archive, ARCHIVE_ERRNO_MISC,
			    "Can't initialize ZIP decompression.");
			return (ARCHIVE_FATAL);
		}
		/* Stream structure has been set up. */
		zip->stream_valid = 1;
		/* We've initialized decompression for this stream. */
		zip->decompress_init = 1;
	}
	return (ARCHIVE_OK);
}

static int
zip_read_data_deflate(struct archive_read *a, const void **buff,
    size_t *size, int64_t *offset)
{
	struct zip *zip;
	ssize_t bytes_avail;
	const void *compressed_buff;
	int r;

	(void)offset; /* UNUSED */

	zip = (struct zip *)(a->format->data);

	/* If the buffer hasn't been allocated, allocate it now. */
	if (zip->uncompressed_buffer == NULL) {
		zip->uncompressed_buffer_size = 256 * 1024;
		zip->uncompressed_buffer
		    = (unsigned char *)malloc(zip->uncompressed_buffer_size);
		if (zip->uncompressed_buffer == NULL) {
			archive_set_error(&a->archive, ENOMEM,
			    "No memory for ZIP decompression");
			return (ARCHIVE_FATAL);
		}
	}

	r = zip_deflate_init(a, zip);
	if (r != ARCHIVE_OK)
		return (r);

	/*
	 * Note: '1' here is a performance optimization.
	 * Recall that the decompression layer returns a count of
	 * available bytes; asking for more than that forces the
	 * decompressor to combine reads by copying data.
	 */
	compressed_buff = __archive_read_ahead(a, 1, &bytes_avail);
	if (0 == (zip->entry->zip_flags & ZIP_LENGTH_AT_END)
	    && bytes_avail > zip->entry_bytes_remaining) {
		bytes_avail = (ssize_t)zip->entry_bytes_remaining;
	}
	if (bytes_avail <= 0) {
		archive_set_error(&a->archive, ARCHIVE_ERRNO_FILE_FORMAT,
		    "Truncated ZIP file body");
		return (ARCHIVE_FATAL);
	}

	/*
	 * A bug in zlib.h: stream.next_in should be marked 'const'
	 * but isn't (the library never alters data through the
	 * next_in pointer, only reads it).  The result: this ugly
	 * cast to remove 'const'.
	 */
	zip->stream.next_in = (Bytef *)(uintptr_t)(const void *)compressed_buff;
	zip->stream.avail_in = (uInt)bytes_avail;
	zip->stream.total_in = 0;
	zip->stream.next_out = zip->uncompressed_buffer;
	zip->stream.avail_out = (uInt)zip->uncompressed_buffer_size;
	zip->stream.total_out = 0;

	r = inflate(&zip->stream, 0);
	switch (r) {
	case Z_OK:
		break;
	case Z_STREAM_END:
		zip->end_of_entry = 1;
		break;
	case Z_MEM_ERROR:
		archive_set_error(&a->archive, ENOMEM,
		    "Out of memory for ZIP decompression");
		return (ARCHIVE_FATAL);
	default:
		archive_set_error(&a->archive, ARCHIVE_ERRNO_MISC,
		    "ZIP decompression failed (%d)", r);
		return (ARCHIVE_FATAL);
	}

	/* Consume as much as the compressor actually used. */
	bytes_avail = zip->stream.total_in;
	__archive_read_consume(a, bytes_avail);
	zip->entry_bytes_remaining -= bytes_avail;
	zip->entry_compressed_bytes_read += bytes_avail;

	*size = zip->stream.total_out;
	zip->entry_uncompressed_bytes_read += zip->stream.total_out;
	*buff = zip->uncompressed_buffer;

	if (zip->end_of_entry && (zip->entry->zip_flags & ZIP_LENGTH_AT_END)) {
		const char *p;

		if (NULL == (p = __archive_read_ahead(a, 24, NULL))) {
			archive_set_error(&a->archive,
			    ARCHIVE_ERRNO_FILE_FORMAT,
			    "Truncated ZIP end-of-file record");
			return (ARCHIVE_FATAL);
		}
		/* Consume the optional PK\007\010 marker. */
		if (p[0] == 'P' && p[1] == 'K' &&
		    p[2] == '\007' && p[3] == '\010') {
			p += 4;
			zip->unconsumed = 4;
		}
		if (zip->entry->flags & LA_USED_ZIP64) {
			zip->entry->crc32 = archive_le32dec(p);
			zip->entry->compressed_size = archive_le64dec(p + 4);
			zip->entry->uncompressed_size = archive_le64dec(p + 12);
			zip->unconsumed += 20;
		} else {
			zip->entry->crc32 = archive_le32dec(p);
			zip->entry->compressed_size = archive_le32dec(p + 4);
			zip->entry->uncompressed_size = archive_le32dec(p + 8);
			zip->unconsumed += 12;
		}
	}

	return (ARCHIVE_OK);
}
#endif

static int
archive_read_format_zip_read_data(struct archive_read *a,
    const void **buff, size_t *size, int64_t *offset)
{
	int r;
	struct zip *zip = (struct zip *)(a->format->data);

	if (zip->has_encrypted_entries == ARCHIVE_READ_FORMAT_ENCRYPTION_DONT_KNOW) {
		zip->has_encrypted_entries = 0;
	}

	*offset = zip->entry_uncompressed_bytes_read;
	*size = 0;
	*buff = NULL;

	/* If we hit end-of-entry last time, return ARCHIVE_EOF. */
	if (zip->end_of_entry)
		return (ARCHIVE_EOF);

	/* Return EOF immediately if this is a non-regular file. */
	if (AE_IFREG != (zip->entry->mode & AE_IFMT))
		return (ARCHIVE_EOF);

	if (zip->entry->zip_flags & (ZIP_ENCRYPTED | ZIP_STRONG_ENCRYPTED)) {
		zip->has_encrypted_entries = 1;
		archive_set_error(&a->archive, ARCHIVE_ERRNO_FILE_FORMAT,
		    "Encrypted file is unsupported");
		return (ARCHIVE_FAILED);
	}

	__archive_read_consume(a, zip->unconsumed);
	zip->unconsumed = 0;

	switch(zip->entry->compression) {
	case 0:  /* No compression. */
		r =  zip_read_data_none(a, buff, size, offset);
		break;
#ifdef HAVE_ZLIB_H
	case 8: /* Deflate compression. */
		r =  zip_read_data_deflate(a, buff, size, offset);
		break;
#endif
	default: /* Unsupported compression. */
		/* Return a warning. */
		archive_set_error(&a->archive, ARCHIVE_ERRNO_FILE_FORMAT,
		    "Unsupported ZIP compression method (%s)",
		    compression_name(zip->entry->compression));
		/* We can't decompress this entry, but we will
		 * be able to skip() it and try the next entry. */
		return (ARCHIVE_FAILED);
		break;
	}
	if (r != ARCHIVE_OK)
		return (r);
	/* Update checksum */
	if (*size)
		zip->entry_crc32 = zip->crc32func(zip->entry_crc32, *buff,
		    (unsigned)*size);
	/* If we hit the end, swallow any end-of-data marker. */
	if (zip->end_of_entry) {
		/* Check file size, CRC against these values. */
		if (zip->entry->compressed_size !=
		    zip->entry_compressed_bytes_read) {
			archive_set_error(&a->archive, ARCHIVE_ERRNO_MISC,
			    "ZIP compressed data is wrong size "
			    "(read %jd, expected %jd)",
			    (intmax_t)zip->entry_compressed_bytes_read,
			    (intmax_t)zip->entry->compressed_size);
			return (ARCHIVE_WARN);
		}
		/* Size field only stores the lower 32 bits of the actual
		 * size. */
		if ((zip->entry->uncompressed_size & UINT32_MAX)
		    != (zip->entry_uncompressed_bytes_read & UINT32_MAX)) {
			archive_set_error(&a->archive, ARCHIVE_ERRNO_MISC,
			    "ZIP uncompressed data is wrong size "
			    "(read %jd, expected %jd)\n",
			    (intmax_t)zip->entry_uncompressed_bytes_read,
			    (intmax_t)zip->entry->uncompressed_size);
			return (ARCHIVE_WARN);
		}
		/* Check computed CRC against header */
		if (zip->entry->crc32 != zip->entry_crc32
		    && !zip->ignore_crc32) {
			archive_set_error(&a->archive, ARCHIVE_ERRNO_MISC,
			    "ZIP bad CRC: 0x%lx should be 0x%lx",
			    (unsigned long)zip->entry_crc32,
			    (unsigned long)zip->entry->crc32);
			return (ARCHIVE_WARN);
		}
	}

	return (ARCHIVE_OK);
}

static int
archive_read_format_zip_cleanup(struct archive_read *a)
{
	struct zip *zip;
	struct zip_entry *zip_entry, *next_zip_entry;

	zip = (struct zip *)(a->format->data);
#ifdef HAVE_ZLIB_H
	if (zip->stream_valid)
		inflateEnd(&zip->stream);
	free(zip->uncompressed_buffer);
#endif
	if (zip->zip_entries) {
		zip_entry = zip->zip_entries;
		while (zip_entry != NULL) {
			next_zip_entry = zip_entry->next;
			archive_string_free(&zip_entry->rsrcname);
			free(zip_entry);
			zip_entry = next_zip_entry;
		}
	}
	free(zip);
	(a->format->data) = NULL;
	return (ARCHIVE_OK);
}

static int
archive_read_format_zip_has_encrypted_entries(struct archive_read *_a)
{
	if (_a && _a->format) {
		struct zip * zip = (struct zip *)_a->format->data;
		if (zip) {
			return zip->has_encrypted_entries;
		}
	}
	return ARCHIVE_READ_FORMAT_ENCRYPTION_DONT_KNOW;
}

static int
archive_read_format_zip_options(struct archive_read *a,
    const char *key, const char *val)
{
	struct zip *zip;
	int ret = ARCHIVE_FAILED;

	zip = (struct zip *)(a->format->data);
	if (strcmp(key, "compat-2x")  == 0) {
		/* Handle filenames as libarchive 2.x */
		zip->init_default_conversion = (val != NULL) ? 1 : 0;
		return (ARCHIVE_OK);
	} else if (strcmp(key, "hdrcharset")  == 0) {
		if (val == NULL || val[0] == 0)
			archive_set_error(&a->archive, ARCHIVE_ERRNO_MISC,
			    "zip: hdrcharset option needs a character-set name"
			);
		else {
			zip->sconv = archive_string_conversion_from_charset(
			    &a->archive, val, 0);
			if (zip->sconv != NULL) {
				if (strcmp(val, "UTF-8") == 0)
					zip->sconv_utf8 = zip->sconv;
				ret = ARCHIVE_OK;
			} else
				ret = ARCHIVE_FATAL;
		}
		return (ret);
	} else if (strcmp(key, "ignorecrc32") == 0) {
		/* Mostly useful for testing. */
		if (val == NULL || val[0] == 0) {
			zip->crc32func = real_crc32;
			zip->ignore_crc32 = 0;
		} else {
			zip->crc32func = fake_crc32;
			zip->ignore_crc32 = 1;
		}
		return (ARCHIVE_OK);
	} else if (strcmp(key, "mac-ext") == 0) {
		zip->process_mac_extensions = (val != NULL && val[0] != 0);
		return (ARCHIVE_OK);
	}

	/* Note: The "warn" return is just to inform the options
	 * supervisor that we didn't handle it.  It will generate
	 * a suitable error if no one used this option. */
	return (ARCHIVE_WARN);
}

int
archive_read_support_format_zip(struct archive *a)
{
	int r;
	r = archive_read_support_format_zip_streamable(a);
	if (r != ARCHIVE_OK)
		return r;
	return (archive_read_support_format_zip_seekable(a));
}

/* ------------------------------------------------------------------------ */

/*
 * Streaming-mode support
 */


static int
archive_read_support_format_zip_capabilities_streamable(struct archive_read * a)
{
	(void)a; /* UNUSED */
	return (ARCHIVE_READ_FORMAT_CAPS_ENCRYPT_DATA | ARCHIVE_READ_FORMAT_CAPS_ENCRYPT_METADATA);
}

static int
archive_read_format_zip_streamable_bid(struct archive_read *a, int best_bid)
{
	const char *p;

	(void)best_bid; /* UNUSED */

	if ((p = __archive_read_ahead(a, 4, NULL)) == NULL)
		return (-1);

	/*
	 * Bid of 29 here comes from:
	 *  + 16 bits for "PK",
	 *  + next 16-bit field has 6 options so contributes
	 *    about 16 - log_2(6) ~= 16 - 2.6 ~= 13 bits
	 *
	 * So we've effectively verified ~29 total bits of check data.
	 */
	if (p[0] == 'P' && p[1] == 'K') {
		if ((p[2] == '\001' && p[3] == '\002')
		    || (p[2] == '\003' && p[3] == '\004')
		    || (p[2] == '\005' && p[3] == '\006')
		    || (p[2] == '\006' && p[3] == '\006')
		    || (p[2] == '\007' && p[3] == '\010')
		    || (p[2] == '0' && p[3] == '0'))
			return (29);
	}

	/* TODO: It's worth looking ahead a little bit for a valid
	 * PK signature.  In particular, that would make it possible
	 * to read some UUEncoded SFX files or SFX files coming from
	 * a network socket. */

	return (0);
}

static int
archive_read_format_zip_streamable_read_header(struct archive_read *a,
    struct archive_entry *entry)
{
	struct zip *zip;

	a->archive.archive_format = ARCHIVE_FORMAT_ZIP;
	if (a->archive.archive_format_name == NULL)
		a->archive.archive_format_name = "ZIP";

	zip = (struct zip *)(a->format->data);

	/*
	 * It should be sufficient to call archive_read_next_header() for
	 * a reader to determine if an entry is encrypted or not. If the
	 * encryption of an entry is only detectable when calling
	 * archive_read_data(), so be it. We'll do the same check there
	 * as well.
	 */
	if (zip->has_encrypted_entries == ARCHIVE_READ_FORMAT_ENCRYPTION_DONT_KNOW) {
		zip->has_encrypted_entries = 0;
	}

	/* Make sure we have a zip_entry structure to use. */
	if (zip->zip_entries == NULL) {
		zip->zip_entries = malloc(sizeof(struct zip_entry));
		if (zip->zip_entries == NULL) {
			archive_set_error(&a->archive, ENOMEM,
			    "Out  of memory");
			return ARCHIVE_FATAL;
		}
	}
	zip->entry = zip->zip_entries;
	memset(zip->entry, 0, sizeof(struct zip_entry));

	/* Search ahead for the next local file header. */
	__archive_read_consume(a, zip->unconsumed);
	zip->unconsumed = 0;
	for (;;) {
		int64_t skipped = 0;
		const char *p, *end;
		ssize_t bytes;

		p = __archive_read_ahead(a, 4, &bytes);
		if (p == NULL)
			return (ARCHIVE_FATAL);
		end = p + bytes;

		while (p + 4 <= end) {
			if (p[0] == 'P' && p[1] == 'K') {
				if (p[2] == '\003' && p[3] == '\004') {
					/* Regular file entry. */
					__archive_read_consume(a, skipped);
					return zip_read_local_file_header(a,
					    entry, zip);
				}

                              /*
                               * TODO: We cannot restore permissions
                               * based only on the local file headers.
                               * Consider scanning the central
                               * directory and returning additional
                               * entries for at least directories.
                               * This would allow us to properly set
                               * directory permissions.
			       *
			       * This won't help us fix symlinks
			       * and may not help with regular file
			       * permissions, either.  <sigh>
                               */
                              if (p[2] == '\001' && p[3] == '\002') {
                                      return (ARCHIVE_EOF);
                              }

                              /* End of central directory?  Must be an
                               * empty archive. */
                              if ((p[2] == '\005' && p[3] == '\006')
                                  || (p[2] == '\006' && p[3] == '\006'))
                                      return (ARCHIVE_EOF);
			}
			++p;
			++skipped;
		}
		__archive_read_consume(a, skipped);
	}
}

static int
archive_read_format_zip_read_data_skip_streamable(struct archive_read *a)
{
	struct zip *zip;
	int64_t bytes_skipped;

	zip = (struct zip *)(a->format->data);
	bytes_skipped = __archive_read_consume(a, zip->unconsumed);
	zip->unconsumed = 0;
	if (bytes_skipped < 0)
		return (ARCHIVE_FATAL);

	/* If we've already read to end of data, we're done. */
	if (zip->end_of_entry)
		return (ARCHIVE_OK);

	/* So we know we're streaming... */
	if (0 == (zip->entry->zip_flags & ZIP_LENGTH_AT_END)
	    || zip->entry->compressed_size > 0) {
		/* We know the compressed length, so we can just skip. */
		bytes_skipped = __archive_read_consume(a, zip->entry_bytes_remaining);
		if (bytes_skipped < 0)
			return (ARCHIVE_FATAL);
		return (ARCHIVE_OK);
	}

	/* We're streaming and we don't know the length. */
	/* If the body is compressed and we know the format, we can
	 * find an exact end-of-entry by decompressing it. */
	switch (zip->entry->compression) {
#ifdef HAVE_ZLIB_H
	case 8: /* Deflate compression. */
		while (!zip->end_of_entry) {
			int64_t offset = 0;
			const void *buff = NULL;
			size_t size = 0;
			int r;
			r =  zip_read_data_deflate(a, &buff, &size, &offset);
			if (r != ARCHIVE_OK)
				return (r);
		}
		return ARCHIVE_OK;
#endif
	default: /* Uncompressed or unknown. */
		/* Scan for a PK\007\010 signature. */
		for (;;) {
			const char *p, *buff;
			ssize_t bytes_avail;
			buff = __archive_read_ahead(a, 16, &bytes_avail);
			if (bytes_avail < 16) {
				archive_set_error(&a->archive,
				    ARCHIVE_ERRNO_FILE_FORMAT,
				    "Truncated ZIP file data");
				return (ARCHIVE_FATAL);
			}
			p = buff;
			while (p <= buff + bytes_avail - 16) {
				if (p[3] == 'P') { p += 3; }
				else if (p[3] == 'K') { p += 2; }
				else if (p[3] == '\007') { p += 1; }
				else if (p[3] == '\010' && p[2] == '\007'
				    && p[1] == 'K' && p[0] == 'P') {
					if (zip->entry->flags & LA_USED_ZIP64)
						__archive_read_consume(a, p - buff + 24);
					else
						__archive_read_consume(a, p - buff + 16);
					return ARCHIVE_OK;
				} else { p += 4; }
			}
			__archive_read_consume(a, p - buff);
		}
	}
}

int
archive_read_support_format_zip_streamable(struct archive *_a)
{
	struct archive_read *a = (struct archive_read *)_a;
	struct zip *zip;
	int r;

	archive_check_magic(_a, ARCHIVE_READ_MAGIC,
	    ARCHIVE_STATE_NEW, "archive_read_support_format_zip");

	zip = (struct zip *)malloc(sizeof(*zip));
	if (zip == NULL) {
		archive_set_error(&a->archive, ENOMEM,
		    "Can't allocate zip data");
		return (ARCHIVE_FATAL);
	}
	memset(zip, 0, sizeof(*zip));

	/* Streamable reader doesn't support mac extensions. */
	zip->process_mac_extensions = 0;

	/*
	 * Until enough data has been read, we cannot tell about
	 * any encrypted entries yet.
	 */
	zip->has_encrypted_entries = ARCHIVE_READ_FORMAT_ENCRYPTION_DONT_KNOW;
	zip->crc32func = real_crc32;

	r = __archive_read_register_format(a,
	    zip,
	    "zip",
	    archive_read_format_zip_streamable_bid,
	    archive_read_format_zip_options,
	    archive_read_format_zip_streamable_read_header,
	    archive_read_format_zip_read_data,
	    archive_read_format_zip_read_data_skip_streamable,
	    NULL,
	    archive_read_format_zip_cleanup,
	    archive_read_support_format_zip_capabilities_streamable,
	    archive_read_format_zip_has_encrypted_entries);

	if (r != ARCHIVE_OK)
		free(zip);
	return (ARCHIVE_OK);
}

/* ------------------------------------------------------------------------ */

/*
 * Seeking-mode support
 */

static int
archive_read_support_format_zip_capabilities_seekable(struct archive_read * a)
{
	(void)a; /* UNUSED */
	return (ARCHIVE_READ_FORMAT_CAPS_ENCRYPT_DATA | ARCHIVE_READ_FORMAT_CAPS_ENCRYPT_METADATA);
}

/*
 * TODO: This is a performance sink because it forces the read core to
 * drop buffered data from the start of file, which will then have to
 * be re-read again if this bidder loses.
 *
 * We workaround this a little by passing in the best bid so far so
 * that later bidders can do nothing if they know they'll never
 * outbid.  But we can certainly do better...
 */
static int
read_eocd(struct zip *zip, const char *p, int64_t current_offset)
{
	/* Sanity-check the EOCD we've found. */

	/* This must be the first volume. */
	if (archive_le16dec(p + 4) != 0)
		return 0;
	/* Central directory must be on this volume. */
	if (archive_le16dec(p + 4) != archive_le16dec(p + 6))
		return 0;
	/* All central directory entries must be on this volume. */
	if (archive_le16dec(p + 10) != archive_le16dec(p + 8))
		return 0;
	/* Central directory can't extend beyond start of EOCD record. */
	if (archive_le32dec(p + 16) + archive_le32dec(p + 12)
	    > current_offset)
		return 0;

	/* Save the central directory location for later use. */
	zip->central_directory_offset = archive_le32dec(p + 16);

	/* This is just a tiny bit higher than the maximum
	   returned by the streaming Zip bidder.  This ensures
	   that the more accurate seeking Zip parser wins
	   whenever seek is available. */
	return 32;
}

static int
read_zip64_eocd(struct archive_read *a, struct zip *zip, const char *p)
{
	int64_t eocd64_offset;
	int64_t eocd64_size;

	/* Sanity-check the locator record. */

	/* Central dir must be on first volume. */
	if (archive_le32dec(p + 4) != 0)
		return 0;
	/* Must be only a single volume. */
	if (archive_le32dec(p + 16) != 1)
		return 0;

	/* Find the Zip64 EOCD record. */
	eocd64_offset = archive_le64dec(p + 8);
	if (__archive_read_seek(a, eocd64_offset, SEEK_SET) < 0)
		return 0;
	if ((p = __archive_read_ahead(a, 56, NULL)) == NULL)
		return 0;
	/* Make sure we can read all of it. */
	eocd64_size = archive_le64dec(p + 4) + 12;
	if (eocd64_size < 56 || eocd64_size > 16384)
		return 0;
	if ((p = __archive_read_ahead(a, eocd64_size, NULL)) == NULL)
		return 0;

	/* Sanity-check the EOCD64 */
	if (archive_le32dec(p + 16) != 0) /* Must be disk #0 */
		return 0;
	if (archive_le32dec(p + 20) != 0) /* CD must be on disk #0 */
		return 0;
	/* CD can't be split. */
	if (archive_le64dec(p + 24) != archive_le64dec(p + 32))
		return 0;

	/* Save the central directory offset for later use. */
	zip->central_directory_offset = archive_le64dec(p + 48);

	return 32;
}

static int
archive_read_format_zip_seekable_bid(struct archive_read *a, int best_bid)
{
	struct zip *zip = (struct zip *)a->format->data;
	int64_t file_size, current_offset;
	const char *p;
	int i, tail;

	/* If someone has already bid more than 32, then avoid
	   trashing the look-ahead buffers with a seek. */
	if (best_bid > 32)
		return (-1);

	file_size = __archive_read_seek(a, 0, SEEK_END);
	if (file_size <= 0)
		return 0;

	/* Search last 16k of file for end-of-central-directory
	 * record (which starts with PK\005\006) or Zip64 locator
	 * record (which begins with PK\006\007) */
	tail = zipmin(1024 * 16, file_size);
	current_offset = __archive_read_seek(a, -tail, SEEK_END);
	if (current_offset < 0)
		return 0;
	if ((p = __archive_read_ahead(a, (size_t)tail, NULL)) == NULL)
		return 0;
	/* TODO: Rework this to search backwards from the end.  We
	 * normally expect the EOCD record to be at the very end, so
	 * that should be significantly faster.  Tricky part: Make
	 * sure we still prefer the Zip64 locator if it's present. */
	for (i = 0; i <= tail - 22;) {
		switch (p[i + 3]) {
		case 'P': i += 3; break;
		case 'K': i += 2; break;
		case 005: i += 1; break;
		case 006:
			if (memcmp(p + i, "PK\005\006", 4) == 0) {
				int ret = read_eocd(zip, p + i, current_offset + i);
				if (ret > 0)
					return (ret);
			}
			i += 1; /* Look for PK\006\007 next */
			break;
		case 007:
			if (memcmp(p + i, "PK\006\007", 4) == 0) {
				int ret = read_zip64_eocd(a, zip, p + i);
				if (ret > 0)
					return (ret);
			}
			i += 4;
			break;
		default: i += 4; break;
		}
	}
	return 0;
}

/* The red-black trees are only used in seeking mode to manage
 * the in-memory copy of the central directory. */

static int
cmp_node(const struct archive_rb_node *n1, const struct archive_rb_node *n2)
{
	const struct zip_entry *e1 = (const struct zip_entry *)n1;
	const struct zip_entry *e2 = (const struct zip_entry *)n2;

	if (e1->local_header_offset > e2->local_header_offset)
		return -1;
	if (e1->local_header_offset < e2->local_header_offset)
		return 1;
	return 0;
}

static int
cmp_key(const struct archive_rb_node *n, const void *key)
{
	/* This function won't be called */
	(void)n; /* UNUSED */
	(void)key; /* UNUSED */
	return 1;
}

static const struct archive_rb_tree_ops rb_ops = {
	&cmp_node, &cmp_key
};

static int
rsrc_cmp_node(const struct archive_rb_node *n1,
    const struct archive_rb_node *n2)
{
	const struct zip_entry *e1 = (const struct zip_entry *)n1;
	const struct zip_entry *e2 = (const struct zip_entry *)n2;

	return (strcmp(e2->rsrcname.s, e1->rsrcname.s));
}

static int
rsrc_cmp_key(const struct archive_rb_node *n, const void *key)
{
	const struct zip_entry *e = (const struct zip_entry *)n;
	return (strcmp((const char *)key, e->rsrcname.s));
}

static const struct archive_rb_tree_ops rb_rsrc_ops = {
	&rsrc_cmp_node, &rsrc_cmp_key
};

static const char *
rsrc_basename(const char *name, size_t name_length)
{
	const char *s, *r;

	r = s = name;
	for (;;) {
		s = memchr(s, '/', name_length - (s - name));
		if (s == NULL)
			break;
		r = ++s;
	}
	return (r);
}

static void
expose_parent_dirs(struct zip *zip, const char *name, size_t name_length)
{
	struct archive_string str;
	struct zip_entry *dir;
	char *s;

	archive_string_init(&str);
	archive_strncpy(&str, name, name_length);
	for (;;) {
		s = strrchr(str.s, '/');
		if (s == NULL)
			break;
		*s = '\0';
		/* Transfer the parent directory from zip->tree_rsrc RB
		 * tree to zip->tree RB tree to expose. */
		dir = (struct zip_entry *)
		    __archive_rb_tree_find_node(&zip->tree_rsrc, str.s);
		if (dir == NULL)
			break;
		__archive_rb_tree_remove_node(&zip->tree_rsrc, &dir->node);
		archive_string_free(&dir->rsrcname);
		__archive_rb_tree_insert_node(&zip->tree, &dir->node);
	}
	archive_string_free(&str);
}

static int
slurp_central_directory(struct archive_read *a, struct zip *zip)
{
	ssize_t i;
	unsigned found;
	int64_t correction;
	ssize_t bytes_avail;
	const char *p;

	/*
	 * Find the start of the central directory.  The end-of-CD
	 * record has our starting point, but there are lots of
	 * Zip archives which have had other data prepended to the
	 * file, which makes the recorded offsets all too small.
	 * So we search forward from the specified offset until we
	 * find the real start of the central directory.  Then we
	 * know the correction we need to apply to account for leading
	 * padding.
	 */
	if (__archive_read_seek(a, zip->central_directory_offset, SEEK_SET) < 0)
		return ARCHIVE_FATAL;

	found = 0;
	while (!found) {
		if ((p = __archive_read_ahead(a, 20, &bytes_avail)) == NULL)
			return ARCHIVE_FATAL;
		for (found = 0, i = 0; !found && i < bytes_avail - 4;) {
			switch (p[i + 3]) {
			case 'P': i += 3; break;
			case 'K': i += 2; break;
			case 001: i += 1; break;
			case 002:
				if (memcmp(p + i, "PK\001\002", 4) == 0) {
					p += i;
					found = 1;
				} else
					i += 4;
				break;
			case 005: i += 1; break;
			case 006:
				if (memcmp(p + i, "PK\005\006", 4) == 0) {
					p += i;
					found = 1;
				} else if (memcmp(p + i, "PK\006\006", 4) == 0) {
					p += i;
					found = 1;
				} else
					i += 1;
				break;
			default: i += 4; break;
			}
		}
		__archive_read_consume(a, i);
	}
	correction = archive_filter_bytes(&a->archive, 0) - zip->central_directory_offset;

	__archive_rb_tree_init(&zip->tree, &rb_ops);
	__archive_rb_tree_init(&zip->tree_rsrc, &rb_rsrc_ops);

	zip->central_directory_entries_total = 0;
	while (1) {
		struct zip_entry *zip_entry;
		size_t filename_length, extra_length, comment_length;
		uint32_t external_attributes;
		const char *name, *r;

		if ((p = __archive_read_ahead(a, 4, NULL)) == NULL)
			return ARCHIVE_FATAL;
		if (memcmp(p, "PK\006\006", 4) == 0
		    || memcmp(p, "PK\005\006", 4) == 0) {
			break;
		} else if (memcmp(p, "PK\001\002", 4) != 0) {
			archive_set_error(&a->archive,
			    -1, "Invalid central directory signature");
			return ARCHIVE_FATAL;
		}
		if ((p = __archive_read_ahead(a, 46, NULL)) == NULL)
			return ARCHIVE_FATAL;

		zip_entry = calloc(1, sizeof(struct zip_entry));
		zip_entry->next = zip->zip_entries;
		zip_entry->flags |= LA_FROM_CENTRAL_DIRECTORY;
		zip->zip_entries = zip_entry;
		zip->central_directory_entries_total++;

		/* version = p[4]; */
		zip_entry->system = p[5];
		/* version_required = archive_le16dec(p + 6); */
		zip_entry->zip_flags = archive_le16dec(p + 8);
		if (zip_entry->zip_flags & (ZIP_ENCRYPTED | ZIP_STRONG_ENCRYPTED)){
			zip->has_encrypted_entries = 1;
		}
		zip_entry->compression = (char)archive_le16dec(p + 10);
		zip_entry->mtime = zip_time(p + 12);
		zip_entry->crc32 = archive_le32dec(p + 16);
		zip_entry->compressed_size = archive_le32dec(p + 20);
		zip_entry->uncompressed_size = archive_le32dec(p + 24);
		filename_length = archive_le16dec(p + 28);
		extra_length = archive_le16dec(p + 30);
		comment_length = archive_le16dec(p + 32);
		/* disk_start = archive_le16dec(p + 34); */ /* Better be zero. */
		/* internal_attributes = archive_le16dec(p + 36); */ /* text bit */
		external_attributes = archive_le32dec(p + 38);
		zip_entry->local_header_offset =
		    archive_le32dec(p + 42) + correction;

		/* If we can't guess the mode, leave it zero here;
		   when we read the local file header we might get
		   more information. */
		zip_entry->mode = 0;
		if (zip_entry->system == 3) {
			zip_entry->mode = external_attributes >> 16;
		}

		/* We're done with the regular data; get the filename and
		 * extra data. */
		__archive_read_consume(a, 46);
		if ((p = __archive_read_ahead(a, filename_length + extra_length, NULL))
		    == NULL) {
			archive_set_error(&a->archive, ARCHIVE_ERRNO_FILE_FORMAT,
			    "Truncated ZIP file header");
			return ARCHIVE_FATAL;
		}
		process_extra(p + filename_length, extra_length, zip_entry);

		/*
		 * Mac resource fork files are stored under the
		 * "__MACOSX/" directory, so we should check if
		 * it is.
		 */
		if (!zip->process_mac_extensions) {
			/* Treat every entry as a regular entry. */
			__archive_rb_tree_insert_node(&zip->tree,
			    &zip_entry->node);
		} else {
			name = p;
			r = rsrc_basename(name, filename_length);
			if (filename_length >= 9 &&
			    strncmp("__MACOSX/", name, 9) == 0) {
				/* If this file is not a resource fork nor
				 * a directory. We should treat it as a non
				 * resource fork file to expose it. */
				if (name[filename_length-1] != '/' &&
				    (r - name < 3 || r[0] != '.' || r[1] != '_')) {
					__archive_rb_tree_insert_node(&zip->tree,
					    &zip_entry->node);
					/* Expose its parent directories. */
					expose_parent_dirs(zip, name, filename_length);
				} else {
					/* This file is a resource fork file or
					 * a directory. */
					archive_strncpy(&(zip_entry->rsrcname), name,
					    filename_length);
					__archive_rb_tree_insert_node(&zip->tree_rsrc,
					    &zip_entry->node);
				}
			} else {
				/* Generate resource fork name to find its resource
				 * file at zip->tree_rsrc. */
				archive_strcpy(&(zip_entry->rsrcname), "__MACOSX/");
				archive_strncat(&(zip_entry->rsrcname), name, r - name);
				archive_strcat(&(zip_entry->rsrcname), "._");
				archive_strncat(&(zip_entry->rsrcname),
				    name + (r - name), filename_length - (r - name));
				/* Register an entry to RB tree to sort it by
				 * file offset. */
				__archive_rb_tree_insert_node(&zip->tree,
				    &zip_entry->node);
			}
		}

		/* Skip the comment too ... */
		__archive_read_consume(a,
		    filename_length + extra_length + comment_length);
	}

	return ARCHIVE_OK;
}

static ssize_t
zip_get_local_file_header_size(struct archive_read *a, size_t extra)
{
	const char *p;
	ssize_t filename_length, extra_length;

	if ((p = __archive_read_ahead(a, extra + 30, NULL)) == NULL) {
		archive_set_error(&a->archive, ARCHIVE_ERRNO_FILE_FORMAT,
		    "Truncated ZIP file header");
		return (ARCHIVE_WARN);
	}
	p += extra;

	if (memcmp(p, "PK\003\004", 4) != 0) {
		archive_set_error(&a->archive, -1, "Damaged Zip archive");
		return ARCHIVE_WARN;
	}
	filename_length = archive_le16dec(p + 26);
	extra_length = archive_le16dec(p + 28);

	return (30 + filename_length + extra_length);
}

static int
zip_read_mac_metadata(struct archive_read *a, struct archive_entry *entry,
    struct zip_entry *rsrc)
{
	struct zip *zip = (struct zip *)a->format->data;
	unsigned char *metadata, *mp;
	int64_t offset = archive_filter_bytes(&a->archive, 0);
	size_t remaining_bytes, metadata_bytes;
	ssize_t hsize;
	int ret = ARCHIVE_OK, eof;

	switch(rsrc->compression) {
	case 0:  /* No compression. */
#ifdef HAVE_ZLIB_H
	case 8: /* Deflate compression. */
#endif
		break;
	default: /* Unsupported compression. */
		/* Return a warning. */
		archive_set_error(&a->archive, ARCHIVE_ERRNO_FILE_FORMAT,
		    "Unsupported ZIP compression method (%s)",
		    compression_name(rsrc->compression));
		/* We can't decompress this entry, but we will
		 * be able to skip() it and try the next entry. */
		return (ARCHIVE_WARN);
	}

	if (rsrc->uncompressed_size > (4 * 1024 * 1024)) {
		archive_set_error(&a->archive, ARCHIVE_ERRNO_FILE_FORMAT,
		    "Mac metadata is too large: %jd > 4M bytes",
		    (intmax_t)rsrc->uncompressed_size);
		return (ARCHIVE_WARN);
	}

	metadata = malloc((size_t)rsrc->uncompressed_size);
	if (metadata == NULL) {
		archive_set_error(&a->archive, ENOMEM,
		    "Can't allocate memory for Mac metadata");
		return (ARCHIVE_FATAL);
	}

	if (offset < rsrc->local_header_offset)
		__archive_read_consume(a, rsrc->local_header_offset - offset);
	else if (offset != rsrc->local_header_offset) {
		__archive_read_seek(a, rsrc->local_header_offset, SEEK_SET);
	}

	hsize = zip_get_local_file_header_size(a, 0);
	__archive_read_consume(a, hsize);

	remaining_bytes = (size_t)rsrc->compressed_size;
	metadata_bytes = (size_t)rsrc->uncompressed_size;
	mp = metadata;
	eof = 0;
	while (!eof && remaining_bytes) {
		const unsigned char *p;
		ssize_t bytes_avail;
		size_t bytes_used;

		p = __archive_read_ahead(a, 1, &bytes_avail);
		if (p == NULL) {
			archive_set_error(&a->archive,
			    ARCHIVE_ERRNO_FILE_FORMAT,
			    "Truncated ZIP file header");
			ret = ARCHIVE_WARN;
			goto exit_mac_metadata;
		}
		if ((size_t)bytes_avail > remaining_bytes)
			bytes_avail = remaining_bytes;
		switch(rsrc->compression) {
		case 0:  /* No compression. */
			memcpy(mp, p, bytes_avail);
			bytes_used = (size_t)bytes_avail;
			metadata_bytes -= bytes_used;
			mp += bytes_used;
			if (metadata_bytes == 0)
				eof = 1;
			break;
#ifdef HAVE_ZLIB_H
		case 8: /* Deflate compression. */
		{
			int r;

			ret = zip_deflate_init(a, zip);
			if (ret != ARCHIVE_OK)
				goto exit_mac_metadata;
			zip->stream.next_in =
			    (Bytef *)(uintptr_t)(const void *)p;
			zip->stream.avail_in = (uInt)bytes_avail;
			zip->stream.total_in = 0;
			zip->stream.next_out = mp;
			zip->stream.avail_out = (uInt)metadata_bytes;
			zip->stream.total_out = 0;

			r = inflate(&zip->stream, 0);
			switch (r) {
			case Z_OK:
				break;
			case Z_STREAM_END:
				eof = 1;
				break;
			case Z_MEM_ERROR:
				archive_set_error(&a->archive, ENOMEM,
				    "Out of memory for ZIP decompression");
				ret = ARCHIVE_FATAL;
				goto exit_mac_metadata;
			default:
				archive_set_error(&a->archive,
				    ARCHIVE_ERRNO_MISC,
				    "ZIP decompression failed (%d)", r);
				ret = ARCHIVE_FATAL;
				goto exit_mac_metadata;
			}
			bytes_used = zip->stream.total_in;
			metadata_bytes -= zip->stream.total_out;
			mp += zip->stream.total_out;
			break;
		}
#endif
		default:
			bytes_used = 0;
			break;
		}
		__archive_read_consume(a, bytes_used);
		remaining_bytes -= bytes_used;
	}
	archive_entry_copy_mac_metadata(entry, metadata,
	    (size_t)rsrc->uncompressed_size - metadata_bytes);

exit_mac_metadata:
	__archive_read_seek(a, offset, SEEK_SET);
	zip->decompress_init = 0;
	free(metadata);
	return (ret);
}

static int
archive_read_format_zip_seekable_read_header(struct archive_read *a,
	struct archive_entry *entry)
{
	struct zip *zip = (struct zip *)a->format->data;
	struct zip_entry *rsrc;
	int64_t offset;
	int r, ret = ARCHIVE_OK;

	/*
	 * It should be sufficient to call archive_read_next_header() for
	 * a reader to determine if an entry is encrypted or not. If the
	 * encryption of an entry is only detectable when calling
	 * archive_read_data(), so be it. We'll do the same check there
	 * as well.
	 */
	if (zip->has_encrypted_entries == ARCHIVE_READ_FORMAT_ENCRYPTION_DONT_KNOW) {
		zip->has_encrypted_entries = 0;
	}

	a->archive.archive_format = ARCHIVE_FORMAT_ZIP;
	if (a->archive.archive_format_name == NULL)
		a->archive.archive_format_name = "ZIP";

	if (zip->zip_entries == NULL) {
		r = slurp_central_directory(a, zip);
		if (r != ARCHIVE_OK)
			return r;
		/* Get first entry whose local header offset is lower than
		 * other entries in the archive file. */
		zip->entry =
		    (struct zip_entry *)ARCHIVE_RB_TREE_MIN(&zip->tree);
	} else if (zip->entry != NULL) {
		/* Get next entry in local header offset order. */
		zip->entry = (struct zip_entry *)__archive_rb_tree_iterate(
		    &zip->tree, &zip->entry->node, ARCHIVE_RB_DIR_RIGHT);
	}

	if (zip->entry == NULL)
		return ARCHIVE_EOF;

	if (zip->entry->rsrcname.s)
		rsrc = (struct zip_entry *)__archive_rb_tree_find_node(
		    &zip->tree_rsrc, zip->entry->rsrcname.s);
	else
		rsrc = NULL;

	/* File entries are sorted by the header offset, we should mostly
	 * use __archive_read_consume to advance a read point to avoid redundant
	 * data reading.  */
	offset = archive_filter_bytes(&a->archive, 0);
	if (offset < zip->entry->local_header_offset)
		__archive_read_consume(a,
		    zip->entry->local_header_offset - offset);
	else if (offset != zip->entry->local_header_offset) {
		__archive_read_seek(a, zip->entry->local_header_offset, SEEK_SET);
	}
	zip->unconsumed = 0;
	r = zip_read_local_file_header(a, entry, zip);
	if (r != ARCHIVE_OK)
		return r;
	if (rsrc) {
		int ret2 = zip_read_mac_metadata(a, entry, rsrc);
		if (ret2 < ret)
			ret = ret2;
	}
	return (ret);
}

/*
 * We're going to seek for the next header anyway, so we don't
 * need to bother doing anything here.
 */
static int
archive_read_format_zip_read_data_skip_seekable(struct archive_read *a)
{
	struct zip *zip;
	zip = (struct zip *)(a->format->data);

	zip->unconsumed = 0;
	return (ARCHIVE_OK);
}

int
archive_read_support_format_zip_seekable(struct archive *_a)
{
	struct archive_read *a = (struct archive_read *)_a;
	struct zip *zip;
	int r;

	archive_check_magic(_a, ARCHIVE_READ_MAGIC,
	    ARCHIVE_STATE_NEW, "archive_read_support_format_zip_seekable");

	zip = (struct zip *)malloc(sizeof(*zip));
	if (zip == NULL) {
		archive_set_error(&a->archive, ENOMEM,
		    "Can't allocate zip data");
		return (ARCHIVE_FATAL);
	}
	memset(zip, 0, sizeof(*zip));

#ifdef HAVE_COPYFILE_H
	/* Set this by default on Mac OS. */
	zip->process_mac_extensions = 1;
#endif

	/*
	 * Until enough data has been read, we cannot tell about
	 * any encrypted entries yet.
	 */
	zip->has_encrypted_entries = ARCHIVE_READ_FORMAT_ENCRYPTION_DONT_KNOW;
	zip->crc32func = real_crc32;

	r = __archive_read_register_format(a,
	    zip,
	    "zip",
	    archive_read_format_zip_seekable_bid,
	    archive_read_format_zip_options,
	    archive_read_format_zip_seekable_read_header,
	    archive_read_format_zip_read_data,
	    archive_read_format_zip_read_data_skip_seekable,
	    NULL,
	    archive_read_format_zip_cleanup,
	    archive_read_support_format_zip_capabilities_seekable,
	    archive_read_format_zip_has_encrypted_entries);

	if (r != ARCHIVE_OK)
		free(zip);
	return (ARCHIVE_OK);
}
