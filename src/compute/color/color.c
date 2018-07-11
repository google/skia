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

#include <math.h>
#include "color.h"

//
// COLOR UTILITIES
//

//
// CONVERT FROM 0xAARRGGBB WORD ORDER INTO f32[4]
//

void
color_rgb32_to_rgba_f32(float rgba[4], const uint32_t rgb, const float opacity)
{
  rgba[2] = (float)(rgb        & 0xFF) / 255.0f; // b
  rgba[1] = (float)(rgb  >> 8  & 0xFF) / 255.0f; // g
  rgba[0] = (float)(rgb  >> 16 & 0xFF) / 255.0f; // r
  rgba[3] = opacity;
}

void
color_argb32_to_rgba_f32(float rgba[4], const uint32_t argb)
{
  rgba[2] = (float)(argb       & 0xFF) / 255.0f; // b
  rgba[1] = (float)(argb >> 8  & 0xFF) / 255.0f; // g
  rgba[0] = (float)(argb >> 16 & 0xFF) / 255.0f; // r
  rgba[3] = (float)(argb >> 24 & 0xFF) / 255.0f; // a
}

//
// { r, g, b, a } ==> { r*a, g*a, b*a, a }
//

void
color_premultiply_rgba_f32(float rgba[4])
{
  if (rgba[3] < 1.0f)
    {
      for (uint32_t ii=0; ii<3; ii++)
        rgba[ii] = rgba[ii] * rgba[3];
    }
}

//
// EXT_framebuffer_sRGB:
// https://www.opengl.org/registry/specs/EXT/framebuffer_sRGB.txt
//
//        {  cs / 12.92,                 cs <= 0.04045
//   cl = {
//        {  ((cs + 0.055)/1.055)^2.4,   cs >  0.04045
//
//
//        {  0.0,                          0         <= cl
//        {  12.92 * cl,                   0         <  cl <  0.0031308
//   cs = {  1.055 * cl^0.41666 - 0.055,   0.0031308 <= cl <  1
//        {  1.0,                                       cl >= 1
//

void
color_srgb_to_linear_rgb_f32(float rgb[3])
{
  for (uint32_t ii=0; ii<3; ii++)
    {
      if (rgb[ii] <= 0.04045f)
        {
          rgb[ii] = rgb[ii] / 12.92f;
        }
      else
        {
          rgb[ii] = powf((rgb[ii] + 0.055f) / 1.055f, 2.4f);
        }
    }
}

void
color_linear_to_srgb_rgb_f32(float rgb[3])
{
  for (uint32_t ii=0; ii<3; ii++)
    {
      if (rgb[ii] < 0.0031308f)
        {
          rgb[ii] = rgb[ii] * 12.92f;
        }
      else
        {
          rgb[ii] = powf(rgb[ii], 0.41666f) * 1.044f - 0.055f;
        }
    }
}

//
//
//

static
float
lerpf(float const a, float const b, float const t)
{
  return fmaf(t, b, fmaf(-t, a, a));
}

//
//
//

void
color_linear_lerp_rgba_f32(float       rgba_m[4],
                           float const rgba_a[4],
                           float const rgba_b[4],
                           float const t)
{
  for (int32_t ii=0; ii<4; ii++)
    rgba_m[ii] = lerpf(rgba_a[ii],rgba_b[ii],t);
}

//
//
//
