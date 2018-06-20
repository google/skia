/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 *
 */

//
//
//

#include "common/cl/assert_cl.h"

#include "extent_cl_12.h"
#include "runtime_cl_12.h"
#include "styling_cl_12.h"
#include "composition_cl_12.h"

#include "context.h"
#include "surface.h"

//
//
//

#include <stdio.h>

//
// BUILD
//

struct skc_surface_impl
{
  struct skc_surface        * surface;
  struct skc_runtime        * runtime;

  // framebuffer
  // struct skc_extent_pdrw      fb;
  // struct skc_extent_phrN_pdwN fb;

  // for now, a single in-order command queue
  cl_command_queue            cq;

  struct {
    cl_kernel                 render;
  } kernels;
};

//
// we might want concurrent access to the same surface as long as
// the clips don't overlap.
//
// this would require acquiring a cq on demand when it is determined
// that the clipped render won't overlap
//
// { tile clip , cq } pair
//
// skc_uint4                clip;
// cl_command_queue         cq
//

struct skc_surface_render
{
  skc_uint                      clip[4];

  struct skc_surface_impl     * impl;
  struct skc_styling          * styling;
  struct skc_composition      * composition;

  skc_surface_render_pfn_notify notify;
  void                        * data;

  cl_mem                        fb;

  skc_grid_t                    grid;

  skc_subbuf_id_t               id;
};

//
//
//

static
void
skc_surface_pfn_clear(struct skc_surface_impl * const impl,
                      float                     const rgba[4],
                      skc_uint                  const rect[4],
                      void                     *      fb)
{
  size_t const origin[3] = { rect[0], rect[1], 0 };
  size_t const region[3] = { rect[2], rect[3], 1 };

  cl(EnqueueFillImage(impl->cq,
                      (cl_mem)fb,
                      rgba,
                      origin,
                      region,
                      0,NULL,NULL));
}

//
//
//

static
void
skc_surface_pfn_blit(struct skc_surface_impl * const impl,
                     skc_uint                  const rect[4],
                     skc_int                   const txty[2])
{
  ;
}

//
//
//

#if 0 // #ifndef NDEBUG
#define SKC_SURFACE_DEBUG
#endif

#ifdef SKC_SURFACE_DEBUG

#define SKC_SURFACE_WIDTH  4096
#define SKC_SURFACE_HEIGHT 4096

static
void
skc_surface_debug(struct skc_surface_impl * const impl)
{
  //
  // MAP
  //
  cl_uchar4 * const rgba = skc_extent_phrN_pdwN_map(&impl->fb,
                                                    impl->cq,
                                                    NULL);
  cl(Finish(impl->cq));

  //
  // WRITE
  //
  FILE* file;

  errno_t ferr = fopen_s(&file,"surface.ppm","wb");

  fprintf(file,"P6\n%u %u\n255\n",SKC_SURFACE_WIDTH,SKC_SURFACE_HEIGHT);

  for (skc_uint ii=0; ii<SKC_SURFACE_HEIGHT*SKC_SURFACE_WIDTH; ii++)
    fwrite(rgba + ii,sizeof(skc_uchar),3,file); // R,G,B

  ferr = fclose(file);

  //
  // UNMAP
  //
  skc_extent_phrN_pdwN_unmap(&impl->fb,rgba,impl->cq,NULL);

  cl(Flush(impl->cq));
}

#endif

//
//
//

void
skc_surface_render_complete(struct skc_surface_render * const render)
{
#ifdef SKC_SURFACE_DEBUG
  // write fb out
  skc_surface_debug(render->impl);
#endif

  // notify
  if (render->notify != NULL) {
    render->notify(render->impl->surface,
                   render->styling,
                   render->composition,
                   render->data);
  }

  // unlock and release the styling and composition
  skc_styling_unlock_and_release(render->styling);
  skc_composition_unlock_and_release(render->composition);

  // grid is now complete
  skc_grid_complete(render->grid);
}

static
void
skc_surface_render_cb(cl_event event, cl_int status, struct skc_surface_render * const render)
{
  SKC_CL_CB(status);

  // as quickly as possible, enqueue next stage in pipeline to context command scheduler
  SKC_SCHEDULER_SCHEDULE(render->impl->runtime->scheduler,
                         skc_surface_render_complete,
                         render);
}

//
//
//

static
void
skc_surface_grid_pfn_execute(skc_grid_t const grid)
{
  struct skc_surface_render   * const render  = skc_grid_get_data(grid);
  struct skc_surface_impl     * const impl    = render->impl;
  struct skc_runtime          * const runtime = impl->runtime;

  // get the composition args
  struct skc_composition_impl * const ci      = render->composition->impl;
  struct skc_place_atomics    * const atomics = ci->atomics.hr;

  if (atomics->offsets > 0)
    {
      // acquire the rbo
      cl(EnqueueAcquireGLObjects(impl->cq,1,&render->fb,0,NULL,NULL));

      // get the styling args
      struct skc_styling_impl * const si = render->styling->impl;

      cl(SetKernelArg(impl->kernels.render,0,SKC_CL_ARG(si->layers.drN)));
      cl(SetKernelArg(impl->kernels.render,1,SKC_CL_ARG(si->groups.drN)));
      cl(SetKernelArg(impl->kernels.render,2,SKC_CL_ARG(si->extras.drN)));

      cl(SetKernelArg(impl->kernels.render,3,SKC_CL_ARG(ci->keys.drw)));
      cl(SetKernelArg(impl->kernels.render,4,SKC_CL_ARG(atomics->keys)));
      cl(SetKernelArg(impl->kernels.render,5,SKC_CL_ARG(ci->offsets.drw)));
      cl(SetKernelArg(impl->kernels.render,6,SKC_CL_ARG(atomics->offsets)));

      // block pool
      cl(SetKernelArg(impl->kernels.render,7,SKC_CL_ARG(impl->runtime->block_pool.blocks.drw)));

      // surface
      cl(SetKernelArg(impl->kernels.render,8,SKC_CL_ARG(render->fb)));

#if 1
      // tile clip
      cl(SetKernelArg(impl->kernels.render,9,sizeof(skc_uint4),render->clip));
#else
      // surface pitch (height)
      skc_uint const surface_pitch = SKC_SURFACE_HEIGHT;
      cl(SetKernelArg(impl->kernels.render,9,SKC_CL_ARG(surface_pitch)));
      // tile clip
      cl(SetKernelArg(impl->kernels.render,10,sizeof(skc_uint4),render->clip));
#endif

      // launch render kernel
      skc_device_enqueue_kernel(runtime->device,
                                SKC_DEVICE_KERNEL_ID_RENDER,
                                impl->cq,
                                impl->kernels.render,
                                atomics->offsets,
                                0,NULL,NULL);


      cl_event complete;

      // give the rbo back
      cl(EnqueueReleaseGLObjects(impl->cq,1,&render->fb,0,NULL,&complete));

      // notify anyone listening...
      cl(SetEventCallback(complete,CL_COMPLETE,skc_surface_render_cb,render));
      cl(ReleaseEvent(complete));

      // flush it
      cl(Flush(impl->cq));
    }
  else
    {
      skc_surface_render_complete(render);
    }
}

//
//
//

static
void
skc_surface_pfn_release(struct skc_surface_impl * const impl)
{
  if (--impl->surface->ref_count != 0)
    return;

  //
  // otherwise, release all resources
  //
  
  // drain the command queue
  cl(Finish(impl->cq));

  struct skc_runtime * const runtime = impl->runtime;  

  // release the kernel
  cl(ReleaseKernel(impl->kernels.render));

  // free surface host
  skc_runtime_host_perm_free(runtime,impl->surface);

  // release the cq
  skc_runtime_release_cq_in_order(runtime,impl->cq);

  // release fb
  // skc_extent_phrN_pdwN_free(runtime,&impl->fb);
  
  // free surface impl
  skc_runtime_host_perm_free(runtime,impl);
}

//
//
//

static
void
skc_surface_grid_pfn_dispose(skc_grid_t const grid)
{
  struct skc_surface_render * const render  = skc_grid_get_data(grid);
  struct skc_surface_impl   * const impl    = render->impl;
  struct skc_runtime        * const runtime = impl->runtime;

  // free the render object
  skc_runtime_host_temp_free(runtime,render,render->id);

  // release the surface
  skc_surface_pfn_release(impl);
}

//
//
//

static
void
skc_surface_pfn_render(struct skc_surface_impl * const impl,
                       uint32_t                  const clip[4],
                       skc_styling_t                   styling,
                       skc_composition_t               composition,
                       skc_surface_render_pfn_notify   notify,
                       void                          * data,
                       void                          * fb)
{
  // retain surface
  skc_surface_retain(impl->surface);

  //
  // FIXME -- we used to seal the styling and composition objects if
  // they weren't already.  Either test that they're sealed or seal
  // them here.
  //

  // retain and lock the styling and composition 
  skc_styling_retain_and_lock(styling);
  skc_composition_retain_and_lock(composition);

  //
  // allocate a render instance
  //
  skc_subbuf_id_t                   id;
  struct skc_surface_render * const render = skc_runtime_host_temp_alloc(impl->runtime,
                                                                         SKC_MEM_FLAGS_READ_WRITE,
                                                                         sizeof(*render),&id,NULL);
  render->id          = id;

  render->clip[0]     = clip[0];
  render->clip[1]     = clip[1];
  render->clip[2]     = clip[2];
  render->clip[3]     = clip[3];

  render->impl        = impl;
  render->styling     = styling;
  render->composition = composition;

  render->notify      = notify;
  render->data        = data;

  render->fb          = fb;

  render->grid        = SKC_GRID_DEPS_ATTACH(impl->runtime->deps,
                                             NULL, // invalidation not necessary
                                             render,
                                             NULL, // no waiting
                                             skc_surface_grid_pfn_execute,
                                             skc_surface_grid_pfn_dispose);

  // declare happens-after relationships
  skc_grid_happens_after_grid(render->grid,styling->impl->grid);
  skc_grid_happens_after_grid(render->grid,composition->impl->grids.sort);

  // wait for styling and composition
  skc_grid_start(render->grid);
}

//
//
//

skc_err
skc_surface_cl_12_create(struct skc_context   * const context,
                         struct skc_surface * * const surface)
{
  struct skc_runtime * const runtime = context->runtime;

  // allocate surface
  (*surface) = skc_runtime_host_perm_alloc(runtime,SKC_MEM_FLAGS_READ_WRITE,sizeof(**surface));

  // allocate impl
  struct skc_surface_impl * const impl = skc_runtime_host_perm_alloc(runtime,SKC_MEM_FLAGS_READ_WRITE,sizeof(*impl));

  // initialize surface
  // SKC_ASSERT_STATE_INIT((*impl),SKC_SURFACE_STATE_READY);

  (*surface)->context    = context;
  (*surface)->impl       = impl;
  (*surface)->ref_count  = 1;

  (*surface)->release    = skc_surface_pfn_release;
  (*surface)->clear      = skc_surface_pfn_clear;
  (*surface)->blit       = skc_surface_pfn_blit;
  (*surface)->render     = skc_surface_pfn_render;

  // intialize impl
  impl->surface          = *surface;
  impl->runtime          = runtime;

#if 0
  // FIXME -- 4K x 4K -- temporarily fixed size
  size_t const fb_size = sizeof(skc_uchar4) * SKC_SURFACE_WIDTH * SKC_SURFACE_HEIGHT;

  // create framebuffer
  skc_extent_phrN_pdwN_alloc(runtime,&impl->fb,fb_size);
#endif

  // acquire a command queue
  impl->cq               = skc_runtime_acquire_cq_in_order(runtime);

  // acquire kernel
  impl->kernels.render   = skc_device_acquire_kernel(runtime->device,SKC_DEVICE_KERNEL_ID_RENDER);

  return SKC_ERR_SUCCESS;
}

//
//
//
