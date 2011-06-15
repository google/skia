
/* pngvalid.c - validate libpng by constructing then reading png files.
 *
 * Last changed in libpng 1.5.2 [March 31, 2011]
 * Copyright (c) 2011 Glenn Randers-Pehrson
 * Written by John Cunningham Bowler
 *
 * This code is released under the libpng license.
 * For conditions of distribution and use, see the disclaimer
 * and license in png.h
 *
 * NOTES:
 *   This is a C program that is intended to be linked against libpng.  It
 *   generates bitmaps internally, stores them as PNG files (using the
 *   sequential write code) then reads them back (using the sequential
 *   read code) and validates that the result has the correct data.
 *
 *   The program can be modified and extended to test the correctness of
 *   transformations performed by libpng.
 */

#include "png.h"
#if PNG_LIBPNG_VER < 10500
/* This delibarately lacks the PNG_CONST. */
typedef png_byte *png_const_bytep;

/* This is copied from 1.5.1 png.h: */
#define PNG_INTERLACE_ADAM7_PASSES 7
#define PNG_PASS_START_ROW(pass) (((1U&~(pass))<<(3-((pass)>>1)))&7)
#define PNG_PASS_START_COL(pass) (((1U& (pass))<<(3-(((pass)+1)>>1)))&7)
#define PNG_PASS_ROW_SHIFT(pass) ((pass)>2?(8-(pass))>>1:3)
#define PNG_PASS_COL_SHIFT(pass) ((pass)>1?(7-(pass))>>1:3)
#define PNG_PASS_ROWS(height, pass) (((height)+(((1<<PNG_PASS_ROW_SHIFT(pass))\
   -1)-PNG_PASS_START_ROW(pass)))>>PNG_PASS_ROW_SHIFT(pass))
#define PNG_PASS_COLS(width, pass) (((width)+(((1<<PNG_PASS_COL_SHIFT(pass))\
   -1)-PNG_PASS_START_COL(pass)))>>PNG_PASS_COL_SHIFT(pass))
#define PNG_ROW_FROM_PASS_ROW(yIn, pass) \
   (((yIn)<<PNG_PASS_ROW_SHIFT(pass))+PNG_PASS_START_ROW(pass))
#define PNG_COL_FROM_PASS_COL(xIn, pass) \
   (((xIn)<<PNG_PASS_COL_SHIFT(pass))+PNG_PASS_START_COL(pass))
#define PNG_PASS_MASK(pass,off) ( \
   ((0x110145AFU>>(((7-(off))-(pass))<<2)) & 0xFU) | \
   ((0x01145AF0U>>(((7-(off))-(pass))<<2)) & 0xF0U))
#define PNG_ROW_IN_INTERLACE_PASS(y, pass) \
   ((PNG_PASS_MASK(pass,0) >> ((y)&7)) & 1)
#define PNG_COL_IN_INTERLACE_PASS(x, pass) \
   ((PNG_PASS_MASK(pass,1) >> ((x)&7)) & 1)

/* These are needed too for the defualt build: */
#define PNG_WRITE_16BIT_SUPPORTED
#define PNG_READ_16BIT_SUPPORTED
#endif

#include "zlib.h"   /* For crc32 */

#include <float.h>  /* For floating point constants */
#include <stdlib.h> /* For malloc */
#include <string.h> /* For memcpy, memset */
#include <math.h>   /* For floor */

/* Unused formal parameter errors are removed using the following macro which is
 * expected to have no bad effects on performance.
 */
#ifndef UNUSED
#  if defined(__GNUC__) || defined(_MSC_VER)
#     define UNUSED(param) (void)param;
#  else
#     define UNUSED(param)
#  endif
#endif

/***************************** EXCEPTION HANDLING *****************************/
#include "contrib/visupng/cexcept.h"
struct png_store;
define_exception_type(struct png_store*);

/* The following are macros to reduce typing everywhere where the well known
 * name 'the_exception_context' must be defined.
 */
#define anon_context(ps) struct exception_context *the_exception_context = \
   &(ps)->exception_context
#define context(ps,fault) anon_context(ps); png_store *fault

/******************************* UTILITIES ************************************/
/* Error handling is particularly problematic in production code - error
 * handlers often themselves have bugs which lead to programs that detect
 * minor errors crashing.  The following functions deal with one very
 * common class of errors in error handlers - attempting to format error or
 * warning messages into buffers that are too small.
 */
static size_t safecat(char *buffer, size_t bufsize, size_t pos,
   PNG_CONST char *cat)
{
   while (pos < bufsize && cat != NULL && *cat != 0)
      buffer[pos++] = *cat++;

   if (pos >= bufsize)
      pos = bufsize-1;

   buffer[pos] = 0;
   return pos;
}

static size_t safecatn(char *buffer, size_t bufsize, size_t pos, int n)
{
   char number[64];
   sprintf(number, "%d", n);
   return safecat(buffer, bufsize, pos, number);
}

static size_t safecatd(char *buffer, size_t bufsize, size_t pos, double d,
    int precision)
{
   char number[64];
   sprintf(number, "%.*f", precision, d);
   return safecat(buffer, bufsize, pos, number);
}

static PNG_CONST char invalid[] = "invalid";
static PNG_CONST char sep[] = ": ";

/* NOTE: this is indexed by ln2(bit_depth)! */
static PNG_CONST char *bit_depths[8] =
{
   "1", "2", "4", "8", "16", invalid, invalid, invalid
};

static PNG_CONST char *colour_types[8] =
{
   "greyscale", invalid, "truecolour", "indexed-colour",
   "greyscale with alpha", invalid, "truecolour with alpha", invalid
};

/* To get log-bit-depth from bit depth, returns 0 to 7 (7 on error). */
static unsigned int
log2depth(png_byte bit_depth)
{
   switch (bit_depth)
   {
      case 1:
         return 0;

      case 2:
         return 1;

      case 4:
         return 2;

      case 8:
         return 3;

      case 16:
         return 4;

      default:
         return 7;
   }
}

/* A numeric ID based on PNG file characteristics.  The 'do_interlace' field
 * simply records whether pngvalid did the interlace itself or whether it
 * was done by libpng.  Width and height must be less than 256.
 */
#define FILEID(col, depth, interlace, width, height, do_interlace) \
   ((png_uint_32)((col) + ((depth)<<3) + ((interlace)<<8) + \
    (((do_interlace)!=0)<<15) + ((width)<<16) + ((height)<<24)))

#define COL_FROM_ID(id) ((png_byte)((id)& 0x7U))
#define DEPTH_FROM_ID(id) ((png_byte)(((id) >> 3) & 0x1fU))
#define INTERLACE_FROM_ID(id) ((int)(((id) >> 8) & 0x3))
#define DO_INTERLACE_FROM_ID(id) ((int)(((id)>>15) & 1))
#define WIDTH_FROM_ID(id) (((id)>>16) & 0xff)
#define HEIGHT_FROM_ID(id) (((id)>>24) & 0xff)

/* Utility to construct a standard name for a standard image. */
static size_t
standard_name(char *buffer, size_t bufsize, size_t pos, png_byte colour_type,
    int log_bit_depth, int interlace_type, png_uint_32 w, png_uint_32 h,
    int do_interlace)
{
   pos = safecat(buffer, bufsize, pos, colour_types[colour_type]);
   pos = safecat(buffer, bufsize, pos, " ");
   pos = safecat(buffer, bufsize, pos, bit_depths[log_bit_depth]);
   pos = safecat(buffer, bufsize, pos, " bit ");

   if (interlace_type != PNG_INTERLACE_NONE)
      pos = safecat(buffer, bufsize, pos, "interlaced");
   if (do_interlace)
      pos = safecat(buffer, bufsize, pos, "(pngvalid)");
   else
      pos = safecat(buffer, bufsize, pos, "(libpng)");
   if (w > 0 || h > 0)
   {
      pos = safecat(buffer, bufsize, pos, " ");
      pos = safecatn(buffer, bufsize, pos, w);
      pos = safecat(buffer, bufsize, pos, "x");
      pos = safecatn(buffer, bufsize, pos, h);
   }

   return pos;
}

static size_t
standard_name_from_id(char *buffer, size_t bufsize, size_t pos, png_uint_32 id)
{
   return standard_name(buffer, bufsize, pos, COL_FROM_ID(id),
      log2depth(DEPTH_FROM_ID(id)), INTERLACE_FROM_ID(id),
      WIDTH_FROM_ID(id), HEIGHT_FROM_ID(id), DO_INTERLACE_FROM_ID(id));
}

/* Convenience API and defines to list valid formats.  Note that 16 bit read and
 * write support is required to do 16 bit read tests (we must be able to make a
 * 16 bit image to test!)
 */
#ifdef PNG_WRITE_16BIT_SUPPORTED
#  define WRITE_BDHI 4
#  ifdef PNG_READ_16BIT_SUPPORTED
#     define READ_BDHI 4
#     define DO_16BIT
#  endif
#else
#  define WRITE_BDHI 3
#endif
#ifndef DO_16BIT
#  define READ_BDHI 3
#endif

static int
next_format(png_bytep colour_type, png_bytep bit_depth)
{
   if (*bit_depth == 0)
   {
      *colour_type = 0, *bit_depth = 1;
      return 1;
   }

   *bit_depth = (png_byte)(*bit_depth << 1);

   /* Palette images are restricted to 8 bit depth */
   if (*bit_depth <= 8
#     ifdef DO_16BIT
         || (*colour_type != 3 && *bit_depth <= 16)
#     endif
      )
      return 1;

   /* Move to the next color type, or return 0 at the end. */
   switch (*colour_type)
   {
      case 0:
         *colour_type = 2;
         *bit_depth = 8;
         return 1;

      case 2:
         *colour_type = 3;
         *bit_depth = 1;
         return 1;

      case 3:
         *colour_type = 4;
         *bit_depth = 8;
         return 1;

      case 4:
         *colour_type = 6;
         *bit_depth = 8;
         return 1;

      default:
         return 0;
   }
}

static unsigned int
sample(png_const_bytep row, png_byte colour_type, png_byte bit_depth,
    png_uint_32 x, unsigned int sample_index)
{
   png_uint_32 bit_index, result;

   /* Find a sample index for the desired sample: */
   x *= bit_depth;
   bit_index = x;

   if ((colour_type & 1) == 0) /* !palette */
   {
      if (colour_type & 2)
         bit_index *= 3;

      if (colour_type & 4)
         bit_index += x; /* Alpha channel */

      if (colour_type & (2+4))
         bit_index += sample_index * bit_depth; /* Multiple channels: select one */
   }

   /* Return the sample from the row as an integer. */
   row += bit_index >> 3;
   result = *row;

   if (bit_depth == 8)
      return result;

   else if (bit_depth > 8)
      return (result << 8) + *++row;

   /* Less than 8 bits per sample. */
   bit_index &= 7;
   return (result >> (8-bit_index-bit_depth)) & ((1U<<bit_depth)-1);
}

/* Copy a single pixel, of a given size, from one buffer to another -
 * while this is basically bit addressed there is an implicit assumption
 * that pixels 8 or more bits in size are byte aligned and that pixels
 * do not otherwise cross byte boundaries.  (This is, so far as I know,
 * universally true in bitmap computer graphics.  [JCB 20101212])
 *
 * NOTE: The to and from buffers may be the same.
 */
static void
pixel_copy(png_bytep toBuffer, png_uint_32 toIndex,
   png_const_bytep fromBuffer, png_uint_32 fromIndex, unsigned int pixelSize)
{
   /* Assume we can multiply by 'size' without overflow because we are
    * just working in a single buffer.
    */
   toIndex *= pixelSize;
   fromIndex *= pixelSize;
   if (pixelSize < 8) /* Sub-byte */
   {
      /* Mask to select the location of the copied pixel: */
      unsigned int destMask = ((1U<<pixelSize)-1) << (8-pixelSize-(toIndex&7));
      /* The following read the entire pixels and clears the extra: */
      unsigned int destByte = toBuffer[toIndex >> 3] & ~destMask;
      unsigned int sourceByte = fromBuffer[fromIndex >> 3];

      /* Don't rely on << or >> supporting '0' here, just in case: */
      fromIndex &= 7;
      if (fromIndex > 0) sourceByte <<= fromIndex;
      if ((toIndex & 7) > 0) sourceByte >>= toIndex & 7;

      toBuffer[toIndex >> 3] = (png_byte)(destByte | (sourceByte & destMask));
   }
   else /* One or more bytes */
      memmove(toBuffer+(toIndex>>3), fromBuffer+(fromIndex>>3), pixelSize>>3);
}

/* Compare pixels - they are assumed to start at the first byte in the
 * given buffers.
 */
static int
pixel_cmp(png_const_bytep pa, png_const_bytep pb, png_uint_32 bit_width)
{
   if (memcmp(pa, pb, bit_width>>3) == 0)
   {
      png_uint_32 p;

      if ((bit_width & 7) == 0) return 0;

      /* Ok, any differences? */
      p = pa[bit_width >> 3];
      p ^= pb[bit_width >> 3];

      if (p == 0) return 0;

      /* There are, but they may not be significant, remove the bits
       * after the end (the low order bits in PNG.)
       */
      bit_width &= 7;
      p >>= 8-bit_width;

      if (p == 0) return 0;
   }

   return 1; /* Different */
}

/*************************** BASIC PNG FILE WRITING ***************************/
/* A png_store takes data from the sequential writer or provides data
 * to the sequential reader.  It can also store the result of a PNG
 * write for later retrieval.
 */
#define STORE_BUFFER_SIZE 500 /* arbitrary */
typedef struct png_store_buffer
{
   struct png_store_buffer*  prev;    /* NOTE: stored in reverse order */
   png_byte                  buffer[STORE_BUFFER_SIZE];
} png_store_buffer;

#define FILE_NAME_SIZE 64

typedef struct png_store_file
{
   struct png_store_file*  next;      /* as many as you like... */
   char                    name[FILE_NAME_SIZE];
   png_uint_32             id;        /* must be correct (see FILEID) */
   png_size_t              datacount; /* In this (the last) buffer */
   png_store_buffer        data;      /* Last buffer in file */
} png_store_file;

/* The following is a pool of memory allocated by a single libpng read or write
 * operation.
 */
typedef struct store_pool
{
   struct png_store    *store;   /* Back pointer */
   struct store_memory *list;    /* List of allocated memory */
   png_byte             mark[4]; /* Before and after data */

   /* Statistics for this run. */
   png_alloc_size_t     max;     /* Maximum single allocation */
   png_alloc_size_t     current; /* Current allocation */
   png_alloc_size_t     limit;   /* Highest current allocation */
   png_alloc_size_t     total;   /* Total allocation */

   /* Overall statistics (retained across successive runs). */
   png_alloc_size_t     max_max;
   png_alloc_size_t     max_limit;
   png_alloc_size_t     max_total;
} store_pool;

typedef struct png_store
{
   /* For cexcept.h exception handling - simply store one of these;
    * the context is a self pointer but it may point to a different
    * png_store (in fact it never does in this program.)
    */
   struct exception_context
                      exception_context;

   unsigned int       verbose :1;
   unsigned int       treat_warnings_as_errors :1;
   unsigned int       expect_error :1;
   unsigned int       expect_warning :1;
   unsigned int       saw_warning :1;
   unsigned int       speed :1;
   unsigned int       progressive :1; /* use progressive read */
   unsigned int       validated :1;   /* used as a temporary flag */
   int                nerrors;
   int                nwarnings;
   char               test[128]; /* Name of test */
   char               error[256];

   /* Read fields */
   png_structp        pread;    /* Used to read a saved file */
   png_infop          piread;
   png_store_file*    current;  /* Set when reading */
   png_store_buffer*  next;     /* Set when reading */
   png_size_t         readpos;  /* Position in *next */
   png_byte*          image;    /* Buffer for reading interlaced images */
   size_t             cb_image; /* Size of this buffer */
   store_pool         read_memory_pool;

   /* Write fields */
   png_store_file*    saved;
   png_structp        pwrite;   /* Used when writing a new file */
   png_infop          piwrite;
   png_size_t         writepos; /* Position in .new */
   char               wname[FILE_NAME_SIZE];
   png_store_buffer   new;      /* The end of the new PNG file being written. */
   store_pool         write_memory_pool;
} png_store;

/* Initialization and cleanup */
static void
store_pool_mark(png_byte *mark)
{
   /* Generate a new mark.  This uses a boring repeatable algorithm and it is
    * implemented here so that it gives the same set of numbers on every
    * architecture.  It's a linear congruential generator (Knuth or Sedgewick
    * "Algorithms") but it comes from the 'feedback taps' table in Horowitz and
    * Hill, "The Art of Electronics".
    */
   static png_uint_32 u0 = 0x12345678, u1 = 1;

   /* There are thirty three bits, the next bit in the sequence is bit-33 XOR
    * bit-20.  The top 1 bit is in u1, the bottom 32 are in u0.
    */
   int i;
   for (i=0; i<4; ++i)
   {
      /* First generate 8 new bits then shift them in at the end. */
      png_uint_32 u = ((u0 >> (20-8)) ^ ((u1 << 7) | (u0 >> (32-7)))) & 0xff;
      u1 <<= 8;
      u1 |= u0 >> 24;
      u0 <<= 8;
      u0 |= u;
      *mark++ = (png_byte)u;
   }
}

/* Use this for random 32 bit values, this function makes sure the result is
 * non-zero.
 */
static png_uint_32
random_32(void)
{

   for(;;)
   {
      png_byte mark[4];
      png_uint_32 result;

      store_pool_mark(mark);
      result = png_get_uint_32(mark);

      if (result != 0)
         return result;
   }
}

static void
store_pool_init(png_store *ps, store_pool *pool)
{
   memset(pool, 0, sizeof *pool);

   pool->store = ps;
   pool->list = NULL;
   pool->max = pool->current = pool->limit = pool->total = 0;
   pool->max_max = pool->max_limit = pool->max_total = 0;
   store_pool_mark(pool->mark);
}

static void
store_init(png_store* ps)
{
   memset(ps, 0, sizeof *ps);
   init_exception_context(&ps->exception_context);
   store_pool_init(ps, &ps->read_memory_pool);
   store_pool_init(ps, &ps->write_memory_pool);
   ps->verbose = 0;
   ps->treat_warnings_as_errors = 0;
   ps->expect_error = 0;
   ps->expect_warning = 0;
   ps->saw_warning = 0;
   ps->speed = 0;
   ps->progressive = 0;
   ps->validated = 0;
   ps->nerrors = ps->nwarnings = 0;
   ps->pread = NULL;
   ps->piread = NULL;
   ps->saved = ps->current = NULL;
   ps->next = NULL;
   ps->readpos = 0;
   ps->image = NULL;
   ps->cb_image = 0;
   ps->pwrite = NULL;
   ps->piwrite = NULL;
   ps->writepos = 0;
   ps->new.prev = NULL;
}

/* This somewhat odd function is used when reading an image to ensure that the
 * buffer is big enough - this is why a png_structp is available.
 */
static void
store_ensure_image(png_store *ps, png_structp pp, size_t cb)
{
   if (ps->cb_image < cb)
   {
      if (ps->image != NULL)
      {
         free(ps->image-1);
         ps->cb_image = 0;
      }

      /* The buffer is deliberately mis-aligned. */
      ps->image = malloc(cb+1);
      if (ps->image == NULL)
         png_error(pp, "OOM allocating image buffer");

      ++(ps->image);
      ps->cb_image = cb;
   }

   /* And, for error checking, the whole buffer is set to '1' - this
    * matches what happens with the 'size' test images on write and also
    * matches the unused bits in the test rows.
    */
   memset(ps->image, 0xff, cb);
}

static void
store_freebuffer(png_store_buffer* psb)
{
   if (psb->prev)
   {
      store_freebuffer(psb->prev);
      free(psb->prev);
      psb->prev = NULL;
   }
}

static void
store_freenew(png_store *ps)
{
   store_freebuffer(&ps->new);
   ps->writepos = 0;
}

static void
store_storenew(png_store *ps)
{
   png_store_buffer *pb;

   if (ps->writepos != STORE_BUFFER_SIZE)
      png_error(ps->pwrite, "invalid store call");

   pb = malloc(sizeof *pb);

   if (pb == NULL)
      png_error(ps->pwrite, "store new: OOM");

   *pb = ps->new;
   ps->new.prev = pb;
   ps->writepos = 0;
}

static void
store_freefile(png_store_file **ppf)
{
   if (*ppf != NULL)
   {
      store_freefile(&(*ppf)->next);

      store_freebuffer(&(*ppf)->data);
      (*ppf)->datacount = 0;
      free(*ppf);
      *ppf = NULL;
   }
}

/* Main interface to file storeage, after writing a new PNG file (see the API
 * below) call store_storefile to store the result with the given name and id.
 */
static void
store_storefile(png_store *ps, png_uint_32 id)
{
   png_store_file *pf = malloc(sizeof *pf);
   if (pf == NULL)
      png_error(ps->pwrite, "storefile: OOM");
   safecat(pf->name, sizeof pf->name, 0, ps->wname);
   pf->id = id;
   pf->data = ps->new;
   pf->datacount = ps->writepos;
   ps->new.prev = NULL;
   ps->writepos = 0;

   /* And save it. */
   pf->next = ps->saved;
   ps->saved = pf;
}

/* Generate an error message (in the given buffer) */
static size_t
store_message(png_store *ps, png_structp pp, char *buffer, size_t bufsize,
   size_t pos, PNG_CONST char *msg)
{
   if (pp != NULL && pp == ps->pread)
   {
      /* Reading a file */
      pos = safecat(buffer, bufsize, pos, "read: ");

      if (ps->current != NULL)
      {
         pos = safecat(buffer, bufsize, pos, ps->current->name);
         pos = safecat(buffer, bufsize, pos, sep);
      }
   }

   else if (pp != NULL && pp == ps->pwrite)
   {
      /* Writing a file */
      pos = safecat(buffer, bufsize, pos, "write: ");
      pos = safecat(buffer, bufsize, pos, ps->wname);
      pos = safecat(buffer, bufsize, pos, sep);
   }

   else
   {
      /* Neither reading nor writing (or a memory error in struct delete) */
      pos = safecat(buffer, bufsize, pos, "pngvalid: ");
   }

   if (ps->test[0] != 0)
   {
      pos = safecat(buffer, bufsize, pos, ps->test);
      pos = safecat(buffer, bufsize, pos, sep);
   }
   pos = safecat(buffer, bufsize, pos, msg);
   return pos;
}

/* Log an error or warning - the relevant count is always incremented. */
static void
store_log(png_store* ps, png_structp pp, png_const_charp message, int is_error)
{
   /* The warning is copied to the error buffer if there are no errors and it is
    * the first warning.  The error is copied to the error buffer if it is the
    * first error (overwriting any prior warnings).
    */
   if (is_error ? (ps->nerrors)++ == 0 :
       (ps->nwarnings)++ == 0 && ps->nerrors == 0)
      store_message(ps, pp, ps->error, sizeof ps->error, 0, message);

   if (ps->verbose)
   {
      char buffer[256];
      size_t pos;

      if (is_error)
         pos = safecat(buffer, sizeof buffer, 0, "error: ");
      else
         pos = safecat(buffer, sizeof buffer, 0, "warning: ");

      store_message(ps, pp, buffer, sizeof buffer, pos, message);
      fputs(buffer, stderr);
      fputc('\n', stderr);
   }
}

/* Functions to use as PNG callbacks. */
static void
store_error(png_structp pp, png_const_charp message) /* PNG_NORETURN */
{
   png_store *ps = png_get_error_ptr(pp);

   if (!ps->expect_error)
      store_log(ps, pp, message, 1 /* error */);

   /* And finally throw an exception. */
   {
      struct exception_context *the_exception_context = &ps->exception_context;
      Throw ps;
   }
}

static void
store_warning(png_structp pp, png_const_charp message)
{
   png_store *ps = png_get_error_ptr(pp);

   if (!ps->expect_warning)
      store_log(ps, pp, message, 0 /* warning */);
   else
      ps->saw_warning = 1;
}

static void
store_write(png_structp pp, png_bytep pb, png_size_t st)
{
   png_store *ps = png_get_io_ptr(pp);

   if (ps->pwrite != pp)
      png_error(pp, "store state damaged");

   while (st > 0)
   {
      size_t cb;

      if (ps->writepos >= STORE_BUFFER_SIZE)
         store_storenew(ps);

      cb = st;

      if (cb > STORE_BUFFER_SIZE - ps->writepos)
         cb = STORE_BUFFER_SIZE - ps->writepos;

      memcpy(ps->new.buffer + ps->writepos, pb, cb);
      pb += cb;
      st -= cb;
      ps->writepos += cb;
   }
}

static void
store_flush(png_structp pp)
{
   UNUSED(pp) /*DOES NOTHING*/
}

static size_t
store_read_buffer_size(png_store *ps)
{
   /* Return the bytes available for read in the current buffer. */
   if (ps->next != &ps->current->data)
      return STORE_BUFFER_SIZE;

   return ps->current->datacount;
}

/* Return total bytes available for read. */
static size_t
store_read_buffer_avail(png_store *ps)
{
   if (ps->current != NULL && ps->next != NULL)
   {
      png_store_buffer *next = &ps->current->data;
      size_t cbAvail = ps->current->datacount;

      while (next != ps->next && next != NULL)
      {
         next = next->prev;
         cbAvail += STORE_BUFFER_SIZE;
      }

      if (next != ps->next)
         png_error(ps->pread, "buffer read error");

      if (cbAvail > ps->readpos)
         return cbAvail - ps->readpos;
   }

   return 0;
}

static int
store_read_buffer_next(png_store *ps)
{
   png_store_buffer *pbOld = ps->next;
   png_store_buffer *pbNew = &ps->current->data;
   if (pbOld != pbNew)
   {
      while (pbNew != NULL && pbNew->prev != pbOld)
         pbNew = pbNew->prev;

      if (pbNew != NULL)
      {
         ps->next = pbNew;
         ps->readpos = 0;
         return 1;
      }

      png_error(ps->pread, "buffer lost");
   }

   return 0; /* EOF or error */
}

/* Need separate implementation and callback to allow use of the same code
 * during progressive read, where the io_ptr is set internally by libpng.
 */
static void
store_read_imp(png_store *ps, png_bytep pb, png_size_t st)
{
   if (ps->current == NULL || ps->next == NULL)
      png_error(ps->pread, "store state damaged");

   while (st > 0)
   {
      size_t cbAvail = store_read_buffer_size(ps) - ps->readpos;

      if (cbAvail > 0)
      {
         if (cbAvail > st) cbAvail = st;
         memcpy(pb, ps->next->buffer + ps->readpos, cbAvail);
         st -= cbAvail;
         pb += cbAvail;
         ps->readpos += cbAvail;
      }

      else if (!store_read_buffer_next(ps))
         png_error(ps->pread, "read beyond end of file");
   }
}

static void
store_read(png_structp pp, png_bytep pb, png_size_t st)
{
   png_store *ps = png_get_io_ptr(pp);

   if (ps == NULL || ps->pread != pp)
      png_error(pp, "bad store read call");

   store_read_imp(ps, pb, st);
}

static void
store_progressive_read(png_store *ps, png_structp pp, png_infop pi)
{
   /* Notice that a call to store_read will cause this function to fail because
    * readpos will be set.
    */
   if (ps->pread != pp || ps->current == NULL || ps->next == NULL)
      png_error(pp, "store state damaged (progressive)");

   do
   {
      if (ps->readpos != 0)
         png_error(pp, "store_read called during progressive read");

      png_process_data(pp, pi, ps->next->buffer, store_read_buffer_size(ps));
   }
   while (store_read_buffer_next(ps));
}

/***************************** MEMORY MANAGEMENT*** ***************************/
/* A store_memory is simply the header for an allocated block of memory.  The
 * pointer returned to libpng is just after the end of the header block, the
 * allocated memory is followed by a second copy of the 'mark'.
 */
typedef struct store_memory
{
   store_pool          *pool;    /* Originating pool */
   struct store_memory *next;    /* Singly linked list */
   png_alloc_size_t     size;    /* Size of memory allocated */
   png_byte             mark[4]; /* ID marker */
} store_memory;

/* Handle a fatal error in memory allocation.  This calls png_error if the
 * libpng struct is non-NULL, else it outputs a message and returns.  This means
 * that a memory problem while libpng is running will abort (png_error) the
 * handling of particular file while one in cleanup (after the destroy of the
 * struct has returned) will simply keep going and free (or attempt to free)
 * all the memory.
 */
static void
store_pool_error(png_store *ps, png_structp pp, PNG_CONST char *msg)
{
   if (pp != NULL)
      png_error(pp, msg);

   /* Else we have to do it ourselves.  png_error eventually calls store_log,
    * above.  store_log accepts a NULL png_structp - it just changes what gets
    * output by store_message.
    */
   store_log(ps, pp, msg, 1 /* error */);
}

static void
store_memory_free(png_structp pp, store_pool *pool, store_memory *memory)
{
   /* Note that pp may be NULL (see store_pool_delete below), the caller has
    * found 'memory' in pool->list *and* unlinked this entry, so this is a valid
    * pointer (for sure), but the contents may have been trashed.
    */
   if (memory->pool != pool)
      store_pool_error(pool->store, pp, "memory corrupted (pool)");

   else if (memcmp(memory->mark, pool->mark, sizeof memory->mark) != 0)
      store_pool_error(pool->store, pp, "memory corrupted (start)");

   /* It should be safe to read the size field now. */
   else
   {
      png_alloc_size_t cb = memory->size;

      if (cb > pool->max)
         store_pool_error(pool->store, pp, "memory corrupted (size)");

      else if (memcmp((png_bytep)(memory+1)+cb, pool->mark, sizeof pool->mark)
         != 0)
         store_pool_error(pool->store, pp, "memory corrupted (end)");

      /* Finally give the library a chance to find problems too: */
      else
         {
         pool->current -= cb;
         free(memory);
         }
   }
}

static void
store_pool_delete(png_store *ps, store_pool *pool)
{
   if (pool->list != NULL)
   {
      fprintf(stderr, "%s: %s %s: memory lost (list follows):\n", ps->test,
         pool == &ps->read_memory_pool ? "read" : "write",
         pool == &ps->read_memory_pool ? (ps->current != NULL ?
            ps->current->name : "unknown file") : ps->wname);
      ++ps->nerrors;

      do
      {
         store_memory *next = pool->list;
         pool->list = next->next;
         next->next = NULL;

         fprintf(stderr, "\t%lu bytes @ %p\n",
             (unsigned long)next->size, (PNG_CONST void*)(next+1));
         /* The NULL means this will always return, even if the memory is
          * corrupted.
          */
         store_memory_free(NULL, pool, next);
      }
      while (pool->list != NULL);
   }

   /* And reset the other fields too for the next time. */
   if (pool->max > pool->max_max) pool->max_max = pool->max;
   pool->max = 0;
   if (pool->current != 0) /* unexpected internal error */
      fprintf(stderr, "%s: %s %s: memory counter mismatch (internal error)\n",
         ps->test, pool == &ps->read_memory_pool ? "read" : "write",
         pool == &ps->read_memory_pool ? (ps->current != NULL ?
            ps->current->name : "unknown file") : ps->wname);
   pool->current = 0;

   if (pool->limit > pool->max_limit)
      pool->max_limit = pool->limit;

   pool->limit = 0;

   if (pool->total > pool->max_total)
      pool->max_total = pool->total;

   pool->total = 0;

   /* Get a new mark too. */
   store_pool_mark(pool->mark);
}

/* The memory callbacks: */
static png_voidp
store_malloc(png_structp pp, png_alloc_size_t cb)
{
   store_pool *pool = png_get_mem_ptr(pp);
   store_memory *new = malloc(cb + (sizeof *new) + (sizeof pool->mark));

   if (new != NULL)
   {
      if (cb > pool->max)
         pool->max = cb;

      pool->current += cb;

      if (pool->current > pool->limit)
         pool->limit = pool->current;

      pool->total += cb;

      new->size = cb;
      memcpy(new->mark, pool->mark, sizeof new->mark);
      memcpy((png_byte*)(new+1) + cb, pool->mark, sizeof pool->mark);
      new->pool = pool;
      new->next = pool->list;
      pool->list = new;
      ++new;
   }

   else
      store_pool_error(pool->store, pp, "out of memory");

   return new;
}

static void
store_free(png_structp pp, png_voidp memory)
{
   store_pool *pool = png_get_mem_ptr(pp);
   store_memory *this = memory, **test;

   /* First check that this 'memory' really is valid memory - it must be in the
    * pool list.  If it is, use the shared memory_free function to free it.
    */
   --this;
   for (test = &pool->list; *test != this; test = &(*test)->next)
   {
      if (*test == NULL)
      {
         store_pool_error(pool->store, pp, "bad pointer to free");
         return;
      }
   }

   /* Unlink this entry, *test == this. */
   *test = this->next;
   this->next = NULL;
   store_memory_free(pp, pool, this);
}

/* Setup functions. */
/* Cleanup when aborting a write or after storing the new file. */
static void
store_write_reset(png_store *ps)
{
   if (ps->pwrite != NULL)
   {
      anon_context(ps);

      Try
         png_destroy_write_struct(&ps->pwrite, &ps->piwrite);

      Catch_anonymous
      {
         /* memory corruption: continue. */
      }

      ps->pwrite = NULL;
      ps->piwrite = NULL;
   }

   /* And make sure that all the memory has been freed - this will output
    * spurious errors in the case of memory corruption above, but this is safe.
    */
   store_pool_delete(ps, &ps->write_memory_pool);

   store_freenew(ps);
}

/* The following is the main write function, it returns a png_struct and,
 * optionally, a png_info suitable for writiing a new PNG file.  Use
 * store_storefile above to record this file after it has been written.  The
 * returned libpng structures as destroyed by store_write_reset above.
 */
static png_structp
set_store_for_write(png_store *ps, png_infopp ppi,
   PNG_CONST char * volatile name)
{
   anon_context(ps);

   Try
   {
      if (ps->pwrite != NULL)
         png_error(ps->pwrite, "write store already in use");

      store_write_reset(ps);
      safecat(ps->wname, sizeof ps->wname, 0, name);

      /* Don't do the slow memory checks if doing a speed test. */
      if (ps->speed)
         ps->pwrite = png_create_write_struct(PNG_LIBPNG_VER_STRING,
            ps, store_error, store_warning);

      else
         ps->pwrite = png_create_write_struct_2(PNG_LIBPNG_VER_STRING,
            ps, store_error, store_warning, &ps->write_memory_pool,
            store_malloc, store_free);

      png_set_write_fn(ps->pwrite, ps, store_write, store_flush);

      if (ppi != NULL)
         *ppi = ps->piwrite = png_create_info_struct(ps->pwrite);
   }

   Catch_anonymous
      return NULL;

   return ps->pwrite;
}

/* Cleanup when finished reading (either due to error or in the success case).
 */
static void
store_read_reset(png_store *ps)
{
   if (ps->pread != NULL)
   {
      anon_context(ps);

      Try
         png_destroy_read_struct(&ps->pread, &ps->piread, NULL);

      Catch_anonymous
      {
         /* error already output: continue */
      }

      ps->pread = NULL;
      ps->piread = NULL;
   }

   /* Always do this to be safe. */
   store_pool_delete(ps, &ps->read_memory_pool);

   ps->current = NULL;
   ps->next = NULL;
   ps->readpos = 0;
   ps->validated = 0;
}

static void
store_read_set(png_store *ps, png_uint_32 id)
{
   png_store_file *pf = ps->saved;

   while (pf != NULL)
   {
      if (pf->id == id)
      {
         ps->current = pf;
         ps->next = NULL;
         store_read_buffer_next(ps);
         return;
      }

      pf = pf->next;
   }

      {
      size_t pos;
      char msg[FILE_NAME_SIZE+64];

      pos = standard_name_from_id(msg, sizeof msg, 0, id);
      pos = safecat(msg, sizeof msg, pos, ": file not found");
      png_error(ps->pread, msg);
      }
}

/* The main interface for reading a saved file - pass the id number of the file
 * to retrieve.  Ids must be unique or the earlier file will be hidden.  The API
 * returns a png_struct and, optionally, a png_info.  Both of these will be
 * destroyed by store_read_reset above.
 */
static png_structp
set_store_for_read(png_store *ps, png_infopp ppi, png_uint_32 id,
   PNG_CONST char *name)
{
   /* Set the name for png_error */
   safecat(ps->test, sizeof ps->test, 0, name);

   if (ps->pread != NULL)
      png_error(ps->pread, "read store already in use");

   store_read_reset(ps);

   /* Both the create APIs can return NULL if used in their default mode
    * (because there is no other way of handling an error because the jmp_buf
    * by default is stored in png_struct and that has not been allocated!)
    * However, given that store_error works correctly in these circumstances
    * we don't ever expect NULL in this program.
    */
   if (ps->speed)
      ps->pread = png_create_read_struct(PNG_LIBPNG_VER_STRING, ps,
          store_error, store_warning);

   else
      ps->pread = png_create_read_struct_2(PNG_LIBPNG_VER_STRING, ps,
          store_error, store_warning, &ps->read_memory_pool, store_malloc,
          store_free);

   if (ps->pread == NULL)
   {
      struct exception_context *the_exception_context = &ps->exception_context;

      store_log(ps, NULL, "png_create_read_struct returned NULL (unexpected)",
         1 /*error*/);

      Throw ps;
   }

   store_read_set(ps, id);

   if (ppi != NULL)
      *ppi = ps->piread = png_create_info_struct(ps->pread);

   return ps->pread;
}

/* The overall cleanup of a store simply calls the above then removes all the
 * saved files.  This does not delete the store itself.
 */
static void
store_delete(png_store *ps)
{
   store_write_reset(ps);
   store_read_reset(ps);
   store_freefile(&ps->saved);

   if (ps->image != NULL)
   {
      free(ps->image-1);
      ps->image = NULL;
      ps->cb_image = 0;
   }
}

/*********************** PNG FILE MODIFICATION ON READ ************************/
/* Files may be modified on read.  The following structure contains a complete
 * png_store together with extra members to handle modification and a special
 * read callback for libpng.  To use this the 'modifications' field must be set
 * to a list of png_modification structures that actually perform the
 * modification, otherwise a png_modifier is functionally equivalent to a
 * png_store.  There is a special read function, set_modifier_for_read, which
 * replaces set_store_for_read.
 */
typedef struct png_modifier
{
   png_store               this;             /* I am a png_store */
   struct png_modification *modifications;   /* Changes to make */

   enum modifier_state
   {
      modifier_start,                        /* Initial value */
      modifier_signature,                    /* Have a signature */
      modifier_IHDR                          /* Have an IHDR */
   }                        state;           /* My state */

   /* Information from IHDR: */
   png_byte                 bit_depth;       /* From IHDR */
   png_byte                 colour_type;     /* From IHDR */

   /* While handling PLTE, IDAT and IEND these chunks may be pended to allow
    * other chunks to be inserted.
    */
   png_uint_32              pending_len;
   png_uint_32              pending_chunk;

   /* Test values */
   double                  *gammas;
   unsigned int             ngammas;

   /* Lowest sbit to test (libpng fails for sbit < 8) */
   png_byte                 sbitlow;

   /* Error control - these are the limits on errors accepted by the gamma tests
    * below.
    */
   double                   maxout8;  /* Maximum output value error */
   double                   maxabs8;  /* Absolute sample error 0..1 */
   double                   maxpc8;   /* Percentage sample error 0..100% */
   double                   maxout16; /* Maximum output value error */
   double                   maxabs16; /* Absolute sample error 0..1 */
   double                   maxpc16;  /* Percentage sample error 0..100% */

   /* Logged 8 and 16 bit errors ('output' values): */
   double                   error_gray_2;
   double                   error_gray_4;
   double                   error_gray_8;
   double                   error_gray_16;
   double                   error_color_8;
   double                   error_color_16;

   /* Flags: */
   /* Whether or not to interlace. */
   int                      interlace_type :9; /* int, but must store '1' */

   /* Run the standard tests? */
   unsigned int             test_standard :1;

   /* Run the odd-sized image and interlace read/write tests? */
   unsigned int             test_size :1;

   /* Run tests on reading with a combiniation of transforms, */
   unsigned int             test_transform :1;

   /* When to use the use_input_precision option: */
   unsigned int             use_input_precision :1;
   unsigned int             use_input_precision_sbit :1;
   unsigned int             use_input_precision_16to8 :1;

   /* Which gamma tests to run: */
   unsigned int             test_gamma_threshold :1;
   unsigned int             test_gamma_transform :1; /* main tests */
   unsigned int             test_gamma_sbit :1;
   unsigned int             test_gamma_strip16 :1;

   unsigned int             log :1;   /* Log max error */

   /* Buffer information, the buffer size limits the size of the chunks that can
    * be modified - they must fit (including header and CRC) into the buffer!
    */
   size_t                   flush;           /* Count of bytes to flush */
   size_t                   buffer_count;    /* Bytes in buffer */
   size_t                   buffer_position; /* Position in buffer */
   png_byte                 buffer[1024];
} png_modifier;

static double abserr(png_modifier *pm, png_byte bit_depth)
{
   return bit_depth == 16 ? pm->maxabs16 : pm->maxabs8;
}

static double pcerr(png_modifier *pm, png_byte bit_depth)
{
   return (bit_depth == 16 ? pm->maxpc16 : pm->maxpc8) * .01;
}

static double outerr(png_modifier *pm, png_byte bit_depth)
{
   /* There is a serious error in the 2 and 4 bit grayscale transform because
    * the gamma table value (8 bits) is simply shifted, not rounded, so the
    * error in 4 bit greyscale gamma is up to the value below.  This is a hack
    * to allow pngvalid to succeed:
    */
   if (bit_depth == 2)
      return .73182-.5;

   if (bit_depth == 4)
      return .90644-.5;

   if (bit_depth == 16)
     return pm->maxout16;

   return pm->maxout8;
}

/* This returns true if the test should be stopped now because it has already
 * failed and it is running silently.
 */
static int fail(png_modifier *pm)
{
   return !pm->log && !pm->this.verbose && (pm->this.nerrors > 0 ||
       (pm->this.treat_warnings_as_errors && pm->this.nwarnings > 0));
}

static void
modifier_init(png_modifier *pm)
{
   memset(pm, 0, sizeof *pm);
   store_init(&pm->this);
   pm->modifications = NULL;
   pm->state = modifier_start;
   pm->sbitlow = 1U;
   pm->maxout8 = pm->maxpc8 = pm->maxabs8 = 0;
   pm->maxout16 = pm->maxpc16 = pm->maxabs16 = 0;
   pm->error_gray_2 = pm->error_gray_4 = pm->error_gray_8 = 0;
   pm->error_gray_16 = pm->error_color_8 = pm->error_color_16 = 0;
   pm->interlace_type = PNG_INTERLACE_NONE;
   pm->test_standard = 0;
   pm->test_size = 0;
   pm->test_transform = 0;
   pm->use_input_precision = 0;
   pm->use_input_precision_sbit = 0;
   pm->use_input_precision_16to8 = 0;
   pm->test_gamma_threshold = 0;
   pm->test_gamma_transform = 0;
   pm->test_gamma_sbit = 0;
   pm->test_gamma_strip16 = 0;
   pm->log = 0;

   /* Rely on the memset for all the other fields - there are no pointers */
}

/* One modification structure must be provided for each chunk to be modified (in
 * fact more than one can be provided if multiple separate changes are desired
 * for a single chunk.)  Modifications include adding a new chunk when a
 * suitable chunk does not exist.
 *
 * The caller of modify_fn will reset the CRC of the chunk and record 'modified'
 * or 'added' as appropriate if the modify_fn returns 1 (true).  If the
 * modify_fn is NULL the chunk is simply removed.
 */
typedef struct png_modification
{
   struct png_modification *next;
   png_uint_32              chunk;

   /* If the following is NULL all matching chunks will be removed: */
   int                    (*modify_fn)(struct png_modifier *pm,
                               struct png_modification *me, int add);

   /* If the following is set to PLTE, IDAT or IEND and the chunk has not been
    * found and modified (and there is a modify_fn) the modify_fn will be called
    * to add the chunk before the relevant chunk.
    */
   png_uint_32              add;
   unsigned int             modified :1;     /* Chunk was modified */
   unsigned int             added    :1;     /* Chunk was added */
   unsigned int             removed  :1;     /* Chunk was removed */
} png_modification;

static void modification_reset(png_modification *pmm)
{
   if (pmm != NULL)
   {
      pmm->modified = 0;
      pmm->added = 0;
      pmm->removed = 0;
      modification_reset(pmm->next);
   }
}

static void
modification_init(png_modification *pmm)
{
   memset(pmm, 0, sizeof *pmm);
   pmm->next = NULL;
   pmm->chunk = 0;
   pmm->modify_fn = NULL;
   pmm->add = 0;
   modification_reset(pmm);
}

static void
modifier_reset(png_modifier *pm)
{
   store_read_reset(&pm->this);
   pm->modifications = NULL;
   pm->state = modifier_start;
   pm->bit_depth = pm->colour_type = 0;
   pm->pending_len = pm->pending_chunk = 0;
   pm->flush = pm->buffer_count = pm->buffer_position = 0;
}

/* Convenience macros. */
#define CHUNK(a,b,c,d) (((a)<<24)+((b)<<16)+((c)<<8)+(d))
#define CHUNK_IHDR CHUNK(73,72,68,82)
#define CHUNK_PLTE CHUNK(80,76,84,69)
#define CHUNK_IDAT CHUNK(73,68,65,84)
#define CHUNK_IEND CHUNK(73,69,78,68)
#define CHUNK_cHRM CHUNK(99,72,82,77)
#define CHUNK_gAMA CHUNK(103,65,77,65)
#define CHUNK_sBIT CHUNK(115,66,73,84)
#define CHUNK_sRGB CHUNK(115,82,71,66)

/* The guts of modification are performed during a read. */
static void
modifier_crc(png_bytep buffer)
{
   /* Recalculate the chunk CRC - a complete chunk must be in
    * the buffer, at the start.
    */
   uInt datalen = png_get_uint_32(buffer);
   png_save_uint_32(buffer+datalen+8, crc32(0L, buffer+4, datalen+4));
}

static void
modifier_setbuffer(png_modifier *pm)
{
   modifier_crc(pm->buffer);
   pm->buffer_count = png_get_uint_32(pm->buffer)+12;
   pm->buffer_position = 0;
}

/* Separate the callback into the actual implementation (which is passed the
 * png_modifier explicitly) and the callback, which gets the modifier from the
 * png_struct.
 */
static void
modifier_read_imp(png_modifier *pm, png_bytep pb, png_size_t st)
{
   while (st > 0)
   {
      size_t cb;
      png_uint_32 len, chunk;
      png_modification *mod;

      if (pm->buffer_position >= pm->buffer_count) switch (pm->state)
      {
         static png_byte sign[8] = { 137, 80, 78, 71, 13, 10, 26, 10 };
         case modifier_start:
            store_read_imp(&pm->this, pm->buffer, 8); /* size of signature. */
            pm->buffer_count = 8;
            pm->buffer_position = 0;

            if (memcmp(pm->buffer, sign, 8) != 0)
               png_error(pm->this.pread, "invalid PNG file signature");
            pm->state = modifier_signature;
            break;

         case modifier_signature:
            store_read_imp(&pm->this, pm->buffer, 13+12); /* size of IHDR */
            pm->buffer_count = 13+12;
            pm->buffer_position = 0;

            if (png_get_uint_32(pm->buffer) != 13 ||
                png_get_uint_32(pm->buffer+4) != CHUNK_IHDR)
               png_error(pm->this.pread, "invalid IHDR");

            /* Check the list of modifiers for modifications to the IHDR. */
            mod = pm->modifications;
            while (mod != NULL)
            {
               if (mod->chunk == CHUNK_IHDR && mod->modify_fn &&
                   (*mod->modify_fn)(pm, mod, 0))
                  {
                  mod->modified = 1;
                  modifier_setbuffer(pm);
                  }

               /* Ignore removal or add if IHDR! */
               mod = mod->next;
            }

            /* Cache information from the IHDR (the modified one.) */
            pm->bit_depth = pm->buffer[8+8];
            pm->colour_type = pm->buffer[8+8+1];

            pm->state = modifier_IHDR;
            pm->flush = 0;
            break;

         case modifier_IHDR:
         default:
            /* Read a new chunk and process it until we see PLTE, IDAT or
             * IEND.  'flush' indicates that there is still some data to
             * output from the preceding chunk.
             */
            if ((cb = pm->flush) > 0)
            {
               if (cb > st) cb = st;
               pm->flush -= cb;
               store_read_imp(&pm->this, pb, cb);
               pb += cb;
               st -= cb;
               if (st <= 0) return;
            }

            /* No more bytes to flush, read a header, or handle a pending
             * chunk.
             */
            if (pm->pending_chunk != 0)
            {
               png_save_uint_32(pm->buffer, pm->pending_len);
               png_save_uint_32(pm->buffer+4, pm->pending_chunk);
               pm->pending_len = 0;
               pm->pending_chunk = 0;
            }
            else
               store_read_imp(&pm->this, pm->buffer, 8);

            pm->buffer_count = 8;
            pm->buffer_position = 0;

            /* Check for something to modify or a terminator chunk. */
            len = png_get_uint_32(pm->buffer);
            chunk = png_get_uint_32(pm->buffer+4);

            /* Terminators first, they may have to be delayed for added
             * chunks
             */
            if (chunk == CHUNK_PLTE || chunk == CHUNK_IDAT ||
                chunk == CHUNK_IEND)
            {
               mod = pm->modifications;

               while (mod != NULL)
               {
                  if ((mod->add == chunk ||
                      (mod->add == CHUNK_PLTE && chunk == CHUNK_IDAT)) &&
                      mod->modify_fn != NULL && !mod->modified && !mod->added)
                  {
                     /* Regardless of what the modify function does do not run
                      * this again.
                      */
                     mod->added = 1;

                     if ((*mod->modify_fn)(pm, mod, 1 /*add*/))
                     {
                        /* Reset the CRC on a new chunk */
                        if (pm->buffer_count > 0)
                           modifier_setbuffer(pm);

                        else
                           {
                           pm->buffer_position = 0;
                           mod->removed = 1;
                           }

                        /* The buffer has been filled with something (we assume)
                         * so output this.  Pend the current chunk.
                         */
                        pm->pending_len = len;
                        pm->pending_chunk = chunk;
                        break; /* out of while */
                     }
                  }

                  mod = mod->next;
               }

               /* Don't do any further processing if the buffer was modified -
                * otherwise the code will end up modifying a chunk that was
                * just added.
                */
               if (mod != NULL)
                  break; /* out of switch */
            }

            /* If we get to here then this chunk may need to be modified.  To
             * do this it must be less than 1024 bytes in total size, otherwise
             * it just gets flushed.
             */
            if (len+12 <= sizeof pm->buffer)
            {
               store_read_imp(&pm->this, pm->buffer+pm->buffer_count,
                   len+12-pm->buffer_count);
               pm->buffer_count = len+12;

               /* Check for a modification, else leave it be. */
               mod = pm->modifications;
               while (mod != NULL)
               {
                  if (mod->chunk == chunk)
                  {
                     if (mod->modify_fn == NULL)
                     {
                        /* Remove this chunk */
                        pm->buffer_count = pm->buffer_position = 0;
                        mod->removed = 1;
                        break; /* Terminate the while loop */
                     }

                     else if ((*mod->modify_fn)(pm, mod, 0))
                     {
                        mod->modified = 1;
                        /* The chunk may have been removed: */
                        if (pm->buffer_count == 0)
                        {
                           pm->buffer_position = 0;
                           break;
                        }
                        modifier_setbuffer(pm);
                     }
                  }

                  mod = mod->next;
               }
            }

            else
               pm->flush = len+12 - pm->buffer_count; /* data + crc */

            /* Take the data from the buffer (if there is any). */
            break;
      }

      /* Here to read from the modifier buffer (not directly from
       * the store, as in the flush case above.)
       */
      cb = pm->buffer_count - pm->buffer_position;

      if (cb > st)
         cb = st;

      memcpy(pb, pm->buffer + pm->buffer_position, cb);
      st -= cb;
      pb += cb;
      pm->buffer_position += cb;
   }
}

/* The callback: */
static void
modifier_read(png_structp pp, png_bytep pb, png_size_t st)
{
   png_modifier *pm = png_get_io_ptr(pp);

   if (pm == NULL || pm->this.pread != pp)
      png_error(pp, "bad modifier_read call");

   modifier_read_imp(pm, pb, st);
}

/* Like store_progressive_read but the data is getting changed as we go so we
 * need a local buffer.
 */
static void
modifier_progressive_read(png_modifier *pm, png_structp pp, png_infop pi)
{
   if (pm->this.pread != pp || pm->this.current == NULL ||
       pm->this.next == NULL)
      png_error(pp, "store state damaged (progressive)");

   /* This is another Horowitz and Hill random noise generator.  In this case
    * the aim is to stress the progressive reader with truely horrible variable
    * buffer sizes in the range 1..500, so a sequence of 9 bit random numbers
    * is generated.  We could probably just count from 1 to 32767 and get as
    * good a result.
    */
   for (;;)
   {
      static png_uint_32 noise = 1;
      png_size_t cb, cbAvail;
      png_byte buffer[512];

      /* Generate 15 more bits of stuff: */
      noise = (noise << 9) | ((noise ^ (noise >> (9-5))) & 0x1ff);
      cb = noise & 0x1ff;

      /* Check that this number of bytes are available (in the current buffer.)
       * (This doesn't quite work - the modifier might delete a chunk; unlikely
       * but possible, it doesn't happen at present because the modifier only
       * adds chunks to standard images.)
       */
      cbAvail = store_read_buffer_avail(&pm->this);
      if (pm->buffer_count > pm->buffer_position)
         cbAvail += pm->buffer_count - pm->buffer_position;

      if (cb > cbAvail)
      {
         /* Check for EOF: */
         if (cbAvail == 0)
            break;

         cb = cbAvail;
      }

      modifier_read_imp(pm, buffer, cb);
      png_process_data(pp, pi, buffer, cb);
   }

   /* Check the invariants at the end (if this fails it's a problem in this
    * file!)
    */
   if (pm->buffer_count > pm->buffer_position ||
       pm->this.next != &pm->this.current->data ||
       pm->this.readpos < pm->this.current->datacount)
      png_error(pp, "progressive read implementation error");
}

/* Set up a modifier. */
static png_structp
set_modifier_for_read(png_modifier *pm, png_infopp ppi, png_uint_32 id,
    PNG_CONST char *name)
{
   /* Do this first so that the modifier fields are cleared even if an error
    * happens allocating the png_struct.  No allocation is done here so no
    * cleanup is required.
    */
   pm->state = modifier_start;
   pm->bit_depth = 0;
   pm->colour_type = 255;

   pm->pending_len = 0;
   pm->pending_chunk = 0;
   pm->flush = 0;
   pm->buffer_count = 0;
   pm->buffer_position = 0;

   return set_store_for_read(&pm->this, ppi, id, name);
}

/***************************** STANDARD PNG FILES *****************************/
/* Standard files - write and save standard files. */
/* There are two basic forms of standard images.  Those which attempt to have
 * all the possible pixel values (not possible for 16bpp images, but a range of
 * values are produced) and those which have a range of image sizes.  The former
 * are used for testing transforms, in particular gamma correction and bit
 * reduction and increase.  The latter are reserved for testing the behavior of
 * libpng with respect to 'odd' image sizes - particularly small images where
 * rows become 1 byte and interlace passes disappear.
 *
 * The first, most useful, set are the 'transform' images, the second set of
 * small images are the 'size' images.
 *
 * The transform files are constructed with rows which fit into a 1024 byte row
 * buffer.  This makes allocation easier below.  Further regardless of the file
 * format every row has 128 pixels (giving 1024 bytes for 64bpp formats).
 *
 * Files are stored with no gAMA or sBIT chunks, with a PLTE only when needed
 * and with an ID derived from the colour type, bit depth and interlace type
 * as above (FILEID).  The width (128) and height (variable) are not stored in
 * the FILEID - instead the fields are set to 0, indicating a transform file.
 *
 * The size files ar constructed with rows a maximum of 128 bytes wide, allowing
 * a maximum width of 16 pixels (for the 64bpp case.)  They also have a maximum
 * height of 16 rows.  The width and height are stored in the FILEID and, being
 * non-zero, indicate a size file.
 */

/* The number of passes is related to the interlace type. There wass no libpng
 * API to determine this prior to 1.5, so we need an inquiry function:
 */
static int
npasses_from_interlace_type(png_structp pp, int interlace_type)
{
   switch (interlace_type)
   {
   default:
      png_error(pp, "invalid interlace type");

   case PNG_INTERLACE_NONE:
      return 1;

   case PNG_INTERLACE_ADAM7:
      return PNG_INTERLACE_ADAM7_PASSES;
   }
}

static unsigned int
bit_size(png_structp pp, png_byte colour_type, png_byte bit_depth)
{
   switch (colour_type)
   {
      case 0:  return bit_depth;

      case 2:  return 3*bit_depth;

      case 3:  return bit_depth;

      case 4:  return 2*bit_depth;

      case 6:  return 4*bit_depth;

      default: png_error(pp, "invalid color type");
   }
}

#define TRANSFORM_WIDTH  128U
#define TRANSFORM_ROWMAX (TRANSFORM_WIDTH*8U)
#define SIZE_ROWMAX (16*8U) /* 16 pixels, max 8 bytes each - 128 bytes */
#define STANDARD_ROWMAX TRANSFORM_ROWMAX /* The larger of the two */

/* So the maximum image sizes are as follows.  A 'transform' image may require
 * more than 65535 bytes.  The size images are a maximum of 2046 bytes.
 */
#define TRANSFORM_IMAGEMAX (TRANSFORM_ROWMAX * (png_uint_32)2048)
#define SIZE_IMAGEMAX (SIZE_ROWMAX * 16U)

static size_t
transform_rowsize(png_structp pp, png_byte colour_type, png_byte bit_depth)
{
   return (TRANSFORM_WIDTH * bit_size(pp, colour_type, bit_depth)) / 8;
}

/* transform_width(pp, colour_type, bit_depth) current returns the same number
 * every time, so just use a macro:
 */
#define transform_width(pp, colour_type, bit_depth) TRANSFORM_WIDTH

static png_uint_32
transform_height(png_structp pp, png_byte colour_type, png_byte bit_depth)
{
   switch (bit_size(pp, colour_type, bit_depth))
   {
      case 1:
      case 2:
      case 4:
         return 1;   /* Total of 128 pixels */

      case 8:
         return 2;   /* Total of 256 pixels/bytes */

      case 16:
         return 512; /* Total of 65536 pixels */

      case 24:
      case 32:
         return 512; /* 65536 pixels */

      case 48:
      case 64:
         return 2048;/* 4 x 65536 pixels. */

      default:
         return 0;   /* Error, will be caught later */
   }
}

/* The following can only be defined here, now we have the definitions
 * of the transform image sizes.
 */
static png_uint_32
standard_width(png_structp pp, png_uint_32 id)
{
   png_uint_32 width = WIDTH_FROM_ID(id);
   UNUSED(pp)

   if (width == 0)
      width = transform_width(pp, COL_FROM_ID(id), DEPTH_FROM_ID(id));

   return width;
}

static png_uint_32
standard_height(png_structp pp, png_uint_32 id)
{
   png_uint_32 height = HEIGHT_FROM_ID(id);

   if (height == 0)
      height = transform_height(pp, COL_FROM_ID(id), DEPTH_FROM_ID(id));

   return height;
}

static png_uint_32
standard_rowsize(png_structp pp, png_uint_32 id)
{
   png_uint_32 width = standard_width(pp, id);

   /* This won't overflow: */
   width *= bit_size(pp, COL_FROM_ID(id), DEPTH_FROM_ID(id));
   return (width + 7) / 8;
}

static void
transform_row(png_structp pp, png_byte buffer[TRANSFORM_ROWMAX],
   png_byte colour_type, png_byte bit_depth, png_uint_32 y)
{
   png_uint_32 v = y << 7;
   png_uint_32 i = 0;

   switch (bit_size(pp, colour_type, bit_depth))
   {
      case 1:
         while (i<128/8) buffer[i] = v & 0xff, v += 17, ++i;
         return;

      case 2:
         while (i<128/4) buffer[i] = v & 0xff, v += 33, ++i;
         return;

      case 4:
         while (i<128/2) buffer[i] = v & 0xff, v += 65, ++i;
         return;

      case 8:
         /* 256 bytes total, 128 bytes in each row set as follows: */
         while (i<128) buffer[i] = v & 0xff, ++v, ++i;
         return;

      case 16:
         /* Generate all 65536 pixel values in order, which includes the 8 bit
          * GA case as well as the 16 bit G case.
          */
         while (i<128)
            buffer[2*i] = (v>>8) & 0xff, buffer[2*i+1] = v & 0xff, ++v, ++i;

         return;

      case 24:
         /* 65535 pixels, but rotate the values. */
         while (i<128)
         {
            /* Three bytes per pixel, r, g, b, make b by r^g */
            buffer[3*i+0] = (v >> 8) & 0xff;
            buffer[3*i+1] = v & 0xff;
            buffer[3*i+2] = ((v >> 8) ^ v) & 0xff;
            ++v;
            ++i;
         }

         return;

      case 32:
         /* 65535 pixels, r, g, b, a; just replicate */
         while (i<128)
         {
            buffer[4*i+0] = (v >> 8) & 0xff;
            buffer[4*i+1] = v & 0xff;
            buffer[4*i+2] = (v >> 8) & 0xff;
            buffer[4*i+3] = v & 0xff;
            ++v;
            ++i;
         }

         return;

      case 48:
         /* y is maximum 2047, giving 4x65536 pixels, make 'r' increase by 1 at
          * each pixel, g increase by 257 (0x101) and 'b' by 0x1111:
          */
         while (i<128)
         {
            png_uint_32 t = v++;
            buffer[6*i+0] = (t >> 8) & 0xff;
            buffer[6*i+1] = t & 0xff;
            t *= 257;
            buffer[6*i+2] = (t >> 8) & 0xff;
            buffer[6*i+3] = t & 0xff;
            t *= 17;
            buffer[6*i+4] = (t >> 8) & 0xff;
            buffer[6*i+5] = t & 0xff;
            ++i;
         }

         return;

      case 64:
         /* As above in the 32 bit case. */
         while (i<128)
         {
            png_uint_32 t = v++;
            buffer[8*i+0] = (t >> 8) & 0xff;
            buffer[8*i+1] = t & 0xff;
            buffer[8*i+4] = (t >> 8) & 0xff;
            buffer[8*i+5] = t & 0xff;
            t *= 257;
            buffer[8*i+2] = (t >> 8) & 0xff;
            buffer[8*i+3] = t & 0xff;
            buffer[8*i+6] = (t >> 8) & 0xff;
            buffer[8*i+7] = t & 0xff;
            ++i;
         }
         return;

      default:
         break;
   }

   png_error(pp, "internal error");
}

/* This is just to do the right cast - could be changed to a function to check
 * 'bd' but there isn't much point.
 */
#define DEPTH(bd) ((png_byte)(1U << (bd)))

/* Make a standardized image given a an image colour type, bit depth and
 * interlace type.  The standard images have a very restricted range of
 * rows and heights and are used for testing transforms rather than image
 * layout details.  See make_size_images below for a way to make images
 * that test odd sizes along with the libpng interlace handling.
 */
static void
make_transform_image(png_store* PNG_CONST ps, png_byte PNG_CONST colour_type,
    png_byte PNG_CONST bit_depth, int interlace_type, png_const_charp name)
{
   context(ps, fault);

   Try
   {
      png_infop pi;
      png_structp pp = set_store_for_write(ps, &pi, name);
      png_uint_32 h;

      /* In the event of a problem return control to the Catch statement below
       * to do the clean up - it is not possible to 'return' directly from a Try
       * block.
       */
      if (pp == NULL)
         Throw ps;

      h = transform_height(pp, colour_type, bit_depth);

      png_set_IHDR(pp, pi, transform_width(pp, colour_type, bit_depth), h,
         bit_depth, colour_type, interlace_type,
         PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

      if (colour_type == 3) /* palette */
      {
         unsigned int i = 0;
         png_color pal[256];

         do
            pal[i].red = pal[i].green = pal[i].blue = (png_byte)i;
         while(++i < 256U);

         png_set_PLTE(pp, pi, pal, 256);
      }

      png_write_info(pp, pi);

      if (png_get_rowbytes(pp, pi) !=
          transform_rowsize(pp, colour_type, bit_depth))
         png_error(pp, "row size incorrect");

      else
      {
         /* Somewhat confusingly this must be called *after* png_write_info
          * because if it is called before, the information in *pp has not been
          * updated to reflect the interlaced image.
          */
         int npasses = png_set_interlace_handling(pp);
         int pass;

         if (npasses != npasses_from_interlace_type(pp, interlace_type))
            png_error(pp, "write: png_set_interlace_handling failed");

         for (pass=0; pass<npasses; ++pass)
         {
            png_uint_32 y;

            for (y=0; y<h; ++y)
            {
               png_byte buffer[TRANSFORM_ROWMAX];

               transform_row(pp, buffer, colour_type, bit_depth, y);
               png_write_row(pp, buffer);
            }
         }
      }

      png_write_end(pp, pi);

      /* And store this under the appropriate id, then clean up. */
      store_storefile(ps, FILEID(colour_type, bit_depth, interlace_type,
         0, 0, 0));

      store_write_reset(ps);
   }

   Catch(fault)
   {
      /* Use the png_store returned by the exception. This may help the compiler
       * because 'ps' is not used in this branch of the setjmp.  Note that fault
       * and ps will always be the same value.
       */
      store_write_reset(fault);
   }
}

static void
make_standard(png_store* PNG_CONST ps, png_byte PNG_CONST colour_type, int bdlo,
    int PNG_CONST bdhi)
{
   for (; bdlo <= bdhi; ++bdlo)
   {
      int interlace_type;

      for (interlace_type = PNG_INTERLACE_NONE;
           interlace_type < PNG_INTERLACE_LAST; ++interlace_type)
      {
         char name[FILE_NAME_SIZE];

         standard_name(name, sizeof name, 0, colour_type, bdlo, interlace_type,
            0, 0, 0);
         make_transform_image(ps, colour_type, DEPTH(bdlo), interlace_type,
            name);
      }
   }
}

static void
make_transform_images(png_store *ps)
{
   /* This is in case of errors. */
   safecat(ps->test, sizeof ps->test, 0, "make standard images");

   /* Arguments are colour_type, low bit depth, high bit depth
    */
   make_standard(ps, 0, 0, WRITE_BDHI);
   make_standard(ps, 2, 3, WRITE_BDHI);
   make_standard(ps, 3, 0, 3 /*palette: max 8 bits*/);
   make_standard(ps, 4, 3, WRITE_BDHI);
   make_standard(ps, 6, 3, WRITE_BDHI);
}

/* The following two routines use the PNG interlace support macros from
 * png.h to interlace or deinterlace rows.
 */
static void
interlace_row(png_bytep buffer, png_const_bytep imageRow,
   unsigned int pixel_size, png_uint_32 w, int pass)
{
   png_uint_32 xin, xout, xstep;

   /* Note that this can, trivially, be optimized to a memcpy on pass 7, the
    * code is presented this way to make it easier to understand.  In practice
    * consult the code in the libpng source to see other ways of doing this.
    */
   xin = PNG_PASS_START_COL(pass);
   xstep = 1U<<PNG_PASS_COL_SHIFT(pass);

   for (xout=0; xin<w; xin+=xstep)
   {
      pixel_copy(buffer, xout, imageRow, xin, pixel_size);
      ++xout;
   }
}

static void
deinterlace_row(png_bytep buffer, png_const_bytep row,
   unsigned int pixel_size, png_uint_32 w, int pass)
{
   /* The inverse of the above, 'row' is part of row 'y' of the output image,
    * in 'buffer'.  The image is 'w' wide and this is pass 'pass', distribute
    * the pixels of row into buffer and return the number written (to allow
    * this to be checked).
    */
   png_uint_32 xin, xout, xstep;

   xout = PNG_PASS_START_COL(pass);
   xstep = 1U<<PNG_PASS_COL_SHIFT(pass);

   for (xin=0; xout<w; xout+=xstep)
   {
      pixel_copy(buffer, xout, row, xin, pixel_size);
      ++xin;
   }
}

/* Build a single row for the 'size' test images, this fills in only the
 * first bit_width bits of the sample row.
 */
static void
size_row(png_byte buffer[SIZE_ROWMAX], png_uint_32 bit_width, png_uint_32 y)
{
   /* height is in the range 1 to 16, so: */
   y = ((y & 1) << 7) + ((y & 2) << 6) + ((y & 4) << 5) + ((y & 8) << 4);
   /* the following ensures bits are set in small images: */
   y ^= 0xA5;

   while (bit_width >= 8)
      *buffer++ = (png_byte)y++, bit_width -= 8;

   /* There may be up to 7 remaining bits, these go in the most significant
    * bits of the byte.
    */
   if (bit_width > 0)
   {
      png_uint_32 mask = (1U<<(8-bit_width))-1;
      *buffer = (png_byte)((*buffer & mask) | (y & ~mask));
   }
}

static void
make_size_image(png_store* PNG_CONST ps, png_byte PNG_CONST colour_type,
    png_byte PNG_CONST bit_depth, int PNG_CONST interlace_type,
    png_uint_32 PNG_CONST w, png_uint_32 PNG_CONST h,
    int PNG_CONST do_interlace)
{
   context(ps, fault);

   Try
   {
      png_infop pi;
      png_structp pp;
      unsigned int pixel_size;

      /* Make a name and get an appropriate id for the store: */
      char name[FILE_NAME_SIZE];
      PNG_CONST png_uint_32 id = FILEID(colour_type, bit_depth, interlace_type,
         w, h, do_interlace);

      standard_name_from_id(name, sizeof name, 0, id);
      pp = set_store_for_write(ps, &pi, name);

      /* In the event of a problem return control to the Catch statement below
       * to do the clean up - it is not possible to 'return' directly from a Try
       * block.
       */
      if (pp == NULL)
         Throw ps;

      png_set_IHDR(pp, pi, w, h, bit_depth, colour_type, interlace_type,
         PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

      /* Same palette as make_transform_image - I don' think there is any
       * benefit from using a different one (JB 20101211)
       */
      if (colour_type == 3) /* palette */
      {
         unsigned int i = 0;
         png_color pal[256];

         do
            pal[i].red = pal[i].green = pal[i].blue = (png_byte)i;
         while(++i < 256U);

         png_set_PLTE(pp, pi, pal, 256);
      }

      png_write_info(pp, pi);

      /* Calculate the bit size, divide by 8 to get the byte size - this won't
       * overflow because we know the w values are all small enough even for
       * a system where 'unsigned int' is only 16 bits.
       */
      pixel_size = bit_size(pp, colour_type, bit_depth);
      if (png_get_rowbytes(pp, pi) != ((w * pixel_size) + 7) / 8)
         png_error(pp, "row size incorrect");

      else
      {
         int npasses = npasses_from_interlace_type(pp, interlace_type);
         png_uint_32 y;
         int pass;
         png_byte image[16][SIZE_ROWMAX];

         /* To help consistent error detection make the parts of this buffer
          * that aren't set below all '1':
          */
         memset(image, 0xff, sizeof image);

         if (!do_interlace && npasses != png_set_interlace_handling(pp))
            png_error(pp, "write: png_set_interlace_handling failed");

         /* Prepare the whole image first to avoid making it 7 times: */
         for (y=0; y<h; ++y)
            size_row(image[y], w * pixel_size, y);

         for (pass=0; pass<npasses; ++pass)
         {
            /* The following two are for checking the macros: */
            PNG_CONST png_uint_32 wPass = PNG_PASS_COLS(w, pass);

            /* If do_interlace is set we don't call png_write_row for every
             * row because some of them are empty.  In fact, for a 1x1 image,
             * most of them are empty!
             */
            for (y=0; y<h; ++y)
            {
               png_const_bytep row = image[y];
               png_byte tempRow[SIZE_ROWMAX];

               /* If do_interlace *and* the image is interlaced we
                * need a reduced interlace row, this may be reduced
                * to empty.
                */
               if (do_interlace && interlace_type == PNG_INTERLACE_ADAM7)
               {
                  /* The row must not be written if it doesn't exist, notice
                   * that there are two conditions here, either the row isn't
                   * ever in the pass or the row would be but isn't wide
                   * enough to contribute any pixels.  In fact the wPass test
                   * can be used to skip the whole y loop in this case.
                   */
                  if (PNG_ROW_IN_INTERLACE_PASS(y, pass) && wPass > 0)
                  {
                     /* Set to all 1's for error detection (libpng tends to
                      * set unset things to 0).
                      */
                     memset(tempRow, 0xff, sizeof tempRow);
                     interlace_row(tempRow, row, pixel_size, w, pass);
                     row = tempRow;
                  }
                  else
                     continue;
               }

               /* Only get to here if the row has some pixels in it. */
               png_write_row(pp, row);
            }
         }
      }

      png_write_end(pp, pi);

      /* And store this under the appropriate id, then clean up. */
      store_storefile(ps, id);

      store_write_reset(ps);
   }

   Catch(fault)
   {
      /* Use the png_store returned by the exception. This may help the compiler
       * because 'ps' is not used in this branch of the setjmp.  Note that fault
       * and ps will always be the same value.
       */
      store_write_reset(fault);
   }
}

static void
make_size(png_store* PNG_CONST ps, png_byte PNG_CONST colour_type, int bdlo,
    int PNG_CONST bdhi)
{
   for (; bdlo <= bdhi; ++bdlo)
   {
      png_uint_32 width;

      for (width = 1; width <= 16; ++width)
      {
         png_uint_32 height;

         for (height = 1; height <= 16; ++height)
         {
            /* The four combinations of DIY interlace and interlace or not -
             * no interlace + DIY should be identical to no interlace with
             * libpng doing it.
             */
            make_size_image(ps, colour_type, DEPTH(bdlo), PNG_INTERLACE_NONE,
               width, height, 0);
            make_size_image(ps, colour_type, DEPTH(bdlo), PNG_INTERLACE_NONE,
               width, height, 1);
            make_size_image(ps, colour_type, DEPTH(bdlo), PNG_INTERLACE_ADAM7,
               width, height, 0);
            make_size_image(ps, colour_type, DEPTH(bdlo), PNG_INTERLACE_ADAM7,
               width, height, 1);
         }
      }
   }
}

static void
make_size_images(png_store *ps)
{
   /* This is in case of errors. */
   safecat(ps->test, sizeof ps->test, 0, "make size images");

   /* Arguments are colour_type, low bit depth, high bit depth
    */
   make_size(ps, 0, 0, WRITE_BDHI);
   make_size(ps, 2, 3, WRITE_BDHI);
   make_size(ps, 3, 0, 3 /*palette: max 8 bits*/);
   make_size(ps, 4, 3, WRITE_BDHI);
   make_size(ps, 6, 3, WRITE_BDHI);
}

/* Return a row based on image id and 'y' for checking: */
static void
standard_row(png_structp pp, png_byte std[STANDARD_ROWMAX], png_uint_32 id,
   png_uint_32 y)
{
   if (WIDTH_FROM_ID(id) == 0)
      transform_row(pp, std, COL_FROM_ID(id), DEPTH_FROM_ID(id), y);
   else
      size_row(std, WIDTH_FROM_ID(id) * bit_size(pp, COL_FROM_ID(id),
         DEPTH_FROM_ID(id)), y);
}

/* Tests - individual test cases */
/* Like 'make_standard' but errors are deliberately introduced into the calls
 * to ensure that they get detected - it should not be possible to write an
 * invalid image with libpng!
 */
static void
sBIT0_error_fn(png_structp pp, png_infop pi)
{
   /* 0 is invalid... */
   png_color_8 bad;
   bad.red = bad.green = bad.blue = bad.gray = bad.alpha = 0;
   png_set_sBIT(pp, pi, &bad);
}

static void
sBIT_error_fn(png_structp pp, png_infop pi)
{
   png_byte bit_depth;
   png_color_8 bad;

   if (png_get_color_type(pp, pi) == PNG_COLOR_TYPE_PALETTE)
      bit_depth = 8;

   else
      bit_depth = png_get_bit_depth(pp, pi);

   /* Now we know the bit depth we can easily generate an invalid sBIT entry */
   bad.red = bad.green = bad.blue = bad.gray = bad.alpha =
      (png_byte)(bit_depth+1);
   png_set_sBIT(pp, pi, &bad);
}

static PNG_CONST struct
{
   void          (*fn)(png_structp, png_infop);
   PNG_CONST char *msg;
   unsigned int    warning :1; /* the error is a warning... */
} error_test[] =
    {
       { sBIT0_error_fn, "sBIT(0): failed to detect error", 1 },
       { sBIT_error_fn, "sBIT(too big): failed to detect error", 1 },
    };

static void
make_error(png_store* volatile ps, png_byte PNG_CONST colour_type,
    png_byte bit_depth, int interlace_type, int test, png_const_charp name)
{
   context(ps, fault);

   Try
   {
      png_structp pp;
      png_infop pi;

      pp = set_store_for_write(ps, &pi, name);

      if (pp == NULL)
         Throw ps;

      png_set_IHDR(pp, pi, transform_width(pp, colour_type, bit_depth),
         transform_height(pp, colour_type, bit_depth), bit_depth, colour_type,
         interlace_type, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

      if (colour_type == 3) /* palette */
      {
         unsigned int i = 0;
         png_color pal[256];

         do
            pal[i].red = pal[i].green = pal[i].blue = (png_byte)i;
         while(++i < 256U);

         png_set_PLTE(pp, pi, pal, 256);
      }

      /* Time for a few errors, these are in various optional chunks, the
       * standard tests test the standard chunks pretty well.
       */
#     define exception__prev exception_prev_1
#     define exception__env exception_env_1
      Try
      {
         /* Expect this to throw: */
         ps->expect_error = !error_test[test].warning;
         ps->expect_warning = error_test[test].warning;
         ps->saw_warning = 0;
         error_test[test].fn(pp, pi);

         /* Normally the error is only detected here: */
         png_write_info(pp, pi);

         /* And handle the case where it was only a warning: */
         if (ps->expect_warning && ps->saw_warning)
            Throw ps;

         /* If we get here there is a problem, we have success - no error or
          * no warning - when we shouldn't have success.  Log an error.
          */
         store_log(ps, pp, error_test[test].msg, 1 /*error*/);
      }

      Catch (fault)
         ps = fault; /* expected exit, make sure ps is not clobbered */
#undef exception__prev
#undef exception__env

      /* And clear these flags */
      ps->expect_error = 0;
      ps->expect_warning = 0;

      /* Now write the whole image, just to make sure that the detected, or
       * undetected, errro has not created problems inside libpng.
       */
      if (png_get_rowbytes(pp, pi) !=
          transform_rowsize(pp, colour_type, bit_depth))
         png_error(pp, "row size incorrect");

      else
      {
         png_uint_32 h = transform_height(pp, colour_type, bit_depth);
         int npasses = png_set_interlace_handling(pp);
         int pass;

         if (npasses != npasses_from_interlace_type(pp, interlace_type))
            png_error(pp, "write: png_set_interlace_handling failed");

         for (pass=0; pass<npasses; ++pass)
         {
            png_uint_32 y;

            for (y=0; y<h; ++y)
            {
               png_byte buffer[TRANSFORM_ROWMAX];

               transform_row(pp, buffer, colour_type, bit_depth, y);
               png_write_row(pp, buffer);
            }
         }
      }

      png_write_end(pp, pi);

      /* The following deletes the file that was just written. */
      store_write_reset(ps);
   }

   Catch(fault)
   {
      store_write_reset(fault);
   }
}

static int
make_errors(png_modifier* PNG_CONST pm, png_byte PNG_CONST colour_type,
    int bdlo, int PNG_CONST bdhi)
{
   for (; bdlo <= bdhi; ++bdlo)
   {
      int interlace_type;

      for (interlace_type = PNG_INTERLACE_NONE;
           interlace_type < PNG_INTERLACE_LAST; ++interlace_type)
      {
         unsigned int test;
         char name[FILE_NAME_SIZE];

         standard_name(name, sizeof name, 0, colour_type, bdlo, interlace_type,
            0, 0, 0);

         for (test=0; test<(sizeof error_test)/(sizeof error_test[0]); ++test)
         {
            make_error(&pm->this, colour_type, DEPTH(bdlo), interlace_type,
               test, name);

            if (fail(pm))
               return 0;
         }
      }
   }

   return 1; /* keep going */
}

static void
perform_error_test(png_modifier *pm)
{
   /* Need to do this here because we just write in this test. */
   safecat(pm->this.test, sizeof pm->this.test, 0, "error test");

   if (!make_errors(pm, 0, 0, WRITE_BDHI))
      return;

   if (!make_errors(pm, 2, 3, WRITE_BDHI))
      return;

   if (!make_errors(pm, 3, 0, 3))
      return;

   if (!make_errors(pm, 4, 3, WRITE_BDHI))
      return;

   if (!make_errors(pm, 6, 3, WRITE_BDHI))
      return;
}

/* Because we want to use the same code in both the progressive reader and the
 * sequential reader it is necessary to deal with the fact that the progressive
 * reader callbacks only have one parameter (png_get_progressive_ptr()), so this
 * must contain all the test parameters and all the local variables directly
 * accessible to the sequential reader implementation.
 *
 * The technique adopted is to reinvent part of what Dijkstra termed a
 * 'display'; an array of pointers to the stack frames of enclosing functions so
 * that a nested function definition can access the local (C auto) variables of
 * the functions that contain its definition.  In fact C provides the first
 * pointer (the local variables - the stack frame pointer) and the last (the
 * global variables - the BCPL global vector typically implemented as global
 * addresses), this code requires one more pointer to make the display - the
 * local variables (and function call parameters) of the function that actually
 * invokes either the progressive or sequential reader.
 *
 * Perhaps confusingly this technique is confounded with classes - the
 * 'standard_display' defined here is sub-classed as the 'gamma_display' below.
 * A gamma_display is a standard_display, taking advantage of the ANSI-C
 * requirement that the pointer to the first member of a structure must be the
 * same as the pointer to the structure.  This allows us to reuse standard_
 * functions in the gamma test code; something that could not be done with
 * nested funtions!
 */
typedef struct standard_palette_entry /* pngvalid format palette! */
{
   png_byte red;
   png_byte green;
   png_byte blue;
   png_byte alpha;
} standard_palette[256];

typedef struct standard_display
{
   png_store*  ps;             /* Test parameters (passed to the function) */
   png_byte    colour_type;
   png_byte    bit_depth;
   png_byte    red_sBIT;       /* Input data sBIT values. */
   png_byte    green_sBIT;
   png_byte    blue_sBIT;
   png_byte    alpha_sBIT;
   int         interlace_type;
   png_uint_32 id;             /* Calculated file ID */
   png_uint_32 w;              /* Width of image */
   png_uint_32 h;              /* Height of image */
   int         npasses;        /* Number of interlaced passes */
   png_uint_32 pixel_size;     /* Width of one pixel in bits */
   png_uint_32 bit_width;      /* Width of output row in bits */
   size_t      cbRow;          /* Bytes in a row of the output image */
   int         do_interlace;   /* Do interlacing internally */
   int         is_transparent; /* Transparecy information was present. */
   struct
   {
      png_uint_16 red;
      png_uint_16 green;
      png_uint_16 blue;
   }           transparent;    /* The transparent color, if set. */
   standard_palette
               palette;
} standard_display;

static void
standard_display_init(standard_display *dp, png_store* ps, png_uint_32 id,
   int do_interlace)
{
   dp->ps = ps;
   dp->colour_type = COL_FROM_ID(id);
   dp->bit_depth = DEPTH_FROM_ID(id);
   dp->alpha_sBIT = dp->blue_sBIT = dp->green_sBIT = dp->alpha_sBIT =
      dp->bit_depth;
   dp->interlace_type = INTERLACE_FROM_ID(id);
   dp->id = id;
   /* All the rest are filled in after the read_info: */
   dp->w = 0;
   dp->h = 0;
   dp->npasses = 0;
   dp->pixel_size = 0;
   dp->bit_width = 0;
   dp->cbRow = 0;
   dp->do_interlace = do_interlace;
   dp->is_transparent = 0;
   /* Preset the transparent color to black: */
   memset(&dp->transparent, 0, sizeof dp->transparent);
   /* Preset the palette to full intensity/opaque througout: */
   memset(dp->palette, 0xff, sizeof dp->palette);

}

/* Call this only if the colour type is 3 - PNG_COLOR_TYPE_PALETTE - otherwise
 * it will png_error out.  The API returns true if tRNS information was
 * present.
 */
static int
standard_palette_init(standard_palette palette, png_structp pp, png_infop pi)
{
   png_colorp pal;
   png_bytep trans_alpha;
   int num;

   pal = 0;
   num = -1;
   if (png_get_PLTE(pp, pi, &pal, &num) & PNG_INFO_PLTE)
   {
      int i;

      for (i=0; i<num; ++i)
      {
         palette[i].red = pal[i].red;
         palette[i].green = pal[i].green;
         palette[i].blue = pal[i].blue;
      }

      /* Mark the remainder of the entries with a flag value: */
      for (; i<256; ++i)
         palette[i].red = palette[i].green = palette[i].blue = 126;
   }

   else /* !png_get_PLTE */
      png_error(pp, "validate: missing PLTE with color type 3");

   trans_alpha = 0;
   num = -1;
   if (png_get_tRNS(pp, pi, &trans_alpha, &num, 0) & PNG_INFO_tRNS)
   {
      int i;

      /* Any of these are crash-worthy - given the implementation of
       * png_get_tRNS up to 1.5 an app won't crash if it just checks the
       * result above and fails to check that the variables it passed have
       * actually been filled in!  Note that if the app were to pass the
       * last, png_color_16p, variable too it couldn't rely on this.
       */
      if (trans_alpha == 0 || num <= 0 || num > 256)
         png_error(pp, "validate: unexpected png_get_tRNS (palette) result");

      for (i=0; i<num; ++i)
         palette[i].alpha = trans_alpha[i];

      for (; i<256; ++i)
         palette[i].alpha = 255;

      return 1; /* transparency */
   }

   else
   {
      /* No transparency - just set the alpha channel to opaque. */
      int i;

      for (i=0; i<256; ++i)
         palette[i].alpha = 255;

      return 0; /* no transparency */
   }
}

/* By passing a 'standard_display' the progressive callbacks can be used
 * directly by the sequential code, the functions suffixed "_imp" are the
 * implementations, the functions without the suffix are the callbacks.
 *
 * The code for the info callback is split into two because this callback calls
 * png_read_update_info or png_start_read_image and what gets called depends on
 * whether the info needs updating (we want to test both calls in pngvalid.)
 */
static void
standard_info_part1(standard_display *dp, png_structp pp, png_infop pi)
{
   if (png_get_bit_depth(pp, pi) != dp->bit_depth)
      png_error(pp, "validate: bit depth changed");

   if (png_get_color_type(pp, pi) != dp->colour_type)
      png_error(pp, "validate: color type changed");

   if (png_get_filter_type(pp, pi) != PNG_FILTER_TYPE_BASE)
      png_error(pp, "validate: filter type changed");

   if (png_get_interlace_type(pp, pi) != dp->interlace_type)
      png_error(pp, "validate: interlacing changed");

   if (png_get_compression_type(pp, pi) != PNG_COMPRESSION_TYPE_BASE)
      png_error(pp, "validate: compression type changed");

   dp->w = png_get_image_width(pp, pi);

   if (dp->w != standard_width(pp, dp->id))
      png_error(pp, "validate: image width changed");

   dp->h = png_get_image_height(pp, pi);

   if (dp->h != standard_height(pp, dp->id))
      png_error(pp, "validate: image height changed");

   /* Record (but don't check at present) the input sBIT according to the colour
    * type information.
    */
   {
      png_color_8p sBIT = 0;

      if (png_get_sBIT(pp, pi, &sBIT) & PNG_INFO_sBIT)
      {
         int sBIT_invalid = 0;

         if (sBIT == 0)
            png_error(pp, "validate: unexpected png_get_sBIT result");

         if (dp->colour_type & PNG_COLOR_MASK_COLOR)
         {
            if (sBIT->red == 0 || sBIT->red > dp->bit_depth)
               sBIT_invalid = 1;
            else
               dp->red_sBIT = sBIT->red;

            if (sBIT->green == 0 || sBIT->green > dp->bit_depth)
               sBIT_invalid = 1;
            else
               dp->green_sBIT = sBIT->green;

            if (sBIT->blue == 0 || sBIT->blue > dp->bit_depth)
               sBIT_invalid = 1;
            else
               dp->blue_sBIT = sBIT->blue;
         }

         else /* !COLOR */
         {
            if (sBIT->gray == 0 || sBIT->gray > dp->bit_depth)
               sBIT_invalid = 1;
            else
               dp->blue_sBIT = dp->green_sBIT = dp->red_sBIT = sBIT->gray;
         }

         /* All 8 bits in tRNS for a palette image are significant - see the
          * spec.
          */
         if (dp->colour_type & PNG_COLOR_MASK_ALPHA)
         {
            if (sBIT->alpha == 0 || sBIT->alpha > dp->bit_depth)
               sBIT_invalid = 1;
            else
               dp->alpha_sBIT = sBIT->alpha;
         }

         if (sBIT_invalid)
            png_error(pp, "validate: sBIT value out of range");
      }
   }

   /* Important: this is validating the value *before* any transforms have been
    * put in place.  It doesn't matter for the standard tests, where there are
    * no transforms, but it does for other tests where rowbytes may change after
    * png_read_update_info.
    */
   if (png_get_rowbytes(pp, pi) != standard_rowsize(pp, dp->id))
      png_error(pp, "validate: row size changed");

   /* The palette is never read for non-palette images, even though it is valid
    * - this could be changed.
    */
   if (dp->colour_type == 3) /* palette */
   {
      int i;

      dp->is_transparent = standard_palette_init(dp->palette, pp, pi);

      /* And validate the result. */
      for (i=0; i<256; ++i)
         if (dp->palette[i].red != i || dp->palette[i].green != i ||
            dp->palette[i].blue != i)
            png_error(pp, "validate: color type 3 PLTE chunk changed");
   }

   /* In any case always check for a tranparent color: */
   {
      png_color_16p trans_color = 0;

      if (png_get_tRNS(pp, pi, 0, 0, &trans_color) & PNG_INFO_tRNS)
      {
         if (trans_color == 0)
            png_error(pp, "validate: unexpected png_get_tRNS (color) result");

         switch (dp->colour_type)
         {
         case 0:
            dp->transparent.red = dp->transparent.green = dp->transparent.blue =
               trans_color->gray;
            dp->is_transparent = 1;
            break;

         case 2:
            dp->transparent.red = trans_color->red;
            dp->transparent.green = trans_color->green;
            dp->transparent.blue = trans_color->blue;
            dp->is_transparent = 1;
            break;

         case 3:
            /* Not expected because it should result in the array case
             * above.
             */
            png_error(pp, "validate: unexpected png_get_tRNS result");
            break;

         default:
            png_error(pp, "validate: invalid tRNS chunk with alpha image");
         }
      }
   }

   /* Read the number of passes - expected to match the value used when
    * creating the image (interlaced or not).  This has the side effect of
    * turning on interlace handling (if do_interlace is not set.)
    */
   dp->npasses = npasses_from_interlace_type(pp, dp->interlace_type);
   if (!dp->do_interlace && dp->npasses != png_set_interlace_handling(pp))
      png_error(pp, "validate: file changed interlace type");

   /* Caller calls png_read_update_info or png_start_read_image now, then calls
    * part2.
    */
}

/* This must be called *after* the png_read_update_info call to get the correct
 * 'rowbytes' value, otherwise png_get_rowbytes will refer to the untransformed
 * image.
 */
static void
standard_info_part2(standard_display *dp, png_structp pp, png_infop pi,
    int nImages)
{
   /* Record cbRow now that it can be found. */
   dp->pixel_size = bit_size(pp, png_get_color_type(pp, pi),
      png_get_bit_depth(pp, pi));
   dp->bit_width = png_get_image_width(pp, pi) * dp->pixel_size;
   dp->cbRow = png_get_rowbytes(pp, pi);

   /* Validate the rowbytes here again. */
   if (dp->cbRow != (dp->bit_width+7)/8)
      png_error(pp, "bad png_get_rowbytes calculation");

   /* Then ensure there is enough space for the output image(s). */
   store_ensure_image(dp->ps, pp, nImages * dp->cbRow * dp->h);
}

static void
standard_info_imp(standard_display *dp, png_structp pp, png_infop pi,
    int nImages)
{
   /* Note that the validation routine has the side effect of turning on
    * interlace handling in the subsequent code.
    */
   standard_info_part1(dp, pp, pi);

   /* And the info callback has to call this (or png_read_update_info - see
    * below in the png_modifier code for that variant.
    */
   png_start_read_image(pp);

   /* Validate the height, width and rowbytes plus ensure that sufficient buffer
    * exists for decoding the image.
    */
   standard_info_part2(dp, pp, pi, nImages);
}

static void
standard_info(png_structp pp, png_infop pi)
{
   standard_display *dp = png_get_progressive_ptr(pp);

   /* Call with nImages==1 because the progressive reader can only produce one
    * image.
    */
   standard_info_imp(dp, pp, pi, 1 /*only one image*/);
}

static void
progressive_row(png_structp pp, png_bytep new_row, png_uint_32 y, int pass)
{
   PNG_CONST standard_display *dp = png_get_progressive_ptr(pp);

   /* When handling interlacing some rows will be absent in each pass, the
    * callback still gets called, but with a NULL pointer.  This is checked
    * in the 'else' clause below.  We need our own 'cbRow', but we can't call
    * png_get_rowbytes because we got no info structure.
    */
   if (new_row != NULL)
   {
      png_bytep row;

      /* In the case where the reader doesn't do the interlace it gives
       * us the y in the sub-image:
       */
      if (dp->do_interlace && dp->interlace_type == PNG_INTERLACE_ADAM7)
      {
         /* Use this opportunity to validate the png 'current' APIs: */
         if (y != png_get_current_row_number(pp))
            png_error(pp, "png_get_current_row_number is broken");

         if (pass != png_get_current_pass_number(pp))
            png_error(pp, "png_get_current_pass_number is broken");

         y = PNG_ROW_FROM_PASS_ROW(y, pass);
      }

      /* Validate this just in case. */
      if (y >= dp->h)
         png_error(pp, "invalid y to progressive row callback");

      row = dp->ps->image + y * dp->cbRow;

      /* Combine the new row into the old: */
      if (dp->do_interlace)
      {
         if (dp->interlace_type == PNG_INTERLACE_ADAM7)
            deinterlace_row(row, new_row, dp->pixel_size, dp->w, pass);
         else
            memcpy(row, new_row, dp->cbRow);
      }
      else
         png_progressive_combine_row(pp, row, new_row);
   } else if (dp->interlace_type == PNG_INTERLACE_ADAM7 &&
      PNG_ROW_IN_INTERLACE_PASS(y, pass) &&
      PNG_PASS_COLS(dp->w, pass) > 0)
      png_error(pp, "missing row in progressive de-interlacing");
}

static void
sequential_row(standard_display *dp, png_structp pp, png_infop pi,
    PNG_CONST png_bytep pImage, PNG_CONST png_bytep pDisplay)
{
   PNG_CONST int         npasses = dp->npasses;
   PNG_CONST int         do_interlace = dp->do_interlace &&
      dp->interlace_type == PNG_INTERLACE_ADAM7;
   PNG_CONST png_uint_32 height = standard_height(pp, dp->id);
   PNG_CONST png_uint_32 width = standard_width(pp, dp->id);
   PNG_CONST size_t      cbRow = dp->cbRow;
   int pass;

   for (pass=0; pass<npasses; ++pass)
   {
      png_uint_32 y;
      png_uint_32 wPass = PNG_PASS_COLS(width, pass);
      png_bytep pRow1 = pImage;
      png_bytep pRow2 = pDisplay;

      for (y=0; y<height; ++y)
      {
         if (do_interlace)
         {
            /* wPass may be zero or this row may not be in this pass.
             * png_read_row must not be called in either case.
             */
            if (wPass > 0 && PNG_ROW_IN_INTERLACE_PASS(y, pass))
            {
               /* Read the row into a pair of temporary buffers, then do the
                * merge here into the output rows.
                */
               png_byte row[STANDARD_ROWMAX], display[STANDARD_ROWMAX];

               /* The following aids (to some extent) error detection - we can
                * see where png_read_row wrote.  Use opposite values in row and
                * display to make this easier.
                */
               memset(row, 0xff, sizeof row);
               memset(display, 0, sizeof display);

               png_read_row(pp, row, display);

               if (pRow1 != NULL)
                  deinterlace_row(pRow1, row, dp->pixel_size, dp->w, pass);

               if (pRow2 != NULL)
                  deinterlace_row(pRow2, display, dp->pixel_size, dp->w, pass);
            }
         }
         else
            png_read_row(pp, pRow1, pRow2);

         if (pRow1 != NULL)
            pRow1 += cbRow;

         if (pRow2 != NULL)
            pRow2 += cbRow;
      }
   }

   /* And finish the read operation (only really necessary if the caller wants
    * to find additional data in png_info from chunks after the last IDAT.)
    */
   png_read_end(pp, pi);
}

static void
standard_row_validate(standard_display *dp, png_structp pp, png_const_bytep row,
   png_const_bytep display, png_uint_32 y)
{
   png_byte std[STANDARD_ROWMAX];

   memset(std, 0xff, sizeof std);
   standard_row(pp, std, dp->id, y);

   /* At the end both the 'row' and 'display' arrays should end up identical.
    * In earlier passes 'row' will be partially filled in, with only the pixels
    * that have been read so far, but 'display' will have those pixels
    * replicated to fill the unread pixels while reading an interlaced image.
    * The side effect inside the libpng sequential reader is that the 'row'
    * array retains the correct values for unwritten pixels within the row
    * bytes, while the 'display' array gets bits off the end of the image (in
    * the last byte) trashed.  Unfortunately in the progressive reader the
    * row bytes are always trashed, so we always do a pixel_cmp here even though
    * a memcmp of all cbRow bytes will succeed for the sequential reader.
    */
   if (row != NULL && pixel_cmp(std, row, dp->bit_width) != 0)
   {
      char msg[64];
      sprintf(msg, "PNG image row %d changed", y);
      png_error(pp, msg);
   }

   /* In this case use pixel_cmp because we need to compare a partial
    * byte at the end of the row if the row is not an exact multiple
    * of 8 bits wide.
    */
   if (display != NULL && pixel_cmp(std, display, dp->bit_width) != 0)
   {
      char msg[64];
      sprintf(msg, "display row %d changed", y);
      png_error(pp, msg);
   }
}

static void
standard_image_validate(standard_display *dp, png_structp pp,
   png_const_bytep pImage, png_const_bytep pDisplay)
{
   png_uint_32 y;

   for (y=0; y<dp->h; ++y)
   {
      standard_row_validate(dp, pp, pImage, pDisplay, y);

      if (pImage != NULL)
         pImage += dp->cbRow;

      if (pDisplay != NULL)
         pDisplay += dp->cbRow;
   }

   /* This avoids false positives if the validation code is never called! */
   dp->ps->validated = 1;
}

static void
standard_end(png_structp pp, png_infop pi)
{
   standard_display *dp = png_get_progressive_ptr(pp);

   UNUSED(pi)

   /* Validate the image - progressive reading only produces one variant for
    * interlaced images.
    */
   standard_image_validate(dp, pp, dp->ps->image, NULL);
}

/* A single test run checking the standard image to ensure it is not damaged. */
static void
standard_test(png_store* PNG_CONST psIn, png_uint_32 PNG_CONST id,
   int do_interlace)
{
   standard_display d;
   context(psIn, fault);

   /* Set up the display (stack frame) variables from the arguments to the
    * function and initialize the locals that are filled in later.
    */
   standard_display_init(&d, psIn, id, do_interlace);

   /* Everything is protected by a Try/Catch.  The functions called also
    * typically have local Try/Catch blocks.
    */
   Try
   {
      png_structp pp;
      png_infop pi;

      /* Get a png_struct for reading the image. This will throw an error if it
       * fails, so we don't need to check the result.
       */
      pp = set_store_for_read(d.ps, &pi, d.id,
         d.do_interlace ?  (d.ps->progressive ?
            "pngvalid progressive deinterlacer" :
            "pngvalid sequential deinterlacer") : (d.ps->progressive ?
               "progressive reader" : "sequential reader"));

      /* Introduce the correct read function. */
      if (d.ps->progressive)
      {
         png_set_progressive_read_fn(pp, &d, standard_info, progressive_row,
            standard_end);

         /* Now feed data into the reader until we reach the end: */
         store_progressive_read(d.ps, pp, pi);
      }
      else
      {
         /* Note that this takes the store, not the display. */
         png_set_read_fn(pp, d.ps, store_read);

         /* Check the header values: */
         png_read_info(pp, pi);

         /* The code tests both versions of the images that the sequential
          * reader can produce.
          */
         standard_info_imp(&d, pp, pi, 2 /*images*/);

         /* Need the total bytes in the image below; we can't get to this point
          * unless the PNG file values have been checked against the expected
          * values.
          */
         {
            PNG_CONST png_bytep pImage = d.ps->image;
            PNG_CONST png_bytep pDisplay = pImage + d.cbRow * d.h;

            sequential_row(&d, pp, pi, pImage, pDisplay);

            /* After the last pass loop over the rows again to check that the
             * image is correct.
             */
            standard_image_validate(&d, pp, pImage, pDisplay);
         }
      }

      /* Check for validation. */
      if (!d.ps->validated)
         png_error(pp, "image read failed silently");

      /* Successful completion. */
   }

   Catch(fault)
      d.ps = fault; /* make sure this hasn't been clobbered. */

   /* In either case clean up the store. */
   store_read_reset(d.ps);
}

static int
test_standard(png_modifier* PNG_CONST pm, png_byte PNG_CONST colour_type,
    int bdlo, int PNG_CONST bdhi)
{
   for (; bdlo <= bdhi; ++bdlo)
   {
      int interlace_type;

      for (interlace_type = PNG_INTERLACE_NONE;
           interlace_type < PNG_INTERLACE_LAST; ++interlace_type)
      {
         standard_test(&pm->this, FILEID(colour_type, DEPTH(bdlo),
            interlace_type, 0, 0, 0), 0/*do_interlace*/);

         if (fail(pm))
            return 0;
      }
   }

   return 1; /* keep going */
}

static void
perform_standard_test(png_modifier *pm)
{
   /* Test each colour type over the valid range of bit depths (expressed as
    * log2(bit_depth) in turn, stop as soon as any error is detected.
    */
   if (!test_standard(pm, 0, 0, READ_BDHI))
      return;

   if (!test_standard(pm, 2, 3, READ_BDHI))
      return;

   if (!test_standard(pm, 3, 0, 3))
      return;

   if (!test_standard(pm, 4, 3, READ_BDHI))
      return;

   if (!test_standard(pm, 6, 3, READ_BDHI))
      return;
}


/********************************** SIZE TESTS ********************************/
static int
test_size(png_modifier* PNG_CONST pm, png_byte PNG_CONST colour_type,
    int bdlo, int PNG_CONST bdhi)
{
   /* Run the tests on each combination.
    *
    * NOTE: on my 32 bit x86 each of the following blocks takes
    * a total of 3.5 seconds if done across every combo of bit depth
    * width and height.  This is a waste of time in practice, hence the
    * hinc and winc stuff:
    */
   static PNG_CONST png_byte hinc[] = {1, 3, 11, 1, 5};
   static PNG_CONST png_byte winc[] = {1, 9, 5, 7, 1};
   for (; bdlo <= bdhi; ++bdlo)
   {
      png_uint_32 h, w;

      for (h=1; h<=16; h+=hinc[bdlo]) for (w=1; w<=16; w+=winc[bdlo])
      {
         /* First test all the 'size' images against the sequential
          * reader using libpng to deinterlace (where required.)  This
          * validates the write side of libpng.  There are four possibilities
          * to validate.
          */
         standard_test(&pm->this, FILEID(colour_type, DEPTH(bdlo),
            PNG_INTERLACE_NONE, w, h, 0), 0/*do_interlace*/);

         if (fail(pm))
            return 0;

         standard_test(&pm->this, FILEID(colour_type, DEPTH(bdlo),
            PNG_INTERLACE_NONE, w, h, 1), 0/*do_interlace*/);

         if (fail(pm))
            return 0;

         standard_test(&pm->this, FILEID(colour_type, DEPTH(bdlo),
            PNG_INTERLACE_ADAM7, w, h, 0), 0/*do_interlace*/);

         if (fail(pm))
            return 0;

         standard_test(&pm->this, FILEID(colour_type, DEPTH(bdlo),
            PNG_INTERLACE_ADAM7, w, h, 1), 0/*do_interlace*/);

         if (fail(pm))
            return 0;

         /* Now validate the interlaced read side - do_interlace true,
          * in the progressive case this does actually make a difference
          * to the code used in the non-interlaced case too.
          */
         standard_test(&pm->this, FILEID(colour_type, DEPTH(bdlo),
            PNG_INTERLACE_NONE, w, h, 0), 1/*do_interlace*/);

         if (fail(pm))
            return 0;

         standard_test(&pm->this, FILEID(colour_type, DEPTH(bdlo),
            PNG_INTERLACE_ADAM7, w, h, 0), 1/*do_interlace*/);

         if (fail(pm))
            return 0;
      }
   }

   return 1; /* keep going */
}

static void
perform_size_test(png_modifier *pm)
{
   /* Test each colour type over the valid range of bit depths (expressed as
    * log2(bit_depth) in turn, stop as soon as any error is detected.
    */
   if (!test_size(pm, 0, 0, READ_BDHI))
      return;

   if (!test_size(pm, 2, 3, READ_BDHI))
      return;

   /* For the moment don't do the palette test - it's a waste of time when
    * compared to the greyscale test.
    */
#if 0
   if (!test_size(pm, 3, 0, 3))
      return;
#endif

   if (!test_size(pm, 4, 3, READ_BDHI))
      return;

   if (!test_size(pm, 6, 3, READ_BDHI))
      return;
}


/******************************* TRANSFORM TESTS ******************************/
/* A set of tests to validate libpng image transforms.  The possibilities here
 * are legion because the transforms can be combined in a combinatorial
 * fashion.  To deal with this some measure of restraint is required, otherwise
 * the tests would take forever.
 */
typedef struct image_pixel
{
   /* A local (pngvalid) representation of a PNG pixel, in all its
    * various forms.
    */
   unsigned int red, green, blue, alpha; /* For non-palette images. */
   unsigned int palette_index;           /* For a palette image. */
   png_byte colour_type;                 /* As in the spec. */
   png_byte bit_depth;                   /* Defines bit size in row */
   png_byte sample_depth;                /* Scale of samples */
   int      have_tRNS;                   /* tRNS chunk may need processing */

   /* For checking the code calculates double precision floating point values
    * along with an error value, accumulated from the transforms.  Because an
    * sBIT setting allows larger error bounds (indeed, by the spec, apparently
    * up to just less than +/-1 in the scaled value) the *lowest* sBIT for each
    * channel is stored.  This sBIT value is folded in to the stored error value
    * at the end of the application of the transforms to the pixel.
    */
   double   redf, greenf, bluef, alphaf;
   double   rede, greene, bluee, alphae;
   png_byte red_sBIT, green_sBIT, blue_sBIT, alpha_sBIT;
} image_pixel;

/* Shared utility function, see below. */
static void
image_pixel_setf(image_pixel *this, unsigned int max)
{
   this->redf = this->red / (double)max;
   this->greenf = this->green / (double)max;
   this->bluef = this->blue / (double)max;
   this->alphaf = this->alpha / (double)max;

   if (this->red < max)
      this->rede = this->redf * DBL_EPSILON;
   else
      this->rede = 0;
   if (this->green < max)
      this->greene = this->greenf * DBL_EPSILON;
   else
      this->greene = 0;
   if (this->blue < max)
      this->bluee = this->bluef * DBL_EPSILON;
   else
      this->bluee = 0;
   if (this->alpha < max)
      this->alphae = this->alphaf * DBL_EPSILON;
   else
      this->alphae = 0;
}

/* Initialize the structure for the next pixel - call this before doing any
 * transforms and call it for each pixel since all the fields may need to be
 * reset.
 */
static void
image_pixel_init(image_pixel *this, png_const_bytep row, png_byte colour_type,
    png_byte bit_depth, png_uint_32 x, standard_palette palette)
{
   PNG_CONST png_byte sample_depth = (png_byte)(colour_type ==
      PNG_COLOR_TYPE_PALETTE ? 8 : bit_depth);
   PNG_CONST unsigned int max = (1U<<sample_depth)-1;

   /* Initially just set everything to the same number and the alpha to opaque.
    * Note that this currently assumes a simple palette where entry x has colour
    * rgb(x,x,x)!
    */
   this->palette_index = this->red = this->green = this->blue =
      sample(row, colour_type, bit_depth, x, 0);
   this->alpha = max;
   this->red_sBIT = this->green_sBIT = this->blue_sBIT = this->alpha_sBIT =
      sample_depth;

   /* Then override as appropriate: */
   if (colour_type == 3) /* palette */
   {
      /* This permits the caller to default to the sample value. */
      if (palette != 0)
      {
         PNG_CONST unsigned int i = this->palette_index;

         this->red = palette[i].red;
         this->green = palette[i].green;
         this->blue = palette[i].blue;
         this->alpha = palette[i].alpha;
      }
   }

   else /* not palette */
   {
      unsigned int i = 0;

      if (colour_type & 2)
      {
         this->green = sample(row, colour_type, bit_depth, x, 1);
         this->blue = sample(row, colour_type, bit_depth, x, 2);
         i = 2;
      }
      if (colour_type & 4)
         this->alpha = sample(row, colour_type, bit_depth, x, ++i);
   }

   /* Calculate the scaled values, these are simply the values divided by
    * 'max' and the error is initialized to the double precision epsilon value
    * from the header file.
    */
   image_pixel_setf(this, max);

   /* Store the input information for use in the transforms - these will
    * modify the information.
    */
   this->colour_type = colour_type;
   this->bit_depth = bit_depth;
   this->sample_depth = sample_depth;
   this->have_tRNS = 0;
}

/* Convert a palette image to an rgb image.  This necessarily converts the tRNS
 * chunk at the same time, because the tRNS will be in palette form.
 */
static void
image_pixel_convert_PLTE(image_pixel *this, const standard_display *display)
{
   if (this->colour_type == PNG_COLOR_TYPE_PALETTE)
   {
      PNG_CONST unsigned int i = this->palette_index;

      this->bit_depth = this->sample_depth;
      this->red = display->palette[i].red;
      this->green = display->palette[i].green;
      this->blue = display->palette[i].blue;
      this->red_sBIT = display->red_sBIT;
      this->green_sBIT = display->green_sBIT;
      this->blue_sBIT = display->blue_sBIT;

      if (this->have_tRNS)
      {
         this->alpha = display->palette[i].alpha;
         this->colour_type = PNG_COLOR_TYPE_RGB_ALPHA;
         this->have_tRNS = 0;
      }
      else
      {
         this->alpha = 255;
         this->colour_type = PNG_COLOR_TYPE_RGB;
      }
      this->alpha_sBIT = 8;

      /* And regenerate the scaled values and all the errors, which are now set
       * back to the initial values.
       */
      image_pixel_setf(this, 255);
   }
}

/* Add an alpha channel, this will glom in the tRNS information because tRNS is
 * not valid in an alpha image.  The bit depth will invariably be set to at
 * least 8.  Palette images will be converted to alpha (using the above API).
 */
static void
image_pixel_add_alpha(image_pixel *this, const standard_display *display)
{
   if (this->colour_type == PNG_COLOR_TYPE_PALETTE)
      image_pixel_convert_PLTE(this, display);

   if ((this->colour_type & PNG_COLOR_MASK_ALPHA) == 0)
   {
      if (this->colour_type == PNG_COLOR_TYPE_GRAY)
      {
         if (this->bit_depth < 8)
            this->bit_depth = 8;

         if (this->have_tRNS)
         {
            this->have_tRNS = 0;

            /* Check the input, original, channel value here against the
             * original tRNS gray chunk valie.
             */
            if (this->red == display->transparent.red)
               this->alphaf = 0;
            else
               this->alphaf = 1;
         }
         else
            this->alphaf = 1;

         this->colour_type = PNG_COLOR_TYPE_GRAY_ALPHA;
      }

      else if (this->colour_type == PNG_COLOR_TYPE_RGB)
      {
         if (this->have_tRNS)
         {
            this->have_tRNS = 0;

            /* Again, check the exact input values, not the current transformed
             * value!
             */
            if (this->red == display->transparent.red &&
               this->green == display->transparent.green &&
               this->blue == display->transparent.blue)
               this->alphaf = 0;
            else
               this->alphaf = 1;

            this->colour_type = PNG_COLOR_TYPE_RGB_ALPHA;
         }
      }

      /* The error in the alpha is zero and the sBIT value comes from the
       * original sBIT data (actually it will always be the original bit depth).
       */
      this->alphae = 0;
      this->alpha_sBIT = display->alpha_sBIT;
   }
}

struct transform_display;
typedef struct image_transform
{
   /* The name of this transform: a string. */
   PNG_CONST char *name;

   /* Each transform can be disabled from the command line: */
   int enable;

   /* The global list of transforms; read only. */
   struct image_transform *PNG_CONST list;

   /* The global count of the number of times this transform has been set on an
    * image.
    */
   unsigned int global_use;

   /* The local count of the number of times this transform has been set. */
   unsigned int local_use;

   /* The next transform in the list, each transform must call its own next
    * transform after it has processed the pixel successfully.
    */
   PNG_CONST struct image_transform *next;

   /* A single transform for the image, expressed as a series of function
    * callbacks and some space for values.
    *
    * First a callback to set the transform on the current png_read_struct:
    */
   void (*set)(PNG_CONST struct image_transform *this,
      struct transform_display *that, png_structp pp, png_infop pi);

   /* Then a transform that takes an input pixel in one PNG format or another
    * and modifies it by a pngvalid implementation of the transform (thus
    * duplicating the libpng intent without, we hope, duplicating the bugs
    * in the libpng implementation!)  The png_structp is solely to allow error
    * reporting via png_error and png_warning.
    */
   void (*mod)(PNG_CONST struct image_transform *this, image_pixel *that,
      png_structp pp, PNG_CONST struct transform_display *display);

   /* Add this transform to the list and return true if the transform is
    * meaningful for this colour type and bit depth - if false then the
    * transform should have no effect on the image so there's not a lot of
    * point running it.
    */
   int (*add)(struct image_transform *this,
      PNG_CONST struct image_transform **that, png_byte colour_type,
      png_byte bit_depth);
} image_transform;

typedef struct transform_display
{
   standard_display this;

   /* Parameters */
   png_modifier*              pm;
   PNG_CONST image_transform* transform_list;

   /* Local variables */
   png_byte output_colour_type;
   png_byte output_bit_depth;
} transform_display;

/* Two functions to end the list: */
static void
image_transform_set_end(PNG_CONST image_transform *this,
   transform_display *that, png_structp pp, png_infop pi)
{
   UNUSED(this)
   UNUSED(that)
   UNUSED(pp)
   UNUSED(pi)
}

/* At the end of the list recalculate the output image pixel value from the
 * double precision values set up by the preceding 'mod' calls:
 */
static unsigned int
sample_scale(double sample_value, unsigned int scale)
{
   sample_value = floor(sample_value * scale + .5);

   /* Return NaN as 0: */
   if (!(sample_value > 0))
      sample_value = 0;
   else if (sample_value > scale)
      sample_value = scale;

   return (unsigned int)sample_value;
}

static void
image_transform_mod_end(PNG_CONST image_transform *this, image_pixel *that,
    png_structp pp, PNG_CONST transform_display *display)
{
   PNG_CONST unsigned int scale = (1U<<that->sample_depth)-1;

   UNUSED(this)
   UNUSED(pp)
   UNUSED(display)

   /* At the end recalculate the digitized red green and blue values according
    * to the current sample_depth of the pixel.
    *
    * The sample value is simply scaled to the maximum, checking for over
    * and underflow (which can both happen for some image transforms,
    * including simple size scaling, though libpng doesn't do that at present.
    */
   that->red = sample_scale(that->redf, scale);

   /* The error value is increased, at the end, according to the lowest sBIT
    * value seen.  Common sense tells us that the intermediate integer
    * representations are no more accurate than +/- 0.5 in the integral values,
    * the sBIT allows the implementation to be worse than this.  In addition the
    * PNG specification actually permits any error within the range (-1..+1),
    * but that is ignored here.  Instead the final digitized value is compared,
    * below to the digitized value of the error limits - this has the net effect
    * of allowing (almost) +/-1 in the output value.  It's difficult to see how
    * any algorithm that digitizes intermediate results can be more accurate.
    */
   that->rede += 1./(2*((1U<<that->red_sBIT)-1));

   if (that->colour_type & PNG_COLOR_MASK_COLOR)
   {
      that->green = sample_scale(that->greenf, scale);
      that->blue = sample_scale(that->bluef, scale);
      that->greene += 1./(2*((1U<<that->green_sBIT)-1));
      that->bluee += 1./(2*((1U<<that->blue_sBIT)-1));
   }
   else
   {
      that->blue = that->green = that->red;
      that->bluef = that->greenf = that->redf;
      that->bluee = that->greene = that->rede;
   }

   if ((that->colour_type & PNG_COLOR_MASK_ALPHA) ||
      that->colour_type == PNG_COLOR_TYPE_PALETTE)
   {
      that->alpha = sample_scale(that->alphaf, scale);
      that->alphae += 1./(2*((1U<<that->alpha_sBIT)-1));
   }
   else
   {
      that->alpha = scale; /* opaque */
      that->alpha = 1;     /* Override this. */
      that->alphae = 0;    /* It's exact ;-) */
   }
}

/* Static 'end' structure: */
static image_transform image_transform_end =
{
   "(end)", /* name */
   1, /* enable */
   0, /* list */
   0, /* global_use */
   0, /* local_use */
   0, /* next */
   image_transform_set_end,
   image_transform_mod_end,
   0 /* never called, I want it to crash if it is! */
};

/* Reader callbacks and implementations, where they differ from the standard
 * ones.
 */
static void
transform_display_init(transform_display *dp, png_modifier *pm, png_uint_32 id,
    PNG_CONST image_transform *transform_list)
{
   /* Standard fields */
   standard_display_init(&dp->this, &pm->this, id, 0/*do_interlace*/);

   /* Parameter fields */
   dp->pm = pm;
   dp->transform_list = transform_list;

   /* Local variable fields */
   dp->output_colour_type = 255; /* invalid */
   dp->output_bit_depth = 255;  /* invalid */
}

static void
transform_info_imp(transform_display *dp, png_structp pp, png_infop pi)
{
   /* Reuse the standard stuff as appropriate. */
   standard_info_part1(&dp->this, pp, pi);

   /* Now set the list of transforms. */
   dp->transform_list->set(dp->transform_list, dp, pp, pi);

   /* Update the info structure for these transforms: */
   png_read_update_info(pp, pi);

   /* And get the output information into the standard_display */
   standard_info_part2(&dp->this, pp, pi, 1/*images*/);

   /* Plus the extra stuff we need for the transform tests: */
   dp->output_colour_type = png_get_color_type(pp, pi);
   dp->output_bit_depth = png_get_bit_depth(pp, pi);

   /* Validate the combination of colour type and bit depth that we are getting
    * out of libpng; the semantics of something not in the PNG spec are, at
    * best, unclear.
    */
   switch (dp->output_colour_type)
   {
   case PNG_COLOR_TYPE_PALETTE:
      if (dp->output_bit_depth > 8) goto error;
      /*FALL THROUGH*/
   case PNG_COLOR_TYPE_GRAY:
      if (dp->output_bit_depth == 1 || dp->output_bit_depth == 2 ||
         dp->output_bit_depth == 4)
         break;
      /*FALL THROUGH*/
   default:
      if (dp->output_bit_depth == 8 || dp->output_bit_depth == 16)
         break;
      /*FALL THROUGH*/
   error:
      {
         char message[128];
         size_t pos;

         pos = safecat(message, sizeof message, 0,
            "invalid final bit depth: colour type(");
         pos = safecatn(message, sizeof message, pos, dp->output_colour_type);
         pos = safecat(message, sizeof message, pos, ") with bit depth: ");
         pos = safecatn(message, sizeof message, pos, dp->output_bit_depth);

         png_error(pp, message);
      }
   }

   /* Use a test pixel to check that the output agrees with what we expect -
    * this avoids running the whole test if the output is unexpected.
    */
   {
      image_pixel test_pixel;

      memset(&test_pixel, 0, sizeof test_pixel);
      test_pixel.colour_type = dp->this.colour_type; /* input */
      test_pixel.bit_depth = dp->this.bit_depth;
      if (test_pixel.colour_type == PNG_COLOR_TYPE_PALETTE)
         test_pixel.sample_depth = 8;
      else
         test_pixel.sample_depth = test_pixel.bit_depth;
      /* Don't need sBIT here */
      test_pixel.have_tRNS = dp->this.is_transparent;

      dp->transform_list->mod(dp->transform_list, &test_pixel, pp, dp);

      if (test_pixel.colour_type != dp->output_colour_type)
      {
         char message[128];
         size_t pos = safecat(message, sizeof message, 0, "colour type ");

         pos = safecatn(message, sizeof message, pos, dp->output_colour_type);
         pos = safecat(message, sizeof message, pos, " expected ");
         pos = safecatn(message, sizeof message, pos, test_pixel.colour_type);

         png_error(pp, message);
      }

      if (test_pixel.bit_depth != dp->output_bit_depth)
      {
         char message[128];
         size_t pos = safecat(message, sizeof message, 0, "bit depth ");

         pos = safecatn(message, sizeof message, pos, dp->output_bit_depth);
         pos = safecat(message, sizeof message, pos, " expected ");
         pos = safecatn(message, sizeof message, pos, test_pixel.bit_depth);

         png_error(pp, message);
      }

      /* If both bit depth and colour type are correct check the sample depth.
       * I believe these are both internal errors.
       */
      if (test_pixel.colour_type == PNG_COLOR_TYPE_PALETTE)
      {
         if (test_pixel.sample_depth != 8) /* oops - internal error! */
            png_error(pp, "pngvalid: internal: palette sample depth not 8");
      }
      else if (test_pixel.sample_depth != dp->output_bit_depth)
      {
         char message[128];
         size_t pos = safecat(message, sizeof message, 0,
            "internal: sample depth ");

         pos = safecatn(message, sizeof message, pos, dp->output_bit_depth);
         pos = safecat(message, sizeof message, pos, " expected ");
         pos = safecatn(message, sizeof message, pos, test_pixel.sample_depth);

         png_error(pp, message);
      }
   }
}

static void
transform_info(png_structp pp, png_infop pi)
{
   transform_info_imp(png_get_progressive_ptr(pp), pp, pi);
}

static void
transform_range_check(png_structp pp, unsigned int r, unsigned int g,
   unsigned int b, unsigned int a, unsigned int in_digitized, double in,
   unsigned int out, png_byte sample_depth, double err, PNG_CONST char *name)
{
   /* Compare the scaled, digitzed, values of our local calculation (in+-err)
    * with the digitized values libpng produced;  'sample_depth' is the actual
    * digitization depth of the libpng output colors (the bit depth except for
    * palette images where it is always 8.)
    */
   unsigned int max = (1U<<sample_depth)-1;
   double in_min = ceil((in-err)*max - .5);
   double in_max = floor((in+err)*max + .5);
   if (!(out >= in_min && out <= in_max))
   {
      char message[256];
      size_t pos;

      pos = safecat(message, sizeof message, 0, name);
      pos = safecat(message, sizeof message, pos, " output value error: rgba(");
      pos = safecatn(message, sizeof message, pos, r);
      pos = safecat(message, sizeof message, pos, ",");
      pos = safecatn(message, sizeof message, pos, g);
      pos = safecat(message, sizeof message, pos, ",");
      pos = safecatn(message, sizeof message, pos, b);
      pos = safecat(message, sizeof message, pos, ",");
      pos = safecatn(message, sizeof message, pos, a);
      pos = safecat(message, sizeof message, pos, "): ");
      pos = safecatn(message, sizeof message, pos, out);
      pos = safecat(message, sizeof message, pos, " expected: ");
      pos = safecatn(message, sizeof message, pos, in_digitized);
      pos = safecat(message, sizeof message, pos, " (");
      pos = safecatd(message, sizeof message, pos, (in-err)*max, 3);
      pos = safecat(message, sizeof message, pos, "..");
      pos = safecatd(message, sizeof message, pos, (in+err)*max, 3);
      pos = safecat(message, sizeof message, pos, ")");

      png_error(pp, message);
   }
}

static void
transform_image_validate(transform_display *dp, png_structp pp, png_infop pi,
    png_const_bytep pRow)
{
   /* Constants for the loop below: */
   PNG_CONST png_byte in_ct = dp->this.colour_type;
   PNG_CONST png_byte in_bd = dp->this.bit_depth;
   PNG_CONST png_uint_32 w = dp->this.w;
   PNG_CONST png_uint_32 h = dp->this.h;
   PNG_CONST size_t cbRow = dp->this.cbRow;
   PNG_CONST png_byte out_ct = dp->output_colour_type;
   PNG_CONST png_byte out_bd = dp->output_bit_depth;
   PNG_CONST png_byte sample_depth = (png_byte)(out_ct ==
      PNG_COLOR_TYPE_PALETTE ? 8 : out_bd);
   PNG_CONST png_byte red_sBIT = dp->this.red_sBIT;
   PNG_CONST png_byte green_sBIT = dp->this.green_sBIT;
   PNG_CONST png_byte blue_sBIT = dp->this.blue_sBIT;
   PNG_CONST png_byte alpha_sBIT = dp->this.alpha_sBIT;
   PNG_CONST int have_tRNS = dp->this.is_transparent;

   standard_palette out_palette;
   png_uint_32 y;

   UNUSED(pi)

   /* Read the palette corresponding to the output if the output colour type
    * indicates a palette, othewise set out_palette to garbage.
    */
   if (out_ct == PNG_COLOR_TYPE_PALETTE)
      (void)standard_palette_init(out_palette, pp, pi);
   else
      memset(out_palette, 0x5e, sizeof out_palette);

   for (y=0; y<h; ++y, pRow += cbRow)
   {
      png_uint_32 x;

      /* The original, standard, row pre-transforms. */
      png_byte std[STANDARD_ROWMAX];

      transform_row(pp, std, in_ct, in_bd, y);

      /* Go through each original pixel transforming it and comparing with what
       * libpng did to the same pixel.
       */
      for (x=0; x<w; ++x)
      {
         image_pixel in_pixel, out_pixel;
         unsigned int r, g, b, a;

         /* Find out what we think the pixel should be: */
         image_pixel_init(&in_pixel, std, in_ct, in_bd, x, dp->this.palette);

         in_pixel.red_sBIT = red_sBIT;
         in_pixel.green_sBIT = green_sBIT;
         in_pixel.blue_sBIT = blue_sBIT;
         in_pixel.alpha_sBIT = alpha_sBIT;
         in_pixel.have_tRNS = have_tRNS;

         /* For error detection, below. */
         r = in_pixel.red;
         g = in_pixel.green;
         b = in_pixel.blue;
         a = in_pixel.alpha;

         dp->transform_list->mod(dp->transform_list, &in_pixel, pp, dp);

         /* Read the output pixel and compare it to what we got, we don't
          * use the error field here, so no need to update sBIT.
          */
         image_pixel_init(&out_pixel, pRow, out_ct, out_bd, x, out_palette);

         /* We don't expect changes to the index here even if the bit depth is
          * changed.
          */
         if (in_ct == PNG_COLOR_TYPE_PALETTE &&
            out_ct == PNG_COLOR_TYPE_PALETTE)
         {
            if (in_pixel.palette_index != out_pixel.palette_index)
               png_error(pp, "unexpected transformed palette index");
         }

         /* Check the colours for palette images too - in fact the palette could
          * be separately verified itself in most cases.
          */
         if (in_pixel.red != out_pixel.red)
            transform_range_check(pp, r, g, b, a, in_pixel.red, in_pixel.redf,
               out_pixel.red, sample_depth, in_pixel.rede, "red/gray");

         if ((out_ct & PNG_COLOR_MASK_COLOR) != 0 &&
            in_pixel.green != out_pixel.green)
            transform_range_check(pp, r, g, b, a, in_pixel.green,
               in_pixel.greenf, out_pixel.green, sample_depth, in_pixel.greene,
               "green");

         if ((out_ct & PNG_COLOR_MASK_COLOR) != 0 &&
            in_pixel.blue != out_pixel.blue)
            transform_range_check(pp, r, g, b, a, in_pixel.blue, in_pixel.bluef,
               out_pixel.blue, sample_depth, in_pixel.bluee, "blue");

         if ((out_ct & PNG_COLOR_MASK_ALPHA) != 0 &&
            in_pixel.alpha != out_pixel.alpha)
            transform_range_check(pp, r, g, b, a, in_pixel.alpha,
               in_pixel.alphaf, out_pixel.alpha, sample_depth, in_pixel.alphae,
               "alpha");
      } /* pixel (x) loop */
   } /* row (y) loop */

   /* Record that something was actually checked to avoid a false positive. */
   dp->this.ps->validated = 1;
}

static void
transform_end(png_structp pp, png_infop pi)
{
   transform_display *dp = png_get_progressive_ptr(pp);

   transform_image_validate(dp, pp, pi, dp->this.ps->image);
}

/* A single test run. */
static void
transform_test(png_modifier *pmIn, PNG_CONST png_uint_32 idIn,
    PNG_CONST image_transform* transform_listIn, PNG_CONST char *name)
{
   transform_display d;
   context(&pmIn->this, fault);

   transform_display_init(&d, pmIn, idIn, transform_listIn);

   Try
   {
      png_structp pp;
      png_infop pi;

      /* Get a png_struct for writing the image. */
      pp = set_modifier_for_read(d.pm, &pi, d.this.id, name);

#     if 0
         /* Logging (debugging only) */
         {
            char buffer[256];

            (void)store_message(&d.pm->this, pp, buffer, sizeof buffer, 0,
               "running test");

            fprintf(stderr, "%s\n", buffer);
         }
#     endif

      /* Introduce the correct read function. */
      if (d.pm->this.progressive)
      {
         /* Share the row function with the standard implementation. */
         png_set_progressive_read_fn(pp, &d, transform_info, progressive_row,
            transform_end);

         /* Now feed data into the reader until we reach the end: */
         modifier_progressive_read(d.pm, pp, pi);
      }
      else
      {
         /* modifier_read expects a png_modifier* */
         png_set_read_fn(pp, d.pm, modifier_read);

         /* Check the header values: */
         png_read_info(pp, pi);

         /* Process the 'info' requirements. Only one image is generated */
         transform_info_imp(&d, pp, pi);

         sequential_row(&d.this, pp, pi, NULL, d.this.ps->image);

         transform_image_validate(&d, pp, pi, d.this.ps->image);
      }

      modifier_reset(d.pm);
   }

   Catch(fault)
      modifier_reset((png_modifier*)fault);
}

/* The transforms: */
#define ITSTRUCT(name) image_transform_##name
#define IT(name,prev)\
static image_transform ITSTRUCT(name) =\
{\
   #name,\
   1, /*enable*/\
   &ITSTRUCT(prev), /*list*/\
   0, /*global_use*/\
   0, /*local_use*/\
   0, /*next*/\
   image_transform_png_set_##name##_set,\
   image_transform_png_set_##name##_mod,\
   image_transform_png_set_##name##_add\
}

/* To save code: */
static int
image_transform_default_add(image_transform *this,
    PNG_CONST image_transform **that, png_byte colour_type, png_byte bit_depth)
{
   UNUSED(colour_type)
   UNUSED(bit_depth)

   this->next = *that;
   *that = this;

   return 1;
}

/* png_set_palette_to_rgb */
static void
image_transform_png_set_palette_to_rgb_set(PNG_CONST image_transform *this,
    transform_display *that, png_structp pp, png_infop pi)
{
   png_set_palette_to_rgb(pp);
   this->next->set(this->next, that, pp, pi);
}

static void
image_transform_png_set_palette_to_rgb_mod(PNG_CONST image_transform *this,
    image_pixel *that, png_structp pp, PNG_CONST transform_display *display)
{
   if (that->colour_type == PNG_COLOR_TYPE_PALETTE)
      image_pixel_convert_PLTE(that, &display->this);

   this->next->mod(this->next, that, pp, display);
}

static int
image_transform_png_set_palette_to_rgb_add(image_transform *this,
    PNG_CONST image_transform **that, png_byte colour_type, png_byte bit_depth)
{
   UNUSED(bit_depth)

   this->next = *that;
   *that = this;

   return colour_type == PNG_COLOR_TYPE_PALETTE;
}

IT(palette_to_rgb, end);


/* png_set_tRNS_to_alpha */
static void
image_transform_png_set_tRNS_to_alpha_set(PNG_CONST image_transform *this,
   transform_display *that, png_structp pp, png_infop pi)
{
   png_set_tRNS_to_alpha(pp);
   this->next->set(this->next, that, pp, pi);
}

static void
image_transform_png_set_tRNS_to_alpha_mod(PNG_CONST image_transform *this,
   image_pixel *that, png_structp pp, PNG_CONST transform_display *display)
{
   /* LIBPNG BUG: this always forces palette images to RGB. */
   if (that->colour_type == PNG_COLOR_TYPE_PALETTE)
      image_pixel_convert_PLTE(that, &display->this);

   /* This effectively does an 'expand' only if there is some transparency to
    * covert to an alpha channel.
    */
   if (that->have_tRNS)
      image_pixel_add_alpha(that, &display->this);

   /* LIBPNG BUG: otherwise libpng still expands to 8 bits! */
   else
   {
      if (that->bit_depth < 8)
         that->bit_depth =8;
      if (that->sample_depth < 8)
         that->sample_depth = 8;
   }

   this->next->mod(this->next, that, pp, display);
}

static int
image_transform_png_set_tRNS_to_alpha_add(image_transform *this,
    PNG_CONST image_transform **that, png_byte colour_type, png_byte bit_depth)
{
   UNUSED(bit_depth)

   this->next = *that;
   *that = this;

   /* We don't know yet whether there will be a tRNS chunk, but we know that
    * this transformation should do nothing if there already is an alpha
    * channel.
    */
   return (colour_type & PNG_COLOR_MASK_ALPHA) == 0;
}

IT(tRNS_to_alpha,palette_to_rgb);

/* png_set_gray_to_rgb */
static void
image_transform_png_set_gray_to_rgb_set(PNG_CONST image_transform *this,
    transform_display *that, png_structp pp, png_infop pi)
{
   png_set_gray_to_rgb(pp);
   this->next->set(this->next, that, pp, pi);
}

static void
image_transform_png_set_gray_to_rgb_mod(PNG_CONST image_transform *this,
    image_pixel *that, png_structp pp, PNG_CONST transform_display *display)
{
   /* NOTE: we can actually pend the tRNS processing at this point because we
    * can correctly recognize the original pixel value even though we have
    * mapped the one gray channel to the three RGB ones, but in fact libpng
    * doesn't do this, so we don't either.
    */
   if ((that->colour_type & PNG_COLOR_MASK_COLOR) == 0 && that->have_tRNS)
      image_pixel_add_alpha(that, &display->this);

   /* Simply expand the bit depth and alter the colour type as required. */
   if (that->colour_type == PNG_COLOR_TYPE_GRAY)
   {
      /* RGB images have a bit depth at least equal to '8' */
      if (that->bit_depth < 8)
         that->sample_depth = that->bit_depth = 8;

      /* And just changing the colour type works here because the green and blue
       * channels are being maintained in lock-step with the red/gray:
       */
      that->colour_type = PNG_COLOR_TYPE_RGB;
   }

   else if (that->colour_type == PNG_COLOR_TYPE_GRAY_ALPHA)
      that->colour_type = PNG_COLOR_TYPE_RGB_ALPHA;

   this->next->mod(this->next, that, pp, display);
}

static int
image_transform_png_set_gray_to_rgb_add(image_transform *this,
    PNG_CONST image_transform **that, png_byte colour_type, png_byte bit_depth)
{
   UNUSED(bit_depth)

   this->next = *that;
   *that = this;

   return (colour_type & PNG_COLOR_MASK_COLOR) == 0;
}

IT(gray_to_rgb,tRNS_to_alpha);

/* png_set_expand */
static void
image_transform_png_set_expand_set(PNG_CONST image_transform *this,
    transform_display *that, png_structp pp, png_infop pi)
{
   png_set_expand(pp);
   this->next->set(this->next, that, pp, pi);
}

static void
image_transform_png_set_expand_mod(PNG_CONST image_transform *this,
    image_pixel *that, png_structp pp, PNG_CONST transform_display *display)
{
   /* The general expand case depends on what the colour type is: */
   if (that->colour_type == PNG_COLOR_TYPE_PALETTE)
      image_pixel_convert_PLTE(that, &display->this);
   else if (that->bit_depth < 8) /* grayscale */
      that->sample_depth = that->bit_depth = 8;

   if (that->have_tRNS)
      image_pixel_add_alpha(that, &display->this);

   this->next->mod(this->next, that, pp, display);
}

static int
image_transform_png_set_expand_add(image_transform *this,
    PNG_CONST image_transform **that, png_byte colour_type, png_byte bit_depth)
{
   UNUSED(bit_depth)

   this->next = *that;
   *that = this;

   /* 'expand' should do nothing for RGBA or GA input - no tRNS and the bit
    * depth is at least 8 already.
    */
   return (colour_type & PNG_COLOR_MASK_ALPHA) == 0;
}

IT(expand,gray_to_rgb);

/* png_set_expand_gray_1_2_4_to_8
 * LIBPNG BUG: this just does an 'expand'
 */
static void
image_transform_png_set_expand_gray_1_2_4_to_8_set(
    PNG_CONST image_transform *this, transform_display *that, png_structp pp,
    png_infop pi)
{
   png_set_expand_gray_1_2_4_to_8(pp);
   this->next->set(this->next, that, pp, pi);
}

static void
image_transform_png_set_expand_gray_1_2_4_to_8_mod(
    PNG_CONST image_transform *this, image_pixel *that, png_structp pp,
    PNG_CONST transform_display *display)
{
   image_transform_png_set_expand_mod(this, that, pp, display);
}

static int
image_transform_png_set_expand_gray_1_2_4_to_8_add(image_transform *this,
    PNG_CONST image_transform **that, png_byte colour_type, png_byte bit_depth)
{
   return image_transform_png_set_expand_add(this, that, colour_type,
      bit_depth);
}

IT(expand_gray_1_2_4_to_8, expand);
/* png_set_expand_16 */
static void
image_transform_png_set_expand_16_set(PNG_CONST image_transform *this,
    transform_display *that, png_structp pp, png_infop pi)
{
   png_set_expand_16(pp);
   this->next->set(this->next, that, pp, pi);
}

static void
image_transform_png_set_expand_16_mod(PNG_CONST image_transform *this,
    image_pixel *that, png_structp pp, PNG_CONST transform_display *display)
{
   /* Expect expand_16 to expand everything to 16 bits as a result of also
    * causing 'expand' to happen.
    */
   if (that->colour_type == PNG_COLOR_TYPE_PALETTE)
      image_pixel_convert_PLTE(that, &display->this);

   if (that->have_tRNS)
      image_pixel_add_alpha(that, &display->this);

   if (that->bit_depth < 16)
      that->sample_depth = that->bit_depth = 16;

   this->next->mod(this->next, that, pp, display);
}

static int
image_transform_png_set_expand_16_add(image_transform *this,
    PNG_CONST image_transform **that, png_byte colour_type, png_byte bit_depth)
{
   UNUSED(colour_type)

   this->next = *that;
   *that = this;

   /* expand_16 does something unless the bit depth is already 16. */
   return bit_depth < 16;
}

IT(expand_16, expand_gray_1_2_4_to_8);

/* png_set_strip_16 */
static void
image_transform_png_set_strip_16_set(PNG_CONST image_transform *this,
    transform_display *that, png_structp pp, png_infop pi)
{
   png_set_strip_16(pp);
   this->next->set(this->next, that, pp, pi);
}

static void
image_transform_png_set_strip_16_mod(PNG_CONST image_transform *this,
    image_pixel *that, png_structp pp, PNG_CONST transform_display *display)
{
   if (that->bit_depth == 16)
   {
      that->sample_depth = that->bit_depth = 8;
      if (that->red_sBIT > 8) that->red_sBIT = 8;
      if (that->green_sBIT > 8) that->green_sBIT = 8;
      if (that->blue_sBIT > 8) that->blue_sBIT = 8;
      if (that->alpha_sBIT > 8) that->alpha_sBIT = 8;

#     ifndef PNG_READ_16_TO_8_ACCURATE_SCALE_SUPPORTED
         /* The strip 16 algoirithm drops the low 8 bits rather than calculating
          * 1/257, so we need to adjust the permitted errors appropriately:
          */
         {
            PNG_CONST double d = (255-128.5)/65535;
            that->rede += d;
            that->greene += d;
            that->bluee += d;
            that->alphae += d;
         }
#     endif
   }

   this->next->mod(this->next, that, pp, display);
}

static int
image_transform_png_set_strip_16_add(image_transform *this,
    PNG_CONST image_transform **that, png_byte colour_type, png_byte bit_depth)
{
   UNUSED(colour_type)

   this->next = *that;
   *that = this;

   return bit_depth > 8;
}

IT(strip_16, expand_16);

/* png_set_strip_alpha */
static void
image_transform_png_set_strip_alpha_set(PNG_CONST image_transform *this,
    transform_display *that, png_structp pp, png_infop pi)
{
   png_set_strip_alpha(pp);
   this->next->set(this->next, that, pp, pi);
}

static void
image_transform_png_set_strip_alpha_mod(PNG_CONST image_transform *this,
    image_pixel *that, png_structp pp, PNG_CONST transform_display *display)
{
   if (that->colour_type == PNG_COLOR_TYPE_GRAY_ALPHA)
      that->colour_type = PNG_COLOR_TYPE_GRAY;
   else if (that->colour_type == PNG_COLOR_TYPE_RGB_ALPHA)
      that->colour_type = PNG_COLOR_TYPE_RGB;

   that->have_tRNS = 0;
   that->alphaf = 1;
   that->alphae = 0;

   this->next->mod(this->next, that, pp, display);
}

static int
image_transform_png_set_strip_alpha_add(image_transform *this,
    PNG_CONST image_transform **that, png_byte colour_type, png_byte bit_depth)
{
   UNUSED(bit_depth)

   this->next = *that;
   *that = this;

   return (colour_type & PNG_COLOR_MASK_ALPHA) != 0;
}

IT(strip_alpha,strip_16);

/* png_set_rgb_to_gray(png_structp, int err_action, double red, double green)
 * png_set_rgb_to_gray_fixed(png_structp, int err_action, png_fixed_point red,
 *    png_fixed_point green)
 * png_get_rgb_to_gray_status
 *
 * At present the APIs are simply tested using the 16.16 fixed point conversion
 * values known to be used inside libpng:
 *
 *   red:    6968
 *   green: 23434
 *   blue:   2366
 *
 * NOTE: this currently ignores the gamma because no gamma is being set, the
 * tests on gamma need to happen in the gamma test set.
 */
static void
image_transform_png_set_rgb_to_gray_set(PNG_CONST image_transform *this,
    transform_display *that, png_structp pp, png_infop pi)
{
   PNG_CONST int error_action = 1; /* no error, no defines in png.h */

#  ifdef PNG_FLOATING_POINT_SUPPORTED
      png_set_rgb_to_gray(pp, error_action, -1, -1);
#  else
      png_set_rgb_to_gray_fixed(pp, error_action, -1, -1);
#  endif

   this->next->set(this->next, that, pp, pi);
}

static void
image_transform_png_set_rgb_to_gray_mod(PNG_CONST image_transform *this,
    image_pixel *that, png_structp pp, PNG_CONST transform_display *display)
{
   if ((that->colour_type & PNG_COLOR_MASK_COLOR) != 0)
   {
      if (that->colour_type == PNG_COLOR_TYPE_PALETTE)
         image_pixel_convert_PLTE(that, &display->this);

      /* Image now has RGB channels... */
      that->bluef = that->greenf = that->redf = (that->redf * 6968 +
         that->greenf * 23434 + that->bluef * 2366) / 32768;
      that->bluee = that->greene = that->rede = (that->rede * 6968 +
         that->greene * 23434 + that->bluee * 2366) / 32768 *
         (1 + DBL_EPSILON * 6);

      /* The sBIT is the minium of the three colour channel sBITs. */
      if (that->red_sBIT > that->green_sBIT)
         that->red_sBIT = that->green_sBIT;
      if (that->red_sBIT > that->blue_sBIT)
         that->red_sBIT = that->blue_sBIT;
      that->blue_sBIT = that->green_sBIT = that->red_sBIT;

      /* And zap the colour bit in the type: */
      if (that->colour_type == PNG_COLOR_TYPE_RGB)
         that->colour_type = PNG_COLOR_TYPE_GRAY;
      else if (that->colour_type == PNG_COLOR_TYPE_RGB_ALPHA)
         that->colour_type = PNG_COLOR_TYPE_GRAY_ALPHA;
   }

   this->next->mod(this->next, that, pp, display);
}

static int
image_transform_png_set_rgb_to_gray_add(image_transform *this,
    PNG_CONST image_transform **that, png_byte colour_type, png_byte bit_depth)
{
   UNUSED(bit_depth)

   this->next = *that;
   *that = this;

   return (colour_type & PNG_COLOR_MASK_COLOR) != 0;
}

IT(rgb_to_gray,strip_alpha);

/* png_set_background(png_structp, png_const_color_16p background_color,
 *    int background_gamma_code, int need_expand, double background_gamma)
 * png_set_background_fixed(png_structp, png_const_color_16p background_color,
 *    int background_gamma_code, int need_expand,
 *    png_fixed_point background_gamma)
 *
 * As with rgb_to_gray this ignores the gamma.
*/
static void
image_transform_png_set_background_set(PNG_CONST image_transform *this,
    transform_display *that, png_structp pp, png_infop pi)
{
   png_color_16 back;

   /* Since we don't know the output bit depth at this point we must use the
    * input values and ask libpng to expand the chunk as required.
    */
   back.index = 255; /* Should not be used */
   back.gray = back.blue = back.green = back.red =
      (png_uint_16)((1U << that->this.bit_depth) >> 1);

#  ifdef PNG_FLOATING_POINT_SUPPORTED
      png_set_background(pp, &back, PNG_BACKGROUND_GAMMA_FILE, 1, 0);
#  else
      png_set_background_fixed(pp, &back, PNG_BACKGROUND_GAMMA_FILE, 1, 0);
#  endif

   this->next->set(this->next, that, pp, pi);
}

static void
image_transform_png_set_background_mod(PNG_CONST image_transform *this,
    image_pixel *that, png_structp pp, PNG_CONST transform_display *display)
{
   /* Check for tRNS first: */
   if (that->have_tRNS && that->colour_type != PNG_COLOR_TYPE_PALETTE)
      image_pixel_add_alpha(that, &display->this);

   /* This is only necessary if the alpha value is less than 1. */
   if (that->alphaf < 1)
   {
      /* Repeat the calculation above and scale the result: */
      unsigned int tmp = (1U << display->this.bit_depth);
      double component = (tmp >> 1)/(double)(tmp-1);

      /* Now we do the background calculation without any gamma correction. */
      if (that->alphaf <= 0)
      {
         that->bluef = that->greenf = that->redf = component;
         that->bluee = that->greene = that->rede = component * DBL_EPSILON;
         that->blue_sBIT = that->green_sBIT = that->red_sBIT = that->bit_depth;
      }

      else
      {
         component *= 1-that->alphaf;
         that->redf = that->redf * that->alphaf + component;
         that->rede = that->rede * that->alphaf + that->redf * 3 * DBL_EPSILON;
         that->greenf = that->greenf * that->alphaf + component;
         that->greene = that->greene * that->alphaf + that->greenf * 3 *
            DBL_EPSILON;
         that->bluef = that->bluef * that->alphaf + component;
         that->bluee = that->bluee * that->alphaf + that->bluef * 3 *
            DBL_EPSILON;
      }

      /* Remove the alpha type and set the alpha. */
      that->alphaf = 1;
      that->alphae = 0;

      if (that->colour_type == PNG_COLOR_TYPE_RGB_ALPHA)
         that->colour_type = PNG_COLOR_TYPE_RGB;
      else if (that->colour_type == PNG_COLOR_TYPE_GRAY_ALPHA)
         that->colour_type = PNG_COLOR_TYPE_GRAY;
   }

   this->next->mod(this->next, that, pp, display);
}

#define image_transform_png_set_background_add image_transform_default_add

IT(background,rgb_to_gray);

static image_transform *PNG_CONST image_transform_first = &ITSTRUCT(background);

static void
transform_enable(PNG_CONST char *name)
{
   /* Everything starts out enabled, so if we see an 'enable' disabled
    * everything else the first time round.
    */
   static int all_disabled = 0;
   int found_it = 0;
   image_transform *list = image_transform_first;

   while (list != &image_transform_end)
   {
      if (strcmp(list->name, name) == 0)
      {
         list->enable = 1;
         found_it = 1;
      }
      else if (!all_disabled)
         list->enable = 0;

      list = list->list;
   }

   all_disabled = 1;

   if (!found_it)
   {
      fprintf(stderr, "pngvalid: --transform-enable=%s: unknown transform\n",
         name);
      exit(1);
   }
}

static void
transform_disable(PNG_CONST char *name)
{
   image_transform *list = image_transform_first;

   while (list != &image_transform_end)
   {
      if (strcmp(list->name, name) == 0)
      {
         list->enable = 0;
         return;
      }

      list = list->list;
   }

   fprintf(stderr, "pngvalid: --transform-disable=%s: unknown transform\n",
      name);
   exit(1);
}

static void
image_transform_reset_count(void)
{
   image_transform *next = image_transform_first;
   int count = 0;

   while (next != &image_transform_end)
   {
      next->local_use = 0;
      next->next = 0;
      next = next->list;
      ++count;
   }

   /* This can only happen if we every have more than 32 transforms (excluding
    * the end) in the list.
    */
   if (count > 32) abort();
}

static int
image_transform_test_counter(png_uint_32 counter, unsigned int max)
{
   /* Test the list to see if there is any point contining, given a current
    * counter and a 'max' value.
    */
   image_transform *next = image_transform_first;

   while (next != &image_transform_end)
   {
      /* For max 0 or 1 continue until the counter overflows: */
      counter >>= 1;

      /* Continue if any entry hasn't reacked the max. */
      if (max > 1 && next->local_use < max)
         return 1;
      next = next->list;
   }

   return max <= 1 && counter == 0;
}

static png_uint_32
image_transform_add(PNG_CONST image_transform **this, unsigned int max,
   png_uint_32 counter, char *name, size_t sizeof_name, size_t *pos,
   png_byte colour_type, png_byte bit_depth)
{
   for (;;) /* until we manage to add something */
   {
      png_uint_32 mask;
      image_transform *list;

      /* Find the next counter value, if the counter is zero this is the start
       * of the list.  This routine always returns the current counter (not the
       * next) so it returns 0 at the end and expects 0 at the beginning.
       */
      if (counter == 0) /* first time */
      {
         image_transform_reset_count();
         if (max <= 1)
            counter = 1;
         else
            counter = random_32();
      }
      else /* advance the counter */
      {
         switch (max)
         {
            case 0:  ++counter; break;
            case 1:  counter <<= 1; break;
            default: counter = random_32(); break;
         }
      }

      /* Now add all these items, if possible */
      *this = &image_transform_end;
      list = image_transform_first;
      mask = 1;

      /* Go through the whole list adding anything that the counter selects: */
      while (list != &image_transform_end)
      {
         if ((counter & mask) != 0 && list->enable &&
             (max == 0 || list->local_use < max))
         {
            /* Candidate to add: */
            if (list->add(list, this, colour_type, bit_depth) || max == 0)
            {
               /* Added, so add to the name too. */
               *pos = safecat(name, sizeof_name, *pos, " +");
               *pos = safecat(name, sizeof_name, *pos, list->name);
            }

            else
            {
               /* Not useful and max>0, so remvoe it from *this: */
               *this = list->next;
               list->next = 0;

               /* And, since we know it isn't useful, stop it being added again
                * in this run:
                */
               list->local_use = max;
            }
         }

         mask <<= 1;
         list = list->list;
      }

      /* Now if anything was added we have something to do. */
      if (*this != &image_transform_end)
         return counter;

      /* Nothing added, but was there anything in there to add? */
      if (!image_transform_test_counter(counter, max))
         return 0;
   }
}

#ifdef THIS_IS_THE_PROFORMA
static void
image_transform_png_set_@_set(PNG_CONST image_transform *this,
    transform_display *that, png_structp pp, png_infop pi)
{
   png_set_@(pp);
   this->next->set(this->next, that, pp, pi);
}

static void
image_transform_png_set_@_mod(PNG_CONST image_transform *this,
    image_pixel *that, png_structp pp, PNG_CONST transform_display *display)
{
   this->next->mod(this->next, that, pp, display);
}

static int
image_transform_png_set_@_add(image_transform *this,
    PNG_CONST image_transform **that, char *name, size_t sizeof_name,
    size_t *pos, png_byte colour_type, png_byte bit_depth)
{
   this->next = *that;
   *that = this;

   *pos = safecat(name, sizeof_name, *pos, " +@");

   return 1;
}

IT(@);
#endif

/* png_set_quantize(png_structp, png_colorp palette, int num_palette,
 *    int maximum_colors, png_const_uint_16p histogram, int full_quantize)
 *
 * Very difficult to validate this!
 */
/*NOTE: TBD NYI */

/* The data layout transforms are handled by swapping our own channel data,
 * necessarily these need to happen at the end of the transform list because the
 * semantic of the channels changes after these are executed.  Some of these,
 * like set_shift and set_packing, can't be done at present because they change
 * the layout of the data at the sub-sample level so sample() won't get the
 * right answer.
 */
/* png_set_invert_alpha */
/*NOTE: TBD NYI */

/* png_set_bgr */
/*NOTE: TBD NYI */

/* png_set_swap_alpha */
/*NOTE: TBD NYI */

/* png_set_swap */
/*NOTE: TBD NYI */

/* png_set_filler, (png_structp png_ptr, png_uint_32 filler, int flags)); */
/*NOTE: TBD NYI */

/* png_set_add_alpha, (png_structp png_ptr, png_uint_32 filler, int flags)); */
/*NOTE: TBD NYI */

/* png_set_packing */
/*NOTE: TBD NYI */

/* png_set_packswap */
/*NOTE: TBD NYI */

/* png_set_invert_mono */
/*NOTE: TBD NYI */

/* png_set_shift(png_structp, png_const_color_8p true_bits) */
/*NOTE: TBD NYI */

static int
test_transform(png_modifier* PNG_CONST pm, png_byte PNG_CONST colour_type,
    int bdlo, int PNG_CONST bdhi, png_uint_32 max)
{
   for (; bdlo <= bdhi; ++bdlo)
   {
      PNG_CONST png_byte bit_depth = DEPTH(bdlo);
      png_uint_32 counter = 0;
      size_t base_pos;
      char name[64];

      base_pos = safecat(name, sizeof name, 0, "transform:");

      for (;;)
      {
         size_t pos = base_pos;
         PNG_CONST image_transform *list = 0;

         counter = image_transform_add(&list, max, counter, name, sizeof name,
            &pos, colour_type, bit_depth);

         if (counter == 0)
            break;

         /* The command line can change this to checking interlaced images. */
         transform_test(pm, FILEID(colour_type, bit_depth, pm->interlace_type,
            0, 0, 0), list, name);

         if (fail(pm))
            return 0;
      }
   }

   return 1; /* keep going */
}

static void
perform_transform_test(png_modifier *pm)
{
   /* Test each colour type over the valid range of bit depths (expressed as
    * log2(bit_depth) in turn, stop as soon as any error is detected.
    */
   if (!test_transform(pm, 0, 0, READ_BDHI, 1))
      return;

   if (!test_transform(pm, 2, 3, READ_BDHI, 1))
      return;

   if (!test_transform(pm, 3, 0, 3, 1))
      return;

   if (!test_transform(pm, 4, 3, READ_BDHI, 1))
      return;

   if (!test_transform(pm, 6, 3, READ_BDHI, 1))
      return;
}


/********************************* GAMMA TESTS ********************************/
/* Gamma test images. */
typedef struct gamma_modification
{
   png_modification this;
   png_fixed_point  gamma;
} gamma_modification;

static int
gamma_modify(png_modifier *pm, png_modification *me, int add)
{
   UNUSED(add)
   /* This simply dumps the given gamma value into the buffer. */
   png_save_uint_32(pm->buffer, 4);
   png_save_uint_32(pm->buffer+4, CHUNK_gAMA);
   png_save_uint_32(pm->buffer+8, ((gamma_modification*)me)->gamma);
   return 1;
}

static void
gamma_modification_init(gamma_modification *me, png_modifier *pm, double gammad)
{
   double g;

   modification_init(&me->this);
   me->this.chunk = CHUNK_gAMA;
   me->this.modify_fn = gamma_modify;
   me->this.add = CHUNK_PLTE;
   g = floor(gammad * 100000 + .5);
   me->gamma = (png_fixed_point)g;
   me->this.next = pm->modifications;
   pm->modifications = &me->this;
}

typedef struct srgb_modification
{
   png_modification this;
   png_byte         intent;
} srgb_modification;

static int
srgb_modify(png_modifier *pm, png_modification *me, int add)
{
   UNUSED(add)
   /* As above, ignore add and just make a new chunk */
   png_save_uint_32(pm->buffer, 1);
   png_save_uint_32(pm->buffer+4, CHUNK_sRGB);
   pm->buffer[8] = ((srgb_modification*)me)->intent;
   return 1;
}

static void
srgb_modification_init(srgb_modification *me, png_modifier *pm, png_byte intent)
{
   modification_init(&me->this);
   me->this.chunk = CHUNK_sBIT;

   if (intent <= 3) /* if valid, else *delete* sRGB chunks */
   {
      me->this.modify_fn = srgb_modify;
      me->this.add = CHUNK_PLTE;
      me->intent = intent;
   }

   else
   {
      me->this.modify_fn = 0;
      me->this.add = 0;
      me->intent = 0;
   }

   me->this.next = pm->modifications;
   pm->modifications = &me->this;
}

typedef struct sbit_modification
{
   png_modification this;
   png_byte         sbit;
} sbit_modification;

static int
sbit_modify(png_modifier *pm, png_modification *me, int add)
{
   png_byte sbit = ((sbit_modification*)me)->sbit;
   if (pm->bit_depth > sbit)
   {
      int cb = 0;
      switch (pm->colour_type)
      {
         case 0:
            cb = 1;
            break;

         case 2:
         case 3:
            cb = 3;
            break;

         case 4:
            cb = 2;
            break;

         case 6:
            cb = 4;
            break;

         default:
            png_error(pm->this.pread,
               "unexpected colour type in sBIT modification");
      }

      png_save_uint_32(pm->buffer, cb);
      png_save_uint_32(pm->buffer+4, CHUNK_sBIT);

      while (cb > 0)
         (pm->buffer+8)[--cb] = sbit;

      return 1;
   }
   else if (!add)
   {
      /* Remove the sBIT chunk */
      pm->buffer_count = pm->buffer_position = 0;
      return 1;
   }
   else
      return 0; /* do nothing */
}

static void
sbit_modification_init(sbit_modification *me, png_modifier *pm, png_byte sbit)
{
   modification_init(&me->this);
   me->this.chunk = CHUNK_sBIT;
   me->this.modify_fn = sbit_modify;
   me->this.add = CHUNK_PLTE;
   me->sbit = sbit;
   me->this.next = pm->modifications;
   pm->modifications = &me->this;
}

/* Reader callbacks and implementations, where they differ from the standard
 * ones.
 */
typedef struct gamma_display
{
   standard_display this;

   /* Parameters */
   png_modifier*    pm;
   double           file_gamma;
   double           screen_gamma;
   png_byte         sbit;
   int              threshold_test;
   PNG_CONST char*  name;
   int              speed;
   int              use_input_precision;
   int              strip16;

   /* Local variables */
   double       maxerrout;
   double       maxerrpc;
   double       maxerrabs;
} gamma_display;

static void
gamma_display_init(gamma_display *dp, png_modifier *pm, png_uint_32 id,
    double file_gamma, double screen_gamma, png_byte sbit, int threshold_test,
    int speed, int use_input_precision, int strip16)
{
   /* Standard fields */
   standard_display_init(&dp->this, &pm->this, id, 0/*do_interlace*/);

   /* Parameter fields */
   dp->pm = pm;
   dp->file_gamma = file_gamma;
   dp->screen_gamma = screen_gamma;
   dp->sbit = sbit;
   dp->threshold_test = threshold_test;
   dp->speed = speed;
   dp->use_input_precision = use_input_precision;
   dp->strip16 = strip16;

   /* Local variable fields */
   dp->maxerrout = dp->maxerrpc = dp->maxerrabs = 0;
}

static void
gamma_info_imp(gamma_display *dp, png_structp pp, png_infop pi)
{
   /* Reuse the standard stuff as appropriate. */
   standard_info_part1(&dp->this, pp, pi);

   /* If requested strip 16 to 8 bits - this is handled automagically below
    * because the output bit depth is read from the library.  Note that there
    * are interactions with sBIT but, internally, libpng makes sbit at most
    * PNG_MAX_GAMMA_8 when doing the following.
    */
   if (dp->strip16)
#     ifdef PNG_READ_16_TO_8_SUPPORTED
         png_set_strip_16(pp);
#     else
         png_error(pp, "strip16 (16 to 8 bit conversion) not supported");
#     endif

   png_read_update_info(pp, pi);

   /* Now we may get a different cbRow: */
   standard_info_part2(&dp->this, pp, pi, 1 /*images*/);
}

static void
gamma_info(png_structp pp, png_infop pi)
{
   gamma_info_imp(png_get_progressive_ptr(pp), pp, pi);
}

static void
gamma_image_validate(gamma_display *dp, png_structp pp, png_infop pi,
    png_const_bytep pRow)
{
   /* Get some constants derived from the input and output file formats: */
   PNG_CONST png_byte sbit = dp->sbit;
   PNG_CONST double file_gamma = dp->file_gamma;
   PNG_CONST double screen_gamma = dp->screen_gamma;
   PNG_CONST int use_input_precision = dp->use_input_precision;
   PNG_CONST int speed = dp->speed;
   PNG_CONST png_byte in_ct = dp->this.colour_type;
   PNG_CONST png_byte in_bd = dp->this.bit_depth;
   PNG_CONST png_uint_32 w = dp->this.w;
   PNG_CONST png_uint_32 h = dp->this.h;
   PNG_CONST size_t cbRow = dp->this.cbRow;
   PNG_CONST png_byte out_ct = png_get_color_type(pp, pi);
   PNG_CONST png_byte out_bd = png_get_bit_depth(pp, pi);
   PNG_CONST unsigned int outmax = (1U<<out_bd)-1;
   PNG_CONST double maxabs = abserr(dp->pm, out_bd);
   PNG_CONST double maxout = outerr(dp->pm, out_bd);
   PNG_CONST double maxpc = pcerr(dp->pm, out_bd);

   /* There are three sources of error, firstly the quantization in the
    * file encoding, determined by sbit and/or the file depth, secondly
    * the output (screen) gamma and thirdly the output file encoding.
    *
    * Since this API receives the screen and file gamma in double
    * precision it is possible to calculate an exact answer given an input
    * pixel value.  Therefore we assume that the *input* value is exact -
    * sample/maxsample - calculate the corresponding gamma corrected
    * output to the limits of double precision arithmetic and compare with
    * what libpng returns.
    *
    * Since the library must quantize the output to 8 or 16 bits there is
    * a fundamental limit on the accuracy of the output of +/-.5 - this
    * quantization limit is included in addition to the other limits
    * specified by the paramaters to the API.  (Effectively, add .5
    * everywhere.)
    *
    * The behavior of the 'sbit' paramter is defined by section 12.5
    * (sample depth scaling) of the PNG spec.  That section forces the
    * decoder to assume that the PNG values have been scaled if sBIT is
    * present:
    *
    *     png-sample = floor( input-sample * (max-out/max-in) + .5);
    *
    * This means that only a subset of the possible PNG values should
    * appear in the input. However, the spec allows the encoder to use a
    * variety of approximations to the above and doesn't require any
    * restriction of the values produced.
    *
    * Nevertheless the spec requires that the upper 'sBIT' bits of the
    * value stored in a PNG file be the original sample bits.
    * Consequently the code below simply scales the top sbit bits by
    * (1<<sbit)-1 to obtain an original sample value.
    *
    * Because there is limited precision in the input it is arguable that
    * an acceptable result is any valid result from input-.5 to input+.5.
    * The basic tests below do not do this, however if
    * 'use_input_precision' is set a subsequent test is performed below.
    */
   PNG_CONST int processing = (fabs(screen_gamma*file_gamma-1) >=
      PNG_GAMMA_THRESHOLD && !dp->threshold_test && !speed && in_ct != 3) ||
      in_bd != out_bd;

   PNG_CONST unsigned int samples_per_pixel = (out_ct & 2U) ? 3U : 1U;

   PNG_CONST double gamma_correction = 1/(file_gamma*screen_gamma);/* Overall */

   double maxerrout = 0, maxerrabs = 0, maxerrpc = 0;
   png_uint_32 y;

   for (y=0; y<h; ++y, pRow += cbRow)
   {
      unsigned int s, x;
      png_byte std[STANDARD_ROWMAX];

      transform_row(pp, std, in_ct, in_bd, y);

      if (processing)
      {
         for (x=0; x<w; ++x) for (s=0; s<samples_per_pixel; ++s)
         {
            /* Input sample values: */
            PNG_CONST unsigned int
               id = sample(std, in_ct, in_bd, x, s);

            PNG_CONST unsigned int
               od = sample(pRow, out_ct, out_bd, x, s);

            PNG_CONST unsigned int
               isbit = id >> (in_bd-sbit);

            double i, input_sample, encoded_sample, output;
            double encoded_error, error;
            double es_lo, es_hi;

            /* First check on the 'perfect' result obtained from the
             * digitized input value, id, and compare this against the
             * actual digitized result, 'od'.  'i' is the input result
             * in the range 0..1:
             *
             * NOTE: sBIT should be taken into account here but isn't,
             * as described above.
             */
            i = isbit; i /= (1U<<sbit)-1;

            /* Then get the gamma corrected version of 'i' and compare
             * to 'od', any error less than .5 is insignificant - just
             * quantization of the output value to the nearest digital
             * value (nevertheless the error is still recorded - it's
             * interesting ;-)
             */
            encoded_sample = pow(i, gamma_correction) * outmax;
            encoded_error = fabs(od-encoded_sample);

            if (encoded_error > maxerrout)
               maxerrout = encoded_error;

            if (encoded_error < .5+maxout)
               continue;

            /* There may be an error, so calculate the actual sample
             * values - unencoded light intensity values.  Note that
             * in practice these are not unencoded because they
             * include a 'viewing correction' to decrease or
             * (normally) increase the perceptual contrast of the
             * image.  There's nothing we can do about this - we don't
             * know what it is - so assume the unencoded value is
             * perceptually linear.
             */
            input_sample = pow(i, 1/file_gamma); /* In range 0..1 */
            output = od;
            output /= outmax;
            output = pow(output, screen_gamma);

            /* Now we have the numbers for real errors, both absolute
             * values as as a percentage of the correct value (output):
             */
            error = fabs(input_sample-output);

            if (error > maxerrabs)
               maxerrabs = error;

            /* The following is an attempt to ignore the tendency of
             * quantization to dominate the percentage errors for low
             * output sample values:
             */
            if (input_sample*maxpc > .5+maxabs)
            {
               double percentage_error = error/input_sample;
               if (percentage_error > maxerrpc) maxerrpc = percentage_error;
            }

            /* Now calculate the digitization limits for
             * 'encoded_sample' using the 'max' values.  Note that
             * maxout is in the encoded space but maxpc and maxabs are
             * in linear light space.
             *
             * First find the maximum error in linear light space,
             * range 0..1:
             */
            {
               double tmp = input_sample * maxpc;
               if (tmp < maxabs) tmp = maxabs;

               /* Low bound - the minimum of the three: */
               es_lo = encoded_sample - maxout;

               if (es_lo > 0 && input_sample-tmp > 0)
               {
                  double low_value = outmax * pow(input_sample-tmp,
                     1/screen_gamma);
                  if (low_value < es_lo) es_lo = low_value;
               }

               else
                  es_lo = 0;

               es_hi = encoded_sample + maxout;

               if (es_hi < outmax && input_sample+tmp < 1)
               {
                  double high_value = outmax * pow(input_sample+tmp,
                     1/screen_gamma);
                  if (high_value > es_hi) es_hi = high_value;
               }

               else
                  es_hi = outmax;
            }

            /* The primary test is that the final encoded value
             * returned by the library should be between the two limits
             * (inclusive) that were calculated above.  At this point
             * quantization of the output must be taken into account.
             */
            if (od+.5 < es_lo || od-.5 > es_hi)
            {
               /* There has been an error in processing. */
               double is_lo, is_hi;

               if (use_input_precision)
               {
                  /* Ok, something is wrong - this actually happens in
                   * current libpng sbit processing.  Assume that the
                   * input value (id, adjusted for sbit) can be
                   * anywhere between value-.5 and value+.5 - quite a
                   * large range if sbit is low.
                   */
                  double tmp = (isbit - .5)/((1U<<sbit)-1);

                  if (tmp > 0)
                  {
                     is_lo = outmax * pow(tmp, gamma_correction) - maxout;
                     if (is_lo < 0) is_lo = 0;
                  }

                  else
                     is_lo = 0;

                  tmp = (isbit + .5)/((1U<<sbit)-1);

                  if (tmp < 1)
                  {
                     is_hi = outmax * pow(tmp, gamma_correction) + maxout;
                     if (is_hi > outmax) is_hi = outmax;
                  }

                  else
                     is_hi = outmax;

                  if (!(od+.5 < is_lo || od-.5 > is_hi))
                     continue;
               }
               else
                  is_lo = es_lo, is_hi = es_hi;

               {
                  char msg[256];

                  sprintf(msg,
                   "error: %.3f; %u{%u;%u} -> %u not %.2f (%.1f-%.1f)",
                     od-encoded_sample, id, sbit, isbit, od,
                     encoded_sample, is_lo, is_hi);

                  png_warning(pp, msg);
               }
            }
         }
      }

      else if (!speed && memcmp(std, pRow, cbRow) != 0)
      {
         char msg[64];

         /* No transform is expected on the threshold tests. */
         sprintf(msg, "gamma: below threshold row %d changed", y);

         png_error(pp, msg);
      }
   } /* row (y) loop */

   dp->maxerrout = maxerrout;
   dp->maxerrabs = maxerrabs;
   dp->maxerrpc = maxerrpc;
   dp->this.ps->validated = 1;
}

static void
gamma_end(png_structp pp, png_infop pi)
{
   gamma_display *dp = png_get_progressive_ptr(pp);

   gamma_image_validate(dp, pp, pi, dp->this.ps->image);
}

/* A single test run checking a gamma transformation.
 *
 * maxabs: maximum absolute error as a fraction
 * maxout: maximum output error in the output units
 * maxpc:  maximum percentage error (as a percentage)
 */
static void
gamma_test(png_modifier *pmIn, PNG_CONST png_byte colour_typeIn,
    PNG_CONST png_byte bit_depthIn, PNG_CONST int interlace_typeIn,
    PNG_CONST double file_gammaIn, PNG_CONST double screen_gammaIn,
    PNG_CONST png_byte sbitIn, PNG_CONST int threshold_testIn,
    PNG_CONST char *name, PNG_CONST int speedIn,
    PNG_CONST int use_input_precisionIn, PNG_CONST int strip16In)
{
   gamma_display d;
   context(&pmIn->this, fault);

   gamma_display_init(&d, pmIn, FILEID(colour_typeIn, bit_depthIn,
      interlace_typeIn, 0, 0, 0), file_gammaIn, screen_gammaIn, sbitIn,
      threshold_testIn, speedIn, use_input_precisionIn, strip16In);

   Try
   {
      png_structp pp;
      png_infop pi;
      gamma_modification gamma_mod;
      srgb_modification srgb_mod;
      sbit_modification sbit_mod;

      /* Make an appropriate modifier to set the PNG file gamma to the
       * given gamma value and the sBIT chunk to the given precision.
       */
      d.pm->modifications = NULL;
      gamma_modification_init(&gamma_mod, d.pm, d.file_gamma);
      srgb_modification_init(&srgb_mod, d.pm, 127 /*delete*/);
      sbit_modification_init(&sbit_mod, d.pm, d.sbit);

      modification_reset(d.pm->modifications);

      /* Get a png_struct for writing the image. */
      pp = set_modifier_for_read(d.pm, &pi, d.this.id, name);

      /* Set up gamma processing. */
#ifdef PNG_FLOATING_POINT_SUPPORTED
      png_set_gamma(pp, d.screen_gamma, d.file_gamma);
#else
      {
         png_fixed_point s = floor(d.screen_gamma*100000+.5);
         png_fixed_point f = floor(d.file_gamma*100000+.5);
         png_set_gamma_fixed(pp, s, f);
      }
#endif

      /* Introduce the correct read function. */
      if (d.pm->this.progressive)
      {
         /* Share the row function with the standard implementation. */
         png_set_progressive_read_fn(pp, &d, gamma_info, progressive_row,
            gamma_end);

         /* Now feed data into the reader until we reach the end: */
         modifier_progressive_read(d.pm, pp, pi);
      }
      else
      {
         /* modifier_read expects a png_modifier* */
         png_set_read_fn(pp, d.pm, modifier_read);

         /* Check the header values: */
         png_read_info(pp, pi);

         /* Process the 'info' requirements. Only one image is generated */
         gamma_info_imp(&d, pp, pi);

         sequential_row(&d.this, pp, pi, NULL, d.this.ps->image);

         gamma_image_validate(&d, pp, pi, d.this.ps->image);
      }

      modifier_reset(d.pm);

      if (d.pm->log && !d.threshold_test && !d.speed)
         fprintf(stderr, "%d bit %s %s: max error %f (%.2g, %2g%%)\n",
            d.this.bit_depth, colour_types[d.this.colour_type], d.name,
            d.maxerrout, d.maxerrabs, 100*d.maxerrpc);

      /* Log the summary values too. */
      if (d.this.colour_type == 0 || d.this.colour_type == 4)
      {
         switch (d.this.bit_depth)
         {
         case 1:
            break;

         case 2:
            if (d.maxerrout > d.pm->error_gray_2)
               d.pm->error_gray_2 = d.maxerrout;

            break;

         case 4:
            if (d.maxerrout > d.pm->error_gray_4)
               d.pm->error_gray_4 = d.maxerrout;

            break;

         case 8:
            if (d.maxerrout > d.pm->error_gray_8)
               d.pm->error_gray_8 = d.maxerrout;

            break;

         case 16:
            if (d.maxerrout > d.pm->error_gray_16)
               d.pm->error_gray_16 = d.maxerrout;

            break;

         default:
            png_error(pp, "bad bit depth (internal: 1)");
         }
      }

      else if (d.this.colour_type == 2 || d.this.colour_type == 6)
      {
         switch (d.this.bit_depth)
         {
         case 8:

            if (d.maxerrout > d.pm->error_color_8)
               d.pm->error_color_8 = d.maxerrout;

            break;

         case 16:

            if (d.maxerrout > d.pm->error_color_16)
               d.pm->error_color_16 = d.maxerrout;

            break;

         default:
            png_error(pp, "bad bit depth (internal: 2)");
         }
      }
   }

   Catch(fault)
      modifier_reset((png_modifier*)fault);
}

static void gamma_threshold_test(png_modifier *pm, png_byte colour_type,
    png_byte bit_depth, int interlace_type, double file_gamma,
    double screen_gamma)
{
   size_t pos = 0;
   char name[64];
   pos = safecat(name, sizeof name, pos, "threshold ");
   pos = safecatd(name, sizeof name, pos, file_gamma, 3);
   pos = safecat(name, sizeof name, pos, "/");
   pos = safecatd(name, sizeof name, pos, screen_gamma, 3);

   (void)gamma_test(pm, colour_type, bit_depth, interlace_type, file_gamma,
      screen_gamma, bit_depth, 1, name, 0 /*speed*/, 0 /*no input precision*/,
      0 /*no strip16*/);
}

static void
perform_gamma_threshold_tests(png_modifier *pm)
{
   png_byte colour_type = 0;
   png_byte bit_depth = 0;

   while (next_format(&colour_type, &bit_depth))
   {
      double test_gamma = 1.0;
      while (test_gamma >= .4)
      {
         /* There's little point testing the interlacing vs non-interlacing,
          * but this can be set from the command line.
          */
         gamma_threshold_test(pm, colour_type, bit_depth, pm->interlace_type,
            test_gamma, 1/test_gamma);
         test_gamma *= .95;
      }

      /* And a special test for sRGB */
      gamma_threshold_test(pm, colour_type, bit_depth, pm->interlace_type,
          .45455, 2.2);

      if (fail(pm))
         return;
   }
}

static void gamma_transform_test(png_modifier *pm,
   PNG_CONST png_byte colour_type, PNG_CONST png_byte bit_depth,
   PNG_CONST int interlace_type, PNG_CONST double file_gamma,
   PNG_CONST double screen_gamma, PNG_CONST png_byte sbit, PNG_CONST int speed,
   PNG_CONST int use_input_precision, PNG_CONST int strip16)
{
   size_t pos = 0;
   char name[64];

   if (sbit != bit_depth)
   {
      pos = safecat(name, sizeof name, pos, "sbit(");
      pos = safecatn(name, sizeof name, pos, sbit);
      pos = safecat(name, sizeof name, pos, ") ");
   }

   else
      pos = safecat(name, sizeof name, pos, "gamma ");

   if (strip16)
      pos = safecat(name, sizeof name, pos, "16to8 ");

   pos = safecatd(name, sizeof name, pos, file_gamma, 3);
   pos = safecat(name, sizeof name, pos, "->");
   pos = safecatd(name, sizeof name, pos, screen_gamma, 3);

   gamma_test(pm, colour_type, bit_depth, interlace_type, file_gamma,
      screen_gamma, sbit, 0, name, speed, use_input_precision, strip16);
}

static void perform_gamma_transform_tests(png_modifier *pm, int speed)
{
   png_byte colour_type = 0;
   png_byte bit_depth = 0;

   /* Ignore palette images - the gamma correction happens on the palette entry,
    * haven't got the tests for this yet.
    */
   while (next_format(&colour_type, &bit_depth)) if (colour_type != 3)
   {
      unsigned int i, j;

      for (i=0; i<pm->ngammas; ++i) for (j=0; j<pm->ngammas; ++j) if (i != j)
      {
         gamma_transform_test(pm, colour_type, bit_depth, pm->interlace_type,
            1/pm->gammas[i], pm->gammas[j], bit_depth, speed,
            pm->use_input_precision, 0 /*do not strip16*/);

         if (fail(pm))
            return;
      }
   }
}

static void perform_gamma_sbit_tests(png_modifier *pm, int speed)
{
   png_byte sbit;

   /* The only interesting cases are colour and grayscale, alpha is ignored here
    * for overall speed.  Only bit depths 8 and 16 are tested.
    */
   for (sbit=pm->sbitlow; sbit<(1<<READ_BDHI); ++sbit)
   {
      unsigned int i, j;

      for (i=0; i<pm->ngammas; ++i)
      {
         for (j=0; j<pm->ngammas; ++j)
         {
            if (i != j)
            {
               if (sbit < 8)
               {
                  gamma_transform_test(pm, 0, 8, pm->interlace_type,
                      1/pm->gammas[i], pm->gammas[j], sbit, speed,
                      pm->use_input_precision_sbit, 0 /*strip16*/);

                  if (fail(pm))
                     return;

                  gamma_transform_test(pm, 2, 8, pm->interlace_type,
                      1/pm->gammas[i], pm->gammas[j], sbit, speed,
                      pm->use_input_precision_sbit, 0 /*strip16*/);

                  if (fail(pm))
                     return;
               }

#ifdef DO_16BIT
               gamma_transform_test(pm, 0, 16, pm->interlace_type,
                   1/pm->gammas[i], pm->gammas[j], sbit, speed,
                   pm->use_input_precision_sbit, 0 /*strip16*/);

               if (fail(pm))
                  return;

               gamma_transform_test(pm, 2, 16, pm->interlace_type,
                   1/pm->gammas[i], pm->gammas[j], sbit, speed,
                   pm->use_input_precision_sbit, 0 /*strip16*/);

               if (fail(pm))
                  return;
#endif
            }
         }
      }
   }
}

/* Note that this requires a 16 bit source image but produces 8 bit output, so
 * we only need the 16bit write support.
 */
#ifdef PNG_READ_16_TO_8_SUPPORTED
static void perform_gamma_strip16_tests(png_modifier *pm, int speed)
{
#  ifndef PNG_MAX_GAMMA_8
#     define PNG_MAX_GAMMA_8 11
#  endif
   /* Include the alpha cases here. Note that sbit matches the internal value
    * used by the library - otherwise we will get spurious errors from the
    * internal sbit style approximation.
    *
    * The threshold test is here because otherwise the 16 to 8 conversion will
    * proceed *without* gamma correction, and the tests above will fail (but not
    * by much) - this could be fixed, it only appears with the -g option.
    */
   unsigned int i, j;
   for (i=0; i<pm->ngammas; ++i)
   {
      for (j=0; j<pm->ngammas; ++j)
      {
         if (i != j &&
             fabs(pm->gammas[j]/pm->gammas[i]-1) >= PNG_GAMMA_THRESHOLD)
         {
            gamma_transform_test(pm, 0, 16, pm->interlace_type, 1/pm->gammas[i],
                pm->gammas[j], PNG_MAX_GAMMA_8, speed,
                pm->use_input_precision_16to8, 1 /*strip16*/);

            if (fail(pm))
               return;

            gamma_transform_test(pm, 2, 16, pm->interlace_type, 1/pm->gammas[i],
                pm->gammas[j], PNG_MAX_GAMMA_8, speed,
                pm->use_input_precision_16to8, 1 /*strip16*/);

            if (fail(pm))
               return;

            gamma_transform_test(pm, 4, 16, pm->interlace_type, 1/pm->gammas[i],
                pm->gammas[j], PNG_MAX_GAMMA_8, speed,
                pm->use_input_precision_16to8, 1 /*strip16*/);

            if (fail(pm))
               return;

            gamma_transform_test(pm, 6, 16, pm->interlace_type, 1/pm->gammas[i],
                pm->gammas[j], PNG_MAX_GAMMA_8, speed,
                pm->use_input_precision_16to8, 1 /*strip16*/);

            if (fail(pm))
               return;
         }
      }
   }
}
#endif /* 16 to 8 bit conversion */

static void
perform_gamma_test(png_modifier *pm, int speed, int summary)
{
   /* First some arbitrary no-transform tests: */
   if (!speed && pm->test_gamma_threshold)
   {
      perform_gamma_threshold_tests(pm);

      if (fail(pm))
         return;
   }

   /* Now some real transforms. */
   if (pm->test_gamma_transform)
   {
      perform_gamma_transform_tests(pm, speed);

      if (summary)
      {
         printf("Gamma correction error summary\n\n");
         printf("The printed value is the maximum error in the pixel values\n");
         printf("calculated by the libpng gamma correction code.  The error\n");
         printf("is calculated as the difference between the output pixel\n");
         printf("value (always an integer) and the ideal value from the\n");
         printf("libpng specification (typically not an integer).\n\n");

         printf("Expect this value to be less than .5 for 8 bit formats,\n");
         printf("less than 1 for formats with fewer than 8 bits and a small\n");
         printf("number (typically less than 5) for the 16 bit formats.\n");
         printf("For performance reasons the value for 16 bit formats\n");
         printf("increases when the image file includes an sBIT chunk.\n\n");

         printf("  2 bit gray:  %.5f\n", pm->error_gray_2);
         printf("  4 bit gray:  %.5f\n", pm->error_gray_4);
         printf("  8 bit gray:  %.5f\n", pm->error_gray_8);
         printf("  8 bit color: %.5f\n", pm->error_color_8);
#ifdef DO_16BIT
         printf(" 16 bit gray:  %.5f\n", pm->error_gray_16);
         printf(" 16 bit color: %.5f\n", pm->error_color_16);
#endif
      }
   }

   /* The sbit tests produce much larger errors: */
   if (pm->test_gamma_sbit)
   {
      pm->error_gray_2 = pm->error_gray_4 = pm->error_gray_8 =
      pm->error_gray_16 = pm->error_color_8 = pm->error_color_16 = 0;
      perform_gamma_sbit_tests(pm, speed);

      if (summary)
      {
         printf("Gamma correction with sBIT:\n");

         if (pm->sbitlow < 8U)
         {
            printf("  2 bit gray:  %.5f\n", pm->error_gray_2);
            printf("  4 bit gray:  %.5f\n", pm->error_gray_4);
            printf("  8 bit gray:  %.5f\n", pm->error_gray_8);
            printf("  8 bit color: %.5f\n", pm->error_color_8);
         }

   #ifdef DO_16BIT
         printf(" 16 bit gray:  %.5f\n", pm->error_gray_16);
         printf(" 16 bit color: %.5f\n", pm->error_color_16);
   #endif
      }
   }

#ifdef PNG_READ_16_TO_8_SUPPORTED
   if (pm->test_gamma_strip16)
   {
      /* The 16 to 8 bit strip operations: */
      pm->error_gray_2 = pm->error_gray_4 = pm->error_gray_8 =
      pm->error_gray_16 = pm->error_color_8 = pm->error_color_16 = 0;
      perform_gamma_strip16_tests(pm, speed);

      if (summary)
      {
         printf("Gamma correction with 16 to 8 bit reduction:\n");
         printf(" 16 bit gray:  %.5f\n", pm->error_gray_16);
         printf(" 16 bit color: %.5f\n", pm->error_color_16);
      }
   }
#endif
}

/* INTERLACE MACRO VALIDATION */
/* This is copied verbatim from the specification, it is simply the pass
 * number in which each pixel in each 8x8 tile appears.  The array must
 * be indexed adam7[y][x] and notice that the pass numbers are based at
 * 1, not 0 - the base libpng uses.
 */
static PNG_CONST
png_byte adam7[8][8] =
{
   { 1,6,4,6,2,6,4,6 },
   { 7,7,7,7,7,7,7,7 },
   { 5,6,5,6,5,6,5,6 },
   { 7,7,7,7,7,7,7,7 },
   { 3,6,4,6,3,6,4,6 },
   { 7,7,7,7,7,7,7,7 },
   { 5,6,5,6,5,6,5,6 },
   { 7,7,7,7,7,7,7,7 }
};

/* This routine validates all the interlace support macros in png.h for
 * a variety of valid PNG widths and heights.  It uses a number of similarly
 * named internal routines that feed off the above array.
 */
static png_uint_32
png_pass_start_row(int pass)
{
   int x, y;
   ++pass;
   for (y=0; y<8; ++y) for (x=0; x<8; ++x) if (adam7[y][x] == pass)
      return y;
   return 0xf;
}

static png_uint_32
png_pass_start_col(int pass)
{
   int x, y;
   ++pass;
   for (x=0; x<8; ++x) for (y=0; y<8; ++y) if (adam7[y][x] == pass)
      return x;
   return 0xf;
}

static int
png_pass_row_shift(int pass)
{
   int x, y, base=(-1), inc=8;
   ++pass;
   for (y=0; y<8; ++y) for (x=0; x<8; ++x) if (adam7[y][x] == pass)
   {
      if (base == (-1))
         base = y;
      else if (base == y)
         {}
      else if (inc == y-base)
         base=y;
      else if (inc == 8)
         inc = y-base, base=y;
      else if (inc != y-base)
         return 0xff; /* error - more than one 'inc' value! */
   }

   if (base == (-1)) return 0xfe; /* error - no row in pass! */

   /* The shift is always 1, 2 or 3 - no pass has all the rows! */
   switch (inc)
   {
case 2: return 1;
case 4: return 2;
case 8: return 3;
default: break;
   }

   /* error - unrecognized 'inc' */
   return (inc << 8) + 0xfd;
}

static int
png_pass_col_shift(int pass)
{
   int x, y, base=(-1), inc=8;
   ++pass;
   for (x=0; x<8; ++x) for (y=0; y<8; ++y) if (adam7[y][x] == pass)
   {
      if (base == (-1))
         base = x;
      else if (base == x)
         {}
      else if (inc == x-base)
         base=x;
      else if (inc == 8)
         inc = x-base, base=x;
      else if (inc != x-base)
         return 0xff; /* error - more than one 'inc' value! */
   }

   if (base == (-1)) return 0xfe; /* error - no row in pass! */

   /* The shift is always 1, 2 or 3 - no pass has all the rows! */
   switch (inc)
   {
case 1: return 0; /* pass 7 has all the columns */
case 2: return 1;
case 4: return 2;
case 8: return 3;
default: break;
   }

   /* error - unrecognized 'inc' */
   return (inc << 8) + 0xfd;
}

static png_uint_32
png_row_from_pass_row(png_uint_32 yIn, int pass)
{
   /* By examination of the array: */
   switch (pass)
   {
case 0: return yIn * 8;
case 1: return yIn * 8;
case 2: return yIn * 8 + 4;
case 3: return yIn * 4;
case 4: return yIn * 4 + 2;
case 5: return yIn * 2;
case 6: return yIn * 2 + 1;
default: break;
   }

   return 0xff; /* bad pass number */
}

static png_uint_32
png_col_from_pass_col(png_uint_32 xIn, int pass)
{
   /* By examination of the array: */
   switch (pass)
   {
case 0: return xIn * 8;
case 1: return xIn * 8 + 4;
case 2: return xIn * 4;
case 3: return xIn * 4 + 2;
case 4: return xIn * 2;
case 5: return xIn * 2 + 1;
case 6: return xIn;
default: break;
   }

   return 0xff; /* bad pass number */
}

static int
png_row_in_interlace_pass(png_uint_32 y, int pass)
{
   /* Is row 'y' in pass 'pass'? */
   int x;
   y &= 7;
   ++pass;
   for (x=0; x<8; ++x) if (adam7[y][x] == pass)
      return 1;

   return 0;
}

static int
png_col_in_interlace_pass(png_uint_32 x, int pass)
{
   /* Is column 'x' in pass 'pass'? */
   int y;
   x &= 7;
   ++pass;
   for (y=0; y<8; ++y) if (adam7[y][x] == pass)
      return 1;

   return 0;
}

static png_uint_32
png_pass_rows(png_uint_32 height, int pass)
{
   png_uint_32 tiles = height>>3;
   png_uint_32 rows = 0;
   unsigned int x, y;

   height &= 7;
   ++pass;
   for (y=0; y<8; ++y) for (x=0; x<8; ++x) if (adam7[y][x] == pass)
   {
      rows += tiles;
      if (y < height) ++rows;
      break; /* i.e. break the 'x', column, loop. */
   }

   return rows;
}

static png_uint_32
png_pass_cols(png_uint_32 width, int pass)
{
   png_uint_32 tiles = width>>3;
   png_uint_32 cols = 0;
   unsigned int x, y;

   width &= 7;
   ++pass;
   for (x=0; x<8; ++x) for (y=0; y<8; ++y) if (adam7[y][x] == pass)
   {
      cols += tiles;
      if (x < width) ++cols;
      break; /* i.e. break the 'y', row, loop. */
   }

   return cols;
}

static void
perform_interlace_macro_validation(void)
{
   /* The macros to validate, first those that depend only on pass:
    *
    * PNG_PASS_START_ROW(pass)
    * PNG_PASS_START_COL(pass)
    * PNG_PASS_ROW_SHIFT(pass)
    * PNG_PASS_COL_SHIFT(pass)
    */
   int pass;

   for (pass=0; pass<7; ++pass)
   {
      png_uint_32 m, f, v;

      m = PNG_PASS_START_ROW(pass);
      f = png_pass_start_row(pass);
      if (m != f)
      {
         fprintf(stderr, "PNG_PASS_START_ROW(%d) = %u != %x\n", pass, m, f);
         exit(1);
      }

      m = PNG_PASS_START_COL(pass);
      f = png_pass_start_col(pass);
      if (m != f)
      {
         fprintf(stderr, "PNG_PASS_START_COL(%d) = %u != %x\n", pass, m, f);
         exit(1);
      }

      m = PNG_PASS_ROW_SHIFT(pass);
      f = png_pass_row_shift(pass);
      if (m != f)
      {
         fprintf(stderr, "PNG_PASS_ROW_SHIFT(%d) = %u != %x\n", pass, m, f);
         exit(1);
      }

      m = PNG_PASS_COL_SHIFT(pass);
      f = png_pass_col_shift(pass);
      if (m != f)
      {
         fprintf(stderr, "PNG_PASS_COL_SHIFT(%d) = %u != %x\n", pass, m, f);
         exit(1);
      }

      /* Macros that depend on the image or sub-image height too:
       *
       * PNG_PASS_ROWS(height, pass)
       * PNG_PASS_COLS(width, pass)
       * PNG_ROW_FROM_PASS_ROW(yIn, pass)
       * PNG_COL_FROM_PASS_COL(xIn, pass)
       * PNG_ROW_IN_INTERLACE_PASS(y, pass)
       * PNG_COL_IN_INTERLACE_PASS(x, pass)
       */
      for (v=0;;)
      {
         /* First the base 0 stuff: */
         m = PNG_ROW_FROM_PASS_ROW(v, pass);
         f = png_row_from_pass_row(v, pass);
         if (m != f)
         {
            fprintf(stderr, "PNG_ROW_FROM_PASS_ROW(%u, %d) = %u != %x\n",
               v, pass, m, f);
            exit(1);
         }

         m = PNG_COL_FROM_PASS_COL(v, pass);
         f = png_col_from_pass_col(v, pass);
         if (m != f)
         {
            fprintf(stderr, "PNG_COL_FROM_PASS_COL(%u, %d) = %u != %x\n",
               v, pass, m, f);
            exit(1);
         }

         m = PNG_ROW_IN_INTERLACE_PASS(v, pass);
         f = png_row_in_interlace_pass(v, pass);
         if (m != f)
         {
            fprintf(stderr, "PNG_ROW_IN_INTERLACE_PASS(%u, %d) = %u != %x\n",
               v, pass, m, f);
            exit(1);
         }

         m = PNG_COL_IN_INTERLACE_PASS(v, pass);
         f = png_col_in_interlace_pass(v, pass);
         if (m != f)
         {
            fprintf(stderr, "PNG_COL_IN_INTERLACE_PASS(%u, %d) = %u != %x\n",
               v, pass, m, f);
            exit(1);
         }

         /* Then the base 1 stuff: */
         ++v;
         m = PNG_PASS_ROWS(v, pass);
         f = png_pass_rows(v, pass);
         if (m != f)
         {
            fprintf(stderr, "PNG_PASS_ROWS(%u, %d) = %u != %x\n",
               v, pass, m, f);
            exit(1);
         }

         m = PNG_PASS_COLS(v, pass);
         f = png_pass_cols(v, pass);
         if (m != f)
         {
            fprintf(stderr, "PNG_PASS_COLS(%u, %d) = %u != %x\n",
               v, pass, m, f);
            exit(1);
         }

         /* Move to the next v - the stepping algorithm starts skipping
          * values above 1024.
          */
         if (v > 1024)
         {
            if (v == PNG_UINT_31_MAX)
               break;

            v = (v << 1) ^ v;
            if (v >= PNG_UINT_31_MAX)
               v = PNG_UINT_31_MAX-1;
         }
      }
   }
}

/* main program */
int main(int argc, PNG_CONST char **argv)
{
   volatile int summary = 1;  /* Print the error summary at the end */

   /* Create the given output file on success: */
   PNG_CONST char *volatile touch = NULL;

   /* This is an array of standard gamma values (believe it or not I've seen
    * every one of these mentioned somewhere.)
    *
    * In the following list the most useful values are first!
    */
   static double
      gammas[]={2.2, 1.0, 2.2/1.45, 1.8, 1.5, 2.4, 2.5, 2.62, 2.9};

   png_modifier pm;
   context(&pm.this, fault);

   modifier_init(&pm);

   /* Preallocate the image buffer, because we know how big it needs to be,
    * note that, for testing purposes, it is deliberately mis-aligned.
    */
   pm.this.image = malloc(2*TRANSFORM_IMAGEMAX+1);

   if (pm.this.image != NULL)
   {
      /* Ignore OOM at this point - the 'ensure' routine above will allocate
       * the array appropriately.
       */
      ++(pm.this.image);
      pm.this.cb_image = 2*TRANSFORM_IMAGEMAX;
   }

   /* Default to error on warning: */
   pm.this.treat_warnings_as_errors = 1;

   /* Store the test gammas */
   pm.gammas = gammas;
   pm.ngammas = 0; /* default to off */
   pm.sbitlow = 8U; /* because libpng doesn't do sBIT below 8! */
   pm.use_input_precision_16to8 = 1U; /* Because of the way libpng does it */

   /* Some default values (set the behavior for 'make check' here).
    * These values simply control the maximum error permitted in the gamma
    * transformations.  The practial limits for human perception are described
    * below (the setting for maxpc16), however for 8 bit encodings it isn't
    * possible to meet the accepted capabilities of human vision - i.e. 8 bit
    * images can never be good enough, regardless of encoding.
    */
   pm.maxout8 = .1;     /* Arithmetic error in *encoded* value */
   pm.maxabs8 = .00005; /* 1/20000 */
   pm.maxpc8 = .499;    /* I.e., .499% fractional error */
   pm.maxout16 = .499;  /* Error in *encoded* value */
   pm.maxabs16 = .00005;/* 1/20000 */

   /* NOTE: this is a reasonable perceptual limit. We assume that humans can
    * perceive light level differences of 1% over a 100:1 range, so we need to
    * maintain 1 in 10000 accuracy (in linear light space), which is what the
    * following guarantees.  It also allows significantly higher errors at
    * higher 16 bit values, which is important for performance.  The actual
    * maximum 16 bit error is about +/-1.9 in the fixed point implementation but
    * this is only allowed for values >38149 by the following:
    */
   pm.maxpc16 = .005;   /* I.e., 1/200% - 1/20000 */

   /* Now parse the command line options. */
   while (--argc >= 1)
   {
      if (strcmp(*++argv, "-v") == 0)
         pm.this.verbose = 1;

      else if (strcmp(*argv, "-l") == 0)
         pm.log = 1;

      else if (strcmp(*argv, "-q") == 0)
         summary = pm.this.verbose = pm.log = 0;

      else if (strcmp(*argv, "-w") == 0)
         pm.this.treat_warnings_as_errors = 0;

      else if (strcmp(*argv, "--speed") == 0)
         pm.this.speed = 1, pm.ngammas = (sizeof gammas)/(sizeof gammas[0]),
            pm.test_standard = 0;

      else if (strcmp(*argv, "--size") == 0)
         pm.test_size = 1;

      else if (strcmp(*argv, "--nosize") == 0)
         pm.test_size = 0;

      else if (strcmp(*argv, "--standard") == 0)
         pm.test_standard = 1;

      else if (strcmp(*argv, "--nostandard") == 0)
         pm.test_standard = 0;

      else if (strcmp(*argv, "--transform") == 0)
         pm.test_transform = 1;

      else if (strcmp(*argv, "--notransform") == 0)
         pm.test_transform = 0;

      else if (strncmp(*argv, "--transform-disable=",
         sizeof "--transform-disable") == 0)
         {
         pm.test_transform = 1;
         transform_disable(*argv + sizeof "--transform-disable");
         }

      else if (strncmp(*argv, "--transform-enable=",
         sizeof "--transform-enable") == 0)
         {
         pm.test_transform = 1;
         transform_enable(*argv + sizeof "--transform-enable");
         }

      else if (strcmp(*argv, "--gamma") == 0)
         {
         /* Just do two gamma tests here (2.2 and linear) for speed: */
         pm.ngammas = 2U;
         pm.test_gamma_threshold = 1;
         pm.test_gamma_transform = 1;
         pm.test_gamma_sbit = 1;
         pm.test_gamma_strip16 = 1;
         }

      else if (strcmp(*argv, "--nogamma") == 0)
         pm.ngammas = 0;

      else if (strcmp(*argv, "--gamma-threshold") == 0)
         pm.ngammas = 2U, pm.test_gamma_threshold = 1;

      else if (strcmp(*argv, "--nogamma-threshold") == 0)
         pm.test_gamma_threshold = 0;

      else if (strcmp(*argv, "--gamma-transform") == 0)
         pm.ngammas = 2U, pm.test_gamma_transform = 1;

      else if (strcmp(*argv, "--nogamma-transform") == 0)
         pm.test_gamma_transform = 0;

      else if (strcmp(*argv, "--gamma-sbit") == 0)
         pm.ngammas = 2U, pm.test_gamma_sbit = 1;

      else if (strcmp(*argv, "--nogamma-sbit") == 0)
         pm.test_gamma_sbit = 0;

      else if (strcmp(*argv, "--gamma-16-to-8") == 0)
         pm.ngammas = 2U, pm.test_gamma_strip16 = 1;

      else if (strcmp(*argv, "--nogamma-16-to-8") == 0)
         pm.test_gamma_strip16 = 0;

      else if (strcmp(*argv, "--all-gammas") == 0)
         pm.ngammas = (sizeof gammas)/(sizeof gammas[0]);

      else if (strcmp(*argv, "--progressive-read") == 0)
         pm.this.progressive = 1;

      else if (strcmp(*argv, "--interlace") == 0)
         pm.interlace_type = PNG_INTERLACE_ADAM7;

      else if (argc >= 1 && strcmp(*argv, "--sbitlow") == 0)
         --argc, pm.sbitlow = (png_byte)atoi(*++argv);

      else if (argc >= 1 && strcmp(*argv, "--touch") == 0)
         --argc, touch = *++argv;

      else if (argc >= 1 && strncmp(*argv, "--max", 4) == 0)
      {
         --argc;

         if (strcmp(4+*argv, "abs8") == 0)
            pm.maxabs8 = atof(*++argv);

         else if (strcmp(4+*argv, "abs16") == 0)
            pm.maxabs16 = atof(*++argv);

         else if (strcmp(4+*argv, "out8") == 0)
            pm.maxout8 = atof(*++argv);

         else if (strcmp(4+*argv, "out16") == 0)
            pm.maxout16 = atof(*++argv);

         else if (strcmp(4+*argv, "pc8") == 0)
            pm.maxpc8 = atof(*++argv);

         else if (strcmp(4+*argv, "pc16") == 0)
            pm.maxpc16 = atof(*++argv);

         else
         {
            fprintf(stderr, "pngvalid: %s: unknown 'max' option\n", *argv);
            exit(1);
         }
      }

      else
      {
         fprintf(stderr, "pngvalid: %s: unknown argument\n", *argv);
         exit(1);
      }
   }

   /* If pngvalid is run with no arguments default to a reasonable set of the
    * tests.
    */
   if (pm.test_standard == 0 && pm.test_size == 0 && pm.test_transform == 0 &&
      pm.ngammas == 0)
   {
      pm.test_standard = 1;
      pm.test_size = 1;
      pm.test_transform = 1;
      pm.ngammas = 3U;
   }

   if (pm.ngammas > 0 &&
      pm.test_gamma_threshold == 0 && pm.test_gamma_transform == 0 &&
      pm.test_gamma_sbit == 0 && pm.test_gamma_strip16 == 0)
   {
      pm.test_gamma_threshold = 1;
      pm.test_gamma_transform = 1;
      pm.test_gamma_sbit = 1;
      pm.test_gamma_strip16 = 1;
   }

   else if (pm.ngammas == 0)
   {
      /* Nothing to test so turn everything off: */
      pm.test_gamma_threshold = 0;
      pm.test_gamma_transform = 0;
      pm.test_gamma_sbit = 0;
      pm.test_gamma_strip16 = 0;
   }

   Try
   {
      /* Make useful base images */
      make_transform_images(&pm.this);

      /* Perform the standard and gamma tests. */
      if (pm.test_standard)
      {
         perform_interlace_macro_validation();
         perform_standard_test(&pm);
         perform_error_test(&pm);
      }

      /* Various oddly sized images: */
      if (pm.test_size)
      {
         make_size_images(&pm.this);
         perform_size_test(&pm);
      }

      /* Combinatorial transforms: */
      if (pm.test_transform)
         perform_transform_test(&pm);

      if (pm.ngammas > 0)
         perform_gamma_test(&pm, pm.this.speed != 0,
            summary && !pm.this.speed);
   }

   Catch(fault)
   {
      fprintf(stderr, "pngvalid: test aborted (probably failed in cleanup)\n");
      if (!pm.this.verbose)
      {
         if (pm.this.error[0] != 0)
            fprintf(stderr, "pngvalid: first error: %s\n", pm.this.error);

         fprintf(stderr, "pngvalid: run with -v to see what happened\n");
      }
      exit(1);
   }

   if (summary && !pm.this.speed)
   {
      printf("Results using %s point arithmetic %s\n",
#if defined(PNG_FLOATING_ARITHMETIC_SUPPORTED) || PNG_LIBPNG_VER < 10500
         "floating",
#else
         "fixed",
#endif
         (pm.this.nerrors || (pm.this.treat_warnings_as_errors &&
            pm.this.nwarnings)) ? "(errors)" : (pm.this.nwarnings ?
               "(warnings)" : "(no errors or warnings)")
      );
      printf("Allocated memory statistics (in bytes):\n"
         "\tread  %lu maximum single, %lu peak, %lu total\n"
         "\twrite %lu maximum single, %lu peak, %lu total\n",
         (unsigned long)pm.this.read_memory_pool.max_max,
         (unsigned long)pm.this.read_memory_pool.max_limit,
         (unsigned long)pm.this.read_memory_pool.max_total,
         (unsigned long)pm.this.write_memory_pool.max_max,
         (unsigned long)pm.this.write_memory_pool.max_limit,
         (unsigned long)pm.this.write_memory_pool.max_total);
   }

   /* Do this here to provoke memory corruption errors in memory not directly
    * allocated by libpng - not a complete test, but better than nothing.
    */
   store_delete(&pm.this);

   /* Error exit if there are any errors, and maybe if there are any
    * warnings.
    */
   if (pm.this.nerrors || (pm.this.treat_warnings_as_errors &&
       pm.this.nwarnings))
   {
      if (!pm.this.verbose)
         fprintf(stderr, "pngvalid: %s\n", pm.this.error);

      fprintf(stderr, "pngvalid: %d errors, %d warnings\n", pm.this.nerrors,
          pm.this.nwarnings);

      exit(1);
   }

   /* Success case. */
   if (touch != NULL)
   {
      FILE *fsuccess = fopen(touch, "wt");

      if (fsuccess != NULL)
      {
         int error = 0;
         fprintf(fsuccess, "PNG validation succeeded\n");
         fflush(fsuccess);
         error = ferror(fsuccess);

         if (fclose(fsuccess) || error)
         {
            fprintf(stderr, "%s: write failed\n", touch);
            exit(1);
         }
      }
   }

   return 0;
}
