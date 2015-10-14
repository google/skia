#include <assert.h>
#include <stdlib.h>
#include <string.h>

#define SAT(x) (((x) < -32768) ? -32768 : (((x) > 32767) ? 32767 : (x)))

static void test_pabsb_c(char *pDst, char *pSrc, int xmm)
{
   int i;

   for ( i = 0; i < (8 << xmm); i++ )
      pDst[ i ] = pSrc[ i ] > 0 ? pSrc[i ] : -pSrc[ i ];
}

static void test_pabsw_c(short *pDst, short *pSrc, int xmm)
{
   int i;

   for ( i = 0; i < (4 << xmm); i++ )
      pDst[ i ] = pSrc[ i ] > 0 ? pSrc[i ] : -pSrc[ i ];
}

static void test_pabsd_c(int *pDst, int *pSrc, int xmm)
{
   int i;

   for ( i = 0; i < (2 << xmm); i++ )
      pDst[ i ] = pSrc[ i ] > 0 ? pSrc[i ] : -pSrc[ i ];
}

static void test_psignb_c(char *pDst, char *pSrc, int xmm)
{
   int i;

   for ( i = 0; i < (8 << xmm); i++ )
      pDst[ i ] = pSrc[i] ? ( pSrc[ i ] >= 0 ? pDst[i ] : -pDst[ i ] ) : 0;
}

static void test_psignw_c(short *pDst, short *pSrc, int xmm)
{
   int i;

   for ( i = 0; i < (4 << xmm); i++ )
      pDst[ i ] = pSrc[i] ? ( pSrc[ i ] >= 0 ? pDst[i ] : -pDst[ i ] ) : 0;
}

static void test_psignd_c(int *pDst, int *pSrc, int xmm)
{
   int i;

   for ( i = 0; i < (2 << xmm); i++ )
      pDst[ i ] = pSrc[i] ? ( pSrc[ i ] >= 0 ? pDst[i ] : -pDst[ i ] ) : 0;
}

static void test_phaddw_c(unsigned short *pDst,unsigned short *pSrc, int xmm)
{
   int i;

   for ( i = 0; i < (2 << xmm); i++ )
      pDst[ i ] = pDst[ i * 2 ] + pDst[ i * 2 + 1 ];

   for ( i = 0; i < (2 << xmm); i++ )
      pDst[ i + (2 << xmm) ] = pSrc[ i * 2 ] + pSrc[ i * 2 + 1 ];
}

static void test_phaddsw_c(short *pDst, short *pSrc, int xmm)
{
   int i;

   for ( i = 0; i < (2 << xmm); i++ )
      pDst[ i ] = SAT( pDst[ i * 2 ] + pDst[ i * 2 + 1 ] );

   for ( i = 0; i < (2 << xmm); i++ )
      pDst[ i + (2 << xmm) ] = SAT( pSrc[ i * 2 ] + pSrc[ i * 2 + 1 ] );
}

static void test_phaddd_c(unsigned int *pDst, unsigned int *pSrc, int xmm)
{
   int i;

   for ( i = 0; i < (1 << xmm); i++ )
      pDst[ i ] = pDst[ i * 2 ] + pDst[ i * 2 + 1 ];

   for ( i = 0; i < (1 << xmm); i++ )
      pDst[ i + (1 << xmm) ] = pSrc[ i * 2 ] + pSrc[ i * 2 + 1 ];
}

static void test_phsubw_c(unsigned short *pDst,unsigned short *pSrc, int xmm)
{
   int i;

   for ( i = 0; i < (2 << xmm); i++ )
      pDst[ i ] = pDst[ i * 2 ] - pDst[ i * 2 + 1 ];

   for ( i = 0; i < (2 << xmm); i++ )
      pDst[ i + (2 << xmm) ] = pSrc[ i * 2 ] - pSrc[ i * 2 + 1 ];
}

static void test_phsubsw_c(short *pDst, short *pSrc, int xmm)
{
   int i;

   for ( i = 0; i < (2 << xmm); i++ )
      pDst[ i ] = SAT( pDst[ i * 2 ] - pDst[ i * 2 + 1 ] );

   for ( i = 0; i < (2 << xmm); i++ )
      pDst[ i + (2 << xmm) ] = SAT( pSrc[ i * 2 ] - pSrc[ i * 2 + 1 ] );
}

static void test_phsubd_c(unsigned int *pDst, unsigned int *pSrc, int xmm)
{
   int i;

   for ( i = 0; i < (1 << xmm); i++ )
      pDst[ i ] = pDst[ i * 2 ] - pDst[ i * 2 + 1 ];

   for ( i = 0; i < (1 << xmm); i++ )
      pDst[ i + (1 << xmm) ] = pSrc[ i * 2 ] - pSrc[ i * 2 + 1 ];
}

static void test_pmulhrsw_c(short *pDst, short *pSrc, int xmm)
{
   int i;

   for ( i = 0; i < (4 << xmm); i++ )
   {
      int a = pSrc[ i ] * pDst[ i ];
         pDst[i] = (short)(((a >> 14) + 1) >> 1);
   }
}

static void test_pmaddubsw_c(unsigned char *pDst, signed char *pSrc, int xmm)
{
   int i;

   for ( i = 0; i < (4 << xmm); i++ )
   {
      int a = pSrc[ 2 * i ] * pDst[ 2 * i ] + pSrc[ 2 * i + 1 ] * pDst[ 2 * i + 1];
         ((signed short *)pDst)[i] = SAT(a);
   }
}

static void test_pshufb_c(unsigned char *pDst, unsigned char *pSrc, int xmm)
{
   unsigned char bla[16];
   int i;

   memcpy( bla, pDst, ( 8 << xmm ) );

   for ( i = 0; i < (8 << xmm); i++ )
      pDst[ i ] = (pSrc[ i ] >= 0x80) ? 0 : bla[ pSrc[ i ] & ((1 << (xmm + 3)) - 1) ];
}

static void test_palignr_c(unsigned char *pDst, unsigned char *pSrc, int xmm)
{
   int i;

   for ( i = 0; i < 3; i++ )
      pDst[ i + (8 << xmm) - 3 ] = pDst[ i ];

   for ( i = 3; i < (8 << xmm); i++ )
      pDst[ i - 3 ] = pSrc[ i ];
}

static void randomize_args(unsigned char *pDst, unsigned char *pSrc)
{
   int j; 
   for ( j = 0; j < 16; j++ )
   {
      pDst[ j ] = rand() % 256;
      pSrc[ j ] = rand() % 256;
   }
}

#define CHECK_FUNCTION(instruction, extension, additionnal, pDst, pSrc) \
   do { \
      unsigned char temp_dst[16]; \
      unsigned char temp_src[16]; \
      randomize_args( pDst, pSrc ); \
      memcpy( temp_dst, pDst, 16 ); \
      memcpy( temp_src, pSrc, 16 ); \
      test_##instruction##_c( pDst, pSrc, additionnal ); \
      test_##instruction##_##extension( temp_dst, temp_src ); \
      assert( !memcmp( pDst, temp_dst, (8 << additionnal) ) ); \
   } while( 0 )

#define CHECK_FUNCTIONS(instruction) \
   CHECK_FUNCTION(instruction, mmx, 0, pDst, pSrc); \
   CHECK_FUNCTION(instruction, xmm, 1, pDst, pSrc)

      
void main(int nArgC, char *pArgv[])
{
   void *pSrc = malloc(16);
   void *pDst = malloc(16);
   int nIter = atoi( pArgv[ 1 ] );
   int i;

   for ( i = 0; i < nIter; i++ )
   {
      CHECK_FUNCTIONS( psignb );
      CHECK_FUNCTIONS( psignw );
      CHECK_FUNCTIONS( psignd );

      CHECK_FUNCTIONS( pabsb );
      CHECK_FUNCTIONS( pabsw );
      CHECK_FUNCTIONS( pabsd );

      CHECK_FUNCTIONS( phaddw );
      CHECK_FUNCTIONS( phaddsw );
      CHECK_FUNCTIONS( phaddd );

      CHECK_FUNCTIONS( phsubw );
      CHECK_FUNCTIONS( phsubsw );
      CHECK_FUNCTIONS( phsubd );

      CHECK_FUNCTIONS( pmulhrsw );
         CHECK_FUNCTIONS( pmaddubsw );
      
         CHECK_FUNCTIONS( pshufb );
      CHECK_FUNCTIONS( palignr );
   }
}