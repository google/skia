/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 */

//
//
//

#include "transpose.h"
#include "common/macros.h"

//
// Rows must be an even number.  This is enforced elsewhere.
//
// The transpose requires (cols_log2 * rows/2) row-pair blends.
//
void
hsg_transpose(uint32_t                   const cols_log2,
              uint32_t                   const rows,
              void (*pfn_blend)(uint32_t const cols_log2,
                                uint32_t const row_ll, // lower-left
                                uint32_t const row_ur, // upper-right
                                void *         blend),
              void *                           blend,
              void (*pfn_remap)(uint32_t const row_from,
                                uint32_t const row_to,
                                void *         remap),
              void *                           remap)
{
  // get mapping array
  uint32_t * map_curr = ALLOCA_MACRO(rows * sizeof(*map_curr));
  uint32_t * map_next = ALLOCA_MACRO(rows * sizeof(*map_next));

  // init the mapping array
  for (uint32_t ii=0; ii<rows; ii++)
    map_curr[ii] = ii;

  // successively transpose rows using blends
  for (uint32_t cc=1; cc<=cols_log2; cc++)
    {
      uint32_t const mask = BITS_TO_MASK(cc);

      for (uint32_t ii=0; ii<rows; ii++)
        {
          uint32_t const left = map_curr[ii];
          uint32_t const stay = left & ~mask;

          if (left != stay) // will be swapped away
            {
              for (uint32_t jj=0; jj<rows; jj++)
                {
                  if (map_curr[jj] == stay)
                    {
                      map_next[jj] = stay;
                      map_next[ii] = stay + (rows << (cc-1));

                      pfn_blend(cc,ii,jj,blend); // log2,left,right,payload

                      break;
                    }
                }
            }
        }

      uint32_t * tmp = map_curr;

      map_curr = map_next;
      map_next = tmp;
    }

  // write out the remapping
  for (uint32_t ii=0; ii<rows; ii++)
    pfn_remap(ii,map_curr[ii] >> cols_log2,remap);
}

//
// test it!
//

#ifdef HS_TRANSPOSE_DEBUG

#include <stdio.h>

static uint32_t cols; // implicit on SIMD/GPU

static
void
hsg_debug_blend(uint32_t const cols_log2,
                uint32_t const row_ll, // lower-left
                uint32_t const row_ur, // upper-right
                uint32_t *     b)
{
  fprintf(stdout,"BLEND( %u, %3u, %3u )\n",cols_log2,row_ll,row_ur);

  uint32_t * const ll = ALLOCA(cols * sizeof(*b));
  uint32_t * const ur = ALLOCA(cols * sizeof(*b));

  memcpy(ll,b+row_ll*cols,cols * sizeof(*b));
  memcpy(ur,b+row_ur*cols,cols * sizeof(*b));

  for (uint32_t ii=0; ii<cols; ii++)
    b[row_ll*cols+ii] = ((ii >> cols_log2-1) & 1) ? ll[ii] : ur[ii^(1<<cols_log2-1)];

  for (uint32_t ii=0; ii<cols; ii++)
    b[row_ur*cols+ii] = ((ii >> cols_log2-1) & 1) ? ll[ii^(1<<cols_log2-1)] : ur[ii];
}

static
void
hsg_debug_remap(uint32_t   const row_from,
                uint32_t   const row_to,
                uint32_t * const r)
{
  fprintf(stdout,"REMAP( %3u, %3u )\n",row_from,row_to);

  r[row_to] = row_from;
}

static
void
hsg_debug_print(uint32_t         const rows,
                uint32_t const * const m,
                uint32_t const * const r)
{
  for (uint32_t rr=0; rr<rows; rr++) {
    for (uint32_t cc=0; cc<cols; cc++)
      fprintf(stdout,"%4u ",m[r[rr]*cols + cc]);
    fprintf(stdout,"\n");
  }
}

int
main(int argc, char * argv[])
{
  uint32_t const cols_log2 = (argc <= 1) ? 3 : strtoul(argv[1],NULL,0);
  uint32_t const rows      = (argc <= 2) ? 6 : strtoul(argv[2],NULL,0);

  if (rows & 1)
    return;

  cols = 1 << cols_log2;

  uint32_t * const b = ALLOCA(cols * rows * sizeof(*b));
  uint32_t * const r = ALLOCA(       rows * sizeof(*r));

  for (uint32_t rr=0; rr<rows; rr++) {
    r[rr] = rr;
    for (uint32_t cc=0; cc<cols; cc++)
      b[rr*cols+cc] = cc*rows+rr;
  }

  hsg_debug_print(rows,b,r);

  hsg_transpose(cols_log2,rows,
                hsg_debug_blend,b,
                hsg_debug_remap,r);

  hsg_debug_print(rows,b,r);

  return 0;
}

#endif

//
//
//
