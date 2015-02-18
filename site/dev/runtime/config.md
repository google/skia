Runtime Configuration Settings
==============================

Here is a (partial) list of Skia's runtime configuration settings:

## Warning suppression:

* configuration name: images.gif.suppressDecoderWarnings
  environment variable: skia_images_gif_suppressDecoderWarnings
  type: boolean
  description: Suppress GIF warnings and errors when calling image decode
               functions.  
  default: true.

* configuration name: images.jpeg.suppressDecoderWarnings
  environment variable: skia_images_jpeg_suppressDecoderWarnings
  type: boolean
  description: Suppress most JPG warnings when calling decode functions.
  default: false in debug, true otherwise.
  
* configuration name: images.jpeg.suppressDecoderErrors
  environment variable: skia_images_jpeg_suppressDecoderErrors
  type: boolean
  description: Suppress most JPG error messages when decode function fails.
  default: false in debug, true otherwise.

*  configuration name: images.png.suppressDecoderWarnings
  environment variable: skia_images_png_suppressDecoderWarnings
  type: boolean
  description: Suppress most PNG warnings when calling image decode functions.
  default: false in debug, true otherwise.

## Other:

* configuration name: bitmap.filter
  environment variable: skia_bitmap_filter
  type: string
  description: Which scanline bitmap filter to use \[mitchell, lanczos, hamming,
               gaussian, triangle, box\] 
  default: mitchell

* configuration name: mask.filter.analyticNinePatch
  environment variable: skia_mask_filter_analyticNinePatch
  type: boolean
  description: Use the faster analytic blur approach for ninepatch rects
  default: \?

* configuration name: gpu.deferContext
  environment variable: skia_gpu_deferContext
  type: boolean
  description: Defers rendering in GrContext via GrInOrderDrawBuffer
  default: true

* configuration name: gpu.dumpFontCache
  environment variable: skia_gpu_dumpFontCache
  type: boolean
  description: Dump the contents of the font cache before every purge
  default: false

* configuration name: bitmap.filter.highQualitySSE
  environment variable: skia_bitmap_filter_highQualitySSE
  type: boolean
  description: Use SSE optimized version of high quality image filters
  default: false

## Use:

These configuration values can be changed at runtime by including this in your
program:

<!--?prettify?-->
~~~~
#include "SkRTConf.h"
/*...*/
int main() { 
    SK_CONF_SET( configuration_name, new_value );
    /*...*/
~~~~

Or by setting the corresponding environment variable before starting the
program. For example, in Bourne shell:

<!--?prettify?-->
~~~~
#!/bin/sh
export skia_environment_variable="new_value"
your_program
~~~~

