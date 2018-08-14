/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 *
 */

#include "gl/GrGLInterface.h"
#include "gl/GrGLUtil.h"

//
//
//

#include <cuda_runtime_api.h>

//
//
//

#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>

//
//
//

#ifdef __cplusplus
extern "C" {
#endif

#include <assert_cuda.h>

  //
  // #include <cuda_gl_interop.h> // can't include in Skia
  //

  extern __host__ cudaError_t CUDARTAPI 
  cudaGraphicsGLRegisterBuffer(struct cudaGraphicsResource **resource, GrGLuint buffer, unsigned int flags);

  extern __host__ cudaError_t CUDARTAPI 
  cudaGraphicsGLRegisterImage(struct cudaGraphicsResource **resource, GrGLuint image, GrGLenum target, unsigned int flags);

#ifdef __cplusplus
}
#endif

//
//
//

#include "interop.h"

//
//
//

#if 1
#define PXL_IMAGE_FORMAT GR_GL_RGBA8
#else
#define PXL_IMAGE_FORMAT GR_GL_RGBA16
#endif

//
//
//

struct pxl_interop_surface
{
  bool                   use;
  pxl_interop_surface_t  idx;

  GrGLuint               fbo;
  GrGLuint               rbo;

  cudaGraphicsResource_t cgr;
  cudaSurfaceObject_t    cso;
};

//
//
//

struct pxl_interop
{
  const struct GrGLInterface* gl;

  struct pxl_interop_surface* surfaces;

  struct {
    uint32_t                  size;
    uint32_t                  avail;
    pxl_interop_surface_t*    indices;
  } pool;

  uint32_t                    width;
  uint32_t                    height;
};

//
// for now, make these asserts always checked
//

#ifndef NDEBUG
#define NDEBUG 
#endif

//
//
//

struct pxl_interop*
pxl_interop_create(const struct GrGLInterface* const gl, uint32_t const surface_count_max)
{
  struct pxl_interop* const interop = (struct pxl_interop*)calloc(1,sizeof(*interop));

  //
  // init rest of struct

  interop->gl           = gl;

  interop->surfaces     = (struct pxl_interop_surface*)calloc(surface_count_max,sizeof(*interop->surfaces));

  interop->pool.indices = (pxl_interop_surface_t*)calloc(surface_count_max,sizeof(*interop->pool.indices));
  interop->pool.size    = surface_count_max;
  interop->pool.avail   = surface_count_max;

  // init pool
  for (uint32_t ii=0; ii<surface_count_max; ii++)
    interop->pool.indices[ii] = ii;

  interop->width  = 0;
  interop->height = 0;

  // return it
  return interop;
}

//
//
//

static
void
pxl_interop_register_is(struct pxl_interop         * const interop,
                        struct pxl_interop_surface * const is,
                        cudaStream_t                       stream)
{
  // do nothing is interop dimensions are empty
  if ((interop->width == 0) || (interop->height == 0))
    {
      is->cgr = NULL;
      return;
    }

  //
  // otherwise...
  //

  // register rbo
#if 0
  cuda(GraphicsGLRegisterImage(&is->cgr,is->rbo,GR_GL_RENDERBUFFER,
                               cudaGraphicsRegisterFlagsSurfaceLoadStore |
                               cudaGraphicsRegisterFlagsWriteDiscard));
#else
  cudaError_t err = cudaGraphicsGLRegisterImage(&is->cgr,is->rbo,GR_GL_RENDERBUFFER,
                                                cudaGraphicsRegisterFlagsSurfaceLoadStore |
                                                cudaGraphicsRegisterFlagsWriteDiscard);

  cuda_assert(err,__FILE__,__LINE__,true);
#endif

  // map graphics resources
  cuda(GraphicsMapResources(1,&is->cgr,stream));

  struct cudaResourceDesc cuda_surface_desc;
  cuda_surface_desc.resType = cudaResourceTypeArray;

  cuda(GraphicsSubResourceGetMappedArray(&cuda_surface_desc.res.array.array,is->cgr,0,0));

  cuda(CreateSurfaceObject(&is->cso,&cuda_surface_desc));

  // unmap
  cuda(GraphicsUnmapResources(1,&is->cgr,stream));
}


void
pxl_interop_acquire(struct pxl_interop    * const interop,
                    pxl_interop_surface_t * const surface,
                    cudaStream_t                  stream)
{
  assert(interop->pool.avail > 0);

  *surface = interop->pool.indices[--interop->pool.avail];

  struct pxl_interop_surface* const is = interop->surfaces + *surface;

  // set id
  is->use = true;
  is->idx = *surface;

  // create render buffer object w/a color buffer
  GR_GL_CALL(interop->gl,
             GenRenderbuffers(1,&is->rbo));

  // bind the new rbo
  GR_GL_CALL(interop->gl,
             BindRenderbuffer(GR_GL_RENDERBUFFER,is->rbo));

  // resize rbo
  GR_GL_CALL(interop->gl,
             RenderbufferStorage(GR_GL_RENDERBUFFER,
                                 PXL_IMAGE_FORMAT,
                                 interop->width,
                                 interop->height));

#ifdef PXL_INTEROP_UNBIND
  // unbind the new rbo
  GR_GL_CALL(interop->gl,
             BindRenderbuffer(GR_GL_RENDERBUFFER,0));
#endif

  // create frame buffer object
  GR_GL_CALL(interop->gl,
             GenFramebuffers(1,&is->fbo));

  // bind the new frame buffer
  GR_GL_CALL(interop->gl,
             BindFramebuffer(GR_GL_FRAMEBUFFER,is->fbo));
  
  // attach rbo to fbo
  GR_GL_CALL(interop->gl,
             FramebufferRenderbuffer(GR_GL_FRAMEBUFFER,
                                     GR_GL_COLOR_ATTACHMENT0,
                                     GR_GL_RENDERBUFFER,
                                     is->rbo));

#ifdef PXL_INTEROP_UNBIND
  // unbind the new fbo
  GR_GL_CALL(interop->gl,
             BindFramebuffer(GR_GL_FRAMEBUFFER,0));
#endif


  pxl_interop_register_is(interop,is,stream);
}

//
//
//

static
void
pxl_interop_release_is(struct pxl_interop         * const interop,
                       struct pxl_interop_surface * const is)
{
  assert(is->use);

  if (is->cgr != NULL)
    {
      // destroy surface object
      cuda(DestroySurfaceObject(is->cso));

      // unregister interop resource
      cuda(GraphicsUnregisterResource(is->cgr));
    }

  // delete rbo
  GR_GL_CALL(interop->gl,
             DeleteRenderbuffers(1,&is->rbo));

  // delete fbo
  GR_GL_CALL(interop->gl,
             DeleteFramebuffers(1,&is->fbo));

  // reset
  is->use = false;
}

static 
struct pxl_interop_surface*
pxl_interop_get_is(struct pxl_interop  * const interop,
                   pxl_interop_surface_t const surface)
{
  assert(surface < interop->pool.size);

  return interop->surfaces + surface;
}

void
pxl_interop_release(struct pxl_interop  * const interop,
                    pxl_interop_surface_t const surface)
{
  struct pxl_interop_surface* const is = pxl_interop_get_is(interop,surface);

  pxl_interop_release_is(interop,is);

  // return to avail
  interop->pool.indices[interop->pool.avail++] = surface;
}

//
//
//

void
pxl_interop_dispose(struct pxl_interop * const interop)
{
  for (uint32_t ii=0; ii<interop->pool.size; ii++)
    {
      struct pxl_interop_surface* const is = pxl_interop_get_is(interop,ii);

      if (is->use)
        {
          pxl_interop_release_is(interop,is);

          interop->pool.indices[interop->pool.avail++] = ii;
        }
    }

  // free buffers and resources
  free(interop->pool.indices);
  free(interop->surfaces);
  free(interop);
}

//
//
//

static
void
pxl_interop_remap_is(struct pxl_interop         * const interop,
                     struct pxl_interop_surface * const is,
                     uint32_t                     const width,
                     uint32_t                     const height,
                     cudaStream_t                       stream)
{
  if (is->cgr != NULL)
    {
      // destroy surface object
      cuda(DestroySurfaceObject(is->cso));

      // unregister interop resource
      cuda(GraphicsUnregisterResource(is->cgr));
    }

  // resize rbo
  GR_GL_CALL(interop->gl,
             NamedRenderbufferStorage(is->rbo,
                                      PXL_IMAGE_FORMAT,
                                      width,height));

  // probe fbo status
  // glCheckNamedFramebufferStatus(interop->fb[index],0);

  pxl_interop_register_is(interop,is,stream);
}

//
//
//

void
pxl_interop_size_set(struct pxl_interop * const interop,
                     uint32_t             const width,
                     uint32_t             const height,
                     cudaStream_t               stream)
{
  interop->width  = width;
  interop->height = height;

  for (uint32_t ii=0; ii<interop->pool.size; ii++)
    {
      struct pxl_interop_surface* const is = pxl_interop_get_is(interop,ii);

      if (is->use)
        {
          pxl_interop_remap_is(interop,is,width,height,stream);
        }
    }
}

//
//
//

void
pxl_interop_size_get(struct pxl_interop * const interop, uint32_t * const width, uint32_t * const height)
{
  *width  = interop->width;
  *height = interop->height;
}

//
//
//

cudaSurfaceObject_t
pxl_interop_get_surface(struct pxl_interop * const interop, pxl_interop_surface_t const surface)
{
  struct pxl_interop_surface* const is = pxl_interop_get_is(interop,surface);  

  assert(is->use);

  return is->cso;
}

//
//
//

void
pxl_interop_clear(struct pxl_interop  * const interop, 
                  pxl_interop_surface_t const surface,
                  float                 const rgba[4])
{
  struct pxl_interop_surface* const is = pxl_interop_get_is(interop,surface);  

  assert(is->use);

  /*
    static const GLenum attachments[] = { GL_COLOR_ATTACHMENT0 };
    glInvalidateNamedFramebufferData(interop->fb[interop->index],1,attachments);
  */
    
  GR_GL_CALL(interop->gl,
             BindFramebuffer(GR_GL_DRAW_FRAMEBUFFER,is->fbo));

  GR_GL_CALL(interop->gl,
             ClearColor(rgba[0],
                        rgba[1],
                        rgba[2],
                        rgba[3]));

  GR_GL_CALL(interop->gl,
             Clear(GR_GL_COLOR_BUFFER_BIT));

#ifdef PXL_INTEROP_UNBIND
  GR_GL_CALL(interop->gl,
             BindFramebuffer(GR_GL_DRAW_FRAMEBUFFER,0));
#endif
}


//
//
//

static
void
pxl_interop_blit_to_fbo(struct pxl_interop  * const interop, 
                        pxl_interop_surface_t const surface,
                        GrGLuint              const fbo)
{
  struct pxl_interop_surface* const is = pxl_interop_get_is(interop,surface);  

  assert(is->use);

  GR_GL_CALL(interop->gl,
             BindFramebuffer(GR_GL_READ_FRAMEBUFFER,is->fbo));

  GR_GL_CALL(interop->gl,
             BindFramebuffer(GR_GL_DRAW_FRAMEBUFFER,fbo));

  GR_GL_CALL(interop->gl,
             BlitFramebuffer(0,0,interop->width,interop->height,
                             0,0,interop->width,interop->height,
                             GR_GL_COLOR_BUFFER_BIT,
                             GR_GL_NEAREST));

#ifdef PXL_INTEROP_UNBIND
  GR_GL_CALL(interop->gl,
             BindFramebuffer(GR_GL_READ_FRAMEBUFFER,0));
#endif
}



void
pxl_interop_blit(struct pxl_interop* const interop, const pxl_interop_surface_t surface)
{
  pxl_interop_blit_to_fbo(interop,surface,0);
}

//
//
//

void
pxl_interop_window_swap(struct pxl_interop* const interop)
{
  // glfwSwapBuffers(interop->window);
}

//
//
//

void
pxl_interop_map(struct pxl_interop*   const interop, 
                const pxl_interop_surface_t surface, 
                cudaStream_t                stream)
{
  struct pxl_interop_surface* const is = pxl_interop_get_is(interop,surface);  
  
  if (is->cgr != NULL)
    {
      // map graphics resources
      cuda(GraphicsMapResources(1,&is->cgr,stream));
    }
}
 
void
pxl_interop_unmap(struct pxl_interop*   const interop, 
                  const pxl_interop_surface_t surface, 
                  cudaStream_t                stream)
{
  struct pxl_interop_surface* const is = pxl_interop_get_is(interop,surface);  
  
  if (is->cgr != NULL)
    {
      cuda(GraphicsUnmapResources(1,&is->cgr,stream));
    }
}

//
//
//

void
pxl_interop_snap_create(struct pxl_interop  * const interop,
                        pxl_interop_surface_t const surface,
                        GrGLuint            * const snap)
{
  GrGLuint rbo;

  //
  // CREATE A NEW FBO+RBO
  //

  // create render buffer object w/a color buffer
  GR_GL_CALL(interop->gl,
             GenRenderbuffers(1,&rbo));

  // bind the new rbo
  GR_GL_CALL(interop->gl,
             BindRenderbuffer(GR_GL_RENDERBUFFER,rbo));

  // resize rbo
  GR_GL_CALL(interop->gl,
             RenderbufferStorage(GR_GL_RENDERBUFFER,
                                 PXL_IMAGE_FORMAT,
                                 interop->width,
                                 interop->height));

#ifdef PXL_INTEROP_UNBIND
  // unbind the new rbo
  GR_GL_CALL(interop->gl,
             BindRenderbuffer(GR_GL_RENDERBUFFER,0));
#endif

  // create frame buffer object
  GR_GL_CALL(interop->gl,
             GenFramebuffers(1,snap));

  // bind the new frame buffer
  GR_GL_CALL(interop->gl,
             BindFramebuffer(GR_GL_FRAMEBUFFER,*snap));
  
  // attach rbo to fbo
  GR_GL_CALL(interop->gl,
             FramebufferRenderbuffer(GR_GL_FRAMEBUFFER,
                                     GR_GL_COLOR_ATTACHMENT0,
                                     GR_GL_RENDERBUFFER,
                                     rbo));

#ifdef PXL_INTEROP_UNBIND
  // unbind the new fbo
  GR_GL_CALL(interop->gl,
             BindFramebuffer(GR_GL_FRAMEBUFFER,0));
#endif

  //
  // BLIT TO IT
  //
  pxl_interop_blit_to_fbo(interop,surface,*snap);

}


void
pxl_interop_snap_dispose(struct pxl_interop * const interop,
                         GrGLuint             const snap)
{
  GrGLuint rbo;

  // bind the snap which is a frame buffer
  GR_GL_CALL(interop->gl,
             BindFramebuffer(GR_GL_FRAMEBUFFER,snap));


  // get the rbo color attachment
  GR_GL_CALL(interop->gl,
             GetFramebufferAttachmentParameteriv(GR_GL_FRAMEBUFFER,
                                                 GR_GL_COLOR_ATTACHMENT0,
                                                 GR_GL_RENDERBUFFER,
                                                 (GrGLint*)&rbo));

#ifdef PXL_INTEROP_UNBIND
  // unbind the new fbo
  GR_GL_CALL(interop->gl,
             BindFramebuffer(GR_GL_FRAMEBUFFER,0));
#endif

  // delete rbo
  GR_GL_CALL(interop->gl,
             DeleteRenderbuffers(1,&rbo));

  // delete fbo
  GR_GL_CALL(interop->gl,
             DeleteFramebuffers(1,&snap));
  
}


//
//
//


