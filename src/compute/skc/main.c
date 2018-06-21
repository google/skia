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

#include <glad/glad.h>
#include <GLFW/glfw3.h>

//
//
//

#include <stdio.h>
#include <stdlib.h>
#include <conio.h>

#include "skc_create_cl.h"

#include "common/cl/find_cl.h"
#include "common/cl/assert_cl.h"

#include "svg/svg_doc.h"
#include "svg2skc/svg2skc.h"
#include "svg2skc/transform_stack.h"

//
//
//

#include <CL/opencl.h>
#include "platforms/cl_12/gl/interop.h"

//
//
//

void
skc_runtime_cl_12_debug(struct skc_context * const context);

//
//
//



//
//
//

static
void
is_render_complete(skc_surface_t     surface,
                   skc_styling_t     styling,
                   skc_composition_t composition,
                   bool            * quit)
{
  *quit = true;
}

//
//
//

int
main(int argc, char** argv)
{
  //
  //
  //
  if (argc <= 1)
    {
      fprintf(stderr,"-- missing filename\n");
      return EXIT_FAILURE; // no filename
    }

  //
  // load test file
  //
  // #include "test/lion.inl"

  struct svg_doc * svg_doc = svg_doc_parse(argv[1],false);

  fprintf(stderr,"p/r/l = %u / %u / %u\n",
          svg_doc_path_count(svg_doc),
          svg_doc_raster_count(svg_doc),
          svg_doc_layer_count(svg_doc));

  //
  // fire up GL
  //
  GLFWwindow * window;

  skc_interop_init(&window);

  //
  // find platform and device by name
  //
  cl_platform_id platform_id_cl;
  cl_device_id   device_id_cl;

  cl(FindIdsByName("Intel","Graphics",
                   &platform_id_cl,
                   &device_id_cl,
                   0,NULL,NULL,
                   true));

  //
  // get GL and device contexts
  //
  HGLRC hGLRC = wglGetCurrentContext();
  HDC   hDC   = wglGetCurrentDC();

  //
  // create the CL context
  //
  cl_context_properties context_properties_cl[] =
    {
      CL_CONTEXT_PLATFORM, (cl_context_properties)platform_id_cl,
      CL_GL_CONTEXT_KHR,   (cl_context_properties)hGLRC,
      CL_WGL_HDC_KHR,      (cl_context_properties)hDC,
      0
    };

  cl_int     cl_err;
  cl_context context_cl = clCreateContext(context_properties_cl,
                                          1,
                                          &device_id_cl,
                                          NULL,
                                          NULL,
                                          &cl_err); cl_ok(cl_err);
  //
  // create SKC context
  //
  skc_context_t context;

  skc_err err = skc_context_create_cl(&context,
                                      context_cl,
                                      device_id_cl);

  //
  // associate
  //
  skc_interop_register(context);

  //
  // create path builder
  //
  skc_path_builder_t path_builder;

  err = skc_path_builder_create(context,&path_builder);

  //
  // create raster builder
  //
  skc_raster_builder_t raster_builder;

  err = skc_raster_builder_create(context,&raster_builder);

  //
  // create a composition
  //
  skc_composition_t composition;

  err = skc_composition_create(context,&composition);

  //
  // create a styling instance
  //
  skc_styling_t styling;

  err = skc_styling_create(context,
                           &styling,
                           svg_doc_layer_count(svg_doc),
                           1000,
                           2 * 1024 * 1024);

  //
  // create a surface
  //
  skc_surface_t surface;

  err = skc_surface_create(context,&surface);

  //
  // create a transform stack
  //
  struct skc_transform_stack * ts = skc_transform_stack_create(32);

  // prime the transform stack with subpixel scale
  skc_transform_stack_push_scale(ts,32.0,32.0);

  //
  // rasterize, render and reclaim svg until escape
  //
  while (!glfwWindowShouldClose(window))
    {
      // save stack
      uint32_t const ts_save = skc_transform_stack_save(ts);

      // poll glfw
      skc_interop_poll(window,ts);

      // decode paths
      skc_path_t * paths = svg_doc_paths_decode(svg_doc,path_builder);

      // decode rasters
      skc_raster_t * rasters = svg_doc_rasters_decode(svg_doc,ts,paths,raster_builder);

      // restore the transform stack
      skc_transform_stack_restore(ts,ts_save);

      // decode layers -- places rasters
      svg_doc_layers_decode(svg_doc,rasters,composition,styling,true/*is_srgb*/);

      // seal the composition
      skc_composition_seal(composition);

      bool           quit   = false;
      uint32_t const clip[] = { 0, 0, 65535, 65535 }; // tile clip is <= 9 bits (512)

      // render the styled composition to the surface
      skc_surface_render(surface,clip,styling,composition,
                         is_render_complete,&quit,
                         skc_interop_get_fb(window));

      // release the paths
      svg_doc_paths_release(svg_doc,paths,context);

      // rasters have been released
      svg_doc_rasters_release(svg_doc,rasters,context);

      // spin until framebuffer is rendered
      while (!quit) {
        // fprintf(stderr,"WAITING ON: !quit\n");
        skc_context_wait(context);
      }

      // blit and swap
      skc_interop_blit(window);

      // print out some useful debug info
      skc_runtime_cl_12_debug(context);

      // rewind the doc
      svg_doc_rewind(svg_doc);

      //
      // I don't like this being here
      //
      float    const rgba[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
      uint32_t       rect[4] = { 0 };

      skc_interop_get_dim(rect+2);

      skc_surface_clear(surface,rgba,rect,
                        skc_interop_get_fb(window));

      // exit(EXIT_SUCCESS);

      // reset styling
      skc_styling_reset(styling);

      // unseal the composition
      skc_composition_unseal(composition,true);
    }

  //
  // dispose of mundane resources
  //
  skc_transform_stack_release(ts);

  //
  // dispose of all SKC resources
  //
  err = skc_surface_release(surface);
  err = skc_styling_release(styling);
  err = skc_composition_release(composition);
  err = skc_raster_builder_release(raster_builder);
  err = skc_path_builder_release(path_builder);
  err = skc_context_release(context);

  //
  // GLFW CLEANUP
  //
  glfwDestroyWindow(window);
  glfwTerminate();

  //
  //
  //
  return EXIT_SUCCESS;
}

//
//
//
