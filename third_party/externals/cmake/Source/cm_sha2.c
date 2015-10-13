/*
 * FILE:	sha2.c
 * AUTHOR:	Aaron D. Gifford
 *		http://www.aarongifford.com/computers/sha.html
 *
 * Copyright (c) 2000-2003, Aaron D. Gifford
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
 * 3. Neither the name of the copyright holder nor the names of contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTOR(S) ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTOR(S) BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $Id: sha2.c,v 1.4 2004/01/07 22:58:18 adg Exp $
 */

#include <string.h>	/* memcpy()/memset() or bcopy()/bzero() */
#include <assert.h>	/* assert() */
#include "cm_sha2.h"	/* "sha2.h" -> "cm_sha2.h" renamed for CMake */

/*
 * ASSERT NOTE:
 * Some sanity checking code is included using assert().  On my FreeBSD
 * system, this additional code can be removed by compiling with NDEBUG
 * defined.  Check your own systems manpage on assert() to see how to
 * compile WITHOUT the sanity checking code on your system.
 *
 * UNROLLED TRANSFORM LOOP NOTE:
 * You can define SHA2_UNROLL_TRANSFORM to use the unrolled transform
 * loop version for the hash transform rounds (defined using macros
 * later in this file).  Either define on the command line, for example:
 *
 *   cc -DSHA2_UNROLL_TRANSFORM -o sha2 sha2.c sha2prog.c
 *
 * or define below:
 *
 *   #define SHA2_UNROLL_TRANSFORM
 *
 */


/*** SHA-224/256/384/512 Machine Architecture Definitions *************/
/*
 * BYTE_ORDER NOTE:
 *
 * Please make sure that your system defines BYTE_ORDER.  If your
 * architecture is little-endian, make sure it also defines
 * LITTLE_ENDIAN and that the two (BYTE_ORDER and LITTLE_ENDIAN) are
 * equivilent.
 *
 * If your system does not define the above, then you can do so by
 * hand like this:
 *
 *   #define LITTLE_ENDIAN 1234
 *   #define BIG_ENDIAN    4321
 *
 * And for little-endian machines, add:
 *
 *   #define BYTE_ORDER LITTLE_ENDIAN
 *
 * Or for big-endian machines:
 *
 *   #define BYTE_ORDER BIG_ENDIAN
 *
 * The FreeBSD machine this was written on defines BYTE_ORDER
 * appropriately by including <sys/types.h> (which in turn includes
 * <machine/endian.h> where the appropriate definitions are actually
 * made).
 */
#if !defined(BYTE_ORDER) || (BYTE_ORDER != LITTLE_ENDIAN && BYTE_ORDER != BIG_ENDIAN)
/* CMake modification: use byte order from cmIML.  */
# include "cmIML/ABI.h"
# undef BYTE_ORDER
# undef BIG_ENDIAN
# undef LITTLE_ENDIAN
# define BYTE_ORDER    cmIML_ABI_ENDIAN_ID
# define BIG_ENDIAN    cmIML_ABI_ENDIAN_ID_BIG
# define LITTLE_ENDIAN cmIML_ABI_ENDIAN_ID_LITTLE
#endif

/* CMake modification: use types computed in header.  */
typedef cm_sha2_uint8_t  sha_byte;	/* Exactly 1 byte */
typedef cm_sha2_uint32_t sha_word32;	/* Exactly 4 bytes */
typedef cm_sha2_uint64_t sha_word64;	/* Exactly 8 bytes */
#define SHA_UINT32_C(x) cmIML_INT_UINT32_C(x)
#define SHA_UINT64_C(x) cmIML_INT_UINT64_C(x)
#if defined(__clang__)
# pragma clang diagnostic ignored "-Wcast-align"
#endif

/*** ENDIAN REVERSAL MACROS *******************************************/
#if BYTE_ORDER == LITTLE_ENDIAN
#define REVERSE32(w,x)	{ \
	sha_word32 tmp = (w); \
	tmp = (tmp >> 16) | (tmp << 16); \
	(x) = ((tmp & SHA_UINT32_C(0xff00ff00)) >> 8) | \
	      ((tmp & SHA_UINT32_C(0x00ff00ff)) << 8); \
}
#define REVERSE64(w,x)	{ \
	sha_word64 tmp = (w); \
	tmp = (tmp >> 32) | (tmp << 32); \
	tmp = ((tmp & SHA_UINT64_C(0xff00ff00ff00ff00)) >> 8) | \
	      ((tmp & SHA_UINT64_C(0x00ff00ff00ff00ff)) << 8); \
	(x) = ((tmp & SHA_UINT64_C(0xffff0000ffff0000)) >> 16) | \
	      ((tmp & SHA_UINT64_C(0x0000ffff0000ffff)) << 16); \
}
#endif /* BYTE_ORDER == LITTLE_ENDIAN */

/*
 * Macro for incrementally adding the unsigned 64-bit integer n to the
 * unsigned 128-bit integer (represented using a two-element array of
 * 64-bit words):
 */
#define ADDINC128(w,n)	{ \
	(w)[0] += (sha_word64)(n); \
	if ((w)[0] < (n)) { \
		(w)[1]++; \
	} \
}

/*
 * Macros for copying blocks of memory and for zeroing out ranges
 * of memory.  Using these macros makes it easy to switch from
 * using memset()/memcpy() and using bzero()/bcopy().
 *
 * Please define either SHA2_USE_MEMSET_MEMCPY or define
 * SHA2_USE_BZERO_BCOPY depending on which function set you
 * choose to use:
 */
#if !defined(SHA2_USE_MEMSET_MEMCPY) && !defined(SHA2_USE_BZERO_BCOPY)
/* Default to memset()/memcpy() if no option is specified */
#define	SHA2_USE_MEMSET_MEMCPY	1
#endif
#if defined(SHA2_USE_MEMSET_MEMCPY) && defined(SHA2_USE_BZERO_BCOPY)
/* Abort with an error if BOTH options are defined */
#error Define either SHA2_USE_MEMSET_MEMCPY or SHA2_USE_BZERO_BCOPY, not both!
#endif

#ifdef SHA2_USE_MEMSET_MEMCPY
#define MEMSET_BZERO(p,l)	memset((p), 0, (l))
#define MEMCPY_BCOPY(d,s,l)	memcpy((d), (s), (l))
#endif
#ifdef SHA2_USE_BZERO_BCOPY
#define MEMSET_BZERO(p,l)	bzero((p), (l))
#define MEMCPY_BCOPY(d,s,l)	bcopy((s), (d), (l))
#endif


/*** THE SIX LOGICAL FUNCTIONS ****************************************/
/*
 * Bit shifting and rotation (used by the six SHA-XYZ logical functions:
 *
 *   NOTE:  In the original SHA-256/384/512 document, the shift-right
 *   function was named R and the rotate-right function was called S.
 *   (See: http://csrc.nist.gov/cryptval/shs/sha256-384-512.pdf on the
 *   web.)
 *
 *   The newer NIST FIPS 180-2 document uses a much clearer naming
 *   scheme, SHR for shift-right, ROTR for rotate-right, and ROTL for
 *   rotate-left.  (See:
 *   http://csrc.nist.gov/publications/fips/fips180-2/fips180-2.pdf
 *   on the web.)
 *
 *   WARNING: These macros must be used cautiously, since they reference
 *   supplied parameters sometimes more than once, and thus could have
 *   unexpected side-effects if used without taking this into account.
 */
/* Shift-right (used in SHA-256, SHA-384, and SHA-512): */
#define SHR(b,x) 		((x) >> (b))
/* 32-bit Rotate-right (used in SHA-256): */
#define ROTR32(b,x)	(((x) >> (b)) | ((x) << (32 - (b))))
/* 64-bit Rotate-right (used in SHA-384 and SHA-512): */
#define ROTR64(b,x)	(((x) >> (b)) | ((x) << (64 - (b))))
/* 32-bit Rotate-left (used in SHA-1): */
#define ROTL32(b,x)	(((x) << (b)) | ((x) >> (32 - (b))))

/* Two logical functions used in SHA-1, SHA-254, SHA-256, SHA-384, and SHA-512: */
#define Ch(x,y,z)	(((x) & (y)) ^ ((~(x)) & (z)))
#define Maj(x,y,z)	(((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))

/* Function used in SHA-1: */
#define Parity(x,y,z)	((x) ^ (y) ^ (z))

/* Four logical functions used in SHA-256: */
#define Sigma0_256(x)	(ROTR32(2,  (x)) ^ ROTR32(13, (x)) ^ ROTR32(22, (x)))
#define Sigma1_256(x)	(ROTR32(6,  (x)) ^ ROTR32(11, (x)) ^ ROTR32(25, (x)))
#define sigma0_256(x)	(ROTR32(7,  (x)) ^ ROTR32(18, (x)) ^ SHR(   3 , (x)))
#define sigma1_256(x)	(ROTR32(17, (x)) ^ ROTR32(19, (x)) ^ SHR(   10, (x)))

/* Four of six logical functions used in SHA-384 and SHA-512: */
#define Sigma0_512(x)	(ROTR64(28, (x)) ^ ROTR64(34, (x)) ^ ROTR64(39, (x)))
#define Sigma1_512(x)	(ROTR64(14, (x)) ^ ROTR64(18, (x)) ^ ROTR64(41, (x)))
#define sigma0_512(x)	(ROTR64( 1, (x)) ^ ROTR64( 8, (x)) ^ SHR(    7, (x)))
#define sigma1_512(x)	(ROTR64(19, (x)) ^ ROTR64(61, (x)) ^ SHR(    6, (x)))

/*** INTERNAL FUNCTION PROTOTYPES *************************************/

/* SHA-224 and SHA-256: */
void SHA256_Internal_Init(SHA_CTX*, const sha_word32*);
void SHA256_Internal_Last(SHA_CTX*);
void SHA256_Internal_Transform(SHA_CTX*, const sha_word32*);

/* SHA-384 and SHA-512: */
void SHA512_Internal_Init(SHA_CTX*, const sha_word64*);
void SHA512_Internal_Last(SHA_CTX*);
void SHA512_Internal_Transform(SHA_CTX*, const sha_word64*);


/*** SHA2 INITIAL HASH VALUES AND CONSTANTS ***************************/

/* Hash constant words K for SHA-1: */
#define K1_0_TO_19	SHA_UINT32_C(0x5a827999)
#define K1_20_TO_39	SHA_UINT32_C(0x6ed9eba1)
#define K1_40_TO_59	SHA_UINT32_C(0x8f1bbcdc)
#define K1_60_TO_79	SHA_UINT32_C(0xca62c1d6)

/* Initial hash value H for SHA-1: */
static const sha_word32 sha1_initial_hash_value[5] = {
	SHA_UINT32_C(0x67452301),
	SHA_UINT32_C(0xefcdab89),
	SHA_UINT32_C(0x98badcfe),
	SHA_UINT32_C(0x10325476),
	SHA_UINT32_C(0xc3d2e1f0)
};

/* Hash constant words K for SHA-224 and SHA-256: */
static const sha_word32 K256[64] = {
	SHA_UINT32_C(0x428a2f98), SHA_UINT32_C(0x71374491),
	SHA_UINT32_C(0xb5c0fbcf), SHA_UINT32_C(0xe9b5dba5),
	SHA_UINT32_C(0x3956c25b), SHA_UINT32_C(0x59f111f1),
	SHA_UINT32_C(0x923f82a4), SHA_UINT32_C(0xab1c5ed5),
	SHA_UINT32_C(0xd807aa98), SHA_UINT32_C(0x12835b01),
	SHA_UINT32_C(0x243185be), SHA_UINT32_C(0x550c7dc3),
	SHA_UINT32_C(0x72be5d74), SHA_UINT32_C(0x80deb1fe),
	SHA_UINT32_C(0x9bdc06a7), SHA_UINT32_C(0xc19bf174),
	SHA_UINT32_C(0xe49b69c1), SHA_UINT32_C(0xefbe4786),
	SHA_UINT32_C(0x0fc19dc6), SHA_UINT32_C(0x240ca1cc),
	SHA_UINT32_C(0x2de92c6f), SHA_UINT32_C(0x4a7484aa),
	SHA_UINT32_C(0x5cb0a9dc), SHA_UINT32_C(0x76f988da),
	SHA_UINT32_C(0x983e5152), SHA_UINT32_C(0xa831c66d),
	SHA_UINT32_C(0xb00327c8), SHA_UINT32_C(0xbf597fc7),
	SHA_UINT32_C(0xc6e00bf3), SHA_UINT32_C(0xd5a79147),
	SHA_UINT32_C(0x06ca6351), SHA_UINT32_C(0x14292967),
	SHA_UINT32_C(0x27b70a85), SHA_UINT32_C(0x2e1b2138),
	SHA_UINT32_C(0x4d2c6dfc), SHA_UINT32_C(0x53380d13),
	SHA_UINT32_C(0x650a7354), SHA_UINT32_C(0x766a0abb),
	SHA_UINT32_C(0x81c2c92e), SHA_UINT32_C(0x92722c85),
	SHA_UINT32_C(0xa2bfe8a1), SHA_UINT32_C(0xa81a664b),
	SHA_UINT32_C(0xc24b8b70), SHA_UINT32_C(0xc76c51a3),
	SHA_UINT32_C(0xd192e819), SHA_UINT32_C(0xd6990624),
	SHA_UINT32_C(0xf40e3585), SHA_UINT32_C(0x106aa070),
	SHA_UINT32_C(0x19a4c116), SHA_UINT32_C(0x1e376c08),
	SHA_UINT32_C(0x2748774c), SHA_UINT32_C(0x34b0bcb5),
	SHA_UINT32_C(0x391c0cb3), SHA_UINT32_C(0x4ed8aa4a),
	SHA_UINT32_C(0x5b9cca4f), SHA_UINT32_C(0x682e6ff3),
	SHA_UINT32_C(0x748f82ee), SHA_UINT32_C(0x78a5636f),
	SHA_UINT32_C(0x84c87814), SHA_UINT32_C(0x8cc70208),
	SHA_UINT32_C(0x90befffa), SHA_UINT32_C(0xa4506ceb),
	SHA_UINT32_C(0xbef9a3f7), SHA_UINT32_C(0xc67178f2)
};

/* Initial hash value H for SHA-224: */
static const sha_word32 sha224_initial_hash_value[8] = {
	SHA_UINT32_C(0xc1059ed8),
	SHA_UINT32_C(0x367cd507),
	SHA_UINT32_C(0x3070dd17),
	SHA_UINT32_C(0xf70e5939),
	SHA_UINT32_C(0xffc00b31),
	SHA_UINT32_C(0x68581511),
	SHA_UINT32_C(0x64f98fa7),
	SHA_UINT32_C(0xbefa4fa4)
};

/* Initial hash value H for SHA-256: */
static const sha_word32 sha256_initial_hash_value[8] = {
	SHA_UINT32_C(0x6a09e667),
	SHA_UINT32_C(0xbb67ae85),
	SHA_UINT32_C(0x3c6ef372),
	SHA_UINT32_C(0xa54ff53a),
	SHA_UINT32_C(0x510e527f),
	SHA_UINT32_C(0x9b05688c),
	SHA_UINT32_C(0x1f83d9ab),
	SHA_UINT32_C(0x5be0cd19)
};

/* Hash constant words K for SHA-384 and SHA-512: */
static const sha_word64 K512[80] = {
	SHA_UINT64_C(0x428a2f98d728ae22), SHA_UINT64_C(0x7137449123ef65cd),
	SHA_UINT64_C(0xb5c0fbcfec4d3b2f), SHA_UINT64_C(0xe9b5dba58189dbbc),
	SHA_UINT64_C(0x3956c25bf348b538), SHA_UINT64_C(0x59f111f1b605d019),
	SHA_UINT64_C(0x923f82a4af194f9b), SHA_UINT64_C(0xab1c5ed5da6d8118),
	SHA_UINT64_C(0xd807aa98a3030242), SHA_UINT64_C(0x12835b0145706fbe),
	SHA_UINT64_C(0x243185be4ee4b28c), SHA_UINT64_C(0x550c7dc3d5ffb4e2),
	SHA_UINT64_C(0x72be5d74f27b896f), SHA_UINT64_C(0x80deb1fe3b1696b1),
	SHA_UINT64_C(0x9bdc06a725c71235), SHA_UINT64_C(0xc19bf174cf692694),
	SHA_UINT64_C(0xe49b69c19ef14ad2), SHA_UINT64_C(0xefbe4786384f25e3),
	SHA_UINT64_C(0x0fc19dc68b8cd5b5), SHA_UINT64_C(0x240ca1cc77ac9c65),
	SHA_UINT64_C(0x2de92c6f592b0275), SHA_UINT64_C(0x4a7484aa6ea6e483),
	SHA_UINT64_C(0x5cb0a9dcbd41fbd4), SHA_UINT64_C(0x76f988da831153b5),
	SHA_UINT64_C(0x983e5152ee66dfab), SHA_UINT64_C(0xa831c66d2db43210),
	SHA_UINT64_C(0xb00327c898fb213f), SHA_UINT64_C(0xbf597fc7beef0ee4),
	SHA_UINT64_C(0xc6e00bf33da88fc2), SHA_UINT64_C(0xd5a79147930aa725),
	SHA_UINT64_C(0x06ca6351e003826f), SHA_UINT64_C(0x142929670a0e6e70),
	SHA_UINT64_C(0x27b70a8546d22ffc), SHA_UINT64_C(0x2e1b21385c26c926),
	SHA_UINT64_C(0x4d2c6dfc5ac42aed), SHA_UINT64_C(0x53380d139d95b3df),
	SHA_UINT64_C(0x650a73548baf63de), SHA_UINT64_C(0x766a0abb3c77b2a8),
	SHA_UINT64_C(0x81c2c92e47edaee6), SHA_UINT64_C(0x92722c851482353b),
	SHA_UINT64_C(0xa2bfe8a14cf10364), SHA_UINT64_C(0xa81a664bbc423001),
	SHA_UINT64_C(0xc24b8b70d0f89791), SHA_UINT64_C(0xc76c51a30654be30),
	SHA_UINT64_C(0xd192e819d6ef5218), SHA_UINT64_C(0xd69906245565a910),
	SHA_UINT64_C(0xf40e35855771202a), SHA_UINT64_C(0x106aa07032bbd1b8),
	SHA_UINT64_C(0x19a4c116b8d2d0c8), SHA_UINT64_C(0x1e376c085141ab53),
	SHA_UINT64_C(0x2748774cdf8eeb99), SHA_UINT64_C(0x34b0bcb5e19b48a8),
	SHA_UINT64_C(0x391c0cb3c5c95a63), SHA_UINT64_C(0x4ed8aa4ae3418acb),
	SHA_UINT64_C(0x5b9cca4f7763e373), SHA_UINT64_C(0x682e6ff3d6b2b8a3),
	SHA_UINT64_C(0x748f82ee5defb2fc), SHA_UINT64_C(0x78a5636f43172f60),
	SHA_UINT64_C(0x84c87814a1f0ab72), SHA_UINT64_C(0x8cc702081a6439ec),
	SHA_UINT64_C(0x90befffa23631e28), SHA_UINT64_C(0xa4506cebde82bde9),
	SHA_UINT64_C(0xbef9a3f7b2c67915), SHA_UINT64_C(0xc67178f2e372532b),
	SHA_UINT64_C(0xca273eceea26619c), SHA_UINT64_C(0xd186b8c721c0c207),
	SHA_UINT64_C(0xeada7dd6cde0eb1e), SHA_UINT64_C(0xf57d4f7fee6ed178),
	SHA_UINT64_C(0x06f067aa72176fba), SHA_UINT64_C(0x0a637dc5a2c898a6),
	SHA_UINT64_C(0x113f9804bef90dae), SHA_UINT64_C(0x1b710b35131c471b),
	SHA_UINT64_C(0x28db77f523047d84), SHA_UINT64_C(0x32caab7b40c72493),
	SHA_UINT64_C(0x3c9ebe0a15c9bebc), SHA_UINT64_C(0x431d67c49c100d4c),
	SHA_UINT64_C(0x4cc5d4becb3e42b6), SHA_UINT64_C(0x597f299cfc657e2a),
	SHA_UINT64_C(0x5fcb6fab3ad6faec), SHA_UINT64_C(0x6c44198c4a475817)
};

/* Initial hash value H for SHA-384 */
static const sha_word64 sha384_initial_hash_value[8] = {
	SHA_UINT64_C(0xcbbb9d5dc1059ed8),
	SHA_UINT64_C(0x629a292a367cd507),
	SHA_UINT64_C(0x9159015a3070dd17),
	SHA_UINT64_C(0x152fecd8f70e5939),
	SHA_UINT64_C(0x67332667ffc00b31),
	SHA_UINT64_C(0x8eb44a8768581511),
	SHA_UINT64_C(0xdb0c2e0d64f98fa7),
	SHA_UINT64_C(0x47b5481dbefa4fa4)
};

/* Initial hash value H for SHA-512 */
static const sha_word64 sha512_initial_hash_value[8] = {
	SHA_UINT64_C(0x6a09e667f3bcc908),
	SHA_UINT64_C(0xbb67ae8584caa73b),
	SHA_UINT64_C(0x3c6ef372fe94f82b),
	SHA_UINT64_C(0xa54ff53a5f1d36f1),
	SHA_UINT64_C(0x510e527fade682d1),
	SHA_UINT64_C(0x9b05688c2b3e6c1f),
	SHA_UINT64_C(0x1f83d9abfb41bd6b),
	SHA_UINT64_C(0x5be0cd19137e2179)
};

/*
 * Constant used by SHA224/256/384/512_End() functions for converting the
 * digest to a readable hexadecimal character string:
 */
static const char *sha_hex_digits = "0123456789abcdef";


/*** SHA-1: ***********************************************************/
void SHA1_Init(SHA_CTX* context) {
	/* Sanity check: */
	assert(context != (SHA_CTX*)0);

	MEMCPY_BCOPY(context->s1.state, sha1_initial_hash_value, sizeof(sha_word32) * 5);
	MEMSET_BZERO(context->s1.buffer, 64);
	context->s1.bitcount = 0;
}

#ifdef SHA2_UNROLL_TRANSFORM

/* Unrolled SHA-1 round macros: */

#if BYTE_ORDER == LITTLE_ENDIAN

#define ROUND1_0_TO_15(a,b,c,d,e)				\
	REVERSE32(*data++, W1[j]);				\
	(e) = ROTL32(5, (a)) + Ch((b), (c), (d)) + (e) +	\
	     K1_0_TO_19 + W1[j];	\
	(b) = ROTL32(30, (b));		\
	j++;

#else /* BYTE_ORDER == LITTLE_ENDIAN */

#define ROUND1_0_TO_15(a,b,c,d,e)				\
	(e) = ROTL32(5, (a)) + Ch((b), (c), (d)) + (e) +	\
	     K1_0_TO_19 + ( W1[j] = *data++ );		\
	(b) = ROTL32(30, (b));	\
	j++;

#endif /* BYTE_ORDER == LITTLE_ENDIAN */

#define ROUND1_16_TO_19(a,b,c,d,e)	\
	T1 = W1[(j+13)&0x0f] ^ W1[(j+8)&0x0f] ^ W1[(j+2)&0x0f] ^ W1[j&0x0f];	\
	(e) = ROTL32(5, a) + Ch(b,c,d) + e + K1_0_TO_19 + ( W1[j&0x0f] = ROTL32(1, T1) );	\
	(b) = ROTL32(30, b);	\
	j++;

#define ROUND1_20_TO_39(a,b,c,d,e)	\
	T1 = W1[(j+13)&0x0f] ^ W1[(j+8)&0x0f] ^ W1[(j+2)&0x0f] ^ W1[j&0x0f];	\
	(e) = ROTL32(5, a) + Parity(b,c,d) + e + K1_20_TO_39 + ( W1[j&0x0f] = ROTL32(1, T1) );	\
	(b) = ROTL32(30, b);	\
	j++;

#define ROUND1_40_TO_59(a,b,c,d,e)	\
	T1 = W1[(j+13)&0x0f] ^ W1[(j+8)&0x0f] ^ W1[(j+2)&0x0f] ^ W1[j&0x0f];	\
	(e) = ROTL32(5, a) + Maj(b,c,d) + e + K1_40_TO_59 + ( W1[j&0x0f] = ROTL32(1, T1) );	\
	(b) = ROTL32(30, b);	\
	j++;

#define ROUND1_60_TO_79(a,b,c,d,e)	\
	T1 = W1[(j+13)&0x0f] ^ W1[(j+8)&0x0f] ^ W1[(j+2)&0x0f] ^ W1[j&0x0f];	\
	(e) = ROTL32(5, a) + Parity(b,c,d) + e + K1_60_TO_79 + ( W1[j&0x0f] = ROTL32(1, T1) );	\
	(b) = ROTL32(30, b);	\
	j++;

void SHA1_Internal_Transform(SHA_CTX* context, const sha_word32* data) {
	sha_word32	a, b, c, d, e;
	sha_word32	T1, *W1;
	int		j;

	W1 = (sha_word32*)context->s1.buffer;

	/* Initialize registers with the prev. intermediate value */
	a = context->s1.state[0];
	b = context->s1.state[1];
	c = context->s1.state[2];
	d = context->s1.state[3];
	e = context->s1.state[4];

	j = 0;

	/* Rounds 0 to 15 unrolled: */
	ROUND1_0_TO_15(a,b,c,d,e);
	ROUND1_0_TO_15(e,a,b,c,d);
	ROUND1_0_TO_15(d,e,a,b,c);
	ROUND1_0_TO_15(c,d,e,a,b);
	ROUND1_0_TO_15(b,c,d,e,a);
	ROUND1_0_TO_15(a,b,c,d,e);
	ROUND1_0_TO_15(e,a,b,c,d);
	ROUND1_0_TO_15(d,e,a,b,c);
	ROUND1_0_TO_15(c,d,e,a,b);
	ROUND1_0_TO_15(b,c,d,e,a);
	ROUND1_0_TO_15(a,b,c,d,e);
	ROUND1_0_TO_15(e,a,b,c,d);
	ROUND1_0_TO_15(d,e,a,b,c);
	ROUND1_0_TO_15(c,d,e,a,b);
	ROUND1_0_TO_15(b,c,d,e,a);
	ROUND1_0_TO_15(a,b,c,d,e);

	/* Rounds 16 to 19 unrolled: */
	ROUND1_16_TO_19(e,a,b,c,d);
	ROUND1_16_TO_19(d,e,a,b,c);
	ROUND1_16_TO_19(c,d,e,a,b);
	ROUND1_16_TO_19(b,c,d,e,a);

	/* Rounds 20 to 39 unrolled: */
	ROUND1_20_TO_39(a,b,c,d,e);
	ROUND1_20_TO_39(e,a,b,c,d);
	ROUND1_20_TO_39(d,e,a,b,c);
	ROUND1_20_TO_39(c,d,e,a,b);
	ROUND1_20_TO_39(b,c,d,e,a);
	ROUND1_20_TO_39(a,b,c,d,e);
	ROUND1_20_TO_39(e,a,b,c,d);
	ROUND1_20_TO_39(d,e,a,b,c);
	ROUND1_20_TO_39(c,d,e,a,b);
	ROUND1_20_TO_39(b,c,d,e,a);
	ROUND1_20_TO_39(a,b,c,d,e);
	ROUND1_20_TO_39(e,a,b,c,d);
	ROUND1_20_TO_39(d,e,a,b,c);
	ROUND1_20_TO_39(c,d,e,a,b);
	ROUND1_20_TO_39(b,c,d,e,a);
	ROUND1_20_TO_39(a,b,c,d,e);
	ROUND1_20_TO_39(e,a,b,c,d);
	ROUND1_20_TO_39(d,e,a,b,c);
	ROUND1_20_TO_39(c,d,e,a,b);
	ROUND1_20_TO_39(b,c,d,e,a);

	/* Rounds 40 to 59 unrolled: */
	ROUND1_40_TO_59(a,b,c,d,e);
	ROUND1_40_TO_59(e,a,b,c,d);
	ROUND1_40_TO_59(d,e,a,b,c);
	ROUND1_40_TO_59(c,d,e,a,b);
	ROUND1_40_TO_59(b,c,d,e,a);
	ROUND1_40_TO_59(a,b,c,d,e);
	ROUND1_40_TO_59(e,a,b,c,d);
	ROUND1_40_TO_59(d,e,a,b,c);
	ROUND1_40_TO_59(c,d,e,a,b);
	ROUND1_40_TO_59(b,c,d,e,a);
	ROUND1_40_TO_59(a,b,c,d,e);
	ROUND1_40_TO_59(e,a,b,c,d);
	ROUND1_40_TO_59(d,e,a,b,c);
	ROUND1_40_TO_59(c,d,e,a,b);
	ROUND1_40_TO_59(b,c,d,e,a);
	ROUND1_40_TO_59(a,b,c,d,e);
	ROUND1_40_TO_59(e,a,b,c,d);
	ROUND1_40_TO_59(d,e,a,b,c);
	ROUND1_40_TO_59(c,d,e,a,b);
	ROUND1_40_TO_59(b,c,d,e,a);

	/* Rounds 60 to 79 unrolled: */
	ROUND1_60_TO_79(a,b,c,d,e);
	ROUND1_60_TO_79(e,a,b,c,d);
	ROUND1_60_TO_79(d,e,a,b,c);
	ROUND1_60_TO_79(c,d,e,a,b);
	ROUND1_60_TO_79(b,c,d,e,a);
	ROUND1_60_TO_79(a,b,c,d,e);
	ROUND1_60_TO_79(e,a,b,c,d);
	ROUND1_60_TO_79(d,e,a,b,c);
	ROUND1_60_TO_79(c,d,e,a,b);
	ROUND1_60_TO_79(b,c,d,e,a);
	ROUND1_60_TO_79(a,b,c,d,e);
	ROUND1_60_TO_79(e,a,b,c,d);
	ROUND1_60_TO_79(d,e,a,b,c);
	ROUND1_60_TO_79(c,d,e,a,b);
	ROUND1_60_TO_79(b,c,d,e,a);
	ROUND1_60_TO_79(a,b,c,d,e);
	ROUND1_60_TO_79(e,a,b,c,d);
	ROUND1_60_TO_79(d,e,a,b,c);
	ROUND1_60_TO_79(c,d,e,a,b);
	ROUND1_60_TO_79(b,c,d,e,a);

	/* Compute the current intermediate hash value */
	context->s1.state[0] += a;
	context->s1.state[1] += b;
	context->s1.state[2] += c;
	context->s1.state[3] += d;
	context->s1.state[4] += e;

	/* Clean up */
	a = b = c = d = e = T1 = 0;
}

#else  /* SHA2_UNROLL_TRANSFORM */

void SHA1_Internal_Transform(SHA_CTX* context, const sha_word32* data) {
	sha_word32	a, b, c, d, e;
	sha_word32	T1, *W1;
	int		j;

	W1 = (sha_word32*)context->s1.buffer;

	/* Initialize registers with the prev. intermediate value */
	a = context->s1.state[0];
	b = context->s1.state[1];
	c = context->s1.state[2];
	d = context->s1.state[3];
	e = context->s1.state[4];
	j = 0;
	do {
#if BYTE_ORDER == LITTLE_ENDIAN
		T1 = data[j];
		/* Copy data while converting to host byte order */
		REVERSE32(*data++, W1[j]);
		T1 = ROTL32(5, a) + Ch(b, c, d) + e + K1_0_TO_19 + W1[j];
#else /* BYTE_ORDER == LITTLE_ENDIAN */
		T1 = ROTL32(5, a) + Ch(b, c, d) + e + K1_0_TO_19 + (W1[j] = *data++);
#endif /* BYTE_ORDER == LITTLE_ENDIAN */
		e = d;
		d = c;
		c = ROTL32(30, b);
		b = a;
		a = T1;
		j++;
	} while (j < 16);

	do {
		T1 = W1[(j+13)&0x0f] ^ W1[(j+8)&0x0f] ^ W1[(j+2)&0x0f] ^ W1[j&0x0f];
		T1 = ROTL32(5, a) + Ch(b,c,d) + e + K1_0_TO_19 + (W1[j&0x0f] = ROTL32(1, T1));
		e = d;
		d = c;
		c = ROTL32(30, b);
		b = a;
		a = T1;
		j++;
	} while (j < 20);

	do {
		T1 = W1[(j+13)&0x0f] ^ W1[(j+8)&0x0f] ^ W1[(j+2)&0x0f] ^ W1[j&0x0f];
		T1 = ROTL32(5, a) + Parity(b,c,d) + e + K1_20_TO_39 + (W1[j&0x0f] = ROTL32(1, T1));
		e = d;
		d = c;
		c = ROTL32(30, b);
		b = a;
		a = T1;
		j++;
	} while (j < 40);

	do {
		T1 = W1[(j+13)&0x0f] ^ W1[(j+8)&0x0f] ^ W1[(j+2)&0x0f] ^ W1[j&0x0f];
		T1 = ROTL32(5, a) + Maj(b,c,d) + e + K1_40_TO_59 + (W1[j&0x0f] = ROTL32(1, T1));
		e = d;
		d = c;
		c = ROTL32(30, b);
		b = a;
		a = T1;
		j++;
	} while (j < 60);

	do {
		T1 = W1[(j+13)&0x0f] ^ W1[(j+8)&0x0f] ^ W1[(j+2)&0x0f] ^ W1[j&0x0f];
		T1 = ROTL32(5, a) + Parity(b,c,d) + e + K1_60_TO_79 + (W1[j&0x0f] = ROTL32(1, T1));
		e = d;
		d = c;
		c = ROTL32(30, b);
		b = a;
		a = T1;
		j++;
	} while (j < 80);


	/* Compute the current intermediate hash value */
	context->s1.state[0] += a;
	context->s1.state[1] += b;
	context->s1.state[2] += c;
	context->s1.state[3] += d;
	context->s1.state[4] += e;

	/* Clean up */
	a = b = c = d = e = T1 = 0;
}

#endif /* SHA2_UNROLL_TRANSFORM */

void SHA1_Update(SHA_CTX* context, const sha_byte *data, size_t len) {
	unsigned int	freespace, usedspace;
	if (len == 0) {
		/* Calling with no data is valid - we do nothing */
		return;
	}

	/* Sanity check: */
	assert(context != (SHA_CTX*)0 && data != (sha_byte*)0);

	usedspace = (unsigned int)((context->s1.bitcount >> 3) % 64);
	if (usedspace > 0) {
		/* Calculate how much free space is available in the buffer */
		freespace = 64 - usedspace;

		if (len >= freespace) {
			/* Fill the buffer completely and process it */
			MEMCPY_BCOPY(&context->s1.buffer[usedspace], data, freespace);
			context->s1.bitcount += freespace << 3;
			len -= freespace;
			data += freespace;
			SHA1_Internal_Transform(context, (const sha_word32*)context->s1.buffer);
		} else {
			/* The buffer is not yet full */
			MEMCPY_BCOPY(&context->s1.buffer[usedspace], data, len);
			context->s1.bitcount += len << 3;
			/* Clean up: */
			usedspace = freespace = 0;
			return;
		}
	}
	while (len >= 64) {
		/* Process as many complete blocks as we can */
		SHA1_Internal_Transform(context, (const sha_word32*)data);
		context->s1.bitcount += 512;
		len -= 64;
		data += 64;
	}
	if (len > 0) {
		/* There's left-overs, so save 'em */
		MEMCPY_BCOPY(context->s1.buffer, data, len);
		context->s1.bitcount += len << 3;
	}
	/* Clean up: */
	usedspace = freespace = 0;
}

void SHA1_Final(sha_byte digest[], SHA_CTX* context) {
	sha_word32	*d = (sha_word32*)digest;
	unsigned int	usedspace;

	/* Sanity check: */
	assert(context != (SHA_CTX*)0);

	if (digest == (sha_byte*)0) {
		/*
		 * No digest buffer, so we can do nothing
		 * except clean up and go home
		 */
		MEMSET_BZERO(context, sizeof(*context));
		return;
	}

	usedspace = (unsigned int)((context->s1.bitcount >> 3) % 64);
	if (usedspace == 0) {
		/* Set-up for the last transform: */
		MEMSET_BZERO(context->s1.buffer, 56);

		/* Begin padding with a 1 bit: */
		*context->s1.buffer = 0x80;
	} else {
		/* Begin padding with a 1 bit: */
		context->s1.buffer[usedspace++] = 0x80;

		if (usedspace <= 56) {
			/* Set-up for the last transform: */
			MEMSET_BZERO(&context->s1.buffer[usedspace], 56 - usedspace);
		} else {
			if (usedspace < 64) {
				MEMSET_BZERO(&context->s1.buffer[usedspace], 64 - usedspace);
			}
			/* Do second-to-last transform: */
			SHA1_Internal_Transform(context, (const sha_word32*)context->s1.buffer);

			/* And set-up for the last transform: */
			MEMSET_BZERO(context->s1.buffer, 56);
		}
		/* Clean up: */
		usedspace = 0;
	}
	/* Set the bit count: */
#if BYTE_ORDER == LITTLE_ENDIAN
	/* Convert FROM host byte order */
	REVERSE64(context->s1.bitcount,context->s1.bitcount);
#endif
	MEMCPY_BCOPY(&context->s1.buffer[56], &context->s1.bitcount,
		     sizeof(sha_word64));

	/* Final transform: */
	SHA1_Internal_Transform(context, (const sha_word32*)context->s1.buffer);

	/* Save the hash data for output: */
#if BYTE_ORDER == LITTLE_ENDIAN
	{
		/* Convert TO host byte order */
		int	j;
		for (j = 0; j < (SHA1_DIGEST_LENGTH >> 2); j++) {
			REVERSE32(context->s1.state[j],context->s1.state[j]);
			*d++ = context->s1.state[j];
		}
	}
#else
	MEMCPY_BCOPY(d, context->s1.state, SHA1_DIGEST_LENGTH);
#endif

	/* Clean up: */
	MEMSET_BZERO(context, sizeof(*context));
}

char *SHA1_End(SHA_CTX* context, char buffer[]) {
	sha_byte	digest[SHA1_DIGEST_LENGTH], *d = digest;
	int		i;

	/* Sanity check: */
	assert(context != (SHA_CTX*)0);

	if (buffer != (char*)0) {
		SHA1_Final(digest, context);

		for (i = 0; i < SHA1_DIGEST_LENGTH; i++) {
			*buffer++ = sha_hex_digits[(*d & 0xf0) >> 4];
			*buffer++ = sha_hex_digits[*d & 0x0f];
			d++;
		}
		*buffer = (char)0;
	} else {
		MEMSET_BZERO(context, sizeof(*context));
	}
	MEMSET_BZERO(digest, SHA1_DIGEST_LENGTH);
	return buffer;
}

char* SHA1_Data(const sha_byte* data, size_t len, char digest[SHA1_DIGEST_STRING_LENGTH]) {
	SHA_CTX	context;

	SHA1_Init(&context);
	SHA1_Update(&context, data, len);
	return SHA1_End(&context, digest);
}


/*** SHA-256: *********************************************************/
void SHA256_Internal_Init(SHA_CTX* context, const sha_word32* ihv) {
	/* Sanity check: */
	assert(context != (SHA_CTX*)0);

	MEMCPY_BCOPY(context->s256.state, ihv, sizeof(sha_word32) * 8);
	MEMSET_BZERO(context->s256.buffer, 64);
	context->s256.bitcount = 0;
}

void SHA256_Init(SHA_CTX* context) {
	SHA256_Internal_Init(context, sha256_initial_hash_value);
}

#ifdef SHA2_UNROLL_TRANSFORM

/* Unrolled SHA-256 round macros: */

#if BYTE_ORDER == LITTLE_ENDIAN

#define ROUND256_0_TO_15(a,b,c,d,e,f,g,h)	\
	REVERSE32(*data++, W256[j]); \
	T1 = (h) + Sigma1_256(e) + Ch((e), (f), (g)) + \
	     K256[j] + W256[j]; \
	(d) += T1; \
	(h) = T1 + Sigma0_256(a) + Maj((a), (b), (c)); \
	j++


#else /* BYTE_ORDER == LITTLE_ENDIAN */

#define ROUND256_0_TO_15(a,b,c,d,e,f,g,h)	\
	T1 = (h) + Sigma1_256(e) + Ch((e), (f), (g)) + \
	     K256[j] + (W256[j] = *data++); \
	(d) += T1; \
	(h) = T1 + Sigma0_256(a) + Maj((a), (b), (c)); \
	j++

#endif /* BYTE_ORDER == LITTLE_ENDIAN */

#define ROUND256(a,b,c,d,e,f,g,h)	\
	s0 = W256[(j+1)&0x0f]; \
	s0 = sigma0_256(s0); \
	s1 = W256[(j+14)&0x0f]; \
	s1 = sigma1_256(s1); \
	T1 = (h) + Sigma1_256(e) + Ch((e), (f), (g)) + K256[j] + \
	     (W256[j&0x0f] += s1 + W256[(j+9)&0x0f] + s0); \
	(d) += T1; \
	(h) = T1 + Sigma0_256(a) + Maj((a), (b), (c)); \
	j++

void SHA256_Internal_Transform(SHA_CTX* context, const sha_word32* data) {
	sha_word32	a, b, c, d, e, f, g, h, s0, s1;
	sha_word32	T1, *W256;
	int		j;

	W256 = (sha_word32*)context->s256.buffer;

	/* Initialize registers with the prev. intermediate value */
	a = context->s256.state[0];
	b = context->s256.state[1];
	c = context->s256.state[2];
	d = context->s256.state[3];
	e = context->s256.state[4];
	f = context->s256.state[5];
	g = context->s256.state[6];
	h = context->s256.state[7];

	j = 0;
	do {
		/* Rounds 0 to 15 (unrolled): */
		ROUND256_0_TO_15(a,b,c,d,e,f,g,h);
		ROUND256_0_TO_15(h,a,b,c,d,e,f,g);
		ROUND256_0_TO_15(g,h,a,b,c,d,e,f);
		ROUND256_0_TO_15(f,g,h,a,b,c,d,e);
		ROUND256_0_TO_15(e,f,g,h,a,b,c,d);
		ROUND256_0_TO_15(d,e,f,g,h,a,b,c);
		ROUND256_0_TO_15(c,d,e,f,g,h,a,b);
		ROUND256_0_TO_15(b,c,d,e,f,g,h,a);
	} while (j < 16);

	/* Now for the remaining rounds to 64: */
	do {
		ROUND256(a,b,c,d,e,f,g,h);
		ROUND256(h,a,b,c,d,e,f,g);
		ROUND256(g,h,a,b,c,d,e,f);
		ROUND256(f,g,h,a,b,c,d,e);
		ROUND256(e,f,g,h,a,b,c,d);
		ROUND256(d,e,f,g,h,a,b,c);
		ROUND256(c,d,e,f,g,h,a,b);
		ROUND256(b,c,d,e,f,g,h,a);
	} while (j < 64);

	/* Compute the current intermediate hash value */
	context->s256.state[0] += a;
	context->s256.state[1] += b;
	context->s256.state[2] += c;
	context->s256.state[3] += d;
	context->s256.state[4] += e;
	context->s256.state[5] += f;
	context->s256.state[6] += g;
	context->s256.state[7] += h;

	/* Clean up */
	a = b = c = d = e = f = g = h = T1 = 0;
}

#else /* SHA2_UNROLL_TRANSFORM */

void SHA256_Internal_Transform(SHA_CTX* context, const sha_word32* data) {
	sha_word32	a, b, c, d, e, f, g, h, s0, s1;
	sha_word32	T1, T2, *W256;
	int		j;

	W256 = (sha_word32*)context->s256.buffer;

	/* Initialize registers with the prev. intermediate value */
	a = context->s256.state[0];
	b = context->s256.state[1];
	c = context->s256.state[2];
	d = context->s256.state[3];
	e = context->s256.state[4];
	f = context->s256.state[5];
	g = context->s256.state[6];
	h = context->s256.state[7];

	j = 0;
	do {
#if BYTE_ORDER == LITTLE_ENDIAN
		/* Copy data while converting to host byte order */
		REVERSE32(*data++,W256[j]);
		/* Apply the SHA-256 compression function to update a..h */
		T1 = h + Sigma1_256(e) + Ch(e, f, g) + K256[j] + W256[j];
#else /* BYTE_ORDER == LITTLE_ENDIAN */
		/* Apply the SHA-256 compression function to update a..h with copy */
		T1 = h + Sigma1_256(e) + Ch(e, f, g) + K256[j] + (W256[j] = *data++);
#endif /* BYTE_ORDER == LITTLE_ENDIAN */
		T2 = Sigma0_256(a) + Maj(a, b, c);
		h = g;
		g = f;
		f = e;
		e = d + T1;
		d = c;
		c = b;
		b = a;
		a = T1 + T2;

		j++;
	} while (j < 16);

	do {
		/* Part of the message block expansion: */
		s0 = W256[(j+1)&0x0f];
		s0 = sigma0_256(s0);
		s1 = W256[(j+14)&0x0f];
		s1 = sigma1_256(s1);

		/* Apply the SHA-256 compression function to update a..h */
		T1 = h + Sigma1_256(e) + Ch(e, f, g) + K256[j] +
		     (W256[j&0x0f] += s1 + W256[(j+9)&0x0f] + s0);
		T2 = Sigma0_256(a) + Maj(a, b, c);
		h = g;
		g = f;
		f = e;
		e = d + T1;
		d = c;
		c = b;
		b = a;
		a = T1 + T2;

		j++;
	} while (j < 64);

	/* Compute the current intermediate hash value */
	context->s256.state[0] += a;
	context->s256.state[1] += b;
	context->s256.state[2] += c;
	context->s256.state[3] += d;
	context->s256.state[4] += e;
	context->s256.state[5] += f;
	context->s256.state[6] += g;
	context->s256.state[7] += h;

	/* Clean up */
	a = b = c = d = e = f = g = h = T1 = T2 = 0;
}

#endif /* SHA2_UNROLL_TRANSFORM */

void SHA256_Update(SHA_CTX* context, const sha_byte *data, size_t len) {
	unsigned int	freespace, usedspace;

	if (len == 0) {
		/* Calling with no data is valid - we do nothing */
		return;
	}

	/* Sanity check: */
	assert(context != (SHA_CTX*)0 && data != (sha_byte*)0);

	usedspace = (unsigned int)((context->s256.bitcount >> 3) % 64);
	if (usedspace > 0) {
		/* Calculate how much free space is available in the buffer */
		freespace = 64 - usedspace;

		if (len >= freespace) {
			/* Fill the buffer completely and process it */
			MEMCPY_BCOPY(&context->s256.buffer[usedspace], data, freespace);
			context->s256.bitcount += freespace << 3;
			len -= freespace;
			data += freespace;
			SHA256_Internal_Transform(context, (const sha_word32*)context->s256.buffer);
		} else {
			/* The buffer is not yet full */
			MEMCPY_BCOPY(&context->s256.buffer[usedspace], data, len);
			context->s256.bitcount += len << 3;
			/* Clean up: */
			usedspace = freespace = 0;
			return;
		}
	}
	while (len >= 64) {
		/* Process as many complete blocks as we can */
		SHA256_Internal_Transform(context, (const sha_word32*)data);
		context->s256.bitcount += 512;
		len -= 64;
		data += 64;
	}
	if (len > 0) {
		/* There's left-overs, so save 'em */
		MEMCPY_BCOPY(context->s256.buffer, data, len);
		context->s256.bitcount += len << 3;
	}
	/* Clean up: */
	usedspace = freespace = 0;
}

void SHA256_Internal_Last(SHA_CTX* context) {
	unsigned int	usedspace;

	usedspace = (unsigned int)((context->s256.bitcount >> 3) % 64);
#if BYTE_ORDER == LITTLE_ENDIAN
	/* Convert FROM host byte order */
	REVERSE64(context->s256.bitcount,context->s256.bitcount);
#endif
	if (usedspace > 0) {
		/* Begin padding with a 1 bit: */
		context->s256.buffer[usedspace++] = 0x80;

		if (usedspace <= 56) {
			/* Set-up for the last transform: */
			MEMSET_BZERO(&context->s256.buffer[usedspace], 56 - usedspace);
		} else {
			if (usedspace < 64) {
				MEMSET_BZERO(&context->s256.buffer[usedspace], 64 - usedspace);
			}
			/* Do second-to-last transform: */
			SHA256_Internal_Transform(context, (const sha_word32*)context->s256.buffer);

			/* And set-up for the last transform: */
			MEMSET_BZERO(context->s256.buffer, 56);
		}
		/* Clean up: */
		usedspace = 0;
	} else {
		/* Set-up for the last transform: */
		MEMSET_BZERO(context->s256.buffer, 56);

		/* Begin padding with a 1 bit: */
		*context->s256.buffer = 0x80;
	}
	/* Set the bit count: */
	MEMCPY_BCOPY(&context->s256.buffer[56], &context->s256.bitcount,
		     sizeof(sha_word64));

	/* Final transform: */
	SHA256_Internal_Transform(context, (const sha_word32*)context->s256.buffer);
}

void SHA256_Final(sha_byte digest[], SHA_CTX* context) {
	sha_word32	*d = (sha_word32*)digest;

	/* Sanity check: */
	assert(context != (SHA_CTX*)0);

	/* If no digest buffer is passed, we don't bother doing this: */
	if (digest != (sha_byte*)0) {
		SHA256_Internal_Last(context);

		/* Save the hash data for output: */
#if BYTE_ORDER == LITTLE_ENDIAN
		{
			/* Convert TO host byte order */
			int	j;
			for (j = 0; j < (SHA256_DIGEST_LENGTH >> 2); j++) {
				REVERSE32(context->s256.state[j],context->s256.state[j]);
				*d++ = context->s256.state[j];
			}
		}
#else
		MEMCPY_BCOPY(d, context->s256.state, SHA256_DIGEST_LENGTH);
#endif
	}

	/* Clean up state data: */
	MEMSET_BZERO(context, sizeof(*context));
}

char *SHA256_End(SHA_CTX* context, char buffer[]) {
	sha_byte	digest[SHA256_DIGEST_LENGTH], *d = digest;
	int		i;

	/* Sanity check: */
	assert(context != (SHA_CTX*)0);

	if (buffer != (char*)0) {
		SHA256_Final(digest, context);

		for (i = 0; i < SHA256_DIGEST_LENGTH; i++) {
			*buffer++ = sha_hex_digits[(*d & 0xf0) >> 4];
			*buffer++ = sha_hex_digits[*d & 0x0f];
			d++;
		}
		*buffer = (char)0;
	} else {
		MEMSET_BZERO(context, sizeof(*context));
	}
	MEMSET_BZERO(digest, SHA256_DIGEST_LENGTH);
	return buffer;
}

char* SHA256_Data(const sha_byte* data, size_t len, char digest[SHA256_DIGEST_STRING_LENGTH]) {
	SHA_CTX	context;

	SHA256_Init(&context);
	SHA256_Update(&context, data, len);
	return SHA256_End(&context, digest);
}


/*** SHA-224: *********************************************************/
void SHA224_Init(SHA_CTX* context) {
	SHA256_Internal_Init(context, sha224_initial_hash_value);
}

void SHA224_Internal_Transform(SHA_CTX* context, const sha_word32* data) {
	SHA256_Internal_Transform(context, data);
}

void SHA224_Update(SHA_CTX* context, const sha_byte *data, size_t len) {
	SHA256_Update(context, data, len);
}

void SHA224_Final(sha_byte digest[], SHA_CTX* context) {
	sha_word32	*d = (sha_word32*)digest;

	/* Sanity check: */
	assert(context != (SHA_CTX*)0);

	/* If no digest buffer is passed, we don't bother doing this: */
	if (digest != (sha_byte*)0) {
		SHA256_Internal_Last(context);

		/* Save the hash data for output: */
#if BYTE_ORDER == LITTLE_ENDIAN
		{
			/* Convert TO host byte order */
			int	j;
			for (j = 0; j < (SHA224_DIGEST_LENGTH >> 2); j++) {
				REVERSE32(context->s256.state[j],context->s256.state[j]);
				*d++ = context->s256.state[j];
			}
		}
#else
		MEMCPY_BCOPY(d, context->s256.state, SHA224_DIGEST_LENGTH);
#endif
	}

	/* Clean up state data: */
	MEMSET_BZERO(context, sizeof(*context));
}

char *SHA224_End(SHA_CTX* context, char buffer[]) {
	sha_byte	digest[SHA224_DIGEST_LENGTH], *d = digest;
	int		i;

	/* Sanity check: */
	assert(context != (SHA_CTX*)0);

	if (buffer != (char*)0) {
		SHA224_Final(digest, context);

		for (i = 0; i < SHA224_DIGEST_LENGTH; i++) {
			*buffer++ = sha_hex_digits[(*d & 0xf0) >> 4];
			*buffer++ = sha_hex_digits[*d & 0x0f];
			d++;
		}
		*buffer = (char)0;
	} else {
		MEMSET_BZERO(context, sizeof(*context));
	}
	MEMSET_BZERO(digest, SHA224_DIGEST_LENGTH);
	return buffer;
}

char* SHA224_Data(const sha_byte* data, size_t len, char digest[SHA224_DIGEST_STRING_LENGTH]) {
	SHA_CTX	context;

	SHA224_Init(&context);
	SHA224_Update(&context, data, len);
	return SHA224_End(&context, digest);
}


/*** SHA-512: *********************************************************/
void SHA512_Internal_Init(SHA_CTX* context, const sha_word64* ihv) {
	/* Sanity check: */
	assert(context != (SHA_CTX*)0);

	MEMCPY_BCOPY(context->s512.state, ihv, sizeof(sha_word64) * 8);
	MEMSET_BZERO(context->s512.buffer, 128);
	context->s512.bitcount[0] = context->s512.bitcount[1] =  0;
}

void SHA512_Init(SHA_CTX* context) {
	SHA512_Internal_Init(context, sha512_initial_hash_value);
}

#ifdef SHA2_UNROLL_TRANSFORM

/* Unrolled SHA-512 round macros: */
#if BYTE_ORDER == LITTLE_ENDIAN

#define ROUND512_0_TO_15(a,b,c,d,e,f,g,h)	\
	REVERSE64(*data++, W512[j]); \
	T1 = (h) + Sigma1_512(e) + Ch((e), (f), (g)) + \
	     K512[j] + W512[j]; \
	(d) += T1, \
	(h) = T1 + Sigma0_512(a) + Maj((a), (b), (c)), \
	j++


#else /* BYTE_ORDER == LITTLE_ENDIAN */

#define ROUND512_0_TO_15(a,b,c,d,e,f,g,h)	\
	T1 = (h) + Sigma1_512(e) + Ch((e), (f), (g)) + \
	     K512[j] + (W512[j] = *data++); \
	(d) += T1; \
	(h) = T1 + Sigma0_512(a) + Maj((a), (b), (c)); \
	j++

#endif /* BYTE_ORDER == LITTLE_ENDIAN */

#define ROUND512(a,b,c,d,e,f,g,h)	\
	s0 = W512[(j+1)&0x0f]; \
	s0 = sigma0_512(s0); \
	s1 = W512[(j+14)&0x0f]; \
	s1 = sigma1_512(s1); \
	T1 = (h) + Sigma1_512(e) + Ch((e), (f), (g)) + K512[j] + \
	     (W512[j&0x0f] += s1 + W512[(j+9)&0x0f] + s0); \
	(d) += T1; \
	(h) = T1 + Sigma0_512(a) + Maj((a), (b), (c)); \
	j++

void SHA512_Internal_Transform(SHA_CTX* context, const sha_word64* data) {
	sha_word64	a, b, c, d, e, f, g, h, s0, s1;
	sha_word64	T1, *W512 = (sha_word64*)context->s512.buffer;
	int		j;

	/* Initialize registers with the prev. intermediate value */
	a = context->s512.state[0];
	b = context->s512.state[1];
	c = context->s512.state[2];
	d = context->s512.state[3];
	e = context->s512.state[4];
	f = context->s512.state[5];
	g = context->s512.state[6];
	h = context->s512.state[7];

	j = 0;
	do {
		ROUND512_0_TO_15(a,b,c,d,e,f,g,h);
		ROUND512_0_TO_15(h,a,b,c,d,e,f,g);
		ROUND512_0_TO_15(g,h,a,b,c,d,e,f);
		ROUND512_0_TO_15(f,g,h,a,b,c,d,e);
		ROUND512_0_TO_15(e,f,g,h,a,b,c,d);
		ROUND512_0_TO_15(d,e,f,g,h,a,b,c);
		ROUND512_0_TO_15(c,d,e,f,g,h,a,b);
		ROUND512_0_TO_15(b,c,d,e,f,g,h,a);
	} while (j < 16);

	/* Now for the remaining rounds up to 79: */
	do {
		ROUND512(a,b,c,d,e,f,g,h);
		ROUND512(h,a,b,c,d,e,f,g);
		ROUND512(g,h,a,b,c,d,e,f);
		ROUND512(f,g,h,a,b,c,d,e);
		ROUND512(e,f,g,h,a,b,c,d);
		ROUND512(d,e,f,g,h,a,b,c);
		ROUND512(c,d,e,f,g,h,a,b);
		ROUND512(b,c,d,e,f,g,h,a);
	} while (j < 80);

	/* Compute the current intermediate hash value */
	context->s512.state[0] += a;
	context->s512.state[1] += b;
	context->s512.state[2] += c;
	context->s512.state[3] += d;
	context->s512.state[4] += e;
	context->s512.state[5] += f;
	context->s512.state[6] += g;
	context->s512.state[7] += h;

	/* Clean up */
	a = b = c = d = e = f = g = h = T1 = 0;
}

#else /* SHA2_UNROLL_TRANSFORM */

void SHA512_Internal_Transform(SHA_CTX* context, const sha_word64* data) {
	sha_word64	a, b, c, d, e, f, g, h, s0, s1;
	sha_word64	T1, T2, *W512 = (sha_word64*)context->s512.buffer;
	int		j;

	/* Initialize registers with the prev. intermediate value */
	a = context->s512.state[0];
	b = context->s512.state[1];
	c = context->s512.state[2];
	d = context->s512.state[3];
	e = context->s512.state[4];
	f = context->s512.state[5];
	g = context->s512.state[6];
	h = context->s512.state[7];

	j = 0;
	do {
#if BYTE_ORDER == LITTLE_ENDIAN
		/* Convert TO host byte order */
		REVERSE64(*data++, W512[j]);
		/* Apply the SHA-512 compression function to update a..h */
		T1 = h + Sigma1_512(e) + Ch(e, f, g) + K512[j] + W512[j];
#else /* BYTE_ORDER == LITTLE_ENDIAN */
		/* Apply the SHA-512 compression function to update a..h with copy */
		T1 = h + Sigma1_512(e) + Ch(e, f, g) + K512[j] + (W512[j] = *data++);
#endif /* BYTE_ORDER == LITTLE_ENDIAN */
		T2 = Sigma0_512(a) + Maj(a, b, c);
		h = g;
		g = f;
		f = e;
		e = d + T1;
		d = c;
		c = b;
		b = a;
		a = T1 + T2;

		j++;
	} while (j < 16);

	do {
		/* Part of the message block expansion: */
		s0 = W512[(j+1)&0x0f];
		s0 = sigma0_512(s0);
		s1 = W512[(j+14)&0x0f];
		s1 =  sigma1_512(s1);

		/* Apply the SHA-512 compression function to update a..h */
		T1 = h + Sigma1_512(e) + Ch(e, f, g) + K512[j] +
		     (W512[j&0x0f] += s1 + W512[(j+9)&0x0f] + s0);
		T2 = Sigma0_512(a) + Maj(a, b, c);
		h = g;
		g = f;
		f = e;
		e = d + T1;
		d = c;
		c = b;
		b = a;
		a = T1 + T2;

		j++;
	} while (j < 80);

	/* Compute the current intermediate hash value */
	context->s512.state[0] += a;
	context->s512.state[1] += b;
	context->s512.state[2] += c;
	context->s512.state[3] += d;
	context->s512.state[4] += e;
	context->s512.state[5] += f;
	context->s512.state[6] += g;
	context->s512.state[7] += h;

	/* Clean up */
	a = b = c = d = e = f = g = h = T1 = T2 = 0;
}

#endif /* SHA2_UNROLL_TRANSFORM */

void SHA512_Update(SHA_CTX* context, const sha_byte *data, size_t len) {
	unsigned int	freespace, usedspace;

	if (len == 0) {
		/* Calling with no data is valid - we do nothing */
		return;
	}

	/* Sanity check: */
	assert(context != (SHA_CTX*)0 && data != (sha_byte*)0);

	usedspace = (unsigned int)((context->s512.bitcount[0] >> 3) % 128);
	if (usedspace > 0) {
		/* Calculate how much free space is available in the buffer */
		freespace = 128 - usedspace;

		if (len >= freespace) {
			/* Fill the buffer completely and process it */
			MEMCPY_BCOPY(&context->s512.buffer[usedspace], data, freespace);
			ADDINC128(context->s512.bitcount, freespace << 3);
			len -= freespace;
			data += freespace;
			SHA512_Internal_Transform(context, (const sha_word64*)context->s512.buffer);
		} else {
			/* The buffer is not yet full */
			MEMCPY_BCOPY(&context->s512.buffer[usedspace], data, len);
			ADDINC128(context->s512.bitcount, len << 3);
			/* Clean up: */
			usedspace = freespace = 0;
			return;
		}
	}
	while (len >= 128) {
		/* Process as many complete blocks as we can */
		SHA512_Internal_Transform(context, (const sha_word64*)data);
		ADDINC128(context->s512.bitcount, 1024);
		len -= 128;
		data += 128;
	}
	if (len > 0) {
		/* There's left-overs, so save 'em */
		MEMCPY_BCOPY(context->s512.buffer, data, len);
		ADDINC128(context->s512.bitcount, len << 3);
	}
	/* Clean up: */
	usedspace = freespace = 0;
}

void SHA512_Internal_Last(SHA_CTX* context) {
	unsigned int	usedspace;

	usedspace = (unsigned int)((context->s512.bitcount[0] >> 3) % 128);
#if BYTE_ORDER == LITTLE_ENDIAN
	/* Convert FROM host byte order */
	REVERSE64(context->s512.bitcount[0],context->s512.bitcount[0]);
	REVERSE64(context->s512.bitcount[1],context->s512.bitcount[1]);
#endif
	if (usedspace > 0) {
		/* Begin padding with a 1 bit: */
		context->s512.buffer[usedspace++] = 0x80;

		if (usedspace <= 112) {
			/* Set-up for the last transform: */
			MEMSET_BZERO(&context->s512.buffer[usedspace], 112 - usedspace);
		} else {
			if (usedspace < 128) {
				MEMSET_BZERO(&context->s512.buffer[usedspace], 128 - usedspace);
			}
			/* Do second-to-last transform: */
			SHA512_Internal_Transform(context, (const sha_word64*)context->s512.buffer);

			/* And set-up for the last transform: */
			MEMSET_BZERO(context->s512.buffer, 112);
		}
		/* Clean up: */
		usedspace = 0;
	} else {
		/* Prepare for final transform: */
		MEMSET_BZERO(context->s512.buffer, 112);

		/* Begin padding with a 1 bit: */
		*context->s512.buffer = 0x80;
	}
	/* Store the length of input data (in bits): */
	MEMCPY_BCOPY(&context->s512.buffer[112], &context->s512.bitcount[1],
		     sizeof(sha_word64));
	MEMCPY_BCOPY(&context->s512.buffer[120], &context->s512.bitcount[0],
		     sizeof(sha_word64));

	/* Final transform: */
	SHA512_Internal_Transform(context, (const sha_word64*)context->s512.buffer);
}

void SHA512_Final(sha_byte digest[], SHA_CTX* context) {
	sha_word64	*d = (sha_word64*)digest;

	/* Sanity check: */
	assert(context != (SHA_CTX*)0);

	/* If no digest buffer is passed, we don't bother doing this: */
	if (digest != (sha_byte*)0) {
		SHA512_Internal_Last(context);

		/* Save the hash data for output: */
#if BYTE_ORDER == LITTLE_ENDIAN
		{
			/* Convert TO host byte order */
			int	j;
			for (j = 0; j < (SHA512_DIGEST_LENGTH >> 3); j++) {
				REVERSE64(context->s512.state[j],context->s512.state[j]);
				*d++ = context->s512.state[j];
			}
		}
#else
		MEMCPY_BCOPY(d, context->s512.state, SHA512_DIGEST_LENGTH);
#endif
	}

	/* Zero out state data */
	MEMSET_BZERO(context, sizeof(*context));
}

char *SHA512_End(SHA_CTX* context, char buffer[]) {
	sha_byte	digest[SHA512_DIGEST_LENGTH], *d = digest;
	int		i;

	/* Sanity check: */
	assert(context != (SHA_CTX*)0);

	if (buffer != (char*)0) {
		SHA512_Final(digest, context);

		for (i = 0; i < SHA512_DIGEST_LENGTH; i++) {
			*buffer++ = sha_hex_digits[(*d & 0xf0) >> 4];
			*buffer++ = sha_hex_digits[*d & 0x0f];
			d++;
		}
		*buffer = (char)0;
	} else {
		MEMSET_BZERO(context, sizeof(*context));
	}
	MEMSET_BZERO(digest, SHA512_DIGEST_LENGTH);
	return buffer;
}

char* SHA512_Data(const sha_byte* data, size_t len, char digest[SHA512_DIGEST_STRING_LENGTH]) {
	SHA_CTX	context;

	SHA512_Init(&context);
	SHA512_Update(&context, data, len);
	return SHA512_End(&context, digest);
}


/*** SHA-384: *********************************************************/
void SHA384_Init(SHA_CTX* context) {
	SHA512_Internal_Init(context, sha384_initial_hash_value);
}

void SHA384_Update(SHA_CTX* context, const sha_byte* data, size_t len) {
	SHA512_Update(context, data, len);
}

void SHA384_Final(sha_byte digest[], SHA_CTX* context) {
	sha_word64	*d = (sha_word64*)digest;

	/* Sanity check: */
	assert(context != (SHA_CTX*)0);

	/* If no digest buffer is passed, we don't bother doing this: */
	if (digest != (sha_byte*)0) {
		SHA512_Internal_Last(context);

		/* Save the hash data for output: */
#if BYTE_ORDER == LITTLE_ENDIAN
		{
			/* Convert TO host byte order */
			int	j;
			for (j = 0; j < (SHA384_DIGEST_LENGTH >> 3); j++) {
				REVERSE64(context->s512.state[j],context->s512.state[j]);
				*d++ = context->s512.state[j];
			}
		}
#else
		MEMCPY_BCOPY(d, context->s512.state, SHA384_DIGEST_LENGTH);
#endif
	}

	/* Zero out state data */
	MEMSET_BZERO(context, sizeof(*context));
}

char *SHA384_End(SHA_CTX* context, char buffer[]) {
	sha_byte	digest[SHA384_DIGEST_LENGTH], *d = digest;
	int		i;

	/* Sanity check: */
	assert(context != (SHA_CTX*)0);

	if (buffer != (char*)0) {
		SHA384_Final(digest, context);

		for (i = 0; i < SHA384_DIGEST_LENGTH; i++) {
			*buffer++ = sha_hex_digits[(*d & 0xf0) >> 4];
			*buffer++ = sha_hex_digits[*d & 0x0f];
			d++;
		}
		*buffer = (char)0;
	} else {
		MEMSET_BZERO(context, sizeof(*context));
	}
	MEMSET_BZERO(digest, SHA384_DIGEST_LENGTH);
	return buffer;
}

char* SHA384_Data(const sha_byte* data, size_t len, char digest[SHA384_DIGEST_STRING_LENGTH]) {
	SHA_CTX	context;

	SHA384_Init(&context);
	SHA384_Update(&context, data, len);
	return SHA384_End(&context, digest);
}
