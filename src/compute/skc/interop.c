/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 *
 */

//
//
//

#include <glad/glad.h>
#include <glfw/glfw3.h>

//
//
//

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

//
//
//

#include "common/cl/assert_cl.h"
#include "types.h"

//
//
//

#include "interop.h"
#include "context.h"
#include "runtime_cl_12.h"

//
//
//

#include "svg2skc/transform_stack.h"

//
//
//

#if 1
#define SKC_IMAGE_FORMAT GL_RGBA8
#else
#define SKC_IMAGE_FORMAT GL_RGBA16F
#endif

//
//
//

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

//
//
//

struct skc_interop_fb
{
  cl_context context;

  GLuint     fbo;
  GLuint     rbo;

  cl_mem     mem;

  int        width;
  int        height;

  bool       is_srgb;
  bool       is_vsync_on;
  bool       is_fullscreen;
  bool       is_iconified;
  bool       is_resized;
  bool       is_spinning;
  bool       is_info;

  skc_float  scale;
  skc_float2 translate;
  float      rotate_theta;
};

static struct skc_interop_fb fb =
  {
    .mem           = NULL,

    .is_srgb       = true,
    .is_vsync_on   = false,
    .is_fullscreen = false,
    .is_iconified  = false,
    .is_resized    = true,
    .is_spinning   = false,
    .is_info       = false,

    .scale         = 1.0f,
    .translate     = { 0.0f, 0.0f },
    .rotate_theta  = 0.0f
  };

//
// FPS COUNTER FROM HERE:
//
// http://antongerdelan.net/opengl/glcontext2.html
//

static
void
skc_interop_fps(GLFWwindow * window)
{
  if (fb.is_fullscreen)
    return;

  // static fps counters
  static double stamp_prev  = 0.0;
  static int    frame_count = 0;

  // locals
  double const  stamp_curr  = glfwGetTime();
  double const  elapsed     = stamp_curr - stamp_prev;

  if (elapsed >= 0.5)
    {
      stamp_prev = stamp_curr;

      double const fps = (double)frame_count / elapsed;

      char tmp[64];

      sprintf_s(tmp,64,"(%d x %d) - VSync %s - sRGB %s - FPS: %.2f",
                fb.width,fb.height,
                fb.is_vsync_on ? "ON"      : "OFF",
                fb.is_srgb     ? "ENABLED" : "DISABLED",
                fps);

      glfwSetWindowTitle(window,tmp);

      frame_count = 0;
    }

  frame_count++;
}

//
// INITIALIZE GLFW/GLAD
//

static
void
skc_interop_error_callback(int error, char const * description)
{
  fputs(description,stderr);
}

//
//
//

static
void
skc_interop_iconify_callback(GLFWwindow * window, int iconified)
{
  fb.is_iconified = iconified;
}

//
//
//

static
void
skc_interop_key_callback(GLFWwindow * window, int key, int scancode, int action, int mods)
{
  if (action == GLFW_RELEASE)
    return;

  switch (key)
    {
    case GLFW_KEY_EQUAL:
      fb.rotate_theta = 0.0f;
      break;

    case GLFW_KEY_I:
      fb.is_info = true;
      break;

    case GLFW_KEY_R:
      fb.is_spinning ^= true;
      break;

    case GLFW_KEY_S:
      fb.is_srgb ^= true;
      if (fb.is_srgb)
        glEnable(GL_FRAMEBUFFER_SRGB);
      else
        glDisable(GL_FRAMEBUFFER_SRGB);
      break;

    case GLFW_KEY_V:
      fb.is_vsync_on ^= true;
      glfwSwapInterval(fb.is_vsync_on ? 1 : 0);
      break;

    case GLFW_KEY_W:
      glfwSetWindowSize(window,1024,1024);
      break;

    case GLFW_KEY_ESCAPE:
      glfwSetWindowShouldClose(window,GL_TRUE);
      break;
    }
}

static
void
skc_interop_window_size_callback(GLFWwindow * window, int width, int height)
{
  fb.width      = width;
  fb.height     = height;
  fb.is_resized = true;

#if 0
  skc_render_kernel_set_clip(0,0,width,height);
#endif
}

static
void
skc_interop_scale(double const scale_offset)
{
#define SKC_SCALE_FACTOR 1.05

  static double scale_exp = 0.0;

  scale_exp += scale_offset;
  fb.scale   = (float)pow(SKC_SCALE_FACTOR,scale_exp);
}

static
void
skc_interop_scroll_callback(GLFWwindow * window, double xoffset, double yoffset)
{
  bool const ctrl =
    (glfwGetKey(window,GLFW_KEY_LEFT_CONTROL)  == GLFW_PRESS) ||
    (glfwGetKey(window,GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS);

  if (!ctrl)
    return;

  skc_interop_scale(yoffset);
}

static
void
skc_interop_translate(float const dx, float const dy)
{
  float const dx_scaled = dx / fb.scale;
  float const dy_scaled = dy / fb.scale;

  float const cos_theta = cosf(fb.rotate_theta); // replace with cospi if available
  float const sin_theta = sinf(fb.rotate_theta); // replace with sinpi if available

  fb.translate.x += dx_scaled*cos_theta + dy_scaled*sin_theta;
  fb.translate.y += dy_scaled*cos_theta - dx_scaled*sin_theta;
}

static
void
skc_interop_cursor_position_callback(GLFWwindow * window, double x, double y)
{
  int const state = glfwGetMouseButton(window,GLFW_MOUSE_BUTTON_LEFT);

  static bool  is_mouse_dragging = false;
  static float x_prev=0.0, y_prev=0.0;

  float const mx = (float)x;
  float const my = (float)y;

  if (state == GLFW_PRESS)
    {
      if (is_mouse_dragging)
        {
          const bool ctrl =
            (glfwGetKey(window,GLFW_KEY_LEFT_CONTROL)  == GLFW_PRESS) ||
            (glfwGetKey(window,GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS);

          if (ctrl)
            {
              float const cx  = 0.5f * fb.width;
              float const cy  = 0.5f * fb.height;

              // find angle between mouse and center
              float const vx  = x_prev - cx;
              float const vy  = y_prev - cy;

              float const wx  = mx - cx;
              float const wy  = my - cy;

              float const len = sqrtf((vx*vx + vy*vy) * (wx*wx + wy*wy));

              if (len > 0.0f)
                {
                  float const dot = vx*wx + vy*wy;
                  float const da  = acosf(dot / len);

                  if (vx*wy - vy*wx >= 0.0f)
                    fb.rotate_theta += da;
                  else
                    fb.rotate_theta -= da;

                  fb.rotate_theta = fmodf(fb.rotate_theta,(float)(M_PI*2.0));
                }
            }
          else
            {
              skc_interop_translate(mx - x_prev,
                                    my - y_prev);
            }
        }
      else
        {
          is_mouse_dragging = true;
        }

      x_prev = mx;
      y_prev = my;
    }
  else
    {
      is_mouse_dragging = false;
    }
}

//
//
//

static
void
skc_interop_resize()
{
  fb.is_resized = false;

  // release the image2d
  if (fb.mem != NULL)
    cl(ReleaseMemObject(fb.mem));

  // resize rbo
  glNamedRenderbufferStorage(fb.rbo,
                             SKC_IMAGE_FORMAT,
                             fb.width,
                             fb.height);

  // attach rbo to fbo
  glNamedFramebufferRenderbuffer(fb.fbo,
                                 GL_COLOR_ATTACHMENT0,
                                 GL_RENDERBUFFER,
                                 fb.rbo);
  //
  //
  //
  cl_int cl_err;

  fb.mem = clCreateFromGLRenderbuffer(fb.context,
                                      CL_MEM_WRITE_ONLY,
                                      fb.rbo,
                                      &cl_err); cl_ok(cl_err);
  //
  // for debugging porpoises!
  //
  cl_image_format format;

  cl(GetImageInfo(fb.mem,
                  CL_IMAGE_FORMAT,
                  sizeof(format),
                  &format,
                  NULL));
}

//
//
//

static
void
skc_interop_acquire()
{
  // frame buffer object
  glCreateFramebuffers(1,&fb.fbo);

  // render buffer object w/a color buffer
  glCreateRenderbuffers(1,&fb.rbo);

  // size rbo
  glNamedRenderbufferStorage(fb.rbo,
                             SKC_IMAGE_FORMAT,
                             fb.width,
                             fb.height);

  // attach rbo to fbo
  glNamedFramebufferRenderbuffer(fb.fbo,
                                 GL_COLOR_ATTACHMENT0,
                                 GL_RENDERBUFFER,
                                 fb.rbo);
}

void
skc_interop_register(skc_context_t context)
{
  fb.context = context->runtime->cl.context;
}

//
//
//

void
skc_interop_init(GLFWwindow * * window)
{
  //
  // INITIALIZE GLFW/GLAD
  //
  glfwSetErrorCallback(skc_interop_error_callback);

  if (!glfwInit())
    exit(EXIT_FAILURE);

  GLFWmonitor       * const primary = glfwGetPrimaryMonitor();
  GLFWvidmode const * const mode    = glfwGetVideoMode(primary);

  if (fb.is_fullscreen)
    {
      fb.width  = mode->width;
      fb.height = mode->height;
    }
  else
    {
      fb.width  = 1600;
      fb.height = 1024;
    }

  glfwWindowHint(GLFW_ALPHA_BITS,            0);
  glfwWindowHint(GLFW_DEPTH_BITS,            0);
  glfwWindowHint(GLFW_STENCIL_BITS,          0);

  glfwWindowHint(GLFW_SRGB_CAPABLE,          GL_TRUE);

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);

  glfwWindowHint(GLFW_OPENGL_PROFILE,        GLFW_OPENGL_CORE_PROFILE);

  *window = glfwCreateWindow(fb.width,fb.height,
                             "Skia Compute",
                             fb.is_fullscreen ? primary : NULL,
                             NULL);

  if (*window == NULL)
    {
      glfwTerminate();
      exit(EXIT_FAILURE);
    }

  glfwMakeContextCurrent(*window);

  // set up GLAD
  gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

  // ignore vsync for now
  glfwSwapInterval(fb.is_vsync_on ? 1 : 0);

  // only copy r/g/b
  glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_FALSE);

  // enable SRGB, disable scissor
  glEnable(GL_FRAMEBUFFER_SRGB);
  glDisable(GL_SCISSOR_TEST);

  //
  // SET USER POINTER AND CALLBACKS
  //
  glfwSetKeyCallback            (*window,skc_interop_key_callback);
  glfwSetFramebufferSizeCallback(*window,skc_interop_window_size_callback);
  glfwSetScrollCallback         (*window,skc_interop_scroll_callback);
  glfwSetCursorPosCallback      (*window,skc_interop_cursor_position_callback);
  glfwSetWindowIconifyCallback  (*window,skc_interop_iconify_callback);

  //
  //
  //
  fprintf(stderr,
          "GL_VENDOR   : %s\n"
          "GL_RENDERER : %s\n",
          glGetString(GL_VENDOR),
          glGetString(GL_RENDERER));

  //
  // acquire an FBO/RBO
  //
  skc_interop_acquire();
}

//
//
//

#define SKC_ROTATE_STEP ((float)(M_PI / 180.0))

static
void
skc_interop_transform(struct skc_transform_stack * ts)
{
  // OpenGL'ism
  skc_transform_stack_push_affine(ts,
                                  1.0f, 0.0f,0.0f,
                                  0.0f,-1.0f,(float)fb.height);
  // multiply
  skc_transform_stack_concat(ts);

  // spinner...
  if (fb.is_spinning)
    fb.rotate_theta = fmodf(fb.rotate_theta + SKC_ROTATE_STEP,(float)(M_PI*2.0));
  
  // always rotate and scale around surface center point
  skc_transform_stack_push_rotate_scale_xy(ts,
                                           fb.rotate_theta,
                                           fb.scale,fb.scale,
                                           0.5f*fb.width,0.5f*fb.height);
  skc_transform_stack_concat(ts);

  // where did the mouse take us?
  skc_transform_stack_push_translate(ts,
                                     fb.translate.x,fb.translate.y);
  skc_transform_stack_concat(ts);
}


void
skc_interop_poll(GLFWwindow                 * window,
                 struct skc_transform_stack * ts)
{
  // wait until uniconified
  while (fb.is_iconified)
    {
      glfwWaitEvents();
      continue;
    }

  // what's happended?
  glfwPollEvents();

  // resize?
  if (fb.is_resized)
    skc_interop_resize();

  // monitor fps
  skc_interop_fps(window);

  skc_interop_transform(ts);
}

//
//
//

void
skc_interop_blit(GLFWwindow * window)
{
  // blit skc rbo
  glBlitNamedFramebuffer(fb.fbo,0,
                         0,0,fb.width,fb.height,
                         0,0,fb.width,fb.height,
                         GL_COLOR_BUFFER_BIT,
                         GL_NEAREST);

#if 0
  //
  // FIXME -- this clear does nothing!
  //
  // As a hack we're clearing the interop'd RBO with a
  // clEnqueueFillImage().
  //
  float const rgba[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
  // GLenum const attachments[] = { GL_COLOR_ATTACHMENT0 };
  // glInvalidateNamedFramebufferData(fb.fbo,1,attachments);
  glClearNamedFramebufferfv(fb.fbo,GL_COLOR,0,rgba);
#endif

  // swap buffers
  glfwSwapBuffers(window);
}

//
//
//

void *
skc_interop_get_fb(GLFWwindow * window)
{
  glFlush();

  return fb.mem;
}

//
//
//

void
skc_interop_get_dim(uint32_t dim[2])
{
  dim[0] = fb.width;
  dim[1] = fb.height;
}

//
//
//


