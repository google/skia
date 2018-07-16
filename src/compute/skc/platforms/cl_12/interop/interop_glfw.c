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

//
//
//

#include "interop.h"

//
//
//

#include "skc_cl.h"
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

struct skc_interop
{
  GLFWwindow              * window;

  cl_context                context_cl;

  GLuint                    fbo;
  GLuint                    rbo;

  struct skc_framebuffer_cl fb;

  int                       width;
  int                       height;

  bool                      is_msecs;
  bool                      is_srgb;
  bool                      is_vsync_on;
  bool                      is_fullscreen;
  bool                      is_iconified;
  bool                      is_resized;
  bool                      is_spinning;
  bool                      is_transform;

  skc_float                 scale;
  skc_float2                translate;
  float                     rotate_theta;

  int                       key;
};

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
  struct skc_interop * interop = glfwGetWindowUserPointer(window);

  interop->is_iconified = iconified;
}

//
//
//

static
void
skc_interop_key_callback(GLFWwindow * window, int key, int scancode, int action, int mods)
{
  struct skc_interop * interop = glfwGetWindowUserPointer(window);

  if (action == GLFW_RELEASE)
    return;

  switch (key)
    {
    case GLFW_KEY_EQUAL:
      interop->rotate_theta = 0.0f;
      interop->is_transform = true;
      break;

    case GLFW_KEY_M:
      interop->is_msecs ^= true;
      break;

    case GLFW_KEY_R:
      interop->is_spinning ^= true;
      break;

    case GLFW_KEY_S:
      interop->is_srgb ^= true;
      if (interop->is_srgb)
        glEnable(GL_FRAMEBUFFER_SRGB);
      else
        glDisable(GL_FRAMEBUFFER_SRGB);
      break;

    case GLFW_KEY_V:
      interop->is_vsync_on ^= true;
      glfwSwapInterval(interop->is_vsync_on ? 1 : 0);
      break;

    case GLFW_KEY_W:
      glfwSetWindowSize(window,1024,1024);
      break;

    case GLFW_KEY_ESCAPE:
      glfwSetWindowShouldClose(window,GL_TRUE);
      break;

    default:
      interop->key = key;
    }
}

static
void
skc_interop_window_size_callback(GLFWwindow * window, int width, int height)
{
  struct skc_interop * interop = glfwGetWindowUserPointer(window);

  interop->width        = width;
  interop->height       = height;
  interop->is_resized   = true;
  interop->is_transform = true;

#if 0
  skc_render_kernel_set_clip(0,0,width,height);
#endif
}

static
void
skc_interop_scale(struct skc_interop * interop, double const scale_offset)
{
#define SKC_SCALE_FACTOR 1.05

  static double scale_exp = 0.0;

  scale_exp += scale_offset;

  interop->scale = (float)pow(SKC_SCALE_FACTOR,scale_exp);
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

  struct skc_interop * interop = glfwGetWindowUserPointer(window);

  skc_interop_scale(interop,yoffset);

  interop->is_transform = true;
}

static
void
skc_interop_translate(struct skc_interop * interop, float const dx, float const dy)
{
  float const dx_scaled = dx / interop->scale;
  float const dy_scaled = dy / interop->scale;

  float const cos_theta = cosf(interop->rotate_theta); // replace with cospi if available
  float const sin_theta = sinf(interop->rotate_theta); // replace with sinpi if available

  interop->translate.x += dx_scaled*cos_theta + dy_scaled*sin_theta;
  interop->translate.y += dy_scaled*cos_theta - dx_scaled*sin_theta;
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
      struct skc_interop * interop = glfwGetWindowUserPointer(window);

      if (is_mouse_dragging)
        {
          const bool ctrl =
            (glfwGetKey(window,GLFW_KEY_LEFT_CONTROL)  == GLFW_PRESS) ||
            (glfwGetKey(window,GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS);

          if (ctrl)
            {
              float const cx  = 0.5f * interop->width;
              float const cy  = 0.5f * interop->height;

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
                    interop->rotate_theta += da;
                  else
                    interop->rotate_theta -= da;

                  interop->rotate_theta = fmodf(interop->rotate_theta,(float)(M_PI*2.0));
                }
            }
          else
            {
              skc_interop_translate(interop,
                                    mx - x_prev,
                                    my - y_prev);
            }

          interop->is_transform = true;
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
skc_interop_acquire(struct skc_interop * interop)
{
  // frame buffer object
  glCreateFramebuffers(1,&interop->fbo);

  // render buffer object w/a color buffer
  glCreateRenderbuffers(1,&interop->rbo);

  // size rbo
  glNamedRenderbufferStorage(interop->rbo,
                             SKC_IMAGE_FORMAT,
                             interop->width,
                             interop->height);

  // attach rbo to fbo
  glNamedFramebufferRenderbuffer(interop->fbo,
                                 GL_COLOR_ATTACHMENT0,
                                 GL_RENDERBUFFER,
                                 interop->rbo);
}

//
//
//

struct skc_interop *
skc_interop_create()
{
  struct skc_interop * interop = malloc(sizeof(*interop));

  *interop = (struct skc_interop)
    {
     .fb            = { .type        = SKC_FRAMEBUFFER_CL_GL_RENDERBUFFER,
                        .mem         = NULL,
                        .interop     = interop,
                        .post_render = skc_interop_blit },

     .is_msecs      = false,
     .is_srgb       = true,
     .is_vsync_on   = false,
     .is_fullscreen = false,
     .is_iconified  = false,
     .is_resized    = true,
     .is_spinning   = false,
     .is_transform  = true,

     .scale         = 1.0f,
     .translate     = { 0.0f, 0.0f },
     .rotate_theta  = 0.0f,

     .key           = 0
    };

  //
  // INITIALIZE GLFW/GLAD
  //
  glfwSetErrorCallback(skc_interop_error_callback);

  if (!glfwInit())
    exit(EXIT_FAILURE);

  GLFWmonitor       * const primary = glfwGetPrimaryMonitor();
  GLFWvidmode const * const mode    = glfwGetVideoMode(primary);

  if (interop->is_fullscreen)
    {
      interop->width  = mode->width;
      interop->height = mode->height;
    }
  else
    {
      interop->width  = 1600;
      interop->height = 1600;
    }

  glfwWindowHint(GLFW_ALPHA_BITS,            0);
  glfwWindowHint(GLFW_DEPTH_BITS,            0);
  glfwWindowHint(GLFW_STENCIL_BITS,          0);

  glfwWindowHint(GLFW_SRGB_CAPABLE,          GL_TRUE);

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);

  glfwWindowHint(GLFW_OPENGL_PROFILE,        GLFW_OPENGL_CORE_PROFILE);

  interop->window = glfwCreateWindow(interop->width,
                                     interop->height,
                                     "Skia Compute",
                                     interop->is_fullscreen ? primary : NULL,
                                     NULL);

  if (interop->window == NULL)
    {
      glfwTerminate();
      exit(EXIT_FAILURE);
    }

  // save back pointer
  glfwSetWindowUserPointer(interop->window,interop);

  glfwMakeContextCurrent(interop->window);

  // set up GLAD
  gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

  // ignore vsync for now
  glfwSwapInterval(interop->is_vsync_on ? 1 : 0);

  // only copy r/g/b
  glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_FALSE);

  // enable SRGB, disable scissor
  glEnable(GL_FRAMEBUFFER_SRGB);
  glDisable(GL_SCISSOR_TEST);

  //
  // SET USER POINTER AND CALLBACKS
  //
  glfwSetKeyCallback            (interop->window,skc_interop_key_callback);
  glfwSetFramebufferSizeCallback(interop->window,skc_interop_window_size_callback);
  glfwSetScrollCallback         (interop->window,skc_interop_scroll_callback);
  glfwSetCursorPosCallback      (interop->window,skc_interop_cursor_position_callback);
  glfwSetWindowIconifyCallback  (interop->window,skc_interop_iconify_callback);

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
  skc_interop_acquire(interop);

  return interop;
}

//
//
//

void
skc_interop_destroy(struct skc_interop * interop)
{
  glfwDestroyWindow(interop->window);
  glfwTerminate();

  free(interop);
}

//
//
//

void
skc_interop_set_cl_context(struct skc_interop * interop,
                           cl_context           context_cl)
{
  interop->context_cl = context_cl;
}

//
//
//

cl_context_properties
skc_interop_get_wgl_context()
{
  return (cl_context_properties)wglGetCurrentContext();
}

cl_context_properties
skc_interop_get_wgl_dc()
{
  return (cl_context_properties)wglGetCurrentDC();
}

//
//
//

#define SKC_ROTATE_STEP ((float)(M_PI / 180.0))

void
skc_interop_transform(struct skc_interop         * interop,
                      struct skc_transform_stack * ts)
{
  // OpenGL'ism
  skc_transform_stack_push_affine(ts,
                                  1.0f, 0.0f,0.0f,
                                  0.0f,-1.0f,(float)interop->height);
  // multiply
  skc_transform_stack_concat(ts);

  // spinner...
  if (interop->is_spinning)
    interop->rotate_theta = fmodf(interop->rotate_theta + SKC_ROTATE_STEP,(float)(M_PI*2.0));

  // always rotate and scale around surface center point
  skc_transform_stack_push_rotate_scale_xy(ts,
                                           interop->rotate_theta,
                                           interop->scale,
                                           interop->scale,
                                           0.5f*interop->width,
                                           0.5f*interop->height);
  skc_transform_stack_concat(ts);

  // where did the mouse take us?
  skc_transform_stack_push_translate(ts,
                                     interop->translate.x,
                                     interop->translate.y);
  skc_transform_stack_concat(ts);
}

//
//
//

static
void
skc_interop_resize(struct skc_interop * interop)
{
  interop->is_resized = false;

  // release the image2d
  if (interop->fb.mem != NULL)
    cl(ReleaseMemObject(interop->fb.mem));

  // resize rbo
  glNamedRenderbufferStorage(interop->rbo,
                             SKC_IMAGE_FORMAT,
                             interop->width,
                             interop->height);

  // attach rbo to fbo
  glNamedFramebufferRenderbuffer(interop->fbo,
                                 GL_COLOR_ATTACHMENT0,
                                 GL_RENDERBUFFER,
                                 interop->rbo);
  //
  //
  //
  cl_int cl_err;

  interop->fb.mem = clCreateFromGLRenderbuffer(interop->context_cl,
                                               CL_MEM_WRITE_ONLY,
                                               interop->rbo,
                                               &cl_err); cl_ok(cl_err);
  //
  // for debugging porpoises!
  //
#if 0
  cl_image_format format;

  cl(GetImageInfo(interop->fb.mem,
                  CL_IMAGE_FORMAT,
                  sizeof(format),
                  &format,
                  NULL));
#endif
}

//
// FPS COUNTER FROM HERE:
//
// http://antongerdelan.net/opengl/glcontext2.html
//

static
void
skc_interop_fps(struct skc_interop * interop)
{
  if (interop->is_fullscreen)
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

      char tmp[64];

      if (interop->is_msecs)
        {
          double const msecs = min(elapsed * 1000 / frame_count,9999.9);

          sprintf_s(tmp,64,"%5.1f MSECS - (%d x %d) - VSync %s - sRGB %s",
                    msecs,
                    interop->width,interop->height,
                    interop->is_vsync_on ? "ON"      : "OFF",
                    interop->is_srgb     ? "ENABLED" : "DISABLED");
        }
      else
        {
          double const fps = min((double)frame_count / elapsed,9999.9);

          sprintf_s(tmp,64,"%5.1f FPS - (%d x %d) - VSync %s - sRGB %s",
                    fps,
                    interop->width,interop->height,
                    interop->is_vsync_on ? "ON"      : "OFF",
                    interop->is_srgb     ? "ENABLED" : "DISABLED");
        }

      glfwSetWindowTitle(interop->window,tmp);

      frame_count = 0;
    }

  frame_count++;
}

//
//
//

bool
skc_interop_poll(struct skc_interop * interop, int * key)
{
  // wait until uniconified
  while (interop->is_iconified)
    {
      glfwWaitEvents();
      continue;
    }

  // what's happended?
  glfwPollEvents();

  // resize?
  if (interop->is_resized)
    skc_interop_resize(interop);

  // monitor fps
  skc_interop_fps(interop);

  if (key != NULL)
    {
      *key         = interop->key;
      interop->key = 0;
    }

  bool const is_transform = interop->is_transform || interop->is_spinning;

  interop->is_transform = false;

  return is_transform;
}

//
//
//

void
skc_interop_blit(struct skc_interop * interop)
{
  // blit skc rbo
  glBlitNamedFramebuffer(interop->fbo,0,
                         0,0,interop->width,interop->height,
                         0,0,interop->width,interop->height,
                         GL_COLOR_BUFFER_BIT,
                         GL_NEAREST);

  // swap buffers
  glfwSwapBuffers(interop->window);

#if 0
  //
  // FIXME -- this clear does nothing!
  //
  // As a hack we're clearing the interop'd RBO with a
  // clEnqueueFillImage().
  //
  GLenum const attachments[] = { GL_COLOR_ATTACHMENT0 };
  glInvalidateNamedFramebufferData(interop->fbo,1,attachments);
  float  const rgba[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
  glClearNamedFramebufferfv(interop->fbo,GL_COLOR,0,rgba);
#endif
}

//
//
//

skc_framebuffer_t
skc_interop_get_framebuffer(struct skc_interop * interop)
{
  // glFlush();
  glFinish();

  return &interop->fb;
}

//
//
//

bool
skc_interop_should_exit(struct skc_interop * interop)
{
  return glfwWindowShouldClose(interop->window);
}

//
//
//

void
skc_interop_get_size(struct skc_interop * interop,
                     uint32_t           * width,
                     uint32_t           * height)
{
  *width  = interop->width;
  *height = interop->height;
}

//
//
//
