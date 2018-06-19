/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 *
 */

#pragma once

//
//
//

#include "skc.h"
#include "allocator.h"

//
// EXTENT TYPES
//
// Classification of operations on allocated GPU memory
//
// h  = host
// d  = device
//
// c  = append using non-atomic incremented count
// x  = append using atomically incremented index
// p  = allocated from pool of indices
// g  = gathered by pull kernel
//
// w1 = write once
// wN = write many
//
// r1 = read once
// rN = read many
//
// rw = read/write many
//
//                                host<>device memory model
//                       +--------------------+--------------------+
//     extent type       |      split         |      shared        |  examples
//  ---------------------+--------------------+--------------------+--------------------
//                       |                    |                    |
//   extent_atomic       |   device+mapped    |   device+mapped    |  atomically op'd device extent + read-only host snapshot
//                       |                    |                    |
//   extent_dxrw         |      device        |      device        |  ttsk_array, ttpk_array, ttck_array, *_offsets
//   extent_hcw1_dr1     |      mapped        |      mapped        |  command_queue, buffer
//   extent_hcrw         |       host         |       host         |  queue
//                       |                    |                    |
//  ---------------------+--------------------+--------------------+--------------------
//                       |                    |                    |
//   extent_hcw1_drN     |     memcpy'd       |      mapped        |  stack_transforms, stack_stroke_props
//   extent_hgw1_drN     |   scatter/gather   |      mapped        |  layer_props
//                       |                    |                    |
//   block_pool_dprw     |      device        |      device        |  ttsb_pool, ttpb_pool
//   block_pool_hp_drw   |      device        |      device        |  raster_pool
//                       |                    |                    |
//  ---------------------+--------------------+--------------------+--------------------
//                       |                    |                    |
//   block_pool_hp_drw   | block_pool_hp_drw  | block_pool_hp_drw  |  path_block_pool
//   staging buffer      | extent_hw_dr       |        --          |
//                       |                    |                    |
//

struct skc_extent_hrw;
struct skc_extent_drw;

struct skc_extent_hrw_drN;
struct skc_extent_hw1_drN;
struct skc_extent_hrN_drw;

struct skc_extent_atomic;

struct skc_extent_hcrw;
struct skc_extent_dxrw;

struct skc_block_pool_dprw;

struct skc_id_pool_hp;

struct skc_extent_hcw1_dr1;
struct skc_extent_hcw1_drN;
struct skc_extent_hgw1_drN;

//
//
//

void *
skc_extent_hrw_drN_get_hrw(struct skc_extent_hrw_drN * extent);

void *
skc_extent_hw1_drN_get_hw1(struct skc_extent_hw1_drN * extent);

//
//
//

struct skc_extent_hrw *
skc_extent_hrw_alloc(struct skc_allocator * const allocator,
                     size_t                 const size);

void
skc_extent_hrw_free(struct skc_allocator  * const allocator,
                    struct skc_extent_hrw * const extent);

void *
skc_extent_hrw_get_hrw(struct skc_extent_hrw * extent);

//
//
//

struct skc_extent_drw *
skc_extent_drw_alloc(struct skc_allocator * const allocator,
                     size_t                 const size);

void
skc_extent_drw_free(struct skc_allocator  * const allocator,
                    struct skc_extent_drw * const extent);

void
skc_extent_drw_fill(struct skc_command_queue * const cq,
                    struct skc_extent_drw    * const extent,
                    void               const * const pattern,
                    size_t                     const pattern_size,
                    size_t                     const size);

//
//
//

struct skc_extent_hw_dr *
skc_extent_hw_dr_alloc(struct skc_allocator * const allocator,
                       size_t                 const size);

void
skc_extent_hw_dr_free(struct skc_allocator    * const allocator,
                      struct skc_extent_hw_dr * const extent);

void
skc_extent_hw_dr_map(struct skc_command_queue * const cq,
                     struct skc_extent_hw_dr  * const extent);

void
skc_extent_hw_dr_unmap(struct skc_command_queue * const cq,
                       struct skc_extent_hw_dr  * const extent);

void
skc_extent_hw_dr_memcpy(struct skc_extent_hw_dr * const extent,
                        void const * SKC_RESTRICT const src, 
                        size_t                    const offset, 
                        size_t                    const size);
//
//
//

struct skc_extent_hr_drw *
skc_extent_hr_drw_alloc(struct skc_allocator * const allocator,
                        size_t                 const size);

void
skc_extent_hr_drw_free(struct skc_allocator     * const allocator,
                       struct skc_extent_hr_drw * const extent);

void
skc_extent_hr_drw_snap(struct skc_command_queue * const cq,
                       struct skc_extent_hr_drw * const extent,
                       size_t                     const size);

void
skc_extent_hr_drw_fill(struct skc_command_queue * const cq,
                       struct skc_extent_hr_drw * const extent,
                       void               const * const pattern,
                       size_t                     const pattern_size,
                       size_t                     const size);

//
//
//

struct skc_extent_atomic *
skc_extent_atomic_alloc(struct skc_allocator * const allocator,
                        size_t                 const size);

void
skc_extent_atomic_free(struct skc_allocator     * const allocator,
                       struct skc_extent_atomic * const extent);

void
skc_extent_atomic_snap(struct skc_command_queue       * const cq,
                       struct skc_extent_atomic const * const extent);

void
skc_extent_atomic_zero(struct skc_command_queue       * const cq,
                       struct skc_extent_atomic const * const extent);

//
//
//


struct skc_extent_dxrw *
skc_extent_dxrw_alloc(struct skc_allocator     * const allocator,
                      size_t                     const elem_size,
                      skc_uint                   const elem_count,
                      struct skc_extent_atomic * const atomic,
                      size_t                     const atomic_offset);

void
skc_extent_dxrw_free(struct skc_allocator   * const allocator,
                     struct skc_extent_dxrw * const extent);

//
//
//

struct skc_extent_hcrw *
skc_extent_hcrw_alloc(struct skc_allocator * const allocator,
                      size_t                 const elem_size,
                      skc_uint               const elem_count);

void
skc_extent_hcrw_free(struct skc_allocator   * const allocator,
                     struct skc_extent_hcrw * const extent);

void
skc_extent_hcrw_reset(struct skc_extent_hcrw * const extent);

skc_bool
skc_extent_hcrw_is_full(struct skc_extent_hcrw const * const extent);

//
//
//

struct skc_extent_hcw1_dr1 *
skc_extent_hcw1_dr1_alloc(struct skc_allocator * const allocator,
                          skc_uint               const elem_size,
                          skc_uint               const elem_count);

void
skc_extent_hcw1_dr1_free(struct skc_allocator       * const allocator,
                         struct skc_extent_hcw1_dr1 * const extent);

void
skc_extent_hcw1_dr1_map(struct skc_command_queue   * const cq,
                        struct skc_extent_hcw1_dr1 * const extent);

void
skc_extent_hcw1_dr1_unmap(struct skc_command_queue   * const cq,
                          struct skc_extent_hcw1_dr1 * const extent);

void
skc_extent_hcw1_dr1_reset(struct skc_extent_hcw1_dr1 * const extent);

skc_bool
skc_extent_hcw1_dr1_is_full(struct skc_extent_hcw1_dr1 const * const extent);

skc_uint
skc_extent_hcw1_dr1_rem(struct skc_extent_hcw1_dr1 * const extent);

void
skc_extent_hcw1_dr1_append(struct skc_extent_hcw1_dr1 * const extent,
                           void  const * SKC_RESTRICT   const elem_ptr,
                           skc_uint                     const elem_count_clamped);

//
// Note: on a shared memory device this reuses the hcw1_dr1
// implementation and unmaps the extent instead of copying
//

struct skc_extent_hcw1_drN_unified *
skc_extent_hcw1_drN_unified_alloc(struct skc_allocator * const allocator,
                                  skc_uint               const elem_size,
                                  skc_uint               const elem_count);

void
skc_extent_hcw1_drN_unified_free(struct skc_allocator               * const allocator,
                                 struct skc_extent_hcw1_drN_unified * const extent);

void
skc_extent_hcw1_drN_unified_map(struct skc_command_queue           * const cq,
                                struct skc_extent_hcw1_drN_unified * const extent);

void
skc_extent_hcw1_drN_unified_unmap(struct skc_command_queue           * const cq,
                                  struct skc_extent_hcw1_drN_unified * const extent);

void
skc_extent_hcw1_drN_unified_reset(struct skc_extent_hcw1_drN_unified * const extent);

skc_bool
skc_extent_hcw1_drN_unified_is_full(struct skc_extent_hcw1_drN_unified const * const extent);

skc_uint
skc_extent_hcw1_drN_unified_rem(struct skc_extent_hcw1_drN_unified * const extent);

void
skc_extent_hcw1_drN_unified_append(struct skc_extent_hcw1_drN_unified * const extent,
                                   void            const * SKC_RESTRICT const elem_ptr,
                                   skc_uint                             const elem_count_clamped);
//
//
//

struct skc_id_pool_hp *
skc_id_pool_hp_alloc(struct skc_allocator * const allocator,
                     skc_uint               const count);

void
skc_id_pool_hp_free(struct skc_allocator  * const allocator,
                    struct skc_id_pool_hp * const extent);

void
skc_id_pool_hp_acquire(struct skc_id_pool_hp * const extent, 
                       skc_uint              * const id);

void
skc_id_pool_hp_release_1(struct skc_id_pool_hp * const extent, 
                         skc_uint                const id);

void
skc_id_pool_hp_release_n(struct skc_id_pool_hp * const extent, 
                         skc_uint        const * const id, 
                         skc_uint                const count);

//
//
//

struct skc_block_pool_dprw *
skc_block_pool_dprw_alloc(struct skc_allocator * const allocator,
                          union skc_ring       * const ring_d,
                          skc_uint               const block_size,
                          skc_uint               const block_count);

void
skc_block_pool_dprw_free(struct skc_allocator       * const allocator,
                         struct skc_block_pool_dprw * const extent);

//
//
//

struct skc_extent_hgw1_drN_unified *
skc_extent_hgw1_drN_unified_alloc(struct skc_allocator * const allocator,
                          skc_uint               const elem_size,
                          skc_uint               const elem_count);

void
skc_extent_hgw1_drN_unified_free(struct skc_allocator       * const allocator,
                         struct skc_extent_hgw1_drN_unified * const extent);

void
skc_extent_hgw1_drN_unified_reset(struct skc_extent_hgw1_drN_unified * const extent);

void
skc_extent_hgw1_drN_unified_snap(struct skc_command_queue         * const cq,
                         struct skc_extent_hgw1_drN_unified const * const extent);

//
//
//

#if 0

//
//
//

struct skc_block_pool_hp_drw *
skc_block_pool_hp_drw_alloc(struct skc_allocator * const allocator,
                            skc_uint               const elem_size,
                            skc_uint               const elem_count);

void
skc_block_pool_hp_drw_free(struct skc_allocator         * const allocator,
                           struct skc_block_pool_hp_drw * const extent);

//
//
//

#endif

//
//
//

