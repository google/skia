/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 *
 */

#include <string.h>

// #include "extent.h"

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
// s  = size is available
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

//
// HIGH-LEVEL EXTENTS ARE BUILT FROM SIMPLER STRUCTURES
//

//
// COUNTERS FOR POOLS -- TYPICALLY ATOMIC WHEN ON DEVICE
//

union skc_ring
{
  skc_uint2  u32v2;

  skc_uint   u32a2[2];
  
  struct {
    skc_uint reads;  // number of reads
    skc_uint writes; // number of writes
  };
};

//
// POOL OF INDICES TO BLOCKS
//

struct skc_pool_h
{
  skc_uint * indices;
};

struct skc_pool_d
{
  cl_mem   * indices; // FIXME -- READ POOL INDICES THROUGH CONSTANT CACHE?
};

//
// LOW-LEVEL EXTENTS -- SIZES ARE STORED ELSEWHERE
//

struct skc_extent_hrw
{
  void * hrw; // host pointer to host extent -- read/write
};

struct skc_extent_drw
{
  cl_mem drw; // device pointer to device extent -- read/write
};

struct skc_extent_hw_dr
{
  void * hw;  // host   pointer to shared extent -- write-only + write-combined
  cl_mem dr;  // device pointer to shared extent -- read-only
};

//
//
//

#if 0
static
void * 
skc_runtime_svm_alloc(struct skc_runtime_cl * const runtime_cl, size_t const size)
{
  return clSVMAlloc(runtime_cl->context,
                    CL_MEM_READ_WRITE | CL_MEM_SVM_FINE_GRAIN_BUFFER,
                    size,
                    0);
}

static
void * 
skc_runtime_svm_atomic_alloc(struct skc_runtime_cl * const runtime_cl, size_t const size) // WE DON'T NEED THIS HERE
{
  return clSVMAlloc(runtime_cl->context,
                    CL_MEM_READ_WRITE | CL_MEM_SVM_FINE_GRAIN_BUFFER | CL_MEM_SVM_ATOMICS,
                    size,
                    0);
}

static 
void
skc_runtime_svm_free(struct skc_runtime_cl * const runtime_cl, void * const buffer)
{
  clSVMFree(runtime_cl->context,buffer);
}
#endif

//
//
//

void 
skc_command_queue_fill_device(struct skc_command_queue * const cq,
                              cl_mem                           buffer, 
                              void               const * const pattern, 
                              size_t                     const pattern_size,
                              size_t                     const size);

void *
skc_command_queue_map_wi(struct skc_command_queue * const cq,
                         cl_mem                           buffer);

void
skc_command_queue_unmap(struct skc_command_queue * const cq, 
                        cl_mem                           buffer, 
                        void                     * const mapped);

void
skc_command_queue_read(struct skc_command_queue * const cq, 
                       cl_mem                           buffer, 
                       void                     * const ptr);

//
//
//

struct skc_extent_hrw *
skc_extent_hrw_alloc(struct skc_allocator * const allocator,
                     size_t                 const size)
{
  struct skc_extent_hrw * extent;
  
  extent      = skc_allocator_alloc_host(allocator,sizeof(*extent));
  extent->hrw = skc_allocator_alloc_host(allocator,size);

  return extent;
}



void
skc_extent_hrw_free(struct skc_allocator  * const allocator,
                    struct skc_extent_hrw * const extent)
{
  skc_allocator_free_host(allocator,extent->hrw);
  skc_allocator_free_host(allocator,extent);
}

//
//
//

struct skc_extent_drw *
skc_extent_drw_alloc(struct skc_allocator * const allocator,
                     size_t                 const size)
{
  struct skc_extent_drw * extent;

  extent      = skc_allocator_alloc_host  (allocator,sizeof(*extent));
  extent->drw = skc_allocator_alloc_device(allocator,size);

  return extent;
}

void
skc_extent_drw_free(struct skc_allocator  * const allocator,
                    struct skc_extent_drw * const extent)
{
  skc_allocator_free_device(allocator,extent->drw);
  skc_allocator_free_host  (allocator,extent);
}

void
skc_extent_drw_fill(struct skc_command_queue * const cq,
                    struct skc_extent_drw    * const extent,
                    void               const * const pattern,
                    size_t                     const pattern_size,
                    size_t                     const size)
{
  skc_command_queue_fill_device(cq,extent->drw,pattern,pattern_size,size);
}

//
// WRITE-COMBINED / WRITE-INVALIDATE
//

struct skc_extent_hw_dr *
skc_extent_hw_dr_alloc(struct skc_allocator * const allocator,
                       size_t                 const size)
{
  struct skc_extent_hw_dr * extent;

  extent     = skc_allocator_alloc_host(allocator,sizeof(*extent));
  extent->hw = NULL;
  extent->dr = skc_allocator_alloc_device_wc(allocator,size); // write-combined mem

  return extent;
}

void
skc_extent_hw_dr_free(struct skc_allocator    * const allocator,
                      struct skc_extent_hw_dr * const extent)
{
  skc_allocator_free_device(allocator,extent->dr);
  skc_allocator_free_host  (allocator,extent);
}

void
skc_extent_hw_dr_map(struct skc_command_queue * const cq,
                     struct skc_extent_hw_dr  * const extent)
{
  extent->hw = skc_command_queue_map_wi(cq,extent->dr);
}

void
skc_extent_hw_dr_unmap(struct skc_command_queue * const cq,
                       struct skc_extent_hw_dr  * const extent)
{
  skc_command_queue_unmap(cq,extent->dr,extent->hw);
}

void
skc_extent_hw_dr_memcpy(struct skc_extent_hw_dr * const extent,
                        void const * SKC_RESTRICT const src, 
                        size_t                    const offset, 
                        size_t                    const size)
{
  void * SKC_RESTRICT const dst = (char *)extent->hw + offset;

  memcpy(dst,src,size);
}
//
// SNAPSHOT
//

struct skc_extent_hr_drw
{
  void * hr;  // host   pointer to shared extent -- readable snapshot
  cl_mem drw; // device pointer to shared extent -- read/write
};

struct skc_extent_hr_drw *
skc_extent_hr_drw_alloc(struct skc_allocator * const allocator,
                        size_t                 const size)
{
  struct skc_extent_hr_drw * extent;

  extent      = skc_allocator_alloc_host  (allocator,sizeof(*extent));
  extent->hr  = skc_allocator_alloc_host  (allocator,size);
  extent->drw = skc_allocator_alloc_device(allocator,size);

  return extent;
}

void
skc_extent_hr_drw_free(struct skc_allocator     * const allocator,
                       struct skc_extent_hr_drw * const extent)
{
  skc_allocator_free_host  (allocator,extent->hr);
  skc_allocator_free_device(allocator,extent->drw);
  skc_allocator_free_host  (allocator,extent);
}

void
skc_extent_hr_drw_snap(struct skc_command_queue * const cq,
                       struct skc_extent_hr_drw * const extent,
                       size_t                     const size)
{
  skc_command_queue_read(cq,extent->drw,extent->hr);
}

void
skc_extent_hr_drw_fill(struct skc_command_queue * const cq,
                       struct skc_extent_hr_drw * const extent,
                       void               const * const pattern,
                       size_t                     const pattern_size,
                       size_t                     const size)
{
  skc_command_queue_fill_device(cq,extent->drw,pattern,pattern_size,size);
}

//
//
//

struct skc_extent_atomic
{
  struct skc_extent_hr_drw * hr_drw;
  size_t                     size; // typically a very small extent
};

//
//
//

struct skc_extent_atomic *
skc_extent_atomic_alloc(struct skc_allocator * const allocator,
                        size_t                 const size)
{
  struct skc_extent_atomic * extent;

  extent         = skc_allocator_alloc_host(allocator,sizeof(*extent));
  extent->hr_drw = skc_extent_hr_drw_alloc(allocator,size);
  extent->size   = size;

  return extent;
}

void
skc_extent_atomic_free(struct skc_allocator     * const allocator,
                       struct skc_extent_atomic * const extent)
{
  skc_extent_hr_drw_free (allocator,extent->hr_drw);
  skc_allocator_free_host(allocator,extent);
}

void
skc_extent_atomic_snap(struct skc_command_queue       * const cq,
                       struct skc_extent_atomic const * const extent)
{
  skc_extent_hr_drw_snap(cq,extent->hr_drw,extent->size);
}

void
skc_extent_atomic_zero(struct skc_command_queue       * const cq,
                       struct skc_extent_atomic const * const extent)
{
  skc_uint const zero = 0;

  skc_extent_hr_drw_fill(cq,extent->hr_drw,&zero,sizeof(zero),extent->size);
}

//
//
//

struct skc_extent_dxrw
{
  struct skc_extent_drw    * drw;

  size_t                     elem_size;
  skc_uint                   elem_count;

#if 0 // SKC_EXTENT_ATOMIC_IS_IGNORED
  struct skc_extent_atomic * atomic;
  size_t                     atomic_offset;
#endif
};

//
//
//

struct skc_extent_dxrw *
skc_extent_dxrw_alloc(struct skc_allocator     * const allocator,
                      size_t                     const elem_size,
                      skc_uint                   const elem_count,
                      struct skc_extent_atomic * const atomic,
                      size_t                     const atomic_offset)
{
  struct skc_extent_dxrw * extent;

  extent                = skc_allocator_alloc_host(allocator,sizeof(*extent));
  extent->drw           = skc_extent_drw_alloc(allocator,elem_size * elem_count);

  extent->elem_size     = elem_size;
  extent->elem_count    = elem_count;

  //
  // note that passing in the atomic and its member has no real use at
  // this point since the current programming style requires passing
  // in the atomic extent -- which may have multiple members -- to the
  // compute kernel
  //
#if 0 // SKC_EXTENT_ATOMIC_IS_IGNORED  
  extent->atomic        = atomic;
  extent->atomic_offset = atomic_offset;
#endif
  
  return extent;
}

void
skc_extent_dxrw_free(struct skc_allocator   * const allocator,
                     struct skc_extent_dxrw * const extent)
{
  skc_extent_drw_free    (allocator,extent->drw);
  skc_allocator_free_host(allocator,extent);
}

//
//
//

struct skc_extent_hcrw
{
  struct skc_extent_hrw * hrw;
  size_t                  elem_size;
  skc_uint                elem_count;
  skc_uint                counter;
};

//
//
//

struct skc_extent_hcrw *
skc_extent_hcrw_alloc(struct skc_allocator * const allocator,
                      size_t                 const elem_size,
                      skc_uint               const elem_count)
{
  struct skc_extent_hcrw * extent;

  extent             = skc_allocator_alloc_host(allocator,sizeof(*extent));
  extent->hrw        = skc_extent_hrw_alloc(allocator,elem_size * elem_count);
  extent->elem_size  = elem_size;
  extent->elem_count = elem_count;
  extent->counter    = 0;

  return extent;
}

void
skc_extent_hcrw_free(struct skc_allocator   * const allocator,
                     struct skc_extent_hcrw * const extent)
{
  skc_extent_hrw_free    (allocator,extent->hrw);
  skc_allocator_free_host(allocator,extent);
}

void
skc_extent_hcrw_reset(struct skc_extent_hcrw * const extent)
{
  extent->counter = 0;
}

skc_bool
skc_extent_hcrw_is_full(struct skc_extent_hcrw const * const extent)
{
  return (extent->counter == extent->elem_count);
}

//
//
//

struct skc_extent_hcw1_dr1
{
  struct skc_extent_hw_dr * hw_dr; // mapped memory
  size_t                    elem_size;
  skc_uint                  elem_count;
  skc_uint                  counter;
};

//
//
//

struct skc_extent_hcw1_dr1 *
skc_extent_hcw1_dr1_alloc(struct skc_allocator * const allocator,
                          skc_uint               const elem_size,
                          skc_uint               const elem_count)
{
  struct skc_extent_hcw1_dr1 * extent;

  extent             = skc_allocator_alloc_host(allocator,sizeof(*extent));
  extent->hw_dr      = skc_extent_hw_dr_alloc(allocator,elem_size * elem_count);
  extent->elem_size  = elem_size;
  extent->elem_count = elem_count;
  extent->counter    = 0;

  return extent;
}

void
skc_extent_hcw1_dr1_free(struct skc_allocator       * const allocator,
                         struct skc_extent_hcw1_dr1 * const extent)
{
  skc_extent_hw_dr_free  (allocator,extent->hw_dr);
  skc_allocator_free_host(allocator,extent);
}

void
skc_extent_hcw1_dr1_map(struct skc_command_queue   * const cq,
                        struct skc_extent_hcw1_dr1 * const extent)
{
  skc_extent_hw_dr_map(cq,extent->hw_dr);
}

void
skc_extent_hcw1_dr1_unmap(struct skc_command_queue   * const cq,
                          struct skc_extent_hcw1_dr1 * const extent)
{
  skc_extent_hw_dr_unmap(cq,extent->hw_dr);
}

void
skc_extent_hcw1_dr1_reset(struct skc_extent_hcw1_dr1 * const extent)
{
  extent->counter = 0;
}

skc_bool
skc_extent_hcw1_dr1_is_full(struct skc_extent_hcw1_dr1 const * const extent)
{
  return (extent->counter == extent->elem_count);
}

skc_uint
skc_extent_hcw1_dr1_rem(struct skc_extent_hcw1_dr1 * const extent)
{
  return extent->elem_count - extent->counter;
}

void
skc_extent_hcw1_dr1_append(struct skc_extent_hcw1_dr1 * const extent,
                           void  const * SKC_RESTRICT   const elem_ptr,
                           skc_uint                     const elem_count_clamped)
{
  skc_extent_hw_dr_memcpy(extent->hw_dr,
                          elem_ptr,
                          extent->elem_size * extent->counter,
                          extent->elem_size * elem_count_clamped);
}

//
//
//

struct skc_extent_hcw1_drN_unified
{
  struct skc_extent_hw_dr * hw_dr; // mapped memory
  size_t                    elem_size;
  skc_uint                  elem_count;
  skc_uint                  counter;
};

//
//
//

struct skc_extent_hcw1_drN_unified *
skc_extent_hcw1_drN_unified_alloc(struct skc_allocator * const allocator,
                                  skc_uint               const elem_size,
                                  skc_uint               const elem_count)
{
  struct skc_extent_hcw1_drN_unified * extent;

  extent             = skc_allocator_alloc_host(allocator,sizeof(*extent));
  extent->hw_dr      = skc_extent_hw_dr_alloc(allocator,elem_size * elem_count);
  extent->elem_size  = elem_size;
  extent->elem_count = elem_count;
  extent->counter    = 0;

  return extent;
}

void
skc_extent_hcw1_drN_unified_free(struct skc_allocator               * const allocator,
                                 struct skc_extent_hcw1_drN_unified * const extent)
{
  skc_extent_hw_dr_free  (allocator,extent->hw_dr);
  skc_allocator_free_host(allocator,extent);
}

void
skc_extent_hcw1_drN_unified_map(struct skc_command_queue           * const cq,
                                struct skc_extent_hcw1_drN_unified * const extent)
{
  skc_extent_hw_dr_map(cq,extent->hw_dr);
}


void
skc_extent_hcw1_drN_unified_unmap(struct skc_command_queue           * const cq,
                                  struct skc_extent_hcw1_drN_unified * const extent)
{
  skc_extent_hw_dr_unmap(cq,extent->hw_dr);
}

void
skc_extent_hcw1_drN_unified_reset(struct skc_extent_hcw1_drN_unified * const extent)
{
  extent->counter = 0;
}

skc_bool
skc_extent_hcw1_drN_unified_is_full(struct skc_extent_hcw1_drN_unified const * const extent)
{
  return (extent->counter == extent->elem_count);
}


skc_uint
skc_extent_hcw1_drN_unified_rem(struct skc_extent_hcw1_drN_unified * const extent)
{
  return extent->elem_count - extent->counter;
}


void
skc_extent_hcw1_drN_unified_append(struct skc_extent_hcw1_drN_unified * const extent,
                                   void            const * SKC_RESTRICT const elem_ptr,
                                   skc_uint                             const elem_count_clamped)
{
  skc_extent_hw_dr_memcpy(extent->hw_dr,
                          elem_ptr,
                          extent->elem_size * extent->counter,
                          extent->elem_size * elem_count_clamped);
}

//
//
//

struct skc_id_pool_hp *
skc_id_pool_hp_alloc(struct skc_allocator * const allocator,
                     skc_uint               const count)
{
  return NULL;
}

void
skc_id_pool_hp_free(struct skc_allocator  * const allocator,
                    struct skc_id_pool_hp * const extent)
{
  ;
}

void
skc_id_pool_hp_acquire(struct skc_id_pool_hp * const extent, 
                       skc_uint              * const id)
{
  ;
}

void
skc_id_pool_hp_release_1(struct skc_id_pool_hp * const extent, 
                         skc_uint                const id)
{
  ;
}

void
skc_id_pool_hp_release_n(struct skc_id_pool_hp * const extent, 
                         skc_uint        const * const id, 
                         skc_uint                const count)
{
  ;
}

//
//
//

struct skc_block_pool_dprw *
skc_block_pool_dprw_alloc(struct skc_allocator * const allocator,
                          union skc_ring       * const ring_d,
                          skc_uint               const block_size,
                          skc_uint               const block_count)
{
  return NULL;
}

void
skc_block_pool_dprw_free(struct skc_allocator       * const allocator,
                         struct skc_block_pool_dprw * const extent)
{
  ;
}

//
//
//

struct skc_extent_hgw1_drN *
skc_extent_hgw1_drN_alloc(struct skc_allocator * const allocator,
                          skc_uint               const elem_size,
                          skc_uint               const elem_count)
{
  return NULL;
}

void
skc_extent_hgw1_drN_free(struct skc_allocator       * const allocator,
                         struct skc_extent_hgw1_drN * const extent)
{
  ;
}

void
skc_extent_hgw1_drN_reset(struct skc_extent_hgw1_drN * const extent)
{
  ;
}

void
skc_extent_hgw1_drN_snap(struct skc_command_queue         * const cq,
                         struct skc_extent_hgw1_drN const * const extent)
{
  ;
}

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
                            skc_uint               const elem_count)
{
  return NULL;
}

void
skc_block_pool_hp_drw_free(struct skc_allocator         * const allocator,
                           struct skc_block_pool_hp_drw * const extent)
{
  ;
}

//
//
//

#endif

//
//
//
